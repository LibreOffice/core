/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>

using namespace com::sun::star;
using namespace ::cpo;

bool XMLCodeNameProvider::_getCodeName( const cpo::uno::Any& aAny, OUString& rCodeName )
{
    cpo::uno::Sequence<beans::PropertyValue> aProps;
    if( !(aAny >>= aProps) )
        return false;

    for (const auto& rProp : aProps)
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

constexpr OUString gsDocName( u"*doc*"_ustr );
constexpr OUString gsCodeNameProp( u"CodeName"_ustr );

XMLCodeNameProvider::XMLCodeNameProvider( ScDocument* pDoc ) :
    mpDoc( pDoc )
{
}

XMLCodeNameProvider::~XMLCodeNameProvider()
{
}

namespace
{
/// The code name a document gives the sheet, or nothing where it gives none.
OUString lcl_GetStatedCodeName(const ScDocument* pDoc, SCTAB nTab)
{
    OUString aCodeName;
    if (pDoc->IsCodeNameStated(nTab))
        pDoc->GetCodeName(nTab, aCodeName);
    return aCodeName;
}
}

bool XMLCodeNameProvider::hasByName( const OUString& aName )
{
    if( aName == gsDocName )
        return !mpDoc->GetCodeName().isEmpty();

    SCTAB nCount = mpDoc->GetTableCount();
    OUString sSheetName;
    for( SCTAB i = 0; i < nCount; i++ )
    {
        if( mpDoc->GetName( i, sSheetName ) && sSheetName == aName )
            return !lcl_GetStatedCodeName( mpDoc, i ).isEmpty();
    }

    return false;
}

cpo::uno::Any XMLCodeNameProvider::getByName( const OUString& aName )
{
    cpo::uno::Any aRet;
    if( aName == gsDocName )
    {
        OUString sUCodeName( mpDoc->GetCodeName() );
        aRet <<= cpo::uno::Sequence{ comphelper::makePropertyValue(gsCodeNameProp, sUCodeName) };
        return aRet;
    }

    SCTAB nCount = mpDoc->GetTableCount();
    OUString sSheetName;
    for( SCTAB i = 0; i < nCount; i++ )
    {
        if( mpDoc->GetName( i, sSheetName ) && sSheetName == aName )
        {
            aRet <<= cpo::uno::Sequence{ comphelper::makePropertyValue(
                gsCodeNameProp, lcl_GetStatedCodeName( mpDoc, i )) };
            return aRet;
        }
    }

    return aRet;
}

cpo::uno::Sequence< OUString > XMLCodeNameProvider::getElementNames(  )
{
    SCTAB nCount = mpDoc->GetTableCount() + 1;
    std::vector< OUString > aNames;
    aNames.reserve(nCount);

    if( !mpDoc->GetCodeName().isEmpty() )
        aNames.push_back(gsDocName);

    OUString sSheetName;
    for( SCTAB i = 0; i < nCount; i++ )
    {
        if (!lcl_GetStatedCodeName( mpDoc, i ).isEmpty() && mpDoc->GetName( i, sSheetName ))
            aNames.push_back(sSheetName);
    }

    return comphelper::containerToSequence(aNames);
}

cpo::uno::Type XMLCodeNameProvider::getElementType(  )
{
    return cppu::UnoType<cpo::uno::Sequence<beans::PropertyValue>>::get();
}

bool XMLCodeNameProvider::hasElements()
{
    if( !mpDoc->GetCodeName().isEmpty() )
        return true;

    SCTAB nCount = mpDoc->GetTableCount();
    OUString sSheetName;
    for( SCTAB i = 0; i < nCount; i++ )
    {
        if (!lcl_GetStatedCodeName( mpDoc, i ).isEmpty() && mpDoc->GetName(i, sSheetName))
            return true;
    }

    return false;
}

void XMLCodeNameProvider::set( const uno::Reference< container::XNameAccess>& xNameAccess, ScDocument *pDoc )
{
    cpo::uno::Any aAny;
    OUString sDocName(u"*doc*"_ustr);
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
