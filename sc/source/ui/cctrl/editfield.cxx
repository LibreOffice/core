/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: editfield.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 13:16:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#ifdef SC_DLLIMPLEMENTATION
#undef SC_DLLIMPLEMENTATION
#endif

#ifndef SC_EDITFIELD_HXX
#include "editfield.hxx"
#endif

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

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

ScDoubleField::ScDoubleField( Window* pParent, WinBits nStyle ) :
    Edit( pParent, nStyle )
{
}

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

