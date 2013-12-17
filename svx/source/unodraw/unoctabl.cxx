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

#include <unotools/pathoptions.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase2.hxx>
#include <svx/xtable.hxx>

using namespace ::com::sun::star;
using namespace ::cppu;

namespace {

class SvxUnoColorTable : public WeakImplHelper2< container::XNameContainer, lang::XServiceInfo >
{
private:
    XColorListRef pList;

public:
    SvxUnoColorTable() throw();
    virtual ~SvxUnoColorTable() throw();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const  OUString& ServiceName ) throw( uno::RuntimeException);
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames() throw( uno::RuntimeException);

    // XNameContainer
    virtual void SAL_CALL insertByName( const  OUString& aName, const  uno::Any& aElement ) throw( lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException);
    virtual void SAL_CALL removeByName( const  OUString& Name ) throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);

    // XNameReplace
    virtual void SAL_CALL replaceByName( const  OUString& aName, const  uno::Any& aElement ) throw( lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);

    // XNameAccess
    virtual uno::Any SAL_CALL getByName( const  OUString& aName ) throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);

    virtual uno::Sequence<  OUString > SAL_CALL getElementNames() throw( uno::RuntimeException);

    virtual sal_Bool SAL_CALL hasByName( const  OUString& aName ) throw( uno::RuntimeException);

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() throw( uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw( uno::RuntimeException);
};

SvxUnoColorTable::SvxUnoColorTable() throw()
{
    pList = XPropertyList::CreatePropertyList( XCOLOR_LIST, SvtPathOptions().GetPalettePath(), "" )->AsColorList();
}

SvxUnoColorTable::~SvxUnoColorTable() throw()
{
}

sal_Bool SAL_CALL SvxUnoColorTable::supportsService( const  OUString& ServiceName ) throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSNL( getSupportedServiceNames() );
    const OUString * pArray = aSNL.getConstArray();

    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;

    return sal_False;
}

OUString SAL_CALL SvxUnoColorTable::getImplementationName() throw( uno::RuntimeException )
{
    return OUString("com.sun.star.drawing.SvxUnoColorTable");
}

uno::Sequence< OUString > SAL_CALL SvxUnoColorTable::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = "com.sun.star.drawing.ColorTable";
    return aSNS;
}

// XNameContainer
void SAL_CALL SvxUnoColorTable::insertByName( const OUString& aName, const uno::Any& aElement )
    throw( lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException )
{
    if( hasByName( aName ) )
        throw container::ElementExistException();

    sal_Int32 nColor = 0;
    if( !(aElement >>= nColor) )
        throw lang::IllegalArgumentException();

    if( pList.is() )
    {
        XColorEntry* pEntry = new XColorEntry( Color( (ColorData)nColor ), aName  );
        pList->Insert( pEntry, pList->Count() );
    }
}

void SAL_CALL SvxUnoColorTable::removeByName( const OUString& Name )
    throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    long nIndex = pList.is() ? pList->GetIndex( Name ) : -1;
    if( nIndex == -1 )
        throw container::NoSuchElementException();

    pList->Remove( nIndex );
}

// XNameReplace
void SAL_CALL SvxUnoColorTable::replaceByName( const OUString& aName, const uno::Any& aElement )
    throw( lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
    sal_Int32 nColor = 0;
    if( !(aElement >>= nColor) )
        throw lang::IllegalArgumentException();

    long nIndex = pList.is() ? pList->GetIndex( aName ) : -1;
    if( nIndex == -1  )
        throw container::NoSuchElementException();

    XColorEntry* pEntry = new XColorEntry( Color( (ColorData)nColor ), aName );
    delete pList->Replace( nIndex, pEntry );
}

// XNameAccess
uno::Any SAL_CALL SvxUnoColorTable::getByName( const OUString& aName )
    throw( container::NoSuchElementException,  lang::WrappedTargetException, uno::RuntimeException)
{
    long nIndex = pList.is() ? pList->GetIndex( aName ) : -1;
    if( nIndex == -1 )
        throw container::NoSuchElementException();

    XColorEntry* pEntry = pList->GetColor( nIndex );
    return uno::Any( (sal_Int32) pEntry->GetColor().GetRGBColor() );
}

uno::Sequence< OUString > SAL_CALL SvxUnoColorTable::getElementNames()
    throw( uno::RuntimeException )
{
    const long nCount = pList.is() ? pList->Count() : 0;

    uno::Sequence< OUString > aSeq( nCount );
    OUString* pStrings = aSeq.getArray();

    for( long nIndex = 0; nIndex < nCount; nIndex++ )
    {
        XColorEntry* pEntry = pList->GetColor( (long)nIndex );
        pStrings[nIndex] = pEntry->GetName();
    }

    return aSeq;
}

sal_Bool SAL_CALL SvxUnoColorTable::hasByName( const OUString& aName )
    throw( uno::RuntimeException )
{
    long nIndex = pList.is() ? pList->GetIndex( aName ) : -1;
    return nIndex != -1;
}

// XElementAccess
uno::Type SAL_CALL SvxUnoColorTable::getElementType()
    throw( uno::RuntimeException )
{
    return ::getCppuType((const sal_Int32*)0);
}

sal_Bool SAL_CALL SvxUnoColorTable::hasElements()
    throw( uno::RuntimeException )
{
    return pList.is() && pList->Count() != 0;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_drawing_SvxUnoColorTable_implementation_getFactory(
    SAL_UNUSED_PARAMETER css::uno::XComponentContext *,
    uno_Sequence * arguments)
{
    assert(arguments != 0 && arguments->nElements == 0); (void) arguments;
    css::uno::Reference<css::uno::XInterface> x(
        static_cast<cppu::OWeakObject *>(new SvxUnoColorTable));
    x->acquire();
    return x.get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
