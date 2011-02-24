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

// INCLUDE ---------------------------------------------------------------

#include "XMLCodeNameProvider.hxx"
#include "document.hxx"

using namespace com::sun::star;

using ::rtl::OUString;

sal_Bool XMLCodeNameProvider::_getCodeName( const uno::Any& aAny, String& rCodeName )
{
    uno::Sequence<beans::PropertyValue> aProps;
    if( !(aAny >>= aProps) )
        return sal_False;

    OUString sCodeNameProp( RTL_CONSTASCII_USTRINGPARAM("CodeName") );
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

    return sal_False;
}


XMLCodeNameProvider::XMLCodeNameProvider( ScDocument* pDoc ) :
    mpDoc( pDoc ),
    msDocName( RTL_CONSTASCII_USTRINGPARAM("*doc*") ),
    msCodeNameProp( RTL_CONSTASCII_USTRINGPARAM("CodeName") )
{
}

XMLCodeNameProvider::~XMLCodeNameProvider()
{
}

::sal_Bool SAL_CALL XMLCodeNameProvider::hasByName( const OUString& aName )
    throw (uno::RuntimeException )
{
    if( aName == msDocName )
        return mpDoc->GetCodeName().Len() > 0;

    SCTAB nCount = mpDoc->GetTableCount();
    String sName( aName );
    String sSheetName, sCodeName;
    for( SCTAB i = 0; i < nCount; i++ )
    {
        if( mpDoc->GetName( i, sSheetName ) && sSheetName == sName )
        {
            mpDoc->GetCodeName( i, sCodeName );
            return sCodeName.Len() > 0;
        }
    }

    return sal_False;
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
    String sName( aName );
    String sSheetName, sCodeName;
    for( SCTAB i = 0; i < nCount; i++ )
    {
        if( mpDoc->GetName( i, sSheetName ) && sSheetName == sName )
        {
            mpDoc->GetCodeName( i, sCodeName );
            OUString sUCodeName( sCodeName );
            aProps[0].Value <<= sUCodeName;
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

    if( mpDoc->GetCodeName().Len() )
        aNames[nRealCount++] = msDocName;

    String sSheetName, sCodeName;
    for( SCTAB i = 0; i < nCount; i++ )
    {
        mpDoc->GetCodeName( i, sCodeName );
        if( sCodeName.Len() > 0 )
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
    if( mpDoc->GetCodeName().Len() > 0 )
        return sal_True;

    SCTAB nCount = mpDoc->GetTableCount();
    String sSheetName, sCodeName;
    for( SCTAB i = 0; i < nCount; i++ )
    {
        mpDoc->GetCodeName( i, sCodeName );
        if( sCodeName.Len() > 0 && mpDoc->GetName( i, sSheetName ) )
            return sal_True;
    }

    return sal_False;
}

void XMLCodeNameProvider::set( const uno::Reference< container::XNameAccess>& xNameAccess, ScDocument *pDoc )
{
    uno::Any aAny;
    OUString sDocName( RTL_CONSTASCII_USTRINGPARAM("*doc*") );
    String sCodeName;
    if( xNameAccess->hasByName( sDocName ) )
    {
        aAny = xNameAccess->getByName( sDocName );
        if( _getCodeName( aAny, sCodeName ) )
            pDoc->SetCodeName( sCodeName );
    }

    SCTAB nCount = pDoc->GetTableCount();
    String sSheetName;
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
