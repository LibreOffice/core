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


#include "pdfihelper.hxx"

#include <rtl/ustrbuf.hxx>
#include <basegfx/numeric/ftools.hxx>

using namespace pdfi;
using namespace com::sun::star;

OUString pdfi::getColorString( const rendering::ARGBColor& rCol )
{
    OUStringBuffer aBuf( 7 );
    const sal_uInt8 nRed  ( sal::static_int_cast<sal_Int8>( basegfx::fround( rCol.Red * 255.0 ) ) );
    const sal_uInt8 nGreen( sal::static_int_cast<sal_Int8>( basegfx::fround( rCol.Green * 255.0 ) ) );
    const sal_uInt8 nBlue ( sal::static_int_cast<sal_Int8>( basegfx::fround( rCol.Blue * 255.0 ) ) );
    aBuf.append( sal_Unicode('#') );
    if( nRed < 10 )
        aBuf.append( sal_Unicode('0') );
    aBuf.append( sal_Int32(nRed), 16 );
    if( nGreen < 10 )
        aBuf.append( sal_Unicode('0') );
    aBuf.append( sal_Int32(nGreen), 16 );
    if( nBlue < 10 )
        aBuf.append( sal_Unicode('0') );
    aBuf.append( sal_Int32(nBlue), 16 );

    // TODO(F3): respect alpha transparency (polygons etc.)
    OSL_ASSERT(rCol.Alpha == 1.0);

    return aBuf.makeStringAndClear();
}

OUString pdfi::unitMMString( double fMM )
{
    OUStringBuffer aBuf( 32 );
    aBuf.append( rtl_math_round( fMM, 2, rtl_math_RoundingMode_Floor ) );
    aBuf.appendAscii( "mm" );

    return aBuf.makeStringAndClear();
}

OUString pdfi::convertPixelToUnitString( double fPix )
{
    OUStringBuffer aBuf( 32 );
    aBuf.append( rtl_math_round( convPx2mm( fPix ), 2, rtl_math_RoundingMode_Floor ) );
    aBuf.appendAscii( "mm" );

    return aBuf.makeStringAndClear();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
