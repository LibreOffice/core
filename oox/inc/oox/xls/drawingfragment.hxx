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

#ifndef OOX_XLS_DRAWINGFRAGMENT_HXX
#define OOX_XLS_DRAWINGFRAGMENT_HXX

#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/Size.hpp>
#include "oox/drawingml/shape.hxx"
#include "oox/drawingml/shapegroupcontext.hxx"
#include "oox/ole/axcontrol.hxx"
#include "oox/ole/vbaproject.hxx"
#include "oox/vml/vmldrawing.hxx"
#include "oox/vml/vmldrawingfragment.hxx"
#include "oox/vml/vmltextbox.hxx"
#include "oox/xls/excelhandlers.hxx"

namespace oox { namespace ole {
    struct AxFontData;
    class AxMorphDataModelBase;
} }

namespace oox {
namespace xls {

// ============================================================================
// DrawingML
// ============================================================================

/** Absolute position in spreadsheet (in EMUs) independent from cells. */
struct AnchorPosModel : public ::oox::drawingml::EmuPoint
{
    inline explicit     AnchorPosModel() : ::oox::drawingml::EmuPoint( -1, -1 ) {}
    inline bool         isValid() const { return (X >= 0) && (Y >= 0); }
};

// ----------------------------------------------------------------------------

/** Absolute size in spreadsheet (in EMUs). */
struct AnchorSizeModel : public ::oox::drawingml::EmuSize
{
    inline explicit     AnchorSizeModel() : ::oox::drawingml::EmuSize( -1, -1 ) {}
    inline bool         isValid() const { return (Width >= 0) && (Height >= 0); }
};

// ----------------------------------------------------------------------------

/** Position in spreadsheet (cell position and offset inside cell in EMUs). */
struct AnchorCellModel
{
    sal_Int32           mnCol;              /// Column index.
    sal_Int32           mnRow;              /// Row index.
    sal_Int64           mnColOffset;        /// X offset in column mnCol (EMUs).
    sal_Int64           mnRowOffset;        /// Y offset in row mnRow (EMUs).

    explicit            AnchorCellModel();
    inline bool         isValid() const { return (mnCol >= 0) && (mnRow >= 0); }
};

// ----------------------------------------------------------------------------

/** Application-specific client data of a shape. */
struct AnchorClientDataModel
{
    bool                mbLocksWithSheet;
    bool                mbPrintsWithSheet;

    explicit            AnchorClientDataModel();
};

// ============================================================================

/** Contains the position of a shape in the spreadsheet. Supports different
    shape anchor modes (absolute, one-cell, two-cell). */
class ShapeAnchor : public WorksheetHelper
{
public:
    explicit            ShapeAnchor( const WorksheetHelper& rHelper );

    /** Imports the shape anchor (one of the elements xdr:absoluteAnchor, xdr:oneCellAnchor, xdr:twoCellAnchor). */
    void                importAnchor( sal_Int32 nElement, const AttributeList& rAttribs );
    /** Imports the absolute anchor position from the xdr:pos element. */
    void                importPos( const AttributeList& rAttribs );
    /** Imports the absolute anchor size from the xdr:ext element. */
    void                importExt( const AttributeList& rAttribs );
    /** Imports the shape client data from the xdr:clientData element. */
    void                importClientData( const AttributeList& rAttribs );
    /** Sets an attribute of the cell-dependent anchor position from xdr:from and xdr:to elements. */
    void                setCellPos( sal_Int32 nElement, sal_Int32 nParentContext, const ::rtl::OUString& rValue );
    /** Imports and converts the VML specific client anchor. */
    void                importVmlAnchor( const ::rtl::OUString& rAnchor );

    /** Returns true, if the anchor contains valid position and size settings. */
    bool                isValidAnchor() const;

    /** Calculates the resulting shape anchor in 1/100 mm. */
    ::com::sun::star::awt::Rectangle
                        calcApiLocation(
                            const ::com::sun::star::awt::Size& rApiSheetSize,
                            const AnchorSizeModel& rEmuSheetSize ) const;

    /** Calculates the resulting shape anchor in EMUs. */
    ::com::sun::star::awt::Rectangle
                        calcEmuLocation( const AnchorSizeModel& rEmuSheetSize ) const;

private:
    enum AnchorType { ANCHOR_ABSOLUTE, ANCHOR_ONECELL, ANCHOR_TWOCELL, ANCHOR_VML, ANCHOR_INVALID };

    AnchorType          meType;             /// Type of this shape anchor.
    AnchorPosModel      maPos;              /// Top-left position, if anchor is of type absolute.
    AnchorSizeModel     maSize;             /// Anchor size, if anchor is not of type two-cell.
    AnchorCellModel     maFrom;             /// Top-left position, if anchor is not of type absolute.
    AnchorCellModel     maTo;               /// Bottom-right position, if anchor is of type two-cell.
    AnchorClientDataModel maClientData;     /// Shape client data.
    sal_Int32           mnEditAs;           /// Anchor mode as shown in the UI.
};

typedef ::boost::shared_ptr< ShapeAnchor > ShapeAnchorRef;

// ============================================================================

class ShapeMacroAttacher : public ::oox::ole::VbaMacroAttacherBase
{
public:
    explicit            ShapeMacroAttacher( const ::rtl::OUString& rMacroName,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rxShape );

private:
    virtual void        attachMacro( const ::rtl::OUString& rMacroUrl );

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
    ::rtl::OUString     maMacroName;
};

// ============================================================================

/** Context handler for creation of shapes embedded in group shapes. */
class GroupShapeContext : public ::oox::drawingml::ShapeGroupContext, public WorksheetHelper
{
public:
    explicit            GroupShapeContext(
                            ::oox::core::ContextHandler& rParent,
                            const WorksheetHelper& rHelper,
                            const ::oox::drawingml::ShapePtr& rxParentShape,
                            const ::oox::drawingml::ShapePtr& rxShape );

    static ::oox::core::ContextHandlerRef
                        createShapeContext(
                            ::oox::core::ContextHandler& rParent,
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
                            const ::rtl::OUString& rFragmentPath );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onCharacters( const ::rtl::OUString& rChars );
    virtual void        onEndElement();

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
                        mxDrawPage;             /// Drawing page of this sheet.
    ::com::sun::star::awt::Size maApiSheetSize; /// Sheet size in 1/100 mm.
    AnchorSizeModel     maEmuSheetSize;         /// Sheet size in EMU.
    ::oox::drawingml::ShapePtr mxShape;         /// Current top-level shape.
    ShapeAnchorRef      mxAnchor;               /// Current anchor of top-level shape.
};

// ============================================================================
// VML
// ============================================================================

class VmlControlMacroAttacher : public ::oox::ole::VbaMacroAttacherBase
{
public:
    explicit            VmlControlMacroAttacher( const ::rtl::OUString& rMacroName,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >& rxCtrlFormIC,
                            sal_Int32 nCtrlIndex, sal_Int32 nCtrlType, sal_Int32 nDropStyle );

private:
    virtual void        attachMacro( const ::rtl::OUString& rMacroUrl );

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
    virtual ::rtl::OUString getShapeBaseName( const ::oox::vml::ShapeBase& rShape ) const;

    /** Calculates the shape rectangle from a cell anchor string. */
    virtual bool        convertClientAnchor(
                            ::com::sun::star::awt::Rectangle& orShapeRect,
                            const ::rtl::OUString& rShapeAnchor ) const;

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
    sal_uInt32          convertControlTextColor( const ::rtl::OUString& rTextColor ) const;
    /** Converts the passed VML textbox font to an ActiveX form control font. */
    void                convertControlFontData(
                            ::oox::ole::AxFontData& rAxFontData, sal_uInt32& rnOleTextColor,
                            const ::oox::vml::TextFontModel& rFontModel ) const;
    /** Converts the caption, the font settings, and the horizontal alignment
        from the passed VML textbox to ActiveX form control settings. */
    void                convertControlText(
                            ::oox::ole::AxFontData& rAxFontData, sal_uInt32& rnOleTextColor, ::rtl::OUString& rCaption,
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
                            const ::rtl::OUString& rFragmentPath );

protected:
    virtual void        finalizeImport();
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */