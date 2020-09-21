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
#include <editeng/outlobj.hxx>
#include <editeng/outliner.hxx>
#include <editeng/unoprnms.hxx>
#include <editeng/unoforou.hxx>
#include <editeng/unoipset.hxx>

#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/lang/Locale.hpp>

using namespace com::sun::star;

static const SvxItemPropertySet* ImplGetSvxTextPortionPropertySet()
{
    static const SfxItemPropertyMapEntry aSvxTextPortionPropertyMap[] =
    {
        SVX_UNOEDIT_CHAR_PROPERTIES,
        SVX_UNOEDIT_FONT_PROPERTIES,
        SVX_UNOEDIT_OUTLINER_PROPERTIES,
        SVX_UNOEDIT_PARA_PROPERTIES,
        {u"TextField",                 EE_FEATURE_FIELD,
            cppu::UnoType<text::XTextField>::get(), beans::PropertyAttribute::READONLY, 0 },
        {u"TextPortionType",           WID_PORTIONTYPE,
            ::cppu::UnoType<OUString>::get(), beans::PropertyAttribute::READONLY, 0 },
        {u"TextUserDefinedAttributes", EE_CHAR_XMLATTRIBS,
            cppu::UnoType<css::container::XNameContainer>::get(), 0, 0},
        {u"ParaUserDefinedAttributes", EE_PARA_XMLATTRIBS,
            cppu::UnoType<css::container::XNameContainer>::get(), 0, 0},
        { u"", 0, css::uno::Type(), 0, 0 }
    };
    static SvxItemPropertySet aSvxTextPortionPropertySet( aSvxTextPortionPropertyMap, EditEngine::GetGlobalItemPool() );
    return &aSvxTextPortionPropertySet;
}

SwTextAPIObject::SwTextAPIObject( std::unique_ptr<SwTextAPIEditSource> p )
: SvxUnoText( p.get(), ImplGetSvxTextPortionPropertySet(), uno::Reference < text::XText >() )
, pSource(std::move(p))
{
}

SwTextAPIObject::~SwTextAPIObject() throw()
{
    pSource->Dispose();
    pSource.reset();
}

struct SwTextAPIEditSource_Impl
{
    // needed for "internal" refcounting
    SfxItemPool*                    mpPool;
    SwDoc*                          mpDoc;
    std::unique_ptr<Outliner> mpOutliner;
    std::unique_ptr<SvxOutlinerForwarder> mpTextForwarder;
    sal_Int32                       mnRef;
};

SwTextAPIEditSource::SwTextAPIEditSource( const SwTextAPIEditSource& rSource )
: SvxEditSource( *this )
{
    // shallow copy; uses internal refcounting
    m_pImpl = rSource.m_pImpl;
    m_pImpl->mnRef++;
}

std::unique_ptr<SvxEditSource> SwTextAPIEditSource::Clone() const
{
    return std::unique_ptr<SvxEditSource>(new SwTextAPIEditSource( *this ));
}

void SwTextAPIEditSource::UpdateData()
{
    // data is kept in outliner all the time
}

SwTextAPIEditSource::SwTextAPIEditSource(SwDoc* pDoc)
: m_pImpl(new SwTextAPIEditSource_Impl)
{
    m_pImpl->mpPool = &pDoc->GetDocShell()->GetPool();
    m_pImpl->mpDoc = pDoc;
    m_pImpl->mnRef = 1;
}

SwTextAPIEditSource::~SwTextAPIEditSource()
{
    if (!--m_pImpl->mnRef)
        delete m_pImpl;
}

void SwTextAPIEditSource::Dispose()
{
    m_pImpl->mpPool=nullptr;
    m_pImpl->mpDoc=nullptr;
    m_pImpl->mpTextForwarder.reset();
    m_pImpl->mpOutliner.reset();
}

SvxTextForwarder* SwTextAPIEditSource::GetTextForwarder()
{
    if( !m_pImpl->mpPool )
        return nullptr; // mpPool == 0 can be used to flag this as disposed

    if( !m_pImpl->mpOutliner )
    {
        //init draw model first
        m_pImpl->mpDoc->getIDocumentDrawModelAccess().GetOrCreateDrawModel();
        m_pImpl->mpOutliner.reset(new Outliner(m_pImpl->mpPool, OutlinerMode::TextObject));
        m_pImpl->mpDoc->SetCalcFieldValueHdl(m_pImpl->mpOutliner.get());
    }

    if( !m_pImpl->mpTextForwarder )
    {
        m_pImpl->mpTextForwarder.reset(new SvxOutlinerForwarder(*m_pImpl->mpOutliner, false));
    }

    return m_pImpl->mpTextForwarder.get();
}

void SwTextAPIEditSource::SetText( OutlinerParaObject const & rText )
{
    if ( m_pImpl->mpPool )
    {
        if( !m_pImpl->mpOutliner )
        {
            //init draw model first
            m_pImpl->mpDoc->getIDocumentDrawModelAccess().GetOrCreateDrawModel();
            m_pImpl->mpOutliner.reset(new Outliner(m_pImpl->mpPool, OutlinerMode::TextObject));
            m_pImpl->mpDoc->SetCalcFieldValueHdl(m_pImpl->mpOutliner.get());
        }

        m_pImpl->mpOutliner->SetText( rText );
    }
}

void SwTextAPIEditSource::SetString( const OUString& rText )
{
    if ( !m_pImpl->mpPool )
        return;

    if( !m_pImpl->mpOutliner )
    {
        //init draw model first
        m_pImpl->mpDoc->getIDocumentDrawModelAccess().GetOrCreateDrawModel();
        m_pImpl->mpOutliner.reset(new Outliner(m_pImpl->mpPool, OutlinerMode::TextObject));
        m_pImpl->mpDoc->SetCalcFieldValueHdl(m_pImpl->mpOutliner.get());
    }
    else
        m_pImpl->mpOutliner->Clear();
    m_pImpl->mpOutliner->Insert( rText );
}

std::unique_ptr<OutlinerParaObject> SwTextAPIEditSource::CreateText()
{
    if ( m_pImpl->mpPool && m_pImpl->mpOutliner )
        return m_pImpl->mpOutliner->CreateParaObject();
    else
        return nullptr;
}

OUString SwTextAPIEditSource::GetText() const
{
    if ( m_pImpl->mpPool && m_pImpl->mpOutliner )
        return m_pImpl->mpOutliner->GetEditEngine().GetText();
    else
        return OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
