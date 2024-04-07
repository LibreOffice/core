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

#include <filter/msfilter/msdffimp.hxx>
#include <vcl/graph.hxx>
#include "xlescher.hxx"
#include "xiroot.hxx"
#include <oox/ole/olehelper.hxx>
#include <rtl/ustring.hxx>
#include <svx/svdobj.hxx>
#include <map>
#include <memory>
#include <vector>

namespace com::sun::star {
    namespace drawing { class XShape; }
    namespace form { class XForm; }
}

namespace com::sun::star::container { class XNameContainer; }

class SdrObjList;
class ScfProgressBar;
class ScfPropertySet;
class ScRangeList;
class XclImpChart;
class XclImpDffConverter;
class XclImpDrawing;

// Drawing objects ============================================================

class XclImpDrawObjBase;
typedef std::shared_ptr< XclImpDrawObjBase > XclImpDrawObjRef;

/** Base class for drawing objects (OBJ records). */
class XclImpDrawObjBase : protected XclImpRoot
{
public:
    explicit            XclImpDrawObjBase( const XclImpRoot& rRoot );
    virtual             ~XclImpDrawObjBase() override;

    /** Reads the BIFF3 OBJ record, returns a new drawing object. */
    static XclImpDrawObjRef ReadObj3( const XclImpRoot& rRoot, XclImpStream& rStrm );
    /** Reads the BIFF4 OBJ record, returns a new drawing object. */
    static XclImpDrawObjRef ReadObj4( const XclImpRoot& rRoot, XclImpStream& rStrm );
    /** Reads the BIFF5 OBJ record, returns a new drawing object. */
    static XclImpDrawObjRef ReadObj5( const XclImpRoot& rRoot, XclImpStream& rStrm );
    /** Reads the BIFF8 OBJ record, returns a new drawing object. */
    static XclImpDrawObjRef ReadObj8( const XclImpRoot& rRoot, XclImpStream& rStrm );

    /** Sets whether this is an area object (then its width and height must be greater than 0). */
    void         SetAreaObj( bool bAreaObj ) { mbAreaObj = bAreaObj; }
    /** If set to true, a new SdrObject will be created while in DFF import. */
    void         SetSimpleMacro( bool bMacro ) { mbSimpleMacro = bMacro; }

    /** Sets the object anchor explicitly. */
    void                SetAnchor( const XclObjAnchor& rAnchor );
    /** Sets shape data from DFF stream. */
    void                SetDffData(
        const DffObjData& rDffObjData, const OUString& rObjName, const OUString& rHyperlink,
        bool bVisible, bool bAutoMargin );

    /** If set to false, the SdrObject will not be created, processed, or inserted into the draw page. */
    void         SetProcessSdrObj( bool bProcess ) { mbProcessSdr = bProcess; }
    /** If set to false, the SdrObject will be created or processed, but not be inserted into the draw page. */
    void         SetInsertSdrObj( bool bInsert ) { mbInsertSdr = bInsert; }
    /** If set to true, a new SdrObject will be created while in DFF import. */
    void         SetCustomDffObj( bool bCustom ) { mbCustomDff = bCustom; }

    /** Returns the sheet index and Excel object identifier from OBJ record. */
    sal_uInt16   GetObjId() const { return mnObjId; }
    /** Returns the Excel object type from OBJ record. */
    sal_uInt16   GetObjType() const { return mnObjType; }
    /** Returns the name of this object, may generate a default name. */
    virtual OUString GetObjName() const;
    /** Returns associated macro name, if set, otherwise zero length string. */
    const OUString& GetMacroName() const { return maMacroName; }

    /** Returns the shape identifier used in the DFF stream. */
    sal_uInt32   GetDffShapeId() const { return mnDffShapeId; }
    /** Returns the shape flags from the DFF stream. */
    ShapeFlag    GetDffFlags() const { return mnDffFlags; }

    /** Returns true, if the object is hidden. */
    bool         IsHidden() const { return mbHidden; }
    /** Returns true, if the object is visible. */
    bool         IsVisible() const { return mbVisible; }
    /** Returns true, if the object is printable. */
    bool         IsPrintable() const { return mbPrintable; }

    /** Returns the object anchor if existing, null otherwise. */
    const XclObjAnchor* GetAnchor() const;
    /** Returns true, if the passed size is valid for this object. */
    bool                IsValidSize( const tools::Rectangle& rAnchorRect ) const;
    /** Returns the range in the sheet covered by this object. */
    ScRange             GetUsedArea( SCTAB nScTab ) const;

    /** Returns true, if the object is valid and will be processed. */
    bool         IsProcessSdrObj() const { return mbProcessSdr && !mbHidden; }
    /** Returns true, if the SdrObject will be created or processed, but not be inserted into the draw page. */
    bool         IsInsertSdrObj() const { return mbInsertSdr; }

    /** Returns the needed size on the progress bar (calls virtual DoGetProgressSize() function). */
    std::size_t         GetProgressSize() const;
    /** Creates and returns an SdrObject from the contained data. Caller takes ownership! */
    rtl::Reference<SdrObject>        CreateSdrObject( XclImpDffConverter& rDffConv, const tools::Rectangle& rAnchorRect, bool bIsDff ) const;
    /** Additional processing for the passed SdrObject before insertion into
        the drawing page (calls virtual DoPreProcessSdrObj() function). */
    void                PreProcessSdrObject( XclImpDffConverter& rDffConv, SdrObject& rSdrObj );
    /** Additional processing for the passed SdrObject after insertion into the
        drawing page (calls virtual DoPostProcessSdrObj() function). */
    void                PostProcessSdrObject( XclImpDffConverter& rDffConv, SdrObject& rSdrObj ) const;
    SCTAB               GetTab() const { return mnTab; }

protected:
    /** Reads the object name in a BIFF5 OBJ record. */
    void                ReadName5( XclImpStream& rStrm, sal_uInt16 nNameLen );
    /** Reads the macro link in a BIFF3 OBJ record. */
    void                ReadMacro3( XclImpStream& rStrm, sal_uInt16 nMacroSize );
    /** Reads the macro link in a BIFF4 OBJ record. */
    void                ReadMacro4( XclImpStream& rStrm, sal_uInt16 nMacroSize );
    /** Reads the macro link in a BIFF5 OBJ record. */
    void                ReadMacro5( XclImpStream& rStrm, sal_uInt16 nMacroSize );
    /** Reads the contents of the ftMacro sub structure in an OBJ record. */
    void                ReadMacro8( XclImpStream& rStrm );

    /** Converts the passed line formatting to the passed SdrObject. */
    void                ConvertLineStyle( SdrObject& rSdrObj, const XclObjLineData& rLineData ) const;
    /** Converts the passed fill formatting to the passed SdrObject. */
    void                ConvertFillStyle( SdrObject& rSdrObj, const XclObjFillData& rFillData ) const;
    /** Converts the passed frame flags to the passed SdrObject. */
    void                ConvertFrameStyle( SdrObject& rSdrObj, sal_uInt16 nFrameFlags ) const;

    /** Returns a solid line color from the passed line data struct. */
    Color               GetSolidLineColor( const XclObjLineData& rLineData ) const;
    /** Returns a solid fill color from the passed fill data struct. */
    Color               GetSolidFillColor( const XclObjFillData& rFillData ) const;

    /** Derived classes read the contents of the a BIFF3 OBJ record from the passed stream. */
    virtual void        DoReadObj3( XclImpStream& rStrm, sal_uInt16 nMacroSize );
    /** Derived classes read the contents of the a BIFF4 OBJ record from the passed stream. */
    virtual void        DoReadObj4( XclImpStream& rStrm, sal_uInt16 nMacroSize );
    /** Derived classes read the contents of the a BIFF5 OBJ record from the passed stream. */
    virtual void        DoReadObj5( XclImpStream& rStrm, sal_uInt16 nNameLen, sal_uInt16 nMacroSize );
    /** Derived classes read the contents of the specified subrecord of a BIFF8 OBJ record from stream. */
    virtual void        DoReadObj8SubRec( XclImpStream& rStrm, sal_uInt16 nSubRecId, sal_uInt16 nSubRecSize );

    /** Derived classes may return a progress bar size different from 1. */
    virtual std::size_t DoGetProgressSize() const;
    /** Derived classes create and return a new SdrObject from the contained data. Caller takes ownership! */
    virtual rtl::Reference<SdrObject> DoCreateSdrObj( XclImpDffConverter& rDffConv, const tools::Rectangle& rAnchorRect ) const;
    /** Derived classes may perform additional processing for the passed SdrObject before insertion. */
    virtual void        DoPreProcessSdrObj( XclImpDffConverter& rDffConv, SdrObject& rSdrObj ) const;
    /** Derived classes may perform additional processing for the passed SdrObject after insertion. */
    virtual void        DoPostProcessSdrObj( XclImpDffConverter& rDffConv, SdrObject& rSdrObj ) const;

    /** Notify that the document contains a macro event handler */
    void NotifyMacroEventRead();
private:
    /** Reads the contents of a BIFF3 OBJ record. */
    void                ImplReadObj3( XclImpStream& rStrm );
    /** Reads the contents of a BIFF4 OBJ record. */
    void                ImplReadObj4( XclImpStream& rStrm );
    /** Reads the contents of a BIFF5 OBJ record. */
    void                ImplReadObj5( XclImpStream& rStrm );
    /** Reads the contents of a BIFF8 OBJ record. */
    void                ImplReadObj8( XclImpStream& rStrm );

private:
    XclObjAnchor        maAnchor;       /// The position of the object in its parent.
    sal_uInt16          mnObjId;        /// The object identifier (unique per drawing).
    SCTAB               mnTab;          /// Location of object
    sal_uInt16          mnObjType;      /// The Excel object type from OBJ record.
    sal_uInt32          mnDffShapeId;   /// Shape ID from DFF stream.
    ShapeFlag           mnDffFlags;     /// Shape flags from DFF stream.
    OUString       maObjName;      /// Name of the object.
    OUString       maMacroName;    /// Name of an attached macro.
    OUString       maHyperlink;    /// On-click hyperlink URL.
    bool                mbHasAnchor;    /// true = maAnchor is initialized.
    bool                mbHidden;       /// true = Object is hidden.
    bool                mbVisible;      /// true = Object is visible.
    bool                mbPrintable;    /// true = Object is printable.
    bool                mbAreaObj;      /// true = Width and height must be greater than 0.
    bool                mbAutoMargin;   /// true = Set automatic text margin.
    bool                mbSimpleMacro;  /// true = Create simple macro link and hyperlink.
    bool                mbProcessSdr;   /// true = Object is valid, do processing and insertion.
    bool                mbInsertSdr;    /// true = Insert the SdrObject into draw page.
    bool                mbCustomDff;    /// true = Recreate SdrObject in DFF import.
    bool                mbNotifyMacroEventRead; /// true == If we have already seen a macro event
};

class XclImpDrawObjVector
{
private:
    std::vector< XclImpDrawObjRef > mObjs;

public:
    explicit     XclImpDrawObjVector() : mObjs() {}

    std::vector< XclImpDrawObjRef >::const_iterator begin() const { return mObjs.begin(); }
    std::vector< XclImpDrawObjRef >::const_iterator end() const { return mObjs.end(); }
    void push_back(const XclImpDrawObjRef& rObj) { mObjs.push_back(rObj); }

    /** Tries to insert the passed object into the last group or appends it. */
    void                InsertGrouped( XclImpDrawObjRef const & xDrawObj );

    /** Returns the needed size on the progress bar for all contained objects. */
    std::size_t         GetProgressSize() const;
};

/** A placeholder object for unknown object types. */
class XclImpPhObj : public XclImpDrawObjBase
{
public:
    explicit            XclImpPhObj( const XclImpRoot& rRoot );
};

/** A group object. */
class XclImpGroupObj final : public XclImpDrawObjBase
{
public:
    explicit            XclImpGroupObj( const XclImpRoot& rRoot );

    /** Tries to insert the drawing object into this or a nested group. */
    bool                TryInsert( XclImpDrawObjRef const & xDrawObj );

private:
    /** Reads the contents of the a BIFF3 OBJ record from the passed stream. */
    virtual void        DoReadObj3( XclImpStream& rStrm, sal_uInt16 nMacroSize ) override;
    /** Reads the contents of the a BIFF4 OBJ record from the passed stream. */
    virtual void        DoReadObj4( XclImpStream& rStrm, sal_uInt16 nMacroSize ) override;
    /** Reads the contents of the a BIFF5 OBJ record from the passed stream. */
    virtual void        DoReadObj5( XclImpStream& rStrm, sal_uInt16 nNameLen, sal_uInt16 nMacroSize ) override;
    /** Returns a progress bar size that takes all group children into account. */
    virtual std::size_t DoGetProgressSize() const override;
    /** Creates and returns a new SdrObject from the contained data. Caller takes ownership! */
    virtual rtl::Reference<SdrObject> DoCreateSdrObj( XclImpDffConverter& rDffConv, const tools::Rectangle& rAnchorRect ) const override;

    XclImpDrawObjVector maChildren;         /// Grouped objects.
    sal_uInt16          mnFirstUngrouped;   /// Object identifier of first object not grouped into this group.
};

/** A line object. */
class XclImpLineObj final : public XclImpDrawObjBase
{
public:
    explicit            XclImpLineObj( const XclImpRoot& rRoot );

private:
    /** Reads the contents of the a BIFF3 OBJ record from the passed stream. */
    virtual void        DoReadObj3( XclImpStream& rStrm, sal_uInt16 nMacroSize ) override;
    /** Reads the contents of the a BIFF4 OBJ record from the passed stream. */
    virtual void        DoReadObj4( XclImpStream& rStrm, sal_uInt16 nMacroSize ) override;
    /** Reads the contents of the a BIFF5 OBJ record from the passed stream. */
    virtual void        DoReadObj5( XclImpStream& rStrm, sal_uInt16 nNameLen, sal_uInt16 nMacroSize ) override;
    /** Creates and returns a new SdrObject from the contained data. Caller takes ownership! */
    virtual rtl::Reference<SdrObject> DoCreateSdrObj( XclImpDffConverter& rDffConv, const tools::Rectangle& rAnchorRect ) const override;

    XclObjLineData      maLineData;     /// BIFF5 line formatting.
    sal_uInt16          mnArrows;       /// Line arrows.
    sal_uInt8           mnStartPoint;   /// Starting point.
};

/** A rectangle or oval object. */
class XclImpRectObj : public XclImpDrawObjBase
{
public:
    explicit            XclImpRectObj( const XclImpRoot& rRoot );

protected:
    /** Reads fil data, line data, and frame flags. */
    void                ReadFrameData( XclImpStream& rStrm );

    /** Converts fill formatting, line formatting, and frame style. */
    void                ConvertRectStyle( SdrObject& rSdrObj ) const;

    /** Reads the contents of the a BIFF3 OBJ record from the passed stream. */
    virtual void        DoReadObj3( XclImpStream& rStrm, sal_uInt16 nMacroSize ) override;
    /** Reads the contents of the a BIFF4 OBJ record from the passed stream. */
    virtual void        DoReadObj4( XclImpStream& rStrm, sal_uInt16 nMacroSize ) override;
    /** Reads the contents of the a BIFF5 OBJ record from the passed stream. */
    virtual void        DoReadObj5( XclImpStream& rStrm, sal_uInt16 nNameLen, sal_uInt16 nMacroSize ) override;
    /** Creates and returns a new SdrObject from the contained data. Caller takes ownership! */
    virtual rtl::Reference<SdrObject> DoCreateSdrObj( XclImpDffConverter& rDffConv, const tools::Rectangle& rAnchorRect ) const override;

protected:
    XclObjFillData      maFillData;     /// BIFF5 fill formatting.
    XclObjLineData      maLineData;     /// BIFF5 line formatting.
    sal_uInt16          mnFrameFlags;   /// Additional flags.
};

/** An oval object. */
class XclImpOvalObj : public XclImpRectObj
{
public:
    explicit            XclImpOvalObj( const XclImpRoot& rRoot );

protected:
    /** Creates and returns a new SdrObject from the contained data. Caller takes ownership! */
    virtual rtl::Reference<SdrObject> DoCreateSdrObj( XclImpDffConverter& rDffConv, const tools::Rectangle& rAnchorRect ) const override;
};

/** An arc object. */
class XclImpArcObj final : public XclImpDrawObjBase
{
public:
    explicit            XclImpArcObj( const XclImpRoot& rRoot );

private:
    /** Reads the contents of the a BIFF3 OBJ record from the passed stream. */
    virtual void        DoReadObj3( XclImpStream& rStrm, sal_uInt16 nMacroSize ) override;
    /** Reads the contents of the a BIFF4 OBJ record from the passed stream. */
    virtual void        DoReadObj4( XclImpStream& rStrm, sal_uInt16 nMacroSize ) override;
    /** Reads the contents of the a BIFF5 OBJ record from the passed stream. */
    virtual void        DoReadObj5( XclImpStream& rStrm, sal_uInt16 nNameLen, sal_uInt16 nMacroSize ) override;
    /** Creates and returns a new SdrObject from the contained data. Caller takes ownership! */
    virtual rtl::Reference<SdrObject> DoCreateSdrObj( XclImpDffConverter& rDffConv, const tools::Rectangle& rAnchorRect ) const override;

    XclObjFillData      maFillData;     /// BIFF5 fill formatting.
    XclObjLineData      maLineData;     /// BIFF5 line formatting.
    sal_uInt8           mnQuadrant;     /// Visible quadrant of the circle.
};

/** A polygon object. */
class XclImpPolygonObj final : public XclImpRectObj
{
public:
    explicit            XclImpPolygonObj( const XclImpRoot& rRoot );

private:
    /** Reads the COORDLIST record following the OBJ record. */
    void                ReadCoordList( XclImpStream& rStrm );

    /** Reads the contents of the a BIFF4 OBJ record from the passed stream. */
    virtual void        DoReadObj4( XclImpStream& rStrm, sal_uInt16 nMacroSize ) override;
    /** Reads the contents of the a BIFF5 OBJ record from the passed stream. */
    virtual void        DoReadObj5( XclImpStream& rStrm, sal_uInt16 nNameLen, sal_uInt16 nMacroSize ) override;
    /** Creates and returns a new SdrObject from the contained data. Caller takes ownership! */
    virtual rtl::Reference<SdrObject> DoCreateSdrObj( XclImpDffConverter& rDffConv, const tools::Rectangle& rAnchorRect ) const override;

    typedef std::vector< Point > PointVector;
    PointVector         maCoords;       /// Coordinates relative to bounding rectangle.
    sal_uInt16          mnPolyFlags;    /// Additional flags.
    sal_uInt16          mnPointCount;   /// Polygon point count.
};

struct XclImpObjTextData
{
    XclObjTextData      maData;         /// BIFF5 text data.
    XclImpStringRef     mxString;       /// Plain or rich string.

    /** Reads a byte string from the passed stream. */
    void                ReadByteString( XclImpStream& rStrm );
    /** Reads text formatting from the passed stream. */
    void                ReadFormats( XclImpStream& rStrm );
};

/** A drawing object supporting text contents. Used for all simple objects in BIFF8. */
class XclImpTextObj : public XclImpRectObj
{
public:
    explicit            XclImpTextObj( const XclImpRoot& rRoot );

    /** Stores the passed textbox data. */
    void         SetTextData( const XclImpObjTextData& rTextData ) { maTextData = rTextData; }

protected:
    /** Reads the contents of the a BIFF3 OBJ record from the passed stream. */
    virtual void        DoReadObj3( XclImpStream& rStrm, sal_uInt16 nMacroSize ) override;
    /** Reads the contents of the a BIFF4 OBJ record from the passed stream. */
    virtual void        DoReadObj4( XclImpStream& rStrm, sal_uInt16 nMacroSize ) override;
    /** Reads the contents of the a BIFF5 OBJ record from the passed stream. */
    virtual void        DoReadObj5( XclImpStream& rStrm, sal_uInt16 nNameLen, sal_uInt16 nMacroSize ) override;
    /** Creates and returns a new SdrObject from the contained data. Caller takes ownership! */
    virtual rtl::Reference<SdrObject> DoCreateSdrObj( XclImpDffConverter& rDffConv, const tools::Rectangle& rAnchorRect ) const override;
    /** Inserts the contained text data at the passed object. */
    virtual void        DoPreProcessSdrObj( XclImpDffConverter& rDffConv, SdrObject& rSdrObj ) const override;

protected:
    XclImpObjTextData   maTextData;     /// Textbox data from BIFF5 OBJ or BIFF8 TXO record.
};

/** A chart object. This is the drawing object wrapper for the chart data. */
class XclImpChartObj : public XclImpRectObj
{
public:
    /** @param bOwnTab  True = chart is on an own sheet; false = chart is an embedded object. */
    explicit            XclImpChartObj( const XclImpRoot& rRoot, bool bOwnTab = false );

    /** Reads the complete chart substream (BOF/EOF block). */
    void                ReadChartSubStream( XclImpStream& rStrm );

protected:
    /** Reads the contents of the a BIFF3 OBJ record from the passed stream. */
    virtual void        DoReadObj3( XclImpStream& rStrm, sal_uInt16 nMacroSize ) override;
    /** Reads the contents of the a BIFF4 OBJ record from the passed stream. */
    virtual void        DoReadObj4( XclImpStream& rStrm, sal_uInt16 nMacroSize ) override;
    /** Reads the contents of the a BIFF5 OBJ record from the passed stream. */
    virtual void        DoReadObj5( XclImpStream& rStrm, sal_uInt16 nNameLen, sal_uInt16 nMacroSize ) override;
    /** Reads the contents of the specified subrecord of a BIFF8 OBJ record from stream. */
    virtual void        DoReadObj8SubRec( XclImpStream& rStrm, sal_uInt16 nSubRecId, sal_uInt16 nSubRecSize ) override;
    /** Returns the needed size on the progress bar. */
    virtual std::size_t DoGetProgressSize() const override;
    /** Creates and returns a new SdrObject from the contained data. Caller takes ownership! */
    virtual rtl::Reference<SdrObject> DoCreateSdrObj( XclImpDffConverter& rDffConv, const tools::Rectangle& rAnchorRect ) const override;
    /** Converts the chart document. */
    virtual void        DoPostProcessSdrObj( XclImpDffConverter& rDffConv, SdrObject& rSdrObj ) const override;

private:
    /** Calculates the object anchor of a sheet chart (chart fills one page). */
    void                FinalizeTabChart();

private:
    typedef std::shared_ptr< XclImpChart > XclImpChartRef;

    XclImpChartRef      mxChart;        /// The chart itself (BOF/EOF substream data).
    bool                mbOwnTab;       /// true = own sheet; false = embedded object.
};

/** A note object, which is a specialized text box object. */
class XclImpNoteObj : public XclImpTextObj
{
public:
    explicit            XclImpNoteObj( const XclImpRoot& rRoot );

    /** Sets note flags and the note position in the Calc sheet. */
    void                SetNoteData( const ScAddress& rScPos, sal_uInt16 nNoteFlags );

protected:
    /** Inserts the note into the document, sets visibility. */
    virtual void        DoPreProcessSdrObj( XclImpDffConverter& rDffConv, SdrObject& rSdrObj ) const override;

private:
    ScAddress           maScPos;        /// Cell position of the note object.
    sal_uInt16          mnNoteFlags;    /// Flags from NOTE record.
};

/** Helper base class for TBX and OCX form controls to manage spreadsheet links. */
class XclImpControlHelper
{
public:
    explicit            XclImpControlHelper( const XclImpRoot& rRoot, XclCtrlBindMode eBindMode );
    virtual             ~XclImpControlHelper();

    /** Returns true, if a linked cell address is present. */
    bool         HasCellLink() const { return mxCellLink != nullptr; }

    /** Returns the SdrObject from the passed control shape and sets the bounding rectangle. */
    rtl::Reference<SdrObject>        CreateSdrObjectFromShape(
                            const css::uno::Reference< css::drawing::XShape >& rxShape,
                            const tools::Rectangle& rAnchorRect ) const;

    /** Sets additional properties to the form control model, calls virtual DoProcessControl(). */
    void                ProcessControl( const XclImpDrawObjBase& rDrawObj ) const;

protected:
    /** Reads the formula for the linked cell from the current position of the stream. */
    void                ReadCellLinkFormula( XclImpStream& rStrm, bool bWithBoundSize );
    /** Reads the formula for the source range from the current position of the stream. */
    void                ReadSourceRangeFormula( XclImpStream& rStrm, bool bWithBoundSize );

    /** Derived classes will set additional properties for the current form control. */
    virtual void        DoProcessControl( ScfPropertySet& rPropSet ) const;

    void ApplySheetLinkProps() const;
    mutable css::uno::Reference< css::drawing::XShape >
                                   mxShape;        /// The UNO wrapper of the control shape.
    std::shared_ptr< ScAddress > mxCellLink;     /// Linked cell in the Calc document.
private:
    /** Reads a list of cell ranges from a formula at the current stream position. */
    void                ReadRangeList( ScRangeList& rScRanges, XclImpStream& rStrm );
    /** Reads leading formula size and a list of cell ranges from a formula if the leading size is not zero. */
    void                ReadRangeList( ScRangeList& rScRanges, XclImpStream& rStrm, bool bWithBoundSize );

private:
    const XclImpRoot&            mrRoot;     /// Not derived from XclImpRoot to allow multiple inheritance.
    std::shared_ptr< ScRange >   mxSrcRange; /// Source data range in the Calc document.
    XclCtrlBindMode              meBindMode; /// Value binding mode.
};

/** Base class for textbox based form controls. */
class XclImpTbxObjBase : public XclImpTextObj, public XclImpControlHelper
{
public:
    explicit            XclImpTbxObjBase( const XclImpRoot& rRoot );

    /** Sets line and fill formatting from the passed DFF property set. */
    void                SetDffProperties( const DffPropSet& rDffPropSet );

    /** Returns the service name of the control component to be created. */
    OUString GetServiceName() const { return DoGetServiceName(); }
    /** Fills the passed macro event descriptor. */
    bool                FillMacroDescriptor(
                            css::script::ScriptEventDescriptor& rDescriptor ) const;

protected:
    /** Sets control text formatting. */
    void                ConvertFont( ScfPropertySet& rPropSet ) const;
    /** Sets control label and text formatting. */
    void                ConvertLabel( ScfPropertySet& rPropSet ) const;

    /** Creates and returns a new SdrObject from the contained data. Caller takes ownership! */
    virtual rtl::Reference<SdrObject> DoCreateSdrObj( XclImpDffConverter& rDffConv, const tools::Rectangle& rAnchorRect ) const override;
    /** Additional processing on the SdrObject, calls new virtual function DoProcessControl(). */
    virtual void        DoPreProcessSdrObj( XclImpDffConverter& rDffConv, SdrObject& rSdrObj ) const override;

    /** Derived classes return the service name of the control component to be created. */
    virtual OUString DoGetServiceName() const = 0;
    /** Derived classes return the type of the macro event to be created. */
    virtual XclTbxEventType DoGetEventType() const = 0;
};

/** A button control. */
class XclImpButtonObj : public XclImpTbxObjBase
{
public:
    explicit            XclImpButtonObj( const XclImpRoot& rRoot );

protected:
    /** Sets additional properties for the current form control. */
    virtual void        DoProcessControl( ScfPropertySet& rPropSet ) const override;
    /** Returns the service name of the control component to be created. */
    virtual OUString DoGetServiceName() const override;
    /** Returns the type of the macro event to be created. */
    virtual XclTbxEventType DoGetEventType() const override;
};

/** A checkbox control. */
class XclImpCheckBoxObj : public XclImpTbxObjBase
{
public:
    explicit            XclImpCheckBoxObj( const XclImpRoot& rRoot );

protected:
    /** Reads the contents of the a BIFF5 OBJ record from the passed stream. */
    virtual void        DoReadObj5( XclImpStream& rStrm, sal_uInt16 nNameLen, sal_uInt16 nMacroSize ) override;
    /** Reads the contents of the specified subrecord of a BIFF8 OBJ record from stream. */
    virtual void        DoReadObj8SubRec( XclImpStream& rStrm, sal_uInt16 nSubRecId, sal_uInt16 nSubRecSize ) override;
    /** Sets additional properties for the current form control. */
    virtual void        DoProcessControl( ScfPropertySet& rPropSet ) const override;
    /** Returns the service name of the control component to be created. */
    virtual OUString DoGetServiceName() const override;
    /** Returns the type of the macro event to be created. */
    virtual XclTbxEventType DoGetEventType() const override;

protected:
    sal_uInt16          mnState;
    sal_uInt16          mnCheckBoxFlags;
};

/** An option button control. */
class XclImpOptionButtonObj final : public XclImpCheckBoxObj
{
public:
    explicit            XclImpOptionButtonObj( const XclImpRoot& rRoot );

private:
    /** Reads the contents of the a BIFF5 OBJ record from the passed stream. */
    virtual void        DoReadObj5( XclImpStream& rStrm, sal_uInt16 nNameLen, sal_uInt16 nMacroSize ) override;
    /** Reads the contents of the specified subrecord of a BIFF8 OBJ record from stream. */
    virtual void        DoReadObj8SubRec( XclImpStream& rStrm, sal_uInt16 nSubRecId, sal_uInt16 nSubRecSize ) override;
    /** Sets additional properties for the current form control. */
    virtual void        DoProcessControl( ScfPropertySet& rPropSet ) const override;
    /** Returns the service name of the control component to be created. */
    virtual OUString DoGetServiceName() const override;
    /** Returns the type of the macro event to be created. */
    virtual XclTbxEventType DoGetEventType() const override;

    sal_uInt16          mnNextInGroup;      /// Next option button in a group.
    sal_uInt16          mnFirstInGroup;     /// 1 = Button is the first in a group.
};

/** A label control. */
class XclImpLabelObj : public XclImpTbxObjBase
{
public:
    explicit            XclImpLabelObj( const XclImpRoot& rRoot );

protected:
    /** Sets additional properties for the current form control. */
    virtual void        DoProcessControl( ScfPropertySet& rPropSet ) const override;
    /** Returns the service name of the control component to be created. */
    virtual OUString DoGetServiceName() const override;
    /** Returns the type of the macro event to be created. */
    virtual XclTbxEventType DoGetEventType() const override;
};

/** A groupbox control. */
class XclImpGroupBoxObj final : public XclImpTbxObjBase
{
public:
    explicit            XclImpGroupBoxObj( const XclImpRoot& rRoot );

private:
    /** Reads the contents of the a BIFF5 OBJ record from the passed stream. */
    virtual void        DoReadObj5( XclImpStream& rStrm, sal_uInt16 nNameLen, sal_uInt16 nMacroSize ) override;
    /** Reads the contents of the specified subrecord of a BIFF8 OBJ record from stream. */
    virtual void        DoReadObj8SubRec( XclImpStream& rStrm, sal_uInt16 nSubRecId, sal_uInt16 nSubRecSize ) override;
    /** Sets additional properties for the current form control. */
    virtual void        DoProcessControl( ScfPropertySet& rPropSet ) const override;
    /** Returns the service name of the control component to be created. */
    virtual OUString DoGetServiceName() const override;
    /** Returns the type of the macro event to be created. */
    virtual XclTbxEventType DoGetEventType() const override;

    sal_uInt16          mnGroupBoxFlags;
};

/** A dialog control. */
class XclImpDialogObj : public XclImpTbxObjBase
{
public:
    explicit            XclImpDialogObj( const XclImpRoot& rRoot );

protected:
    /** Sets additional properties for the current form control. */
    virtual void        DoProcessControl( ScfPropertySet& rPropSet ) const override;
    /** Returns the service name of the control component to be created. */
    virtual OUString DoGetServiceName() const override;
    /** Returns the type of the macro event to be created. */
    virtual XclTbxEventType DoGetEventType() const override;
};

/** An edit control. */
class XclImpEditObj final : public XclImpTbxObjBase
{
public:
    explicit            XclImpEditObj( const XclImpRoot& rRoot );

private:
    /** REturns true, if the field type is numeric. */
    bool                IsNumeric() const;

    /** Reads the contents of the a BIFF5 OBJ record from the passed stream. */
    virtual void        DoReadObj5( XclImpStream& rStrm, sal_uInt16 nNameLen, sal_uInt16 nMacroSize ) override;
    /** Reads the contents of the specified subrecord of a BIFF8 OBJ record from stream. */
    virtual void        DoReadObj8SubRec( XclImpStream& rStrm, sal_uInt16 nSubRecId, sal_uInt16 nSubRecSize ) override;
    /** Sets additional properties for the current form control. */
    virtual void        DoProcessControl( ScfPropertySet& rPropSet ) const override;
    /** Returns the service name of the control component to be created. */
    virtual OUString DoGetServiceName() const override;
    /** Returns the type of the macro event to be created. */
    virtual XclTbxEventType DoGetEventType() const override;

    sal_uInt16          mnContentType;
    sal_uInt16          mnMultiLine;
    sal_uInt16          mnScrollBar;
    sal_uInt16          mnListBoxObjId;
};

/** Base class of scrollable form controls (spin button, scrollbar, listbox, dropdown). */
class XclImpTbxObjScrollableBase : public XclImpTbxObjBase
{
public:
    explicit            XclImpTbxObjScrollableBase( const XclImpRoot& rRoot );

protected:
    /** Reads scrollbar data. */
    void                ReadSbs( XclImpStream& rStrm );

    /** Reads the contents of the specified subrecord of a BIFF8 OBJ record from stream. */
    virtual void        DoReadObj8SubRec( XclImpStream& rStrm, sal_uInt16 nSubRecId, sal_uInt16 nSubRecSize ) override;

protected:
    sal_uInt16          mnValue;
    sal_uInt16          mnMin;
    sal_uInt16          mnMax;
    sal_uInt16          mnStep;
    sal_uInt16          mnPageStep;
    sal_uInt16          mnOrient;
    sal_uInt16          mnThumbWidth;
    sal_uInt16          mnScrollFlags;
};

/** A spinbutton control. */
class XclImpSpinButtonObj : public XclImpTbxObjScrollableBase
{
public:
    explicit            XclImpSpinButtonObj( const XclImpRoot& rRoot );

protected:
    /** Reads the contents of the a BIFF5 OBJ record from the passed stream. */
    virtual void        DoReadObj5( XclImpStream& rStrm, sal_uInt16 nNameLen, sal_uInt16 nMacroSize ) override;
    /** Sets additional properties for the current form control. */
    virtual void        DoProcessControl( ScfPropertySet& rPropSet ) const override;
    /** Returns the service name of the control component to be created. */
    virtual OUString DoGetServiceName() const override;
    /** Returns the type of the macro event to be created. */
    virtual XclTbxEventType DoGetEventType() const override;
};

/** A scrollbar control. */
class XclImpScrollBarObj : public XclImpTbxObjScrollableBase
{
public:
    explicit            XclImpScrollBarObj( const XclImpRoot& rRoot );

protected:
    /** Reads the contents of the a BIFF5 OBJ record from the passed stream. */
    virtual void        DoReadObj5( XclImpStream& rStrm, sal_uInt16 nNameLen, sal_uInt16 nMacroSize ) override;
    /** Sets additional properties for the current form control. */
    virtual void        DoProcessControl( ScfPropertySet& rPropSet ) const override;
    /** Returns the service name of the control component to be created. */
    virtual OUString DoGetServiceName() const override;
    /** Returns the type of the macro event to be created. */
    virtual XclTbxEventType DoGetEventType() const override;
};

/** Base class for list controls (listbox, dropdown). */
class XclImpTbxObjListBase : public XclImpTbxObjScrollableBase
{
public:
    explicit            XclImpTbxObjListBase( const XclImpRoot& rRoot );

protected:
    /** Reads common listbox settings. */
    void                ReadLbsData( XclImpStream& rStrm );
    /** Sets common listbox/dropdown formatting attributes. */
    void                SetBoxFormatting( ScfPropertySet& rPropSet ) const;

protected:
    sal_uInt16          mnEntryCount;
    sal_uInt16          mnSelEntry;
    sal_uInt16          mnListFlags;
    sal_uInt16          mnEditObjId;
    bool                mbHasDefFontIdx;
};

/** A listbox control. */
class XclImpListBoxObj final : public XclImpTbxObjListBase
{
public:
    explicit            XclImpListBoxObj( const XclImpRoot& rRoot );

private:
    /** Reads listbox settings and selection. */
    void                ReadFullLbsData( XclImpStream& rStrm, std::size_t nRecLeft );

    /** Reads the contents of the a BIFF5 OBJ record from the passed stream. */
    virtual void        DoReadObj5( XclImpStream& rStrm, sal_uInt16 nNameLen, sal_uInt16 nMacroSize ) override;
    /** Reads the contents of the specified subrecord of a BIFF8 OBJ record from stream. */
    virtual void        DoReadObj8SubRec( XclImpStream& rStrm, sal_uInt16 nSubRecId, sal_uInt16 nSubRecSize ) override;
    /** Sets additional properties for the current form control. */
    virtual void        DoProcessControl( ScfPropertySet& rPropSet ) const override;
    /** Returns the service name of the control component to be created. */
    virtual OUString DoGetServiceName() const override;
    /** Returns the type of the macro event to be created. */
    virtual XclTbxEventType DoGetEventType() const override;

    ScfUInt8Vec         maSelection;
};

/** A dropdown listbox control. */
class XclImpDropDownObj final : public XclImpTbxObjListBase
{
public:
    explicit            XclImpDropDownObj( const XclImpRoot& rRoot );

private:
    /** Returns the type of the dropdown control. */
    sal_uInt16          GetDropDownType() const;

    /** Reads dropdown box settings. */
    void                ReadFullLbsData( XclImpStream& rStrm );

    /** Reads the contents of the a BIFF5 OBJ record from the passed stream. */
    virtual void        DoReadObj5( XclImpStream& rStrm, sal_uInt16 nNameLen, sal_uInt16 nMacroSize ) override;
    /** Reads the contents of the specified subrecord of a BIFF8 OBJ record from stream. */
    virtual void        DoReadObj8SubRec( XclImpStream& rStrm, sal_uInt16 nSubRecId, sal_uInt16 nSubRecSize ) override;
    /** Sets additional properties for the current form control. */
    virtual void        DoProcessControl( ScfPropertySet& rPropSet ) const override;
    /** Returns the service name of the control component to be created. */
    virtual OUString DoGetServiceName() const override;
    /** Returns the type of the macro event to be created. */
    virtual XclTbxEventType DoGetEventType() const override;

    sal_uInt16          mnLeft;
    sal_uInt16          mnTop;
    sal_uInt16          mnRight;
    sal_uInt16          mnBottom;
    sal_uInt16          mnDropDownFlags;
    sal_uInt16          mnLineCount;
    sal_uInt16          mnMinWidth;
};

/** A picture, an embedded or linked OLE object, or an OCX form control. */
class XclImpPictureObj : public XclImpRectObj, public XclImpControlHelper
{
public:
    explicit            XclImpPictureObj( const XclImpRoot& rRoot );
    /** Returns the ObjectName - can use non-obvious lookup for override in the associated vba document module stream**/
    virtual OUString GetObjName() const override;
    /** Returns the graphic imported from the IMGDATA record. */
    const Graphic& GetGraphic() const { return maGraphic; }

    /** Returns true, if the OLE object will be shown as symbol. */
    bool         IsSymbol() const { return mbSymbol; }
    /** Returns the storage name for the OLE object. */
    OUString            GetOleStorageName() const;

    /** Returns true, if this object is an OCX form control. */
    bool         IsOcxControl() const { return mbEmbedded && mbControl && mbUseCtlsStrm; }
    /** Returns the position in the 'Ctls' stream for additional form control data. */
    std::size_t  GetCtlsStreamPos() const { return mnCtlsStrmPos; }
    /** Returns the size in the 'Ctls' stream for additional form control data. */
    std::size_t  GetCtlsStreamSize() const { return mnCtlsStrmSize; }

protected:
    /** Reads the contents of the a BIFF3 OBJ record from the passed stream. */
    virtual void        DoReadObj3( XclImpStream& rStrm, sal_uInt16 nMacroSize ) override;
    /** Reads the contents of the a BIFF4 OBJ record from the passed stream. */
    virtual void        DoReadObj4( XclImpStream& rStrm, sal_uInt16 nMacroSize ) override;
    /** Reads the contents of the a BIFF5 OBJ record from the passed stream. */
    virtual void        DoReadObj5( XclImpStream& rStrm, sal_uInt16 nNameLen, sal_uInt16 nMacroSize ) override;
    /** Reads the contents of the specified subrecord of a BIFF8 OBJ record from stream. */
    virtual void        DoReadObj8SubRec( XclImpStream& rStrm, sal_uInt16 nSubRecId, sal_uInt16 nSubRecSize ) override;
    /** Creates and returns a new SdrObject from the contained data. Caller takes ownership! */
    virtual rtl::Reference<SdrObject> DoCreateSdrObj( XclImpDffConverter& rDffConv, const tools::Rectangle& rAnchorRect ) const override;
    /** Override to do additional processing on the SdrObject. */
    virtual void        DoPreProcessSdrObj( XclImpDffConverter& rDffConv, SdrObject& rSdrObj ) const override;

private:
    /** Reads and sets the picture flags from a BIFF3-BIFF5 OBJ picture record. */
    void                ReadFlags3( XclImpStream& rStrm );
    /** Reads the contents of the OBJFLAGS subrecord. */
    void                ReadFlags8( XclImpStream& rStrm );
    /** Reads the contents of the OBJPICTFMLA subrecord. */
    void                ReadPictFmla( XclImpStream& rStrm, sal_uInt16 nLinkSize );

private:
    Graphic             maGraphic;      /// Picture or OLE placeholder graphic.
    OUString            maClassName;    /// Class name of embedded OLE object.
    sal_uInt32          mnStorageId;    /// Identifier of the storage for this object.
    std::size_t         mnCtlsStrmPos;  /// Position in 'Ctls' stream for this control.
    std::size_t         mnCtlsStrmSize; /// Size in 'Ctls' stream for this control.
    bool                mbEmbedded;     /// true = Embedded OLE object.
    bool                mbLinked;       /// true = Linked OLE object.
    bool                mbSymbol;       /// true = Show as symbol.
    bool                mbControl;      /// true = Form control, false = OLE object.
    bool                mbUseCtlsStrm;  /// true = Form control data in 'Ctls' stream, false = Own storage.
};

// DFF stream conversion ======================================================

/** The solver container collects all connector rules for connected objects. */
class XclImpSolverContainer : public SvxMSDffSolverContainer
{
public:

    /** Inserts information about a new SdrObject. */
    void                InsertSdrObjectInfo( SdrObject& rSdrObj, sal_uInt32 nDffShapeId, ShapeFlag nDffFlags );
    /** Removes information of an SdrObject (and all child objects if it is a group). */
    void                RemoveSdrObjectInfo( SdrObject& rSdrObj );

    /** Inserts the SdrObject pointers into all connector rules. */
    void                UpdateConnectorRules();
    /** Removes all contained connector rules. */
    void                RemoveConnectorRules();

private:
    /** Updates the data of a connected shape in a connector rule. */
    void                UpdateConnection( sal_uInt32 nDffShapeId, SdrObject*& rpSdrObj, ShapeFlag* pnDffFlags = nullptr );

private:
    /** Stores data about an SdrObject processed during import. */
    struct XclImpSdrInfo
    {
        SdrObject*          mpSdrObj;       /// Pointer to an SdrObject.
        ShapeFlag           mnDffFlags;     /// Shape flags from DFF stream.
        explicit     XclImpSdrInfo() : mpSdrObj( nullptr ), mnDffFlags( ShapeFlag::NONE ) {}
        void         Set( SdrObject* pSdrObj, ShapeFlag nDffFlags )
                                { mpSdrObj = pSdrObj; mnDffFlags = nDffFlags; }
    };
    typedef std::map< sal_uInt32, XclImpSdrInfo > XclImpSdrInfoMap;
    typedef std::map< SdrObject*, sal_uInt32 >    XclImpSdrObjMap;

    XclImpSdrInfoMap    maSdrInfoMap;   /// Maps shape IDs to SdrObjects and flags.
    XclImpSdrObjMap     maSdrObjMap;    /// Maps SdrObjects to shape IDs.
};

/** Simple implementation of the SVX DFF manager. Implements resolving palette
    colors. Used by XclImpDffPropSet (as is), extended by XclImpDffConverter.
 */
class XclImpSimpleDffConverter : public SvxMSDffManager, protected XclImpRoot
{
public:
    explicit            XclImpSimpleDffConverter( const XclImpRoot& rRoot, SvStream& rDffStrm );
    virtual             ~XclImpSimpleDffConverter() override;

protected:
    /** Returns a color from the Excel color palette. */
    virtual bool        GetColorFromPalette( sal_uInt16 nIndex, Color& rColor ) const override;
};

/** This is the central instance for converting binary DFF data into shape
    objects. Used for all sheet shapes and shapes embedded in chart objects.

    The class derives from SvxMSDffManager and SvxMSConvertOCXControls and
    contains core implementation of DFF stream import and OCX form control
    import.
 */
class XclImpDffConverter : public XclImpSimpleDffConverter, private oox::ole::MSConvertOCXControls
{
public:
    explicit            XclImpDffConverter( const XclImpRoot& rRoot, SvStream& rDffStrm );
    virtual             ~XclImpDffConverter() override;

    /** Initializes the internal progress bar with the passed size and starts it. */
    void                StartProgressBar( std::size_t nProgressSize );
    /** Increase the progress bar by the passed value. */
    void                Progress( std::size_t nDelta = 1 );

    /** Initially called before the objects of the passed drawing manager are converted. */
    void                InitializeDrawing( XclImpDrawing& rDrawing, SdrModel& rSdrModel, SdrPage& rSdrPage );
    /** Processes BIFF5 drawing objects without DFF data, inserts into the passed object list. */
    void                ProcessObject( SdrObjList& rObjList, XclImpDrawObjBase& rDrawObj );
    /** Processes all objects in the passed list. */
    void                ProcessDrawing( const XclImpDrawObjVector& rDrawObjs );
    /** Processes a drawing container in the passed DFF stream, converts all objects. */
    void                ProcessDrawing( SvStream& rDffStrm );
    /** Finally called after the objects of the passed drawing manager have been converted. */
    void                FinalizeDrawing();

    /** Creates the SdrObject for the passed Excel TBX form control object. */
    rtl::Reference<SdrObject>        CreateSdrObject( const XclImpTbxObjBase& rTbxObj, const tools::Rectangle& rAnchorRect );
    /** Creates the SdrObject for the passed Excel OLE object or OCX form control object. */
    rtl::Reference<SdrObject>        CreateSdrObject( const XclImpPictureObj& rPicObj, const tools::Rectangle& rAnchorRect );

    /** Returns true, if the conversion of OLE objects is supported. */
    bool                SupportsOleObjects() const;
    /** Returns the default text margin in drawing layer units. */
    sal_Int32    GetDefaultTextMargin() const { return mnDefTextMargin; }

private:
    // virtual functions of SvxMSDffManager

    /** Reads the client anchor from the DFF stream and sets it at the correct object. */
    virtual void        ProcessClientAnchor2(
                            SvStream& rDffStrm,
                            DffRecordHeader& rHeader,
                            DffObjData& rObjData ) override;
    /** Processes a DFF object, reads properties from DFF stream. */
    virtual rtl::Reference<SdrObject>  ProcessObj(
                            SvStream& rDffStrm,
                            DffObjData& rDffObjData,
                            SvxMSDffClientData& rClientData,
                            tools::Rectangle& rTextRect,
                            SdrObject* pOldSdrObj ) override;

    /** Finalize a DFF object, sets anchor after nested objs have been loaded. */
    virtual SdrObject*  FinalizeObj(
                            DffObjData& rDffObjData,
                            SdrObject* pOldSdrObj ) override;

    // virtual functions of SvxMSConvertOCXControls

    /** Inserts the passed control rxFComp into the form. Needs call to SetCurrentForm() before. */
    virtual bool    InsertControl(
                            const css::uno::Reference<
                                css::form::XFormComponent >& rxFormComp,
                            const css::awt::Size& rSize,
                            css::uno::Reference<
                                css::drawing::XShape >* pxShape,
                            bool bFloatingCtrl ) override;

private:
    /** Data per registered drawing manager, will be stacked for recursive calls. */
    struct XclImpDffConvData
    {
        XclImpDrawing&      mrDrawing;          /// Current drawing container with all drawing objects.
        SdrModel&           mrSdrModel;         /// The SdrModel of the drawing manager.
        SdrPage&            mrSdrPage;          /// The SdrPage of the drawing manager.
        XclImpSolverContainer maSolverCont;     /// The solver container for connector rules.
        css::uno::Reference< css::form::XForm >
                            mxCtrlForm;         /// Controls form of current drawing page.
        sal_Int32           mnLastCtrlIndex;    /// Last insertion index of a form control (for macro events).
        bool                mbHasCtrlForm;      /// True = mxCtrlForm is initialized (but maybe still null).

        explicit            XclImpDffConvData( XclImpDrawing& rDrawing,
                                SdrModel& rSdrModel, SdrPage& rSdrPage );
    };

    /** Returns the current drawing manager data struct from top of the stack. */
    XclImpDffConvData&      GetConvData();
    /** Returns the current drawing manager data struct from top of the stack. */
    const XclImpDffConvData& GetConvData() const;

    /** Reads contents of a hyperlink property and returns the extracted URL. */
    OUString            ReadHlinkProperty( SvStream& rDffStrm ) const;

    /** Processes a drawing container (all drawing data of a sheet). */
    bool                ProcessDgContainer( SvStream& rDffStrm, const DffRecordHeader& rDgHeader );
    /** Processes the global shape group container (all shapes of a sheet). */
    bool                ProcessShGrContainer( SvStream& rDffStrm, const DffRecordHeader& rShGrHeader );
    /** Processes the solver container (connectors of a sheet). */
    bool                ProcessSolverContainer( SvStream& rDffStrm, const DffRecordHeader& rSolverHeader );
    /** Processes a shape or shape group container (one top-level shape). */
    bool                ProcessShContainer( SvStream& rDffStrm, const DffRecordHeader& rShHeader );

    /** Inserts the passed SdrObject into the document. This function takes ownership of pSdrObj! */
    void                InsertSdrObject( SdrObjList& rObjList, const XclImpDrawObjBase& rDrawObj, SdrObject* pSdrObj );
    /** Initializes the mxCtrlForm referring to the standard controls form. */
    void                InitControlForm();
    /** Notify that this document contains a macro event handler */
    void                NotifyMacroEventRead();

private:
    typedef std::shared_ptr< ScfProgressBar >     ScfProgressBarRef;
    typedef std::shared_ptr< XclImpDffConvData >  XclImpDffConvDataRef;

    rtl::Reference<SotStorageStream> mxCtlsStrm; /// The 'Ctls' stream for OCX form controls.
    ScfProgressBarRef   mxProgress;         /// The progress bar used in ProcessObj().
    std::vector< XclImpDffConvDataRef >
                        maDataStack;     /// Stack for registered drawing managers.
    sal_uInt32          mnOleImpFlags;      /// Application OLE import settings.
    sal_Int32           mnDefTextMargin;    /// Default margin in text boxes.
    bool mbNotifyMacroEventRead;            /// If we have already seen a macro event
};

// Drawing manager ============================================================

/** Base class for a container for all objects on a drawing (spreadsheet or
    embedded chart object). */
class XclImpDrawing : protected XclImpRoot
{
public:
    explicit            XclImpDrawing( const XclImpRoot& rRoot, bool bOleObjects );
    virtual             ~XclImpDrawing() override;

    /** Reads and returns a bitmap from the IMGDATA record. */
    static Graphic      ReadImgData( const XclImpRoot& rRoot, XclImpStream& rStrm );

    /** Reads a plain OBJ record (without leading DFF data). */
    void                ReadObj( XclImpStream& rStrm );
    /** Reads the MSODRAWING or MSODRAWINGSELECTION record. */
    void                ReadMsoDrawing( XclImpStream& rStrm );

    /** Returns true, if the conversion of OLE objects is supported. */
    bool         SupportsOleObjects() const { return mbOleObjs; }
    /** Finds the OBJ record data related to the DFF shape at the passed position. */
    XclImpDrawObjRef    FindDrawObj( const DffRecordHeader& rHeader ) const;
    /** Finds the OBJ record data specified by the passed object identifier. */
    XclImpDrawObjRef    FindDrawObj( sal_uInt16 nObjId ) const;
    /** Finds the textbox data related to the DFF shape at the passed position. */
    const XclImpObjTextData* FindTextData( const DffRecordHeader& rHeader ) const;

    /** Sets the object with the passed identification to be skipped on import. */
    void                SetSkipObj( sal_uInt16 nObjId );
    /** Returns the size of the progress bar shown while processing all objects. */
    std::size_t         GetProgressSize() const;

    /** Derived classes calculate the resulting rectangle of the passed anchor. */
    virtual tools::Rectangle   CalcAnchorRect( const XclObjAnchor& rAnchor, bool bDffAnchor ) const = 0;
    /** Called whenever an object has been inserted into the draw page. */
    virtual void        OnObjectInserted( const XclImpDrawObjBase& rDrawObj ) = 0;

protected:
    /** Appends a new drawing object to the list of raw objects (without DFF data). */
    void                AppendRawObject( const XclImpDrawObjRef& rxDrawObj );
    /** Converts all objects and inserts them into the current drawing page. */
    void                ImplConvertObjects( XclImpDffConverter& rDffConv, SdrModel& rSdrModel, SdrPage& rSdrPage );

private:
    /** Reads and returns a bitmap from WMF/PICT format. */
    static void         ReadWmf( Graphic& rGraphic, XclImpStream& rStrm );
    /** Reads and returns a bitmap from BMP format. */
    static void         ReadBmp( Graphic& rGraphic, const XclImpRoot& rRoot, XclImpStream& rStrm );

    /** Reads contents of a DFF record and append data to internal DFF stream. */
    void                ReadDffRecord( XclImpStream& rStrm );
    /** Reads a BIFF8 OBJ record following an MSODRAWING record. */
    void                ReadObj8( XclImpStream& rStrm );
    /** Reads the TXO record and following CONTINUE records containing string and formatting. */
    void                ReadTxo( XclImpStream& rStrm );

private:
    typedef std::map< std::size_t, XclImpDrawObjRef > XclImpObjMap;
    typedef std::map< sal_uInt16, XclImpDrawObjRef >  XclImpObjMapById;
    typedef std::shared_ptr< XclImpObjTextData >      XclImpObjTextRef;
    typedef std::map< std::size_t, XclImpObjTextRef > XclImpObjTextMap;

    XclImpDrawObjVector maRawObjs;          /// BIFF5 objects without DFF data.
    SvMemoryStream      maDffStrm;          /// Copy of the DFF page stream in memory.
    XclImpObjMap        maObjMap;           /// Maps BIFF8 drawing objects to DFF stream position.
    XclImpObjMapById    maObjMapId;         /// Maps BIFF8 drawing objects to object ID.
    XclImpObjTextMap    maTextMap;          /// Maps BIFF8 TXO textbox data to DFF stream position.
    ScfUInt16Vec        maSkipObjs;         /// IDs of all objects to be skipped.
    bool                mbOleObjs;          /// True = draw model supports OLE objects.
};

/** Drawing manager of a single sheet. */
class XclImpSheetDrawing : public XclImpDrawing
{
public:
    explicit            XclImpSheetDrawing( const XclImpRoot& rRoot, SCTAB nScTab );

    /** Reads the NOTE record. */
    void                ReadNote( XclImpStream& rStrm );
    /** Inserts a new chart object and reads the chart substream (BOF/EOF block).
        @descr  Used to import chart sheets, which do not have a corresponding OBJ record. */
    void                ReadTabChart( XclImpStream& rStrm );

    /** Returns the total cell range covered by any shapes in the sheet. */
    const ScRange& GetUsedArea() const { return maScUsedArea; }
    /** Converts all objects and inserts them into the sheet drawing page. */
    void                ConvertObjects( XclImpDffConverter& rDffConv );

    /** Calculate the resulting rectangle of the passed anchor. */
    virtual tools::Rectangle   CalcAnchorRect( const XclObjAnchor& rAnchor, bool bDffAnchor ) const override;
    /** On call, updates the used area of the sheet. */
    virtual void        OnObjectInserted( const XclImpDrawObjBase& rDrawObj ) override;

private:
    /** Reads a BIFF3-BIFF5 NOTE record. */
    void                ReadNote3( XclImpStream& rStrm );
    /** Reads a BIFF8 NOTE record. */
    void                ReadNote8( XclImpStream& rStrm );

private:
    ScRange             maScUsedArea;       /// Sheet index and used area in this sheet.
};

// The object manager =========================================================

/** Stores all drawing and OLE objects and additional data related to these objects. */
class XclImpObjectManager : protected XclImpRoot
{
public:
    explicit            XclImpObjectManager( const XclImpRoot& rRoot );
    virtual             ~XclImpObjectManager() override;

    /** Reads the MSODRAWINGGROUP record. */
    void                ReadMsoDrawingGroup( XclImpStream& rStrm );

    /** Returns (initially creates) the drawing manager of the specified sheet. */
    XclImpSheetDrawing& GetSheetDrawing( SCTAB nScTab );
    /** Inserts all objects into the Calc document. */
    void                ConvertObjects();

    /** Returns the default name for the passed object. */
    OUString       GetDefaultObjName( const XclImpDrawObjBase& rDrawObj ) const;
    /** Returns the used area in the sheet with the passed index. */
    ScRange             GetUsedArea( SCTAB nScTab ) const;
    /** Sets the container to receive overridden shape/ctrl names from
        the filter. */
    void SetOleNameOverrideInfo( const css::uno::Reference< css::container::XNameContainer >& rxOverrideInfo ) {  mxOleCtrlNameOverride = rxOverrideInfo; }
    /** Returns the name of overridden name ( or zero length string ) for
        associated object id. */
    OUString GetOleNameOverride( SCTAB nTab, sal_uInt16 nObjId );

private:
    typedef std::map< sal_uInt16, OUString >          DefObjNameMap;
    typedef std::shared_ptr< XclImpSheetDrawing >     XclImpSheetDrawingRef;
    typedef std::map< SCTAB, XclImpSheetDrawingRef >  XclImpSheetDrawingMap;

    css::uno::Reference< css::container::XNameContainer > mxOleCtrlNameOverride;
    DefObjNameMap       maDefObjNames;      /// Default base names for all object types.
    SvMemoryStream      maDggStrm;          /// Copy of global DFF data (DGG container) in memory.
    XclImpSheetDrawingMap maSheetDrawings;  /// Drawing managers of all sheets.
};

// DFF property set helper ====================================================

/** This class reads a DFF property set (msofbtOPT record).

    It can return separate property values or an item set which contains items
    translated from these properties.
 */
class XclImpDffPropSet : protected XclImpRoot
{
public:
    explicit            XclImpDffPropSet( const XclImpRoot& rRoot );

    /** Reads a DFF property set from the stream.
        @descr  The stream must point to the start of a DFF record containing properties. */
    void                Read( XclImpStream& rStrm );

    /** Returns the specified property or zero, if not extant. */
    sal_uInt32          GetPropertyValue( sal_uInt16 nPropId ) const;

    /** Translates the properties and fills the item set. */
    void                FillToItemSet( SfxItemSet& rItemSet ) const;

private:
    typedef std::unique_ptr<SvMemoryStream> SvMemoryStreamPtr;

    SvMemoryStream      maDummyStrm;    /// Dummy DGG stream for DFF manager.
    XclImpSimpleDffConverter maDffConv; /// DFF converter used to resolve palette colors.
    SvMemoryStreamPtr   mxMemStrm;      /// Helper stream.
};

XclImpStream& operator>>( XclImpStream& rStrm, XclImpDffPropSet& rPropSet );

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
