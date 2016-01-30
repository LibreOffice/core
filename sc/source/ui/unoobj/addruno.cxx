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
#include <cppuhelper/supportsservice.hxx>

#include <svl/itemprop.hxx>
#include <vcl/svapp.hxx>

#include "docsh.hxx"
#include "unonames.hxx"
#include "miscuno.hxx"
#include "convuno.hxx"
#include "addruno.hxx"

using namespace com::sun::star;

ScAddressConversionObj::ScAddressConversionObj(ScDocShell* pDocSh, bool _bIsRange) :
    pDocShell( pDocSh ),
    nRefSheet( 0 ),
    bIsRange( _bIsRange )
{
    pDocShell->GetDocument().AddUnoObject(*this);
}

ScAddressConversionObj::~ScAddressConversionObj()
{
    SolarMutexGuard g;

    if (pDocShell)
        pDocShell->GetDocument().RemoveUnoObject(*this);
}

void ScAddressConversionObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>( &rHint );
    if ( pSimpleHint && pSimpleHint->GetId() == SFX_HINT_DYING )
    {
        pDocShell = nullptr;       // invalid
    }
}

bool ScAddressConversionObj::ParseUIString( const OUString& rUIString, ::formula::FormulaGrammar::AddressConvention eConv )
{
    if (!pDocShell)
        return false;

    ScDocument& rDoc = pDocShell->GetDocument();
    bool bSuccess = false;
    if ( bIsRange )
    {
        sal_uInt16 nResult = aRange.ParseAny( rUIString, &rDoc, eConv );
        if ( nResult & SCA_VALID )
        {
            if ( ( nResult & SCA_TAB_3D ) == 0 )
                aRange.aStart.SetTab( static_cast<SCTAB>(nRefSheet) );
            if ( ( nResult & SCA_TAB2_3D ) == 0 )
                aRange.aEnd.SetTab( aRange.aStart.Tab() );
            // different sheets are not supported in CellRangeAddress
            if ( aRange.aStart.Tab() == aRange.aEnd.Tab() )
                bSuccess = true;
        }
    }
    else
    {
        sal_uInt16 nResult = aRange.aStart.Parse( rUIString, &rDoc, eConv );
        if ( nResult & SCA_VALID )
        {
            if ( ( nResult & SCA_TAB_3D ) == 0 )
                aRange.aStart.SetTab( static_cast<SCTAB>(nRefSheet) );
            bSuccess = true;
        }
    }
    return bSuccess;
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScAddressConversionObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if ( bIsRange )
    {
        static const SfxItemPropertyMapEntry aPropertyMap[] =
        {
            { OUString(SC_UNONAME_ADDRESS),  0,  cppu::UnoType<table::CellRangeAddress>::get(), 0, 0 },
            { OUString(SC_UNONAME_PERSREPR), 0,  cppu::UnoType<OUString>::get(),    0, 0 },
            { OUString(SC_UNONAME_XLA1REPR), 0,  cppu::UnoType<OUString>::get(),    0, 0 },
            { OUString(SC_UNONAME_REFSHEET), 0,  cppu::UnoType<sal_Int32>::get(),        0, 0 },
            { OUString(SC_UNONAME_UIREPR),   0,  cppu::UnoType<OUString>::get(),    0, 0 },
            { OUString(SC_UNONAME_XLA1REPR), 0,  cppu::UnoType<OUString>::get(),    0, 0 },
            { OUString(), 0, css::uno::Type(), 0, 0 }
        };
        static uno::Reference<beans::XPropertySetInfo> aRef(new SfxItemPropertySetInfo( aPropertyMap ));
        return aRef;
    }
    else
    {
        static const SfxItemPropertyMapEntry aPropertyMap[] =
        {
            { OUString(SC_UNONAME_ADDRESS),  0,  cppu::UnoType<table::CellAddress>::get(), 0, 0 },
            { OUString(SC_UNONAME_PERSREPR), 0,  cppu::UnoType<OUString>::get(),    0, 0 },
            { OUString(SC_UNONAME_XLA1REPR), 0,  cppu::UnoType<OUString>::get(),    0, 0 },
            { OUString(SC_UNONAME_REFSHEET), 0,  cppu::UnoType<sal_Int32>::get(),        0, 0 },
            { OUString(SC_UNONAME_UIREPR),   0,  cppu::UnoType<OUString>::get(),    0, 0 },
            { OUString(SC_UNONAME_XLA1REPR), 0,  cppu::UnoType<OUString>::get(),    0, 0 },
            { OUString(), 0, css::uno::Type(), 0, 0 }
        };
        static uno::Reference<beans::XPropertySetInfo> aRef(new SfxItemPropertySetInfo( aPropertyMap ));
        return aRef;
    }
}

void SAL_CALL ScAddressConversionObj::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
{
    if ( !pDocShell )
        throw uno::RuntimeException();

    bool bSuccess = false;
    OUString aNameStr(aPropertyName);
    if ( aNameStr == SC_UNONAME_ADDRESS )
    {
        //  read the cell/range address from API struct
        if ( bIsRange )
        {
            table::CellRangeAddress aRangeAddress;
            if ( aValue >>= aRangeAddress )
            {
                ScUnoConversion::FillScRange( aRange, aRangeAddress );
                bSuccess = true;
            }
        }
        else
        {
            table::CellAddress aCellAddress;
            if ( aValue >>= aCellAddress )
            {
                ScUnoConversion::FillScAddress( aRange.aStart, aCellAddress );
                bSuccess = true;
            }
        }
    }
    else if ( aNameStr == SC_UNONAME_REFSHEET )
    {
        //  set the reference sheet
        sal_Int32 nIntVal = 0;
        if ( aValue >>= nIntVal )
        {
            nRefSheet = nIntVal;
            bSuccess = true;
        }
    }
    else if ( aNameStr == SC_UNONAME_UIREPR )
    {
        //  parse the UI representation string
        OUString sRepresentation;
        if (aValue >>= sRepresentation)
        {
            OUString aUIString = sRepresentation;
            bSuccess = ParseUIString( aUIString );
        }
    }
    else if ( aNameStr == SC_UNONAME_PERSREPR || aNameStr == SC_UNONAME_XLA1REPR )
    {
        ::formula::FormulaGrammar::AddressConvention eConv = aNameStr == SC_UNONAME_XLA1REPR ?
            ::formula::FormulaGrammar::CONV_XL_A1 : ::formula::FormulaGrammar::CONV_OOO;

        //  parse the file format string
        OUString sRepresentation;
        if (aValue >>= sRepresentation)
        {
            OUString aUIString(sRepresentation);

            //  cell or range: strip a single "." at the start
            if ( aUIString[0]== (sal_Unicode) '.' )
                aUIString = aUIString.copy( 1 );

            if ( bIsRange )
            {
                //  range: also strip a "." after the last colon
                sal_Int32 nColon = aUIString.lastIndexOf( (sal_Unicode) ':' );
                if ( nColon >= 0 && nColon < aUIString.getLength() - 1 &&
                     aUIString[nColon+1] == '.' )
                    aUIString = aUIString.replaceAt( nColon+1, 1, "" );
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
                        uno::RuntimeException, std::exception)
{
    if ( !pDocShell )
        throw uno::RuntimeException();

    ScDocument& rDoc = pDocShell->GetDocument();
    uno::Any aRet;

    OUString aNameStr(aPropertyName);
    if ( aNameStr == SC_UNONAME_ADDRESS )
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
    else if ( aNameStr == SC_UNONAME_REFSHEET )
    {
        aRet <<= nRefSheet;
    }
    else if ( aNameStr == SC_UNONAME_UIREPR )
    {
        //  generate UI representation string - include sheet only if different from ref sheet
        OUString aFormatStr;
        sal_uInt16 nFlags = SCA_VALID;
        if ( aRange.aStart.Tab() != nRefSheet )
            nFlags |= SCA_TAB_3D;
        if ( bIsRange )
            aFormatStr = aRange.Format(nFlags, &rDoc);
        else
            aFormatStr = aRange.aStart.Format(nFlags, &rDoc);
        aRet <<= aFormatStr;
    }
    else if ( aNameStr == SC_UNONAME_PERSREPR || aNameStr == SC_UNONAME_XLA1REPR )
    {
        ::formula::FormulaGrammar::AddressConvention eConv = aNameStr == SC_UNONAME_XLA1REPR ?
            ::formula::FormulaGrammar::CONV_XL_A1 : ::formula::FormulaGrammar::CONV_OOO;

        //  generate file format string - always include sheet
        OUString aFormatStr(aRange.aStart.Format(SCA_VALID | SCA_TAB_3D, &rDoc, eConv));
        if ( bIsRange )
        {
            //  manually concatenate range so both parts always have the sheet name
            aFormatStr += ":";
            sal_uInt16 nFlags = SCA_VALID;
            if( eConv != ::formula::FormulaGrammar::CONV_XL_A1 )
                nFlags |= SCA_TAB_3D;
            OUString aSecond(aRange.aEnd.Format(nFlags, &rDoc, eConv));
            aFormatStr += aSecond ;
        }
        aRet <<= OUString( aFormatStr );
    }
    else
        throw beans::UnknownPropertyException();

    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScAddressConversionObj )

// lang::XServiceInfo

OUString SAL_CALL ScAddressConversionObj::getImplementationName() throw(uno::RuntimeException, std::exception)
{
    return OUString("ScAddressConversionObj" );
}

sal_Bool SAL_CALL ScAddressConversionObj::supportsService( const OUString& rServiceName )
                                                    throw(uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL ScAddressConversionObj::getSupportedServiceNames()
                                                    throw(uno::RuntimeException, std::exception)
{
    uno::Sequence<OUString> aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = bIsRange ? OUString(SC_SERVICENAME_RANGEADDRESS)
                         : OUString(SC_SERVICENAME_CELLADDRESS);
    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
