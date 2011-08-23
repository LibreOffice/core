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

#ifndef OOX_VML_VMLSHAPE_HXX
#define OOX_VML_VMLSHAPE_HXX

#include <memory>
#include <vector>
#include <com/sun/star/awt/Point.hpp>
#include "oox/vml/vmlformatting.hxx"

namespace com { namespace sun { namespace star {
    namespace awt { struct Rectangle; }
    namespace drawing { class XShape; }
    namespace drawing { class XShapes; }
} } }

namespace oox {
namespace vml {

class Drawing;
struct ShapeParentAnchor;
class ShapeContainer;

// ============================================================================

/** The shape model structure contains all properties shared by all types of shapes. */
struct ShapeTypeModel
{
    ::rtl::OUString     maShapeId;              /// Unique identifier of the shape.
    ::rtl::OUString     maName;                 /// Name of the shape, if present.
    OptValue< sal_Int32 > moShapeType;          /// Builtin shape type identifier.

    OptValue< Int32Pair > moCoordPos;           /// Top-left position of coordinate system for children scaling.
    OptValue< Int32Pair > moCoordSize;          /// Size of coordinate system for children scaling.
    ::rtl::OUString     maPosition;             /// Position type of the shape.
    ::rtl::OUString     maLeft;                 /// X position of the shape bounding box (number with unit).
    ::rtl::OUString     maTop;                  /// Y position of the shape bounding box (number with unit).
    ::rtl::OUString     maWidth;                /// Width of the shape bounding box (number with unit).
    ::rtl::OUString     maHeight;               /// Height of the shape bounding box (number with unit).
    ::rtl::OUString     maMarginLeft;           /// X position of the shape bounding box to shape anchor (number with unit).
    ::rtl::OUString     maMarginTop;            /// Y position of the shape bounding box to shape anchor (number with unit).

    StrokeModel         maStrokeModel;          /// Border line formatting.
    FillModel           maFillModel;            /// Shape fill formatting.

    OptValue< ::rtl::OUString > moGraphicPath;  /// Path to a graphic for this shape.
    OptValue< ::rtl::OUString > moGraphicTitle; /// Title of the graphic.

    explicit            ShapeTypeModel();

    void                assignUsed( const ShapeTypeModel& rSource );
};

// ----------------------------------------------------------------------------

/** A shape template contains all formatting properties of shapes and can serve
    as templates for several shapes in a drawing. */
class ShapeType
{
public:
    explicit            ShapeType( Drawing& rDrawing );
    virtual             ~ShapeType();

    /** Returns read/write access to the shape template model structure. */
    inline ShapeTypeModel& getTypeModel() { return maTypeModel; }
    /** Returns read access to the shape template model structure. */
    inline const ShapeTypeModel& getTypeModel() const { return maTypeModel; }

    /** Returns the shape identifier (which is unique through the containing drawing). */
    inline const ::rtl::OUString& getShapeId() const { return maTypeModel.maShapeId; }
    /** Returns the fragment path to the embedded graphic used by this shape. */
    ::rtl::OUString     getGraphicPath() const;

protected:
    /** Returns the coordinate system of this shape. */
    ::com::sun::star::awt::Rectangle getCoordSystem() const;
    /** Returns the absolute shape rectangle according to the passed anchor. */
    ::com::sun::star::awt::Rectangle getRectangle( const ShapeParentAnchor* pParentAnchor ) const;

private:
    /** Returns the absolute shape rectangle. */
    ::com::sun::star::awt::Rectangle getAbsRectangle() const;
    /** Returns the rectangle relative to the parent coordinate system. */
    ::com::sun::star::awt::Rectangle getRelRectangle() const;

protected:
    Drawing&            mrDrawing;          /// The VML drawing page that contains this shape.
    ShapeTypeModel      maTypeModel;        /// The model structure containing shape type data.
};

// ============================================================================

/** Excel specific shape client data (such as cell anchor). */
struct ShapeClientData
{
    ::rtl::OUString     maAnchor;           /// Cell anchor as comma-separated string.
    ::rtl::OUString     maPictureLink;      /// Target cell range of picture links.
    ::rtl::OUString     maLinkedCell;       /// Link to value cell associated to the control.
    ::rtl::OUString     maSourceRange;      /// Link to cell range used as data source for the control.
    sal_Int32           mnObjType;          /// Type of the shape.
    sal_Int32           mnCol;              /// Column index for spreadsheet cell note.
    sal_Int32           mnRow;              /// Row index for spreadsheet cell note.
    bool                mbPrintObject;      /// True = print the object.
    bool                mbVisible;          /// True = cell note is visible.

    explicit            ShapeClientData();
};

// ----------------------------------------------------------------------------

struct ShapeModel
{
    typedef ::std::vector< ::com::sun::star::awt::Point >   PointVector;
    typedef ::std::auto_ptr< ShapeClientData >              ShapeClientDataPtr;

    ::rtl::OUString     maType;             /// Shape template with default properties.
    PointVector         maPoints;           /// Points for the polyline shape.
    ShapeClientDataPtr  mxClientData;       /// Excel specific shape client data.

    explicit            ShapeModel();

    /** Creates and returns a new shape client data structure. */
    ShapeClientData&    createClientData();
};

// ----------------------------------------------------------------------------

/** A shape object that is part of a drawing. May inherit properties from a
    shape template. */
class ShapeBase : public ShapeType
{
public:
    /** Returns read/write access to the shape model structure. */
    inline ShapeModel&  getShapeModel() { return maShapeModel; }
    /** Returns read access to the shape model structure. */
    inline const ShapeModel& getShapeModel() const { return maShapeModel; }

    /** Final processing after import of the drawing fragment. */
    virtual void        finalizeFragmentImport();

    /** Returns the shape template with the passed identifier from the child shapes. */
    virtual const ShapeType* getChildTypeById( const ::rtl::OUString& rShapeId ) const;
    /** Returns the shape with the passed identifier from the child shapes. */
    virtual const ShapeBase* getChildById( const ::rtl::OUString& rShapeId ) const;

    /** Creates the corresponding XShape and inserts it into the passed container. */
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                        convertAndInsert(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ShapeParentAnchor* pParentAnchor = 0 ) const;

    /** Converts position and formatting into the passed existing XShape. */
    void                convertFormatting(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rxShape,
                            const ShapeParentAnchor* pParentAnchor = 0 ) const;

protected:
    explicit            ShapeBase( Drawing& rDrawing );

    /** Derived classes create the corresponding XShape and insert it into the passed container. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                        implConvertAndInsert(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ::com::sun::star::awt::Rectangle& rShapeRect ) const = 0;

    /** Calculates the final shape rectangle according to the passed anchor,
        if present, otherwise according to the own anchor settings. */
    ::com::sun::star::awt::Rectangle calcShapeRectangle(
                            const ShapeParentAnchor* pParentAnchor ) const;

    /** Converts common shape properties such as formatting attributes. */
    void                convertShapeProperties(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rxShape ) const;

protected:
    ShapeModel          maShapeModel;       /// The model structure containing shape data.
};

// ============================================================================

/** A simple shape object based on a specific UNO shape service. */
class SimpleShape : public ShapeBase
{
public:
    explicit            SimpleShape( Drawing& rDrawing, const ::rtl::OUString& rService );

protected:
    /** Creates the corresponding XShape and inserts it into the passed container. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                        implConvertAndInsert(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ::com::sun::star::awt::Rectangle& rShapeRect ) const;

private:
    ::rtl::OUString     maService;          /// Name of the UNO shape service.
};

// ============================================================================

/** A rectangular shape object. */
class RectangleShape : public SimpleShape
{
public:
    explicit            RectangleShape( Drawing& rDrawing );
};

// ============================================================================

/** An oval shape object. */
class EllipseShape : public SimpleShape
{
public:
    explicit            EllipseShape( Drawing& rDrawing );
};

// ============================================================================

/** A polygon shape object. */
class PolyLineShape : public SimpleShape
{
public:
    explicit            PolyLineShape( Drawing& rDrawing );

protected:
    /** Creates the corresponding XShape and inserts it into the passed container. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                        implConvertAndInsert(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ::com::sun::star::awt::Rectangle& rShapeRect ) const;
};

// ============================================================================

/** A shape object with custom geometry. */
class CustomShape : public SimpleShape
{
public:
    explicit            CustomShape( Drawing& rDrawing );

protected:
    /** Creates the corresponding XShape and inserts it into the passed container. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                        implConvertAndInsert(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ::com::sun::star::awt::Rectangle& rShapeRect ) const;
};

// ============================================================================

/** A complex shape object. This can be a picture shape, a custom shape, an OLE
    object, or an ActiveX form control. */
class ComplexShape : public CustomShape
{
public:
    explicit            ComplexShape( Drawing& rDrawing );

protected:
    /** Creates the corresponding XShape and inserts it into the passed container. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                        implConvertAndInsert(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ::com::sun::star::awt::Rectangle& rShapeRect ) const;
};

// ============================================================================

/** A group shape that extends the basic shape by a container of child shapes. */
class GroupShape : public ShapeBase
{
public:
    explicit            GroupShape( Drawing& rDrawing );
    virtual             ~GroupShape();

    /** Returns read/write access to the container of child shapes and templates. */
    inline ShapeContainer& getChildren() { return *mxChildren; }
    /** Returns read access to the container of child shapes and templates. */
    inline const ShapeContainer& getChildren() const { return *mxChildren; }

    /** Final processing after import of the drawing fragment. */
    virtual void        finalizeFragmentImport();

    /** Returns the shape template with the passed identifier from the child shapes. */
    virtual const ShapeType* getChildTypeById( const ::rtl::OUString& rShapeId ) const;
    /** Returns the shape with the passed identifier from the child shapes. */
    virtual const ShapeBase* getChildById( const ::rtl::OUString& rShapeId ) const;

protected:
    /** Creates the corresponding XShape and inserts it into the passed container. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                        implConvertAndInsert(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ::com::sun::star::awt::Rectangle& rShapeRect ) const;

private:
    typedef ::std::auto_ptr< ShapeContainer > ShapeContainerPtr;
    ShapeContainerPtr   mxChildren;         /// Shapes and templates that are part of this group.
};

// ============================================================================

} // namespace vml
} // namespace oox

#endif

