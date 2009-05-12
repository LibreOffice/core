/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: pdfihelper.cxx,v $
 *
 * $Revision: 1.2 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sdext.hxx"

#include "pdfihelper.hxx"

#include <rtl/ustrbuf.hxx>
#include <basegfx/numeric/ftools.hxx>

using namespace pdfi;
using namespace com::sun::star;

rtl::OUString pdfi::getColorString( const rendering::ARGBColor& rCol )
{
    rtl::OUStringBuffer aBuf( 7 );
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

rtl::OUString pdfi::unitMMString( double fMM )
{
    rtl::OUStringBuffer aBuf( 32 );
    aBuf.append( rtl_math_round( fMM, 2, rtl_math_RoundingMode_Floor ) );
    aBuf.appendAscii( "mm" );

    return aBuf.makeStringAndClear();
}

rtl::OUString pdfi::convertPixelToUnitString( double fPix )
{
    rtl::OUStringBuffer aBuf( 32 );
    aBuf.append( rtl_math_round( convPx2mm( fPix ), 2, rtl_math_RoundingMode_Floor ) );
    aBuf.appendAscii( "mm" );

    return aBuf.makeStringAndClear();
}


