/*************************************************************************
 *
 *  $RCSfile: addruno.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2003-10-21 08:50:05 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>

#include <svtools/itemprop.hxx>

#include "docsh.hxx"
#include "unonames.hxx"
#include "unoguard.hxx"
#include "miscuno.hxx"
#include "convuno.hxx"
#include "addruno.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------------

ScAddressConversionObj::ScAddressConversionObj(ScDocShell* pDocSh, sal_Bool bForRange) :
    pDocShell( pDocSh ),
    nRefSheet( 0 ),
    bIsRange( bForRange )
{
    pDocShell->GetDocument()->AddUnoObject(*this);
}

ScAddressConversionObj::~ScAddressConversionObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);
}

void ScAddressConversionObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       // invalid
    }
}

sal_Bool ScAddressConversionObj::ParseUIString( const String& rUIString )
{
    if (!pDocShell)
        return sal_False;

    ScDocument* pDoc = pDocShell->GetDocument();
    sal_Bool bSuccess = sal_False;
    if ( bIsRange )
    {
        USHORT nResult = aRange.ParseAny( rUIString, pDoc );
        if ( nResult & SCA_VALID )
        {
            if ( ( nResult & SCA_TAB_3D ) == 0 )
                aRange.aStart.SetTab( (USHORT) nRefSheet );
            if ( ( nResult & SCA_TAB2_3D ) == 0 )
                aRange.aEnd.SetTab( aRange.aStart.Tab() );
            // different sheets are not supported in CellRangeAddress
            if ( aRange.aStart.Tab() == aRange.aEnd.Tab() )
                bSuccess = sal_True;
        }
    }
    else
    {
        USHORT nResult = aRange.aStart.Parse( rUIString, pDoc );
        if ( nResult & SCA_VALID )
        {
            if ( ( nResult & SCA_TAB_3D ) == 0 )
                aRange.aStart.SetTab( (USHORT) nRefSheet );
            bSuccess = sal_True;
        }
    }
    return bSuccess;
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScAddressConversionObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    if ( bIsRange )
    {
        static SfxItemPropertyMap aPropertyMap[] =
        {
            {MAP_CHAR_LEN(SC_UNONAME_ADDRESS),  0,  &getCppuType((table::CellRangeAddress*)0), 0, 0 },
            {MAP_CHAR_LEN(SC_UNONAME_PERSREPR), 0,  &getCppuType((rtl::OUString*)0),    0, 0 },
            {MAP_CHAR_LEN(SC_UNONAME_REFSHEET), 0,  &getCppuType((sal_Int32*)0),        0, 0 },
            {MAP_CHAR_LEN(SC_UNONAME_UIREPR),   0,  &getCppuType((rtl::OUString*)0),    0, 0 },
            {0,0,0,0}
        };
        static uno::Reference<beans::XPropertySetInfo> aRef = new SfxItemPropertySetInfo( aPropertyMap );
        return aRef;
    }
    else
    {
        static SfxItemPropertyMap aPropertyMap[] =
        {
            {MAP_CHAR_LEN(SC_UNONAME_ADDRESS),  0,  &getCppuType((table::CellAddress*)0), 0, 0 },
            {MAP_CHAR_LEN(SC_UNONAME_PERSREPR), 0,  &getCppuType((rtl::OUString*)0),    0, 0 },
            {MAP_CHAR_LEN(SC_UNONAME_REFSHEET), 0,  &getCppuType((sal_Int32*)0),        0, 0 },
            {MAP_CHAR_LEN(SC_UNONAME_UIREPR),   0,  &getCppuType((rtl::OUString*)0),    0, 0 },
            {0,0,0,0}
        };
        static uno::Reference<beans::XPropertySetInfo> aRef = new SfxItemPropertySetInfo( aPropertyMap );
        return aRef;
    }
}

void SAL_CALL ScAddressConversionObj::setPropertyValue( const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    if ( !pDocShell )
        throw uno::RuntimeException();

    sal_Bool bSuccess = sal_False;
    String aNameStr = aPropertyName;
    if ( aNameStr.EqualsAscii( SC_UNONAME_ADDRESS ) )
    {
        //  read the cell/range address from API struct
        if ( bIsRange )
        {
            table::CellRangeAddress aRangeAddress;
            if ( aValue >>= aRangeAddress )
            {
                ScUnoConversion::FillScRange( aRange, aRangeAddress );
                bSuccess = sal_True;
            }
        }
        else
        {
            table::CellAddress aCellAddress;
            if ( aValue >>= aCellAddress )
            {
                ScUnoConversion::FillScAddress( aRange.aStart, aCellAddress );
                bSuccess = sal_True;
            }
        }
    }
    else if ( aNameStr.EqualsAscii( SC_UNONAME_REFSHEET ) )
    {
        //  set the reference sheet
        sal_Int32 nIntVal;
        if ( aValue >>= nIntVal )
        {
            nRefSheet = nIntVal;
            bSuccess = sal_True;
        }
    }
    else if ( aNameStr.EqualsAscii( SC_UNONAME_UIREPR ) )
    {
        //  parse the UI representation string
        rtl::OUString sRepresentation;
        if (aValue >>= sRepresentation)
        {
            String aUIString = sRepresentation;
            bSuccess = ParseUIString( aUIString );
        }
    }
    else if ( aNameStr.EqualsAscii( SC_UNONAME_PERSREPR ) )
    {
        //  parse the file format string
        rtl::OUString sRepresentation;
        if (aValue >>= sRepresentation)
        {
            String aUIString = sRepresentation;

            //  cell or range: strip a single "." at the start
            if ( aUIString.GetChar(0) == (sal_Unicode) '.' )
                aUIString.Erase( 0, 1 );

            if ( bIsRange )
            {
                //  range: also strip a "." after the last colon
                sal_Int32 nColon = rtl::OUString(aUIString).lastIndexOf( (sal_Unicode) ':' );
                if ( nColon >= 0 && nColon < aUIString.Len() - 1 &&
                     aUIString.GetChar((xub_StrLen)nColon+1) == (sal_Unicode) '.' )
                    aUIString.Erase( (xub_StrLen)nColon+1, 1 );
            }

            //  parse the rest like a UI string
            bSuccess = ParseUIString( aUIString );
        }
    }
    else
        throw beans::UnknownPropertyException();

    if ( !bSuccess )
        throw lang::IllegalArgumentException();
}

uno::Any SAL_CALL ScAddressConversionObj::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    if ( !pDocShell )
        throw uno::RuntimeException();

    ScDocument* pDoc = pDocShell->GetDocument();
    uno::Any aRet;

    String aNameStr = aPropertyName;
    if ( aNameStr.EqualsAscii( SC_UNONAME_ADDRESS ) )
    {
        if ( bIsRange )
        {
            table::CellRangeAddress aRangeAddress;
            ScUnoConversion::FillApiRange( aRangeAddress, aRange );
            aRet <<= aRangeAddress;
        }
        else
        {
            table::CellAddress aCellAddress;
            ScUnoConversion::FillApiAddress( aCellAddress, aRange.aStart );
            aRet <<= aCellAddress;
        }
    }
    else if ( aNameStr.EqualsAscii( SC_UNONAME_REFSHEET ) )
    {
        aRet <<= nRefSheet;
    }
    else if ( aNameStr.EqualsAscii( SC_UNONAME_UIREPR ) )
    {
        //  generate UI representation string - include sheet only if different from ref sheet
        String aFormatStr;
        USHORT nFlags = SCA_VALID;
        if ( aRange.aStart.Tab() != nRefSheet )
            nFlags |= SCA_TAB_3D;
        if ( bIsRange )
            aRange.Format( aFormatStr, nFlags, pDoc );
        else
            aRange.aStart.Format( aFormatStr, nFlags, pDoc );
        aRet <<= rtl::OUString( aFormatStr );
    }
    else if ( aNameStr.EqualsAscii( SC_UNONAME_PERSREPR ) )
    {
        //  generate file format string - always include sheet
        String aFormatStr;
        aRange.aStart.Format( aFormatStr, SCA_VALID | SCA_TAB_3D, pDoc );
        if ( bIsRange )
        {
            //  manually concatenate range so both parts always have the sheet name
            aFormatStr.Append( (sal_Unicode) ':' );
            String aSecond;
            aRange.aEnd.Format( aSecond, SCA_VALID | SCA_TAB_3D, pDoc );
            aFormatStr.Append( aSecond );
        }
        aRet <<= rtl::OUString( aFormatStr );
    }
    else
        throw beans::UnknownPropertyException();

    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScAddressConversionObj )

// lang::XServiceInfo

rtl::OUString SAL_CALL ScAddressConversionObj::getImplementationName() throw(uno::RuntimeException)
{
    return rtl::OUString::createFromAscii( "ScAddressConversionObj" );
}

sal_Bool SAL_CALL ScAddressConversionObj::supportsService( const rtl::OUString& rServiceName )
                                                    throw(uno::RuntimeException)
{
    String aServiceStr( rServiceName );
    return aServiceStr.EqualsAscii( bIsRange ? SC_SERVICENAME_RANGEADDRESS
                                             : SC_SERVICENAME_CELLADDRESS );
}

uno::Sequence<rtl::OUString> SAL_CALL ScAddressConversionObj::getSupportedServiceNames()
                                                    throw(uno::RuntimeException)
{
    uno::Sequence<rtl::OUString> aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] = rtl::OUString::createFromAscii( bIsRange ? SC_SERVICENAME_RANGEADDRESS
                                                         : SC_SERVICENAME_CELLADDRESS );
    return aRet;
}

