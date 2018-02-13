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
#include <vcl/virdev.hxx>


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
                VclPtr<VirtualDevice> pContent(VclPtr<VirtualDevice>::Create());
                pContent->SetOutputSizePixel(Size(3, 3));

                // Y,X unusual order (!)
                pContent->SetLineColor(Color(aColor));
                pContent->DrawPixel(Point(1,0));
                pContent->DrawPixel(Point(0,1));
                pContent->DrawPixel(Point(1,1));
                pContent->DrawPixel(Point(2,1));
                pContent->DrawPixel(Point(1,2));

// FIXME how do I draw a mask via VirtualDevice?
                pContent->SetLineColor(Color(0x01));
                pContent->DrawPixel(Point(0, 0));
                pContent->DrawPixel(Point(2, 0));
                pContent->DrawPixel(Point(0, 2));
                pContent->DrawPixel(Point(2, 2));

                // create and exchange at aRetVal
                delete aRetVal.set(new BitmapEx(pContent->GetBitmapEx(Point(0,0), pContent->GetOutputSizePixel())));
            }

            return aRetVal.get() ? *aRetVal.get() : BitmapEx();
        }
    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
