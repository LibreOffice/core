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
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ref.hxx>
#include <svx/xtable.hxx>

using namespace ::com::sun::star;

namespace {

class SvxUnoColorTable : public cppu::WeakImplHelper< container::XNameContainer, lang::XServiceInfo >
{
private:
    XColorListRef pList;

public:
    SvxUnoColorTable();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const  OUString& ServiceName ) override;
    virtual uno::Sequence<  OUString > SAL_CALL getSupportedServiceNames() override;

    // XNameContainer
    virtual void SAL_CALL insertByName( const  OUString& aName, const  uno::Any& aElement ) override;
    virtual void SAL_CALL removeByName( const  OUString& Name ) override;

    // XNameReplace
    virtual void SAL_CALL replaceByName( const  OUString& aName, const  uno::Any& aElement ) override;

    // XNameAccess
    virtual uno::Any SAL_CALL getByName( const  OUString& aName ) override;

    virtual uno::Sequence<  OUString > SAL_CALL getElementNames() override;

    virtual sal_Bool SAL_CALL hasByName( const  OUString& aName ) override;

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;
};

SvxUnoColorTable::SvxUnoColorTable()
{
    pList = XPropertyList::AsColorList(
        XPropertyList::CreatePropertyList(
            XPropertyListType::Color, SvtPathOptions().GetPalettePath(), ""));
}

sal_Bool SAL_CALL SvxUnoColorTable::supportsService( const  OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

OUString SAL_CALL SvxUnoColorTable::getImplementationName()
{
    return OUString("com.sun.star.drawing.SvxUnoColorTable");
}

uno::Sequence< OUString > SAL_CALL SvxUnoColorTable::getSupportedServiceNames()
{
    uno::Sequence<OUString> aSNS { "com.sun.star.drawing.ColorTable" };
    return aSNS;
}

// XNameContainer
void SAL_CALL SvxUnoColorTable::insertByName( const OUString& aName, const uno::Any& aElement )
{
    if( hasByName( aName ) )
        throw container::ElementExistException();

    Color aColor;
    if( !(aElement >>= aColor) )
        throw lang::IllegalArgumentException();

    if( pList.is() )
    {
        pList->Insert(std::make_unique<XColorEntry>(aColor, aName));
    }
}

void SAL_CALL SvxUnoColorTable::removeByName( const OUString& Name )
{
    long nIndex = pList.is() ? pList->GetIndex( Name ) : -1;
    if( nIndex == -1 )
        throw container::NoSuchElementException();

    pList->Remove( nIndex );
}

// XNameReplace
void SAL_CALL SvxUnoColorTable::replaceByName( const OUString& aName, const uno::Any& aElement )
{
    Color nColor;
    if( !(aElement >>= nColor) )
        throw lang::IllegalArgumentException();

    long nIndex = pList.is() ? pList->GetIndex( aName ) : -1;
    if( nIndex == -1  )
        throw container::NoSuchElementException();

    pList->Replace(nIndex, std::make_unique<XColorEntry>(nColor, aName ));
}

// XNameAccess
uno::Any SAL_CALL SvxUnoColorTable::getByName( const OUString& aName )
{
    long nIndex = pList.is() ? pList->GetIndex( aName ) : -1;
    if( nIndex == -1 )
        throw container::NoSuchElementException();

    const XColorEntry* pEntry = pList->GetColor(nIndex);
    return uno::Any( static_cast<sal_Int32>(pEntry->GetColor().GetRGBColor()) );
}

uno::Sequence< OUString > SAL_CALL SvxUnoColorTable::getElementNames()
{
    const long nCount = pList.is() ? pList->Count() : 0;

    uno::Sequence< OUString > aSeq( nCount );
    OUString* pStrings = aSeq.getArray();

    for( long nIndex = 0; nIndex < nCount; nIndex++ )
    {
        const XColorEntry* pEntry = pList->GetColor(nIndex);
        pStrings[nIndex] = pEntry->GetName();
    }

    return aSeq;
}

sal_Bool SAL_CALL SvxUnoColorTable::hasByName( const OUString& aName )
{
    long nIndex = pList.is() ? pList->GetIndex( aName ) : -1;
    return nIndex != -1;
}

// XElementAccess
uno::Type SAL_CALL SvxUnoColorTable::getElementType()
{
    return ::cppu::UnoType<sal_Int32>::get();
}

sal_Bool SAL_CALL SvxUnoColorTable::hasElements()
{
    return pList.is() && pList->Count() != 0;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_drawing_SvxUnoColorTable_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SvxUnoColorTable);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
