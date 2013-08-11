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

#include "imp_share.hxx"
#include <com/sun/star/form/binding/XBindableValue.hpp>
#include <com/sun/star/form/binding/XValueBinding.hpp>
#include <com/sun/star/form/binding/XListEntrySink.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/document/XGraphicObjectResolver.hpp>
#include <com/sun/star/script/vba/XVBACompatibility.hpp>

#include <comphelper/processfactory.hxx>
#include <i18nlangtag/languagetag.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace xmlscript
{

Reference< xml::input::XElement > Frame::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException)
{
    if ( !m_xContainer.is() )
        m_xContainer.set( _pImport->_xDialogModelFactory->createInstance( "com.sun.star.awt.UnoFrameModel" ), UNO_QUERY );
    // event
    if (_pImport->isEventElement( nUid, rLocalName ))
    {
       return new EventElement( nUid, rLocalName, xAttributes, this, _pImport );
    }
    else if ( rLocalName == "bulletinboard" )
    {
        // Create new DialogImport for this container
        DialogImport* pFrameImport = new DialogImport( *_pImport );
        pFrameImport->_xDialogModel = m_xContainer;
        return new BulletinBoardElement( rLocalName, xAttributes, this,  pFrameImport );
    }
    else if ( rLocalName == "title" )
    {
        getStringAttr( &_label, "value", xAttributes, _pImport->XMLNS_DIALOGS_UID );

        return new ElementBase( _pImport->XMLNS_DIALOGS_UID, rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        SAL_INFO("xmlscript.xmldlg","****** ARGGGGG!!!! **********");
        throw     xml::sax::SAXException("expected event element!", Reference< XInterface >(), Any() );
    }
}

void Frame::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
        if ( !m_xContainer.is() )
            m_xContainer.set( _pImport->_xDialogModelFactory->createInstance( "com.sun.star.awt.UnoFrameModel" ), UNO_QUERY );
        Reference< beans::XPropertySet > xProps( m_xContainer, UNO_QUERY_THROW );
        // _pImport is what we need to add to ( e.g. the dialog in this case )
    ControlImportContext ctx( _pImport, xProps,   getControlId( _xAttributes ) );

    Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importTextLineColorStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( 0, 0, _xAttributes ); // inherited from BulletinBoardElement
    if (!_label.isEmpty())
    {
        xControlModel->setPropertyValue( "Label" , makeAny( _label ) );
    }
#ifdef SCROLLABLEFRAME
    ctx.importScollableSettings( _xAttributes );
#endif
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

//===
Reference< xml::input::XElement > MultiPage::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    // event
    if (_pImport->isEventElement( nUid, rLocalName ))
    {
        return new EventElement(
            nUid, rLocalName, xAttributes, this, _pImport );
    }
    else if ( rLocalName == "bulletinboard" )
    {
        // Create new DialogImport for this container

        DialogImport* pMultiPageImport = new DialogImport( *_pImport );
                pMultiPageImport->_xDialogModel = m_xContainer;
        return new BulletinBoardElement( rLocalName, xAttributes, this,  pMultiPageImport );
    }
    else
    {

        throw xml::sax::SAXException( "expected event element!", Reference< XInterface >(), Any() );
    }
}

void MultiPage::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
        Reference< beans::XPropertySet > xProps( m_xContainer, UNO_QUERY_THROW );
        // _pImport is what we need to add to ( e.g. the dialog in this case )
    ControlImportContext ctx( _pImport, xProps, getControlId( _xAttributes ));

    Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importTextLineColorStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
        pStyle->importBackgroundColorStyle( xControlModel );
    }

    ctx.importDefaults( 0, 0, _xAttributes ); // inherited from BulletinBoardElement
    ctx.importLongProperty("MultiPageValue" , "value",  _xAttributes );
        ctx.importBooleanProperty( "Decoration", "withtabs",  _xAttributes) ;
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

Reference< xml::input::XElement > Page::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    // event
    if (_pImport->isEventElement( nUid, rLocalName ))
    {
        return new EventElement(
            nUid, rLocalName, xAttributes, this, _pImport );
    }
    else if ( rLocalName == "bulletinboard" )
    {

        DialogImport* pPageImport = new DialogImport( *_pImport );
                pPageImport->_xDialogModel = m_xContainer;
        return new BulletinBoardElement( rLocalName, xAttributes, this,  pPageImport );
    }
    else
    {

        throw xml::sax::SAXException("expected event element!", Reference< XInterface >(), Any() );
    }
}

void Page::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
        Reference< beans::XPropertySet > xProps( m_xContainer, UNO_QUERY_THROW );

    ControlImportContext ctx( _pImport, xProps, getControlId( _xAttributes ));

    Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importTextLineColorStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
        pStyle->importBackgroundColorStyle( xControlModel );
    }

    ctx.importDefaults( 0, 0, _xAttributes ); // inherited from BulletinBoardElement
    ctx.importStringProperty( OUString( "Title" ), OUString( "title" ), _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

// progessmeter
Reference< xml::input::XElement > ProgressBarElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    // event
    if (_pImport->isEventElement( nUid, rLocalName ))
    {
        return new EventElement(
            nUid, rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( "expected event element!" ),
            Reference< XInterface >(), Any() );
    }
}
void ProgressBarElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx( _pImport, getControlId( _xAttributes ), "com.sun.star.awt.UnoControlProgressBarModel" );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFillColorStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importLongProperty( "ProgressValue", "value", _xAttributes );
    ctx.importLongProperty( "ProgressValueMin", "value-min", _xAttributes );
    ctx.importLongProperty( "ProgressValueMax", "value-max", _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

// scrollbar
Reference< xml::input::XElement > ScrollBarElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    // event
    if (_pImport->isEventElement( nUid, rLocalName ))
    {
        return new EventElement( nUid, rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException("expected event element!", Reference< XInterface >(), Any() );
    }
}
void ScrollBarElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx( _pImport, getControlId( _xAttributes ), getControlModelName( "com.sun.star.awt.UnoControlScrollBarModel" , _xAttributes ) );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importOrientationProperty( "Orientation" , "align", _xAttributes );
    ctx.importLongProperty( "BlockIncrement" , "pageincrement" , _xAttributes );
    ctx.importLongProperty( "LineIncrement" , "increment" , _xAttributes );
    ctx.importLongProperty( "ScrollValue" ,"curpos", _xAttributes );
    ctx.importLongProperty( "ScrollValueMax" , "maxpos" , _xAttributes );
    ctx.importLongProperty( "ScrollValueMin","minpos", _xAttributes );
    ctx.importLongProperty( "VisibleSize", "visible-size", _xAttributes );
    ctx.importLongProperty( "RepeatDelay", "repeat", _xAttributes );
    ctx.importBooleanProperty( "Tabstop", "tabstop" , _xAttributes );
    ctx.importBooleanProperty( "LiveScroll", "live-scroll", _xAttributes );
    ctx.importHexLongProperty( "SymbolColor", "symbol-color", _xAttributes );

    ctx.importDataAwareProperty( "linked-cell" , _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

// spinbutton
Reference< xml::input::XElement > SpinButtonElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    // event
    if (_pImport->isEventElement( nUid, rLocalName ))
    {
        return new EventElement( nUid, rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException("expected event element!", Reference< XInterface >(), Any() );
    }
}

void SpinButtonElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx( _pImport, getControlId( _xAttributes ), getControlModelName( "com.sun.star.awt.UnoControlSpinButtonModel", _xAttributes ) );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importOrientationProperty( "Orientation", "align", _xAttributes );
    ctx.importLongProperty("SpinIncrement", "increment", _xAttributes );
    ctx.importLongProperty("SpinValue", "curval" ,_xAttributes );
    ctx.importLongProperty("SpinValueMax", "maxval", _xAttributes );
    ctx.importLongProperty( "SpinValueMin","minval",_xAttributes );
    ctx.importLongProperty( "Repeat", "repeat", _xAttributes );
    ctx.importLongProperty( "RepeatDelay", "repeat-delay",_xAttributes );
    ctx.importBooleanProperty( "Tabstop", "tabstop", _xAttributes );
    ctx.importHexLongProperty( "SymbolColor", "symbol-color" , _xAttributes );
    ctx.importDataAwareProperty( "linked-cell" , _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

// fixedline
Reference< xml::input::XElement > FixedLineElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    // event
    if (_pImport->isEventElement( nUid, rLocalName ))
    {
        return new EventElement( nUid, rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException("expected event element!", Reference< XInterface >(), Any() );
    }
}
void FixedLineElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(_pImport, getControlId( _xAttributes ), "com.sun.star.awt.UnoControlFixedLineModel" );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importTextLineColorStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importStringProperty( "Label", "value", _xAttributes );
    ctx.importOrientationProperty( "Orientation", "align", _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

// patternfield
Reference< xml::input::XElement > PatternFieldElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    // event
    if (_pImport->isEventElement( nUid, rLocalName ))
    {
        return new EventElement( nUid, rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException("expected event element!", Reference< XInterface >(), Any() );
    }
}
void PatternFieldElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx( _pImport, getControlId( _xAttributes ), "com.sun.star.awt.UnoControlPatternFieldModel" );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importTextLineColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty( "Tabstop", "tabstop", _xAttributes );
    ctx.importBooleanProperty( "ReadOnly", "readonly" , _xAttributes );
    ctx.importBooleanProperty( "StrictFormat", "strict-format", _xAttributes );
    ctx.importBooleanProperty( "HideInactiveSelection", "hide-inactive-selection", _xAttributes );
    ctx.importStringProperty( "Text", "value", _xAttributes );
    ctx.importShortProperty( "MaxTextLen", "maxlength", _xAttributes );
    ctx.importStringProperty( "EditMask", "edit-mask", _xAttributes );
    ctx.importStringProperty( "LiteralMask", "literal-mask", _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

// formattedfield
Reference< xml::input::XElement > FormattedFieldElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    // event
    if (_pImport->isEventElement( nUid, rLocalName ))
    {
        return new EventElement( nUid, rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException("expected event element!", Reference< XInterface >(), Any() );
    }
}

void FormattedFieldElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx( _pImport, getControlId( _xAttributes ), "com.sun.star.awt.UnoControlFormattedFieldModel" );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importTextLineColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty( "Tabstop", "tabstop", _xAttributes );
    ctx.importBooleanProperty( "ReadOnly", "readonly" , _xAttributes );
    ctx.importBooleanProperty( "StrictFormat", "strict-format" , _xAttributes );
    ctx.importBooleanProperty( "HideInactiveSelection", "hide-inactive-selection", _xAttributes );
    ctx.importAlignProperty( "Align" , "align" , _xAttributes );
    ctx.importDoubleProperty( "EffectiveMin", "value-min" , _xAttributes );
    ctx.importDoubleProperty( "EffectiveMax", "value-max", _xAttributes);
    ctx.importDoubleProperty( "EffectiveValue", "value", _xAttributes );
    ctx.importStringProperty( "Text", "text", _xAttributes );
    ctx.importShortProperty( "MaxTextLen", "maxlength", _xAttributes );
    ctx.importBooleanProperty( "Spin",  "spin", _xAttributes );
    if (ctx.importLongProperty( "RepeatDelay", "repeat", _xAttributes ))
        ctx.getControlModel()->setPropertyValue( "Repeat" , makeAny(true) );

    OUString sDefault(_xAttributes->getValueByUidName(_pImport->XMLNS_DIALOGS_UID, "value-default") );
    if (!sDefault.isEmpty())
    {
        double d = sDefault.toDouble();
        if (d != 0.0 || sDefault == "0" || sDefault == "0.0" )
        {
            ctx.getControlModel()->setPropertyValue( "EffectiveDefault", makeAny( d ) );
        }
        else // treat as string
        {
            ctx.getControlModel()->setPropertyValue( "EffectiveDefault", makeAny( sDefault ) );
        }
    }

    // format spec
    ctx.getControlModel()->setPropertyValue("FormatsSupplier", makeAny( _pImport->getNumberFormatsSupplier() ) );

    OUString sFormat( _xAttributes->getValueByUidName(_pImport->XMLNS_DIALOGS_UID, "format-code" ) );
    if (!sFormat.isEmpty())
    {
        lang::Locale locale;

        OUString sLocale( _xAttributes->getValueByUidName( _pImport->XMLNS_DIALOGS_UID, "format-locale" ) );
        if (!sLocale.isEmpty())
        {
            // split locale
            // Don't know what may have written what we read here, so parse all
            // old style including the trailing ";Variant" if present.
            sal_Int32 semi0 = sLocale.indexOf( ';' );
            if (semi0 < 0) // no semi at all, try new BCP47 or just language
            {
                locale = LanguageTag::convertToLocale( sLocale, false);
            }
            else
            {
                sal_Int32 semi1 = sLocale.indexOf( ';', semi0 +1 );
                if (semi1 > semi0) // language;country;variant
                {
                    SAL_WARN( "xmlscript.xmldlg", "format-locale with variant that is ignored: " << sLocale);
                    locale.Language = sLocale.copy( 0, semi0 );
                    locale.Country = sLocale.copy( semi0 +1, semi1 - semi0 -1 );
                    // Ignore Variant that no one knows what it would be.
                }
                else // language;country
                {
                    locale.Language = sLocale.copy( 0, semi0 );
                    locale.Country = sLocale.copy( semi0 +1 );
                }
            }
        }

        try
        {
            Reference< util::XNumberFormats > xFormats(
                _pImport->getNumberFormatsSupplier()->getNumberFormats() );
            sal_Int32 nKey = xFormats->queryKey( sFormat, locale, sal_True );
            if (-1 == nKey)
            {
                nKey = xFormats->addNew( sFormat, locale );
            }
            ctx.getControlModel()->setPropertyValue("FormatKey", makeAny( nKey ) );
        }
        catch (const util::MalformedNumberFormatException & exc)
        {
           SAL_WARN( "xmlscript.xmldlg", "### util::MalformedNumberFormatException occurred!" );
            // rethrow
            throw xml::sax::SAXException( exc.Message, Reference< XInterface >(), Any() );
        }
    }
    ctx.importBooleanProperty("TreatAsNumber", "treat-as-number" , _xAttributes );
    ctx.importBooleanProperty("EnforceFormat", "enforce-format", _xAttributes );

    ctx.importDataAwareProperty( "linked-cell" , _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

// timefield
Reference< xml::input::XElement > TimeFieldElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    // event
    if (_pImport->isEventElement( nUid, rLocalName ))
    {
        return new EventElement( nUid, rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException("expected event element!", Reference< XInterface >(), Any() );
    }
}
void TimeFieldElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx( _pImport, getControlId( _xAttributes ), "com.sun.star.awt.UnoControlTimeFieldModel" );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importTextLineColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty("Tabstop", "tabstop", _xAttributes );
    ctx.importBooleanProperty("ReadOnly", "readonly", _xAttributes );
    ctx.importBooleanProperty( "StrictFormat", "strict-format", _xAttributes );
    ctx.importBooleanProperty("HideInactiveSelection","hide-inactive-selection", _xAttributes );
    ctx.importTimeFormatProperty( "TimeFormat", "time-format", _xAttributes );
    ctx.importTimeProperty( "Time", "value", _xAttributes );
    ctx.importTimeProperty( "TimeMin", "value-min", _xAttributes );
    ctx.importTimeProperty( "TimeMax", "value-max", _xAttributes );
    ctx.importBooleanProperty( "Spin", "spin", _xAttributes );
    if (ctx.importLongProperty( "RepeatDelay", "repeat", _xAttributes ))
        ctx.getControlModel()->setPropertyValue("Repeat", makeAny(true) );
    ctx.importStringProperty( "Text", "text" , _xAttributes );
    ctx.importBooleanProperty( "EnforceFormat", "enforce-format" , _xAttributes );

    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

// numericfield
Reference< xml::input::XElement > NumericFieldElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    // event
    if (_pImport->isEventElement( nUid, rLocalName ))
    {
        return new EventElement( nUid, rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException( "expected event element!", Reference< XInterface >(), Any() );
    }
}
void NumericFieldElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx( _pImport, getControlId( _xAttributes ), "com.sun.star.awt.UnoControlNumericFieldModel" );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importTextLineColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty( "Tabstop","tabstop",_xAttributes );
    ctx.importBooleanProperty( "ReadOnly", "readonly",_xAttributes );
    ctx.importBooleanProperty( "StrictFormat", "strict-format", _xAttributes );
    ctx.importBooleanProperty( "HideInactiveSelection", "hide-inactive-selection", _xAttributes );
    ctx.importShortProperty( "DecimalAccuracy", "decimal-accuracy", _xAttributes );
    ctx.importBooleanProperty( "ShowThousandsSeparator", "thousands-separator", _xAttributes );
    ctx.importDoubleProperty( "Value", "value", _xAttributes );
    ctx.importDoubleProperty( "ValueMin", "value-min", _xAttributes );
    ctx.importDoubleProperty( "ValueMax", "value-max", _xAttributes );
    ctx.importDoubleProperty( "ValueStep", "value-step", _xAttributes );
    ctx.importBooleanProperty( "Spin", "spin", _xAttributes );
    if (ctx.importLongProperty( "RepeatDelay", "repeat",  _xAttributes ))
        ctx.getControlModel()->setPropertyValue("Repeat", makeAny(true) );
    ctx.importBooleanProperty( "EnforceFormat", "enforce-format", _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

// datefield
Reference< xml::input::XElement > DateFieldElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    // event
    if (_pImport->isEventElement( nUid, rLocalName ))
    {
        return new EventElement( nUid, rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException("expected event element!", Reference< XInterface >(), Any() );
    }
}
void DateFieldElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx( _pImport, getControlId( _xAttributes ), "com.sun.star.awt.UnoControlDateFieldModel" );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importTextLineColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty( "Tabstop", "tabstop", _xAttributes );
    ctx.importBooleanProperty( "ReadOnly", "readonly", _xAttributes );
    ctx.importBooleanProperty( "StrictFormat", "strict-format", _xAttributes );
    ctx.importBooleanProperty( "HideInactiveSelection", "hide-inactive-selection", _xAttributes );
    ctx.importDateFormatProperty( "DateFormat", "date-format", _xAttributes );
    ctx.importBooleanProperty( "DateShowCentury", "show-century", _xAttributes );
    ctx.importDateProperty( "Date", "value", _xAttributes );
    ctx.importDateProperty( "DateMin", "value-min", _xAttributes );
    ctx.importDateProperty( "DateMax", "value-max", _xAttributes );
    ctx.importBooleanProperty( "Spin", "spin", _xAttributes );
    if (ctx.importLongProperty( "RepeatDelay", "repeat", _xAttributes ))
        ctx.getControlModel()->setPropertyValue( "Repeat", makeAny(true) );
    ctx.importBooleanProperty( "Dropdown", "dropdown", _xAttributes );
    ctx.importStringProperty( "Text", "text", _xAttributes );
    ctx.importBooleanProperty( "EnforceFormat", "enforce-format", _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

// currencyfield
Reference< xml::input::XElement > CurrencyFieldElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    // event
    if (_pImport->isEventElement( nUid, rLocalName ))
    {
        return new EventElement( nUid, rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException( "expected event element!" , Reference< XInterface >(), Any() );
    }
}
void CurrencyFieldElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx( _pImport, getControlId( _xAttributes ), "com.sun.star.awt.UnoControlCurrencyFieldModel" );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importTextLineColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty("Tabstop", "tabstop", _xAttributes );
    ctx.importBooleanProperty( "ReadOnly", "readonly" , _xAttributes );
    ctx.importBooleanProperty( "StrictFormat", "strict-format", _xAttributes );
    ctx.importBooleanProperty( "HideInactiveSelection", "hide-inactive-selection", _xAttributes );
    ctx.importStringProperty( "CurrencySymbol", "currency-symbol", _xAttributes );
    ctx.importShortProperty( "DecimalAccuracy", "decimal-accuracy", _xAttributes );
    ctx.importBooleanProperty( "ShowThousandsSeparator", "thousands-separator", _xAttributes );
    ctx.importDoubleProperty( "Value", "value", _xAttributes );
    ctx.importDoubleProperty( "ValueMin", "value-min", _xAttributes );
    ctx.importDoubleProperty( "ValueMax", "value-max", _xAttributes );
    ctx.importDoubleProperty( "ValueStep", "value-step", _xAttributes );
    ctx.importBooleanProperty( "Spin", "spin", _xAttributes );
    if (ctx.importLongProperty( "RepeatDelay", "repeat", _xAttributes ))
        ctx.getControlModel()->setPropertyValue( "Repeat", makeAny(true) );
    ctx.importBooleanProperty( "PrependCurrencySymbol", "prepend-symbol", _xAttributes );
    ctx.importBooleanProperty( "EnforceFormat", "enforce-format", _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

// filecontrol
Reference< xml::input::XElement > FileControlElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    // event
    if (_pImport->isEventElement( nUid, rLocalName ))
    {
        return new EventElement( nUid, rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException( "expected event element!", Reference< XInterface >(), Any() );
    }
}
void FileControlElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx( _pImport, getControlId( _xAttributes ), "com.sun.star.awt.UnoControlFileControlModel" );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importTextLineColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty( "Tabstop", "tabstop", _xAttributes );
    ctx.importBooleanProperty("HideInactiveSelection","hide-inactive-selection", _xAttributes );
    ctx.importStringProperty( "Text", "value", _xAttributes );
    ctx.importBooleanProperty( "ReadOnly", "readonly", _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

// treecontrol
Reference< xml::input::XElement > TreeControlElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    // event
    if (_pImport->isEventElement( nUid, rLocalName ))
    {
        return new EventElement( nUid, rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException( "expected event element!", Reference< XInterface >(), Any() );
    }
}
void TreeControlElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx( _pImport, getControlId( _xAttributes ), "com.sun.star.awt.tree.TreeControlModel" );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty( "Tabstop", "tabstop", _xAttributes );
    ctx.importSelectionTypeProperty( "SelectionType", "selectiontype", _xAttributes );
    ctx.importBooleanProperty( "RootDisplayed", "rootdisplayed", _xAttributes );
    ctx.importBooleanProperty( "ShowsHandles", "showshandles", _xAttributes );
    ctx.importBooleanProperty( "ShowsRootHandles", "showsroothandles" ,_xAttributes );
    ctx.importBooleanProperty( "Editable", "editable", _xAttributes );
    ctx.importBooleanProperty( "RowHeight", "readonly", _xAttributes );
    ctx.importBooleanProperty( "InvokesStopNodeEditing", "invokesstopnodeediting", _xAttributes );

    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

// imagecontrol
Reference< xml::input::XElement > ImageControlElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    // event
    if (_pImport->isEventElement( nUid, rLocalName ))
    {
        return new EventElement( nUid, rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException( "expected event element!" , Reference< XInterface >(), Any() );
    }
}

void ImageControlElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx( _pImport, getControlId( _xAttributes ), "com.sun.star.awt.UnoControlImageControlModel" );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty( "ScaleImage", "scale-image", _xAttributes );
    Reference< document::XStorageBasedDocument > xDocStorage( _pImport->getDocOwner(), UNO_QUERY );

    ctx.importImageURLProperty( "ImageURL" , "src" , _xAttributes );
    ctx.importBooleanProperty( "Tabstop", "tabstop", _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

// textfield
Reference< xml::input::XElement > TextElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    // event
    if (_pImport->isEventElement( nUid, rLocalName ))
    {
        return new EventElement( nUid, rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException( "expected event element!", Reference< XInterface >(), Any() );
    }
}

void TextElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx( _pImport, getControlId( _xAttributes ), "com.sun.star.awt.UnoControlFixedTextModel" );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importTextLineColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importStringProperty( "Label", "value", _xAttributes );
    ctx.importAlignProperty( "Align", "align", _xAttributes );
    ctx.importVerticalAlignProperty( "VerticalAlign", "valign", _xAttributes );
    ctx.importBooleanProperty( "MultiLine", "multiline" ,_xAttributes );
    ctx.importBooleanProperty( "Tabstop", "tabstop", _xAttributes );
    ctx.importBooleanProperty( "NoLabel", "nolabel", _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

// FixedHyperLink
Reference< xml::input::XElement > FixedHyperLinkElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    // event
    if (_pImport->isEventElement( nUid, rLocalName ))
    {
        return new EventElement( nUid, rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException( "expected event element!" , Reference< XInterface >(), Any() );
    }
}
void FixedHyperLinkElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx( _pImport, getControlId( _xAttributes ), "com.sun.star.awt.UnoControlFixedHyperlinkModel" );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importTextLineColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importStringProperty( "Label", "value", _xAttributes );
    ctx.importStringProperty( "URL", "url", _xAttributes );
    ctx.importStringProperty( "Description", "description", _xAttributes );

    ctx.importAlignProperty( "Align", "align" ,_xAttributes );
    ctx.importVerticalAlignProperty( "VerticalAlign", "valign", _xAttributes );
    ctx.importBooleanProperty( "MultiLine", "multiline", _xAttributes );
    ctx.importBooleanProperty( "Tabstop", "tabstop", _xAttributes );
    ctx.importBooleanProperty( "NoLabel", "nolabel", _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

// edit
Reference< xml::input::XElement > TextFieldElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    // event
    if (_pImport->isEventElement( nUid, rLocalName ))
    {
        return new EventElement( nUid, rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException( "expected event element!", Reference< XInterface >(), Any() );
    }
}
void TextFieldElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx( _pImport, getControlId( _xAttributes ), "com.sun.star.awt.UnoControlEditModel" );
    Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importTextLineColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty("Tabstop", "tabstop", _xAttributes );
    ctx.importAlignProperty( "Align", "align", _xAttributes );
    ctx.importBooleanProperty( "HardLineBreaks", "hard-linebreaks", _xAttributes );
    ctx.importBooleanProperty( "HScroll", "hscroll" ,_xAttributes );
    ctx.importBooleanProperty( "VScroll", "vscroll", _xAttributes );
    ctx.importBooleanProperty("HideInactiveSelection", "hide-inactive-selection", _xAttributes );
    ctx.importShortProperty( "MaxTextLen", "maxlength", _xAttributes );
    ctx.importBooleanProperty( "MultiLine", "multiline", _xAttributes );
    ctx.importBooleanProperty( "ReadOnly", "readonly", _xAttributes );
    ctx.importStringProperty( "Text", "value", _xAttributes );
    ctx.importLineEndFormatProperty( "LineEndFormat", "lineend-format", _xAttributes );
    OUString aValue;
    if (getStringAttr( &aValue, "echochar", _xAttributes, _pImport->XMLNS_DIALOGS_UID ) && !aValue.isEmpty() )
    {
        SAL_WARN_IF( aValue.getLength() != 1, "xmlscript.xmldlg", "### more than one character given for echochar!" );
        sal_Int16 nChar = (sal_Int16)aValue[ 0 ];
        xControlModel->setPropertyValue( "EchoChar", makeAny( nChar ) );
    }

    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

// titledbox
Reference< xml::input::XElement > TitledBoxElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    // event
    if (_pImport->isEventElement( nUid, rLocalName ))
    {
        return new EventElement( nUid, rLocalName, xAttributes, this, _pImport );
    }
    else if (_pImport->XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException( "illegal namespace!", Reference< XInterface >(), Any() );
    }
    // title
    else if ( rLocalName == "title" )
    {
        getStringAttr( &_label, "value", xAttributes, _pImport->XMLNS_DIALOGS_UID );

        return new ElementBase( _pImport->XMLNS_DIALOGS_UID, rLocalName, xAttributes, this, _pImport );
    }
    // radio
    else if ( rLocalName == "radio" )
    {
        // dont create radios here, => titledbox must be inserted first due to radio grouping,
        // possible predecessors!
        Reference< xml::input::XElement > xRet(
            new RadioElement( rLocalName, xAttributes, this, _pImport ) );
        _radios.push_back( xRet );
        return xRet;
    }
    // event
    else if (_pImport->isEventElement( nUid, rLocalName ))
    {
        return new EventElement( nUid, rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        return BulletinBoardElement::startChildElement( nUid, rLocalName, xAttributes );
    }
}

void TitledBoxElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    {
    ControlImportContext ctx(_pImport, getControlId( _xAttributes ), "com.sun.star.awt.UnoControlGroupBoxModel" );
    Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importTextLineColorStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( 0, 0, _xAttributes ); // inherited from BulletinBoardElement

    if (!_label.isEmpty())
    {
        xControlModel->setPropertyValue( "Label", makeAny( _label ) );
    }

    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
    }

    // create radios AFTER group box!
    for ( size_t nPos = 0; nPos < _radios.size(); ++nPos )
    {
        Reference< xml::input::XElement > xRadio( _radios[ nPos ] );
        Reference< xml::input::XAttributes > xAttributes( xRadio->getAttributes() );

        ControlImportContext ctx( _pImport, getControlId( xAttributes ), getControlModelName( "com.sun.star.awt.UnoControlRadioButtonModel", xAttributes ) );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );

        Reference< xml::input::XElement > xStyle( getStyle( xAttributes ) );
        if (xStyle.is())
        {
            StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
            pStyle->importBackgroundColorStyle( xControlModel );
            pStyle->importTextColorStyle( xControlModel );
            pStyle->importTextLineColorStyle( xControlModel );
            pStyle->importFontStyle( xControlModel );
            pStyle->importVisualEffectStyle( xControlModel );
        }

        ctx.importDefaults( _nBasePosX, _nBasePosY, xAttributes );
        ctx.importBooleanProperty( "Tabstop", "tabstop", xAttributes );
        ctx.importStringProperty( "Label", "value", xAttributes );
        ctx.importAlignProperty( "Align", "align", xAttributes );
        ctx.importVerticalAlignProperty( "VerticalAlign", "valign", xAttributes );
        ctx.importImageURLProperty( "ImageURL" ,  "image-src" , _xAttributes );
        ctx.importImagePositionProperty( "ImagePosition", "image-position", xAttributes );
        ctx.importBooleanProperty( "MultiLine", "multiline", xAttributes );
        ctx.importStringProperty( "GroupName", "group-name", xAttributes );

        sal_Int16 nVal = 0;
        sal_Bool bChecked = sal_False;
        if (getBoolAttr( &bChecked, "checked", xAttributes, _pImport->XMLNS_DIALOGS_UID ) && bChecked)
        {
            nVal = 1;
        }
        xControlModel->setPropertyValue( "State", makeAny( nVal ) );
        ctx.importDataAwareProperty( "linked-cell" , xAttributes );
        ::std::vector< Reference< xml::input::XElement > > * radioEvents =
            static_cast< RadioElement * >( xRadio.get() )->getEvents();
        ctx.importEvents( *radioEvents );
        // avoid ring-reference:
        // vector< event elements > holding event elements holding this (via _pParent)
        radioEvents->clear();
    }
    // avoid ring-reference:
    // vector< radio elements > holding radio elements holding this (via _pParent)
    _radios.clear();
}

// radio
Reference< xml::input::XElement > RadioElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    // event
    if (_pImport->isEventElement( nUid, rLocalName ))
    {
        return new EventElement( nUid, rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException("expected event element!", Reference< XInterface >(), Any() );
    }
}

// radiogroup
Reference< xml::input::XElement > RadioGroupElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (_pImport->XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException( "illegal namespace!", Reference< XInterface >(), Any() );
    }
    // radio
    else if ( rLocalName == "radio" )
    {
        // dont create radios here, => titledbox must be inserted first due to radio grouping,
        // possible predecessors!
        Reference< xml::input::XElement > xRet(
            new RadioElement( rLocalName, xAttributes, this, _pImport ) );
        _radios.push_back( xRet );
        return xRet;
    }
    else
    {
        throw xml::sax::SAXException( "expected radio element!", Reference< XInterface >(), Any() );
    }
}
void RadioGroupElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    for ( size_t nPos = 0; nPos < _radios.size(); ++nPos )
    {
        Reference< xml::input::XElement > xRadio( _radios[ nPos ] );
        Reference< xml::input::XAttributes > xAttributes(
            xRadio->getAttributes() );

        ControlImportContext ctx( _pImport, getControlId( xAttributes ), getControlModelName( "com.sun.star.awt.UnoControlRadioButtonModel", xAttributes ) );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );

        Reference< xml::input::XElement > xStyle( getStyle( xAttributes ) );
        if (xStyle.is())
        {
            StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
            pStyle->importBackgroundColorStyle( xControlModel );
            pStyle->importTextColorStyle( xControlModel );
            pStyle->importTextLineColorStyle( xControlModel );
            pStyle->importFontStyle( xControlModel );
            pStyle->importVisualEffectStyle( xControlModel );
        }

        ctx.importDefaults( _nBasePosX, _nBasePosY, xAttributes );
        ctx.importBooleanProperty("Tabstop", "tabstop", xAttributes );
        ctx.importStringProperty( "Label", "value", xAttributes );
        ctx.importAlignProperty( "Align", "align", xAttributes );
        ctx.importVerticalAlignProperty( "VerticalAlign", "valign", xAttributes );
        ctx.importImageURLProperty( "ImageURL" , "image-src" , xAttributes );
        ctx.importImagePositionProperty( "ImagePosition", "image-position", xAttributes );
        ctx.importBooleanProperty( "MultiLine", "multiline", xAttributes );
        ctx.importStringProperty( "GroupName", "group-name", xAttributes );
        sal_Int16 nVal = 0;
        sal_Bool bChecked = sal_False;
        if (getBoolAttr( &bChecked, "checked", xAttributes, _pImport->XMLNS_DIALOGS_UID ) && bChecked)
        {
            nVal = 1;
        }
        xControlModel->setPropertyValue( "State", makeAny( nVal ) );

        ctx.importDataAwareProperty( "linked-cell", xAttributes );

        ::std::vector< Reference< xml::input::XElement > > * radioEvents =
            static_cast< RadioElement * >( xRadio.get() )->getEvents();
        ctx.importEvents( *radioEvents );
        // avoid ring-reference:
        // vector< event elements > holding event elements holding this (via _pParent)
        radioEvents->clear();
    }
    // avoid ring-reference:
    // vector< radio elements > holding radio elements holding this (via _pParent)
    _radios.clear();
}

// menupopup
Reference< xml::input::XElement > MenuPopupElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (_pImport->XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException( "illegal namespace!", Reference< XInterface >(), Any() );
    }
    // menuitem
    else if ( rLocalName == "menuitem" )
    {
        OUString aValue( xAttributes->getValueByUidName( _pImport->XMLNS_DIALOGS_UID,"value" ) );
        SAL_WARN_IF( aValue.isEmpty(), "xmlscript.xmldlg", "### menuitem has no value?" );
        if (!aValue.isEmpty())
        {
            _itemValues.push_back( aValue );

            OUString aSel( xAttributes->getValueByUidName( _pImport->XMLNS_DIALOGS_UID, "selected" ) );
            if (!aSel.isEmpty() && aSel == "true")
            {
                _itemSelected.push_back( static_cast<sal_Int16>(_itemValues.size()) -1 );
            }
        }
        return new ElementBase( _pImport->XMLNS_DIALOGS_UID, rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException("expected menuitem!" , Reference< XInterface >(), Any() );
    }
}
Sequence< OUString > MenuPopupElement::getItemValues()
{
    Sequence< OUString > aRet( _itemValues.size() );
    OUString * pRet = aRet.getArray();
    for ( size_t nPos = _itemValues.size(); nPos--; )
    {
        pRet[ nPos ] = _itemValues[ nPos ];
    }
    return aRet;
}
Sequence< sal_Int16 > MenuPopupElement::getSelectedItems()
{
    Sequence< sal_Int16 > aRet( _itemSelected.size() );
    sal_Int16 * pRet = aRet.getArray();
    for ( size_t nPos = _itemSelected.size(); nPos--; )
    {
        pRet[ nPos ] = _itemSelected[ nPos ];
    }
    return aRet;
}

// menulist
Reference< xml::input::XElement > MenuListElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    // event
    if (_pImport->isEventElement( nUid, rLocalName ))
    {
        return new EventElement( nUid, rLocalName, xAttributes, this, _pImport );
    }
    else if (_pImport->XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException( "illegal namespace!", Reference< XInterface >(), Any() );
    }
    // menupopup
    else if ( rLocalName == "menupopup" )
    {
        _popup = new MenuPopupElement( rLocalName, xAttributes, this, _pImport );
        return _popup;
    }
    else
    {
        throw xml::sax::SAXException( "expected event or menupopup element!", Reference< XInterface >(), Any() );
    }
}

void MenuListElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx( _pImport, getControlId( _xAttributes ), getControlModelName( "com.sun.star.awt.UnoControlListBoxModel", _xAttributes  ) );
    Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importTextLineColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty( "Tabstop", "tabstop", _xAttributes );
    ctx.importBooleanProperty( "MultiSelection", "multiselection", _xAttributes );
    ctx.importBooleanProperty( "ReadOnly", "readonly", _xAttributes );
    ctx.importBooleanProperty( "Dropdown", "spin", _xAttributes );
    ctx.importShortProperty( "LineCount", "linecount", _xAttributes );
    ctx.importAlignProperty( "Align", "align", _xAttributes );
    bool bHasLinkedCell = ctx.importDataAwareProperty( "linked-cell" , _xAttributes );
    bool bHasSrcRange = ctx.importDataAwareProperty( "source-cell-range" , _xAttributes );
    if (_popup.is())
    {
        MenuPopupElement * p = static_cast< MenuPopupElement * >( _popup.get() );
        if ( !bHasSrcRange )
            xControlModel->setPropertyValue( "StringItemList", makeAny( p->getItemValues() ) );
        if ( !bHasLinkedCell )
            xControlModel->setPropertyValue( "SelectedItems", makeAny( p->getSelectedItems() ) );

    }
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

// combobox
Reference< xml::input::XElement > ComboBoxElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    // event
    if (_pImport->isEventElement( nUid, rLocalName ))
    {
        return new EventElement( nUid, rLocalName, xAttributes, this, _pImport );
    }
    else if (_pImport->XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException( "illegal namespace!", Reference< XInterface >(), Any() );
    }
    // menupopup
    else if ( rLocalName == "menupopup" )
    {
        _popup = new MenuPopupElement( rLocalName, xAttributes, this, _pImport );
        return _popup;
    }
    else
    {
        throw xml::sax::SAXException( "expected event or menupopup element!", Reference< XInterface >(), Any() );
    }
}
void ComboBoxElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx( _pImport, getControlId( _xAttributes ), getControlModelName( "com.sun.star.awt.UnoControlComboBoxModel", _xAttributes ) );
    Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importTextLineColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty( "Tabstop", "tabstop", _xAttributes );
    ctx.importBooleanProperty( "ReadOnly", "readonly", _xAttributes );
    ctx.importBooleanProperty( "Autocomplete", "autocomplete", _xAttributes );
    ctx.importBooleanProperty( "Dropdown", "spin", _xAttributes );
    ctx.importBooleanProperty( "HideInactiveSelection", "hide-inactive-selection", _xAttributes );
    ctx.importShortProperty( "MaxTextLen", "maxlength" ,_xAttributes );
    ctx.importShortProperty( "LineCount", "linecount" ,_xAttributes );
    ctx.importStringProperty( "Text", "value", _xAttributes );
    ctx.importAlignProperty( "Align", "align", _xAttributes );
    ctx.importDataAwareProperty( "linked-cell" , _xAttributes );
    bool bHasSrcRange = ctx.importDataAwareProperty( "source-cell-range" , _xAttributes );
    if (_popup.is() && !bHasSrcRange )
    {
        MenuPopupElement * p = static_cast< MenuPopupElement * >( _popup.get() );
        xControlModel->setPropertyValue( "StringItemList", makeAny( p->getItemValues() ) );
    }

    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

// checkbox
Reference< xml::input::XElement > CheckBoxElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    // event
    if (_pImport->isEventElement( nUid, rLocalName ))
    {
        return new EventElement( nUid, rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException( "expected event element!", Reference< XInterface >(), Any() );
    }
}
void CheckBoxElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx( _pImport, getControlId( _xAttributes ), "com.sun.star.awt.UnoControlCheckBoxModel" );
    Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importTextLineColorStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
        pStyle->importVisualEffectStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty( "Tabstop", "tabstop", _xAttributes );
    ctx.importStringProperty( "Label", "value", _xAttributes );
    ctx.importAlignProperty( "Align", "align", _xAttributes );
    ctx.importVerticalAlignProperty( "VerticalAlign", "valign", _xAttributes );
    ctx.importImageURLProperty( "ImageURL" ,  "image-src" , _xAttributes );
    ctx.importImagePositionProperty( "ImagePosition", "image-position", _xAttributes );
    ctx.importBooleanProperty( "MultiLine", "multiline", _xAttributes );

    sal_Bool bTriState = sal_False;
    if (getBoolAttr( &bTriState, "tristate", _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        xControlModel->setPropertyValue( "TriState", makeAny( bTriState ) );
    }
    sal_Bool bChecked = sal_False;
    if (getBoolAttr( &bChecked, "checked", _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        // has "checked" attribute
        sal_Int16 nVal = (bChecked ? 1 : 0);
        xControlModel->setPropertyValue( "State", makeAny( nVal ) );
    }
    else
    {
        sal_Int16 nVal = (bTriState ? 2 : 0); // if tristate set, but checked omitted => dont know!
        xControlModel->setPropertyValue( "State", makeAny( nVal ) );
    }

    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

// button
Reference< xml::input::XElement > ButtonElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    // event
    if (_pImport->isEventElement( nUid, rLocalName ))
    {
        return new EventElement( nUid, rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException( "expected event element!",  Reference< XInterface >(), Any() );
    }
}

void ButtonElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx( _pImport, getControlId( _xAttributes ), "com.sun.star.awt.UnoControlButtonModel" );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importTextLineColorStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty( "Tabstop", "tabstop", _xAttributes );
    ctx.importStringProperty( "Label", "value", _xAttributes );
    ctx.importAlignProperty( "Align", "align", _xAttributes );
    ctx.importVerticalAlignProperty( "VerticalAlign", "valign", _xAttributes );
    ctx.importBooleanProperty( "DefaultButton", "default", _xAttributes );
    ctx.importButtonTypeProperty( "PushButtonType", "button-type", _xAttributes );
    ctx.importImageURLProperty( "ImageURL" , "image-src" , _xAttributes );
    ctx.importImagePositionProperty( "ImagePosition", "image-position", _xAttributes );
    ctx.importImageAlignProperty( "ImageAlign", "image-align", _xAttributes );
    if (ctx.importLongProperty( "RepeatDelay", "repeat", _xAttributes ))
        ctx.getControlModel()->setPropertyValue( "Repeat", makeAny(true) );
    sal_Int32 toggled = 0;
    if (getLongAttr( &toggled, "toggled", _xAttributes, _pImport->XMLNS_DIALOGS_UID ) && toggled == 1)
        ctx.getControlModel()->setPropertyValue( "Toggle" , makeAny(true));
    ctx.importBooleanProperty( "FocusOnClick", "grab-focus", _xAttributes );
    ctx.importBooleanProperty( "MultiLine", "multiline", _xAttributes );
    // State
    sal_Bool bChecked = sal_False;
    if (getBoolAttr( &bChecked, "checked", _xAttributes, _pImport->XMLNS_DIALOGS_UID ) && bChecked)
    {
        sal_Int16 nVal = 1;
        ctx.getControlModel()->setPropertyValue( "State" , makeAny( nVal ) );
    }

    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

// bulletinboard
Reference< xml::input::XElement > BulletinBoardElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (_pImport->XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException("illegal namespace!", Reference< XInterface >(), Any() );
    }
    // button
    else if ( rLocalName == "button" )
    {
        return new ButtonElement( rLocalName, xAttributes, this, _pImport );
    }
    // checkbox
    else if ( rLocalName == "checkbox" )
    {
        return new CheckBoxElement( rLocalName, xAttributes, this, _pImport );
    }
    // combobox
    else if ( rLocalName == "combobox" )
    {
        return new ComboBoxElement( rLocalName, xAttributes, this, _pImport );
    }
    // listbox
    else if ( rLocalName == "menulist" )
    {
        return new MenuListElement( rLocalName, xAttributes, this, _pImport );
    }
    // radiogroup
    else if ( rLocalName == "radiogroup" )
    {
        return new RadioGroupElement( rLocalName, xAttributes, this, _pImport );
    }
    // titledbox
    else if ( rLocalName == "titledbox" )
    {
        return new TitledBoxElement( rLocalName, xAttributes, this, _pImport );
    }
    // text
    else if ( rLocalName == "text" )
    {
        return new TextElement( rLocalName, xAttributes, this, _pImport );
    }
    else if ( rLocalName == "linklabel" )
    {
        return new FixedHyperLinkElement( rLocalName, xAttributes, this, _pImport );
    }
    // textfield
    else if ( rLocalName == "textfield" )
    {
        return new TextFieldElement( rLocalName, xAttributes, this, _pImport );
    }
    // img
    else if ( rLocalName == "img" )
    {
        return new ImageControlElement( rLocalName, xAttributes, this, _pImport );
    }
    // filecontrol
    else if ( rLocalName == "filecontrol" )
    {
        return new FileControlElement( rLocalName, xAttributes, this, _pImport );
    }
    // treecontrol
    else if ( rLocalName == "treecontrol" )
    {
        return new TreeControlElement( rLocalName, xAttributes, this, _pImport );
    }
    // currencyfield
    else if ( rLocalName == "currencyfield" )
    {
        return new CurrencyFieldElement( rLocalName, xAttributes, this, _pImport );
    }
    // datefield
    else if ( rLocalName == "datefield" )
    {
        return new DateFieldElement( rLocalName, xAttributes, this, _pImport );
    }
    // datefield
    else if ( rLocalName == "numericfield" )
    {
        return new NumericFieldElement( rLocalName, xAttributes, this, _pImport );
    }
    // timefield
    else if ( rLocalName == "timefield" )
    {
        return new TimeFieldElement( rLocalName, xAttributes, this, _pImport );
    }
    // patternfield
    else if ( rLocalName == "patternfield" )
    {
        return new PatternFieldElement( rLocalName, xAttributes, this, _pImport );
    }
    // formattedfield
    else if ( rLocalName == "formattedfield" )
    {
        return new FormattedFieldElement( rLocalName, xAttributes, this, _pImport );
    }
    // fixedline
    else if ( rLocalName == "fixedline" )
    {
        return new FixedLineElement( rLocalName, xAttributes, this, _pImport );
    }
    // scrollbar
    else if ( rLocalName == "scrollbar" )
    {
        return new ScrollBarElement( rLocalName, xAttributes, this, _pImport );
    }
    // spinbutton
    else if ( rLocalName == "spinbutton" )
    {
        return new SpinButtonElement( rLocalName, xAttributes, this, _pImport );
    }
    // progressmeter
    else if ( rLocalName == "progressmeter" )
    {
        return new ProgressBarElement( rLocalName, xAttributes, this, _pImport );
    }
    else if ( rLocalName == "multipage" )
    {
        return new MultiPage( rLocalName, xAttributes, this, _pImport );
    }
    else if ( rLocalName == "frame" )
    {
        return new Frame( rLocalName, xAttributes, this, _pImport );
    }
    else if ( rLocalName == "page" )
    {
        return new Page( rLocalName, xAttributes, this, _pImport );
    }
    // bulletinboard
    else if ( rLocalName == "bulletinboard" )
    {
        return new BulletinBoardElement( rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException( "expected styles, bulletinboard or bulletinboard element, not: " + rLocalName, Reference< XInterface >(), Any() );
    }
}

BulletinBoardElement::BulletinBoardElement(
    OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes,
    ElementBase * pParent, DialogImport * pImport )
    SAL_THROW(())
    : ControlElement( rLocalName, xAttributes, pParent, pImport )
{
    OUString aValue( _xAttributes->getValueByUidName( _pImport->XMLNS_DIALOGS_UID, "left" ) );
    if (!aValue.isEmpty())
    {
        _nBasePosX += toInt32( aValue );
    }
    aValue = _xAttributes->getValueByUidName( _pImport->XMLNS_DIALOGS_UID, "top" );
    if (!aValue.isEmpty())
    {
        _nBasePosY += toInt32( aValue );
    }
}

// style
Reference< xml::input::XElement > StyleElement::startChildElement(
    sal_Int32 /*nUid*/, OUString const & /*rLocalName*/,
    Reference< xml::input::XAttributes > const & /*xAttributes*/ )
    throw (xml::sax::SAXException, RuntimeException)
{
    throw xml::sax::SAXException( "unexpected sub elements of style!", Reference< XInterface >(), Any() );
}

void StyleElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    Reference< xml::input::XAttributes > xStyle;
    OUString aStyleId( _xAttributes->getValueByUidName( _pImport->XMLNS_DIALOGS_UID, "style-id" ) );
    if (!aStyleId.isEmpty())
    {
        _pImport->addStyle( aStyleId, this );
    }
    else
    {
        throw xml::sax::SAXException( "missing style-id attribute!", Reference< XInterface >(), Any() );
    }
}

// styles
Reference< xml::input::XElement > StylesElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (_pImport->XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException( "illegal namespace!", Reference< XInterface >(), Any() );
    }
    // style
    else if ( rLocalName == "style" )
    {
        return new StyleElement( rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException( "expected style element!", Reference< XInterface >(), Any() );
    }
}

// window
Reference< xml::input::XElement > WindowElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    // event
    if (_pImport->isEventElement( nUid, rLocalName ))
    {
        return new EventElement( nUid, rLocalName, xAttributes, this, _pImport );
    }
    else if (_pImport->XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException( "illegal namespace!", Reference< XInterface >(), Any() );
    }
    // styles
    else if ( rLocalName == "styles" )
    {
        return new StylesElement( rLocalName, xAttributes, this, _pImport );
    }
    // bulletinboard
    else if ( rLocalName == "bulletinboard" )
    {
        return new BulletinBoardElement( rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException( "expected styles ot bulletinboard element!", Reference< XInterface >(), Any() );
    }
}

void WindowElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    Reference< beans::XPropertySet > xProps(
        _pImport->_xDialogModel, UNO_QUERY_THROW );
    ImportContext ctx( _pImport, xProps, getControlId( _xAttributes ) );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        pStyle->importBackgroundColorStyle( xProps );
        pStyle->importTextColorStyle( xProps );
        pStyle->importTextLineColorStyle( xProps );
        pStyle->importFontStyle( xProps );
    }

    ctx.importDefaults( 0, 0, _xAttributes, false );
    ctx.importBooleanProperty( "Closeable", "closeable", _xAttributes );
    ctx.importBooleanProperty( "Moveable", "moveable", _xAttributes );
    ctx.importBooleanProperty("Sizeable", "resizeable", _xAttributes );
    ctx.importStringProperty("Title", "title", _xAttributes );
    ctx.importBooleanProperty("Decoration", "withtitlebar", _xAttributes );
        ctx.importImageURLProperty( "ImageURL" , "image-src" , _xAttributes );
    ctx.importScollableSettings( _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
