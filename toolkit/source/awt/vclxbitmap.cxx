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

#include <awt/vclxbitmap.hxx>
#include <tools/stream.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/BitmapTools.hxx>




// css::awt::XBitmap
css::awt::Size VCLXBitmap::getSize()
{
    std::scoped_lock aGuard( GetMutex() );

    css::awt::Size aSize( maBitmap.GetSizePixel().Width(), maBitmap.GetSizePixel().Height() );
    return aSize;
}

css::uno::Sequence< sal_Int8 > VCLXBitmap::getDIB()
{
    std::scoped_lock aGuard( GetMutex() );

    SvMemoryStream aMem;
    WriteDIB(BitmapEx(maBitmap).GetBitmap(), aMem, false, true);
    return css::uno::Sequence<sal_Int8>( static_cast<sal_Int8 const *>(aMem.GetData()), aMem.Tell() );
}

css::uno::Sequence< sal_Int8 > VCLXBitmap::getMaskDIB()
{
    std::scoped_lock aGuard( GetMutex() );

    return vcl::bitmap::GetMaskDIB(BitmapEx(maBitmap));
}

sal_Int64 SAL_CALL VCLXBitmap::estimateUsage()
{
    std::scoped_lock aGuard( GetMutex() );

    return maBitmap.GetSizeBytes();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
