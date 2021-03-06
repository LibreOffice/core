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

#pragma once

#include <ooo/vba/excel/XButton.hpp>
#include <ooo/vba/excel/XControlObject.hpp>
#include <ooo/vba/excel/XSheetObject.hpp>
#include <cppuhelper/implbase.hxx>
#include <vbahelper/vbahelperinterface.hxx>
#include "vbapalette.hxx"

namespace com::sun::star {
    namespace container { class XIndexContainer; }
    namespace drawing { class XControlShape; }
}

typedef InheritedHelperInterfaceWeakImpl< ov::excel::XCharacters > ScVbaButtonCharacters_BASE;

/** Simple implementation of the Characters symbol for drawing button objects. */
class ScVbaButtonCharacters : public ScVbaButtonCharacters_BASE
{
public:
    /// @throws css::uno::RuntimeException
    explicit ScVbaButtonCharacters(
        const css::uno::Reference< ov::XHelperInterface >& rxParent,
        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        const css::uno::Reference< css::beans::XPropertySet >& rxPropSet,
        const ScVbaPalette& rPalette,
        const css::uno::Any& rStart,
        const css::uno::Any& rLength );
    virtual ~ScVbaButtonCharacters() override;

    // XCharacters attributes
    virtual OUString SAL_CALL getCaption() override;
    virtual void SAL_CALL setCaption( const OUString& rCaption ) override;
    virtual OUString SAL_CALL getText() override;
    virtual void SAL_CALL setText( const OUString& rText ) override;
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Reference< ov::excel::XFont > SAL_CALL getFont() override;
    virtual void SAL_CALL setFont( const css::uno::Reference< ov::excel::XFont >& rxFont ) override;

    // XCharacters methods
    virtual void SAL_CALL Insert( const OUString& rString ) override;
    virtual void SAL_CALL Delete() override;

    // XHelperInterface
    VBAHELPER_DECL_XHELPERINTERFACE

private:
    /// @throws css::uno::RuntimeException
    OUString getFullString() const;
    /// @throws css::uno::RuntimeException
    void setFullString( const OUString& rString );

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
    /// @throws css::uno::RuntimeException
    explicit ScVbaSheetObjectBase(
        const css::uno::Reference< ov::XHelperInterface >& rxParent,
        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        const css::uno::Reference< css::frame::XModel >& rxModel,
        const css::uno::Reference< css::drawing::XShape >& rxShape );

    // XSheetObject attributes
    virtual double SAL_CALL getLeft() override;
    virtual void SAL_CALL setLeft( double fLeft ) override;
    virtual double SAL_CALL getTop() override;
    virtual void SAL_CALL setTop( double fTop ) override;
    virtual double SAL_CALL getWidth() override;
    virtual void SAL_CALL setWidth( double fWidth ) override;
    virtual double SAL_CALL getHeight() override;
    virtual void SAL_CALL setHeight( double fHeight ) override;
    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL setName( const OUString& rName ) override;
    virtual sal_Int32 SAL_CALL getPlacement() override;
    virtual void SAL_CALL setPlacement( sal_Int32 nPlacement ) override;
    virtual sal_Bool SAL_CALL getPrintObject() override;
    virtual void SAL_CALL setPrintObject( sal_Bool bPrintObject ) override;

    /** Sets default properties after a new object has been created.

        @throws css::uno::RuntimeException
    */
    void setDefaultProperties( sal_Int32 nIndex );

protected:
    /** Derived classes return the base name used for new objects. */
    virtual OUString implGetBaseName() const = 0;
    /** Derived classes set default properties for new drawing objects.

        @throws css::uno::RuntimeException
    */
    virtual void implSetDefaultProperties();

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
    /// @throws css::uno::RuntimeException
    explicit ScVbaControlObjectBase(
        const css::uno::Reference< ov::XHelperInterface >& rxParent,
        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        const css::uno::Reference< css::frame::XModel >& rxModel,
        const css::uno::Reference< css::container::XIndexContainer >& rxFormIC,
        const css::uno::Reference< css::drawing::XControlShape >& rxControlShape );

    // XSheetObject attributes
    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL setName( const OUString& rName ) override;
    virtual OUString SAL_CALL getOnAction() override;
    virtual void SAL_CALL setOnAction( const OUString& rMacroName ) override;
    virtual sal_Bool SAL_CALL getPrintObject() override;
    virtual void SAL_CALL setPrintObject( sal_Bool bPrintObject ) override;

    // XControlObject attributes
    virtual sal_Bool SAL_CALL getAutoSize() override;
    virtual void SAL_CALL setAutoSize( sal_Bool bAutoSize ) override;

    /// Notify that the document contains a macro event handler
    void NotifyMacroEventRead();

protected:
    /// @throws css::uno::RuntimeException
    sal_Int32 getModelIndexInForm() const;

protected:
    css::uno::Reference< css::container::XIndexContainer > mxFormIC;
    css::uno::Reference< css::beans::XPropertySet > mxControlProps;
    bool mbNotifyMacroEventRead;
};

typedef ::cppu::ImplInheritanceHelper< ScVbaControlObjectBase, ov::excel::XButton > ScVbaButton_BASE;

class ScVbaButton : public ScVbaButton_BASE
{
public:
    /// @throws css::uno::RuntimeException
    explicit ScVbaButton(
        const css::uno::Reference< ov::XHelperInterface >& rxParent,
        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        const css::uno::Reference< css::frame::XModel >& rxModel,
        const css::uno::Reference< css::container::XIndexContainer >& rxFormIC,
        const css::uno::Reference< css::drawing::XControlShape >& rxControlShape );

    // XButton attributes
    virtual OUString SAL_CALL getCaption() override;
    virtual void SAL_CALL setCaption( const OUString& rCaption ) override;
    virtual css::uno::Reference< ov::excel::XFont > SAL_CALL getFont() override;
    virtual void SAL_CALL setFont( const css::uno::Reference< ov::excel::XFont >& rxFont ) override;
    virtual sal_Int32 SAL_CALL getHorizontalAlignment() override;
    virtual void SAL_CALL setHorizontalAlignment( sal_Int32 nAlign ) override;
    virtual sal_Int32 SAL_CALL getVerticalAlignment() override;
    virtual void SAL_CALL setVerticalAlignment( sal_Int32 nAlign ) override;
    virtual sal_Int32 SAL_CALL getOrientation() override;
    virtual void SAL_CALL setOrientation( sal_Int32 nOrientation ) override;
    virtual css::uno::Any SAL_CALL getValue() override;
    virtual void SAL_CALL setValue( const css::uno::Any &nValue ) override;
    virtual OUString SAL_CALL getText() override;
    virtual void SAL_CALL setText( const OUString &aText ) override;

    // XButton methods
    css::uno::Reference< ov::excel::XCharacters > SAL_CALL Characters(
        const css::uno::Any& rStart, const css::uno::Any& rLength ) override;

    // XHelperInterface
    VBAHELPER_DECL_XHELPERINTERFACE

protected:
    virtual OUString implGetBaseName() const override;
    virtual void implSetDefaultProperties() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
