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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlscript.hxx"
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
        m_xContainer.set( _pImport->_xDialogModelFactory->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoFrameModel") ) ), UNO_QUERY );
    // event
    if (_pImport->isEventElement( nUid, rLocalName ))
    {
       return new EventElement(
            nUid, rLocalName, xAttributes, this, _pImport );
    }
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("bulletinboard") ))
    {
        // Create new DialogImport for this container
        DialogImport* pFrameImport = new DialogImport( *_pImport );
        pFrameImport->_xDialogModel = m_xContainer;
        return new BulletinBoardElement( rLocalName, xAttributes, this,  pFrameImport );
    }
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("title") ))
    {
        getStringAttr( &_label,
            OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________

void Frame::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
        if ( !m_xContainer.is() )
            m_xContainer.set( _pImport->_xDialogModelFactory->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoFrameModel") ) ), UNO_QUERY );
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
    if (_label.getLength())
    {
        xControlModel->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("Label") ),
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
rtl::OUString _label(RTL_CONSTASCII_USTRINGPARAM("foo"));
    if (_pImport->isEventElement( nUid, rLocalName ))
    {
        return new EventElement(
            nUid, rLocalName, xAttributes, this, _pImport );
    }
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("bulletinboard") ))
    {
        // Create new DialogImport for this container

        DialogImport* pMultiPageImport = new DialogImport( *_pImport );
                pMultiPageImport->_xDialogModel = m_xContainer;
        return new BulletinBoardElement( rLocalName, xAttributes, this,  pMultiPageImport );
    }
    else
    {

        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
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
    ctx.importLongProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("MultiPageValue") ),
                            OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
                            _xAttributes );
        ctx.importBooleanProperty(
            OUString( RTL_CONSTASCII_USTRINGPARAM("Decoration") ),
            OUString( RTL_CONSTASCII_USTRINGPARAM("withtabs") ),
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
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("bulletinboard") ))
    {

        DialogImport* pPageImport = new DialogImport( *_pImport );
                pPageImport->_xDialogModel = m_xContainer;
        return new BulletinBoardElement( rLocalName, xAttributes, this,  pPageImport );
    }
    else
    {

        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
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
    ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Title") ), OUString( RTL_CONSTASCII_USTRINGPARAM("title") ), _xAttributes );
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void ProgressBarElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlProgressBarModel") ) );

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
    ctx.importLongProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ProgressValue") ),
                            OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
                            _xAttributes );
    ctx.importLongProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ProgressValueMin") ),
                            OUString( RTL_CONSTASCII_USTRINGPARAM("value-min") ),
                            _xAttributes );
    ctx.importLongProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ProgressValueMax") ),
                            OUString( RTL_CONSTASCII_USTRINGPARAM("value-max") ),
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void ScrollBarElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        getControlModelName( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlScrollBarModel") ), _xAttributes ) );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importOrientationProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Orientation") ),
                                   OUString( RTL_CONSTASCII_USTRINGPARAM("align") ),
                                   _xAttributes );
    ctx.importLongProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("BlockIncrement") ),
                            OUString( RTL_CONSTASCII_USTRINGPARAM("pageincrement") ),
                            _xAttributes );
    ctx.importLongProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("LineIncrement") ),
                            OUString( RTL_CONSTASCII_USTRINGPARAM("increment") ),
                            _xAttributes );
    ctx.importLongProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ScrollValue") ),
                            OUString( RTL_CONSTASCII_USTRINGPARAM("curpos") ),
                            _xAttributes );
    ctx.importLongProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ScrollValueMax") ),
                            OUString( RTL_CONSTASCII_USTRINGPARAM("maxpos") ),
                            _xAttributes );
    ctx.importLongProperty( OUSTR("ScrollValueMin"), OUSTR("minpos"),
                            _xAttributes );
    ctx.importLongProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("VisibleSize") ),
                            OUString( RTL_CONSTASCII_USTRINGPARAM("visible-size") ),
                            _xAttributes );
    ctx.importLongProperty( OUSTR("RepeatDelay"), OUSTR("repeat"),
                            _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("tabstop") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("LiveScroll") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("live-scroll") ),
                               _xAttributes );
    ctx.importHexLongProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("SymbolColor") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("symbol-color") ),
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void SpinButtonElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
                            _pImport, getControlId( _xAttributes ),
                            getControlModelName( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlSpinButtonModel") ), _xAttributes ) );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importOrientationProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Orientation") ),
    OUString( RTL_CONSTASCII_USTRINGPARAM("align") ),
         _xAttributes );
    ctx.importLongProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("SpinIncrement") ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("increment") ),
        _xAttributes );
    ctx.importLongProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("SpinValue") ),
    OUString( RTL_CONSTASCII_USTRINGPARAM("curval") ),_xAttributes );
    ctx.importLongProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("SpinValueMax") ),
    OUString( RTL_CONSTASCII_USTRINGPARAM("maxval") ), _xAttributes );
    ctx.importLongProperty( OUSTR("SpinValueMin"), OUSTR("minval"),
        _xAttributes );
    ctx.importLongProperty( OUSTR("Repeat"), OUSTR("repeat"), _xAttributes );
    ctx.importLongProperty( OUSTR("RepeatDelay"), OUSTR("repeat-delay"),
_xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
    OUString( RTL_CONSTASCII_USTRINGPARAM("tabstop") ), _xAttributes );
    ctx.importHexLongProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("SymbolColor") ),
    OUString( RTL_CONSTASCII_USTRINGPARAM("symbol-color") ), _xAttributes );
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void FixedLineElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlFixedLineModel") ) );

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
    ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Label") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
                              _xAttributes );
    ctx.importOrientationProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Orientation") ),
                                   OUString( RTL_CONSTASCII_USTRINGPARAM("align") ),
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void PatternFieldElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlPatternFieldModel") ) );

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
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("tabstop") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("readonly") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("StrictFormat") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("strict-format") ),
                               _xAttributes );
    ctx.importBooleanProperty(
        OUSTR("HideInactiveSelection"), OUSTR("hide-inactive-selection"),
        _xAttributes );
    ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Text") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
                              _xAttributes );
    ctx.importShortProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("MaxTextLen") ),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("maxlength") ),
                             _xAttributes );
    ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("EditMask") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("edit-mask") ),
                              _xAttributes );
    ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("LiteralMask") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("literal-mask") ),
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void FormattedFieldElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlFormattedFieldModel") ) );

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
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("tabstop") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("readonly") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("StrictFormat") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("strict-format") ),
                               _xAttributes );
    ctx.importBooleanProperty(
        OUSTR("HideInactiveSelection"), OUSTR("hide-inactive-selection"),
        _xAttributes );
    ctx.importAlignProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Align") ),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("align") ),
                             _xAttributes );
    ctx.importDoubleProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("EffectiveMin") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value-min") ),
                              _xAttributes );
    ctx.importDoubleProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("EffectiveMax") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value-max") ),
                              _xAttributes );
    ctx.importDoubleProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("EffectiveValue") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
                              _xAttributes );
    ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Text") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("text") ),
                              _xAttributes );
    ctx.importShortProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("MaxTextLen") ),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("maxlength") ),
                             _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Spin") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("spin") ),
                               _xAttributes );
    if (ctx.importLongProperty( OUSTR("RepeatDelay"), OUSTR("repeat"),
                                _xAttributes ))
        ctx.getControlModel()->setPropertyValue(
            OUSTR("Repeat"), makeAny(true) );

    OUString sDefault(
        _xAttributes->getValueByUidName(
            _pImport->XMLNS_DIALOGS_UID,
            OUString( RTL_CONSTASCII_USTRINGPARAM("value-default") ) ) );
    if (sDefault.getLength())
    {
        double d = sDefault.toDouble();
        if (d != 0.0 ||
            sDefault.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("0") ) ||
            sDefault.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("0.0") ))
        {
            ctx.getControlModel()->setPropertyValue(
                OUString( RTL_CONSTASCII_USTRINGPARAM("EffectiveDefault") ),
                makeAny( d ) );
        }
        else // treat as string
        {
            ctx.getControlModel()->setPropertyValue(
                OUString( RTL_CONSTASCII_USTRINGPARAM("EffectiveDefault") ),
                makeAny( sDefault ) );
        }
    }

    // format spec
    ctx.getControlModel()->setPropertyValue(
        OUString( RTL_CONSTASCII_USTRINGPARAM("FormatsSupplier") ),
        makeAny( _pImport->getNumberFormatsSupplier() ) );

    OUString sFormat(
        _xAttributes->getValueByUidName(
            _pImport->XMLNS_DIALOGS_UID,
            OUString( RTL_CONSTASCII_USTRINGPARAM("format-code") ) ) );
    if (sFormat.getLength())
    {
        lang::Locale locale;

        OUString sLocale(
            _xAttributes->getValueByUidName(
                _pImport->XMLNS_DIALOGS_UID,
                OUString( RTL_CONSTASCII_USTRINGPARAM("format-locale") ) ) );
        if (sLocale.getLength())
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
                OUString( RTL_CONSTASCII_USTRINGPARAM("FormatKey") ), makeAny( nKey ) );
        }
        catch (util::MalformedNumberFormatException & exc)
        {
            OSL_ENSURE( 0, "### util::MalformedNumberFormatException occurred!" );
            // rethrow
            throw xml::sax::SAXException( exc.Message, Reference< XInterface >(), Any() );
        }
    }
    ctx.importBooleanProperty(
        OUString( RTL_CONSTASCII_USTRINGPARAM("TreatAsNumber") ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("treat-as-number") ),
        _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("EnforceFormat") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("enforce-format") ),
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void TimeFieldElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlTimeFieldModel") ) );

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
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("tabstop") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("readonly") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("StrictFormat") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("strict-format") ),
                               _xAttributes );
    ctx.importBooleanProperty(
        OUSTR("HideInactiveSelection"), OUSTR("hide-inactive-selection"),
        _xAttributes );
    ctx.importTimeFormatProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("TimeFormat") ),
                                  OUString( RTL_CONSTASCII_USTRINGPARAM("time-format") ),
                                  _xAttributes );
    ctx.importLongProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Time") ),
                            OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
                            _xAttributes );
    ctx.importLongProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("TimeMin") ),
                            OUString( RTL_CONSTASCII_USTRINGPARAM("value-min") ),
                            _xAttributes );
    ctx.importLongProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("TimeMax") ),
                            OUString( RTL_CONSTASCII_USTRINGPARAM("value-max") ),
                            _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Spin") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("spin") ),
                               _xAttributes );
    if (ctx.importLongProperty( OUSTR("RepeatDelay"), OUSTR("repeat"),
                                _xAttributes ))
        ctx.getControlModel()->setPropertyValue(
            OUSTR("Repeat"), makeAny(true) );
    ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Text") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("text") ),
                              _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("EnforceFormat") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("enforce-format") ),
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void NumericFieldElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlNumericFieldModel") ) );

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
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("tabstop") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("readonly") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("StrictFormat") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("strict-format") ),
                               _xAttributes );
    ctx.importBooleanProperty(
        OUSTR("HideInactiveSelection"), OUSTR("hide-inactive-selection"),
        _xAttributes );
    ctx.importShortProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("DecimalAccuracy") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("decimal-accuracy") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ShowThousandsSeparator") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("thousands-separator") ),
                               _xAttributes );
    ctx.importDoubleProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Value") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
                              _xAttributes );
    ctx.importDoubleProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ValueMin") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value-min") ),
                              _xAttributes );
    ctx.importDoubleProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ValueMax") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value-max") ),
                              _xAttributes );
    ctx.importDoubleProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ValueStep") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value-step") ),
                              _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Spin") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("spin") ),
                               _xAttributes );
    if (ctx.importLongProperty( OUSTR("RepeatDelay"), OUSTR("repeat"),
                                _xAttributes ))
        ctx.getControlModel()->setPropertyValue(
            OUSTR("Repeat"), makeAny(true) );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("EnforceFormat") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("enforce-format") ),
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void DateFieldElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlDateFieldModel") ) );

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
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("tabstop") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("readonly") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("StrictFormat") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("strict-format") ),
                               _xAttributes );
    ctx.importBooleanProperty(
        OUSTR("HideInactiveSelection"), OUSTR("hide-inactive-selection"),
        _xAttributes );
    ctx.importDateFormatProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("DateFormat") ),
                                  OUString( RTL_CONSTASCII_USTRINGPARAM("date-format") ),
                                  _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("DateShowCentury") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("show-century") ),
                               _xAttributes );
    ctx.importLongProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Date") ),
                            OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
                            _xAttributes );
    ctx.importLongProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("DateMin") ),
                            OUString( RTL_CONSTASCII_USTRINGPARAM("value-min") ),
                            _xAttributes );
    ctx.importLongProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("DateMax") ),
                            OUString( RTL_CONSTASCII_USTRINGPARAM("value-max") ),
                            _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Spin") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("spin") ),
                               _xAttributes );
    if (ctx.importLongProperty( OUSTR("RepeatDelay"), OUSTR("repeat"),
                                _xAttributes ))
        ctx.getControlModel()->setPropertyValue(
            OUSTR("Repeat"), makeAny(true) );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Dropdown") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("dropdown") ),
                               _xAttributes );
    ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Text") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("text") ),
                              _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("EnforceFormat") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("enforce-format") ),
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void CurrencyFieldElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlCurrencyFieldModel") ) );

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
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("tabstop") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("readonly") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("StrictFormat") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("strict-format") ),
                               _xAttributes );
    ctx.importBooleanProperty(
        OUSTR("HideInactiveSelection"), OUSTR("hide-inactive-selection"),
        _xAttributes );
    ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("CurrencySymbol") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("currency-symbol") ),
                              _xAttributes );
    ctx.importShortProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("DecimalAccuracy") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("decimal-accuracy") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ShowThousandsSeparator") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("thousands-separator") ),
                               _xAttributes );
    ctx.importDoubleProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Value") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
                              _xAttributes );
    ctx.importDoubleProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ValueMin") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value-min") ),
                              _xAttributes );
    ctx.importDoubleProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ValueMax") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value-max") ),
                              _xAttributes );
    ctx.importDoubleProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ValueStep") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value-step") ),
                              _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Spin") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("spin") ),
                               _xAttributes );
    if (ctx.importLongProperty( OUSTR("RepeatDelay"), OUSTR("repeat"),
                                _xAttributes ))
        ctx.getControlModel()->setPropertyValue(
            OUSTR("Repeat"), makeAny(true) );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("PrependCurrencySymbol") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("prepend-symbol") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("EnforceFormat") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("enforce-format") ),
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void FileControlElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlFileControlModel") ) );

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
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("tabstop") ),
                               _xAttributes );
    ctx.importBooleanProperty(
        OUSTR("HideInactiveSelection"), OUSTR("hide-inactive-selection"),
        _xAttributes );
    ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Text") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
                              _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("readonly") ),
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void TreeControlElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.tree.TreeControlModel") ) );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("tabstop") ),
                               _xAttributes );
    ctx.importSelectionTypeProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("SelectionType") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("selectiontype") ),
                              _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("RootDisplayed") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("rootdisplayed") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ShowsHandles") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("showshandles") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ShowsRootHandles") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("showsroothandles") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Editable") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("editable") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("RowHeight") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("readonly") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("InvokesStopNodeEditing") ),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("invokesstopnodeediting") ),
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void ImageControlElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlImageControlModel") ) );

    Reference< xml::input::XElement > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ScaleImage") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("scale-image") ),
                               _xAttributes );
    rtl::OUString sURL = _xAttributes->getValueByUidName( _pImport->XMLNS_DIALOGS_UID, OUSTR( "src" ) );
    Reference< document::XStorageBasedDocument > xDocStorage( _pImport->getDocOwner(), UNO_QUERY );

    ctx.importImageURLProperty( OUSTR( "ImageURL" ), OUSTR( "src" ), _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("tabstop") ),
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void TextElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlFixedTextModel") ) );

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
    ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Label") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
                              _xAttributes );
    ctx.importAlignProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Align") ),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("align") ),
                             _xAttributes );
    ctx.importVerticalAlignProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("VerticalAlign") ),
                                     OUString( RTL_CONSTASCII_USTRINGPARAM("valign") ),
                                     _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("MultiLine") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("multiline") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("tabstop") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("NoLabel") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("nolabel") ),
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void FixedHyperLinkElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlFixedHyperlinkModel") ) );

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
    ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Label") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
                              _xAttributes );
    ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("URL") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("url") ),
                              _xAttributes );
    ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Description") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("description") ),
                              _xAttributes );

    ctx.importAlignProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Align") ),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("align") ),
                             _xAttributes );
    ctx.importVerticalAlignProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("VerticalAlign") ),
                                     OUString( RTL_CONSTASCII_USTRINGPARAM("valign") ),
                                     _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("MultiLine") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("multiline") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("tabstop") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("NoLabel") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("nolabel") ),
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void TextFieldElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlEditModel") ) );
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
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("tabstop") ),
                               _xAttributes );
    ctx.importAlignProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Align") ),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("align") ),
                             _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("HardLineBreaks") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("hard-linebreaks") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("HScroll") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("hscroll") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("VScroll") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("vscroll") ),
                               _xAttributes );
    ctx.importBooleanProperty(
        OUSTR("HideInactiveSelection"), OUSTR("hide-inactive-selection"),
        _xAttributes );
    ctx.importShortProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("MaxTextLen") ),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("maxlength") ),
                             _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("MultiLine") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("multiline") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("readonly") ),
                               _xAttributes );
    ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Text") ),
                                  OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
                                  _xAttributes );
    ctx.importLineEndFormatProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("LineEndFormat") ),
                                     OUString( RTL_CONSTASCII_USTRINGPARAM("lineend-format") ),
                                     _xAttributes );
    OUString aValue;
    if (getStringAttr( &aValue,
                       OUString( RTL_CONSTASCII_USTRINGPARAM("echochar") ),
                       _xAttributes,
                       _pImport->XMLNS_DIALOGS_UID ) &&
        aValue.getLength() > 0)
    {
        OSL_ENSURE( aValue.getLength() == 1, "### more than one character given for echochar!" );
        sal_Int16 nChar = (sal_Int16)aValue[ 0 ];
        xControlModel->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("EchoChar") ),
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal namespace!") ),
            Reference< XInterface >(), Any() );
    }
    // title
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("title") ))
    {
        getStringAttr( &_label,
                       OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
                       xAttributes,
                       _pImport->XMLNS_DIALOGS_UID );

        return new ElementBase(
            _pImport->XMLNS_DIALOGS_UID,
            rLocalName, xAttributes, this, _pImport );
    }
    // radio
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("radio") ))
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
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlGroupBoxModel") ) );
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

    if (_label.getLength())
    {
        xControlModel->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("Label") ),
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
            getControlModelName( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlRadioButtonModel") ), xAttributes ) );
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
        ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                                   OUString( RTL_CONSTASCII_USTRINGPARAM("tabstop") ),
                                   xAttributes );
        ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Label") ),
                                  OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
                                  xAttributes );
        ctx.importAlignProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Align") ),
                                 OUString( RTL_CONSTASCII_USTRINGPARAM("align") ),
                                 xAttributes );
        ctx.importVerticalAlignProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("VerticalAlign") ),
                                         OUString( RTL_CONSTASCII_USTRINGPARAM("valign") ),
                                         xAttributes );
        ctx.importImageURLProperty( OUSTR( "ImageURL" ), OUSTR( "image-src" ), _xAttributes );
        ctx.importImagePositionProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ImagePosition") ),
                                         OUString( RTL_CONSTASCII_USTRINGPARAM("image-position") ),
                                         xAttributes );
        ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("MultiLine") ),
                                   OUString( RTL_CONSTASCII_USTRINGPARAM("multiline") ),
                                   xAttributes );
        ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("GroupName") ),
                                  OUString( RTL_CONSTASCII_USTRINGPARAM("group-name") ),
                                  xAttributes );

        sal_Int16 nVal = 0;
        sal_Bool bChecked = sal_False;
        if (getBoolAttr( &bChecked,
                         OUString( RTL_CONSTASCII_USTRINGPARAM("checked") ),
                         xAttributes,
                         _pImport->XMLNS_DIALOGS_UID ) &&
            bChecked)
        {
            nVal = 1;
        }
        xControlModel->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("State") ),
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal namespace!") ),
            Reference< XInterface >(), Any() );
    }
    // radio
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("radio") ))
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected radio element!") ),
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
            getControlModelName( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlRadioButtonModel") ), xAttributes ) );
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
        ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                                   OUString( RTL_CONSTASCII_USTRINGPARAM("tabstop") ),
                                   xAttributes );
        ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Label") ),
                                  OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
                                  xAttributes );
        ctx.importAlignProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Align") ),
                                 OUString( RTL_CONSTASCII_USTRINGPARAM("align") ),
                                 xAttributes );
        ctx.importVerticalAlignProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("VerticalAlign") ),
                                         OUString( RTL_CONSTASCII_USTRINGPARAM("valign") ),
                                         xAttributes );
        ctx.importImageURLProperty( OUSTR( "ImageURL" ), OUSTR( "image-src" ), xAttributes );
        ctx.importImagePositionProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ImagePosition") ),
                                         OUString( RTL_CONSTASCII_USTRINGPARAM("image-position") ),
                                         xAttributes );
        ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("MultiLine") ),
                                   OUString( RTL_CONSTASCII_USTRINGPARAM("multiline") ),
                                   xAttributes );
        ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("GroupName") ),
                                  OUString( RTL_CONSTASCII_USTRINGPARAM("group-name") ),
                                  xAttributes );
        sal_Int16 nVal = 0;
        sal_Bool bChecked = sal_False;
        if (getBoolAttr( &bChecked,
                         OUString( RTL_CONSTASCII_USTRINGPARAM("checked") ),
                         xAttributes,
                         _pImport->XMLNS_DIALOGS_UID ) &&
            bChecked)
        {
            nVal = 1;
        }
        xControlModel->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("State") ),
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal namespace!") ),
            Reference< XInterface >(), Any() );
    }
    // menuitem
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("menuitem") ))
    {
        OUString aValue(
            xAttributes->getValueByUidName(
                _pImport->XMLNS_DIALOGS_UID,
                OUString( RTL_CONSTASCII_USTRINGPARAM("value") ) ) );
        OSL_ENSURE( aValue.getLength(), "### menuitem has no value?" );
        if (aValue.getLength())
        {
            _itemValues.push_back( aValue );

            OUString aSel(
                xAttributes->getValueByUidName(
                    _pImport->XMLNS_DIALOGS_UID,
                    OUString( RTL_CONSTASCII_USTRINGPARAM("selected") ) ) );
            if (aSel.getLength() && aSel.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("true") ))
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected menuitem!") ),
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal namespace!") ),
            Reference< XInterface >(), Any() );
    }
    // menupopup
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("menupopup") ))
    {
        _popup = new MenuPopupElement( rLocalName, xAttributes, this, _pImport );
        return _popup;
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event or menupopup element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void MenuListElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),

    getControlModelName( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlListBoxModel") ), _xAttributes  ) );
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
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("tabstop") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("MultiSelection") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("multiselection") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("readonly") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Dropdown") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("spin") ),
                               _xAttributes );
    ctx.importShortProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("LineCount") ),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("linecount") ),
                             _xAttributes );
    ctx.importAlignProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Align") ),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("align") ),
                             _xAttributes );
    bool bHasLinkedCell = ctx.importDataAwareProperty( OUSTR("linked-cell" ), _xAttributes );
    bool bHasSrcRange = ctx.importDataAwareProperty( OUSTR("source-cell-range" ), _xAttributes );
    if (_popup.is())
    {
        MenuPopupElement * p = static_cast< MenuPopupElement * >( _popup.get() );
        if ( !bHasSrcRange )
            xControlModel->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("StringItemList") ), makeAny( p->getItemValues() ) );
        if ( !bHasLinkedCell )
            xControlModel->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("SelectedItems") ), makeAny( p->getSelectedItems() ) );

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
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal namespace!") ),
            Reference< XInterface >(), Any() );
    }
    // menupopup
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("menupopup") ))
    {
        _popup = new MenuPopupElement( rLocalName, xAttributes, this, _pImport );
        return _popup;
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event or menupopup element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void ComboBoxElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        getControlModelName( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlComboBoxModel") ), _xAttributes ) );
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
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("tabstop") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("readonly") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Autocomplete") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("autocomplete") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Dropdown") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("spin") ),
                               _xAttributes );
    ctx.importBooleanProperty(
        OUSTR("HideInactiveSelection"), OUSTR("hide-inactive-selection"),
        _xAttributes );
    ctx.importShortProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("MaxTextLen") ),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("maxlength") ),
                             _xAttributes );
    ctx.importShortProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("LineCount") ),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("linecount") ),
                             _xAttributes );
    ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Text") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
                              _xAttributes );
    ctx.importAlignProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Align") ),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("align") ),
                             _xAttributes );
    ctx.importDataAwareProperty( OUSTR("linked-cell" ), _xAttributes );
    bool bHasSrcRange = ctx.importDataAwareProperty( OUSTR("source-cell-range" ), _xAttributes );
    if (_popup.is() && !bHasSrcRange )
    {
        MenuPopupElement * p = static_cast< MenuPopupElement * >( _popup.get() );
        xControlModel->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("StringItemList") ),
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void CheckBoxElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlCheckBoxModel") ) );
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
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("tabstop") ),
                               _xAttributes );
    ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Label") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
                              _xAttributes );
    ctx.importAlignProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Align") ),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("align") ),
                             _xAttributes );
    ctx.importVerticalAlignProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("VerticalAlign") ),
                                     OUString( RTL_CONSTASCII_USTRINGPARAM("valign") ),
                                     _xAttributes );
    ctx.importImageURLProperty( OUSTR( "ImageURL" ), OUSTR( "image-src" ), _xAttributes );
    ctx.importImagePositionProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ImagePosition") ),
                                     OUString( RTL_CONSTASCII_USTRINGPARAM("image-position") ),
                                     _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("MultiLine") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("multiline") ),
                               _xAttributes );


    sal_Bool bTriState = sal_False;
    if (getBoolAttr( &bTriState,
                     OUString( RTL_CONSTASCII_USTRINGPARAM("tristate") ),
                     _xAttributes,
                     _pImport->XMLNS_DIALOGS_UID ))
    {
        xControlModel->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("TriState") ),
                                         makeAny( bTriState ) );
    }
    sal_Bool bChecked = sal_False;
    if (getBoolAttr( &bChecked,
                     OUString( RTL_CONSTASCII_USTRINGPARAM("checked") ),
                     _xAttributes,
                     _pImport->XMLNS_DIALOGS_UID ))
    {
        // has "checked" attribute
        sal_Int16 nVal = (bChecked ? 1 : 0);
        xControlModel->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("State") ),
                                         makeAny( nVal ) );
    }
    else
    {
        sal_Int16 nVal = (bTriState ? 2 : 0); // if tristate set, but checked omitted => dont know!
        xControlModel->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("State") ),
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void ButtonElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlButtonModel") ) );

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
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("tabstop") ),
                               _xAttributes );
    ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Label") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
                              _xAttributes );
    ctx.importAlignProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Align") ),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("align") ),
                             _xAttributes );
    ctx.importVerticalAlignProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("VerticalAlign") ),
                                     OUString( RTL_CONSTASCII_USTRINGPARAM("valign") ),
                                     _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("DefaultButton") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("default") ),
                               _xAttributes );
    ctx.importButtonTypeProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("PushButtonType") ),
                                  OUString( RTL_CONSTASCII_USTRINGPARAM("button-type") ),
                                  _xAttributes );
    ctx.importImageURLProperty( OUSTR( "ImageURL" ), OUSTR( "image-src" ), _xAttributes );
    ctx.importImagePositionProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ImagePosition") ),
                                     OUString( RTL_CONSTASCII_USTRINGPARAM("image-position") ),
                                     _xAttributes );
    ctx.importImageAlignProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ImageAlign") ),
                                  OUString( RTL_CONSTASCII_USTRINGPARAM("image-align") ),
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
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("MultiLine") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("multiline") ),
                               _xAttributes );
    // State
    sal_Bool bChecked = sal_False;
    if (getBoolAttr(
            &bChecked,
            OUString( RTL_CONSTASCII_USTRINGPARAM("checked") ),
            _xAttributes,
            _pImport->XMLNS_DIALOGS_UID ) &&
        bChecked)
    {
        sal_Int16 nVal = 1;
        ctx.getControlModel()->setPropertyValue(
            OUString( RTL_CONSTASCII_USTRINGPARAM("State") ), makeAny( nVal ) );
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal namespace!") ),
            Reference< XInterface >(), Any() );
    }
    // button
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("button") ))
    {
        return new ButtonElement( rLocalName, xAttributes, this, _pImport );
    }
    // checkbox
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("checkbox") ))
    {
        return new CheckBoxElement( rLocalName, xAttributes, this, _pImport );
    }
    // combobox
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("combobox") ))
    {
        return new ComboBoxElement( rLocalName, xAttributes, this, _pImport );
    }
    // listbox
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("menulist") ))
    {
        return new MenuListElement( rLocalName, xAttributes, this, _pImport );
    }
    // radiogroup
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("radiogroup") ))
    {
        return new RadioGroupElement( rLocalName, xAttributes, this, _pImport );
    }
    // titledbox
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("titledbox") ))
    {
        return new TitledBoxElement( rLocalName, xAttributes, this, _pImport );
    }
    // text
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("text") ))
    {
        return new TextElement( rLocalName, xAttributes, this, _pImport );
    }
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("linklabel") ))
    {
        return new FixedHyperLinkElement( rLocalName, xAttributes, this, _pImport );
    }
    // textfield
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("textfield") ))
    {
        return new TextFieldElement( rLocalName, xAttributes, this, _pImport );
    }
    // img
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("img") ))
    {
        return new ImageControlElement( rLocalName, xAttributes, this, _pImport );
    }
    // filecontrol
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("filecontrol") ))
    {
        return new FileControlElement( rLocalName, xAttributes, this, _pImport );
    }
    // treecontrol
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("treecontrol") ))
    {
        return new TreeControlElement( rLocalName, xAttributes, this, _pImport );
    }
    // currencyfield
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("currencyfield") ))
    {
        return new CurrencyFieldElement( rLocalName, xAttributes, this, _pImport );
    }
    // datefield
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("datefield") ))
    {
        return new DateFieldElement( rLocalName, xAttributes, this, _pImport );
    }
    // datefield
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("numericfield") ))
    {
        return new NumericFieldElement( rLocalName, xAttributes, this, _pImport );
    }
    // timefield
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("timefield") ))
    {
        return new TimeFieldElement( rLocalName, xAttributes, this, _pImport );
    }
    // patternfield
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("patternfield") ))
    {
        return new PatternFieldElement( rLocalName, xAttributes, this, _pImport );
    }
    // formattedfield
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("formattedfield") ))
    {
        return new FormattedFieldElement( rLocalName, xAttributes, this, _pImport );
    }
    // fixedline
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("fixedline") ))
    {
        return new FixedLineElement( rLocalName, xAttributes, this, _pImport );
    }
    // scrollbar
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("scrollbar") ))
    {
        return new ScrollBarElement( rLocalName, xAttributes, this, _pImport );
    }
    // spinbutton
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("spinbutton") ) )
    {
        return new SpinButtonElement( rLocalName, xAttributes, this, _pImport );
    }
    // progressmeter
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("progressmeter") ))
    {
        return new ProgressBarElement( rLocalName, xAttributes, this, _pImport );
    }
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("multipage") ))
    {
        return new MultiPage( rLocalName, xAttributes, this, _pImport );
    }
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("frame") ))
    {
        return new Frame( rLocalName, xAttributes, this, _pImport );
    }
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("page") ))
    {
        return new Page( rLocalName, xAttributes, this, _pImport );
    }
    // bulletinboard
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("bulletinboard") ))
    {
        return new BulletinBoardElement( rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected styles, bulletinboard or bulletinboard element, not: ") ) + rLocalName,
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
BulletinBoardElement::BulletinBoardElement(
    OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes,
    ElementBase * pParent, DialogImport * pImport )
    SAL_THROW( () )
    : ControlElement( rLocalName, xAttributes, pParent, pImport )
{
    OUString aValue(
        _xAttributes->getValueByUidName(
            _pImport->XMLNS_DIALOGS_UID,
            OUString( RTL_CONSTASCII_USTRINGPARAM("left") ) ) );
    if (aValue.getLength())
    {
        _nBasePosX += toInt32( aValue );
    }
    aValue = _xAttributes->getValueByUidName(
        _pImport->XMLNS_DIALOGS_UID,
        OUString( RTL_CONSTASCII_USTRINGPARAM("top") ) );
    if (aValue.getLength())
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
        OUString( RTL_CONSTASCII_USTRINGPARAM("unexpected sub elements of style!") ),
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("style-id") ) ) );
    if (aStyleId.getLength())
    {
        _pImport->addStyle( aStyleId, this );
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("missing style-id attribute!") ),
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal namespace!") ),
            Reference< XInterface >(), Any() );
    }
    // style
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("style") ))
    {
        return new StyleElement( rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected style element!") ),
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal namespace!") ),
            Reference< XInterface >(), Any() );
    }
    // styles
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("styles") ))
    {
        return new StylesElement( rLocalName, xAttributes, this, _pImport );
    }
    // bulletinboard
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("bulletinboard") ))
    {
        return new BulletinBoardElement( rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected styles ot bulletinboard element!") ),
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
        OUString( RTL_CONSTASCII_USTRINGPARAM("Closeable") ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("closeable") ),
        _xAttributes );
    ctx.importBooleanProperty(
        OUString( RTL_CONSTASCII_USTRINGPARAM("Moveable") ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("moveable") ),
        _xAttributes );
    ctx.importBooleanProperty(
        OUString( RTL_CONSTASCII_USTRINGPARAM("Sizeable") ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("resizeable") ),
        _xAttributes );
    ctx.importStringProperty(
        OUString( RTL_CONSTASCII_USTRINGPARAM("Title") ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("title") ),
        _xAttributes );
    ctx.importBooleanProperty(
        OUString( RTL_CONSTASCII_USTRINGPARAM("Decoration") ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("withtitlebar") ),
        _xAttributes );
        ctx.importImageURLProperty( OUSTR( "ImageURL" ), OUSTR( "image-src" ), _xAttributes );
    ctx.importEvents( _events );
    // avoid ring-reference:
    // vector< event elements > holding event elements holding this (via _pParent)
    _events.clear();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
