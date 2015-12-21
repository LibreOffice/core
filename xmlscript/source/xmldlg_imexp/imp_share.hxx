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

#ifndef INCLUDED_XMLSCRIPT_SOURCE_XMLDLG_IMEXP_IMP_SHARE_HXX
#define INCLUDED_XMLSCRIPT_SOURCE_XMLDLG_IMEXP_IMP_SHARE_HXX

#include "common.hxx"
#include "misc.hxx"
#include <xmlscript/xmldlg_imexp.hxx>
#include <xmlscript/xmllib_imexp.hxx>
#include <xmlscript/xmlmod_imexp.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontEmphasisMark.hpp>
#include <com/sun/star/awt/FontRelief.hpp>
#include <com/sun/star/xml/input/XRoot.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/container/ElementExistException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <osl/diagnose.h>
#include <memory>
#include <vector>

namespace xmlscript
{

inline sal_Int32 toInt32( OUString const & rStr )
{
    sal_Int32 nVal;
    if (rStr.getLength() > 2 && rStr[ 0 ] == '0' && rStr[ 1 ] == 'x')
        nVal = rStr.copy( 2 ).toUInt32( 16 );
    else
        nVal = rStr.toInt32();
    return nVal;
}

inline bool getBoolAttr(
    sal_Bool * pRet, OUString const & rAttrName,
    css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
    sal_Int32 nUid )
{
    OUString aValue( xAttributes->getValueByUidName( nUid, rAttrName ) );
    if (!aValue.isEmpty())
    {
        if ( aValue == "true" )
        {
            *pRet = sal_True;
            return true;
        }
        else if ( aValue == "false" )
        {
            *pRet = sal_False;
            return true;
        }
        else
        {
            throw css::xml::sax::SAXException(
                rAttrName + ": no boolean value (true|false)!",
                css::uno::Reference<css::uno::XInterface>(), css::uno::Any() );
        }
    }
    return false;
}

inline bool getStringAttr(
    OUString * pRet, OUString const & rAttrName,
    css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
    sal_Int32 nUid )
{
    *pRet = xAttributes->getValueByUidName( nUid, rAttrName );
    return (!pRet->isEmpty());
}

inline bool getLongAttr(
    sal_Int32 * pRet, OUString const & rAttrName,
    css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
    sal_Int32 nUid )
{
    OUString aValue( xAttributes->getValueByUidName( nUid, rAttrName ) );
    if (!aValue.isEmpty())
    {
        *pRet = toInt32( aValue );
        return true;
    }
    return false;
}

class ImportContext;

struct DialogImport
    : public ::cppu::WeakImplHelper< css::xml::input::XRoot >
{
    friend class ImportContext;

    css::uno::Reference< css::uno::XComponentContext > _xContext;
    css::uno::Reference< css::util::XNumberFormatsSupplier > _xSupplier;

    std::shared_ptr< std::vector< OUString > > _pStyleNames;
    std::shared_ptr< std::vector< css::uno::Reference< css::xml::input::XElement > > > _pStyles;

    css::uno::Reference< css::container::XNameContainer > _xDialogModel;
    css::uno::Reference< css::lang::XMultiServiceFactory > _xDialogModelFactory;
    css::uno::Reference< css::frame::XModel > _xDoc;

    sal_Int32 XMLNS_DIALOGS_UID, XMLNS_SCRIPT_UID;

public:
    inline bool isEventElement(
        sal_Int32 nUid, OUString const & rLocalName )
    {
        return ((XMLNS_SCRIPT_UID == nUid && (rLocalName == "event" || rLocalName == "listener-event" )) ||
                (XMLNS_DIALOGS_UID == nUid && rLocalName == "event" ));
    }

    void addStyle(
        OUString const & rStyleId,
        css::uno::Reference< css::xml::input::XElement > const & xStyle );
    css::uno::Reference< css::xml::input::XElement > getStyle(
        OUString const & rStyleId ) const;

    inline css::uno::Reference< css::uno::XComponentContext >
    const & getComponentContext()  { return _xContext; }
    css::uno::Reference< css::util::XNumberFormatsSupplier >
    const & getNumberFormatsSupplier();

    inline DialogImport(
        css::uno::Reference<css::uno::XComponentContext> const & xContext,
        css::uno::Reference<css::container::XNameContainer>
        const & xDialogModel,
        std::shared_ptr< std::vector< OUString > >& pStyleNames,
        std::shared_ptr< std::vector< css::uno::Reference< css::xml::input::XElement > > >& pStyles,
        css::uno::Reference<css::frame::XModel> const & xDoc )
        : _xContext( xContext )
        , _pStyleNames( pStyleNames )
        , _pStyles( pStyles )
        , _xDialogModel( xDialogModel )
        , _xDialogModelFactory( xDialogModel, css::uno::UNO_QUERY_THROW ), _xDoc( xDoc )
        , XMLNS_DIALOGS_UID( 0 )
        , XMLNS_SCRIPT_UID( 0 )
        { OSL_ASSERT( _xDialogModel.is() && _xDialogModelFactory.is() &&
                      _xContext.is() ); }
    inline DialogImport( const DialogImport& rOther ) :
        ::cppu::WeakImplHelper< css::xml::input::XRoot >()
        , _xContext( rOther._xContext )
        , _xSupplier( rOther._xSupplier )
        , _pStyleNames( rOther._pStyleNames )
        , _pStyles( rOther._pStyles )
        , _xDialogModel( rOther._xDialogModel )
        , _xDialogModelFactory( rOther._xDialogModelFactory )
        , _xDoc( rOther._xDoc )
        , XMLNS_DIALOGS_UID( rOther.XMLNS_DIALOGS_UID )
        , XMLNS_SCRIPT_UID( rOther.XMLNS_SCRIPT_UID ) {}

    virtual ~DialogImport();

    inline css::uno::Reference< css::frame::XModel > getDocOwner() { return _xDoc; }

    // XRoot
    virtual void SAL_CALL startDocument(
        css::uno::Reference< css::xml::input::XNamespaceMapping >
        const & xNamespaceMapping )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endDocument()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL processingInstruction(
        OUString const & rTarget, OUString const & rData )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDocumentLocator(
        css::uno::Reference< css::xml::sax::XLocator > const & xLocator )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startRootElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
};

class ElementBase
    : public ::cppu::WeakImplHelper< css::xml::input::XElement >
{
protected:
    DialogImport * const _pImport;
    ElementBase * const _pParent;

    const sal_Int32 _nUid;
    const OUString _aLocalName;
    const css::uno::Reference< css::xml::input::XAttributes > _xAttributes;

public:
    ElementBase(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport );
    virtual ~ElementBase();

    // XElement
    virtual css::uno::Reference<css::xml::input::XElement> SAL_CALL getParent()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getLocalName()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getUid()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::xml::input::XAttributes >
    SAL_CALL getAttributes() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL ignorableWhitespace(
        OUString const & rWhitespaces )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL characters( OUString const & rChars )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL processingInstruction(
        OUString const & Target, OUString const & Data )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endElement()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
};

class StylesElement
    : public ElementBase
{
public:
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    inline StylesElement(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : ElementBase( pImport->XMLNS_DIALOGS_UID,
                       rLocalName, xAttributes, pParent, pImport )
        {}
};

class StyleElement
    : public ElementBase
{
    sal_Int32 _backgroundColor;
    sal_Int32 _textColor;
    sal_Int32 _textLineColor;
    sal_Int16 _border;
    sal_Int32 _borderColor;
    css::awt::FontDescriptor _descr;
    sal_Int16 _fontRelief;
    sal_Int16 _fontEmphasisMark;
    sal_Int32 _fillColor;
    sal_Int16 _visualEffect;

    // current highest mask: 0x40
    short _inited, _hasValue;

    void setFontProperties(
        css::uno::Reference< css::beans::XPropertySet > const & xProps );

public:
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endElement()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    bool importTextColorStyle(
        css::uno::Reference< css::beans::XPropertySet > const & xProps );
    bool importTextLineColorStyle(
        css::uno::Reference< css::beans::XPropertySet > const & xProps );
    bool importFillColorStyle(
        css::uno::Reference< css::beans::XPropertySet > const & xProps );
    bool importBackgroundColorStyle(
        css::uno::Reference< css::beans::XPropertySet > const & xProps );
    bool importFontStyle(
        css::uno::Reference< css::beans::XPropertySet > const & xProps );
    bool importBorderStyle(
        css::uno::Reference< css::beans::XPropertySet > const & xProps );
    bool importVisualEffectStyle(
        css::uno::Reference< css::beans::XPropertySet > const & xProps );

    StyleElement(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : ElementBase( pImport->XMLNS_DIALOGS_UID,
                       rLocalName, xAttributes, pParent, pImport )
        , _backgroundColor(0)
        , _textColor(0)
        , _textLineColor(0)
        , _border(0)
        , _borderColor(0)
        , _fontRelief( css::awt::FontRelief::NONE )
        , _fontEmphasisMark( css::awt::FontEmphasisMark::NONE )
        , _fillColor(0)
        , _visualEffect(0)
        , _inited( 0 )
        , _hasValue( 0 )
    {
    }
};

class MenuPopupElement
    : public ElementBase
{
    std::vector< OUString > _itemValues;
    std::vector< sal_Int16 > _itemSelected;
public:
    css::uno::Sequence< OUString > getItemValues();
    css::uno::Sequence< sal_Int16 > getSelectedItems();

    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    inline MenuPopupElement(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : ElementBase( pImport->XMLNS_DIALOGS_UID,
                       rLocalName, xAttributes, pParent, pImport )
        {}
};

class ControlElement
    : public ElementBase
{
    friend class EventElement;

protected:
    sal_Int32 _nBasePosX, _nBasePosY;

    std::vector< css::uno::Reference< css::xml::input::XElement > > _events;

    OUString getControlId(
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes );
    OUString getControlModelName(
        OUString const& rDefaultModel,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes );
    css::uno::Reference< css::xml::input::XElement > getStyle(
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes );
public:
    std::vector<css::uno::Reference< css::xml::input::XElement> >& getEvents()
         { return _events; }

    ControlElement(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport );
};

class ImportContext
{
protected:
    DialogImport * const _pImport;
    const css::uno::Reference< css::beans::XPropertySet > _xControlModel;
    const OUString _aId;

public:
    inline ImportContext(
        DialogImport * pImport,
        css::uno::Reference< css::beans::XPropertySet > const & xControlModel_,
        OUString const & id )
        : _pImport( pImport ),
          _xControlModel( xControlModel_ ),
          _aId( id )
        { OSL_ASSERT( _xControlModel.is() ); }

    inline css::uno::Reference< css::beans::XPropertySet > getControlModel() const
        { return _xControlModel; }

    void importScollableSettings( css::uno::Reference< css::xml::input::XAttributes > const & xAttributes );
    void importDefaults(
        sal_Int32 nBaseX, sal_Int32 nBaseY,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        bool supportPrintable = true );
    void importEvents(
        std::vector< css::uno::Reference< css::xml::input::XElement > >
        const & rEvents );

    bool importStringProperty(
        OUString const & rPropName, OUString const & rAttrName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes );
    bool importDoubleProperty(
        OUString const & rPropName, OUString const & rAttrName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes );
    bool importBooleanProperty(
        OUString const & rPropName, OUString const & rAttrName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes );
    bool importShortProperty(
        OUString const & rPropName, OUString const & rAttrName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes );
    bool importLongProperty(
        OUString const & rPropName, OUString const & rAttrName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes );
    bool importLongProperty(
        sal_Int32 nOffset,
        OUString const & rPropName, OUString const & rAttrName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes );
    bool importHexLongProperty(
        OUString const & rPropName, OUString const & rAttrName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes );
    bool importAlignProperty(
        OUString const & rPropName, OUString const & rAttrName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes );
    bool importVerticalAlignProperty(
        OUString const & rPropName, OUString const & rAttrName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes );
    bool importImageURLProperty( OUString const & rPropName, OUString const & rAttrName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes );
    bool importImageAlignProperty(
        OUString const & rPropName, OUString const & rAttrName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes );
    bool importImagePositionProperty(
        OUString const & rPropName, OUString const & rAttrName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes );
    bool importDateProperty(
        OUString const & rPropName, OUString const & rAttrName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes );
    bool importDateFormatProperty(
        OUString const & rPropName, OUString const & rAttrName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes );
    bool importTimeProperty(
        OUString const & rPropName, OUString const & rAttrName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes );
    bool importTimeFormatProperty(
        OUString const & rPropName, OUString const & rAttrName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes );
    bool importOrientationProperty(
        OUString const & rPropName, OUString const & rAttrName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes );
    bool importButtonTypeProperty(
        OUString const & rPropName, OUString const & rAttrName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes );
    bool importLineEndFormatProperty(
        OUString const & rPropName, OUString const & rAttrName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes );
    bool importSelectionTypeProperty(
        OUString const & rPropName, OUString const & rAttrName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes );
    bool importDataAwareProperty(
        OUString const & rPropName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes );
    bool importImageScaleModeProperty(
        ::rtl::OUString const & rPropName, ::rtl::OUString const & rAttrName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes );
};

class ControlImportContext : public ImportContext
{
public:
    inline ControlImportContext(
        DialogImport * pImport,
        OUString const & rId, OUString const & rControlName )
        : ImportContext(
            pImport,
            css::uno::Reference< css::beans::XPropertySet >(
                pImport->_xDialogModelFactory->createInstance( rControlName ),
                css::uno::UNO_QUERY_THROW ), rId )
        {}
    inline ControlImportContext(
        DialogImport * pImport,
        const css::uno::Reference< css::beans::XPropertySet >& xProps, OUString const & rControlName )
        : ImportContext(
            pImport,
                xProps,
                rControlName )
        {}
    inline ~ControlImportContext()
    {
    }

    inline void finish() throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception)
    {
        try
        {
            _pImport->_xDialogModel->insertByName(
                _aId, css::uno::makeAny(
                    css::uno::Reference<css::awt::XControlModel>::query(
                        _xControlModel ) ) );
        }
        catch(const css::container::ElementExistException &e)
        {
            throw css::lang::WrappedTargetRuntimeException("", e.Context, makeAny(e));
        }
    }
};

class WindowElement
    : public ControlElement
{
public:
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endElement()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    inline WindowElement(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};

class EventElement
    : public ElementBase
{
public:
    virtual void SAL_CALL endElement()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    inline EventElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : ElementBase( nUid, rLocalName, xAttributes, pParent, pImport )
        {}
};

class BulletinBoardElement
    : public ControlElement
{
public:
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    BulletinBoardElement(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport );
};

class ButtonElement
    : public ControlElement
{
public:
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endElement()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    inline ButtonElement(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};

class CheckBoxElement
    : public ControlElement
{
public:
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endElement()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    inline CheckBoxElement(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};

class ComboBoxElement
    : public ControlElement
{
    css::uno::Reference< css::xml::input::XElement > _popup;
public:
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endElement()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    inline ComboBoxElement(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};

class MenuListElement
    : public ControlElement
{
    css::uno::Reference< css::xml::input::XElement > _popup;
public:
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endElement()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    inline MenuListElement(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};

class RadioElement
    : public ControlElement
{
public:
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    inline RadioElement(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};

class RadioGroupElement
    : public ControlElement
{
    std::vector< css::uno::Reference< css::xml::input::XElement > > _radios;
public:
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    void SAL_CALL endElement()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    inline RadioGroupElement(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};

class TitledBoxElement
    : public BulletinBoardElement
{
    OUString _label;
    std::vector< css::uno::Reference< css::xml::input::XElement > > _radios;
public:
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endElement()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    inline TitledBoxElement(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : BulletinBoardElement( rLocalName, xAttributes, pParent, pImport )
        {}
};

class TextElement
    : public ControlElement
{
public:
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endElement()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    inline TextElement(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};
class FixedHyperLinkElement
    : public ControlElement
{
public:
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endElement()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    inline FixedHyperLinkElement(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};

class TextFieldElement
    : public ControlElement
{
public:
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endElement()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    inline TextFieldElement(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};

class ImageControlElement
    : public ControlElement
{
public:
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endElement()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    inline ImageControlElement(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};

class FileControlElement
    : public ControlElement
{
public:
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endElement()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    inline FileControlElement(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};

class TreeControlElement
    : public ControlElement
{
public:
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endElement()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    inline TreeControlElement(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};

class CurrencyFieldElement
    : public ControlElement
{
public:
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endElement()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    inline CurrencyFieldElement(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};

class DateFieldElement
    : public ControlElement
{
public:
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endElement()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    inline DateFieldElement(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};

class NumericFieldElement
    : public ControlElement
{
public:
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endElement()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    inline NumericFieldElement(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};

class TimeFieldElement
    : public ControlElement
{
public:
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endElement()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    inline TimeFieldElement(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};

class PatternFieldElement
    : public ControlElement
{
public:
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endElement()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    inline PatternFieldElement(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};

class FormattedFieldElement
    : public ControlElement
{
public:
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endElement()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    inline FormattedFieldElement(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};

class FixedLineElement
    : public ControlElement
{
public:
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endElement()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    inline FixedLineElement(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};

class ScrollBarElement
    : public ControlElement
{
public:
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endElement()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    inline ScrollBarElement(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};

class SpinButtonElement
    : public ControlElement
{
public:
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endElement()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    inline SpinButtonElement(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};

class MultiPage
    : public ControlElement
{
public:
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endElement()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    inline MultiPage(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {
            m_xContainer.set( _pImport->_xDialogModelFactory->createInstance( "com.sun.star.awt.UnoMultiPageModel" ), css::uno::UNO_QUERY );
        }
private:
    css::uno::Reference< css::container::XNameContainer > m_xContainer;
};

class Frame
    : public ControlElement
{
    OUString _label;
public:
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endElement()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    inline Frame(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
private:
    css::uno::Reference< css::container::XNameContainer > m_xContainer;
};

class Page
    : public ControlElement
{
public:
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endElement()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    inline Page(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {
            m_xContainer.set( _pImport->_xDialogModelFactory->createInstance( "com.sun.star.awt.UnoPageModel" ), css::uno::UNO_QUERY );
        }
private:
    css::uno::Reference< css::container::XNameContainer > m_xContainer;
};

class ProgressBarElement
    : public ControlElement
{
public:
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endElement()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    inline ProgressBarElement(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};

}

#endif // INCLUDED_XMLSCRIPT_SOURCE_XMLDLG_IMEXP_IMP_SHARE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
