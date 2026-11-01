/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "GLContext.h"
#include "GLContextProvider.h"
#include "GLScreenBuffer.h"
#include "SharedSurfaceGL.h"
#include "gfxPlatform.h"
#include "gtest/gtest.h"

namespace mozilla::gl {

TEST(GLScreenBufferSwapChain, RecyclesSurfaceWhenFactoryCoercesColorSpace)
{
  gfxPlatform::GetPlatform();

  nsCString failureId;
  RefPtr<GLContext> gl = GLContextProvider::CreateHeadless({}, &failureId);
  ASSERT_TRUE(gl)
  << failureId.get();
  ASSERT_TRUE(gl->MakeCurrent());

  SwapChain swapChain;
  swapChain.mFactory = MakeUnique<SurfaceFactory_Basic>(*gl);
  ASSERT_FALSE(swapChain.mFactory->SupportsCspaces());

  const gfx::IntSize size(16, 16);
  std::shared_ptr<SharedSurface> firstSurface;
  {
    auto presenter = swapChain.Acquire(size, gfx::ColorSpace2::SRGB,
                                       gfx::TransferFunction::SRGB);
    ASSERT_TRUE(presenter);
    firstSurface = presenter->BackBuffer();
    ASSERT_TRUE(firstSurface);
    EXPECT_EQ(firstSurface->mDesc.colorSpace, gfx::ColorSpace2::Display);
  }

  ASSERT_TRUE(swapChain.StoreRecycledSurface(firstSurface));

  auto presenter = swapChain.Acquire(size, gfx::ColorSpace2::SRGB,
                                     gfx::TransferFunction::SRGB);
  ASSERT_TRUE(presenter);
  EXPECT_EQ(presenter->BackBuffer().get(), firstSurface.get());
}

}  // namespace mozilla::gl
