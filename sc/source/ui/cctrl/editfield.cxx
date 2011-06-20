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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#ifdef SC_DLLIMPLEMENTATION
#undef SC_DLLIMPLEMENTATION
#endif
#include "editfield.hxx"
#include <rtl/math.hxx>
#include <unotools/localedatawrapper.hxx>
#include "global.hxx"

// ============================================================================

namespace {

sal_Unicode lclGetDecSep()
{
    return ScGlobal::GetpLocaleData()->getNumDecimalSep().GetChar( 0 );
}

sal_Unicode lclGetGroupSep()
{
    return ScGlobal::GetpLocaleData()->getNumThousandSep().GetChar( 0 );
}

} // namespace

// ============================================================================

ScDoubleField::ScDoubleField( Window* pParent, const ResId& rResId ) :
    Edit( pParent, rResId )
{
}

bool ScDoubleField::GetValue( double& rfValue ) const
{
    String aStr( GetText() );
    aStr.EraseLeadingAndTrailingChars( ' ' );
    bool bOk = aStr.Len() > 0;
    if( bOk )
    {
        rtl_math_ConversionStatus eStatus;
        sal_Int32 nEnd;
        rfValue = rtl::math::stringToDouble( aStr, lclGetDecSep(), lclGetGroupSep(), &eStatus, &nEnd );
        bOk = (eStatus == rtl_math_ConversionStatus_Ok) && (nEnd == static_cast< sal_Int32 >( aStr.Len() ));
    }
    return bOk;
}

void ScDoubleField::SetValue( double fValue, sal_Int32 nDecPlaces, bool bEraseTrailingDecZeros )
{
    SetText( ::rtl::math::doubleToUString( fValue, rtl_math_StringFormat_G,
        nDecPlaces, lclGetDecSep(), bEraseTrailingDecZeros ) );
}

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
