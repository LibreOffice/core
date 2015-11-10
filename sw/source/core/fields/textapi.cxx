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

#include <textapi.hxx>
#include <doc.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <docsh.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editeng.hxx>

#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

using namespace com::sun::star;

static const SvxItemPropertySet* ImplGetSvxTextPortionPropertySet()
{
    static const SfxItemPropertyMapEntry aSvxTextPortionPropertyMap[] =
    {
        SVX_UNOEDIT_CHAR_PROPERTIES,
        SVX_UNOEDIT_FONT_PROPERTIES,
        SVX_UNOEDIT_OUTLINER_PROPERTIES,
        SVX_UNOEDIT_PARA_PROPERTIES,
        {OUString("TextField"),                 EE_FEATURE_FIELD,
            cppu::UnoType<text::XTextField>::get(), beans::PropertyAttribute::READONLY, 0 },
        {OUString("TextPortionType"),           WID_PORTIONTYPE,
            ::cppu::UnoType<OUString>::get(), beans::PropertyAttribute::READONLY, 0 },
        {OUString("TextUserDefinedAttributes"), EE_CHAR_XMLATTRIBS,
            cppu::UnoType<css::container::XNameContainer>::get(), 0, 0},
        {OUString("ParaUserDefinedAttributes"), EE_PARA_XMLATTRIBS,
            cppu::UnoType<css::container::XNameContainer>::get(), 0, 0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    static SvxItemPropertySet aSvxTextPortionPropertySet( aSvxTextPortionPropertyMap, EditEngine::GetGlobalItemPool() );
    return &aSvxTextPortionPropertySet;
}

SwTextAPIObject::SwTextAPIObject( SwTextAPIEditSource* p )
: SvxUnoText( p, ImplGetSvxTextPortionPropertySet(), uno::Reference < text::XText >() )
, pSource(p)
{
}

SwTextAPIObject::~SwTextAPIObject() throw()
{
    pSource->Dispose();
    delete pSource;
}

struct SwTextAPIEditSource_Impl
{
    // needed for "internal" refcounting
    SfxItemPool*                    mpPool;
    SwDoc*                          mpDoc;
    Outliner*                       mpOutliner;
    SvxOutlinerForwarder*           mpTextForwarder;
    sal_Int32                       mnRef;
};

SwTextAPIEditSource::SwTextAPIEditSource( const SwTextAPIEditSource& rSource )
: SvxEditSource( *this )
{
    // shallow copy; uses internal refcounting
    pImpl = rSource.pImpl;
    pImpl->mnRef++;
}

SvxEditSource* SwTextAPIEditSource::Clone() const
{
    return new SwTextAPIEditSource( *this );
}

void SwTextAPIEditSource::UpdateData()
{
    // data is kept in outliner all the time
}

SwTextAPIEditSource::SwTextAPIEditSource(SwDoc* pDoc)
: pImpl(new SwTextAPIEditSource_Impl)
{
    pImpl->mpPool = &pDoc->GetDocShell()->GetPool();
    pImpl->mpDoc = pDoc;
    pImpl->mpOutliner = nullptr;
    pImpl->mpTextForwarder = nullptr;
    pImpl->mnRef = 1;
}

SwTextAPIEditSource::~SwTextAPIEditSource()
{
    if (!--pImpl->mnRef)
        delete pImpl;
}

void SwTextAPIEditSource::Dispose()
{
    pImpl->mpPool=nullptr;
    pImpl->mpDoc=nullptr;
    DELETEZ(pImpl->mpTextForwarder);
    DELETEZ(pImpl->mpOutliner);
}

SvxTextForwarder* SwTextAPIEditSource::GetTextForwarder()
{
    if( !pImpl->mpPool )
        return nullptr; // mpPool == 0 can be used to flag this as disposed

    if( !pImpl->mpOutliner )
    {
        //init draw model first
        pImpl->mpDoc->getIDocumentDrawModelAccess().GetOrCreateDrawModel();
        pImpl->mpOutliner = new Outliner( pImpl->mpPool, OUTLINERMODE_TEXTOBJECT );
        pImpl->mpDoc->SetCalcFieldValueHdl( pImpl->mpOutliner );
    }

    if( !pImpl->mpTextForwarder )
        pImpl->mpTextForwarder = new SvxOutlinerForwarder( *pImpl->mpOutliner, false );

    return pImpl->mpTextForwarder;
}

void SwTextAPIEditSource::SetText( OutlinerParaObject& rText )
{
    if ( pImpl->mpPool )
    {
        if( !pImpl->mpOutliner )
        {
            //init draw model first
            pImpl->mpDoc->getIDocumentDrawModelAccess().GetOrCreateDrawModel();
            pImpl->mpOutliner = new Outliner( pImpl->mpPool, OUTLINERMODE_TEXTOBJECT );
            pImpl->mpDoc->SetCalcFieldValueHdl( pImpl->mpOutliner );
        }

        pImpl->mpOutliner->SetText( rText );
    }
}

void SwTextAPIEditSource::SetString( const OUString& rText )
{
    if ( pImpl->mpPool )
    {
        if( !pImpl->mpOutliner )
        {
            //init draw model first
            pImpl->mpDoc->getIDocumentDrawModelAccess().GetOrCreateDrawModel();
            pImpl->mpOutliner = new Outliner( pImpl->mpPool, OUTLINERMODE_TEXTOBJECT );
            pImpl->mpDoc->SetCalcFieldValueHdl( pImpl->mpOutliner );
        }
        else
            pImpl->mpOutliner->Clear();
        pImpl->mpOutliner->Insert( rText );
    }
}

OutlinerParaObject* SwTextAPIEditSource::CreateText()
{
    if ( pImpl->mpPool && pImpl->mpOutliner )
        return pImpl->mpOutliner->CreateParaObject();
    else
        return nullptr;
}

OUString SwTextAPIEditSource::GetText()
{
    if ( pImpl->mpPool && pImpl->mpOutliner )
        return pImpl->mpOutliner->GetEditEngine().GetText();
    else
        return OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
