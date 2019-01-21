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

#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <textapi.hxx>
#include <drawdoc.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/unoforou.hxx>
#include <editeng/unoprnms.hxx>
#include <editeng/unoipset.hxx>
#include <Outliner.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdundo.hxx>

namespace com { namespace sun { namespace star { namespace container { class XNameContainer; } } } }
namespace com { namespace sun { namespace star { namespace text { class XTextField; } } } }

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

namespace sd {

class UndoTextAPIChanged : public SdrUndoAction
{
public:
    UndoTextAPIChanged( SdrModel& rModel, TextApiObject* pTextObj );

    virtual void Undo() override;
    virtual void Redo() override;

protected:
    std::unique_ptr<OutlinerParaObject> mpOldText;
    std::unique_ptr<OutlinerParaObject> mpNewText;
    rtl::Reference< TextApiObject > mxTextObj;
};

UndoTextAPIChanged::UndoTextAPIChanged(SdrModel& rModel, TextApiObject* pTextObj )
: SdrUndoAction( rModel )
, mpOldText( pTextObj->CreateText() )
, mxTextObj( pTextObj )
{
}

void UndoTextAPIChanged::Undo()
{
    if( !mpNewText )
        mpNewText = mxTextObj->CreateText();

    mxTextObj->SetText( *mpOldText );
}

void UndoTextAPIChanged::Redo()
{
    if( mpNewText )
    {
        mxTextObj->SetText( *mpNewText );
    }
}

struct TextAPIEditSource_Impl
{
    SdDrawDocument*                 mpDoc;
    Outliner*                       mpOutliner;
    SvxOutlinerForwarder*           mpTextForwarder;
};

class TextAPIEditSource : public SvxEditSource
{
    // refcounted
    std::shared_ptr<TextAPIEditSource_Impl> m_xImpl;

    virtual std::unique_ptr<SvxEditSource> Clone() const override;
    virtual SvxTextForwarder*   GetTextForwarder() override;
    virtual void                UpdateData() override;
    explicit            TextAPIEditSource( const TextAPIEditSource& rSource );

public:
    explicit            TextAPIEditSource(SdDrawDocument* pDoc);

    void                Dispose();
    void                SetText( OutlinerParaObject const & rText );
    std::unique_ptr<OutlinerParaObject> CreateText();
    OUString            GetText();
    SdDrawDocument*     GetDoc() { return m_xImpl->mpDoc; }
};

static const SvxItemPropertySet* ImplGetSdTextPortionPropertyMap()
{
    static const SfxItemPropertyMapEntry aSdTextPortionPropertyEntries[] =
    {
        SVX_UNOEDIT_CHAR_PROPERTIES,
        SVX_UNOEDIT_FONT_PROPERTIES,
        SVX_UNOEDIT_OUTLINER_PROPERTIES,
        SVX_UNOEDIT_PARA_PROPERTIES,
        {OUString("TextField"),                     EE_FEATURE_FIELD,   cppu::UnoType<XTextField>::get(),  PropertyAttribute::READONLY, 0 },
        {OUString("TextPortionType"),               WID_PORTIONTYPE,    ::cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0 },
        {OUString("TextUserDefinedAttributes"),     EE_CHAR_XMLATTRIBS,     cppu::UnoType<XNameContainer>::get(),        0,     0},
        {OUString("ParaUserDefinedAttributes"),     EE_PARA_XMLATTRIBS,     cppu::UnoType<XNameContainer>::get(),        0,     0},
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    static SvxItemPropertySet aSdTextPortionPropertyMap( aSdTextPortionPropertyEntries, SdrObject::GetGlobalDrawObjectItemPool() );

    return &aSdTextPortionPropertyMap;
}

TextApiObject::TextApiObject( std::unique_ptr<TextAPIEditSource> pEditSource )
: SvxUnoText( pEditSource.get(), ImplGetSdTextPortionPropertyMap(), Reference < XText >() )
, mpSource(std::move(pEditSource))
{
}

TextApiObject::~TextApiObject() throw()
{
    dispose();
}

rtl::Reference< TextApiObject > TextApiObject::create( SdDrawDocument* pDoc )
{
    rtl::Reference< TextApiObject > xRet( new TextApiObject( std::make_unique<TextAPIEditSource>( pDoc ) ) );
    return xRet;
}

void TextApiObject::dispose()
{
    if( mpSource )
    {
        mpSource->Dispose();
        mpSource.reset();
    }

}

std::unique_ptr<OutlinerParaObject> TextApiObject::CreateText()
{
    return mpSource->CreateText();
}

void TextApiObject::SetText( OutlinerParaObject const & rText )
{
    SdrModel* pModel = mpSource->GetDoc();
    if( pModel && pModel->IsUndoEnabled() )
        pModel->AddUndo( std::make_unique<UndoTextAPIChanged>( *pModel, this ) );

    mpSource->SetText( rText );
    maSelection.nStartPara = EE_PARA_MAX_COUNT;
}

OUString TextApiObject::GetText()
{
    return mpSource->GetText();
}

TextApiObject* TextApiObject::getImplementation( const css::uno::Reference< css::text::XText >& xText )
{
    TextApiObject* pImpl = dynamic_cast< TextApiObject* >( xText.get() );

    if( !pImpl )
        pImpl = dynamic_cast< TextApiObject* >(  SvxUnoTextBase::getImplementation( xText ) );

    return pImpl;
}

TextAPIEditSource::TextAPIEditSource(const TextAPIEditSource& rSource)
    : SvxEditSource(*this)
    , m_xImpl(rSource.m_xImpl) // shallow copy; uses internal refcounting
{
}

std::unique_ptr<SvxEditSource> TextAPIEditSource::Clone() const
{
    return std::unique_ptr<SvxEditSource>(new TextAPIEditSource( *this ));
}

void TextAPIEditSource::UpdateData()
{
    // data is kept in outliner all the time
}

TextAPIEditSource::TextAPIEditSource(SdDrawDocument* pDoc)
: m_xImpl(new TextAPIEditSource_Impl)
{
    m_xImpl->mpDoc = pDoc;
    m_xImpl->mpOutliner = nullptr;
    m_xImpl->mpTextForwarder = nullptr;
}

void TextAPIEditSource::Dispose()
{
    m_xImpl->mpDoc=nullptr;
    delete m_xImpl->mpTextForwarder;
    m_xImpl->mpTextForwarder = nullptr;

    delete m_xImpl->mpOutliner;
    m_xImpl->mpOutliner = nullptr;
}

SvxTextForwarder* TextAPIEditSource::GetTextForwarder()
{
    if(!m_xImpl->mpDoc)
        return nullptr; // mpDoc == 0 can be used to flag this as disposed

    if (!m_xImpl->mpOutliner)
    {
        //init draw model first
        m_xImpl->mpOutliner = new SdOutliner(m_xImpl->mpDoc, OutlinerMode::TextObject);
        SdDrawDocument::SetCalcFieldValueHdl(m_xImpl->mpOutliner);
    }

    if (!m_xImpl->mpTextForwarder)
        m_xImpl->mpTextForwarder = new SvxOutlinerForwarder(*m_xImpl->mpOutliner, false);

    return m_xImpl->mpTextForwarder;
}

void TextAPIEditSource::SetText( OutlinerParaObject const & rText )
{
    if (m_xImpl->mpDoc)
    {
        if (!m_xImpl->mpOutliner)
        {
            //init draw model first
            m_xImpl->mpOutliner = new SdOutliner(m_xImpl->mpDoc, OutlinerMode::TextObject);
            SdDrawDocument::SetCalcFieldValueHdl(m_xImpl->mpOutliner);
        }

        m_xImpl->mpOutliner->SetText( rText );
    }
}

std::unique_ptr<OutlinerParaObject> TextAPIEditSource::CreateText()
{
    if (m_xImpl->mpDoc && m_xImpl->mpOutliner)
        return m_xImpl->mpOutliner->CreateParaObject();
    else
        return nullptr;
}

OUString TextAPIEditSource::GetText()
{
    if (m_xImpl->mpDoc && m_xImpl->mpOutliner)
        return m_xImpl->mpOutliner->GetEditEngine().GetText();
    else
        return OUString();
}

} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
