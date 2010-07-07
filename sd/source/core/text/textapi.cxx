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
#include "precompiled_sd.hxx"

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

using ::rtl::OUString;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

namespace sd {

class UndoTextAPIChanged : public SdrUndoAction
{
public:
    UndoTextAPIChanged( SdrModel& rModel, TextApiObject* pTextObj );
    ~UndoTextAPIChanged();

    virtual void Undo();
    virtual void Redo();

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

    virtual SvxEditSource*      Clone() const;
    virtual SvxTextForwarder*   GetTextForwarder();
    virtual void                UpdateData();
    explicit            TextAPIEditSource( const TextAPIEditSource& rSource );

public:
                        TextAPIEditSource(SdDrawDocument* pDoc);
    virtual             ~TextAPIEditSource();

    void                Dispose();
    void                SetText( OutlinerParaObject& rText );
    OutlinerParaObject* CreateText();
    String              GetText();
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
        {MAP_CHAR_LEN("TextField"),                     EE_FEATURE_FIELD,   &::getCppuType((const Reference< XTextField >*)0),  PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN("TextPortionType"),               WID_PORTIONTYPE,    &::getCppuType((const OUString*)0), PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN("TextUserDefinedAttributes"),     EE_CHAR_XMLATTRIBS,     &::getCppuType((const Reference< XNameContainer >*)0)  ,        0,     0},
        {MAP_CHAR_LEN("ParaUserDefinedAttributes"),     EE_PARA_XMLATTRIBS,     &::getCppuType((const Reference< XNameContainer >*)0)  ,        0,     0},
        {0,0,0,0,0,0}
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

    // SvxUnoText::dispose();
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
    maSelection.nStartPara = 0xffff;
}

String TextApiObject::GetText()
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
        pImpl->mpTextForwarder = new SvxOutlinerForwarder( *pImpl->mpOutliner, 0 );

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

String TextAPIEditSource::GetText()
{
    if ( pImpl->mpDoc && pImpl->mpOutliner )
        return pImpl->mpOutliner->GetEditEngine().GetText();
    else
        return String();
}

} // namespace sd
