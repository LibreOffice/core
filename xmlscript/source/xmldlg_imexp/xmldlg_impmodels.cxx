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
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/MalformedNumberFormatException.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>

#include <cppuhelper/exc_hlp.hxx>
#include <sal/log.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <i18nlangtag/languagetag.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace xmlscript
{

Reference< xml::input::XElement > Frame::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    if ( !m_xContainer.is() )
        m_xContainer.set( m_pImport->_xDialogModelFactory->createInstance( u"com.sun.star.awt.UnoFrameModel"_ustr ), UNO_QUERY );
    // event
    if (m_pImport->isEventElement( nUid, rLocalName ))
    {
       return new EventElement( nUid, rLocalName, xAttributes, this, m_pImport );
    }
    else if ( rLocalName == "bulletinboard" )
    {
        // Create new DialogImport for this container
        rtl::Reference<DialogImport> pFrameImport = new DialogImport( *m_pImport );
        pFrameImport->_xDialogModel = m_xContainer;
        return new BulletinBoardElement( rLocalName, xAttributes, this,  pFrameImport.get() );
    }
    else if ( rLocalName == "title" )
    {
        getStringAttr( &_label, u"value"_ustr, xAttributes, m_pImport->XMLNS_DIALOGS_UID );

        return new ElementBase( m_pImport->XMLNS_DIALOGS_UID, rLocalName, xAttributes, this, m_pImport );
    }
    else
    {
        SAL_INFO("xmlscript.xmldlg","****** ARGGGGG!!!! **********");
        throw     xml::sax::SAXException(u"expected event element!"_ustr, Reference< XInterface >(), Any() );
    }
}

void Frame::endElement()
{
    if ( !m_xContainer.is() )
            m_xContainer.set( m_pImport->_xDialogModelFactory->createInstance( u"com.sun.star.awt.UnoFrameModel"_ustr ), UNO_QUERY );
    Reference< beans::XPropertySet > xProps( m_xContainer, UNO_QUERY_THROW );
        // m_pImport is what we need to add to ( e.g. the dialog in this case )
    ControlImportContext ctx( m_pImport, xProps,   getControlId( _xAttributes ) );

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
        xControlModel->setPropertyValue( u"Label"_ustr , Any( _label ) );
    }
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();

    ctx.finish();
}


Reference< xml::input::XElement > MultiPage::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    // event
    if (m_pImport->isEventElement( nUid, rLocalName ))
    {
        return new EventElement(
            nUid, rLocalName, xAttributes, this, m_pImport );
    }
    else if ( rLocalName == "bulletinboard" )
    {
        // Create new DialogImport for this container

        rtl::Reference<DialogImport> pMultiPageImport = new DialogImport( *m_pImport );
        pMultiPageImport->_xDialogModel = m_xContainer;
        return new BulletinBoardElement( rLocalName, xAttributes, this,  pMultiPageImport.get() );
    }
    else
    {

        throw xml::sax::SAXException( u"expected event element!"_ustr, Reference< XInterface >(), Any() );
    }
}

void MultiPage::endElement()
{
    Reference< beans::XPropertySet > xProps( m_xContainer, UNO_QUERY_THROW );
        // m_pImport is what we need to add to ( e.g. the dialog in this case )
    ControlImportContext ctx( m_pImport, xProps, getControlId( _xAttributes ));

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
    ctx.importLongProperty(u"MultiPageValue"_ustr , u"value"_ustr,  _xAttributes );
    ctx.importBooleanProperty( u"Decoration"_ustr, u"withtabs"_ustr,  _xAttributes) ;
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();

    ctx.finish();
}

Reference< xml::input::XElement > Page::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    // event
    if (m_pImport->isEventElement( nUid, rLocalName ))
    {
        return new EventElement(
            nUid, rLocalName, xAttributes, this, m_pImport );
    }
    else if ( rLocalName == "bulletinboard" )
    {

        rtl::Reference<DialogImport> pPageImport = new DialogImport( *m_pImport );
        pPageImport->_xDialogModel = m_xContainer;
        return new BulletinBoardElement( rLocalName, xAttributes, this,  pPageImport.get() );
    }
    else
    {

        throw xml::sax::SAXException(u"expected event element!"_ustr, Reference< XInterface >(), Any() );
    }
}

void Page::endElement()
{
    Reference< beans::XPropertySet > xProps( m_xContainer, UNO_QUERY_THROW );

    ControlImportContext ctx( m_pImport, xProps, getControlId( _xAttributes ));

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
    ctx.importStringProperty( u"Title"_ustr, u"title"_ustr, _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();

    ctx.finish();
}

// progressmeter
Reference< xml::input::XElement > ProgressBarElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    // event
    if (!m_pImport->isEventElement( nUid, rLocalName ))
    {
        throw xml::sax::SAXException( u"expected event element!"_ustr, Reference< XInterface >(), Any() );
    }

    return new EventElement(
        nUid, rLocalName, xAttributes, this, m_pImport );
}

void ProgressBarElement::endElement()
{
    ControlImportContext ctx( m_pImport, getControlId( _xAttributes ), u"com.sun.star.awt.UnoControlProgressBarModel"_ustr );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        const Reference< beans::XPropertySet >& xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFillColorStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importLongProperty( u"ProgressValue"_ustr, u"value"_ustr, _xAttributes );
    ctx.importLongProperty( u"ProgressValueMin"_ustr, u"value-min"_ustr, _xAttributes );
    ctx.importLongProperty( u"ProgressValueMax"_ustr, u"value-max"_ustr, _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();

    ctx.finish();
}

// scrollbar
Reference< xml::input::XElement > ScrollBarElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    // event
    if (!m_pImport->isEventElement( nUid, rLocalName ))
    {
        throw xml::sax::SAXException(u"expected event element!"_ustr, Reference< XInterface >(), Any() );
    }

    return new EventElement( nUid, rLocalName, xAttributes, this, m_pImport );
}

void ScrollBarElement::endElement()
{
    ControlImportContext ctx( m_pImport, getControlId( _xAttributes ), getControlModelName( u"com.sun.star.awt.UnoControlScrollBarModel"_ustr , _xAttributes ) );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        const Reference< beans::XPropertySet >& xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importOrientationProperty( u"Orientation"_ustr , u"align"_ustr, _xAttributes );
    ctx.importLongProperty( u"BlockIncrement"_ustr , u"pageincrement"_ustr , _xAttributes );
    ctx.importLongProperty( u"LineIncrement"_ustr , u"increment"_ustr , _xAttributes );
    ctx.importLongProperty( u"ScrollValue"_ustr ,u"curpos"_ustr, _xAttributes );
    ctx.importLongProperty( u"ScrollValueMax"_ustr , u"maxpos"_ustr , _xAttributes );
    ctx.importLongProperty( u"ScrollValueMin"_ustr,u"minpos"_ustr, _xAttributes );
    ctx.importLongProperty( u"VisibleSize"_ustr, u"visible-size"_ustr, _xAttributes );
    ctx.importLongProperty( u"RepeatDelay"_ustr, u"repeat"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"Tabstop"_ustr, u"tabstop"_ustr , _xAttributes );
    ctx.importBooleanProperty( u"LiveScroll"_ustr, u"live-scroll"_ustr, _xAttributes );
    ctx.importHexLongProperty( u"SymbolColor"_ustr, u"symbol-color"_ustr, _xAttributes );

    ctx.importDataAwareProperty( u"linked-cell"_ustr , _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();

    ctx.finish();
}

// spinbutton
Reference< xml::input::XElement > SpinButtonElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    // event
    if (!m_pImport->isEventElement( nUid, rLocalName ))
    {
        throw xml::sax::SAXException(u"expected event element!"_ustr, Reference< XInterface >(), Any() );
    }

    return new EventElement( nUid, rLocalName, xAttributes, this, m_pImport );
}

void SpinButtonElement::endElement()
{
    ControlImportContext ctx( m_pImport, getControlId( _xAttributes ), getControlModelName( u"com.sun.star.awt.UnoControlSpinButtonModel"_ustr, _xAttributes ) );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        const Reference< beans::XPropertySet >& xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importOrientationProperty( u"Orientation"_ustr, u"align"_ustr, _xAttributes );
    ctx.importLongProperty(u"SpinIncrement"_ustr, u"increment"_ustr, _xAttributes );
    ctx.importLongProperty(u"SpinValue"_ustr, u"curval"_ustr ,_xAttributes );
    ctx.importLongProperty(u"SpinValueMax"_ustr, u"maxval"_ustr, _xAttributes );
    ctx.importLongProperty( u"SpinValueMin"_ustr,u"minval"_ustr,_xAttributes );
    ctx.importLongProperty( u"Repeat"_ustr, u"repeat"_ustr, _xAttributes );
    ctx.importLongProperty( u"RepeatDelay"_ustr, u"repeat-delay"_ustr,_xAttributes );
    ctx.importBooleanProperty( u"Tabstop"_ustr, u"tabstop"_ustr, _xAttributes );
    ctx.importHexLongProperty( u"SymbolColor"_ustr, u"symbol-color"_ustr , _xAttributes );
    ctx.importDataAwareProperty( u"linked-cell"_ustr , _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();

    ctx.finish();
}

// fixedline
Reference< xml::input::XElement > FixedLineElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    // event
    if (!m_pImport->isEventElement( nUid, rLocalName ))
    {
        throw xml::sax::SAXException(u"expected event element!"_ustr, Reference< XInterface >(), Any() );
    }

    return new EventElement( nUid, rLocalName, xAttributes, this, m_pImport );
}

void FixedLineElement::endElement()
{
    ControlImportContext ctx(m_pImport, getControlId( _xAttributes ), u"com.sun.star.awt.UnoControlFixedLineModel"_ustr );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        const Reference< beans::XPropertySet >& xControlModel( ctx.getControlModel() );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importTextLineColorStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importStringProperty( u"Label"_ustr, u"value"_ustr, _xAttributes );
    ctx.importOrientationProperty( u"Orientation"_ustr, u"align"_ustr, _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();

    ctx.finish();
}

// patternfield
Reference< xml::input::XElement > PatternFieldElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    // event
    if (!m_pImport->isEventElement( nUid, rLocalName ))
    {
        throw xml::sax::SAXException(u"expected event element!"_ustr, Reference< XInterface >(), Any() );
    }

    return new EventElement( nUid, rLocalName, xAttributes, this, m_pImport );
}

void PatternFieldElement::endElement()
{
    ControlImportContext ctx( m_pImport, getControlId( _xAttributes ), u"com.sun.star.awt.UnoControlPatternFieldModel"_ustr );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        const Reference< beans::XPropertySet >& xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importTextLineColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty( u"Tabstop"_ustr, u"tabstop"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"ReadOnly"_ustr, u"readonly"_ustr , _xAttributes );
    ctx.importBooleanProperty( u"StrictFormat"_ustr, u"strict-format"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"HideInactiveSelection"_ustr, u"hide-inactive-selection"_ustr, _xAttributes );
    ctx.importStringProperty( u"Text"_ustr, u"value"_ustr, _xAttributes );
    ctx.importShortProperty( u"MaxTextLen"_ustr, u"maxlength"_ustr, _xAttributes );
    ctx.importStringProperty( u"EditMask"_ustr, u"edit-mask"_ustr, _xAttributes );
    ctx.importStringProperty( u"LiteralMask"_ustr, u"literal-mask"_ustr, _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();

    ctx.finish();
}

// formattedfield
Reference< xml::input::XElement > FormattedFieldElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    // event
    if (!m_pImport->isEventElement( nUid, rLocalName ))
    {
        throw xml::sax::SAXException(u"expected event element!"_ustr, Reference< XInterface >(), Any() );
    }

    return new EventElement( nUid, rLocalName, xAttributes, this, m_pImport );
}

void FormattedFieldElement::endElement()
{
    ControlImportContext ctx( m_pImport, getControlId( _xAttributes ), u"com.sun.star.awt.UnoControlFormattedFieldModel"_ustr );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        const Reference< beans::XPropertySet >& xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importTextLineColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty( u"Tabstop"_ustr, u"tabstop"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"ReadOnly"_ustr, u"readonly"_ustr , _xAttributes );
    ctx.importBooleanProperty( u"StrictFormat"_ustr, u"strict-format"_ustr , _xAttributes );
    ctx.importBooleanProperty( u"HideInactiveSelection"_ustr, u"hide-inactive-selection"_ustr, _xAttributes );
    ctx.importAlignProperty( u"Align"_ustr , u"align"_ustr , _xAttributes );
    ctx.importDoubleProperty( u"EffectiveMin"_ustr, u"value-min"_ustr , _xAttributes );
    ctx.importDoubleProperty( u"EffectiveMax"_ustr, u"value-max"_ustr, _xAttributes);
    ctx.importDoubleProperty( u"EffectiveValue"_ustr, u"value"_ustr, _xAttributes );
    ctx.importStringProperty( u"Text"_ustr, u"text"_ustr, _xAttributes );
    ctx.importShortProperty( u"MaxTextLen"_ustr, u"maxlength"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"Spin"_ustr,  u"spin"_ustr, _xAttributes );
    if (ctx.importLongProperty( u"RepeatDelay"_ustr, u"repeat"_ustr, _xAttributes ))
        ctx.getControlModel()->setPropertyValue( u"Repeat"_ustr , Any(true) );

    OUString sDefault(_xAttributes->getValueByUidName(m_pImport->XMLNS_DIALOGS_UID, u"value-default"_ustr) );
    if (!sDefault.isEmpty())
    {
        double d = sDefault.toDouble();
        if (d != 0.0 || sDefault == "0" || sDefault == "0.0" )
        {
            ctx.getControlModel()->setPropertyValue( u"EffectiveDefault"_ustr, Any( d ) );
        }
        else // treat as string
        {
            ctx.getControlModel()->setPropertyValue( u"EffectiveDefault"_ustr, Any( sDefault ) );
        }
    }

    // format spec
    ctx.getControlModel()->setPropertyValue(u"FormatsSupplier"_ustr, Any( m_pImport->getNumberFormatsSupplier() ) );

    OUString sFormat( _xAttributes->getValueByUidName(m_pImport->XMLNS_DIALOGS_UID, u"format-code"_ustr ) );
    if (!sFormat.isEmpty())
    {
        lang::Locale locale;

        OUString sLocale( _xAttributes->getValueByUidName( m_pImport->XMLNS_DIALOGS_UID, u"format-locale"_ustr ) );
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
                m_pImport->getNumberFormatsSupplier()->getNumberFormats() );
            sal_Int32 nKey = xFormats->queryKey( sFormat, locale, true );
            if (-1 == nKey)
            {
                nKey = xFormats->addNew( sFormat, locale );
            }
            ctx.getControlModel()->setPropertyValue(u"FormatKey"_ustr, Any( nKey ) );
        }
        catch (const util::MalformedNumberFormatException & exc)
        {
            css::uno::Any anyEx = cppu::getCaughtException();
            SAL_WARN( "xmlscript.xmldlg", exceptionToString(anyEx) );
            // rethrow
            throw xml::sax::SAXException( exc.Message, Reference< XInterface >(), anyEx );
        }
    }
    ctx.importBooleanProperty(u"TreatAsNumber"_ustr, u"treat-as-number"_ustr , _xAttributes );
    ctx.importBooleanProperty(u"EnforceFormat"_ustr, u"enforce-format"_ustr, _xAttributes );

    ctx.importDataAwareProperty( u"linked-cell"_ustr , _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();

    ctx.finish();
}

// timefield
Reference< xml::input::XElement > TimeFieldElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    // event
    if (!m_pImport->isEventElement( nUid, rLocalName ))
    {
        throw xml::sax::SAXException(u"expected event element!"_ustr, Reference< XInterface >(), Any() );
    }

    return new EventElement( nUid, rLocalName, xAttributes, this, m_pImport );
}

void TimeFieldElement::endElement()
{
    ControlImportContext ctx( m_pImport, getControlId( _xAttributes ), u"com.sun.star.awt.UnoControlTimeFieldModel"_ustr );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        const Reference< beans::XPropertySet >& xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importTextLineColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty(u"Tabstop"_ustr, u"tabstop"_ustr, _xAttributes );
    ctx.importBooleanProperty(u"ReadOnly"_ustr, u"readonly"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"StrictFormat"_ustr, u"strict-format"_ustr, _xAttributes );
    ctx.importBooleanProperty(u"HideInactiveSelection"_ustr,u"hide-inactive-selection"_ustr, _xAttributes );
    ctx.importTimeFormatProperty( u"TimeFormat"_ustr, u"time-format"_ustr, _xAttributes );
    ctx.importTimeProperty( u"Time"_ustr, u"value"_ustr, _xAttributes );
    ctx.importTimeProperty( u"TimeMin"_ustr, u"value-min"_ustr, _xAttributes );
    ctx.importTimeProperty( u"TimeMax"_ustr, u"value-max"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"Spin"_ustr, u"spin"_ustr, _xAttributes );
    if (ctx.importLongProperty( u"RepeatDelay"_ustr, u"repeat"_ustr, _xAttributes ))
        ctx.getControlModel()->setPropertyValue(u"Repeat"_ustr, Any(true) );
    ctx.importStringProperty( u"Text"_ustr, u"text"_ustr , _xAttributes );
    ctx.importBooleanProperty( u"EnforceFormat"_ustr, u"enforce-format"_ustr , _xAttributes );

    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();

    ctx.finish();
}

// numericfield
Reference< xml::input::XElement > NumericFieldElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    // event
    if (!m_pImport->isEventElement( nUid, rLocalName ))
    {
        throw xml::sax::SAXException( u"expected event element!"_ustr, Reference< XInterface >(), Any() );
    }

    return new EventElement( nUid, rLocalName, xAttributes, this, m_pImport );
}

void NumericFieldElement::endElement()
{
    ControlImportContext ctx( m_pImport, getControlId( _xAttributes ), u"com.sun.star.awt.UnoControlNumericFieldModel"_ustr );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        const Reference< beans::XPropertySet >& xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importTextLineColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importAlignProperty( u"Align"_ustr, u"align"_ustr, _xAttributes );
    ctx.importVerticalAlignProperty( u"VerticalAlign"_ustr, u"valign"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"Tabstop"_ustr,u"tabstop"_ustr,_xAttributes );
    ctx.importBooleanProperty( u"ReadOnly"_ustr, u"readonly"_ustr,_xAttributes );
    ctx.importBooleanProperty( u"StrictFormat"_ustr, u"strict-format"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"HideInactiveSelection"_ustr, u"hide-inactive-selection"_ustr, _xAttributes );
    ctx.importShortProperty( u"DecimalAccuracy"_ustr, u"decimal-accuracy"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"ShowThousandsSeparator"_ustr, u"thousands-separator"_ustr, _xAttributes );
    ctx.importDoubleProperty( u"Value"_ustr, u"value"_ustr, _xAttributes );
    ctx.importDoubleProperty( u"ValueMin"_ustr, u"value-min"_ustr, _xAttributes );
    ctx.importDoubleProperty( u"ValueMax"_ustr, u"value-max"_ustr, _xAttributes );
    ctx.importDoubleProperty( u"ValueStep"_ustr, u"value-step"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"Spin"_ustr, u"spin"_ustr, _xAttributes );
    if (ctx.importLongProperty( u"RepeatDelay"_ustr, u"repeat"_ustr,  _xAttributes ))
        ctx.getControlModel()->setPropertyValue(u"Repeat"_ustr, Any(true) );
    ctx.importBooleanProperty( u"EnforceFormat"_ustr, u"enforce-format"_ustr, _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();

    ctx.finish();
}

// datefield
Reference< xml::input::XElement > DateFieldElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    // event
    if (!m_pImport->isEventElement( nUid, rLocalName ))
    {
        throw xml::sax::SAXException(u"expected event element!"_ustr, Reference< XInterface >(), Any() );
    }

    return new EventElement( nUid, rLocalName, xAttributes, this, m_pImport );
}

void DateFieldElement::endElement()
{
    ControlImportContext ctx( m_pImport, getControlId( _xAttributes ), u"com.sun.star.awt.UnoControlDateFieldModel"_ustr );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        const Reference< beans::XPropertySet >& xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importTextLineColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importAlignProperty( u"Align"_ustr, u"align"_ustr, _xAttributes );
    ctx.importVerticalAlignProperty( u"VerticalAlign"_ustr, u"valign"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"Tabstop"_ustr, u"tabstop"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"ReadOnly"_ustr, u"readonly"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"StrictFormat"_ustr, u"strict-format"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"HideInactiveSelection"_ustr, u"hide-inactive-selection"_ustr, _xAttributes );
    ctx.importDateFormatProperty( u"DateFormat"_ustr, u"date-format"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"DateShowCentury"_ustr, u"show-century"_ustr, _xAttributes );
    ctx.importDateProperty( u"Date"_ustr, u"value"_ustr, _xAttributes );
    ctx.importDateProperty( u"DateMin"_ustr, u"value-min"_ustr, _xAttributes );
    ctx.importDateProperty( u"DateMax"_ustr, u"value-max"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"Spin"_ustr, u"spin"_ustr, _xAttributes );
    if (ctx.importLongProperty( u"RepeatDelay"_ustr, u"repeat"_ustr, _xAttributes ))
        ctx.getControlModel()->setPropertyValue( u"Repeat"_ustr, Any(true) );
    ctx.importBooleanProperty( u"Dropdown"_ustr, u"dropdown"_ustr, _xAttributes );
    ctx.importStringProperty( u"Text"_ustr, u"text"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"EnforceFormat"_ustr, u"enforce-format"_ustr, _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();

    ctx.finish();
}

// currencyfield
Reference< xml::input::XElement > CurrencyFieldElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    // event
    if (!m_pImport->isEventElement( nUid, rLocalName ))
    {
        throw xml::sax::SAXException( u"expected event element!"_ustr , Reference< XInterface >(), Any() );
    }

    return new EventElement( nUid, rLocalName, xAttributes, this, m_pImport );
}

void CurrencyFieldElement::endElement()
{
    ControlImportContext ctx( m_pImport, getControlId( _xAttributes ), u"com.sun.star.awt.UnoControlCurrencyFieldModel"_ustr );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        const Reference< beans::XPropertySet >& xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importTextLineColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importAlignProperty( u"Align"_ustr, u"align"_ustr, _xAttributes );
    ctx.importVerticalAlignProperty( u"VerticalAlign"_ustr, u"valign"_ustr, _xAttributes );
    ctx.importBooleanProperty(u"Tabstop"_ustr, u"tabstop"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"ReadOnly"_ustr, u"readonly"_ustr , _xAttributes );
    ctx.importBooleanProperty( u"StrictFormat"_ustr, u"strict-format"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"HideInactiveSelection"_ustr, u"hide-inactive-selection"_ustr, _xAttributes );
    ctx.importStringProperty( u"CurrencySymbol"_ustr, u"currency-symbol"_ustr, _xAttributes );
    ctx.importShortProperty( u"DecimalAccuracy"_ustr, u"decimal-accuracy"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"ShowThousandsSeparator"_ustr, u"thousands-separator"_ustr, _xAttributes );
    ctx.importDoubleProperty( u"Value"_ustr, u"value"_ustr, _xAttributes );
    ctx.importDoubleProperty( u"ValueMin"_ustr, u"value-min"_ustr, _xAttributes );
    ctx.importDoubleProperty( u"ValueMax"_ustr, u"value-max"_ustr, _xAttributes );
    ctx.importDoubleProperty( u"ValueStep"_ustr, u"value-step"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"Spin"_ustr, u"spin"_ustr, _xAttributes );
    if (ctx.importLongProperty( u"RepeatDelay"_ustr, u"repeat"_ustr, _xAttributes ))
        ctx.getControlModel()->setPropertyValue( u"Repeat"_ustr, Any(true) );
    ctx.importBooleanProperty( u"PrependCurrencySymbol"_ustr, u"prepend-symbol"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"EnforceFormat"_ustr, u"enforce-format"_ustr, _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();

    ctx.finish();
}

// filecontrol
Reference< xml::input::XElement > FileControlElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    // event
    if (!m_pImport->isEventElement( nUid, rLocalName ))
    {
        throw xml::sax::SAXException( u"expected event element!"_ustr, Reference< XInterface >(), Any() );
    }

    return new EventElement( nUid, rLocalName, xAttributes, this, m_pImport );
}

void FileControlElement::endElement()
{
    ControlImportContext ctx( m_pImport, getControlId( _xAttributes ), u"com.sun.star.awt.UnoControlFileControlModel"_ustr );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        const Reference< beans::XPropertySet >& xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importTextLineColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty( u"Tabstop"_ustr, u"tabstop"_ustr, _xAttributes );
    ctx.importBooleanProperty(u"HideInactiveSelection"_ustr,u"hide-inactive-selection"_ustr, _xAttributes );
    ctx.importStringProperty( u"Text"_ustr, u"value"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"ReadOnly"_ustr, u"readonly"_ustr, _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();

    ctx.finish();
}

// treecontrol
Reference< xml::input::XElement > TreeControlElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    // event
    if (!m_pImport->isEventElement( nUid, rLocalName ))
    {
        throw xml::sax::SAXException( u"expected event element!"_ustr, Reference< XInterface >(), Any() );
    }

    return new EventElement( nUid, rLocalName, xAttributes, this, m_pImport );
}

void TreeControlElement::endElement()
{
    ControlImportContext ctx( m_pImport, getControlId( _xAttributes ), u"com.sun.star.awt.tree.TreeControlModel"_ustr );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        const Reference< beans::XPropertySet >& xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty( u"Tabstop"_ustr, u"tabstop"_ustr, _xAttributes );
    ctx.importSelectionTypeProperty( u"SelectionType"_ustr, u"selectiontype"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"RootDisplayed"_ustr, u"rootdisplayed"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"ShowsHandles"_ustr, u"showshandles"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"ShowsRootHandles"_ustr, u"showsroothandles"_ustr ,_xAttributes );
    ctx.importBooleanProperty( u"Editable"_ustr, u"editable"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"RowHeight"_ustr, u"readonly"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"InvokesStopNodeEditing"_ustr, u"invokesstopnodeediting"_ustr, _xAttributes );

    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();

    ctx.finish();
}

// imagecontrol
Reference< xml::input::XElement > ImageControlElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    // event
    if (!m_pImport->isEventElement( nUid, rLocalName ))
    {
        throw xml::sax::SAXException( u"expected event element!"_ustr , Reference< XInterface >(), Any() );
    }

    return new EventElement( nUid, rLocalName, xAttributes, this, m_pImport );
}

void ImageControlElement::endElement()
{
    ControlImportContext ctx( m_pImport, getControlId( _xAttributes ), u"com.sun.star.awt.UnoControlImageControlModel"_ustr );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        const Reference< beans::XPropertySet >& xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty( u"ScaleImage"_ustr, u"scale-image"_ustr, _xAttributes );
    ctx.importImageScaleModeProperty( u"ScaleMode"_ustr , u"scale-mode"_ustr , _xAttributes );
    ctx.importGraphicOrImageProperty(u"src"_ustr , _xAttributes);
    ctx.importBooleanProperty( u"Tabstop"_ustr, u"tabstop"_ustr, _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();

    ctx.finish();
}

// textfield
Reference< xml::input::XElement > TextElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    // event
    if (!m_pImport->isEventElement( nUid, rLocalName ))
    {
        throw xml::sax::SAXException( u"expected event element!"_ustr, Reference< XInterface >(), Any() );
    }

    return new EventElement( nUid, rLocalName, xAttributes, this, m_pImport );
}

void TextElement::endElement()
{
    ControlImportContext ctx( m_pImport, getControlId( _xAttributes ), u"com.sun.star.awt.UnoControlFixedTextModel"_ustr );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        const Reference< beans::XPropertySet >& xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importTextLineColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importStringProperty( u"Label"_ustr, u"value"_ustr, _xAttributes );
    ctx.importAlignProperty( u"Align"_ustr, u"align"_ustr, _xAttributes );
    ctx.importVerticalAlignProperty( u"VerticalAlign"_ustr, u"valign"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"MultiLine"_ustr, u"multiline"_ustr ,_xAttributes );
    ctx.importBooleanProperty( u"Tabstop"_ustr, u"tabstop"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"NoLabel"_ustr, u"nolabel"_ustr, _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();

    ctx.finish();
}

// FixedHyperLink
Reference< xml::input::XElement > FixedHyperLinkElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    // event
    if (!m_pImport->isEventElement( nUid, rLocalName ))
    {
        throw xml::sax::SAXException( u"expected event element!"_ustr , Reference< XInterface >(), Any() );
    }

    return new EventElement( nUid, rLocalName, xAttributes, this, m_pImport );
}

void FixedHyperLinkElement::endElement()
{
    ControlImportContext ctx( m_pImport, getControlId( _xAttributes ), u"com.sun.star.awt.UnoControlFixedHyperlinkModel"_ustr );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        const Reference< beans::XPropertySet >& xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importTextLineColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importStringProperty( u"Label"_ustr, u"value"_ustr, _xAttributes );
    ctx.importStringProperty( u"URL"_ustr, u"url"_ustr, _xAttributes );

    ctx.importAlignProperty( u"Align"_ustr, u"align"_ustr ,_xAttributes );
    ctx.importVerticalAlignProperty( u"VerticalAlign"_ustr, u"valign"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"MultiLine"_ustr, u"multiline"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"Tabstop"_ustr, u"tabstop"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"NoLabel"_ustr, u"nolabel"_ustr, _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();

    ctx.finish();
}

// edit
Reference< xml::input::XElement > TextFieldElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    // event
    if (!m_pImport->isEventElement( nUid, rLocalName ))
    {
        throw xml::sax::SAXException( u"expected event element!"_ustr, Reference< XInterface >(), Any() );
    }

    return new EventElement( nUid, rLocalName, xAttributes, this, m_pImport );
}

void TextFieldElement::endElement()
{
    ControlImportContext ctx( m_pImport, getControlId( _xAttributes ), u"com.sun.star.awt.UnoControlEditModel"_ustr );
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
    ctx.importBooleanProperty(u"Tabstop"_ustr, u"tabstop"_ustr, _xAttributes );
    ctx.importAlignProperty( u"Align"_ustr, u"align"_ustr, _xAttributes );
    ctx.importVerticalAlignProperty( u"VerticalAlign"_ustr, u"valign"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"HardLineBreaks"_ustr, u"hard-linebreaks"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"HScroll"_ustr, u"hscroll"_ustr ,_xAttributes );
    ctx.importBooleanProperty( u"VScroll"_ustr, u"vscroll"_ustr, _xAttributes );
    ctx.importBooleanProperty(u"HideInactiveSelection"_ustr, u"hide-inactive-selection"_ustr, _xAttributes );
    ctx.importShortProperty( u"MaxTextLen"_ustr, u"maxlength"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"MultiLine"_ustr, u"multiline"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"ReadOnly"_ustr, u"readonly"_ustr, _xAttributes );
    ctx.importStringProperty( u"Text"_ustr, u"value"_ustr, _xAttributes );
    ctx.importLineEndFormatProperty( u"LineEndFormat"_ustr, u"lineend-format"_ustr, _xAttributes );
    OUString aValue;
    if (getStringAttr( &aValue, u"echochar"_ustr, _xAttributes, m_pImport->XMLNS_DIALOGS_UID ) && !aValue.isEmpty() )
    {
        SAL_WARN_IF( aValue.getLength() != 1, "xmlscript.xmldlg", "### more than one character given for echochar!" );
        sal_Int16 nChar = 0;
        if(!aValue.isEmpty())
        {
            nChar = static_cast<sal_Int16>(aValue[ 0 ]);
        }
        xControlModel->setPropertyValue( u"EchoChar"_ustr, Any( nChar ) );
    }

    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();

    ctx.finish();
}

// titledbox
Reference< xml::input::XElement > TitledBoxElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    // event
    if (m_pImport->isEventElement( nUid, rLocalName ))
    {
        return new EventElement( nUid, rLocalName, xAttributes, this, m_pImport );
    }
    else if (m_pImport->XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException( u"illegal namespace!"_ustr, Reference< XInterface >(), Any() );
    }
    // title
    else if ( rLocalName == "title" )
    {
        getStringAttr( &_label, u"value"_ustr, xAttributes, m_pImport->XMLNS_DIALOGS_UID );

        return new ElementBase( m_pImport->XMLNS_DIALOGS_UID, rLocalName, xAttributes, this, m_pImport );
    }
    // radio
    else if ( rLocalName == "radio" )
    {
        // don't create radios here, => titledbox must be inserted first due to radio grouping,
        // possible predecessors!
        Reference< xml::input::XElement > xRet(
            new RadioElement( rLocalName, xAttributes, this, m_pImport ) );
        _radios.push_back( xRet );
        return xRet;
    }
    else
    {
        return BulletinBoardElement::startChildElement( nUid, rLocalName, xAttributes );
    }
}

void TitledBoxElement::endElement()
{
    {
        ControlImportContext ctx(m_pImport, getControlId( _xAttributes ), u"com.sun.star.awt.UnoControlGroupBoxModel"_ustr );
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
            xControlModel->setPropertyValue( u"Label"_ustr, Any( _label ) );
        }

        ctx.importEvents( _events );
        // avoid ring-reference:
        // vector< event elements > holding event elements holding this (via _pParent)
        _events.clear();

        ctx.finish();
    }

    // create radios AFTER group box!
    for (Reference<XElement>& xRadio : _radios)
    {
        Reference< xml::input::XAttributes > xAttributes( xRadio->getAttributes() );

        ControlImportContext ctx( m_pImport, getControlId( xAttributes ), getControlModelName( u"com.sun.star.awt.UnoControlRadioButtonModel"_ustr, xAttributes ) );
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
        ctx.importBooleanProperty( u"Tabstop"_ustr, u"tabstop"_ustr, xAttributes );
        ctx.importStringProperty( u"Label"_ustr, u"value"_ustr, xAttributes );
        ctx.importAlignProperty( u"Align"_ustr, u"align"_ustr, xAttributes );
        ctx.importVerticalAlignProperty( u"VerticalAlign"_ustr, u"valign"_ustr, xAttributes );
        ctx.importGraphicOrImageProperty(u"image-src"_ustr , _xAttributes);
        ctx.importImagePositionProperty( u"ImagePosition"_ustr, u"image-position"_ustr, xAttributes );
        ctx.importBooleanProperty( u"MultiLine"_ustr, u"multiline"_ustr, xAttributes );
        ctx.importStringProperty( u"GroupName"_ustr, u"group-name"_ustr, xAttributes );

        sal_Int16 nVal = 0;
        sal_Bool bChecked = false;
        if (getBoolAttr( &bChecked, u"checked"_ustr, xAttributes, m_pImport->XMLNS_DIALOGS_UID ) && bChecked)
        {
            nVal = 1;
        }
        xControlModel->setPropertyValue( u"State"_ustr, Any( nVal ) );
        ctx.importDataAwareProperty( u"linked-cell"_ustr , xAttributes );
        ::std::vector< Reference< xml::input::XElement > > & radioEvents =
            static_cast< RadioElement * >( xRadio.get() )->getEvents();
        ctx.importEvents( radioEvents );
        // avoid ring-reference:
        // vector< event elements > holding event elements holding this (via _pParent)
        radioEvents.clear();

        ctx.finish();
    }
    // avoid ring-reference:
    // vector< radio elements > holding radio elements holding this (via _pParent)
    _radios.clear();
}

// radio
Reference< xml::input::XElement > RadioElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    // event
    if (!m_pImport->isEventElement( nUid, rLocalName ))
    {
        throw xml::sax::SAXException(u"expected event element!"_ustr, Reference< XInterface >(), Any() );
    }

    return new EventElement( nUid, rLocalName, xAttributes, this, m_pImport );
}

// radiogroup
Reference< xml::input::XElement > RadioGroupElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    if (m_pImport->XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException( u"illegal namespace!"_ustr, Reference< XInterface >(), Any() );
    }
    // radio
    else if ( rLocalName == "radio" )
    {
        // don't create radios here, => titledbox must be inserted first due to radio grouping,
        // possible predecessors!
        Reference< xml::input::XElement > xRet(
            new RadioElement( rLocalName, xAttributes, this, m_pImport ) );
        _radios.push_back( xRet );
        return xRet;
    }
    else
    {
        throw xml::sax::SAXException( u"expected radio element!"_ustr, Reference< XInterface >(), Any() );
    }
}
void RadioGroupElement::endElement()
{
    for (Reference<XElement>& xRadio : _radios)
    {
        Reference< xml::input::XAttributes > xAttributes(
            xRadio->getAttributes() );

        ControlImportContext ctx( m_pImport, getControlId( xAttributes ), getControlModelName( u"com.sun.star.awt.UnoControlRadioButtonModel"_ustr, xAttributes ) );
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
        ctx.importBooleanProperty(u"Tabstop"_ustr, u"tabstop"_ustr, xAttributes );
        ctx.importStringProperty( u"Label"_ustr, u"value"_ustr, xAttributes );
        ctx.importAlignProperty( u"Align"_ustr, u"align"_ustr, xAttributes );
        ctx.importVerticalAlignProperty( u"VerticalAlign"_ustr, u"valign"_ustr, xAttributes );
        ctx.importGraphicOrImageProperty(u"image-src"_ustr , _xAttributes);
        ctx.importImagePositionProperty( u"ImagePosition"_ustr, u"image-position"_ustr, xAttributes );
        ctx.importBooleanProperty( u"MultiLine"_ustr, u"multiline"_ustr, xAttributes );
        ctx.importStringProperty( u"GroupName"_ustr, u"group-name"_ustr, xAttributes );
        sal_Int16 nVal = 0;
        sal_Bool bChecked = false;
        if (getBoolAttr( &bChecked, u"checked"_ustr, xAttributes, m_pImport->XMLNS_DIALOGS_UID ) && bChecked)
        {
            nVal = 1;
        }
        xControlModel->setPropertyValue( u"State"_ustr, Any( nVal ) );

        ctx.importDataAwareProperty( u"linked-cell"_ustr, xAttributes );

        ::std::vector< Reference< xml::input::XElement > > & radioEvents =
            static_cast< RadioElement * >( xRadio.get() )->getEvents();
        ctx.importEvents( radioEvents );
        // avoid ring-reference:
        // vector< event elements > holding event elements holding this (via _pParent)
        radioEvents.clear();

        ctx.finish();
    }
    // avoid ring-reference:
    // vector< radio elements > holding radio elements holding this (via _pParent)
    _radios.clear();
}

// menupopup
Reference< xml::input::XElement > MenuPopupElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    if (m_pImport->XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException( u"illegal namespace!"_ustr, Reference< XInterface >(), Any() );
    }
    // menuitem
    else if ( rLocalName == "menuitem" )
    {
        OUString aValue( xAttributes->getValueByUidName( m_pImport->XMLNS_DIALOGS_UID,u"value"_ustr ) );
        SAL_WARN_IF( aValue.isEmpty() && !_allowEmptyItems, "xmlscript.xmldlg", "### menuitem has no value?" );
        if ((!aValue.isEmpty()) || _allowEmptyItems)
        {
            _itemValues.push_back( aValue );

            OUString aSel( xAttributes->getValueByUidName( m_pImport->XMLNS_DIALOGS_UID, u"selected"_ustr ) );
            if (!aSel.isEmpty() && aSel == "true")
            {
                _itemSelected.push_back( static_cast<sal_Int16>(_itemValues.size()) -1 );
            }
        }
        return new ElementBase( m_pImport->XMLNS_DIALOGS_UID, rLocalName, xAttributes, this, m_pImport );
    }
    else
    {
        throw xml::sax::SAXException(u"expected menuitem!"_ustr , Reference< XInterface >(), Any() );
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
{
    // event
    if (m_pImport->isEventElement( nUid, rLocalName ))
    {
        return new EventElement( nUid, rLocalName, xAttributes, this, m_pImport );
    }
    else if (m_pImport->XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException( u"illegal namespace!"_ustr, Reference< XInterface >(), Any() );
    }
    // menupopup
    else if ( rLocalName == "menupopup" )
    {
        _popup = new MenuPopupElement( rLocalName, xAttributes, this, m_pImport, false );
        return _popup;
    }
    else
    {
        throw xml::sax::SAXException( u"expected event or menupopup element!"_ustr, Reference< XInterface >(), Any() );
    }
}

void MenuListElement::endElement()
{
    ControlImportContext ctx( m_pImport, getControlId( _xAttributes ), getControlModelName( u"com.sun.star.awt.UnoControlListBoxModel"_ustr, _xAttributes  ) );
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
    ctx.importBooleanProperty( u"Tabstop"_ustr, u"tabstop"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"MultiSelection"_ustr, u"multiselection"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"ReadOnly"_ustr, u"readonly"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"Dropdown"_ustr, u"spin"_ustr, _xAttributes );
    ctx.importShortProperty( u"LineCount"_ustr, u"linecount"_ustr, _xAttributes );
    ctx.importAlignProperty( u"Align"_ustr, u"align"_ustr, _xAttributes );
    bool bHasLinkedCell = ctx.importDataAwareProperty( u"linked-cell"_ustr , _xAttributes );
    bool bHasSrcRange = ctx.importDataAwareProperty( u"source-cell-range"_ustr , _xAttributes );
    if (_popup.is())
    {
        if ( !bHasSrcRange )
            xControlModel->setPropertyValue( u"StringItemList"_ustr, Any( _popup->getItemValues() ) );
        if ( !bHasLinkedCell )
            xControlModel->setPropertyValue( u"SelectedItems"_ustr, Any( _popup->getSelectedItems() ) );

    }
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();

    ctx.finish();
}

// combobox
Reference< xml::input::XElement > ComboBoxElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    // event
    if (m_pImport->isEventElement( nUid, rLocalName ))
    {
        return new EventElement( nUid, rLocalName, xAttributes, this, m_pImport );
    }
    else if (m_pImport->XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException( u"illegal namespace!"_ustr, Reference< XInterface >(), Any() );
    }
    // menupopup
    else if ( rLocalName == "menupopup" )
    {
        _popup = new MenuPopupElement( rLocalName, xAttributes, this, m_pImport, true );
        return _popup;
    }
    else
    {
        throw xml::sax::SAXException( u"expected event or menupopup element!"_ustr, Reference< XInterface >(), Any() );
    }
}
void ComboBoxElement::endElement()
{
    ControlImportContext ctx( m_pImport, getControlId( _xAttributes ), getControlModelName( u"com.sun.star.awt.UnoControlComboBoxModel"_ustr, _xAttributes ) );
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
    ctx.importBooleanProperty( u"Tabstop"_ustr, u"tabstop"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"ReadOnly"_ustr, u"readonly"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"Autocomplete"_ustr, u"autocomplete"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"Dropdown"_ustr, u"spin"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"HideInactiveSelection"_ustr, u"hide-inactive-selection"_ustr, _xAttributes );
    ctx.importShortProperty( u"MaxTextLen"_ustr, u"maxlength"_ustr ,_xAttributes );
    ctx.importShortProperty( u"LineCount"_ustr, u"linecount"_ustr ,_xAttributes );
    ctx.importStringProperty( u"Text"_ustr, u"value"_ustr, _xAttributes );
    ctx.importAlignProperty( u"Align"_ustr, u"align"_ustr, _xAttributes );
    ctx.importDataAwareProperty( u"linked-cell"_ustr , _xAttributes );
    bool bHasSrcRange = ctx.importDataAwareProperty( u"source-cell-range"_ustr , _xAttributes );
    if (_popup.is() && !bHasSrcRange )
    {
        xControlModel->setPropertyValue( u"StringItemList"_ustr, Any( _popup->getItemValues() ) );
    }

    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();

    ctx.finish();
}

// checkbox
Reference< xml::input::XElement > CheckBoxElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    // event
    if (!m_pImport->isEventElement( nUid, rLocalName ))
    {
        throw xml::sax::SAXException( u"expected event element!"_ustr, Reference< XInterface >(), Any() );
    }

    return new EventElement( nUid, rLocalName, xAttributes, this, m_pImport );
}

void CheckBoxElement::endElement()
{
    ControlImportContext ctx( m_pImport, getControlId( _xAttributes ), u"com.sun.star.awt.UnoControlCheckBoxModel"_ustr );
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
    ctx.importBooleanProperty( u"Tabstop"_ustr, u"tabstop"_ustr, _xAttributes );
    ctx.importStringProperty( u"Label"_ustr, u"value"_ustr, _xAttributes );
    ctx.importAlignProperty( u"Align"_ustr, u"align"_ustr, _xAttributes );
    ctx.importVerticalAlignProperty( u"VerticalAlign"_ustr, u"valign"_ustr, _xAttributes );
    ctx.importGraphicOrImageProperty(u"image-src"_ustr , _xAttributes);
    ctx.importImagePositionProperty( u"ImagePosition"_ustr, u"image-position"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"MultiLine"_ustr, u"multiline"_ustr, _xAttributes );

    sal_Bool bTriState = false;
    if (getBoolAttr( &bTriState, u"tristate"_ustr, _xAttributes, m_pImport->XMLNS_DIALOGS_UID ))
    {
        xControlModel->setPropertyValue( u"TriState"_ustr, Any( bTriState ) );
    }
    sal_Bool bChecked = false;
    if (getBoolAttr( &bChecked, u"checked"_ustr, _xAttributes, m_pImport->XMLNS_DIALOGS_UID ))
    {
        // has "checked" attribute
        sal_Int16 nVal = (bChecked ? 1 : 0);
        xControlModel->setPropertyValue( u"State"_ustr, Any( nVal ) );
    }
    else
    {
        sal_Int16 nVal = (bTriState ? 2 : 0); // if tristate set, but checked omitted => don't know!
        xControlModel->setPropertyValue( u"State"_ustr, Any( nVal ) );
    }

    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();

    ctx.finish();
}

// button
Reference< xml::input::XElement > ButtonElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    // event
    if (!m_pImport->isEventElement( nUid, rLocalName ))
    {
        throw xml::sax::SAXException( u"expected event element!"_ustr,  Reference< XInterface >(), Any() );
    }

    return new EventElement( nUid, rLocalName, xAttributes, this, m_pImport );
}

void ButtonElement::endElement()
{
    ControlImportContext ctx( m_pImport, getControlId( _xAttributes ), u"com.sun.star.awt.UnoControlButtonModel"_ustr );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        const Reference< beans::XPropertySet >& xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importTextLineColorStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty( u"Tabstop"_ustr, u"tabstop"_ustr, _xAttributes );
    ctx.importStringProperty( u"Label"_ustr, u"value"_ustr, _xAttributes );
    ctx.importAlignProperty( u"Align"_ustr, u"align"_ustr, _xAttributes );
    ctx.importVerticalAlignProperty( u"VerticalAlign"_ustr, u"valign"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"DefaultButton"_ustr, u"default"_ustr, _xAttributes );
    ctx.importButtonTypeProperty( u"PushButtonType"_ustr, u"button-type"_ustr, _xAttributes );
    ctx.importGraphicOrImageProperty(u"image-src"_ustr , _xAttributes);
    ctx.importImagePositionProperty( u"ImagePosition"_ustr, u"image-position"_ustr, _xAttributes );
    ctx.importImageAlignProperty( u"ImageAlign"_ustr, u"image-align"_ustr, _xAttributes );
    if (ctx.importLongProperty( u"RepeatDelay"_ustr, u"repeat"_ustr, _xAttributes ))
        ctx.getControlModel()->setPropertyValue( u"Repeat"_ustr, Any(true) );
    sal_Int32 toggled = 0;
    if (getLongAttr( &toggled, u"toggled"_ustr, _xAttributes, m_pImport->XMLNS_DIALOGS_UID ) && toggled == 1)
        ctx.getControlModel()->setPropertyValue( u"Toggle"_ustr , Any(true));
    ctx.importBooleanProperty( u"FocusOnClick"_ustr, u"grab-focus"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"MultiLine"_ustr, u"multiline"_ustr, _xAttributes );
    // State
    sal_Bool bChecked = false;
    if (getBoolAttr( &bChecked, u"checked"_ustr, _xAttributes, m_pImport->XMLNS_DIALOGS_UID ) && bChecked)
    {
        ctx.getControlModel()->setPropertyValue( u"State"_ustr , Any( sal_Int16(1) ) );
    }

    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();

    ctx.finish();
}

// bulletinboard
Reference< xml::input::XElement > BulletinBoardElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    if (m_pImport->XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException(u"illegal namespace!"_ustr, Reference< XInterface >(), Any() );
    }
    // button
    else if ( rLocalName == "button" )
    {
        return new ButtonElement( rLocalName, xAttributes, this, m_pImport );
    }
    // checkbox
    else if ( rLocalName == "checkbox" )
    {
        return new CheckBoxElement( rLocalName, xAttributes, this, m_pImport );
    }
    // combobox
    else if ( rLocalName == "combobox" )
    {
        return new ComboBoxElement( rLocalName, xAttributes, this, m_pImport );
    }
    // listbox
    else if ( rLocalName == "menulist" )
    {
        return new MenuListElement( rLocalName, xAttributes, this, m_pImport );
    }
    // radiogroup
    else if ( rLocalName == "radiogroup" )
    {
        return new RadioGroupElement( rLocalName, xAttributes, this, m_pImport );
    }
    // titledbox
    else if ( rLocalName == "titledbox" )
    {
        return new TitledBoxElement( rLocalName, xAttributes, this, m_pImport );
    }
    // text
    else if ( rLocalName == "text" )
    {
        return new TextElement( rLocalName, xAttributes, this, m_pImport );
    }
    else if ( rLocalName == "linklabel" )
    {
        return new FixedHyperLinkElement( rLocalName, xAttributes, this, m_pImport );
    }
    // textfield
    else if ( rLocalName == "textfield" )
    {
        return new TextFieldElement( rLocalName, xAttributes, this, m_pImport );
    }
    // img
    else if ( rLocalName == "img" )
    {
        return new ImageControlElement( rLocalName, xAttributes, this, m_pImport );
    }
    // filecontrol
    else if ( rLocalName == "filecontrol" )
    {
        return new FileControlElement( rLocalName, xAttributes, this, m_pImport );
    }
    // treecontrol
    else if ( rLocalName == "treecontrol" )
    {
        return new TreeControlElement( rLocalName, xAttributes, this, m_pImport );
    }
    // currencyfield
    else if ( rLocalName == "currencyfield" )
    {
        return new CurrencyFieldElement( rLocalName, xAttributes, this, m_pImport );
    }
    // datefield
    else if ( rLocalName == "datefield" )
    {
        return new DateFieldElement( rLocalName, xAttributes, this, m_pImport );
    }
    // datefield
    else if ( rLocalName == "numericfield" )
    {
        return new NumericFieldElement( rLocalName, xAttributes, this, m_pImport );
    }
    // timefield
    else if ( rLocalName == "timefield" )
    {
        return new TimeFieldElement( rLocalName, xAttributes, this, m_pImport );
    }
    // patternfield
    else if ( rLocalName == "patternfield" )
    {
        return new PatternFieldElement( rLocalName, xAttributes, this, m_pImport );
    }
    // formattedfield
    else if ( rLocalName == "formattedfield" )
    {
        return new FormattedFieldElement( rLocalName, xAttributes, this, m_pImport );
    }
    // fixedline
    else if ( rLocalName == "fixedline" )
    {
        return new FixedLineElement( rLocalName, xAttributes, this, m_pImport );
    }
    // scrollbar
    else if ( rLocalName == "scrollbar" )
    {
        return new ScrollBarElement( rLocalName, xAttributes, this, m_pImport );
    }
    // spinbutton
    else if ( rLocalName == "spinbutton" )
    {
        return new SpinButtonElement( rLocalName, xAttributes, this, m_pImport );
    }
    // progressmeter
    else if ( rLocalName == "progressmeter" )
    {
        return new ProgressBarElement( rLocalName, xAttributes, this, m_pImport );
    }
    // table
    else if (rLocalName == "table")
    {
        return new GridControlElement( rLocalName, xAttributes, this, m_pImport );
    }
    else if ( rLocalName == "multipage" )
    {
        return new MultiPage( rLocalName, xAttributes, this, m_pImport );
    }
    else if ( rLocalName == "frame" )
    {
        return new Frame( rLocalName, xAttributes, this, m_pImport );
    }
    else if ( rLocalName == "page" )
    {
        return new Page( rLocalName, xAttributes, this, m_pImport );
    }
    // bulletinboard
    else if ( rLocalName == "bulletinboard" )
    {
        return new BulletinBoardElement( rLocalName, xAttributes, this, m_pImport );
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
    : ControlElement( rLocalName, xAttributes, pParent, pImport ),
      mxDialogImport(pImport)
{
    OUString aValue( _xAttributes->getValueByUidName( m_pImport->XMLNS_DIALOGS_UID, u"left"_ustr ) );
    if (!aValue.isEmpty())
    {
        _nBasePosX += toInt32( aValue );
    }
    aValue = _xAttributes->getValueByUidName( m_pImport->XMLNS_DIALOGS_UID, u"top"_ustr );
    if (!aValue.isEmpty())
    {
        _nBasePosY += toInt32( aValue );
    }
}

// style
Reference< xml::input::XElement > StyleElement::startChildElement(
    sal_Int32 /*nUid*/, OUString const & /*rLocalName*/,
    Reference< xml::input::XAttributes > const & /*xAttributes*/ )
{
    throw xml::sax::SAXException( u"unexpected sub elements of style!"_ustr, Reference< XInterface >(), Any() );
}

void StyleElement::endElement()
{
    OUString aStyleId( _xAttributes->getValueByUidName( m_pImport->XMLNS_DIALOGS_UID, u"style-id"_ustr ) );
    if (aStyleId.isEmpty())
    {
        throw xml::sax::SAXException( u"missing style-id attribute!"_ustr, Reference< XInterface >(), Any() );
    }

    m_pImport->addStyle( aStyleId, this );
}

// styles
Reference< xml::input::XElement > StylesElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    if (m_pImport->XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException( u"illegal namespace!"_ustr, Reference< XInterface >(), Any() );
    }
    // style
    else if ( rLocalName == "style" )
    {
        return new StyleElement( rLocalName, xAttributes, this, m_pImport );
    }
    else
    {
        throw xml::sax::SAXException( u"expected style element!"_ustr, Reference< XInterface >(), Any() );
    }
}

// window
Reference< xml::input::XElement > WindowElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    // event
    if (m_pImport->isEventElement( nUid, rLocalName ))
    {
        return new EventElement( nUid, rLocalName, xAttributes, this, m_pImport );
    }
    else if (m_pImport->XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException( u"illegal namespace!"_ustr, Reference< XInterface >(), Any() );
    }
    // styles
    else if ( rLocalName == "styles" )
    {
        return new StylesElement( rLocalName, xAttributes, this, m_pImport );
    }
    // bulletinboard
    else if ( rLocalName == "bulletinboard" )
    {
        return new BulletinBoardElement( rLocalName, xAttributes, this, m_pImport );
    }
    else
    {
        throw xml::sax::SAXException( u"expected styles or bulletinboard element!"_ustr, Reference< XInterface >(), Any() );
    }
}

void WindowElement::endElement()
{
    Reference< beans::XPropertySet > xProps(
        m_pImport->_xDialogModel, UNO_QUERY_THROW );
    ImportContext ctx( m_pImport, xProps, getControlId( _xAttributes ) );

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
    ctx.importBooleanProperty( u"Closeable"_ustr, u"closeable"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"Moveable"_ustr, u"moveable"_ustr, _xAttributes );
    ctx.importBooleanProperty(u"Sizeable"_ustr, u"resizeable"_ustr, _xAttributes );
    ctx.importStringProperty(u"Title"_ustr, u"title"_ustr, _xAttributes );
    ctx.importBooleanProperty(u"Decoration"_ustr, u"withtitlebar"_ustr, _xAttributes );
    ctx.importGraphicOrImageProperty(u"image-src"_ustr , _xAttributes);
    ctx.importScollableSettings( _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

// table
Reference< xml::input::XElement > GridControlElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    // event
    if (!m_pImport->isEventElement( nUid, rLocalName ))
    {
        throw xml::sax::SAXException( u"expected event element!"_ustr, Reference< XInterface >(), Any() );
    }

    return new EventElement( nUid, rLocalName, xAttributes, this, m_pImport );

}

void GridControlElement::endElement()
{
    ControlImportContext ctx( m_pImport, getControlId( _xAttributes ), u"com.sun.star.awt.grid.UnoControlGridModel"_ustr);
    Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importTextLineColorStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }
    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty( u"Tabstop"_ustr, u"tabstop"_ustr, _xAttributes );
    ctx.importVerticalAlignProperty( u"VerticalAlign"_ustr, u"valign"_ustr, _xAttributes );
    ctx.importSelectionTypeProperty( u"SelectionModel"_ustr, u"selectiontype"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"ShowColumnHeader"_ustr, u"showcolumnheader"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"ShowRowHeader"_ustr, u"showrowheader"_ustr, _xAttributes );
    ctx.importHexLongProperty( u"GridLineColor"_ustr, u"gridline-color"_ustr, _xAttributes );
    ctx.importBooleanProperty( u"UseGridLines"_ustr, u"usegridlines"_ustr, _xAttributes );
    ctx.importHexLongProperty( u"HeaderBackgroundColor"_ustr, u"headerbackground-color"_ustr, _xAttributes );
    ctx.importHexLongProperty( u"HeaderTextColor"_ustr, u"headertext-color"_ustr, _xAttributes );
    ctx.importHexLongProperty( u"ActiveSelectionBackgroundColor"_ustr, u"activeselectionbackground-color"_ustr, _xAttributes );
    ctx.importHexLongProperty( u"ActiveSelectionTextColor"_ustr, u"activeselectiontext-color"_ustr, _xAttributes );
    ctx.importHexLongProperty( u"InactiveSelectionBackgroundColor"_ustr, u"inactiveselectionbackground-color"_ustr, _xAttributes );
    ctx.importHexLongProperty( u"InactiveSelectionTextColor"_ustr, u"inactiveselectiontext-color"_ustr, _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();

    ctx.finish();
}

//##################################################################################################

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
