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

#include "oox/xls/drawingbase.hxx"

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

    /** Returns true, if the line formatting is visible (automatic or explicit). */
    bool                isVisible() const;
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

    /** Returns true, if the fill formatting is visible (automatic or explicit). */
    bool                isFilled() const;
};

// ============================================================================

/** This structure contains text formatting attributes from an OBJ record. */
struct BiffObjTextModel
{
    sal_uInt16          mnTextLen;          /// Length of the text (characters).
    sal_uInt16          mnFormatSize;       /// Size of the formatting array (bytes).
    sal_uInt16          mnLinkSize;         /// Size of the linked text formula (bytes).
    sal_uInt16          mnDefFontId;        /// Font index for default font formatting.
    sal_uInt16          mnFlags;            /// Additional flags.
    sal_uInt16          mnOrientation;      /// Text orientation.
    sal_uInt16          mnButtonFlags;      /// Additional flags for push buttons.
    sal_uInt16          mnShortcut;         /// Shortcut character.
    sal_uInt16          mnShortcutEA;       /// East-asian shortcut character.

    explicit            BiffObjTextModel();

    /** Reads text data from a BIFF3/BIFF4 OBJ record. */
    void                readObj3( BiffInputStream& rStrm );
    /** Reads text data from a BIFF5 OBJ record. */
    void                readObj5( BiffInputStream& rStrm );
    /** Reads text data from a BIFF8 TXO record. */
    void                readTxo8( BiffInputStream& rStrm );

    /** Returns the horizontal alignment of the text. */
    sal_uInt8           getHorAlign() const;
    /** Returns the vertical alignment of the text. */
    sal_uInt8           getVerAlign() const;
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

    /** Appends the passed object to the list of objects. */
    void                append( const BiffDrawingObjectRef& rxDrawingObj );
    /** Tries to insert the passed object into the last group or appends it. */
    void                insertGrouped( const BiffDrawingObjectRef& rxDrawingObj );

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

    /** Reads the BIFF3 OBJ record, returns a new drawing object. */
    static BiffDrawingObjectRef importObjBiff3( const WorksheetHelper& rHelper, BiffInputStream& rStrm );
    /** Reads the BIFF4 OBJ record, returns a new drawing object. */
    static BiffDrawingObjectRef importObjBiff4( const WorksheetHelper& rHelper, BiffInputStream& rStrm );
    /** Reads the BIFF5 OBJ record, returns a new drawing object. */
    static BiffDrawingObjectRef importObjBiff5( const WorksheetHelper& rHelper, BiffInputStream& rStrm );
    /** Reads the BIFF8 OBJ record, returns a new drawing object. */
    static BiffDrawingObjectRef importObjBiff8( const WorksheetHelper& rHelper, BiffInputStream& rStrm );

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
    /** Reads the object name in a BIFF5 OBJ record. */
    void                readNameBiff5( BiffInputStream& rStrm, sal_uInt16 nNameLen );
    /** Reads the macro link in a BIFF3 OBJ record. */
    void                readMacroBiff3( BiffInputStream& rStrm, sal_uInt16 nMacroSize );
    /** Reads the macro link in a BIFF4 OBJ record. */
    void                readMacroBiff4( BiffInputStream& rStrm, sal_uInt16 nMacroSize );
    /** Reads the macro link in a BIFF5 OBJ record. */
    void                readMacroBiff5( BiffInputStream& rStrm, sal_uInt16 nMacroSize );
    /** Reads the contents of the ftMacro sub structure in an OBJ record. */
    void                readMacroBiff8( BiffInputStream& rStrm );

    /** Converts the passed line formatting to the passed property map. */
    void                convertLineProperties( ::oox::drawingml::ShapePropertyMap& rPropMap, const BiffObjLineModel& rLineModel, sal_uInt16 nArrows = 0 ) const;
    /** Converts the passed fill formatting to the passed property map. */
    void                convertFillProperties( ::oox::drawingml::ShapePropertyMap& rPropMap, const BiffObjFillModel& rFillModel ) const;
    /** Converts the passed frame flags to the passed property map. */
    void                convertFrameProperties( ::oox::drawingml::ShapePropertyMap& rPropMap, sal_uInt16 nFrameFlags ) const;

    /** Derived classes read the contents of the a BIFF3 OBJ record from the passed stream. */
    virtual void        implReadObjBiff3( BiffInputStream& rStrm, sal_uInt16 nMacroSize );
    /** Derived classes read the contents of the a BIFF4 OBJ record from the passed stream. */
    virtual void        implReadObjBiff4( BiffInputStream& rStrm, sal_uInt16 nMacroSize );
    /** Derived classes read the contents of the a BIFF5 OBJ record from the passed stream. */
    virtual void        implReadObjBiff5( BiffInputStream& rStrm, sal_uInt16 nNameLen, sal_uInt16 nMacroSize );
    /** Derived classes read the contents of the specified subrecord of a BIFF8 OBJ record from stream. */
    virtual void        implReadObjBiff8SubRec( BiffInputStream& rStrm, sal_uInt16 nSubRecId, sal_uInt16 nSubRecSize );

    /** Derived classes create the corresponding XShape and insert it into the passed container. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                        implConvertAndInsert( BiffDrawingBase& rDrawing,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ::com::sun::star::awt::Rectangle& rShapeRect ) const = 0;

private:
    /** Reads the contents of a BIFF3 OBJ record. */
    void                importObjBiff3( BiffInputStream& rStrm );
    /** Reads the contents of a BIFF4 OBJ record. */
    void                importObjBiff4( BiffInputStream& rStrm );
    /** Reads the contents of a BIFF5 OBJ record. */
    void                importObjBiff5( BiffInputStream& rStrm );
    /** Reads the contents of a BIFF8 OBJ record. */
    void                importObjBiff8( BiffInputStream& rStrm );

private:
    ShapeAnchor         maAnchor;       /// Position of the drawing object.
    ::rtl::OUString     maObjName;      /// Name of the object.
    ::rtl::OUString     maMacroName;    /// Name of an attached macro.
    ::rtl::OUString     maHyperlink;    /// On-click hyperlink URL.
    sal_uInt32          mnDffShapeId;   /// Shape identifier from DFF stream (BIFF8 only).
    sal_uInt32          mnDffFlags;     /// Shape flags from DFF stream.
    sal_uInt16          mnObjId;        /// The object identifier (unique per drawing).
    sal_uInt16          mnObjType;      /// The object type from OBJ record.
    bool                mbHasAnchor;    /// True = anchor has been initialized.
    bool                mbHidden;       /// True = object is hidden.
    bool                mbVisible;      /// True = object is visible (form controls).
    bool                mbPrintable;    /// True = object is printable.
    bool                mbAreaObj;      /// True = width and height must be greater than 0.
    bool                mbAutoMargin;   /// True = set automatic text margin.
    bool                mbSimpleMacro;  /// True = create simple macro link and hyperlink.
    bool                mbProcessShape; /// True = object is valid, do processing and insertion.
    bool                mbInsertShape;  /// True = insert the UNO shape into the draw page.
    bool                mbCustomDff;    /// True = recreate UNO shape in DFF import (BIFF8 only).
};

// ============================================================================

/** A placeholder object for unknown/unsupported object types. */
class BiffPlaceholderObject : public BiffDrawingObjectBase
{
public:
    explicit            BiffPlaceholderObject( const WorksheetHelper& rHelper );

protected:
    /** Creates the corresponding XShape and insert it into the passed container. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                        implConvertAndInsert( BiffDrawingBase& rDrawing,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ::com::sun::star::awt::Rectangle& rShapeRect ) const;
};

// ============================================================================

/** A group object that is able to contain other child objects. */
class BiffGroupObject : public BiffDrawingObjectBase
{
public:
    explicit            BiffGroupObject( const WorksheetHelper& rHelper );

    /** Tries to insert the passed drawing object into this or a nested group. */
    bool                tryInsert( const BiffDrawingObjectRef& rxDrawingObj );

protected:
    /** Reads the contents of the a BIFF3 OBJ record from the passed stream. */
    virtual void        implReadObjBiff3( BiffInputStream& rStrm, sal_uInt16 nMacroSize );
    /** Reads the contents of the a BIFF4 OBJ record from the passed stream. */
    virtual void        implReadObjBiff4( BiffInputStream& rStrm, sal_uInt16 nMacroSize );
    /** Reads the contents of the a BIFF5 OBJ record from the passed stream. */
    virtual void        implReadObjBiff5( BiffInputStream& rStrm, sal_uInt16 nNameLen, sal_uInt16 nMacroSize );

    /** Creates the corresponding XShape and insert it into the passed container. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                        implConvertAndInsert( BiffDrawingBase& rDrawing,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ::com::sun::star::awt::Rectangle& rShapeRect ) const;

protected:
    BiffDrawingObjectContainer maChildren;     /// All child objects contained in this group object.
    sal_uInt16          mnFirstUngrouped;   /// Object identfier of first object not grouped into this group.
};

// ============================================================================

/** A simple line object. */
class BiffLineObject : public BiffDrawingObjectBase
{
public:
    explicit            BiffLineObject( const WorksheetHelper& rHelper );

protected:
    /** Reads the contents of the a BIFF3 OBJ record from the passed stream. */
    virtual void        implReadObjBiff3( BiffInputStream& rStrm, sal_uInt16 nMacroSize );
    /** Reads the contents of the a BIFF4 OBJ record from the passed stream. */
    virtual void        implReadObjBiff4( BiffInputStream& rStrm, sal_uInt16 nMacroSize );
    /** Reads the contents of the a BIFF5 OBJ record from the passed stream. */
    virtual void        implReadObjBiff5( BiffInputStream& rStrm, sal_uInt16 nNameLen, sal_uInt16 nMacroSize );

    /** Creates the corresponding XShape and insert it into the passed container. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                        implConvertAndInsert( BiffDrawingBase& rDrawing,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ::com::sun::star::awt::Rectangle& rShapeRect ) const;

protected:
    BiffObjLineModel    maLineModel;    /// Line formatting.
    sal_uInt16          mnArrows;       /// Line arrows.
    sal_uInt8           mnStartPoint;   /// Starting point.
};

// ============================================================================

/** A simple rectangle object (used as base class for oval objects). */
class BiffRectObject : public BiffDrawingObjectBase
{
public:
    explicit            BiffRectObject( const WorksheetHelper& rHelper );

protected:
    /** Reads the fill model, the line model, and frame flags. */
    void                readFrameData( BiffInputStream& rStrm );

    /** Converts fill formatting, line formatting, and frame style. */
    void                convertRectProperties( ::oox::drawingml::ShapePropertyMap& rPropMap ) const;

    /** Reads the contents of the a BIFF3 OBJ record from the passed stream. */
    virtual void        implReadObjBiff3( BiffInputStream& rStrm, sal_uInt16 nMacroSize );
    /** Reads the contents of the a BIFF4 OBJ record from the passed stream. */
    virtual void        implReadObjBiff4( BiffInputStream& rStrm, sal_uInt16 nMacroSize );
    /** Reads the contents of the a BIFF5 OBJ record from the passed stream. */
    virtual void        implReadObjBiff5( BiffInputStream& rStrm, sal_uInt16 nNameLen, sal_uInt16 nMacroSize );

    /** Creates the corresponding XShape and insert it into the passed container. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                        implConvertAndInsert( BiffDrawingBase& rDrawing,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ::com::sun::star::awt::Rectangle& rShapeRect ) const;

protected:
    BiffObjFillModel    maFillModel;    /// Fill formatting.
    BiffObjLineModel    maLineModel;    /// Line formatting.
    sal_uInt16          mnFrameFlags;   /// Additional flags.
};

// ============================================================================

/** A simple oval object. */
class BiffOvalObject : public BiffRectObject
{
public:
    explicit            BiffOvalObject( const WorksheetHelper& rHelper );

protected:
    /** Creates the corresponding XShape and insert it into the passed container. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                        implConvertAndInsert( BiffDrawingBase& rDrawing,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ::com::sun::star::awt::Rectangle& rShapeRect ) const;
};

// ============================================================================

/** A simple arc object. */
class BiffArcObject : public BiffDrawingObjectBase
{
public:
    explicit            BiffArcObject( const WorksheetHelper& rHelper );

protected:
    /** Reads the contents of the a BIFF3 OBJ record from the passed stream. */
    virtual void        implReadObjBiff3( BiffInputStream& rStrm, sal_uInt16 nMacroSize );
    /** Reads the contents of the a BIFF4 OBJ record from the passed stream. */
    virtual void        implReadObjBiff4( BiffInputStream& rStrm, sal_uInt16 nMacroSize );
    /** Reads the contents of the a BIFF5 OBJ record from the passed stream. */
    virtual void        implReadObjBiff5( BiffInputStream& rStrm, sal_uInt16 nNameLen, sal_uInt16 nMacroSize );

    /** Creates the corresponding XShape and insert it into the passed container. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                        implConvertAndInsert( BiffDrawingBase& rDrawing,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ::com::sun::star::awt::Rectangle& rShapeRect ) const;

protected:
    BiffObjFillModel    maFillModel;    /// Fill formatting.
    BiffObjLineModel    maLineModel;    /// Line formatting.
    sal_uInt8           mnQuadrant;     /// Visible quadrant of the circle.
};

// ============================================================================

/** A simple polygon object. */
class BiffPolygonObject : public BiffRectObject
{
public:
    explicit            BiffPolygonObject( const WorksheetHelper& rHelper );

protected:
    /** Reads the contents of the a BIFF4 OBJ record from the passed stream. */
    virtual void        implReadObjBiff4( BiffInputStream& rStrm, sal_uInt16 nMacroSize );
    /** Reads the contents of the a BIFF5 OBJ record from the passed stream. */
    virtual void        implReadObjBiff5( BiffInputStream& rStrm, sal_uInt16 nNameLen, sal_uInt16 nMacroSize );

    /** Creates the corresponding XShape and insert it into the passed container. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                        implConvertAndInsert( BiffDrawingBase& rDrawing,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ::com::sun::star::awt::Rectangle& rShapeRect ) const;

private:
    /** Reads the COORDLIST record following the OBJ record. */
    void                importCoordList( BiffInputStream& rStrm );

protected:
    typedef ::std::vector< ::com::sun::star::awt::Point > PointVector;
    PointVector         maCoords;       /// Coordinates relative to bounding rectangle.
    sal_uInt16          mnPolyFlags;    /// Additional flags.
    sal_uInt16          mnPointCount;   /// Polygon point count.
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

    /** Imports a plain OBJ record (without leading DFF data). */
    void                importObj( BiffInputStream& rStrm );
    /** Sets the object with the passed identifier to be skipped on import. */
    void                setSkipObj( sal_uInt16 nObjId );

    /** Final processing after import of the all drawing objects. */
    void                finalizeImport();

    /** Creates a new UNO shape object, inserts it into the passed UNO shape
        container, and sets the shape position and size. */
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                        createAndInsertXShape(
                            const ::rtl::OUString& rService,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ::com::sun::star::awt::Rectangle& rShapeRect ) const;

    /** Derived classes may want to know that a shape has been inserted. Will
        be called from the convertAndInsert() implementation. */
    virtual void        notifyShapeInserted(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rxShape,
                            const ::com::sun::star::awt::Rectangle& rShapeRect ) = 0;

protected:
    /** Appends a new drawing object to the list of raw objects (without DFF data). */
    void                appendRawObject( const BiffDrawingObjectRef& rxDrawingObj );

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
