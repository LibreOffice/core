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
#include "document.hxx"

using namespace com::sun::star;


sal_Bool XMLCodeNameProvider::_getCodeName( const uno::Any& aAny, OUString& rCodeName )
{
    uno::Sequence<beans::PropertyValue> aProps;
    if( !(aAny >>= aProps) )
        return false;

    OUString sCodeNameProp("CodeName");
    sal_Int32 nPropCount = aProps.getLength();
    for( sal_Int32 i=0; i<nPropCount; i++ )
    {
        if( aProps[i].Name == sCodeNameProp )
        {
            OUString sCodeName;
            if( aProps[i].Value >>= sCodeName )
            {
                rCodeName = sCodeName;
                return sal_True;
            }
        }
    }

    return false;
}


XMLCodeNameProvider::XMLCodeNameProvider( ScDocument* pDoc ) :
    mpDoc( pDoc ),
    msDocName( "*doc*" ),
    msCodeNameProp( "CodeName" )
{
}

XMLCodeNameProvider::~XMLCodeNameProvider()
{
}

::sal_Bool SAL_CALL XMLCodeNameProvider::hasByName( const OUString& aName )
    throw (uno::RuntimeException )
{
    if( aName == msDocName )
        return !mpDoc->GetCodeName().isEmpty();

    SCTAB nCount = mpDoc->GetTableCount();
    OUString sSheetName, sCodeName;
    for( SCTAB i = 0; i < nCount; i++ )
    {
        if( mpDoc->GetName( i, sSheetName ) && sSheetName.equals(aName) )
        {
            mpDoc->GetCodeName( i, sCodeName );
            return !sCodeName.isEmpty();
        }
    }

    return false;
}

uno::Any SAL_CALL XMLCodeNameProvider::getByName( const OUString& aName )
    throw (container::NoSuchElementException,
           lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Any aRet;
    uno::Sequence<beans::PropertyValue> aProps(1);
    aProps[0].Name = msCodeNameProp;
    if( aName == msDocName )
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
        if( mpDoc->GetName( i, sSheetName ) && sSheetName.equals(aName) )
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
    throw (uno::RuntimeException)
{
    SCTAB nCount = mpDoc->GetTableCount() + 1;
    uno::Sequence< OUString > aNames( nCount );
    sal_Int32 nRealCount = 0;

    if( !mpDoc->GetCodeName().isEmpty() )
        aNames[nRealCount++] = msDocName;

    OUString sSheetName, sCodeName;
    for( SCTAB i = 0; i < nCount; i++ )
    {
        mpDoc->GetCodeName( i, sCodeName );
        if (!sCodeName.isEmpty())
        {
            if( mpDoc->GetName( i, sSheetName ) )
                aNames[nRealCount++] = sSheetName;
        }
    }

    if( nCount != nRealCount )
        aNames.realloc( nRealCount );

    return aNames;
}

uno::Type SAL_CALL XMLCodeNameProvider::getElementType(  )
    throw (uno::RuntimeException)
{
    return getCppuType(static_cast<uno::Sequence<beans::PropertyValue>*>(0));
}

::sal_Bool SAL_CALL XMLCodeNameProvider::hasElements()
    throw (uno::RuntimeException )
{
    if( !mpDoc->GetCodeName().isEmpty() )
        return sal_True;

    SCTAB nCount = mpDoc->GetTableCount();
    OUString sSheetName, sCodeName;
    for( SCTAB i = 0; i < nCount; i++ )
    {
        mpDoc->GetCodeName( i, sCodeName );
        if (!sCodeName.isEmpty() && mpDoc->GetName(i, sSheetName))
            return sal_True;
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
