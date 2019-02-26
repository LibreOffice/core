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
#include <misc.hxx>
#include <xmlscript/xmldlg_imexp.hxx>
#include <xmlscript/xmllib_imexp.hxx>
#include <xmlscript/xmlmod_imexp.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/MalformedNumberFormatException.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontEmphasisMark.hpp>
#include <com/sun/star/awt/FontRelief.hpp>
#include <com/sun/star/xml/input/XRoot.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/container/ElementExistException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <osl/diagnose.h>
#include <rtl/ref.hxx>
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
            *pRet = true;
            return true;
        }
        else if ( aValue == "false" )
        {
            *pRet = false;
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
private:
    css::uno::Reference< css::uno::XComponentContext > _xContext;
    css::uno::Reference< css::util::XNumberFormatsSupplier > _xSupplier;

    std::shared_ptr< std::vector< OUString > > _pStyleNames;
    std::shared_ptr< std::vector< css::uno::Reference< css::xml::input::XElement > > > _pStyles;

    css::uno::Reference< css::frame::XModel > _xDoc;
public:
    css::uno::Reference< css::container::XNameContainer > _xDialogModel;
    css::uno::Reference< css::lang::XMultiServiceFactory > _xDialogModelFactory;

    sal_Int32 XMLNS_DIALOGS_UID, XMLNS_SCRIPT_UID;

    bool isEventElement(
        sal_Int32 nUid, OUString const & rLocalName ) const
    {
        return ((XMLNS_SCRIPT_UID == nUid && (rLocalName == "event" || rLocalName == "listener-event" )) ||
                (XMLNS_DIALOGS_UID == nUid && rLocalName == "event" ));
    }

    void addStyle(
        OUString const & rStyleId,
        css::uno::Reference< css::xml::input::XElement > const & xStyle );
    css::uno::Reference< css::xml::input::XElement > getStyle(
        OUString const & rStyleId ) const;

    css::uno::Reference< css::uno::XComponentContext >
    const & getComponentContext() const { return _xContext; }
    css::uno::Reference< css::util::XNumberFormatsSupplier >
    const & getNumberFormatsSupplier();

    DialogImport(
        css::uno::Reference<css::uno::XComponentContext> const & xContext,
        css::uno::Reference<css::container::XNameContainer>
        const & xDialogModel,
        std::shared_ptr< std::vector< OUString > > const & pStyleNames,
        std::shared_ptr< std::vector< css::uno::Reference< css::xml::input::XElement > > > const & pStyles,
        css::uno::Reference<css::frame::XModel> const & xDoc )
        : _xContext( xContext )
        , _pStyleNames( pStyleNames )
        , _pStyles( pStyles )
        , _xDoc( xDoc )
        , _xDialogModel( xDialogModel )
        , _xDialogModelFactory( xDialogModel, css::uno::UNO_QUERY_THROW )
        , XMLNS_DIALOGS_UID( 0 )
        , XMLNS_SCRIPT_UID( 0 )
        { OSL_ASSERT( _xDialogModel.is() && _xContext.is() ); }
    DialogImport( const DialogImport& rOther ) :
        ::cppu::WeakImplHelper< css::xml::input::XRoot >()
        , _xContext( rOther._xContext )
        , _xSupplier( rOther._xSupplier )
        , _pStyleNames( rOther._pStyleNames )
        , _pStyles( rOther._pStyles )
        , _xDoc( rOther._xDoc )
        , _xDialogModel( rOther._xDialogModel )
        , _xDialogModelFactory( rOther._xDialogModelFactory )
        , XMLNS_DIALOGS_UID( rOther.XMLNS_DIALOGS_UID )
        , XMLNS_SCRIPT_UID( rOther.XMLNS_SCRIPT_UID ) {}

    virtual ~DialogImport() override;

    const css::uno::Reference< css::frame::XModel >& getDocOwner() const { return _xDoc; }

    // XRoot
    virtual void SAL_CALL startDocument(
        css::uno::Reference< css::xml::input::XNamespaceMapping >
        const & xNamespaceMapping ) override;
    virtual void SAL_CALL endDocument() override;
    virtual void SAL_CALL processingInstruction(
        OUString const & rTarget, OUString const & rData ) override;
    virtual void SAL_CALL setDocumentLocator(
        css::uno::Reference< css::xml::sax::XLocator > const & xLocator ) override;
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startRootElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes ) override;
};

class ElementBase
    : public ::cppu::WeakImplHelper< css::xml::input::XElement >
{
protected:
    rtl::Reference<DialogImport> const m_xImport;
    rtl::Reference<ElementBase> const m_xParent;
private:
    const sal_Int32 _nUid;
    const OUString _aLocalName;
protected:
    const css::uno::Reference< css::xml::input::XAttributes > _xAttributes;

public:
    ElementBase(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport );
    virtual ~ElementBase() override;

    // XElement
    virtual css::uno::Reference<css::xml::input::XElement> SAL_CALL getParent() override;
    virtual OUString SAL_CALL getLocalName() override;
    virtual sal_Int32 SAL_CALL getUid() override;
    virtual css::uno::Reference< css::xml::input::XAttributes >
    SAL_CALL getAttributes() override;
    virtual void SAL_CALL ignorableWhitespace(
        OUString const & rWhitespaces ) override;
    virtual void SAL_CALL characters( OUString const & rChars ) override;
    virtual void SAL_CALL processingInstruction(
        OUString const & Target, OUString const & Data ) override;
    virtual void SAL_CALL endElement() override;
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes ) override;
};

class StylesElement
    : public ElementBase
{
public:
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes ) override;

    StylesElement(
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
        css::uno::Reference< css::beans::XPropertySet > const & xProps ) const;

public:
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes ) override;
    virtual void SAL_CALL endElement() override;

    void importTextColorStyle(
        css::uno::Reference< css::beans::XPropertySet > const & xProps );
    void importTextLineColorStyle(
        css::uno::Reference< css::beans::XPropertySet > const & xProps );
    void importFillColorStyle(
        css::uno::Reference< css::beans::XPropertySet > const & xProps );
    void importBackgroundColorStyle(
        css::uno::Reference< css::beans::XPropertySet > const & xProps );
    void importFontStyle(
        css::uno::Reference< css::beans::XPropertySet > const & xProps );
    void importBorderStyle(
        css::uno::Reference< css::beans::XPropertySet > const & xProps );
    void importVisualEffectStyle(
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
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes ) override;

    MenuPopupElement(
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
    ImportContext(
        DialogImport * pImport,
        css::uno::Reference< css::beans::XPropertySet > const & xControlModel_,
        OUString const & id )
        : _pImport( pImport ),
          _xControlModel( xControlModel_ ),
          _aId( id )
        { OSL_ASSERT( _xControlModel.is() ); }

    const css::uno::Reference< css::beans::XPropertySet >& getControlModel() const
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
    bool importGraphicOrImageProperty(OUString const & rAttrName,
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
        OUString const & rPropName, OUString const & rAttrName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes );
};

class ControlImportContext : public ImportContext
{
public:
    ControlImportContext(
        DialogImport * pImport,
        OUString const & rId, OUString const & rControlName )
        : ImportContext(
            pImport,
            css::uno::Reference< css::beans::XPropertySet >(
                pImport->_xDialogModelFactory->createInstance( rControlName ),
                css::uno::UNO_QUERY_THROW ), rId )
        {}
    ControlImportContext(
        DialogImport * pImport,
        const css::uno::Reference< css::beans::XPropertySet >& xProps, OUString const & rControlName )
        : ImportContext(
            pImport,
                xProps,
                rControlName )
        {}

    /// @throws css::xml::sax::SAXException
    /// @throws css::uno::RuntimeException
    void finish()
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
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes ) override;
    virtual void SAL_CALL endElement() override;

    WindowElement(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        DialogImport * pImport )
        : ControlElement( rLocalName, xAttributes, nullptr, pImport )
        {}
};

class EventElement
    : public ElementBase
{
public:
    virtual void SAL_CALL endElement() override;

    EventElement(
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
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes ) override;

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
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes ) override;
    virtual void SAL_CALL endElement() override;

    ButtonElement(
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
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes ) override;
    virtual void SAL_CALL endElement() override;

    CheckBoxElement(
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
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes ) override;
    virtual void SAL_CALL endElement() override;

    ComboBoxElement(
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
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes ) override;
    virtual void SAL_CALL endElement() override;

    MenuListElement(
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
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes ) override;

    RadioElement(
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
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes ) override;
    void SAL_CALL endElement() override;

    RadioGroupElement(
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
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes ) override;
    virtual void SAL_CALL endElement() override;

    TitledBoxElement(
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
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes ) override;
    virtual void SAL_CALL endElement() override;

    TextElement(
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
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes ) override;
    virtual void SAL_CALL endElement() override;

    FixedHyperLinkElement(
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
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes ) override;
    virtual void SAL_CALL endElement() override;

    TextFieldElement(
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
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes ) override;
    virtual void SAL_CALL endElement() override;

    ImageControlElement(
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
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes ) override;
    virtual void SAL_CALL endElement() override;

    FileControlElement(
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
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes ) override;
    virtual void SAL_CALL endElement() override;

    TreeControlElement(
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
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes ) override;
    virtual void SAL_CALL endElement() override;

    CurrencyFieldElement(
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
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes ) override;
    virtual void SAL_CALL endElement() override;

    DateFieldElement(
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
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes ) override;
    virtual void SAL_CALL endElement() override;

    NumericFieldElement(
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
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes ) override;
    virtual void SAL_CALL endElement() override;

    TimeFieldElement(
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
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes ) override;
    virtual void SAL_CALL endElement() override;

    PatternFieldElement(
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
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes ) override;
    virtual void SAL_CALL endElement() override;

    FormattedFieldElement(
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
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes ) override;
    virtual void SAL_CALL endElement() override;

    FixedLineElement(
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
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes ) override;
    virtual void SAL_CALL endElement() override;

    ScrollBarElement(
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
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes ) override;
    virtual void SAL_CALL endElement() override;

    SpinButtonElement(
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
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes ) override;
    virtual void SAL_CALL endElement() override;

    MultiPage(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {
            m_xContainer.set( m_xImport->_xDialogModelFactory->createInstance( "com.sun.star.awt.UnoMultiPageModel" ), css::uno::UNO_QUERY );
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
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes ) override;
    virtual void SAL_CALL endElement() override;

    Frame(
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
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes ) override;
    virtual void SAL_CALL endElement() override;

    Page(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {
            m_xContainer.set( m_xImport->_xDialogModelFactory->createInstance( "com.sun.star.awt.UnoPageModel" ), css::uno::UNO_QUERY );
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
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes ) override;
    virtual void SAL_CALL endElement() override;

    ProgressBarElement(
        OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};

//==============================================================================
class GridControlElement
    : public ControlElement
{
public:
    virtual css::uno::Reference< css::xml::input::XElement >
    SAL_CALL startChildElement(
        sal_Int32 nUid,::rtl::OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes ) override;
    virtual void SAL_CALL endElement() override;

    GridControlElement(OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};

}

#endif // INCLUDED_XMLSCRIPT_SOURCE_XMLDLG_IMEXP_IMP_SHARE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
