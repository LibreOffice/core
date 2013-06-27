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

#ifndef OOX_XLS_DRAWINGFRAGMENT_HXX
#define OOX_XLS_DRAWINGFRAGMENT_HXX

#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/Size.hpp>
#include "oox/drawingml/shapegroupcontext.hxx"
#include "oox/ole/axcontrol.hxx"
#include "oox/drawingml/shape.hxx"
#include "oox/ole/vbaproject.hxx"
#include "oox/vml/vmldrawing.hxx"
#include "oox/vml/vmldrawingfragment.hxx"
#include "oox/vml/vmltextbox.hxx"
#include "drawingbase.hxx"
#include "excelhandlers.hxx"

namespace oox { namespace ole {
    struct AxFontData;
    class AxMorphDataModelBase;
} }

namespace oox {
namespace xls {

// ============================================================================
// DrawingML
// ============================================================================

class ShapeMacroAttacher : public ::oox::ole::VbaMacroAttacherBase
{
public:
    explicit            ShapeMacroAttacher( const OUString& rMacroName,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rxShape );

private:
    virtual void        attachMacro( const OUString& rMacroUrl );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > mxShape;
};

// ============================================================================

class Shape : public ::oox::drawingml::Shape, public WorksheetHelper
{
public:
    explicit            Shape(
                            const WorksheetHelper& rHelper,
                            const AttributeList& rAttribs,
                            const sal_Char* pcServiceName = 0 );

protected:
    virtual void        finalizeXShape(
                            ::oox::core::XmlFilterBase& rFilter,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes );

private:
    OUString     maMacroName;
};

// ============================================================================

/** Context handler for creation of shapes embedded in group shapes. */
class GroupShapeContext : public ::oox::drawingml::ShapeGroupContext, public WorksheetHelper
{
public:
    explicit            GroupShapeContext(
                            ::oox::core::ContextHandler2Helper& rParent,
                            const WorksheetHelper& rHelper,
                            const ::oox::drawingml::ShapePtr& rxParentShape,
                            const ::oox::drawingml::ShapePtr& rxShape );

    static ::oox::core::ContextHandlerRef
                        createShapeContext(
                            ::oox::core::ContextHandler2Helper& rParent,
                            const WorksheetHelper& rHelper,
                            sal_Int32 nElement,
                            const AttributeList& rAttribs,
                            const ::oox::drawingml::ShapePtr& rxParentShape,
                            ::oox::drawingml::ShapePtr* pxShape = 0 );

protected:
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL
                        createFastChildContext(
                            sal_Int32 nElement,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& rxAttribs )
                        throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
};

// ============================================================================

/** Fragment handler for a complete sheet drawing. */
class DrawingFragment : public WorksheetFragmentBase
{
public:
    explicit            DrawingFragment(
                            const WorksheetHelper& rHelper,
                            const OUString& rFragmentPath );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onCharacters( const OUString& rChars );
    virtual void        onEndElement();

private:
    typedef ::std::auto_ptr< ShapeAnchor > ShapeAnchorRef;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
                        mxDrawPage;             /// Drawing page of this sheet.
    ::oox::drawingml::ShapePtr mxShape;         /// Current top-level shape.
    ShapeAnchorRef      mxAnchor;               /// Current anchor of top-level shape.
};

// ============================================================================
// VML
// ============================================================================

class VmlControlMacroAttacher : public ::oox::ole::VbaMacroAttacherBase
{
public:
    explicit            VmlControlMacroAttacher( const OUString& rMacroName,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >& rxCtrlFormIC,
                            sal_Int32 nCtrlIndex, sal_Int32 nCtrlType, sal_Int32 nDropStyle );

private:
    virtual void        attachMacro( const OUString& rMacroUrl );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > mxCtrlFormIC;
    sal_Int32           mnCtrlIndex;
    sal_Int32           mnCtrlType;
    sal_Int32           mnDropStyle;
};

// ============================================================================

class VmlDrawing : public ::oox::vml::Drawing, public WorksheetHelper
{
public:
    explicit            VmlDrawing( const WorksheetHelper& rHelper );

    /** Returns the drawing shape for a cell note at the specified position. */
    const ::oox::vml::ShapeBase* getNoteShape( const ::com::sun::star::table::CellAddress& rPos ) const;

    /** Filters cell note shapes. */
    virtual bool        isShapeSupported( const ::oox::vml::ShapeBase& rShape ) const;

    /** Returns additional base names for automatic shape name creation. */
    virtual OUString getShapeBaseName( const ::oox::vml::ShapeBase& rShape ) const;

    /** Calculates the shape rectangle from a cell anchor string. */
    virtual bool        convertClientAnchor(
                            ::com::sun::star::awt::Rectangle& orShapeRect,
                            const OUString& rShapeAnchor ) const;

    /** Creates a UNO control shape for legacy drawing controls. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                        createAndInsertClientXShape(
                            const ::oox::vml::ShapeBase& rShape,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ::com::sun::star::awt::Rectangle& rShapeRect ) const;

    /** Updates the bounding box covering all shapes of this drawing. */
    virtual void        notifyXShapeInserted(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rxShape,
                            const ::com::sun::star::awt::Rectangle& rShapeRect,
                            const ::oox::vml::ShapeBase& rShape, bool bGroupChild );

private:
    /** Converts the passed VML textbox text color to an OLE color. */
    sal_uInt32          convertControlTextColor( const OUString& rTextColor ) const;
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

// ============================================================================

class VmlDrawingFragment : public ::oox::vml::DrawingFragment, public WorksheetHelper
{
public:
    explicit            VmlDrawingFragment(
                            const WorksheetHelper& rHelper,
                            const OUString& rFragmentPath );

protected:
    virtual void        finalizeImport();
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
