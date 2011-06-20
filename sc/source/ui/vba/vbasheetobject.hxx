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

#ifndef SC_VBA_SHEETOBJECT_HXX
#define SC_VBA_SHEETOBJECT_HXX

#include <memory>
#include <ooo/vba/excel/XButton.hpp>
#include <ooo/vba/excel/XControlObject.hpp>
#include <ooo/vba/excel/XSheetObject.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include "vbapalette.hxx"

namespace com { namespace sun { namespace star {
    namespace container { class XIndexContainer; }
    namespace drawing { class XControlShape; }
} } }

// ============================================================================

typedef InheritedHelperInterfaceImpl1< ov::excel::XCharacters > ScVbaButtonCharacters_BASE;

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
    virtual ::rtl::OUString SAL_CALL getCaption() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setCaption( const ::rtl::OUString& rCaption ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getText() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setText( const ::rtl::OUString& rText ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getCount() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::excel::XFont > SAL_CALL getFont() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setFont( const css::uno::Reference< ov::excel::XFont >& rxFont ) throw (css::uno::RuntimeException);

    // XCharacters methods
    virtual void SAL_CALL Insert( const ::rtl::OUString& rString ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Delete() throw (css::uno::RuntimeException);

    // XHelperInterface
    VBAHELPER_DECL_XHELPERINTERFACE

private:
    ::rtl::OUString getFullString() const throw (css::uno::RuntimeException);
    void setFullString( const ::rtl::OUString& rString ) throw (css::uno::RuntimeException);

private:
    ScVbaPalette maPalette;
    css::uno::Reference< css::beans::XPropertySet > mxPropSet;
    sal_Int32 mnStart;
    sal_Int32 mnLength;
};

// ============================================================================

typedef InheritedHelperInterfaceImpl1< ov::excel::XSheetObject > ScVbaSheetObject_BASE;

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
    virtual double SAL_CALL getLeft() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setLeft( double fLeft ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getTop() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setTop( double fTop ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getWidth() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setWidth( double fWidth ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getHeight() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setHeight( double fHeight ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setName( const ::rtl::OUString& rName ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getPlacement() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setPlacement( sal_Int32 nPlacement ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getPrintObject() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setPrintObject( sal_Bool bPrintObject ) throw (css::uno::RuntimeException);

    /** Sets default properties after a new object has been created. */
    void setDefaultProperties( sal_Int32 nIndex ) throw (css::uno::RuntimeException);

protected:
    /** Derived classes return the base name used for new objects. */
    virtual ::rtl::OUString implGetBaseName() const = 0;
    /** Derived classes set default properties for new drawing objects. */
    virtual void implSetDefaultProperties() throw (css::uno::RuntimeException);

protected:
    ScVbaPalette maPalette;
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::drawing::XShape > mxShape;
    css::uno::Reference< css::beans::XPropertySet > mxShapeProps;
};

// ============================================================================

typedef ::cppu::ImplInheritanceHelper1< ScVbaSheetObjectBase, ov::excel::XControlObject > ScVbaControlObject_BASE;

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
    virtual ::rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setName( const ::rtl::OUString& rName ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getOnAction() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setOnAction( const ::rtl::OUString& rMacroName ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getPrintObject() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setPrintObject( sal_Bool bPrintObject ) throw (css::uno::RuntimeException);

    // XControlObject attributes
    virtual sal_Bool SAL_CALL getAutoSize() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setAutoSize( sal_Bool bAutoSize ) throw (css::uno::RuntimeException);

protected:
    sal_Int32 getModelIndexInForm() const throw (css::uno::RuntimeException);

protected:
    css::uno::Reference< css::container::XIndexContainer > mxFormIC;
    css::uno::Reference< css::beans::XPropertySet > mxControlProps;
    ::rtl::OUString maListenerType;
    ::rtl::OUString maEventMethod;
};

// ============================================================================

typedef ::cppu::ImplInheritanceHelper1< ScVbaControlObjectBase, ov::excel::XButton > ScVbaButton_BASE;

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
    virtual ::rtl::OUString SAL_CALL getCaption() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setCaption( const ::rtl::OUString& rCaption ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::excel::XFont > SAL_CALL getFont() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setFont( const css::uno::Reference< ov::excel::XFont >& rxFont ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getHorizontalAlignment() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setHorizontalAlignment( sal_Int32 nAlign ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getVerticalAlignment() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setVerticalAlignment( sal_Int32 nAlign ) throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getOrientation() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setOrientation( sal_Int32 nOrientation ) throw (css::uno::RuntimeException);

    // XButton methods
    css::uno::Reference< ov::excel::XCharacters > SAL_CALL Characters(
        const css::uno::Any& rStart, const css::uno::Any& rLength ) throw (css::uno::RuntimeException);

    // XHelperInterface
    VBAHELPER_DECL_XHELPERINTERFACE

protected:
    virtual ::rtl::OUString implGetBaseName() const;
    virtual void implSetDefaultProperties() throw (css::uno::RuntimeException);
};

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
