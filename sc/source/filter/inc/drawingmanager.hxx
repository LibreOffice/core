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

#ifndef OOX_XLS_DRAWINGMANAGER_HXX
#define OOX_XLS_DRAWINGMANAGER_HXX

#include "drawingbase.hxx"

namespace com { namespace sun { namespace star {
    namespace drawing { class XDrawPage; }
    namespace drawing { class XShape; }
    namespace drawing { class XShapes; }
} } }

namespace oox { namespace drawingml { class ShapePropertyMap; } }

namespace oox {
namespace xls {

// ============================================================================

const sal_uInt16 BIFF_OBJ_INVALID_ID        = 0;

// ============================================================================
// Model structures for BIFF OBJ record data
// ============================================================================

/** This structure contains line formatting attributes from an OBJ record. */
struct BiffObjLineModel
{
    sal_uInt8           mnColorIdx;         /// Index into color palette.
    sal_uInt8           mnStyle;            /// Line dash style.
    sal_uInt8           mnWidth;            /// Line width.
    bool                mbAuto;             /// True = automatic line format.

    explicit            BiffObjLineModel();
};

// ============================================================================

/** This structure contains fill formatting attributes from an OBJ record. */
struct BiffObjFillModel
{
    sal_uInt8           mnBackColorIdx;     /// Index to color palette for background color.
    sal_uInt8           mnPattColorIdx;     /// Index to color palette for pattern foreground color.
    sal_uInt8           mnPattern;          /// Fill pattern.
    bool                mbAuto;             /// True = automatic fill format.

    explicit            BiffObjFillModel();
};

// ============================================================================
// BIFF drawing objects
// ============================================================================

class BiffDrawingBase;
class BiffDrawingObjectBase;
typedef ::boost::shared_ptr< BiffDrawingObjectBase > BiffDrawingObjectRef;

// ----------------------------------------------------------------------------

class BiffDrawingObjectContainer
{
public:
    explicit            BiffDrawingObjectContainer();

    /** Returns true, if the object list is empty. */
    inline bool         empty() const { return maObjects.empty(); }

    /** Creates and inserts all UNO shapes into the passed shape container. */
    void                convertAndInsert( BiffDrawingBase& rDrawing,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ::com::sun::star::awt::Rectangle* pParentRect = 0 ) const;

private:
    typedef RefVector< BiffDrawingObjectBase > BiffDrawingObjectVector;
    BiffDrawingObjectVector maObjects;
};

// ============================================================================

/** Base class for all BIFF drawing objects (OBJ records). */
class BiffDrawingObjectBase : public WorksheetHelper
{
public:
    explicit            BiffDrawingObjectBase( const WorksheetHelper& rHelper );
    virtual             ~BiffDrawingObjectBase();

    /** Sets whether this is an area object (then its width and height must be greater than 0). */
    inline void         setAreaObj( bool bAreaObj ) { mbAreaObj = bAreaObj; }
    /** If set to true, the object supports a simple on-click macro and/or hyperlink. */
    inline void         setSimpleMacro( bool bMacro ) { mbSimpleMacro = bMacro; }

    /** If set to false, the UNO shape will not be created, processed, or inserted into the draw page. */
    inline void         setProcessShape( bool bProcess ) { mbProcessShape = bProcess; }
    /** If set to false, the UNO shape will be created or processed, but not be inserted into the draw page. */
    inline void         setInsertShape( bool bInsert ) { mbInsertShape = bInsert; }
    /** If set to true, a new custom UNO shape will be created while in DFF import (BIFF8 only). */
    inline void         setCustomDffObj( bool bCustom ) { mbCustomDff = bCustom; }

    /** Returns the object identifier from the OBJ record. */
    inline sal_uInt16   getObjId() const { return mnObjId; }
    /** Returns the object type from the OBJ record. */
    inline sal_uInt16   getObjType() const { return mnObjType; }

    /** Returns true, if the object is hidden. */
    inline bool         isHidden() const { return mbHidden; }
    /** Returns true, if the object is visible. */
    inline bool         isVisible() const { return mbVisible; }
    /** Returns true, if the object is printable. */
    inline bool         isPrintable() const { return mbPrintable; }

    /** Creates the UNO shape and inserts it into the passed shape container. */
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                        convertAndInsert( BiffDrawingBase& rDrawing,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ::com::sun::star::awt::Rectangle* pParentRect = 0 ) const;

protected:
    /** Converts the passed line formatting to the passed property map. */
    void                convertLineProperties( ::oox::drawingml::ShapePropertyMap& rPropMap, const BiffObjLineModel& rLineModel, sal_uInt16 nArrows = 0 ) const;
    /** Converts the passed fill formatting to the passed property map. */
    void                convertFillProperties( ::oox::drawingml::ShapePropertyMap& rPropMap, const BiffObjFillModel& rFillModel ) const;

    /** Derived classes create the corresponding XShape and insert it into the passed container. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                        implConvertAndInsert( BiffDrawingBase& rDrawing,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ::com::sun::star::awt::Rectangle& rShapeRect ) const = 0;

private:
    ShapeAnchor         maAnchor;       /// Position of the drawing object.
    ::rtl::OUString     maObjName;      /// Name of the object.
    ::rtl::OUString     maMacroName;    /// Name of an attached macro.
    ::rtl::OUString     maHyperlink;    /// On-click hyperlink URL.
    sal_uInt16          mnObjId;        /// The object identifier (unique per drawing).
    sal_uInt16          mnObjType;      /// The object type from OBJ record.
    bool                mbHidden;       /// True = object is hidden.
    bool                mbVisible;      /// True = object is visible (form controls).
    bool                mbPrintable;    /// True = object is printable.
    bool                mbAreaObj;      /// True = width and height must be greater than 0.
    bool                mbSimpleMacro;  /// True = create simple macro link and hyperlink.
    bool                mbProcessShape; /// True = object is valid, do processing and insertion.
    bool                mbInsertShape;  /// True = insert the UNO shape into the draw page.
    bool                mbCustomDff;    /// True = recreate UNO shape in DFF import (BIFF8 only).
};

// ============================================================================
// BIFF drawing page
// ============================================================================

/** Base class for a container for all objects on a drawing page (in a
    spreadsheet or in an embedded chart object).

    For BIFF import, it is needed to load all drawing objects before converting
    them to UNO shapes. There might be some dummy drawing objects (e.g. the
    dropdown buttons of autofilters) which have to be skipped. The information,
    that a drawing object is a dummy object, may be located after the drawing
    objects themselves.

    The BIFF8 format stores drawing objects in the DFF stream (stored
    fragmented in MSODRAWING records), and in the OBJ records. The DFF stream
    fragments are collected in a single stream, and the complete stream will be
    processed afterwards.
 */
class BiffDrawingBase : public WorksheetHelper
{
public:
    explicit            BiffDrawingBase( const WorksheetHelper& rHelper,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& rxDrawPage );

    /** Sets the object with the passed identifier to be skipped on import. */
    void                setSkipObj( sal_uInt16 nObjId );

    /** Final processing after import of the all drawing objects. */
    void                finalizeImport();

    /** Derived classes may want to know that a shape has been inserted. Will
        be called from the convertAndInsert() implementation. */
    virtual void        notifyShapeInserted(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rxShape,
                            const ::com::sun::star::awt::Rectangle& rShapeRect ) = 0;

private:
    typedef RefMap< sal_uInt16, BiffDrawingObjectBase > BiffDrawingObjectMapById;
    typedef ::std::vector< sal_uInt16 >                 BiffObjIdVector;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >
                        mxDrawPage;         /// UNO draw page used to insert the shapes.
    BiffDrawingObjectContainer maRawObjs;   /// Drawing objects without DFF data.
    BiffDrawingObjectMapById maObjMapId;    /// Maps drawing objects by their object identifiers.
    BiffObjIdVector     maSkipObjs;         /// Identifiers of all objects to be skipped.
};

// ----------------------------------------------------------------------------

/** Drawing manager of a single sheet. */
class BiffSheetDrawing : public BiffDrawingBase
{
public:
    explicit            BiffSheetDrawing( const WorksheetHelper& rHelper );

    /** Called when a new UNO shape has been inserted into the draw page. */
    virtual void        notifyShapeInserted(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rxShape,
                            const ::com::sun::star::awt::Rectangle& rShapeRect );
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
