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

#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBASHEETOBJECT_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBASHEETOBJECT_HXX

#include <ooo/vba/excel/XButton.hpp>
#include <ooo/vba/excel/XControlObject.hpp>
#include <ooo/vba/excel/XSheetObject.hpp>
#include <cppuhelper/implbase.hxx>
#include <vbahelper/vbahelperinterface.hxx>
#include "vbapalette.hxx"

namespace com { namespace sun { namespace star {
    namespace container { class XIndexContainer; }
    namespace drawing { class XControlShape; }
} } }

typedef InheritedHelperInterfaceWeakImpl< ov::excel::XCharacters > ScVbaButtonCharacters_BASE;

/** Simple implementation of the Characters symbol for drawing button objects. */
class ScVbaButtonCharacters : public ScVbaButtonCharacters_BASE
{
public:
    explicit ScVbaButtonCharacters(
        const css::uno::Reference< ov::XHelperInterface >& rxParent,
        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        const css::uno::Reference< css::beans::XPropertySet >& rxPropSet,
        const ScVbaPalette& rPalette,
        const css::uno::Any& rStart,
        const css::uno::Any& rLength ) throw (css::uno::RuntimeException);
    virtual ~ScVbaButtonCharacters();

    // XCharacters attributes
    virtual OUString SAL_CALL getCaption() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setCaption( const OUString& rCaption ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getText() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setText( const OUString& rText ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getCount() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Reference< ov::excel::XFont > SAL_CALL getFont() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setFont( const css::uno::Reference< ov::excel::XFont >& rxFont ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XCharacters methods
    virtual void SAL_CALL Insert( const OUString& rString ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL Delete() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XHelperInterface
    VBAHELPER_DECL_XHELPERINTERFACE

private:
    OUString getFullString() const throw (css::uno::RuntimeException);
    void setFullString( const OUString& rString ) throw (css::uno::RuntimeException);

private:
    ScVbaPalette maPalette;
    css::uno::Reference< css::beans::XPropertySet > mxPropSet;
    sal_Int32 mnStart;
    sal_Int32 mnLength;
};

typedef InheritedHelperInterfaceWeakImpl< ov::excel::XSheetObject > ScVbaSheetObject_BASE;

/** Base class for drawing objects embedded in sheets. */
class ScVbaSheetObjectBase : public ScVbaSheetObject_BASE
{
public:
    explicit ScVbaSheetObjectBase(
        const css::uno::Reference< ov::XHelperInterface >& rxParent,
        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        const css::uno::Reference< css::frame::XModel >& rxModel,
        const css::uno::Reference< css::drawing::XShape >& rxShape ) throw (css::uno::RuntimeException);

    // XSheetObject attributes
    virtual double SAL_CALL getLeft() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setLeft( double fLeft ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual double SAL_CALL getTop() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setTop( double fTop ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual double SAL_CALL getWidth() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setWidth( double fWidth ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual double SAL_CALL getHeight() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setHeight( double fHeight ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getName() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setName( const OUString& rName ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getPlacement() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setPlacement( sal_Int32 nPlacement ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL getPrintObject() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setPrintObject( sal_Bool bPrintObject ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    /** Sets default properties after a new object has been created. */
    void setDefaultProperties( sal_Int32 nIndex ) throw (css::uno::RuntimeException);

protected:
    /** Derived classes return the base name used for new objects. */
    virtual OUString implGetBaseName() const = 0;
    /** Derived classes set default properties for new drawing objects. */
    virtual void implSetDefaultProperties() throw (css::uno::RuntimeException);

protected:
    ScVbaPalette maPalette;
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::drawing::XShape > mxShape;
    css::uno::Reference< css::beans::XPropertySet > mxShapeProps;
};

typedef ::cppu::ImplInheritanceHelper< ScVbaSheetObjectBase, ov::excel::XControlObject > ScVbaControlObject_BASE;

class ScVbaControlObjectBase : public ScVbaControlObject_BASE
{
public:
    /** Specifies the listener used for OnAction events. */
    enum ListenerType
    {
        LISTENER_ACTION,        /// XActionListener.actionPerformed
        LISTENER_MOUSE,         /// XMouseListener.mouseReleased
        LISTENER_TEXT,          /// XTextListener.textChanged
        LISTENER_VALUE,         /// XAdjustmentListener.adjustmentValueChanged
        LISTENER_CHANGE         /// XChangeListener.changed
    };

    explicit ScVbaControlObjectBase(
        const css::uno::Reference< ov::XHelperInterface >& rxParent,
        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        const css::uno::Reference< css::frame::XModel >& rxModel,
        const css::uno::Reference< css::container::XIndexContainer >& rxFormIC,
        const css::uno::Reference< css::drawing::XControlShape >& rxControlShape,
        ListenerType eListenerType ) throw (css::uno::RuntimeException);

    // XSheetObject attributes
    virtual OUString SAL_CALL getName() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setName( const OUString& rName ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getOnAction() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setOnAction( const OUString& rMacroName ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL getPrintObject() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setPrintObject( sal_Bool bPrintObject ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XControlObject attributes
    virtual sal_Bool SAL_CALL getAutoSize() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setAutoSize( sal_Bool bAutoSize ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

protected:
    sal_Int32 getModelIndexInForm() const throw (css::uno::RuntimeException);

protected:
    css::uno::Reference< css::container::XIndexContainer > mxFormIC;
    css::uno::Reference< css::beans::XPropertySet > mxControlProps;
    OUString maListenerType;
    OUString maEventMethod;
};

typedef ::cppu::ImplInheritanceHelper< ScVbaControlObjectBase, ov::excel::XButton > ScVbaButton_BASE;

class ScVbaButton : public ScVbaButton_BASE
{
public:
    explicit ScVbaButton(
        const css::uno::Reference< ov::XHelperInterface >& rxParent,
        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        const css::uno::Reference< css::frame::XModel >& rxModel,
        const css::uno::Reference< css::container::XIndexContainer >& rxFormIC,
        const css::uno::Reference< css::drawing::XControlShape >& rxControlShape ) throw (css::uno::RuntimeException);

    // XButton attributes
    virtual OUString SAL_CALL getCaption() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setCaption( const OUString& rCaption ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Reference< ov::excel::XFont > SAL_CALL getFont() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setFont( const css::uno::Reference< ov::excel::XFont >& rxFont ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getHorizontalAlignment() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setHorizontalAlignment( sal_Int32 nAlign ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getVerticalAlignment() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setVerticalAlignment( sal_Int32 nAlign ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getOrientation() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setOrientation( sal_Int32 nOrientation ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XButton methods
    css::uno::Reference< ov::excel::XCharacters > SAL_CALL Characters(
        const css::uno::Any& rStart, const css::uno::Any& rLength ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XHelperInterface
    VBAHELPER_DECL_XHELPERINTERFACE

protected:
    virtual OUString implGetBaseName() const SAL_OVERRIDE;
    virtual void implSetDefaultProperties() throw (css::uno::RuntimeException) SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
