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
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const  OUString& ServiceName ) override;
    virtual cpo::uno::Sequence<  OUString > getSupportedServiceNames() override;

    // XNameContainer
    virtual void insertByName( const  OUString& aName, const  cpo::uno::Any& aElement ) override;
    virtual void removeByName( const  OUString& Name ) override;

    // XNameReplace
    virtual void replaceByName( const  OUString& aName, const  cpo::uno::Any& aElement ) override;

    // XNameAccess
    virtual cpo::uno::Any getByName( const  OUString& aName ) override;

    virtual cpo::uno::Sequence<  OUString > getElementNames() override;

    virtual bool hasByName( const  OUString& aName ) override;

    // XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;
};

SvxUnoColorTable::SvxUnoColorTable()
  : pList(XPropertyList::AsColorList(
            XPropertyList::CreatePropertyList(
              XPropertyListType::Color, SvtPathOptions().GetPalettePath(), u""_ustr)))
{
}

bool SvxUnoColorTable::supportsService( const  OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

OUString SvxUnoColorTable::getImplementationName()
{
    return u"com.sun.star.drawing.SvxUnoColorTable"_ustr;
}

cpo::uno::Sequence< OUString > SvxUnoColorTable::getSupportedServiceNames()
{
    cpo::uno::Sequence<OUString> aSNS { u"com.sun.star.drawing.ColorTable"_ustr };
    return aSNS;
}

// XNameContainer
void SvxUnoColorTable::insertByName( const OUString& aName, const cpo::uno::Any& aElement )
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

void SvxUnoColorTable::removeByName( const OUString& Name )
{
    tools::Long nIndex = pList.is() ? pList->GetIndex( Name ) : -1;
    if( nIndex == -1 )
        throw container::NoSuchElementException();

    pList->Remove( nIndex );
}

// XNameReplace
void SvxUnoColorTable::replaceByName( const OUString& aName, const cpo::uno::Any& aElement )
{
    Color nColor;
    if( !(aElement >>= nColor) )
        throw lang::IllegalArgumentException();

    tools::Long nIndex = pList.is() ? pList->GetIndex( aName ) : -1;
    if( nIndex == -1  )
        throw container::NoSuchElementException();

    pList->Replace(nIndex, std::make_unique<XColorEntry>(nColor, aName ));
}

// XNameAccess
cpo::uno::Any SvxUnoColorTable::getByName( const OUString& aName )
{
    tools::Long nIndex = pList.is() ? pList->GetIndex( aName ) : -1;
    if( nIndex == -1 )
        throw container::NoSuchElementException();

    const XColorEntry* pEntry = pList->GetColor(nIndex);
    return cpo::uno::Any( static_cast<sal_Int32>(pEntry->GetColor().GetRGBColor()) );
}

cpo::uno::Sequence< OUString > SvxUnoColorTable::getElementNames()
{
    const tools::Long nCount = pList.is() ? pList->Count() : 0;

    cpo::uno::Sequence< OUString > aSeq( nCount );
    OUString* pStrings = aSeq.getArray();

    for( tools::Long nIndex = 0; nIndex < nCount; nIndex++ )
    {
        const XColorEntry* pEntry = pList->GetColor(nIndex);
        pStrings[nIndex] = pEntry->GetName();
    }

    return aSeq;
}

bool SvxUnoColorTable::hasByName( const OUString& aName )
{
    tools::Long nIndex = pList.is() ? pList->GetIndex( aName ) : -1;
    return nIndex != -1;
}

// XElementAccess
cpo::uno::Type SvxUnoColorTable::getElementType()
{
    return ::cppu::UnoType<sal_Int32>::get();
}

bool SvxUnoColorTable::hasElements()
{
    return pList.is() && pList->Count() != 0;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_drawing_SvxUnoColorTable_get_implementation(
    css::uno::XComponentContext *,
    cpo::uno::Sequence<cpo::uno::Any> const &)
{
    return cppu::acquire(new SvxUnoColorTable);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
