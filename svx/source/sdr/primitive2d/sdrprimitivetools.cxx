/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sdr/primitive2d/sdrprimitivetools.hxx>
#include <vcl/bitmapaccess.hxx>
#include <osl/mutex.hxx>
#include <vcl/lazydelete.hxx>


// helper methods

namespace drawinglayer
{
    namespace primitive2d
    {
        BitmapEx createDefaultCross_3x3(const basegfx::BColor& rBColor)
        {
            static vcl::DeleteOnDeinit< BitmapEx > aRetVal(nullptr);
            static basegfx::BColor aColor;
            static ::osl::Mutex aMutex;

            ::osl::MutexGuard aGuard(aMutex);

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

                Bitmap::ReleaseAccess(pWContent);
                Bitmap::ReleaseAccess(pWMask);

                // create and exchange at aRetVal
                delete aRetVal.set(new BitmapEx(aContent, aMask));
            }

            return aRetVal.get() ? *aRetVal.get() : BitmapEx();
        }
    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
