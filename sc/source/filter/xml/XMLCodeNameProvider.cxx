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

#include "XMLCodeNameProvider.hxx"
#include <document.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <comphelper/sequence.hxx>

using namespace com::sun::star;

bool XMLCodeNameProvider::_getCodeName( const uno::Any& aAny, OUString& rCodeName )
{
    uno::Sequence<beans::PropertyValue> aProps;
    if( !(aAny >>= aProps) )
        return false;

    for( const auto& rProp : std::as_const(aProps) )
    {
        if( rProp.Name == "CodeName" )
        {
            OUString sCodeName;
            if( rProp.Value >>= sCodeName )
            {
                rCodeName = sCodeName;
                return true;
            }
        }
    }

    return false;
}

constexpr OUStringLiteral gsDocName( u"*doc*" );
constexpr OUStringLiteral gsCodeNameProp( u"CodeName" );

XMLCodeNameProvider::XMLCodeNameProvider( ScDocument* pDoc ) :
    mpDoc( pDoc )
{
}

XMLCodeNameProvider::~XMLCodeNameProvider()
{
}

sal_Bool SAL_CALL XMLCodeNameProvider::hasByName( const OUString& aName )
{
    if( aName == gsDocName )
        return !mpDoc->GetCodeName().isEmpty();

    SCTAB nCount = mpDoc->GetTableCount();
    OUString sSheetName, sCodeName;
    for( SCTAB i = 0; i < nCount; i++ )
    {
        if( mpDoc->GetName( i, sSheetName ) && sSheetName == aName )
        {
            mpDoc->GetCodeName( i, sCodeName );
            return !sCodeName.isEmpty();
        }
    }

    return false;
}

uno::Any SAL_CALL XMLCodeNameProvider::getByName( const OUString& aName )
{
    uno::Any aRet;
    uno::Sequence<beans::PropertyValue> aProps(1);
    aProps[0].Name = gsCodeNameProp;
    if( aName == gsDocName )
    {
        OUString sUCodeName( mpDoc->GetCodeName() );
        aProps[0].Value <<= sUCodeName;
        aRet <<= aProps;
        return aRet;
    }

    SCTAB nCount = mpDoc->GetTableCount();
    OUString sSheetName, sCodeName;
    for( SCTAB i = 0; i < nCount; i++ )
    {
        if( mpDoc->GetName( i, sSheetName ) && sSheetName == aName )
        {
            mpDoc->GetCodeName( i, sCodeName );
            aProps[0].Value <<= sCodeName;
            aRet <<= aProps;
            return aRet;
        }
    }

    return aRet;
}

uno::Sequence< OUString > SAL_CALL XMLCodeNameProvider::getElementNames(  )
{
    SCTAB nCount = mpDoc->GetTableCount() + 1;
    std::vector< OUString > aNames;
    aNames.reserve(nCount);

    if( !mpDoc->GetCodeName().isEmpty() )
        aNames.push_back(gsDocName);

    OUString sSheetName, sCodeName;
    for( SCTAB i = 0; i < nCount; i++ )
    {
        mpDoc->GetCodeName( i, sCodeName );
        if (!sCodeName.isEmpty())
        {
            if( mpDoc->GetName( i, sSheetName ) )
                aNames.push_back(sSheetName);
        }
    }

    return comphelper::containerToSequence(aNames);
}

uno::Type SAL_CALL XMLCodeNameProvider::getElementType(  )
{
    return cppu::UnoType<uno::Sequence<beans::PropertyValue>>::get();
}

sal_Bool SAL_CALL XMLCodeNameProvider::hasElements()
{
    if( !mpDoc->GetCodeName().isEmpty() )
        return true;

    SCTAB nCount = mpDoc->GetTableCount();
    OUString sSheetName, sCodeName;
    for( SCTAB i = 0; i < nCount; i++ )
    {
        mpDoc->GetCodeName( i, sCodeName );
        if (!sCodeName.isEmpty() && mpDoc->GetName(i, sSheetName))
            return true;
    }

    return false;
}

void XMLCodeNameProvider::set( const uno::Reference< container::XNameAccess>& xNameAccess, ScDocument *pDoc )
{
    uno::Any aAny;
    OUString sDocName("*doc*");
    OUString sCodeName;
    if( xNameAccess->hasByName( sDocName ) )
    {
        aAny = xNameAccess->getByName( sDocName );
        if( _getCodeName( aAny, sCodeName ) )
            pDoc->SetCodeName( sCodeName );
    }

    SCTAB nCount = pDoc->GetTableCount();
    OUString sSheetName;
    for( SCTAB i = 0; i < nCount; i++ )
    {
        if( pDoc->GetName( i, sSheetName ) &&
            xNameAccess->hasByName( sSheetName ) )
        {
            aAny = xNameAccess->getByName( sSheetName );
            if( _getCodeName( aAny, sCodeName ) )
                pDoc->SetCodeName( i, sCodeName );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
