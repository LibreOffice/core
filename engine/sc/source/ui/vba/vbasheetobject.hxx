/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
    /// @throws cpo::uno::RuntimeException
    explicit ScVbaButtonCharacters(
        const cpo::uno::Reference< ov::XHelperInterface >& rxParent,
        const cpo::uno::Reference< cpo::uno::XComponentContext >& rxContext,
        const cpo::uno::Reference< css::beans::XPropertySet >& rxPropSet,
        const ScVbaPalette& rPalette,
        const cpo::uno::Any& rStart,
        const cpo::uno::Any& rLength );
    virtual ~ScVbaButtonCharacters() override;

    // XCharacters attributes
    virtual OUString getCaption() override;
    virtual void setCaption( const OUString& rCaption ) override;
    virtual OUString getText() override;
    virtual void setText( const OUString& rText ) override;
    virtual sal_Int32 getCount() override;
    virtual cpo::uno::Reference< ov::excel::XFont > getFont() override;
    virtual void setFont( const cpo::uno::Reference< ov::excel::XFont >& rxFont ) override;

    // XCharacters methods
    virtual void Insert( const OUString& rString ) override;
    virtual void Delete() override;

    // XHelperInterface
    VBAHELPER_DECL_XHELPERINTERFACE

private:
    /// @throws cpo::uno::RuntimeException
    OUString getFullString() const;
    /// @throws cpo::uno::RuntimeException
    void setFullString( const OUString& rString );

private:
    ScVbaPalette maPalette;
    cpo::uno::Reference< css::beans::XPropertySet > mxPropSet;
    sal_Int32 mnStart;
    sal_Int32 mnLength;
};

typedef InheritedHelperInterfaceWeakImpl< ov::excel::XSheetObject > ScVbaSheetObject_BASE;

/** Base class for drawing objects embedded in sheets. */
class ScVbaSheetObjectBase : public ScVbaSheetObject_BASE
{
public:
    /// @throws cpo::uno::RuntimeException
    explicit ScVbaSheetObjectBase(
        const cpo::uno::Reference< ov::XHelperInterface >& rxParent,
        const cpo::uno::Reference< cpo::uno::XComponentContext >& rxContext,
        const cpo::uno::Reference< css::frame::XModel >& rxModel,
        const cpo::uno::Reference< css::drawing::XShape >& rxShape );

    // XSheetObject attributes
    virtual double getLeft() override;
    virtual void setLeft( double fLeft ) override;
    virtual double getTop() override;
    virtual void setTop( double fTop ) override;
    virtual double getWidth() override;
    virtual void setWidth( double fWidth ) override;
    virtual double getHeight() override;
    virtual void setHeight( double fHeight ) override;
    virtual OUString getName() override;
    virtual void setName( const OUString& rName ) override;
    virtual sal_Int32 getPlacement() override;
    virtual void setPlacement( sal_Int32 nPlacement ) override;
    virtual bool getPrintObject() override;
    virtual void setPrintObject( bool bPrintObject ) override;

    /** Sets default properties after a new object has been created.

        @throws cpo::uno::RuntimeException
    */
    void setDefaultProperties( sal_Int32 nIndex );

protected:
    /** Derived classes return the base name used for new objects. */
    virtual OUString implGetBaseName() const = 0;
    /** Derived classes set default properties for new drawing objects.

        @throws cpo::uno::RuntimeException
    */
    virtual void implSetDefaultProperties();

protected:
    ScVbaPalette maPalette;
    cpo::uno::Reference< css::frame::XModel > mxModel;
    cpo::uno::Reference< css::drawing::XShape > mxShape;
    cpo::uno::Reference< css::beans::XPropertySet > mxShapeProps;
};

typedef ::cppu::ImplInheritanceHelper< ScVbaSheetObjectBase, ov::excel::XControlObject > ScVbaControlObject_BASE;

class ScVbaControlObjectBase : public ScVbaControlObject_BASE
{
public:
    /// @throws cpo::uno::RuntimeException
    explicit ScVbaControlObjectBase(
        const cpo::uno::Reference< ov::XHelperInterface >& rxParent,
        const cpo::uno::Reference< cpo::uno::XComponentContext >& rxContext,
        const cpo::uno::Reference< css::frame::XModel >& rxModel,
        const cpo::uno::Reference< css::container::XIndexContainer >& rxFormIC,
        const cpo::uno::Reference< css::drawing::XControlShape >& rxControlShape );

    // XSheetObject attributes
    virtual OUString getName() override;
    virtual void setName( const OUString& rName ) override;
    virtual OUString getOnAction() override;
    virtual void setOnAction( const OUString& rMacroName ) override;
    virtual bool getPrintObject() override;
    virtual void setPrintObject( bool bPrintObject ) override;

    // XControlObject attributes
    virtual bool getAutoSize() override;
    virtual void setAutoSize( bool bAutoSize ) override;

    /// Notify that the document contains a macro event handler
    void NotifyMacroEventRead();

protected:
    /// @throws cpo::uno::RuntimeException
    sal_Int32 getModelIndexInForm() const;

protected:
    cpo::uno::Reference< css::container::XIndexContainer > mxFormIC;
    cpo::uno::Reference< css::beans::XPropertySet > mxControlProps;
    bool mbNotifyMacroEventRead;
};

typedef ::cppu::ImplInheritanceHelper< ScVbaControlObjectBase, ov::excel::XButton > ScVbaButton_BASE;

class ScVbaButton : public ScVbaButton_BASE
{
public:
    /// @throws cpo::uno::RuntimeException
    explicit ScVbaButton(
        const cpo::uno::Reference< ov::XHelperInterface >& rxParent,
        const cpo::uno::Reference< cpo::uno::XComponentContext >& rxContext,
        const cpo::uno::Reference< css::frame::XModel >& rxModel,
        const cpo::uno::Reference< css::container::XIndexContainer >& rxFormIC,
        const cpo::uno::Reference< css::drawing::XControlShape >& rxControlShape );

    // XButton attributes
    virtual OUString getCaption() override;
    virtual void setCaption( const OUString& rCaption ) override;
    virtual cpo::uno::Reference< ov::excel::XFont > getFont() override;
    virtual void setFont( const cpo::uno::Reference< ov::excel::XFont >& rxFont ) override;
    virtual sal_Int32 getHorizontalAlignment() override;
    virtual void setHorizontalAlignment( sal_Int32 nAlign ) override;
    virtual sal_Int32 getVerticalAlignment() override;
    virtual void setVerticalAlignment( sal_Int32 nAlign ) override;
    virtual sal_Int32 getOrientation() override;
    virtual void setOrientation( sal_Int32 nOrientation ) override;
    virtual cpo::uno::Any getValue() override;
    virtual void setValue( const cpo::uno::Any &nValue ) override;
    virtual OUString getText() override;
    virtual void setText( const OUString &aText ) override;

    // XButton methods
    cpo::uno::Reference< ov::excel::XCharacters > Characters(
        const cpo::uno::Any& rStart, const cpo::uno::Any& rLength ) override;

    // XHelperInterface
    VBAHELPER_DECL_XHELPERINTERFACE

protected:
    virtual OUString implGetBaseName() const override;
    virtual void implSetDefaultProperties() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
