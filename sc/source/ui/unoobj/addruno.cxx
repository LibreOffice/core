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

#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>

#include <svl/itemprop.hxx>
#include <vcl/svapp.hxx>

#include "docsh.hxx"
#include "unonames.hxx"
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

void ScAddressConversionObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       // invalid
    }
}

sal_Bool ScAddressConversionObj::ParseUIString( const String& rUIString, ::formula::FormulaGrammar::AddressConvention eConv )
{
    if (!pDocShell)
        return false;

    ScDocument* pDoc = pDocShell->GetDocument();
    sal_Bool bSuccess = false;
    if ( bIsRange )
    {
        sal_uInt16 nResult = aRange.ParseAny( rUIString, pDoc, eConv );
        if ( nResult & SCA_VALID )
        {
            if ( ( nResult & SCA_TAB_3D ) == 0 )
                aRange.aStart.SetTab( static_cast<SCTAB>(nRefSheet) );
            if ( ( nResult & SCA_TAB2_3D ) == 0 )
                aRange.aEnd.SetTab( aRange.aStart.Tab() );
            // different sheets are not supported in CellRangeAddress
            if ( aRange.aStart.Tab() == aRange.aEnd.Tab() )
                bSuccess = sal_True;
        }
    }
    else
    {
        sal_uInt16 nResult = aRange.aStart.Parse( rUIString, pDoc, eConv );
        if ( nResult & SCA_VALID )
        {
            if ( ( nResult & SCA_TAB_3D ) == 0 )
                aRange.aStart.SetTab( static_cast<SCTAB>(nRefSheet) );
            bSuccess = sal_True;
        }
    }
    return bSuccess;
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScAddressConversionObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if ( bIsRange )
    {
        static const SfxItemPropertyMapEntry aPropertyMap[] =
        {
            {MAP_CHAR_LEN(SC_UNONAME_ADDRESS),  0,  &getCppuType((table::CellRangeAddress*)0), 0, 0 },
            {MAP_CHAR_LEN(SC_UNONAME_PERSREPR), 0,  &getCppuType((OUString*)0),    0, 0 },
            {MAP_CHAR_LEN(SC_UNONAME_XLA1REPR), 0,  &getCppuType((OUString*)0),    0, 0 },
            {MAP_CHAR_LEN(SC_UNONAME_REFSHEET), 0,  &getCppuType((sal_Int32*)0),        0, 0 },
            {MAP_CHAR_LEN(SC_UNONAME_UIREPR),   0,  &getCppuType((OUString*)0),    0, 0 },
            {MAP_CHAR_LEN(SC_UNONAME_XLA1REPR), 0,  &getCppuType((OUString*)0),    0, 0 },
            {0,0,0,0,0,0}
        };
        static uno::Reference<beans::XPropertySetInfo> aRef(new SfxItemPropertySetInfo( aPropertyMap ));
        return aRef;
    }
    else
    {
        static const SfxItemPropertyMapEntry aPropertyMap[] =
        {
            {MAP_CHAR_LEN(SC_UNONAME_ADDRESS),  0,  &getCppuType((table::CellAddress*)0), 0, 0 },
            {MAP_CHAR_LEN(SC_UNONAME_PERSREPR), 0,  &getCppuType((OUString*)0),    0, 0 },
            {MAP_CHAR_LEN(SC_UNONAME_XLA1REPR), 0,  &getCppuType((OUString*)0),    0, 0 },
            {MAP_CHAR_LEN(SC_UNONAME_REFSHEET), 0,  &getCppuType((sal_Int32*)0),        0, 0 },
            {MAP_CHAR_LEN(SC_UNONAME_UIREPR),   0,  &getCppuType((OUString*)0),    0, 0 },
            {MAP_CHAR_LEN(SC_UNONAME_XLA1REPR), 0,  &getCppuType((OUString*)0),    0, 0 },
            {0,0,0,0,0,0}
        };
        static uno::Reference<beans::XPropertySetInfo> aRef(new SfxItemPropertySetInfo( aPropertyMap ));
        return aRef;
    }
}

void SAL_CALL ScAddressConversionObj::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    if ( !pDocShell )
        throw uno::RuntimeException();

    sal_Bool bSuccess = false;
    String aNameStr(aPropertyName);
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
        sal_Int32 nIntVal = 0;
        if ( aValue >>= nIntVal )
        {
            nRefSheet = nIntVal;
            bSuccess = sal_True;
        }
    }
    else if ( aNameStr.EqualsAscii( SC_UNONAME_UIREPR ) )
    {
        //  parse the UI representation string
        OUString sRepresentation;
        if (aValue >>= sRepresentation)
        {
            String aUIString = sRepresentation;
            bSuccess = ParseUIString( aUIString );
        }
    }
    else if ( aNameStr.EqualsAscii( SC_UNONAME_PERSREPR ) || aNameStr.EqualsAscii( SC_UNONAME_XLA1REPR ) )
    {
        ::formula::FormulaGrammar::AddressConvention eConv = aNameStr.EqualsAscii( SC_UNONAME_XLA1REPR ) ?
            ::formula::FormulaGrammar::CONV_XL_A1 : ::formula::FormulaGrammar::CONV_OOO;

        //  parse the file format string
        OUString sRepresentation;
        if (aValue >>= sRepresentation)
        {
            String aUIString(sRepresentation);

            //  cell or range: strip a single "." at the start
            if ( aUIString.GetChar(0) == (sal_Unicode) '.' )
                aUIString.Erase( 0, 1 );

            if ( bIsRange )
            {
                //  range: also strip a "." after the last colon
                sal_Int32 nColon = OUString(aUIString).lastIndexOf( (sal_Unicode) ':' );
                if ( nColon >= 0 && nColon < aUIString.Len() - 1 &&
                     aUIString.GetChar((xub_StrLen)nColon+1) == (sal_Unicode) '.' )
                    aUIString.Erase( (xub_StrLen)nColon+1, 1 );
            }

            //  parse the rest like a UI string
            bSuccess = ParseUIString( aUIString, eConv );
        }
    }
    else
        throw beans::UnknownPropertyException();

    if ( !bSuccess )
        throw lang::IllegalArgumentException();
}

uno::Any SAL_CALL ScAddressConversionObj::getPropertyValue( const OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    if ( !pDocShell )
        throw uno::RuntimeException();

    ScDocument* pDoc = pDocShell->GetDocument();
    uno::Any aRet;

    String aNameStr(aPropertyName);
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
        OUString aFormatStr;
        sal_uInt16 nFlags = SCA_VALID;
        if ( aRange.aStart.Tab() != nRefSheet )
            nFlags |= SCA_TAB_3D;
        if ( bIsRange )
            aFormatStr = aRange.Format(nFlags, pDoc);
        else
            aFormatStr = aRange.aStart.Format(nFlags, pDoc);
        aRet <<= aFormatStr;
    }
    else if ( aNameStr.EqualsAscii( SC_UNONAME_PERSREPR ) || aNameStr.EqualsAscii( SC_UNONAME_XLA1REPR ) )
    {
        ::formula::FormulaGrammar::AddressConvention eConv = aNameStr.EqualsAscii( SC_UNONAME_XLA1REPR ) ?
            ::formula::FormulaGrammar::CONV_XL_A1 : ::formula::FormulaGrammar::CONV_OOO;

        //  generate file format string - always include sheet
        String aFormatStr(aRange.aStart.Format(SCA_VALID | SCA_TAB_3D, pDoc, eConv));
        if ( bIsRange )
        {
            //  manually concatenate range so both parts always have the sheet name
            aFormatStr.Append( (sal_Unicode) ':' );
            sal_uInt16 nFlags = SCA_VALID;
            if( eConv != ::formula::FormulaGrammar::CONV_XL_A1 )
                nFlags |= SCA_TAB_3D;
            OUString aSecond(aRange.aEnd.Format(nFlags, pDoc, eConv));
            aFormatStr.Append( aSecond );
        }
        aRet <<= OUString( aFormatStr );
    }
    else
        throw beans::UnknownPropertyException();

    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScAddressConversionObj )

// lang::XServiceInfo

OUString SAL_CALL ScAddressConversionObj::getImplementationName() throw(uno::RuntimeException)
{
    return OUString("ScAddressConversionObj" );
}

sal_Bool SAL_CALL ScAddressConversionObj::supportsService( const OUString& rServiceName )
                                                    throw(uno::RuntimeException)
{
    String aServiceStr( rServiceName );
    return aServiceStr.EqualsAscii( bIsRange ? SC_SERVICENAME_RANGEADDRESS
                                             : SC_SERVICENAME_CELLADDRESS );
}

uno::Sequence<OUString> SAL_CALL ScAddressConversionObj::getSupportedServiceNames()
                                                    throw(uno::RuntimeException)
{
    uno::Sequence<OUString> aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = bIsRange ? OUString(SC_SERVICENAME_RANGEADDRESS)
                         : OUString(SC_SERVICENAME_CELLADDRESS);
    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
