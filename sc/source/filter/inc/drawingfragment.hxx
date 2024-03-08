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

#include <memory>
#include <oox/drawingml/shapegroupcontext.hxx>
#include <oox/drawingml/color.hxx>
#include <oox/ole/axcontrol.hxx>
#include <oox/drawingml/shape.hxx>
#include <oox/ole/vbaproject.hxx>
#include <oox/vml/vmldrawing.hxx>
#include <oox/vml/vmldrawingfragment.hxx>
#include <oox/vml/vmltextbox.hxx>
#include "drawingbase.hxx"
#include "excelhandlers.hxx"

namespace oox::ole {
    struct AxFontData;
}

namespace oox::xls {

// DrawingML

class ShapeMacroAttacher final : public ::oox::ole::VbaMacroAttacherBase
{
public:
    explicit            ShapeMacroAttacher( const OUString& rMacroName,
                            const css::uno::Reference< css::drawing::XShape >& rxShape );

private:
    virtual void        attachMacro( const OUString& rMacroUrl ) override;

private:
    css::uno::Reference< css::drawing::XShape > mxShape;
};

class Shape final : public ::oox::drawingml::Shape, public WorksheetHelper
{
public:
    explicit            Shape(
                            const WorksheetHelper& rHelper,
                            const AttributeList& rAttribs,
                            const OUString& rServiceName );

private:
    virtual void        finalizeXShape(
                            ::oox::core::XmlFilterBase& rFilter,
                            const css::uno::Reference< css::drawing::XShapes >& rxShapes ) override;

    OUString     maMacroName;
};

/** Context handler for creation of shapes embedded in group shapes. */
class GroupShapeContext final : public ::oox::drawingml::ShapeGroupContext, public WorksheetHelper
{
public:
    explicit            GroupShapeContext(
                            const ::oox::core::FragmentHandler2& rParent,
                            const WorksheetHelper& rHelper,
                            const ::oox::drawingml::ShapePtr& rxParentShape,
                            const ::oox::drawingml::ShapePtr& rxShape );

    static ::oox::core::ContextHandlerRef
                        createShapeContext(
                            ::oox::core::FragmentHandler2& rParent,
                            const WorksheetHelper& rHelper,
                            sal_Int32 nElement,
                            const AttributeList& rAttribs,
                            const ::oox::drawingml::ShapePtr& rxParentShape,
                            ::oox::drawingml::ShapePtr* pxShape = nullptr );

private:
    virtual ::oox::core::ContextHandlerRef
                        onCreateContext(
                            sal_Int32 nElement,
                            const ::oox::AttributeList& rAttribs ) override;
};

/** Fragment handler for a complete sheet drawing. */
class DrawingFragment final : public WorksheetFragmentBase
{
public:
    explicit            DrawingFragment(
                            const WorksheetHelper& rHelper,
                            const OUString& rFragmentPath );

private:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual void        onCharacters( const OUString& rChars ) override;
    virtual void        onEndElement() override;

    typedef ::std::unique_ptr< ShapeAnchor > ShapeAnchorRef;

    css::uno::Reference< css::drawing::XShapes >
                        mxDrawPage;             /// Drawing page of this sheet.
    ::oox::drawingml::ShapePtr mxShape;         /// Current top-level shape.
    ShapeAnchorRef      mxAnchor;               /// Current anchor of top-level shape.
    // for SmartArt. Apply the colors in rFontRefColor (from clrScheme) to all shapes in pShape,
    // the group object which represents the SmartArt.
    void applyFontRefColor(const oox::drawingml::ShapePtr& pShape,
                           const oox::drawingml::Color& rFontRefColor);
};

// VML

class VmlControlMacroAttacher final : public ::oox::ole::VbaMacroAttacherBase
{
public:
    explicit            VmlControlMacroAttacher( const OUString& rMacroName,
                            const css::uno::Reference< css::container::XIndexContainer >& rxCtrlFormIC,
                            sal_Int32 nCtrlIndex, sal_Int32 nCtrlType, sal_Int32 nDropStyle );

private:
    virtual void        attachMacro( const OUString& rMacroUrl ) override;

private:
    css::uno::Reference< css::container::XIndexContainer > mxCtrlFormIC;
    sal_Int32           mnCtrlIndex;
    sal_Int32           mnCtrlType;
    sal_Int32           mnDropStyle;
};

class VmlDrawing final : public ::oox::vml::Drawing, public WorksheetHelper
{
public:
    explicit            VmlDrawing( const WorksheetHelper& rHelper );

    /** Returns the drawing shape for a cell note at the specified position. */
    const ::oox::vml::ShapeBase* getNoteShape( const ScAddress& rPos ) const;

    /** Filters cell note shapes. */
    virtual bool        isShapeSupported( const ::oox::vml::ShapeBase& rShape ) const override;

    /** Returns additional base names for automatic shape name creation. */
    virtual OUString getShapeBaseName( const ::oox::vml::ShapeBase& rShape ) const override;

    /** Calculates the shape rectangle from a cell anchor string. */
    virtual bool        convertClientAnchor(
                            css::awt::Rectangle& orShapeRect,
                            const OUString& rShapeAnchor ) const override;

    /** Creates a UNO control shape for legacy drawing controls. */
    virtual css::uno::Reference< css::drawing::XShape >
                        createAndInsertClientXShape(
                            const ::oox::vml::ShapeBase& rShape,
                            const css::uno::Reference< css::drawing::XShapes >& rxShapes,
                            const css::awt::Rectangle& rShapeRect ) const override;

    /** Updates the bounding box covering all shapes of this drawing. */
    virtual void        notifyXShapeInserted(
                            const css::uno::Reference< css::drawing::XShape >& rxShape,
                            const css::awt::Rectangle& rShapeRect,
                            const ::oox::vml::ShapeBase& rShape, bool bGroupChild ) override;

private:
    /** Converts the passed VML textbox text color to an OLE color. */
    sal_uInt32          convertControlTextColor( std::u16string_view aTextColor ) const;
    /** Converts the passed VML textbox font to an ActiveX form control font. */
    void                convertControlFontData(
                            ::oox::ole::AxFontData& rAxFontData, sal_uInt32& rnOleTextColor,
                            const ::oox::vml::TextFontModel& rFontModel ) const;
    /** Converts the caption, the font settings, and the horizontal alignment
        from the passed VML textbox to ActiveX form control settings. */
    void                convertControlText(
                            ::oox::ole::AxFontData& rAxFontData, sal_uInt32& rnOleTextColor, OUString& rCaption,
                            const ::oox::vml::TextBox* pTextBox, sal_Int32 nTextHAlign ) const;
    /** Converts the passed VML shape background formatting to ActiveX control formatting. */
    void                convertControlBackground(
                            ::oox::ole::AxMorphDataModelBase& rAxModel,
                            const ::oox::vml::ShapeBase& rShape ) const;

private:
    ::oox::ole::ControlConverter maControlConv;
    ::oox::vml::TextFontModel maListBoxFont;
};

class VmlDrawingFragment final : public ::oox::vml::DrawingFragment, public WorksheetHelper
{
public:
    explicit            VmlDrawingFragment(
                            const WorksheetHelper& rHelper,
                            const OUString& rFragmentPath );

private:
    virtual void        finalizeImport() override;
};

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
