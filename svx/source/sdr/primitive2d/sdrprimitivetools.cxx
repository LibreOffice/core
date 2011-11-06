/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include "precompiled_svx.hxx"
#include <svx/sdr/primitive2d/sdrprimitivetools.hxx>
#include <vcl/bmpacc.hxx>
#include <osl/mutex.hxx>
#include <vcl/lazydelete.hxx>

//////////////////////////////////////////////////////////////////////////////
// helper methods

namespace drawinglayer
{
    namespace primitive2d
    {
        BitmapEx createDefaultCross_3x3(const basegfx::BColor& rBColor)
        {
            static vcl::DeleteOnDeinit< BitmapEx > aRetVal(0);
            static basegfx::BColor aColor;
            ::osl::Mutex m_mutex;

            if(!aRetVal.get() || rBColor != aColor)
            {
                // copy values
                aColor = rBColor;

                // create bitmap
                Bitmap aContent(Size(3, 3), 24);
                Bitmap aMask(Size(3, 3), 1);
                BitmapWriteAccess* pWContent = aContent.AcquireWriteAccess();
                BitmapWriteAccess* pWMask = aMask.AcquireWriteAccess();
                OSL_ENSURE(pWContent && pWMask, "No WriteAccess to bitmap (!)");
                const Color aVCLColor(aColor);
                const BitmapColor aPixColor(aVCLColor);
                const BitmapColor aMaskColor(0x01);

                // Y,X unusual order (!)
                pWContent->SetPixel(0, 1, aPixColor);
                pWContent->SetPixel(1, 0, aPixColor);
                pWContent->SetPixel(1, 1, aPixColor);
                pWContent->SetPixel(1, 2, aPixColor);
                pWContent->SetPixel(2, 1, aPixColor);

                pWMask->SetPixel(0, 0, aMaskColor);
                pWMask->SetPixel(0, 2, aMaskColor);
                pWMask->SetPixel(2, 0, aMaskColor);
                pWMask->SetPixel(2, 2, aMaskColor);

                aContent.ReleaseAccess(pWContent);
                aMask.ReleaseAccess(pWMask);

                // create and exchange at aRetVal
                delete aRetVal.set(new BitmapEx(aContent, aMask));
            }

            return aRetVal.get() ? *aRetVal.get() : BitmapEx();
        }

        BitmapEx createDefaultGluepoint_7x7(const basegfx::BColor& rBColorA, const basegfx::BColor& rBColorB)
        {
            static vcl::DeleteOnDeinit< BitmapEx > aRetVal(0);
            static basegfx::BColor aColorA;
            static basegfx::BColor aColorB;
            ::osl::Mutex m_mutex;

            if(!aRetVal.get() || rBColorA != aColorA || rBColorB != aColorB)
            {
                // copy values
                aColorA = rBColorA;
                aColorB = rBColorB;

                // create bitmap
                Bitmap aContent(Size(7, 7), 24);
                Bitmap aMask(Size(7, 7), 1);
                BitmapWriteAccess* pWContent = aContent.AcquireWriteAccess();
                BitmapWriteAccess* pWMask = aMask.AcquireWriteAccess();
                OSL_ENSURE(pWContent && pWMask, "No WriteAccess to bitmap (!)");
                const Color aColA(aColorA);
                const Color aColB(aColorB);
                const BitmapColor aPixColorA(aColA);
                const BitmapColor aPixColorB(aColB);
                const BitmapColor aMaskColor(0x01);

                // Y,X unusual order (!)
                pWContent->SetPixel(0, 1, aPixColorA);
                pWContent->SetPixel(0, 5, aPixColorA);
                pWContent->SetPixel(1, 0, aPixColorA);
                pWContent->SetPixel(1, 2, aPixColorA);
                pWContent->SetPixel(1, 4, aPixColorA);
                pWContent->SetPixel(1, 6, aPixColorA);
                pWContent->SetPixel(2, 1, aPixColorA);
                pWContent->SetPixel(2, 3, aPixColorA);
                pWContent->SetPixel(2, 5, aPixColorA);
                pWContent->SetPixel(3, 2, aPixColorA);
                pWContent->SetPixel(3, 4, aPixColorA);
                pWContent->SetPixel(4, 1, aPixColorA);
                pWContent->SetPixel(4, 3, aPixColorA);
                pWContent->SetPixel(4, 5, aPixColorA);
                pWContent->SetPixel(5, 0, aPixColorA);
                pWContent->SetPixel(5, 2, aPixColorA);
                pWContent->SetPixel(5, 4, aPixColorA);
                pWContent->SetPixel(5, 6, aPixColorA);
                pWContent->SetPixel(6, 1, aPixColorA);
                pWContent->SetPixel(6, 5, aPixColorA);

                pWContent->SetPixel(1, 1, aPixColorB);
                pWContent->SetPixel(1, 5, aPixColorB);
                pWContent->SetPixel(2, 2, aPixColorB);
                pWContent->SetPixel(2, 4, aPixColorB);
                pWContent->SetPixel(3, 3, aPixColorB);
                pWContent->SetPixel(4, 2, aPixColorB);
                pWContent->SetPixel(4, 4, aPixColorB);
                pWContent->SetPixel(5, 1, aPixColorB);
                pWContent->SetPixel(5, 5, aPixColorB);

                pWMask->SetPixel(0, 0, aMaskColor);
                pWMask->SetPixel(0, 2, aMaskColor);
                pWMask->SetPixel(0, 3, aMaskColor);
                pWMask->SetPixel(0, 4, aMaskColor);
                pWMask->SetPixel(0, 6, aMaskColor);
                pWMask->SetPixel(1, 3, aMaskColor);
                pWMask->SetPixel(2, 0, aMaskColor);
                pWMask->SetPixel(2, 6, aMaskColor);
                pWMask->SetPixel(3, 0, aMaskColor);
                pWMask->SetPixel(3, 1, aMaskColor);
                pWMask->SetPixel(3, 5, aMaskColor);
                pWMask->SetPixel(3, 6, aMaskColor);
                pWMask->SetPixel(4, 0, aMaskColor);
                pWMask->SetPixel(4, 6, aMaskColor);
                pWMask->SetPixel(5, 3, aMaskColor);
                pWMask->SetPixel(6, 0, aMaskColor);
                pWMask->SetPixel(6, 2, aMaskColor);
                pWMask->SetPixel(6, 3, aMaskColor);
                pWMask->SetPixel(6, 4, aMaskColor);
                pWMask->SetPixel(6, 6, aMaskColor);

                aContent.ReleaseAccess(pWContent);
                aMask.ReleaseAccess(pWMask);

                // create and exchange at aRetVal
                delete aRetVal.set(new BitmapEx(aContent, aMask));
            }

            return aRetVal.get() ? *aRetVal.get() : BitmapEx();
        }

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
