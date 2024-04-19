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
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/text/XTextField.hpp>

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

namespace com::sun::star::container { class XNameContainer; }

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

namespace sd {

namespace {

class UndoTextAPIChanged : public SdrUndoAction
{
public:
    UndoTextAPIChanged( SdrModel& rModel, TextApiObject* pTextObj );

    virtual void Undo() override;
    virtual void Redo() override;

protected:
    std::optional<OutlinerParaObject> mpOldText;
    std::optional<OutlinerParaObject> mpNewText;
    rtl::Reference< TextApiObject > mxTextObj;
};

}

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

namespace
{

struct OutlinerHolder
{
    SdrModel* mpModel;
    std::unique_ptr<Outliner> mpOutliner;
    std::unique_ptr<SvxOutlinerForwarder> mpTextForwarder;
};

}

class TextAPIEditSource : public SvxEditSource
{
    // refcounted
    std::shared_ptr<OutlinerHolder> mpHolder;

    virtual std::unique_ptr<SvxEditSource> Clone() const override;
    virtual SvxTextForwarder* GetTextForwarder() override;
    virtual void UpdateData() override;
    explicit TextAPIEditSource(const TextAPIEditSource& rSource);

public:
    explicit TextAPIEditSource(SdrModel* pModel);

    void SetText(OutlinerParaObject const & rText);
    std::optional<OutlinerParaObject> CreateText();
    OUString GetText() const;
    SdrModel* getModel() { return mpHolder->mpModel; }
};

static const SvxItemPropertySet* ImplGetSdTextPortionPropertyMap()
{
    static const SfxItemPropertyMapEntry aSdTextPortionPropertyEntries[] =
    {
        SVX_UNOEDIT_CHAR_PROPERTIES,
        SVX_UNOEDIT_FONT_PROPERTIES,
        SVX_UNOEDIT_OUTLINER_PROPERTIES,
        SVX_UNOEDIT_PARA_PROPERTIES,
        {u"TextField"_ustr,                     EE_FEATURE_FIELD,   cppu::UnoType<XTextField>::get(),  PropertyAttribute::READONLY, 0 },
        {u"TextPortionType"_ustr,               WID_PORTIONTYPE,    ::cppu::UnoType<OUString>::get(), PropertyAttribute::READONLY, 0 },
        {u"TextUserDefinedAttributes"_ustr,     EE_CHAR_XMLATTRIBS,     cppu::UnoType<XNameContainer>::get(),        0,     0},
        {u"ParaUserDefinedAttributes"_ustr,     EE_PARA_XMLATTRIBS,     cppu::UnoType<XNameContainer>::get(),        0,     0},
    };
    static SvxItemPropertySet aSdTextPortionPropertyMap( aSdTextPortionPropertyEntries, SdrObject::GetGlobalDrawObjectItemPool() );

    return &aSdTextPortionPropertyMap;
}

TextApiObject::TextApiObject( std::unique_ptr<TextAPIEditSource> pEditSource )
: SvxUnoText( pEditSource.get(), ImplGetSdTextPortionPropertyMap(), Reference < XText >() )
, mpSource(std::move(pEditSource))
{
}

TextApiObject::~TextApiObject() noexcept
{
    dispose();
}

rtl::Reference<TextApiObject> TextApiObject::create(SdrModel* pModel)
{
    rtl::Reference<TextApiObject> xRet(new TextApiObject(std::make_unique<TextAPIEditSource>(pModel)));
    return xRet;
}

void TextApiObject::dispose()
{
    if (mpSource)
        mpSource.reset();
}

std::optional<OutlinerParaObject> TextApiObject::CreateText()
{
    return mpSource->CreateText();
}

void TextApiObject::SetText( OutlinerParaObject const & rText )
{
    SdrModel* pModel = mpSource->getModel();
    if( pModel && pModel->IsUndoEnabled() )
        pModel->AddUndo( std::make_unique<UndoTextAPIChanged>( *pModel, this ) );

    mpSource->SetText( rText );
    maSelection.nStartPara = EE_PARA_MAX_COUNT;
}

OUString TextApiObject::GetText() const
{
    return mpSource->GetText();
}

TextApiObject* TextApiObject::getImplementation( const css::uno::Reference< css::text::XText >& xText )
{
    TextApiObject* pImpl = dynamic_cast< TextApiObject* >( xText.get() );

    if( !pImpl )
        pImpl = dynamic_cast< TextApiObject* >(  comphelper::getFromUnoTunnel<SvxUnoTextBase>( xText ) );

    return pImpl;
}

TextAPIEditSource::TextAPIEditSource(const TextAPIEditSource& rSource)
    : SvxEditSource(*this)
    , mpHolder(rSource.mpHolder) // shallow copy; uses internal refcounting
{
}

std::unique_ptr<SvxEditSource> TextAPIEditSource::Clone() const
{
    return std::unique_ptr<SvxEditSource>(new TextAPIEditSource(*this));
}

void TextAPIEditSource::UpdateData()
{
    // data is kept in outliner all the time
}

TextAPIEditSource::TextAPIEditSource(SdrModel* pModel)
    : mpHolder(std::make_shared<OutlinerHolder>())
{
    mpHolder->mpModel = pModel;
}

SvxTextForwarder* TextAPIEditSource::GetTextForwarder()
{
    if (!mpHolder->mpModel)
        return nullptr; // mpModel == 0 can be used to flag this as disposed

    if (!mpHolder->mpOutliner)
    {
        //init draw model first
        SfxItemPool* pPool = &mpHolder->mpModel->GetItemPool();
        mpHolder->mpOutliner.reset(new SdrOutliner(pPool, OutlinerMode::TextObject));
        SdDrawDocument::SetCalcFieldValueHdl(mpHolder->mpOutliner.get());
    }

    if (!mpHolder->mpTextForwarder)
        mpHolder->mpTextForwarder.reset(new SvxOutlinerForwarder(*mpHolder->mpOutliner, false));

    return mpHolder->mpTextForwarder.get();
}

void TextAPIEditSource::SetText( OutlinerParaObject const & rText )
{
    if (mpHolder->mpModel)
    {
        if (!mpHolder->mpOutliner)
        {
            //init draw model first
            SfxItemPool* pPool = &mpHolder->mpModel->GetItemPool();
            mpHolder->mpOutliner.reset(new SdrOutliner(pPool, OutlinerMode::TextObject));
            SdDrawDocument::SetCalcFieldValueHdl(mpHolder->mpOutliner.get());
        }

        mpHolder->mpOutliner->SetText(rText);
    }
}

std::optional<OutlinerParaObject> TextAPIEditSource::CreateText()
{
    if (mpHolder->mpModel && mpHolder->mpOutliner)
        return mpHolder->mpOutliner->CreateParaObject();
    else
        return std::nullopt;
}

OUString TextAPIEditSource::GetText() const
{
    if (mpHolder->mpModel && mpHolder->mpOutliner)
        return mpHolder->mpOutliner->GetEditEngine().GetText();
    else
        return OUString();
}

} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
