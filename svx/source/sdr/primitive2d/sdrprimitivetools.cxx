/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
