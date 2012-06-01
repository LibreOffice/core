/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <comphelper/componentcontext.hxx>
#include <comphelper/processfactory.hxx>
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

namespace xmlscript
{

Reference< xml::input::XElement > Frame::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException)
{
    if ( !m_xContainer.is() )
        m_xContainer.set( _pImport->_xDialogModelFactory->createInstance( rtl::OUString( "com.sun.star.awt.UnoFrameModel" ) ), UNO_QUERY );
    // event
    if (_pImport->isEventElement( nUid, rLocalName ))
    {
       return new EventElement(
            nUid, rLocalName, xAttributes, this, _pImport );
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
        getStringAttr( &_label,
            OUString( "value" ),
            xAttributes,
            _pImport->XMLNS_DIALOGS_UID );

        return new ElementBase(
            _pImport->XMLNS_DIALOGS_UID,
            rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        OSL_TRACE("****** ARGGGGG!!!! **********");
        throw     xml::sax::SAXException(
            OUString( "expected event element!" ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________

void Frame::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
        if ( !m_xContainer.is() )
            m_xContainer.set( _pImport->_xDialogModelFactory->createInstance( rtl::OUString( "com.sun.star.awt.UnoFrameModel" ) ), UNO_QUERY );
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
        xControlModel->setPropertyValue( OUString( "Label" ),
                                         makeAny( _label ) );
    }
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

        throw xml::sax::SAXException(
            OUString( "expected event element!" ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________

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
    ctx.importLongProperty( OUString( "MultiPageValue" ),
                            OUString( "value" ),
                            _xAttributes );
        ctx.importBooleanProperty(
            OUString( "Decoration" ),
            OUString( "withtabs" ),
        _xAttributes );
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

        throw xml::sax::SAXException(
            OUString( "expected event element!" ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________

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
//__________________________________________________________________________________________________
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
//__________________________________________________________________________________________________
void ProgressBarElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( "com.sun.star.awt.UnoControlProgressBarModel" ) );

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
    ctx.importLongProperty( OUString( "ProgressValue" ),
                            OUString( "value" ),
                            _xAttributes );
    ctx.importLongProperty( OUString( "ProgressValueMin" ),
                            OUString( "value-min" ),
                            _xAttributes );
    ctx.importLongProperty( OUString( "ProgressValueMax" ),
                            OUString( "value-max" ),
                            _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

//##################################################################################################

// scrollbar
//__________________________________________________________________________________________________
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
        throw xml::sax::SAXException(
            OUString( "expected event element!" ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void ScrollBarElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        getControlModelName( OUString( "com.sun.star.awt.UnoControlScrollBarModel" ), _xAttributes ) );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importOrientationProperty( OUString( "Orientation" ),
                                   OUString( "align" ),
                                   _xAttributes );
    ctx.importLongProperty( OUString( "BlockIncrement" ),
                            OUString( "pageincrement" ),
                            _xAttributes );
    ctx.importLongProperty( OUString( "LineIncrement" ),
                            OUString( "increment" ),
                            _xAttributes );
    ctx.importLongProperty( OUString( "ScrollValue" ),
                            OUString( "curpos" ),
                            _xAttributes );
    ctx.importLongProperty( OUString( "ScrollValueMax" ),
                            OUString( "maxpos" ),
                            _xAttributes );
    ctx.importLongProperty( OUSTR("ScrollValueMin"), OUSTR("minpos"),
                            _xAttributes );
    ctx.importLongProperty( OUString( "VisibleSize" ),
                            OUString( "visible-size" ),
                            _xAttributes );
    ctx.importLongProperty( OUSTR("RepeatDelay"), OUSTR("repeat"),
                            _xAttributes );
    ctx.importBooleanProperty( OUString( "Tabstop" ),
                               OUString( "tabstop" ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "LiveScroll" ),
                               OUString( "live-scroll" ),
                               _xAttributes );
    ctx.importHexLongProperty( OUString( "SymbolColor" ),
                               OUString( "symbol-color" ),
                               _xAttributes );

    ctx.importDataAwareProperty( OUSTR("linked-cell" ), _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

//##################################################################################################

// spinbutton
//__________________________________________________________________________________________________
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
        throw xml::sax::SAXException(
            OUString( "expected event element!" ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void SpinButtonElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
                            _pImport, getControlId( _xAttributes ),
                            getControlModelName( OUString( "com.sun.star.awt.UnoControlSpinButtonModel" ), _xAttributes ) );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importOrientationProperty( OUString( "Orientation" ),
    OUString( "align" ),
         _xAttributes );
    ctx.importLongProperty( OUString( "SpinIncrement" ),
        OUString( "increment" ),
        _xAttributes );
    ctx.importLongProperty( OUString( "SpinValue" ),
    OUString( "curval" ),_xAttributes );
    ctx.importLongProperty( OUString( "SpinValueMax" ),
    OUString( "maxval" ), _xAttributes );
    ctx.importLongProperty( OUSTR("SpinValueMin"), OUSTR("minval"),
        _xAttributes );
    ctx.importLongProperty( OUSTR("Repeat"), OUSTR("repeat"), _xAttributes );
    ctx.importLongProperty( OUSTR("RepeatDelay"), OUSTR("repeat-delay"),
_xAttributes );
    ctx.importBooleanProperty( OUString( "Tabstop" ),
    OUString( "tabstop" ), _xAttributes );
    ctx.importHexLongProperty( OUString( "SymbolColor" ),
    OUString( "symbol-color" ), _xAttributes );
    ctx.importDataAwareProperty( OUSTR("linked-cell" ), _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

//##################################################################################################

// fixedline
//__________________________________________________________________________________________________
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
        throw xml::sax::SAXException(
            OUString( "expected event element!" ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void FixedLineElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( "com.sun.star.awt.UnoControlFixedLineModel" ) );

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
    ctx.importStringProperty( OUString( "Label" ),
                              OUString( "value" ),
                              _xAttributes );
    ctx.importOrientationProperty( OUString( "Orientation" ),
                                   OUString( "align" ),
                                   _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

//##################################################################################################

// patternfield
//__________________________________________________________________________________________________
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
        throw xml::sax::SAXException(
            OUString( "expected event element!" ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void PatternFieldElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( "com.sun.star.awt.UnoControlPatternFieldModel" ) );

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
    ctx.importBooleanProperty( OUString( "Tabstop" ),
                               OUString( "tabstop" ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "ReadOnly" ),
                               OUString( "readonly" ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "StrictFormat" ),
                               OUString( "strict-format" ),
                               _xAttributes );
    ctx.importBooleanProperty(
        OUSTR("HideInactiveSelection"), OUSTR("hide-inactive-selection"),
        _xAttributes );
    ctx.importStringProperty( OUString( "Text" ),
                              OUString( "value" ),
                              _xAttributes );
    ctx.importShortProperty( OUString( "MaxTextLen" ),
                             OUString( "maxlength" ),
                             _xAttributes );
    ctx.importStringProperty( OUString( "EditMask" ),
                              OUString( "edit-mask" ),
                              _xAttributes );
    ctx.importStringProperty( OUString( "LiteralMask" ),
                              OUString( "literal-mask" ),
                              _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

//##################################################################################################

// formattedfield
//__________________________________________________________________________________________________
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
        throw xml::sax::SAXException(
            OUString( "expected event element!" ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void FormattedFieldElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( "com.sun.star.awt.UnoControlFormattedFieldModel" ) );

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
    ctx.importBooleanProperty( OUString( "Tabstop" ),
                               OUString( "tabstop" ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "ReadOnly" ),
                               OUString( "readonly" ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "StrictFormat" ),
                               OUString( "strict-format" ),
                               _xAttributes );
    ctx.importBooleanProperty(
        OUSTR("HideInactiveSelection"), OUSTR("hide-inactive-selection"),
        _xAttributes );
    ctx.importAlignProperty( OUString( "Align" ),
                             OUString( "align" ),
                             _xAttributes );
    ctx.importDoubleProperty( OUString( "EffectiveMin" ),
                              OUString( "value-min" ),
                              _xAttributes );
    ctx.importDoubleProperty( OUString( "EffectiveMax" ),
                              OUString( "value-max" ),
                              _xAttributes );
    ctx.importDoubleProperty( OUString( "EffectiveValue" ),
                              OUString( "value" ),
                              _xAttributes );
    ctx.importStringProperty( OUString( "Text" ),
                              OUString( "text" ),
                              _xAttributes );
    ctx.importShortProperty( OUString( "MaxTextLen" ),
                             OUString( "maxlength" ),
                             _xAttributes );
    ctx.importBooleanProperty( OUString( "Spin" ),
                               OUString( "spin" ),
                               _xAttributes );
    if (ctx.importLongProperty( OUSTR("RepeatDelay"), OUSTR("repeat"),
                                _xAttributes ))
        ctx.getControlModel()->setPropertyValue(
            OUSTR("Repeat"), makeAny(true) );

    OUString sDefault(
        _xAttributes->getValueByUidName(
            _pImport->XMLNS_DIALOGS_UID,
            OUString( "value-default" ) ) );
    if (!sDefault.isEmpty())
    {
        double d = sDefault.toDouble();
        if (d != 0.0 ||
            sDefault.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("0") ) ||
            sDefault.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("0.0") ))
        {
            ctx.getControlModel()->setPropertyValue(
                OUString( "EffectiveDefault" ),
                makeAny( d ) );
        }
        else // treat as string
        {
            ctx.getControlModel()->setPropertyValue(
                OUString( "EffectiveDefault" ),
                makeAny( sDefault ) );
        }
    }

    // format spec
    ctx.getControlModel()->setPropertyValue(
        OUString( "FormatsSupplier" ),
        makeAny( _pImport->getNumberFormatsSupplier() ) );

    OUString sFormat(
        _xAttributes->getValueByUidName(
            _pImport->XMLNS_DIALOGS_UID,
            OUString( "format-code" ) ) );
    if (!sFormat.isEmpty())
    {
        lang::Locale locale;

        OUString sLocale(
            _xAttributes->getValueByUidName(
                _pImport->XMLNS_DIALOGS_UID,
                OUString( "format-locale" ) ) );
        if (!sLocale.isEmpty())
        {
            // split locale
            sal_Int32 semi0 = sLocale.indexOf( ';' );
            if (semi0 < 0) // no semi at all, just try language
            {
                locale.Language = sLocale;
            }
            else
            {
                sal_Int32 semi1 = sLocale.indexOf( ';', semi0 +1 );
                if (semi1 > semi0) // language;country;variant
                {
                    locale.Language = sLocale.copy( 0, semi0 );
                    locale.Country = sLocale.copy( semi0 +1, semi1 - semi0 -1 );
                    locale.Variant = sLocale.copy( semi1 +1 );
                }
                else // try language;country
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
            ctx.getControlModel()->setPropertyValue(
                OUString( "FormatKey" ), makeAny( nKey ) );
        }
        catch (const util::MalformedNumberFormatException & exc)
        {
            OSL_FAIL( "### util::MalformedNumberFormatException occurred!" );
            // rethrow
            throw xml::sax::SAXException( exc.Message, Reference< XInterface >(), Any() );
        }
    }
    ctx.importBooleanProperty(
        OUString( "TreatAsNumber" ),
        OUString( "treat-as-number" ),
        _xAttributes );
    ctx.importBooleanProperty( OUString( "EnforceFormat" ),
                               OUString( "enforce-format" ),
                               _xAttributes );

    ctx.importDataAwareProperty( OUSTR("linked-cell" ), _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

//##################################################################################################

// timefield
//__________________________________________________________________________________________________
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
        throw xml::sax::SAXException(
            OUString( "expected event element!" ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void TimeFieldElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( "com.sun.star.awt.UnoControlTimeFieldModel" ) );

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
    ctx.importBooleanProperty( OUString( "Tabstop" ),
                               OUString( "tabstop" ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "ReadOnly" ),
                               OUString( "readonly" ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "StrictFormat" ),
                               OUString( "strict-format" ),
                               _xAttributes );
    ctx.importBooleanProperty(
        OUSTR("HideInactiveSelection"), OUSTR("hide-inactive-selection"),
        _xAttributes );
    ctx.importTimeFormatProperty( OUString( "TimeFormat" ),
                                  OUString( "time-format" ),
                                  _xAttributes );
    ctx.importLongProperty( OUString( "Time" ),
                            OUString( "value" ),
                            _xAttributes );
    ctx.importLongProperty( OUString( "TimeMin" ),
                            OUString( "value-min" ),
                            _xAttributes );
    ctx.importLongProperty( OUString( "TimeMax" ),
                            OUString( "value-max" ),
                            _xAttributes );
    ctx.importBooleanProperty( OUString( "Spin" ),
                               OUString( "spin" ),
                               _xAttributes );
    if (ctx.importLongProperty( OUSTR("RepeatDelay"), OUSTR("repeat"),
                                _xAttributes ))
        ctx.getControlModel()->setPropertyValue(
            OUSTR("Repeat"), makeAny(true) );
    ctx.importStringProperty( OUString( "Text" ),
                              OUString( "text" ),
                              _xAttributes );
    ctx.importBooleanProperty( OUString( "EnforceFormat" ),
                               OUString( "enforce-format" ),
                               _xAttributes );

    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

//##################################################################################################

// numericfield
//__________________________________________________________________________________________________
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
        throw xml::sax::SAXException(
            OUString( "expected event element!" ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void NumericFieldElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( "com.sun.star.awt.UnoControlNumericFieldModel" ) );

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
    ctx.importBooleanProperty( OUString( "Tabstop" ),
                               OUString( "tabstop" ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "ReadOnly" ),
                               OUString( "readonly" ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "StrictFormat" ),
                               OUString( "strict-format" ),
                               _xAttributes );
    ctx.importBooleanProperty(
        OUSTR("HideInactiveSelection"), OUSTR("hide-inactive-selection"),
        _xAttributes );
    ctx.importShortProperty( OUString( "DecimalAccuracy" ),
                               OUString( "decimal-accuracy" ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "ShowThousandsSeparator" ),
                               OUString( "thousands-separator" ),
                               _xAttributes );
    ctx.importDoubleProperty( OUString( "Value" ),
                              OUString( "value" ),
                              _xAttributes );
    ctx.importDoubleProperty( OUString( "ValueMin" ),
                              OUString( "value-min" ),
                              _xAttributes );
    ctx.importDoubleProperty( OUString( "ValueMax" ),
                              OUString( "value-max" ),
                              _xAttributes );
    ctx.importDoubleProperty( OUString( "ValueStep" ),
                              OUString( "value-step" ),
                              _xAttributes );
    ctx.importBooleanProperty( OUString( "Spin" ),
                               OUString( "spin" ),
                               _xAttributes );
    if (ctx.importLongProperty( OUSTR("RepeatDelay"), OUSTR("repeat"),
                                _xAttributes ))
        ctx.getControlModel()->setPropertyValue(
            OUSTR("Repeat"), makeAny(true) );
    ctx.importBooleanProperty( OUString( "EnforceFormat" ),
                               OUString( "enforce-format" ),
                               _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

//##################################################################################################

// datefield
//__________________________________________________________________________________________________
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
        throw xml::sax::SAXException(
            OUString( "expected event element!" ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void DateFieldElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( "com.sun.star.awt.UnoControlDateFieldModel" ) );

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
    ctx.importBooleanProperty( OUString( "Tabstop" ),
                               OUString( "tabstop" ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "ReadOnly" ),
                               OUString( "readonly" ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "StrictFormat" ),
                               OUString( "strict-format" ),
                               _xAttributes );
    ctx.importBooleanProperty(
        OUSTR("HideInactiveSelection"), OUSTR("hide-inactive-selection"),
        _xAttributes );
    ctx.importDateFormatProperty( OUString( "DateFormat" ),
                                  OUString( "date-format" ),
                                  _xAttributes );
    ctx.importBooleanProperty( OUString( "DateShowCentury" ),
                               OUString( "show-century" ),
                               _xAttributes );
    ctx.importLongProperty( OUString( "Date" ),
                            OUString( "value" ),
                            _xAttributes );
    ctx.importLongProperty( OUString( "DateMin" ),
                            OUString( "value-min" ),
                            _xAttributes );
    ctx.importLongProperty( OUString( "DateMax" ),
                            OUString( "value-max" ),
                            _xAttributes );
    ctx.importBooleanProperty( OUString( "Spin" ),
                               OUString( "spin" ),
                               _xAttributes );
    if (ctx.importLongProperty( OUSTR("RepeatDelay"), OUSTR("repeat"),
                                _xAttributes ))
        ctx.getControlModel()->setPropertyValue(
            OUSTR("Repeat"), makeAny(true) );
    ctx.importBooleanProperty( OUString( "Dropdown" ),
                               OUString( "dropdown" ),
                               _xAttributes );
    ctx.importStringProperty( OUString( "Text" ),
                              OUString( "text" ),
                              _xAttributes );
    ctx.importBooleanProperty( OUString( "EnforceFormat" ),
                               OUString( "enforce-format" ),
                               _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

//##################################################################################################

// currencyfield
//__________________________________________________________________________________________________
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
        throw xml::sax::SAXException(
            OUString( "expected event element!" ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void CurrencyFieldElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( "com.sun.star.awt.UnoControlCurrencyFieldModel" ) );

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
    ctx.importBooleanProperty( OUString( "Tabstop" ),
                               OUString( "tabstop" ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "ReadOnly" ),
                               OUString( "readonly" ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "StrictFormat" ),
                               OUString( "strict-format" ),
                               _xAttributes );
    ctx.importBooleanProperty(
        OUSTR("HideInactiveSelection"), OUSTR("hide-inactive-selection"),
        _xAttributes );
    ctx.importStringProperty( OUString( "CurrencySymbol" ),
                              OUString( "currency-symbol" ),
                              _xAttributes );
    ctx.importShortProperty( OUString( "DecimalAccuracy" ),
                               OUString( "decimal-accuracy" ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "ShowThousandsSeparator" ),
                               OUString( "thousands-separator" ),
                               _xAttributes );
    ctx.importDoubleProperty( OUString( "Value" ),
                              OUString( "value" ),
                              _xAttributes );
    ctx.importDoubleProperty( OUString( "ValueMin" ),
                              OUString( "value-min" ),
                              _xAttributes );
    ctx.importDoubleProperty( OUString( "ValueMax" ),
                              OUString( "value-max" ),
                              _xAttributes );
    ctx.importDoubleProperty( OUString( "ValueStep" ),
                              OUString( "value-step" ),
                              _xAttributes );
    ctx.importBooleanProperty( OUString( "Spin" ),
                               OUString( "spin" ),
                               _xAttributes );
    if (ctx.importLongProperty( OUSTR("RepeatDelay"), OUSTR("repeat"),
                                _xAttributes ))
        ctx.getControlModel()->setPropertyValue(
            OUSTR("Repeat"), makeAny(true) );
    ctx.importBooleanProperty( OUString( "PrependCurrencySymbol" ),
                               OUString( "prepend-symbol" ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "EnforceFormat" ),
                               OUString( "enforce-format" ),
                               _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

//##################################################################################################

// filecontrol
//__________________________________________________________________________________________________
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
        throw xml::sax::SAXException(
            OUString( "expected event element!" ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void FileControlElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( "com.sun.star.awt.UnoControlFileControlModel" ) );

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
    ctx.importBooleanProperty( OUString( "Tabstop" ),
                               OUString( "tabstop" ),
                               _xAttributes );
    ctx.importBooleanProperty(
        OUSTR("HideInactiveSelection"), OUSTR("hide-inactive-selection"),
        _xAttributes );
    ctx.importStringProperty( OUString( "Text" ),
                              OUString( "value" ),
                              _xAttributes );
    ctx.importBooleanProperty( OUString( "ReadOnly" ),
                               OUString( "readonly" ),
                               _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}
//##################################################################################################

// treecontrol
//__________________________________________________________________________________________________
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
        throw xml::sax::SAXException(
            OUString( "expected event element!" ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void TreeControlElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( "com.sun.star.awt.tree.TreeControlModel" ) );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty( OUString( "Tabstop" ),
                               OUString( "tabstop" ),
                               _xAttributes );
    ctx.importSelectionTypeProperty( OUString( "SelectionType" ),
                              OUString( "selectiontype" ),
                              _xAttributes );
    ctx.importBooleanProperty( OUString( "RootDisplayed" ),
                               OUString( "rootdisplayed" ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "ShowsHandles" ),
                               OUString( "showshandles" ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "ShowsRootHandles" ),
                               OUString( "showsroothandles" ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "Editable" ),
                               OUString( "editable" ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "RowHeight" ),
                               OUString( "readonly" ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "InvokesStopNodeEditing" ),
                             OUString( "invokesstopnodeediting" ),
                             _xAttributes );

    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

//##################################################################################################

// imagecontrol
//__________________________________________________________________________________________________
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
        throw xml::sax::SAXException(
            OUString( "expected event element!" ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void ImageControlElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( "com.sun.star.awt.UnoControlImageControlModel" ) );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty( OUString( "ScaleImage" ),
                               OUString( "scale-image" ),
                               _xAttributes );
    rtl::OUString sURL = _xAttributes->getValueByUidName( _pImport->XMLNS_DIALOGS_UID, OUSTR( "src" ) );
    Reference< document::XStorageBasedDocument > xDocStorage( _pImport->getDocOwner(), UNO_QUERY );

    ctx.importImageURLProperty( OUSTR( "ImageURL" ), OUSTR( "src" ), _xAttributes );
    ctx.importBooleanProperty( OUString( "Tabstop" ),
                               OUString( "tabstop" ),
                               _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

//##################################################################################################

// textfield
//__________________________________________________________________________________________________
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
        throw xml::sax::SAXException(
            OUString( "expected event element!" ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void TextElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( "com.sun.star.awt.UnoControlFixedTextModel" ) );

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
    ctx.importStringProperty( OUString( "Label" ),
                              OUString( "value" ),
                              _xAttributes );
    ctx.importAlignProperty( OUString( "Align" ),
                             OUString( "align" ),
                             _xAttributes );
    ctx.importVerticalAlignProperty( OUString( "VerticalAlign" ),
                                     OUString( "valign" ),
                                     _xAttributes );
    ctx.importBooleanProperty( OUString( "MultiLine" ),
                               OUString( "multiline" ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "Tabstop" ),
                               OUString( "tabstop" ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "NoLabel" ),
                               OUString( "nolabel" ),
                               _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

//##################################################################################################
// FixedHyperLink
//__________________________________________________________________________________________________
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
        throw xml::sax::SAXException(
            OUString( "expected event element!" ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void FixedHyperLinkElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( "com.sun.star.awt.UnoControlFixedHyperlinkModel" ) );

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
    ctx.importStringProperty( OUString( "Label" ),
                              OUString( "value" ),
                              _xAttributes );
    ctx.importStringProperty( OUString( "URL" ),
                              OUString( "url" ),
                              _xAttributes );
    ctx.importStringProperty( OUString( "Description" ),
                              OUString( "description" ),
                              _xAttributes );

    ctx.importAlignProperty( OUString( "Align" ),
                             OUString( "align" ),
                             _xAttributes );
    ctx.importVerticalAlignProperty( OUString( "VerticalAlign" ),
                                     OUString( "valign" ),
                                     _xAttributes );
    ctx.importBooleanProperty( OUString( "MultiLine" ),
                               OUString( "multiline" ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "Tabstop" ),
                               OUString( "tabstop" ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "NoLabel" ),
                               OUString( "nolabel" ),
                               _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

//##################################################################################################

// edit
//__________________________________________________________________________________________________
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
        throw xml::sax::SAXException(
            OUString( "expected event element!" ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void TextFieldElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( "com.sun.star.awt.UnoControlEditModel" ) );
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
    ctx.importBooleanProperty( OUString( "Tabstop" ),
                               OUString( "tabstop" ),
                               _xAttributes );
    ctx.importAlignProperty( OUString( "Align" ),
                             OUString( "align" ),
                             _xAttributes );
    ctx.importBooleanProperty( OUString( "HardLineBreaks" ),
                               OUString( "hard-linebreaks" ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "HScroll" ),
                               OUString( "hscroll" ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "VScroll" ),
                               OUString( "vscroll" ),
                               _xAttributes );
    ctx.importBooleanProperty(
        OUSTR("HideInactiveSelection"), OUSTR("hide-inactive-selection"),
        _xAttributes );
    ctx.importShortProperty( OUString( "MaxTextLen" ),
                             OUString( "maxlength" ),
                             _xAttributes );
    ctx.importBooleanProperty( OUString( "MultiLine" ),
                               OUString( "multiline" ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "ReadOnly" ),
                               OUString( "readonly" ),
                               _xAttributes );
    ctx.importStringProperty( OUString( "Text" ),
                                  OUString( "value" ),
                                  _xAttributes );
    ctx.importLineEndFormatProperty( OUString( "LineEndFormat" ),
                                     OUString( "lineend-format" ),
                                     _xAttributes );
    OUString aValue;
    if (getStringAttr( &aValue,
                       OUString( "echochar" ),
                       _xAttributes,
                       _pImport->XMLNS_DIALOGS_UID ) &&
                       !aValue.isEmpty() )
    {
        OSL_ENSURE( aValue.getLength() == 1, "### more than one character given for echochar!" );
        sal_Int16 nChar = (sal_Int16)aValue[ 0 ];
        xControlModel->setPropertyValue( OUString( "EchoChar" ),
                                         makeAny( nChar ) );
    }

    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

//##################################################################################################

// titledbox
//__________________________________________________________________________________________________
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
        throw xml::sax::SAXException(
            OUString( "illegal namespace!" ),
            Reference< XInterface >(), Any() );
    }
    // title
    else if ( rLocalName == "title" )
    {
        getStringAttr( &_label,
                       OUString( "value" ),
                       xAttributes,
                       _pImport->XMLNS_DIALOGS_UID );

        return new ElementBase(
            _pImport->XMLNS_DIALOGS_UID,
            rLocalName, xAttributes, this, _pImport );
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
//__________________________________________________________________________________________________
void TitledBoxElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    {
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( "com.sun.star.awt.UnoControlGroupBoxModel" ) );
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
        xControlModel->setPropertyValue( OUString( "Label" ),
                                         makeAny( _label ) );
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
        Reference< xml::input::XAttributes > xAttributes(
            xRadio->getAttributes() );

        ControlImportContext ctx(
            _pImport, getControlId( xAttributes ),
            getControlModelName( OUString( "com.sun.star.awt.UnoControlRadioButtonModel" ), xAttributes ) );
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
        ctx.importBooleanProperty( OUString( "Tabstop" ),
                                   OUString( "tabstop" ),
                                   xAttributes );
        ctx.importStringProperty( OUString( "Label" ),
                                  OUString( "value" ),
                                  xAttributes );
        ctx.importAlignProperty( OUString( "Align" ),
                                 OUString( "align" ),
                                 xAttributes );
        ctx.importVerticalAlignProperty( OUString( "VerticalAlign" ),
                                         OUString( "valign" ),
                                         xAttributes );
        ctx.importImageURLProperty( OUSTR( "ImageURL" ), OUSTR( "image-src" ), _xAttributes );
        ctx.importImagePositionProperty( OUString( "ImagePosition" ),
                                         OUString( "image-position" ),
                                         xAttributes );
        ctx.importBooleanProperty( OUString( "MultiLine" ),
                                   OUString( "multiline" ),
                                   xAttributes );
        ctx.importStringProperty( OUString( "GroupName" ),
                                  OUString( "group-name" ),
                                  xAttributes );

        sal_Int16 nVal = 0;
        sal_Bool bChecked = sal_False;
        if (getBoolAttr( &bChecked,
                         OUString( "checked" ),
                         xAttributes,
                         _pImport->XMLNS_DIALOGS_UID ) &&
            bChecked)
        {
            nVal = 1;
        }
        xControlModel->setPropertyValue( OUString( "State" ),
                                         makeAny( nVal ) );
        ctx.importDataAwareProperty( OUSTR("linked-cell" ), xAttributes );
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

//##################################################################################################

// radio
//__________________________________________________________________________________________________
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
        throw xml::sax::SAXException(
            OUString( "expected event element!" ),
            Reference< XInterface >(), Any() );
    }
}

//##################################################################################################

// radiogroup
//__________________________________________________________________________________________________
Reference< xml::input::XElement > RadioGroupElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (_pImport->XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException(
            OUString( "illegal namespace!" ),
            Reference< XInterface >(), Any() );
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
        throw xml::sax::SAXException(
            OUString( "expected radio element!" ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void RadioGroupElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    for ( size_t nPos = 0; nPos < _radios.size(); ++nPos )
    {
        Reference< xml::input::XElement > xRadio( _radios[ nPos ] );
        Reference< xml::input::XAttributes > xAttributes(
            xRadio->getAttributes() );

        ControlImportContext ctx(
            _pImport, getControlId( xAttributes ),
            getControlModelName( OUString( "com.sun.star.awt.UnoControlRadioButtonModel" ), xAttributes ) );
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
        ctx.importBooleanProperty( OUString( "Tabstop" ),
                                   OUString( "tabstop" ),
                                   xAttributes );
        ctx.importStringProperty( OUString( "Label" ),
                                  OUString( "value" ),
                                  xAttributes );
        ctx.importAlignProperty( OUString( "Align" ),
                                 OUString( "align" ),
                                 xAttributes );
        ctx.importVerticalAlignProperty( OUString( "VerticalAlign" ),
                                         OUString( "valign" ),
                                         xAttributes );
        ctx.importImageURLProperty( OUSTR( "ImageURL" ), OUSTR( "image-src" ), xAttributes );
        ctx.importImagePositionProperty( OUString( "ImagePosition" ),
                                         OUString( "image-position" ),
                                         xAttributes );
        ctx.importBooleanProperty( OUString( "MultiLine" ),
                                   OUString( "multiline" ),
                                   xAttributes );
        ctx.importStringProperty( OUString( "GroupName" ),
                                  OUString( "group-name" ),
                                  xAttributes );
        sal_Int16 nVal = 0;
        sal_Bool bChecked = sal_False;
        if (getBoolAttr( &bChecked,
                         OUString( "checked" ),
                         xAttributes,
                         _pImport->XMLNS_DIALOGS_UID ) &&
            bChecked)
        {
            nVal = 1;
        }
        xControlModel->setPropertyValue( OUString( "State" ),
                                         makeAny( nVal ) );

        ctx.importDataAwareProperty( OUSTR("linked-cell" ), xAttributes );

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

//##################################################################################################

// menupopup
//__________________________________________________________________________________________________
Reference< xml::input::XElement > MenuPopupElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (_pImport->XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException(
            OUString( "illegal namespace!" ),
            Reference< XInterface >(), Any() );
    }
    // menuitem
    else if ( rLocalName == "menuitem" )
    {
        OUString aValue(
            xAttributes->getValueByUidName(
                _pImport->XMLNS_DIALOGS_UID,
                OUString( "value" ) ) );
        OSL_ENSURE( !aValue.isEmpty(), "### menuitem has no value?" );
        if (!aValue.isEmpty())
        {
            _itemValues.push_back( aValue );

            OUString aSel(
                xAttributes->getValueByUidName(
                    _pImport->XMLNS_DIALOGS_UID,
                    OUString( "selected" ) ) );
            if (!aSel.isEmpty() && aSel.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("true") ))
            {
                _itemSelected.push_back(
                    static_cast<sal_Int16>(_itemValues.size()) -1 );
            }
        }
        return new ElementBase(
            _pImport->XMLNS_DIALOGS_UID,
            rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( "expected menuitem!" ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
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
//__________________________________________________________________________________________________
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

//##################################################################################################

// menulist
//__________________________________________________________________________________________________
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
        throw xml::sax::SAXException(
            OUString( "illegal namespace!" ),
            Reference< XInterface >(), Any() );
    }
    // menupopup
    else if ( rLocalName == "menupopup" )
    {
        _popup = new MenuPopupElement( rLocalName, xAttributes, this, _pImport );
        return _popup;
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( "expected event or menupopup element!" ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void MenuListElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),

    getControlModelName( OUString( "com.sun.star.awt.UnoControlListBoxModel" ), _xAttributes  ) );
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
    ctx.importBooleanProperty( OUString( "Tabstop" ),
                               OUString( "tabstop" ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "MultiSelection" ),
                               OUString( "multiselection" ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "ReadOnly" ),
                               OUString( "readonly" ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "Dropdown" ),
                               OUString( "spin" ),
                               _xAttributes );
    ctx.importShortProperty( OUString( "LineCount" ),
                             OUString( "linecount" ),
                             _xAttributes );
    ctx.importAlignProperty( OUString( "Align" ),
                             OUString( "align" ),
                             _xAttributes );
    bool bHasLinkedCell = ctx.importDataAwareProperty( OUSTR("linked-cell" ), _xAttributes );
    bool bHasSrcRange = ctx.importDataAwareProperty( OUSTR("source-cell-range" ), _xAttributes );
    if (_popup.is())
    {
        MenuPopupElement * p = static_cast< MenuPopupElement * >( _popup.get() );
        if ( !bHasSrcRange )
            xControlModel->setPropertyValue( OUString( "StringItemList" ), makeAny( p->getItemValues() ) );
        if ( !bHasLinkedCell )
            xControlModel->setPropertyValue( OUString( "SelectedItems" ), makeAny( p->getSelectedItems() ) );

    }
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

//##################################################################################################

// combobox
//__________________________________________________________________________________________________
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
        throw xml::sax::SAXException(
            OUString( "illegal namespace!" ),
            Reference< XInterface >(), Any() );
    }
    // menupopup
    else if ( rLocalName == "menupopup" )
    {
        _popup = new MenuPopupElement( rLocalName, xAttributes, this, _pImport );
        return _popup;
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( "expected event or menupopup element!" ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void ComboBoxElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        getControlModelName( OUString( "com.sun.star.awt.UnoControlComboBoxModel" ), _xAttributes ) );
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
    ctx.importBooleanProperty( OUString( "Tabstop" ),
                               OUString( "tabstop" ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "ReadOnly" ),
                               OUString( "readonly" ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "Autocomplete" ),
                               OUString( "autocomplete" ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "Dropdown" ),
                               OUString( "spin" ),
                               _xAttributes );
    ctx.importBooleanProperty(
        OUSTR("HideInactiveSelection"), OUSTR("hide-inactive-selection"),
        _xAttributes );
    ctx.importShortProperty( OUString( "MaxTextLen" ),
                             OUString( "maxlength" ),
                             _xAttributes );
    ctx.importShortProperty( OUString( "LineCount" ),
                             OUString( "linecount" ),
                             _xAttributes );
    ctx.importStringProperty( OUString( "Text" ),
                              OUString( "value" ),
                              _xAttributes );
    ctx.importAlignProperty( OUString( "Align" ),
                             OUString( "align" ),
                             _xAttributes );
    ctx.importDataAwareProperty( OUSTR("linked-cell" ), _xAttributes );
    bool bHasSrcRange = ctx.importDataAwareProperty( OUSTR("source-cell-range" ), _xAttributes );
    if (_popup.is() && !bHasSrcRange )
    {
        MenuPopupElement * p = static_cast< MenuPopupElement * >( _popup.get() );
        xControlModel->setPropertyValue( OUString( "StringItemList" ),
                                         makeAny( p->getItemValues() ) );
    }

    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

//##################################################################################################

// checkbox
//__________________________________________________________________________________________________
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
        throw xml::sax::SAXException(
            OUString( "expected event element!" ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void CheckBoxElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( "com.sun.star.awt.UnoControlCheckBoxModel" ) );
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
    ctx.importBooleanProperty( OUString( "Tabstop" ),
                               OUString( "tabstop" ),
                               _xAttributes );
    ctx.importStringProperty( OUString( "Label" ),
                              OUString( "value" ),
                              _xAttributes );
    ctx.importAlignProperty( OUString( "Align" ),
                             OUString( "align" ),
                             _xAttributes );
    ctx.importVerticalAlignProperty( OUString( "VerticalAlign" ),
                                     OUString( "valign" ),
                                     _xAttributes );
    ctx.importImageURLProperty( OUSTR( "ImageURL" ), OUSTR( "image-src" ), _xAttributes );
    ctx.importImagePositionProperty( OUString( "ImagePosition" ),
                                     OUString( "image-position" ),
                                     _xAttributes );
    ctx.importBooleanProperty( OUString( "MultiLine" ),
                               OUString( "multiline" ),
                               _xAttributes );


    sal_Bool bTriState = sal_False;
    if (getBoolAttr( &bTriState,
                     OUString( "tristate" ),
                     _xAttributes,
                     _pImport->XMLNS_DIALOGS_UID ))
    {
        xControlModel->setPropertyValue( OUString( "TriState" ),
                                         makeAny( bTriState ) );
    }
    sal_Bool bChecked = sal_False;
    if (getBoolAttr( &bChecked,
                     OUString( "checked" ),
                     _xAttributes,
                     _pImport->XMLNS_DIALOGS_UID ))
    {
        // has "checked" attribute
        sal_Int16 nVal = (bChecked ? 1 : 0);
        xControlModel->setPropertyValue( OUString( "State" ),
                                         makeAny( nVal ) );
    }
    else
    {
        sal_Int16 nVal = (bTriState ? 2 : 0); // if tristate set, but checked omitted => dont know!
        xControlModel->setPropertyValue( OUString( "State" ),
                                         makeAny( nVal ) );
    }

    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

//##################################################################################################

// button
//__________________________________________________________________________________________________
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
        throw xml::sax::SAXException(
            OUString( "expected event element!" ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void ButtonElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( "com.sun.star.awt.UnoControlButtonModel" ) );

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
    ctx.importBooleanProperty( OUString( "Tabstop" ),
                               OUString( "tabstop" ),
                               _xAttributes );
    ctx.importStringProperty( OUString( "Label" ),
                              OUString( "value" ),
                              _xAttributes );
    ctx.importAlignProperty( OUString( "Align" ),
                             OUString( "align" ),
                             _xAttributes );
    ctx.importVerticalAlignProperty( OUString( "VerticalAlign" ),
                                     OUString( "valign" ),
                                     _xAttributes );
    ctx.importBooleanProperty( OUString( "DefaultButton" ),
                               OUString( "default" ),
                               _xAttributes );
    ctx.importButtonTypeProperty( OUString( "PushButtonType" ),
                                  OUString( "button-type" ),
                                  _xAttributes );
    ctx.importImageURLProperty( OUSTR( "ImageURL" ), OUSTR( "image-src" ), _xAttributes );
    ctx.importImagePositionProperty( OUString( "ImagePosition" ),
                                     OUString( "image-position" ),
                                     _xAttributes );
    ctx.importImageAlignProperty( OUString( "ImageAlign" ),
                                  OUString( "image-align" ),
                                  _xAttributes );
    if (ctx.importLongProperty( OUSTR("RepeatDelay"), OUSTR("repeat"),
                                _xAttributes ))
        ctx.getControlModel()->setPropertyValue(
            OUSTR("Repeat"), makeAny(true) );
    sal_Int32 toggled = 0;
    if (getLongAttr( &toggled, OUSTR("toggled"), _xAttributes,
                     _pImport->XMLNS_DIALOGS_UID ) && toggled == 1)
        ctx.getControlModel()->setPropertyValue(OUSTR("Toggle"), makeAny(true));
    ctx.importBooleanProperty( OUSTR("FocusOnClick"), OUSTR("grab-focus"),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( "MultiLine" ),
                               OUString( "multiline" ),
                               _xAttributes );
    // State
    sal_Bool bChecked = sal_False;
    if (getBoolAttr(
            &bChecked,
            OUString( "checked" ),
            _xAttributes,
            _pImport->XMLNS_DIALOGS_UID ) &&
        bChecked)
    {
        sal_Int16 nVal = 1;
        ctx.getControlModel()->setPropertyValue(
            OUString( "State" ), makeAny( nVal ) );
    }

    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

//##################################################################################################

// bulletinboard
//__________________________________________________________________________________________________
Reference< xml::input::XElement > BulletinBoardElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (_pImport->XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException(
            OUString( "illegal namespace!" ),
            Reference< XInterface >(), Any() );
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
        throw xml::sax::SAXException(
            OUString( "expected styles, bulletinboard or bulletinboard element, not: " ) + rLocalName,
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
BulletinBoardElement::BulletinBoardElement(
    OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes,
    ElementBase * pParent, DialogImport * pImport )
    SAL_THROW(())
    : ControlElement( rLocalName, xAttributes, pParent, pImport )
{
    OUString aValue(
        _xAttributes->getValueByUidName(
            _pImport->XMLNS_DIALOGS_UID,
            OUString( "left" ) ) );
    if (!aValue.isEmpty())
    {
        _nBasePosX += toInt32( aValue );
    }
    aValue = _xAttributes->getValueByUidName(
        _pImport->XMLNS_DIALOGS_UID,
        OUString( "top" ) );
    if (!aValue.isEmpty())
    {
        _nBasePosY += toInt32( aValue );
    }
}

//##################################################################################################

// style
//__________________________________________________________________________________________________
Reference< xml::input::XElement > StyleElement::startChildElement(
    sal_Int32 /*nUid*/, OUString const & /*rLocalName*/,
    Reference< xml::input::XAttributes > const & /*xAttributes*/ )
    throw (xml::sax::SAXException, RuntimeException)
{
    throw xml::sax::SAXException(
        OUString( "unexpected sub elements of style!" ),
        Reference< XInterface >(), Any() );
}
//__________________________________________________________________________________________________
void StyleElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    Reference< xml::input::XAttributes > xStyle;
    OUString aStyleId(
        _xAttributes->getValueByUidName(
            _pImport->XMLNS_DIALOGS_UID,
            OUString( "style-id" ) ) );
    if (!aStyleId.isEmpty())
    {
        _pImport->addStyle( aStyleId, this );
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( "missing style-id attribute!" ),
            Reference< XInterface >(), Any() );
    }
}

//##################################################################################################

// styles
//__________________________________________________________________________________________________
Reference< xml::input::XElement > StylesElement::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (_pImport->XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException(
            OUString( "illegal namespace!" ),
            Reference< XInterface >(), Any() );
    }
    // style
    else if ( rLocalName == "style" )
    {
        return new StyleElement( rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( "expected style element!" ),
            Reference< XInterface >(), Any() );
    }
}

//##################################################################################################

// window
//__________________________________________________________________________________________________
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
        throw xml::sax::SAXException(
            OUString( "illegal namespace!" ),
            Reference< XInterface >(), Any() );
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
        throw xml::sax::SAXException(
            OUString( "expected styles ot bulletinboard element!" ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
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
    ctx.importBooleanProperty(
        OUString( "Closeable" ),
        OUString( "closeable" ),
        _xAttributes );
    ctx.importBooleanProperty(
        OUString( "Moveable" ),
        OUString( "moveable" ),
        _xAttributes );
    ctx.importBooleanProperty(
        OUString( "Sizeable" ),
        OUString( "resizeable" ),
        _xAttributes );
    ctx.importStringProperty(
        OUString( "Title" ),
        OUString( "title" ),
        _xAttributes );
    ctx.importBooleanProperty(
        OUString( "Decoration" ),
        OUString( "withtitlebar" ),
        _xAttributes );
        ctx.importImageURLProperty( OUSTR( "ImageURL" ), OUSTR( "image-src" ), _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
