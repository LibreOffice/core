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

#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <textapi.hxx>
#include <drawdoc.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/outlobj.hxx>
#include "Outliner.hxx"
#include <svx/svdpool.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

namespace sd {

class UndoTextAPIChanged : public SdrUndoAction
{
public:
    UndoTextAPIChanged( SdrModel& rModel, TextApiObject* pTextObj );
    virtual ~UndoTextAPIChanged();

    virtual void Undo() override;
    virtual void Redo() override;

protected:
    OutlinerParaObject* mpOldText;
    OutlinerParaObject* mpNewText;
    rtl::Reference< TextApiObject > mxTextObj;
};

UndoTextAPIChanged::UndoTextAPIChanged(SdrModel& rModel, TextApiObject* pTextObj )
: SdrUndoAction( rModel )
, mpOldText( pTextObj->CreateText() )
, mpNewText( 0 )
, mxTextObj( pTextObj )
{
}

UndoTextAPIChanged::~UndoTextAPIChanged()
{
    delete mpOldText;
    delete mpNewText;
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
    // needed for "internal" refcounting
    SdDrawDocument*                 mpDoc;
    Outliner*                       mpOutliner;
    SvxOutlinerForwarder*           mpTextForwarder;
    sal_Int32                       mnRef;
};

class TextAPIEditSource : public SvxEditSource
{
    TextAPIEditSource_Impl* pImpl;

    virtual SvxEditSource*      Clone() const override;
    virtual SvxTextForwarder*   GetTextForwarder() override;
    virtual void                UpdateData() override;
    explicit            TextAPIEditSource( const TextAPIEditSource& rSource );

public:
                        explicit TextAPIEditSource(SdDrawDocument* pDoc);
    virtual             ~TextAPIEditSource();

    void                Dispose();
    void                SetText( OutlinerParaObject& rText );
    OutlinerParaObject* CreateText();
    OUString            GetText();
    SdDrawDocument*     GetDoc() { return pImpl->mpDoc; }
};

const SvxItemPropertySet* ImplGetSdTextPortionPropertyMap()
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

TextApiObject::TextApiObject( TextAPIEditSource* pEditSource )
: SvxUnoText( pEditSource, ImplGetSdTextPortionPropertyMap(), Reference < XText >() )
, mpSource(pEditSource)
{
}

TextApiObject::~TextApiObject() throw()
{
    dispose();
}

rtl::Reference< TextApiObject > TextApiObject::create( SdDrawDocument* pDoc )
{
    rtl::Reference< TextApiObject > xRet( new TextApiObject( new TextAPIEditSource( pDoc ) ) );
    return xRet;
}

void SAL_CALL TextApiObject::dispose() throw(RuntimeException)
{
    if( mpSource )
    {
        mpSource->Dispose();
        delete mpSource;
        mpSource = 0;
    }

}

OutlinerParaObject* TextApiObject::CreateText()
{
    return mpSource->CreateText();
}

void TextApiObject::SetText( OutlinerParaObject& rText )
{
    SdrModel* pModel = mpSource->GetDoc();
    if( pModel && pModel->IsUndoEnabled() )
        pModel->AddUndo( new UndoTextAPIChanged( *pModel, this ) );

    mpSource->SetText( rText );
    maSelection.nStartPara = EE_PARA_MAX_COUNT;
}

OUString TextApiObject::GetText()
{
    return mpSource->GetText();
}

TextApiObject* TextApiObject::getImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >& xText )
{
    TextApiObject* pImpl = dynamic_cast< TextApiObject* >( xText.get() );

    if( !pImpl )
        pImpl = dynamic_cast< TextApiObject* >(  SvxUnoTextBase::getImplementation( xText ) );

    return pImpl;
}

TextAPIEditSource::TextAPIEditSource( const TextAPIEditSource& rSource )
: SvxEditSource( *this )
{
    // shallow copy; uses internal refcounting
    pImpl = rSource.pImpl;
    pImpl->mnRef++;
}

SvxEditSource* TextAPIEditSource::Clone() const
{
    return new TextAPIEditSource( *this );
}

void TextAPIEditSource::UpdateData()
{
    // data is kept in outliner all the time
}

TextAPIEditSource::TextAPIEditSource(SdDrawDocument* pDoc)
: pImpl(new TextAPIEditSource_Impl)
{
    pImpl->mpDoc = pDoc;
    pImpl->mpOutliner = 0;
    pImpl->mpTextForwarder = 0;
    pImpl->mnRef = 1;
}

TextAPIEditSource::~TextAPIEditSource()
{
    if (!--pImpl->mnRef)
        delete pImpl;
}

void TextAPIEditSource::Dispose()
{
    pImpl->mpDoc=0;
    delete pImpl->mpTextForwarder;
    pImpl->mpTextForwarder = 0;

    delete pImpl->mpOutliner;
    pImpl->mpOutliner = 0;
}

SvxTextForwarder* TextAPIEditSource::GetTextForwarder()
{
    if( !pImpl->mpDoc )
        return 0; // mpDoc == 0 can be used to flag this as disposed

    if( !pImpl->mpOutliner )
    {
        //init draw model first
        pImpl->mpOutliner = new Outliner( pImpl->mpDoc, OUTLINERMODE_TEXTOBJECT );
        pImpl->mpDoc->SetCalcFieldValueHdl( pImpl->mpOutliner );
    }

    if( !pImpl->mpTextForwarder )
        pImpl->mpTextForwarder = new SvxOutlinerForwarder( *pImpl->mpOutliner, false );

    return pImpl->mpTextForwarder;
}

void TextAPIEditSource::SetText( OutlinerParaObject& rText )
{
    if ( pImpl->mpDoc )
    {
        if( !pImpl->mpOutliner )
        {
            //init draw model first
            pImpl->mpOutliner = new Outliner( pImpl->mpDoc, OUTLINERMODE_TEXTOBJECT );
            pImpl->mpDoc->SetCalcFieldValueHdl( pImpl->mpOutliner );
        }

        pImpl->mpOutliner->SetText( rText );
    }
}

OutlinerParaObject* TextAPIEditSource::CreateText()
{
    if ( pImpl->mpDoc && pImpl->mpOutliner )
        return pImpl->mpOutliner->CreateParaObject();
    else
        return 0;
}

OUString TextAPIEditSource::GetText()
{
    if ( pImpl->mpDoc && pImpl->mpOutliner )
        return pImpl->mpOutliner->GetEditEngine().GetText();
    else
        return OUString();
}

} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
