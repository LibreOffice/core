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

#ifndef SC_XICHART_HXX
#define SC_XICHART_HXX

#include <vector>
#include <map>
#include <set>
#include <list>

#include <svl/itemset.hxx>

#include "rangelst.hxx"
#include "types.hxx"
#include "xlchart.hxx"
#include "xlstyle.hxx"
#include "xiescher.hxx"
#include "xistring.hxx"
#include <boost/shared_ptr.hpp>
#include <boost/ptr_container/ptr_map.hpp>

namespace com { namespace sun { namespace star {
    namespace awt
    {
        struct Rectangle;
    }
    namespace frame
    {
        class XModel;
    }
    namespace drawing
    {
        class XShape;
    }
    namespace chart2
    {
        struct ScaleData;
        class XChartDocument;
        class XDiagram;
        class XCoordinateSystem;
        class XChartType;
        class XDataSeries;
        class XRegressionCurve;
        class XAxis;
        class XLegend;
        class XTitle;
        class XFormattedString;
        namespace data
        {
            class XDataProvider;
            class XDataSequence;
            class XLabeledDataSequence;
        }
    }
} } }

struct XclObjLineData;
struct XclObjFillData;

// Common =====================================================================

struct XclImpChRootData;
class XclImpChChart;
class ScTokenArray;

/** Base class for complex chart classes, provides access to other components of the chart. */
class XclImpChRoot : public XclImpRoot
{
public:
    explicit            XclImpChRoot( const XclImpRoot& rRoot, XclImpChChart& rChartData );
    virtual             ~XclImpChRoot();

    /** Returns this root instance - for code readability in derived classes. */
    inline const XclImpChRoot& GetChRoot() const { return *this; }
    /** Returns a reference to the parent chart data object. */
    XclImpChChart&      GetChartData() const;
    /** Returns chart type info for a unique chart type identifier. */
    const XclChTypeInfo& GetChartTypeInfo( XclChTypeId eType ) const;
    /** Returns the first fitting chart type info for an Excel chart type record identifier. */
    const XclChTypeInfo& GetChartTypeInfo( sal_uInt16 nRecId ) const;
    /** Returns an info struct about auto formatting for the passed object type. */
    const XclChFormatInfo& GetFormatInfo( XclChObjectType eObjType ) const;

    /** Returns the default text color for charts. */
    Color               GetFontAutoColor() const;
    /** Returns the automatic line color of linear series. */
    Color               GetSeriesLineAutoColor( sal_uInt16 nFormatIdx ) const;
    /** Returns the automatic fill color of filled series. */
    Color               GetSeriesFillAutoColor( sal_uInt16 nFormatIdx ) const;

    /** Starts the API chart document conversion. Must be called once before all API conversion. */
    void                InitConversion(
        const ::com::sun::star::uno::Reference<
            com::sun::star::chart2::XChartDocument>& xChartDoc, const Rectangle& rChartRect ) const;

    /** Finishes the API chart document conversion. Must be called once after all API conversion. */
    void                FinishConversion( XclImpDffConverter& rDffConv ) const;

    /** Returns the data provider for the chart document. */
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataProvider >
                        GetDataProvider() const;
    /** Returns the drawing shape interface of the specified title object. */
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                        GetTitleShape( const XclChTextKey& rTitleKey ) const;

    /** Converts the passed horizontal coordinate from Excel chart units into 1/100 mm. */
    sal_Int32           CalcHmmFromChartX( sal_Int32 nPosX ) const;
    /** Converts the passed vertical coordinate from Excel chart units into 1/100 mm. */
    sal_Int32           CalcHmmFromChartY( sal_Int32 nPosY ) const;
    /** Converts the passed rectangle from Excel chart units into 1/100 mm. */
    ::com::sun::star::awt::Rectangle CalcHmmFromChartRect( const XclChRectangle& rRect ) const;

    /** Converts the passed horizontal coordinate from 1/100 mm into a relative position. */
    double              CalcRelativeFromHmmX( sal_Int32 nPosX ) const;
    /** Converts the passed vertical coordinate from 1/100 mm into a relative position. */
    double              CalcRelativeFromHmmY( sal_Int32 nPosY ) const;

    /** Converts the passed horizontal coordinate from Excel chart units into a relative position. */
    double              CalcRelativeFromChartX( sal_Int32 nPosX ) const;
    /** Converts the passed vertical coordinate from Excel chart units into a relative position. */
    double              CalcRelativeFromChartY( sal_Int32 nPosY ) const;

    /** Writes all line properties to the passed property set. */
    void                ConvertLineFormat(
                            ScfPropertySet& rPropSet,
                            const XclChLineFormat& rLineFmt,
                            XclChPropertyMode ePropMode ) const;
    /** Writes solid area properties to the passed property set. */
    void                ConvertAreaFormat(
                            ScfPropertySet& rPropSet,
                            const XclChAreaFormat& rAreaFmt,
                            XclChPropertyMode ePropMode ) const;
    /** Writes gradient or bitmap area properties to the passed property set. */
    void                ConvertEscherFormat(
                            ScfPropertySet& rPropSet,
                            const XclChEscherFormat& rEscherFmt,
                            const XclChPicFormat* pPicFmt,
                            sal_uInt32 nDffFillType,
                            XclChPropertyMode ePropMode ) const;
    /** Writes font properties to the passed property set. */
    void                ConvertFont(
                            ScfPropertySet& rPropSet,
                            sal_uInt16 nFontIdx,
                            const Color* pFontColor = 0 ) const;

    /** Writes the pie rotation property for the passed angle. */
    static void         ConvertPieRotation(
                            ScfPropertySet& rPropSet,
                            sal_uInt16 nAngle );

private:
    typedef boost::shared_ptr< XclImpChRootData > XclImpChRootDataRef;
    XclImpChRootDataRef mxChData;           /// Reference to the root data object.
};

// ----------------------------------------------------------------------------

/** Base class for chart record groups. Provides helper functions to read sub records.

    A chart record group consists of a header record, followed by a CHBEGIN
    record, followed by group sub records, and finished with a CHEND record.
 */
class XclImpChGroupBase
{
public:
    virtual             ~XclImpChGroupBase();

    /** Reads the entire record group.
        @descr  First calls ReadHeaderRecord() to read the contents of the
            header record. Then tries to read the sub records. If next record
            is a CHBEGIN record, ReadSubRecord() is called for each following
            record until a CHEND record is found. */
    void                ReadRecordGroup( XclImpStream& rStrm );

    /** Helper to skip a CHBEGIN/CHEND block, includes nested blocks. */
    static void         SkipBlock( XclImpStream& rStrm );

    /** Derived classes implement to read the group header record. */
    virtual void        ReadHeaderRecord( XclImpStream& rStrm ) = 0;
    /** Derived classes implement to read a record from the group. */
    virtual void        ReadSubRecord( XclImpStream& rStrm ) = 0;
};

// Frame formatting ===========================================================

class XclImpChFramePos
{
public:
    /** Reads the CHFRAMEPOS record (frame position and size). */
    void                ReadChFramePos( XclImpStream& rStrm );

    /** Returns read-only access to the imported frame position data. */
    inline const XclChFramePos& GetFramePosData() const { return maData; }

private:
    XclChFramePos       maData;             /// Position of the frame.
};

typedef boost::shared_ptr< XclImpChFramePos > XclImpChFramePosRef;

// ----------------------------------------------------------------------------

/** The CHLINEFORMAT record containing line formatting data. */
class XclImpChLineFormat
{
public:
    /** Creates a new line format object with automatic formatting. */
    inline explicit     XclImpChLineFormat() {}
    /** Creates a new line format object with the passed formatting. */
    inline explicit     XclImpChLineFormat( const XclChLineFormat& rLineFmt ) : maData( rLineFmt ) {}

    /** Reads the CHLINEFORMAT record (basic line properties). */
    void                ReadChLineFormat( XclImpStream& rStrm );

    /** Returns true, if the line format is set to automatic. */
    inline bool         IsAuto() const { return ::get_flag( maData.mnFlags, EXC_CHLINEFORMAT_AUTO ); }
    /** Returns true, if the line style is set to something visible. */
    inline bool         HasLine() const { return IsAuto() || (maData.mnPattern != EXC_CHLINEFORMAT_NONE); }
    /** Returns the line width of this line format (returns 'single', if the line is invisible). */
    inline sal_Int16    GetWeight() const { return (IsAuto() || !HasLine()) ? EXC_CHLINEFORMAT_SINGLE : maData.mnWeight; }
    /** Returns true, if the "show axis" flag is set. */
    inline bool         IsShowAxis() const { return ::get_flag( maData.mnFlags, EXC_CHLINEFORMAT_SHOWAXIS ); }

    /** Converts and writes the contained data to the passed property set. */
    void                Convert( const XclImpChRoot& rRoot,
                            ScfPropertySet& rPropSet, XclChObjectType eObjType,
                            sal_uInt16 nFormatIdx = EXC_CHDATAFORMAT_UNKNOWN ) const;

private:
    XclChLineFormat     maData;             /// Contents of the CHLINEFORMAT record.
};

typedef boost::shared_ptr< XclImpChLineFormat > XclImpChLineFormatRef;

// ----------------------------------------------------------------------------

/** The CHAREAFORMAT record containing simple area formatting data (solid or patterns). */
class XclImpChAreaFormat
{
public:
    /** Creates a new area format object with automatic formatting. */
    inline explicit     XclImpChAreaFormat() {}
    /** Creates a new area format object with the passed formatting. */
    inline explicit     XclImpChAreaFormat( const XclChAreaFormat& rAreaFmt ) : maData( rAreaFmt ) {}

    /** Reads the CHAREAFORMAT record (basic fill properties, e.g. transparent or colored). */
    void                ReadChAreaFormat( XclImpStream& rStrm );

    /** Returns true, if the area format is set to automatic. */
    inline bool         IsAuto() const { return ::get_flag( maData.mnFlags, EXC_CHAREAFORMAT_AUTO ); }
    /** Returns true, if the area style is set to something visible. */
    inline bool         HasArea() const { return IsAuto() || (maData.mnPattern != EXC_PATT_NONE); }

    /** Converts and writes the contained data to the passed property set. */
    void                Convert( const XclImpChRoot& rRoot,
                            ScfPropertySet& rPropSet, XclChObjectType eObjType,
                            sal_uInt16 nFormatIdx = EXC_CHDATAFORMAT_UNKNOWN ) const;

private:
    XclChAreaFormat     maData;             /// Contents of the CHAREAFORMAT record.
};

typedef boost::shared_ptr< XclImpChAreaFormat > XclImpChAreaFormatRef;

// ----------------------------------------------------------------------------

/** The CHESCHERFORMAT record containing complex area formatting data (bitmaps, hatches). */
class XclImpChEscherFormat : public XclImpChGroupBase
{
public:
    explicit            XclImpChEscherFormat( const XclImpRoot& rRoot );

    /** Reads the CHESCHERFORMAT record (complex fill data) (called by base class). */
    virtual void        ReadHeaderRecord( XclImpStream& rStrm );
    /** Reads a record from the CHESCHERFORMAT group (called by base class). */
    virtual void        ReadSubRecord( XclImpStream& rStrm );

    /** Converts and writes the contained data to the passed property set. */
    void                Convert( const XclImpChRoot& rRoot, ScfPropertySet& rPropSet,
                            XclChObjectType eObjType, bool bUsePicFmt ) const;

private:
    XclChEscherFormat   maData;             /// Fill properties for complex areas (CHESCHERFORMAT record).
    XclChPicFormat      maPicFmt;           /// Image options, e.g. stretched, stacked (CHPICFORMAT record).
    sal_uInt32          mnDffFillType;      /// Fill type imported from the DFF property set.
};

typedef boost::shared_ptr< XclImpChEscherFormat > XclImpChEscherFormatRef;

// ----------------------------------------------------------------------------

/** Base class for record groups containing frame formatting.

    Frame formatting can be part of several record groups, e.g. CHFRAME,
    CHDATAFORMAT, CHDROPBAR. It consists of CHLINEFORMAT, CHAREAFORMAT, and
    CHESCHERFORMAT group.
 */
class XclImpChFrameBase : public XclImpChGroupBase
{
public:
    /** Creates a new frame object without internal formatting objects. */
    inline explicit     XclImpChFrameBase() {}
    /** Creates a new frame object with specific default formatting. */
    explicit            XclImpChFrameBase( const XclChFormatInfo& rFmtInfo );

    /** Reads a frame formatting record (called by base class). */
    virtual void        ReadSubRecord( XclImpStream& rStrm );

    /** Returns true, if the line format is set to automatic. */
    inline bool         IsAutoLine() const { return !mxLineFmt || mxLineFmt->IsAuto(); }
    /** Returns true, if the line style is set to something visible. */
    inline bool         HasLine() const { return IsAutoLine() || mxLineFmt->HasLine(); }
    /** Returns the line weight used for this frame. */
    inline sal_Int16    GetLineWeight() const { return mxLineFmt ? mxLineFmt->GetWeight() : EXC_CHLINEFORMAT_SINGLE; }

    /** Returns true, if the area format is set to automatic. */
    inline bool         IsAutoArea() const { return !mxEscherFmt && (!mxAreaFmt || mxAreaFmt->IsAuto()); }
    /** Returns true, if the area style is set to something visible. */
    inline bool         HasArea() const { return mxEscherFmt || IsAutoArea() || mxAreaFmt->HasArea(); }

protected:
    /** Converts and writes the contained line formatting to the passed property set. */
    void                ConvertLineBase( const XclImpChRoot& rRoot,
                            ScfPropertySet& rPropSet, XclChObjectType eObjType,
                            sal_uInt16 nFormatIdx = EXC_CHDATAFORMAT_UNKNOWN ) const;
    /** Converts and writes the contained area formatting to the passed property set. */
    void                ConvertAreaBase( const XclImpChRoot& rRoot,
                            ScfPropertySet& rPropSet, XclChObjectType eObjType,
                            sal_uInt16 nFormatIdx = EXC_CHDATAFORMAT_UNKNOWN, bool bUsePicFmt = false ) const;
    /** Converts and writes the contained data to the passed property set. */
    void                ConvertFrameBase( const XclImpChRoot& rRoot,
                            ScfPropertySet& rPropSet, XclChObjectType eObjType,
                            sal_uInt16 nFormatIdx = EXC_CHDATAFORMAT_UNKNOWN, bool bUsePicFmt = false ) const;

protected:
    XclImpChLineFormatRef mxLineFmt;        /// Line format (CHLINEFORMAT record).
    XclImpChAreaFormatRef mxAreaFmt;        /// Area format (CHAREAFORMAT record).
    XclImpChEscherFormatRef mxEscherFmt;    /// Complex area format (CHESCHERFORMAT record).
};

// ----------------------------------------------------------------------------

/** Represents the CHFRAME record group containing object frame properties.

    The CHFRAME group consists of: CHFRAME, CHBEGIN, CHLINEFORMAT,
    CHAREAFORMAT, CHESCHERFORMAT group, CHEND.
 */
class XclImpChFrame : public XclImpChFrameBase, protected XclImpChRoot
{
public:
    /** Creates a new frame object with specific default formatting. */
    explicit            XclImpChFrame(
                            const XclImpChRoot& rRoot,
                            XclChObjectType eObjType );

    /** Reads the CHFRAME record (called by base class). */
    virtual void        ReadHeaderRecord( XclImpStream& rStrm );

    /** Sets formatting from BIFF3-BIFF5 OBJ record, if own formatting is invisible. */
    void                UpdateObjFrame( const XclObjLineData& rLineData, const XclObjFillData& rFillData );

    /** Converts and writes the contained data to the passed property set. */
    void                Convert( ScfPropertySet& rPropSet, bool bUsePicFmt = false ) const;

private:
    XclChFrame          maData;             /// Contents of the CHFRAME record.
    XclChObjectType     meObjType;          /// Type of the represented object.
};

typedef boost::shared_ptr< XclImpChFrame > XclImpChFrameRef;

// Source links ===============================================================

class XclImpChSourceLink : protected XclImpChRoot
{
public:
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence >   XDataSequenceRef;
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XFormattedString >      XFormattedStringRef;
    typedef ::com::sun::star::uno::Sequence< XFormattedStringRef >                              XFormattedStringSeq;

public:
    explicit            XclImpChSourceLink( const XclImpChRoot& rRoot );
    virtual             ~XclImpChSourceLink();

    /** Reads the CHSOURCELINK record (link to source data). */
    void                ReadChSourceLink( XclImpStream& rStrm );
    /** Sets explicit string data for this text object. */
    void                SetString( const String& rString );
    /** Sets formatting runs read from a CHFORMATRUNS record. */
    void                SetTextFormats( const XclFormatRunVec& rFormats );

    /** Returns the destination object (title, values, category, ...). */
    inline sal_uInt8    GetDestType() const { return maData.mnDestType; }
    /** Returns the link type (to worksheet, directly, default, ...). */
    inline sal_uInt8    GetLinkType() const { return maData.mnLinkType; }

    /** Returns true, if the source link contains explicit string data. */
    inline bool         HasString() const { return mxString && !mxString->IsEmpty(); }
    /** Returns explicit string data or an empty string. */
    OUString            GetString() const { if (mxString) return mxString->GetText(); return OUString(); }
    /** Returns the number of data points of this source link. */
    sal_uInt16          GetCellCount() const;

    /** Converts and writes the contained number format to the passed property set. */
    void                ConvertNumFmt( ScfPropertySet& rPropSet, bool bPercent ) const;

    /** Creates a data sequence containing the link into the Calc document. */
    XDataSequenceRef    CreateDataSequence( const OUString& rRole ) const;
    /** Creates a sequence of formatted string objects. */
    XFormattedStringSeq CreateStringSequence( const XclImpChRoot& rRoot,
                            sal_uInt16 nLeadFontIdx, const Color& rLeadFontColor ) const;

    void                FillSourceLink(::std::vector<ScTokenRef>& rTokens) const;

private:
    XclChSourceLink     maData;             /// Contents of the CHSOURCELINK record.
    XclImpStringRef     mxString;           /// Text data (CHSTRING record).
    boost::shared_ptr< ScTokenArray> mxTokenArray;     /// Token array representing the data ranges.
};

typedef boost::shared_ptr< XclImpChSourceLink > XclImpChSourceLinkRef;

// Text =======================================================================

/** Base class for objects with font settings. Provides font conversion helper functions. */
class XclImpChFontBase
{
public:
    virtual             ~XclImpChFontBase();

    /** Derived classes return the leading font index for the text object. */
    virtual sal_uInt16  GetFontIndex() const = 0;
    /** Derived classes return the leading font color for the text object. */
    virtual Color       GetFontColor() const = 0;
    /** Derived classes return the rotation value for the text object. */
    virtual sal_uInt16  GetRotation() const = 0;

    /** Converts and writes the contained font settings to the passed property set. */
    void                ConvertFontBase( const XclImpChRoot& rRoot, ScfPropertySet& rPropSet ) const;
    /** Converts and writes the contained rotation settings to the passed property set. */
    void                ConvertRotationBase( const XclImpChRoot& rRoot, ScfPropertySet& rPropSet, bool bSupportsStacked ) const;
};

// ----------------------------------------------------------------------------

/** The CHFONT record containing a font index for text objects. */
class XclImpChFont
{
public:
    explicit            XclImpChFont();
    /** Reads the CHFONT record (font index). */
    void                ReadChFont( XclImpStream& rStrm );

    /** Returns the contained font index. */
    inline sal_uInt16   GetFontIndex() const { return mnFontIdx; }

private:
    sal_uInt16          mnFontIdx;          /// Index into font buffer.
};

typedef boost::shared_ptr< XclImpChFont > XclImpChFontRef;

// ----------------------------------------------------------------------------

/** Represents the CHTEXT record group containing text object properties.

    The CHTEXT group consists of: CHTEXT, CHBEGIN, CHFRAMEPOS, CHFONT,
    CHFORMATRUNS, CHSOURCELINK, CHSTRING, CHFRAME group, CHOBJECTLINK, and CHEND.
 */
class XclImpChText : public XclImpChGroupBase, public XclImpChFontBase, protected XclImpChRoot
{
public:
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XTitle > XTitleRef;

public:
    explicit            XclImpChText( const XclImpChRoot& rRoot );

    /** Reads the CHTEXT record (called by base class). */
    virtual void        ReadHeaderRecord( XclImpStream& rStrm );
    /** Reads a record from the CHTEXT group (called by base class). */
    virtual void        ReadSubRecord( XclImpStream& rStrm );

    /** Returns the leading font index for the text object. */
    virtual sal_uInt16  GetFontIndex() const;
    /** Returns the leading font color for the text object. */
    virtual Color       GetFontColor() const;
    /** Returns the rotation value for the text object. */
    virtual sal_uInt16  GetRotation() const;

    /** Sets explicit string data for this text object. */
    void                SetString( const String& rString );
    /** Updates missing parts of this text object from the passed object. */
    void                UpdateText( const XclImpChText* pParentText );
    /** Updates display type of this data point label text object. */
    void                UpdateDataLabel( bool bCateg, bool bValue, bool bPercent );

    /** Returns the target object this text is linked to. */
    inline sal_uInt16   GetLinkTarget() const { return maObjLink.mnTarget; }
    /** Returns the position of the data point label this text is linked to. */
    inline const XclChDataPointPos& GetPointPos() const { return maObjLink.maPointPos; }
    /** Returns true, if this text group contains string data. */
    inline bool         HasString() const { return mxSrcLink && mxSrcLink->HasString(); }
    /** Returns true, if the text object is marked as deleted. */
    inline bool         IsDeleted() const { return ::get_flag( maData.mnFlags, EXC_CHTEXT_DELETED ); }

    /** Converts and writes the contained font settings to the passed property set. */
    void                ConvertFont( ScfPropertySet& rPropSet ) const;
    /** Converts and writes the contained rotation settings to the passed property set. */
    void                ConvertRotation( ScfPropertySet& rPropSet, bool bSupportsStacked ) const;
    /** Converts and writes the contained frame data to the passed property set. */
    void                ConvertFrame( ScfPropertySet& rPropSet ) const;
    /** Converts and writes the contained number format to the passed property set. */
    void                ConvertNumFmt( ScfPropertySet& rPropSet, bool bPercent ) const;
    /** Converts and writes all contained data to the passed data point label property set. */
    void                ConvertDataLabel( ScfPropertySet& rPropSet, const XclChTypeInfo& rTypeInfo ) const;
    /** Creates a title text object. */
    XTitleRef           CreateTitle() const;
    /** Converts the manual position of the specified title */
    void                ConvertTitlePosition( const XclChTextKey& rTitleKey ) const;

private:
    using               XclImpChRoot::ConvertFont;

    /** Reads a CHFRLABELPROPS record. */
    void                ReadChFrLabelProps( XclImpStream& rStrm );

private:
    typedef boost::shared_ptr< XclChFrLabelProps > XclChFrLabelPropsRef;

    XclChText           maData;             /// Contents of the CHTEXT record.
    XclChObjectLink     maObjLink;          /// Link target for this text object.
    XclFormatRunVec     maFormats;          /// Formatting runs (CHFORMATRUNS record).
    XclImpChFramePosRef mxFramePos;         /// Relative text frame position (CHFRAMEPOS record).
    XclImpChSourceLinkRef mxSrcLink;        /// Linked data (CHSOURCELINK with CHSTRING record).
    XclImpChFrameRef    mxFrame;            /// Text object frame properties (CHFRAME group).
    XclImpChFontRef     mxFont;             /// Index into font buffer (CHFONT record).
    XclChFrLabelPropsRef mxLabelProps;      /// Extended data label properties (CHFRLABELPROPS record).
};

typedef boost::shared_ptr< XclImpChText > XclImpChTextRef;

// Data series ================================================================

/** The CHMARKERFORMAT record containing data point marker formatting data. */
class XclImpChMarkerFormat
{
public:
    /** Reads the CHMARKERFORMAT record (data point marker properties). */
    void                ReadChMarkerFormat( XclImpStream& rStrm );

    /** Returns true, if the marker format is set to automatic. */
    inline bool         IsAuto() const { return ::get_flag( maData.mnFlags, EXC_CHMARKERFORMAT_AUTO ); }

    /** Converts and writes the contained data to the passed property set. */
    void                Convert( const XclImpChRoot& rRoot, ScfPropertySet& rPropSet,
                            sal_uInt16 nFormatIdx, sal_Int16 nLineWeight ) const;
    /** Sets the marker fill color as main color to the passed property set. */
    void                ConvertColor( const XclImpChRoot& rRoot,
                            ScfPropertySet& rPropSet, sal_uInt16 nFormatIdx ) const;

private:
    XclChMarkerFormat   maData;             /// Contents of the CHMARKERFORMAT record.
};

typedef boost::shared_ptr< XclImpChMarkerFormat > XclImpChMarkerFormatRef;

// ----------------------------------------------------------------------------

/** The CHPIEFORMAT record containing data point formatting data for pie segments. */
class XclImpChPieFormat
{
public:
    explicit            XclImpChPieFormat();
    /** Reads the CHPIEFORMAT record (pie segment properties). */
    void                ReadChPieFormat( XclImpStream& rStrm );
    /** Converts and writes the contained data to the passed property set. */
    void                Convert( ScfPropertySet& rPropSet ) const;

private:
    sal_uInt16          mnPieDist;          /// Pie distance to diagram center.
};

typedef boost::shared_ptr< XclImpChPieFormat > XclImpChPieFormatRef;

// ----------------------------------------------------------------------------

/** The CHSERIESFORMAT record containing additional settings for a data series. */
class XclImpChSeriesFormat
{
public:
    explicit            XclImpChSeriesFormat();
    /** Reads the CHSERIESFORMAT record (additional settings for a series). */
    void                ReadChSeriesFormat( XclImpStream& rStrm );
    /** Returns true, if the series line is smoothed. */
    inline bool         HasSpline() const { return ::get_flag( mnFlags, EXC_CHSERIESFORMAT_SMOOTHED ); }

private:
    sal_uInt16          mnFlags;            /// Additional flags.
};

typedef boost::shared_ptr< XclImpChSeriesFormat > XclImpChSeriesFormatRef;

// ----------------------------------------------------------------------------

/** The CH3DDATAFORMAT record containing the bar type in 3D bar charts. */
class XclImpCh3dDataFormat
{
public:
    /** Reads the CH3DDATAFORMAT record (3D bar properties). */
    void                ReadCh3dDataFormat( XclImpStream& rStrm );
    /** Converts and writes the contained data to the passed property set. */
    void                Convert( ScfPropertySet& rPropSet ) const;

private:
    XclCh3dDataFormat   maData;             /// Contents of the CH3DDATAFORMAT record.
};

typedef boost::shared_ptr< XclImpCh3dDataFormat > XclImpCh3dDataFormatRef;

// ----------------------------------------------------------------------------

/** The CHATTACHEDLABEL record that contains the type of a data point label. */
class XclImpChAttachedLabel : protected XclImpChRoot
{
public:
    explicit            XclImpChAttachedLabel( const XclImpChRoot& rRoot );
    /** Reads the CHATTACHEDLABEL record (data series/point labels). */
    void                ReadChAttachedLabel( XclImpStream& rStrm );
    /** Creates a CHTEXT group for the label. Clones xParentText and sets additional label settings */
    XclImpChTextRef     CreateDataLabel( const XclImpChText* pParent ) const;

private:
    sal_uInt16          mnFlags;            /// Additional flags.
};

typedef boost::shared_ptr< XclImpChAttachedLabel > XclImpChAttLabelRef;

// ----------------------------------------------------------------------------

/** Represents the CHDATAFORMAT record group containing data point properties.

    The CHDATAFORMAT group consists of: CHDATAFORMAT, CHBEGIN, CHFRAME group,
    CHMARKERFORMAT, CHPIEFORMAT, CH3DDATAFORMAT, CHSERIESFORMAT,
    CHATTACHEDLABEL, CHEND.
 */
class XclImpChDataFormat : public XclImpChFrameBase, protected XclImpChRoot
{
public:
    explicit            XclImpChDataFormat( const XclImpChRoot& rRoot );

    /** Reads the CHDATAFORMAT record (called by base class). */
    virtual void        ReadHeaderRecord( XclImpStream& rStrm );
    /** Reads a record from the CHDATAFORMAT group (called by base class). */
    virtual void        ReadSubRecord( XclImpStream& rStrm );

    /** Sets this object to the specified data point position. */
    void                SetPointPos( const XclChDataPointPos& rPointPos, sal_uInt16 nFormatIdx );
    /** Sets type and text formatting for a data point label (CHTEXT group). */
    inline void         SetDataLabel( XclImpChTextRef xLabel ) { mxLabel = xLabel; }

    /** Updates default data format for series group. */
    void                UpdateGroupFormat( const XclChExtTypeInfo& rTypeInfo );
    /** Updates missing series settings from the passed chart type group data format. */
    void                UpdateSeriesFormat( const XclChExtTypeInfo& rTypeInfo, const XclImpChDataFormat* pGroupFmt );
    /** Updates missing data point settings from the passed series format. */
    void                UpdatePointFormat( const XclChExtTypeInfo& rTypeInfo, const XclImpChDataFormat* pSeriesFmt );
    /** Updates default data format for trend lines. */
    void                UpdateTrendLineFormat();

    /** Returns the position of the data point described by this group. */
    inline const XclChDataPointPos& GetPointPos() const { return maData.maPointPos; }
    /** Returns the format index of the data point described by this group. */
    inline sal_uInt16   GetFormatIdx() const { return maData.mnFormatIdx; }
    /** Returns true, if markers are set to automatic format. */
    inline bool         IsAutoMarker() const { return !mxMarkerFmt || mxMarkerFmt->IsAuto(); }
    /** Returns true, if the series line is smoothed. */
    inline bool         HasSpline() const { return mxSeriesFmt && mxSeriesFmt->HasSpline(); }
    /** Returns the data label text object. */
    inline const XclImpChText* GetDataLabel() const { return mxLabel.get(); }

    /** Converts and writes the contained data to the passed property set. */
    void                Convert( ScfPropertySet& rPropSet, const XclChExtTypeInfo& rTypeInfo ) const;
    /** Writes the line format only, e.g. for trend lines or error bars. */
    void                ConvertLine( ScfPropertySet& rPropSet, XclChObjectType eObjType ) const;
    /** Writes the area format only for the series or a data point. */
    void                ConvertArea( ScfPropertySet& rPropSet, sal_uInt16 nFormatIdx, bool bUsePicFmt ) const;

private:
    /** Removes unused formatting (e.g. pie distance in a bar chart). */
    void                RemoveUnusedFormats( const XclChExtTypeInfo& rTypeInfo );
    /** Updates or creates the data point label. */
    void                UpdateDataLabel( const XclImpChDataFormat* pParentFmt );

private:
    XclChDataFormat     maData;             /// Contents of the CHDATAFORMAT record.
    XclImpChMarkerFormatRef mxMarkerFmt;    /// Data point marker (CHMARKERFORMAT record).
    XclImpChPieFormatRef mxPieFmt;          /// Pie segment format (CHPIEFORMAT record).
    XclImpChSeriesFormatRef mxSeriesFmt;    /// Series properties (CHSERIESFORMAT record).
    XclImpCh3dDataFormatRef mx3dDataFmt;    /// 3D bar format (CH3DDATAFORMAT record).
    XclImpChAttLabelRef mxAttLabel;         /// Data point label type (CHATTACHEDLABEL record).
    XclImpChTextRef     mxLabel;            /// Data point label formatting (CHTEXT group).
};

typedef boost::shared_ptr< XclImpChDataFormat > XclImpChDataFormatRef;

// ----------------------------------------------------------------------------

/** Represents the CHSERTRENDLINE record containing settings for a trend line. */
class XclImpChSerTrendLine : protected XclImpChRoot
{
public:
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XRegressionCurve > XRegressionCurveRef;

public:
    explicit            XclImpChSerTrendLine( const XclImpChRoot& rRoot );

    /** Reads the CHSERTRENDLINE record. */
    void                ReadChSerTrendLine( XclImpStream& rStrm );
    /** Sets formatting information for the trend line. */
    inline void         SetDataFormat( XclImpChDataFormatRef xDataFmt ) { mxDataFmt = xDataFmt; }

    /** Creates an API object representing this trend line. */
    XRegressionCurveRef CreateRegressionCurve() const;

private:
    XclChSerTrendLine   maData;             /// Contents of the CHSERTRENDLINE record.
    XclImpChDataFormatRef mxDataFmt;        /// Formatting settings of the trend line.
};

typedef boost::shared_ptr< XclImpChSerTrendLine > XclImpChSerTrendLineRef;

// ----------------------------------------------------------------------------

/** Represents the CHSERERRORBAR record containing settings for error bars. */
class XclImpChSerErrorBar : protected XclImpChRoot
{
public:
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XLabeledDataSequence >    XLabeledDataSeqRef;
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >                   XPropertySetRef;

public:
    explicit            XclImpChSerErrorBar( const XclImpChRoot& rRoot );

    /** Reads the CHSERERRORBAR record. */
    void                ReadChSerErrorBar( XclImpStream& rStrm );
    /** Sets link and formatting information for the error bars. */
    void                SetSeriesData(
                            XclImpChSourceLinkRef xValueLink,
                            XclImpChDataFormatRef xDataFmt );

    /** Returns the type of this error bar (X/Y, plus/minus). */
    inline sal_uInt8    GetBarType() const { return maData.mnBarType; }
    /** Creates a labeled data sequence object from value data link. */
    XLabeledDataSeqRef  CreateValueSequence() const;

    /** Tries to create an error bar API object from the specified Excel error bars. */
    static XPropertySetRef CreateErrorBar(
                            const XclImpChSerErrorBar* pPosBar,
                            const XclImpChSerErrorBar* pNegBar );

private:
    XclChSerErrorBar    maData;             /// Contents of the CHSERERRORBAR record.
    XclImpChSourceLinkRef mxValueLink;      /// Link data for manual error bar values.
    XclImpChDataFormatRef mxDataFmt;        /// Formatting settings of the error bars.
};

typedef boost::shared_ptr< XclImpChSerErrorBar > XclImpChSerErrorBarRef;

// ----------------------------------------------------------------------------

/** Represents the CHSERIES record group describing a data series in a chart.

    The CHSERIES group consists of: CHSERIES, CHBEGIN, CHSOURCELINK groups,
    CHDATAFORMAT groups, CHSERGROUP, CHSERPARENT, CHSERERRORBAR,
    CHSERTRENDLINE, CHEND.
 */
class XclImpChSeries : public XclImpChGroupBase, protected XclImpChRoot
{
public:
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries >                   XDataSeriesRef;
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XLabeledDataSequence >    XLabeledDataSeqRef;
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >                   XPropertySetRef;

public:
    explicit            XclImpChSeries( const XclImpChRoot& rRoot, sal_uInt16 nSeriesIdx );

    /** Reads the CHSERIES record (called by base class). */
    virtual void        ReadHeaderRecord( XclImpStream& rStrm );
    /** Reads a record from the CHSERIES group (called by base class). */
    virtual void        ReadSubRecord( XclImpStream& rStrm );

    /** Sets a data point or series format (CHDATAFORMAT group) for this series. */
    void                SetDataFormat( const XclImpChDataFormatRef& xDataFmt );
    /** Sets a label text (CHTEXT group) attached  to a series or data point. */
    void                SetDataLabel( const XclImpChTextRef& xLabel );
    /** Adds error bar settings from the passed series to the own series. */
    void                AddChildSeries( const XclImpChSeries& rSeries );
    /** Updates missing series formatting by using default formatting from axes sets. */
    void                FinalizeDataFormats();

    /** Returns the axes set identifier this series is assigned to (primary/secondary). */
    inline sal_uInt16   GetGroupIdx() const { return mnGroupIdx; }
    /** Returns the 0-based series index described by this series. */
    inline sal_uInt16   GetSeriesIdx() const { return mnSeriesIdx; }
    /** Returns the 0-based index of the parent series (e.g. of a trend line). */
    inline sal_uInt16   GetParentIdx() const { return mnParentIdx; }
    /** Returns the format index of the series used for automatic line and area colors. */
    inline sal_uInt16   GetFormatIdx() const { return mxSeriesFmt ? mxSeriesFmt->GetFormatIdx() : EXC_CHDATAFORMAT_DEFAULT; }
    /** Returns true, if the series is child of another series (e.g. trend line). */
    inline bool         HasParentSeries() const { return mnParentIdx != EXC_CHSERIES_INVALID; }
    /** Returns true, if the series contains child series (e.g. trend lines). */
    inline bool         HasChildSeries() const { return !maTrendLines.empty() || !maErrorBars.empty(); }
    /** Returns series title or an empty string, if the series does not contain a title. */
    OUString            GetTitle() const { return mxTitleLink ? mxTitleLink->GetString() : OUString(); }

    /** Returns true, if the series line is smoothed. */
    inline bool         HasSpline() const { return mxSeriesFmt && mxSeriesFmt->HasSpline(); }

    /** Creates a labeled data sequence object from value data link. */
    XLabeledDataSeqRef  CreateValueSequence( const OUString& rValueRole ) const;
    /** Creates a labeled data sequence object from category data link. */
    XLabeledDataSeqRef  CreateCategSequence( const OUString& rCategRole ) const;
    /** Creates a data series object with initialized source links. */
    XDataSeriesRef      CreateDataSeries() const;

    void                FillAllSourceLinks(::std::vector<ScTokenRef>& rTokens) const;

private:
    /** Reads a CHSOURCELINK record. */
    void                ReadChSourceLink( XclImpStream& rStrm );
    /** Reads a CHDATAFORMAT group containing series and point formatting. */
    void                ReadChDataFormat( XclImpStream& rStrm );
    /** Reads a CHSERPARENT record specifying the parent series of this series. */
    void                ReadChSerParent( XclImpStream& rStrm );
    /** Reads a CHSERTRENDLINE record containing trend line settings. */
    void                ReadChSerTrendLine( XclImpStream& rStrm );
    /** Reads a CHSERERRORBAR record containing error bar settings. */
    void                ReadChSerErrorBar( XclImpStream& rStrm );

    /** Creates a new CHDATAFORMAT group with the specified point index. */
    XclImpChDataFormatRef CreateDataFormat( sal_uInt16 nPointIdx, sal_uInt16 nFormatIdx );

    /** Converts all trend lines and inserts them into the passed API data series object. */
    void                ConvertTrendLines( XDataSeriesRef xDataSeries ) const;
    /** Tries to create an error bar API object from the specified Excel error bars. */
    XPropertySetRef     CreateErrorBar( sal_uInt8 nPosBarId, sal_uInt8 nNegBarId ) const;

private:
    typedef ::std::map<sal_uInt16, XclImpChDataFormatRef> XclImpChDataFormatMap;
    typedef ::std::map<sal_uInt16, XclImpChTextRef>       XclImpChTextMap;
    typedef ::std::list< XclImpChSerTrendLineRef >        XclImpChSerTrendLineList;
    typedef ::boost::ptr_map<sal_uInt8, XclImpChSerErrorBar> XclImpChSerErrorBarMap;

    XclChSeries         maData;             /// Contents of the CHSERIES record.
    XclImpChSourceLinkRef mxValueLink;      /// Link data for series values.
    XclImpChSourceLinkRef mxCategLink;      /// Link data for series category names.
    XclImpChSourceLinkRef mxTitleLink;      /// Link data for series title.
    XclImpChSourceLinkRef mxBubbleLink;     /// Link data for series bubble sizes.
    XclImpChDataFormatRef mxSeriesFmt;      /// CHDATAFORMAT group for series format.
    XclImpChDataFormatMap maPointFmts;      /// CHDATAFORMAT groups for data point formats.
    XclImpChTextMap     maLabels;           /// Data point labels (CHTEXT groups).
    XclImpChSerTrendLineList maTrendLines;  /// Trend line settings (CHSERTRENDLINE records).
    XclImpChSerErrorBarMap maErrorBars;     /// Error bar settings (CHSERERRORBAR records).
    sal_uInt16          mnGroupIdx;         /// Chart type group (CHTYPEGROUP group) this series is assigned to.
    sal_uInt16          mnSeriesIdx;        /// 0-based series index.
    sal_uInt16          mnParentIdx;        /// 0-based index of parent series (trend lines and error bars).
};

typedef boost::shared_ptr< XclImpChSeries > XclImpChSeriesRef;

// Chart type groups ==========================================================

class XclImpChType : protected XclImpChRoot
{
public:
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram >          XDiagramRef;
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XCoordinateSystem > XCoordSystemRef;
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >        XChartTypeRef;

public:
    explicit            XclImpChType( const XclImpChRoot& rRoot );

    /** Reads a chart type record (e.g. CHBAR, CHLINE, CHPIE, ...). */
    void                ReadChType( XclImpStream& rStrm );
    /** Final processing after reading the entire chart. */
    void                Finalize( bool bStockChart );

    /** Returns the record identifier of the chart type record. */
    inline sal_uInt16   GetRecId() const { return mnRecId; }
    /** Returns the chart type info struct for the contained chart type. */
    inline const XclChTypeInfo& GetTypeInfo() const { return maTypeInfo; }
    /** Returns true, if the series in this chart type group are stacked on each other (no percentage). */
    bool                IsStacked() const;
    /** Returns true, if the series in this chart type group are stacked on each other as percentage. */
    bool                IsPercent() const;
    /** Returns true, if chart type has category labels enabled (may be disabled in radar charts). */
    bool                HasCategoryLabels() const;

    /** Creates a coordinate system according to the contained chart type. */
    XCoordSystemRef     CreateCoordSystem( bool b3dChart ) const;
    /** Creates and returns an object that represents the contained chart type. */
    XChartTypeRef       CreateChartType( XDiagramRef xDiagram, bool b3dChart ) const;

private:
    XclChType           maData;             /// Contents of the chart type record.
    sal_uInt16          mnRecId;            /// Record identifier for chart type.
    XclChTypeInfo       maTypeInfo;         /// Chart type info for the contained type.
};

// ----------------------------------------------------------------------------

/** Represents the CHCHART3D record that contains 3D view settings. */
class XclImpChChart3d
{
public:
    /** Reads the CHCHART3D record (properties for 3D charts). */
    void                ReadChChart3d( XclImpStream& rStrm );
    /** Returns true, if the data points are clustered on the X axis. */
    inline bool         IsClustered() const { return ::get_flag( maData.mnFlags, EXC_CHCHART3D_CLUSTER ); }
    /** Converts and writes the contained data to the passed property set. */
    void                Convert( ScfPropertySet& rPropSet, bool b3dWallChart ) const;

private:
    XclChChart3d        maData;             /// Contents of the CHCHART3D record.
};

typedef boost::shared_ptr< XclImpChChart3d > XclImpChChart3dRef;

// ----------------------------------------------------------------------------

/** Represents the CHLEGEND record group describing the chart legend.

    The CHLEGEND group consists of: CHLEGEND, CHBEGIN, CHFRAMEPOS, CHFRAME
    group, CHTEXT group, CHEND.
 */
class XclImpChLegend : public XclImpChGroupBase, protected XclImpChRoot
{
public:
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XLegend > XLegendRef;

public:
    explicit            XclImpChLegend( const XclImpChRoot& rRoot );

    /** Reads the CHLEGEND record (called by base class). */
    virtual void        ReadHeaderRecord( XclImpStream& rStrm );
    /** Reads a record from the CHLEGEND group (called by base class). */
    virtual void        ReadSubRecord( XclImpStream& rStrm );
    /** Final processing after reading the entire chart. */
    void                Finalize();

    /** Creates a new legend object. */
    XLegendRef          CreateLegend() const;

private:
    XclChLegend         maData;             /// Contents of the CHLEGEND record.
    XclImpChFramePosRef mxFramePos;         /// Legend frame position (CHFRAMEPOS record).
    XclImpChTextRef     mxText;             /// Legend text format (CHTEXT group).
    XclImpChFrameRef    mxFrame;            /// Legend frame format (CHFRAME group).
};

typedef boost::shared_ptr< XclImpChLegend > XclImpChLegendRef;

// ----------------------------------------------------------------------------

/** Represents the CHDROPBAR record group describing pos/neg bars in line charts.

    The CHDROPBAR group consists of: CHDROPBAR, CHBEGIN, CHLINEFORMAT,
    CHAREAFORMAT, CHESCHERFORMAT group, CHEND.
 */
class XclImpChDropBar : public XclImpChFrameBase
{
public:
    explicit            XclImpChDropBar( sal_uInt16 nDropBar );

    /** Reads the CHDROPBAR record (called by base class). */
    virtual void        ReadHeaderRecord( XclImpStream& rStrm );

    /** Converts and writes the contained frame data to the passed property set. */
    void                Convert( const XclImpChRoot& rRoot, ScfPropertySet& rPropSet ) const;

private:
    sal_uInt16          mnDropBar;          /// Drop bar identifier, needed for auto format.
    sal_uInt16          mnBarDist;          /// Distance between bars (CHDROPBAR record).
};

typedef boost::shared_ptr< XclImpChDropBar > XclImpChDropBarRef;

// ----------------------------------------------------------------------------

/** Represents the CHTYPEGROUP record group describing a group of series.

    The CHTYPEGROUP group consists of: CHTYPEGROUP, CHBEGIN, a chart type
    record (e.g. CHBAR, CHLINE, CHAREA, CHPIE, ...), CHCHART3D, CHLEGEND group,
    CHDEFAULTTEXT groups (CHDEFAULTTEXT with CHTEXT groups), CHDROPBAR groups,
    CHCHARTLINE groups (CHCHARTLINE with CHLINEFORMAT), CHDATAFORMAT group,
    CHEND.
 */
class XclImpChTypeGroup : public XclImpChGroupBase, protected XclImpChRoot
{
public:
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram >                      XDiagramRef;
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XCoordinateSystem >             XCoordSystemRef;
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >                    XChartTypeRef;
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries >                   XDataSeriesRef;
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XLabeledDataSequence >    XLabeledDataSeqRef;

public:
    explicit            XclImpChTypeGroup( const XclImpChRoot& rRoot );

    /** Reads the CHTYPEGROUP record (called by base class). */
    virtual void        ReadHeaderRecord( XclImpStream& rStrm );
    /** Reads a record from the CHTYPEGROUP group (called by base class). */
    virtual void        ReadSubRecord( XclImpStream& rStrm );
    /** Final processing after reading the entire chart. */
    void                Finalize();

    /** Inserts a series attached to this chart type group.*/
    void                AddSeries( XclImpChSeriesRef xSeries );
    /** Marks the passed format index as used. PopUnusedFormatIndex() will not return this index. */
    void                SetUsedFormatIndex( sal_uInt16 nFormatIdx );
    /** Returns the next unused format index and marks it as used. */
    sal_uInt16          PopUnusedFormatIndex();

    /** Returns the index of this chart type group. */
    inline sal_uInt16   GetGroupIdx() const { return maData.mnGroupIdx; }
    /** Returns the chart type info struct for the contained chart type. */
    inline const XclChExtTypeInfo& GetTypeInfo() const { return maTypeInfo; }
    /** Returns true, if this chart type group contains at least one valid series. */
    inline bool         IsValidGroup() const { return !maSeries.empty(); }
    /** Returns true, if the series in this chart type group are stacked on each other (no percentage). */
    inline bool         IsStacked() const { return maType.IsStacked(); }
    /** Returns true, if the series in this chart type group are stacked on each other as percentage. */
    inline bool         IsPercent() const { return maType.IsPercent(); }
    /** Returns true, if the chart is three-dimensional. */
    inline bool         Is3dChart() const { return mxChart3d && maTypeInfo.mbSupports3d; }
    /** Returns true, if chart type supports wall and floor format in 3d mode. */
    inline bool         Is3dWallChart() const { return Is3dChart() && (maTypeInfo.meTypeCateg != EXC_CHTYPECATEG_PIE); }
    /** Returns true, if the series in this chart type group are ordered on the Z axis. */
    inline bool         Is3dDeepChart() const { return Is3dWallChart() && mxChart3d && !mxChart3d->IsClustered(); }
    /** Returns true, if category (X axis) labels are enabled (may be disabled in radar charts). */
    inline bool         HasCategoryLabels() const { return maType.HasCategoryLabels(); }
    /** Returns true, if points of a series show varying automatic area format. */
    bool                HasVarPointFormat() const;
    /** Returns true, if bars are connected with lines (stacked bar charts only). */
    bool                HasConnectorLines() const;

    /** Returns the legend object. */
    inline XclImpChLegendRef GetLegend() const { return mxLegend; }
    /** Returns the default series data format. */
    inline XclImpChDataFormatRef GetGroupFormat() const { return mxGroupFmt; }
    /** Returns series title, if the chart type group contains only one single series. */
    OUString            GetSingleSeriesTitle() const;

    /** Converts and writes all 3D settings to the passed diagram. */
    void                ConvertChart3d( ScfPropertySet& rPropSet ) const;
    /** Creates a coordinate system according to the contained chart type. */
    XCoordSystemRef     CreateCoordSystem() const;
    /** Creates and returns an object that represents the contained chart type. */
    XChartTypeRef       CreateChartType( XDiagramRef xDiagram, sal_Int32 nApiAxesSetIdx ) const;
    /** Creates a labeled data sequence object for axis categories. */
    XLabeledDataSeqRef  CreateCategSequence() const;

private:
    /** Reads a CHDROPBAR record group. */
    void                ReadChDropBar( XclImpStream& rStrm );
    /** Reads a CHCHARTLINE record group. */
    void                ReadChChartLine( XclImpStream& rStrm );
    /** Reads a CHDATAFORMAT record group (default series format). */
    void                ReadChDataFormat( XclImpStream& rStrm );

    /** Returns true, if the chart type group contains a hi-lo line format. */
    inline bool         HasHiLoLine() const { return maChartLines.find( EXC_CHCHARTLINE_HILO ) != maChartLines.end(); }
    /** Returns true, if the chart type group contains drop bar formats. */
    inline bool         HasDropBars() const { return !maDropBars.empty(); }

    /** Inserts the passed series into the chart type. Adds additional properties to the series. */
    void                InsertDataSeries( XChartTypeRef xChartType,
                            XDataSeriesRef xSeries, sal_Int32 nApiAxesSetIdx ) const;
    /** Creates all data series of any chart type except stock charts. */
    void                CreateDataSeries( XChartTypeRef xChartType, sal_Int32 nApiAxesSetIdx ) const;
    /** Creates all data series of of a stock chart. */
    void                CreateStockSeries( XChartTypeRef xChartType, sal_Int32 nApiAxesSetIdx ) const;

private:
    typedef ::std::vector< XclImpChSeriesRef >               XclImpChSeriesVec;
    typedef boost::ptr_map<sal_uInt16, XclImpChDropBar>      XclImpChDropBarMap;
    typedef boost::ptr_map<sal_uInt16, XclImpChLineFormat>   XclImpChLineFormatMap;
    typedef ::std::set< sal_uInt16 >                         UInt16Set;

    XclChTypeGroup      maData;             /// Contents of the CHTYPEGROUP record.
    XclImpChType        maType;             /// Chart type (e.g. CHBAR, CHLINE, ...).
    XclChExtTypeInfo    maTypeInfo;         /// Extended chart type info.
    XclImpChSeriesVec   maSeries;           /// Series attached to this chart type group (CHSERIES groups).
    XclImpChSeriesRef   mxFirstSeries;      /// First series in this chart type group (CHSERIES groups).
    XclImpChChart3dRef  mxChart3d;          /// 3D settings (CHCHART3D record).
    XclImpChLegendRef   mxLegend;           /// Chart legend (CHLEGEND group).
    XclImpChDropBarMap  maDropBars;         /// Dropbars (CHDROPBAR group).
    XclImpChLineFormatMap maChartLines;     /// Global line formats (CHCHARTLINE group).
    XclImpChDataFormatRef mxGroupFmt;       /// Default format for all series (CHDATAFORMAT group).
    UInt16Set           maUnusedFormats;    /// Contains unused format indexes for automatic colors.
};

typedef boost::shared_ptr< XclImpChTypeGroup > XclImpChTypeGroupRef;

// Axes =======================================================================

class XclImpChLabelRange : protected XclImpChRoot
{
public:
    typedef ::com::sun::star::chart2::ScaleData ScaleData;

public:
    explicit            XclImpChLabelRange( const XclImpChRoot& rRoot );
    /** Reads the CHLABELRANGE record (category axis scaling properties). */
    void                ReadChLabelRange( XclImpStream& rStrm );
    /** Reads the CHDATERANGE record (date axis scaling properties). */
    void                ReadChDateRange( XclImpStream& rStrm );
    /** Converts category axis scaling settings. */
    void                Convert( ScfPropertySet& rPropSet, ScaleData& rScaleData, bool bMirrorOrient ) const;
    /** Converts position settings of this axis at a crossing axis. */
    void                ConvertAxisPosition( ScfPropertySet& rPropSet, bool b3dChart ) const;

private:
    XclChLabelRange     maLabelData;        /// Contents of the CHLABELRANGE record.
    XclChDateRange      maDateData;         /// Contents of the CHDATERANGE record.
};

typedef boost::shared_ptr< XclImpChLabelRange > XclImpChLabelRangeRef;

// ----------------------------------------------------------------------------

class XclImpChValueRange : protected XclImpChRoot
{
public:
    typedef ::com::sun::star::chart2::ScaleData ScaleData;

public:
    explicit            XclImpChValueRange( const XclImpChRoot& rRoot );
    /** Reads the CHVALUERANGE record (numeric axis scaling properties). */
    void                ReadChValueRange( XclImpStream& rStrm );
    /** Converts value axis scaling settings. */
    void                Convert( ScaleData& rScaleData, bool bMirrorOrient ) const;
    /** Converts position settings of this axis at a crossing axis. */
    void                ConvertAxisPosition( ScfPropertySet& rPropSet ) const;

private:
    XclChValueRange     maData;             /// Contents of the CHVALUERANGE record.
};

typedef boost::shared_ptr< XclImpChValueRange > XclImpChValueRangeRef;

// ----------------------------------------------------------------------------

class XclImpChTick : protected XclImpChRoot
{
public:
    explicit            XclImpChTick( const XclImpChRoot& rRoot );
    /** Reads the CHTICK record (axis ticks properties). */
    void                ReadChTick( XclImpStream& rStrm );

    /** Returns true, if the axis shows attached labels. */
    inline bool         HasLabels() const { return maData.mnLabelPos != EXC_CHTICK_NOLABEL; }
    /** Returns the leading font color for the axis labels. */
    Color               GetFontColor() const;
    /** Returns the rotation value for the axis labels. */
    sal_uInt16          GetRotation() const;

    /** Converts and writes the contained data to the passed property set. */
    void                Convert( ScfPropertySet& rPropSet ) const;

private:
    XclChTick           maData;             /// Contents of the CHTICK record.
};

typedef boost::shared_ptr< XclImpChTick > XclImpChTickRef;

// ----------------------------------------------------------------------------

/** Represents the CHAXIS record group describing an entire chart axis.

    The CHAXIS group consists of: CHAXIS, CHBEGIN, CHLABELRANGE, CHEXTRANGE,
    CHVALUERANGE, CHFORMAT, CHTICK, CHFONT, CHAXISLINE groups (CHAXISLINE with
    CHLINEFORMAT, CHAREAFORMAT, and CHESCHERFORMAT group), CHEND.
 */
class XclImpChAxis : public XclImpChGroupBase, public XclImpChFontBase, protected XclImpChRoot
{
public:
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis > XAxisRef;

public:
    explicit            XclImpChAxis( const XclImpChRoot& rRoot, sal_uInt16 nAxisType = EXC_CHAXIS_NONE );

    /** Reads the CHAXIS record (called by base class). */
    virtual void        ReadHeaderRecord( XclImpStream& rStrm );
    /** Reads a record from the CHAXIS group (called by base class). */
    virtual void        ReadSubRecord( XclImpStream& rStrm );
    /** Final processing after reading the entire chart. */
    void                Finalize();

    /** Returns the font index for the axis labels. */
    virtual sal_uInt16  GetFontIndex() const;
    /** Returns the font color for the axis labels. */
    virtual Color       GetFontColor() const;
    /** Returns the rotation value for axis labels. */
    virtual sal_uInt16  GetRotation() const;

    /** Returns the type of this axis. */
    inline sal_uInt16   GetAxisType() const { return maData.mnType; }
    /** Returns the axis dimension index used by the chart API. */
    inline sal_Int32    GetApiAxisDimension() const { return maData.GetApiAxisDimension(); }
    /** Returns true, if the axis is active. */
    inline bool         IsActivated() const { return !mxAxisLine || mxAxisLine->IsShowAxis(); }
    /** Returns true, if the axis contains caption labels. */
    inline bool         HasLabels() const { return !mxTick || mxTick->HasLabels(); }
    /** Returns true, if the axis shows its major grid lines. */
    inline bool         HasMajorGrid() const { return static_cast< bool >(mxMajorGrid); }
    /** Returns true, if the axis shows its minor grid lines. */
    inline bool         HasMinorGrid() const { return static_cast< bool >(mxMinorGrid); }

    /** Creates an API axis object. */
    XAxisRef            CreateAxis( const XclImpChTypeGroup& rTypeGroup, const XclImpChAxis* pCrossingAxis ) const;
    /** Converts and writes 3D wall/floor properties to the passed property set. */
    void                ConvertWall( ScfPropertySet& rPropSet ) const;
    /** Converts position settings of this axis at a crossing axis. */
    void                ConvertAxisPosition( ScfPropertySet& rPropSet, const XclImpChTypeGroup& rTypeGroup ) const;

private:
    /** Reads a CHAXISLINE record specifying the target for following line properties. */
    void                ReadChAxisLine( XclImpStream& rStrm );
    /** Creates a CHFRAME object and stores it into the mxWallFrame member. */
    void                CreateWallFrame();

private:
    XclChAxis           maData;             /// Contents of the CHAXIS record.
    XclImpChLabelRangeRef mxLabelRange;     /// Category scaling (CHLABELRANGE record).
    XclImpChValueRangeRef mxValueRange;     /// Value scaling (CHVALUERANGE record).
    XclImpChTickRef     mxTick;             /// Axis ticks (CHTICK record).
    XclImpChFontRef     mxFont;             /// Index into font buffer (CHFONT record).
    XclImpChLineFormatRef mxAxisLine;       /// Axis line format (CHLINEFORMAT record).
    XclImpChLineFormatRef mxMajorGrid;      /// Major grid line format (CHLINEFORMAT record).
    XclImpChLineFormatRef mxMinorGrid;      /// Minor grid line format (CHLINEFORMAT record).
    XclImpChFrameRef    mxWallFrame;        /// Wall/floor format (sub records of CHFRAME group).
    sal_uInt16          mnNumFmtIdx;        /// Index into number format buffer (CHFORMAT record).
};

typedef boost::shared_ptr< XclImpChAxis > XclImpChAxisRef;

// ----------------------------------------------------------------------------

/** Represents the CHAXESSET record group describing an axes set (X/Y/Z axes).

    The CHAXESSET group consists of: CHAXESSET, CHBEGIN, CHFRAMEPOS, CHAXIS
    groups, CHTEXT groups, CHPLOTFRAME group (CHPLOTFRAME with CHFRAME group),
    CHTYPEGROUP group, CHEND.
 */
class XclImpChAxesSet : public XclImpChGroupBase, protected XclImpChRoot
{
public:
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram >          XDiagramRef;
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XCoordinateSystem > XCoordSystemRef;
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis >             XAxisRef;

public:
    explicit            XclImpChAxesSet( const XclImpChRoot& rRoot, sal_uInt16 nAxesSetId );

    /** Reads the CHAXESSET record (called by base class). */
    virtual void        ReadHeaderRecord( XclImpStream& rStrm );
    /** Reads a record from the CHAXESSET group (called by base class). */
    virtual void        ReadSubRecord( XclImpStream& rStrm );
    /** Final processing after reading the entire chart. */
    void                Finalize();

    /** Returns true, if this axes set exists (returns false if this is a dummy object). */
    inline bool         IsValidAxesSet() const { return !maTypeGroups.empty(); }
    /** Returns the index of the axes set (primary/secondary). */
    inline sal_uInt16   GetAxesSetId() const { return maData.mnAxesSetId; }
    /** Returns the axes set index used by the chart API. */
    inline sal_Int32    GetApiAxesSetIndex() const { return maData.GetApiAxesSetIndex(); }

    /** Returns the outer plot area position, if existing. */
    inline XclImpChFramePosRef GetPlotAreaFramePos() const { return mxFramePos; }
    /** Returns the specified chart type group. */
    XclImpChTypeGroupRef GetTypeGroup( sal_uInt16 nGroupIdx ) const;
    /** Returns the first chart type group. */
    XclImpChTypeGroupRef GetFirstTypeGroup() const;
    /** Looks for a legend in all chart type groups and returns it. */
    XclImpChLegendRef   GetLegend() const;
    /** Returns series title, if the axes set contains only one single series. */
    OUString            GetSingleSeriesTitle() const;

    /** Creates a coordinate system and converts all series and axis settings. */
    void                Convert( XDiagramRef xDiagram ) const;
    /** Converts the manual positions of all axis titles. */
    void                ConvertTitlePositions() const;

private:
    /** Reads a CHAXIS record group containing a single axis. */
    void                ReadChAxis( XclImpStream& rStrm );
    /** Reads a CHTEXT record group containing an axis title. */
    void                ReadChText( XclImpStream& rStrm );
    /** Reads the CHPLOTFRAME record group containing diagram area formatting. */
    void                ReadChPlotFrame( XclImpStream& rStrm );
    /** Reads a CHTYPEGROUP record group containing chart type and chart settings. */
    void                ReadChTypeGroup( XclImpStream& rStrm );

    /** Updates text formatting of the passed axis title with global text formatting. */
    void                UpdateAxisTitle( XclImpChTextRef xTitle );

    /** Creates a coordinate system that contains all chart types for this axes set. */
    XCoordSystemRef     CreateCoordSystem( XDiagramRef xDiagram ) const;
    /** Creates and inserts an axis into the container and registers the coordinate system. */
    void                ConvertAxis( XclImpChAxisRef xChAxis, XclImpChTextRef xChAxisTitle,
                            XCoordSystemRef xCoordSystem, const XclImpChAxis* pCrossingAxis ) const;
    /** Creates and returns an API axis object. */
    XAxisRef            CreateAxis( const XclImpChAxis& rChAxis, const XclImpChAxis* pCrossingAxis ) const;
    /** Writes all properties of the background area to the passed diagram. */
    void                ConvertBackground( XDiagramRef xDiagram ) const;

private:
    typedef ::std::map<sal_uInt16, XclImpChTypeGroupRef> XclImpChTypeGroupMap;

    XclChAxesSet        maData;             /// Contents of the CHAXESSET record.
    XclImpChFramePosRef mxFramePos;         /// Outer plot area position (CHFRAMEPOS record).
    XclImpChAxisRef     mxXAxis;            /// The X axis (CHAXIS group).
    XclImpChAxisRef     mxYAxis;            /// The Y axis (CHAXIS group).
    XclImpChAxisRef     mxZAxis;            /// The Z axis (CHAXIS group).
    XclImpChTextRef     mxXAxisTitle;       /// The X axis title (CHTEXT group).
    XclImpChTextRef     mxYAxisTitle;       /// The Y axis title (CHTEXT group).
    XclImpChTextRef     mxZAxisTitle;       /// The Z axis title (CHTEXT group).
    XclImpChFrameRef    mxPlotFrame;        /// Plot area (CHPLOTFRAME group).
    XclImpChTypeGroupMap maTypeGroups;      /// Chart type groups (CHTYPEGROUP group).
};

typedef boost::shared_ptr< XclImpChAxesSet > XclImpChAxesSetRef;

// The chart object ===========================================================

/** Represents the CHCHART record group describing the chart contents.

    The CHCHART group consists of: CHCHART, CHBEGIN, SCL, CHPLOTGROWTH, CHFRAME
    group, CHSERIES groups, CHPROPERTIES, CHDEFAULTTEXT groups (CHDEFAULTTEXT
    with CHTEXT groups), CHUSEDAXESSETS, CHAXESSET groups, CHTEXT groups, CHEND.
 */
class XclImpChChart : public XclImpChGroupBase, protected XclImpChRoot
{
public:
    explicit            XclImpChChart( const XclImpRoot& rRoot );
    virtual             ~XclImpChChart();

    /** Reads the CHCHART record (called by base class). */
    virtual void        ReadHeaderRecord( XclImpStream& rStrm );
    /** Reads a record from the CHCHART group (called by base class). */
    virtual void        ReadSubRecord( XclImpStream& rStrm );
    /** Reads a CHDEFAULTTEXT group (default text formats). */
    void                ReadChDefaultText( XclImpStream& rStrm );
    /** Reads a CHDATAFORMAT group describing a series format or a data point format. */
    void                ReadChDataFormat( XclImpStream& rStrm );

    /** Sets formatting from BIFF3-BIFF5 OBJ record, if own formatting is invisible. */
    void                UpdateObjFrame( const XclObjLineData& rLineData, const XclObjFillData& rFillData );

    /** Returns the specified chart type group. */
    XclImpChTypeGroupRef GetTypeGroup( sal_uInt16 nGroupIdx ) const;
    /** Returns the specified default text. */
    const XclImpChText*  GetDefaultText( XclChTextType eTextType ) const;
    /** Returns true, if the plot area has benn moved and/or resized manually. */
    bool                IsManualPlotArea() const;
    /** Returns the number of units on the progress bar needed for the chart. */
    inline sal_Size     GetProgressSize() const { return 2 * EXC_CHART_PROGRESS_SIZE; }

    /** Converts and writes all properties to the passed chart. */
    void                Convert(
        const ::com::sun::star::uno::Reference<
            com::sun::star::chart2::XChartDocument>& xChartDoc,
        XclImpDffConverter& rDffConv,
        const OUString& rObjName,
        const Rectangle& rChartRect ) const;

private:
    /** Reads a CHSERIES group (data series source and formatting). */
    void                ReadChSeries( XclImpStream& rStrm );
    /** Reads a CHPROPERTIES record (global chart properties). */
    void                ReadChProperties( XclImpStream& rStrm );
    /** Reads a CHAXESSET group (primary/secondary axes set). */
    void                ReadChAxesSet( XclImpStream& rStrm );
    /** Reads a CHTEXT group (chart title and series/point captions). */
    void                ReadChText( XclImpStream& rStrm );

    /** Final processing after reading the entire chart data. */
    void                Finalize();
    /** Finalizes series list, assigns child series to parent series. */
    void                FinalizeSeries();
    /** Assigns all imported CHDATAFORMAT groups to the respective series. */
    void                FinalizeDataFormats();
    /** Finalizes chart title, tries to detect title auto-generated from series name. */
    void                FinalizeTitle();

    /** Creates and returns a new diagram object and converts global chart settings. */
    com::sun::star::uno::Reference<com::sun::star::chart2::XDiagram>
        CreateDiagram() const;

private:
    typedef ::std::vector< XclImpChSeriesRef >                   XclImpChSeriesVec;
    typedef ::std::map<XclChDataPointPos, XclImpChDataFormatRef> XclImpChDataFormatMap;
    typedef ::boost::ptr_map<sal_uInt16, XclImpChText>           XclImpChTextMap;

    XclChRectangle      maRect;             /// Position of the chart on the sheet (CHCHART record).
    XclImpChSeriesVec   maSeries;           /// List of series data (CHSERIES groups).
    XclImpChDataFormatMap maDataFmts;       /// All series and point formats (CHDATAFORMAT groups).
    XclImpChFrameRef    mxFrame;            /// Chart frame format (CHFRAME group).
    XclChProperties     maProps;            /// Chart properties (CHPROPERTIES record).
    XclImpChTextMap     maDefTexts;         /// Default text objects (CHDEFAULTTEXT groups).
    XclImpChAxesSetRef  mxPrimAxesSet;      /// Primary axes set (CHAXESSET group).
    XclImpChAxesSetRef  mxSecnAxesSet;      /// Secondary axes set (CHAXESSET group).
    XclImpChTextRef     mxTitle;            /// Chart title (CHTEXT group).
    XclImpChLegendRef   mxLegend;           /// Chart legend (CHLEGEND group).
};

typedef boost::shared_ptr< XclImpChChart > XclImpChChartRef;

// ----------------------------------------------------------------------------

/** Drawing container of a chart. */
class XclImpChartDrawing : public XclImpDrawing
{
public:
    explicit            XclImpChartDrawing( const XclImpRoot& rRoot, bool bOwnTab );

    /** Converts all objects and inserts them into the chart drawing page. */
    void                ConvertObjects(
                            XclImpDffConverter& rDffConv,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& rxModel,
                            const Rectangle& rChartRect );

    /** Calculate the resulting rectangle of the passed anchor. */
    virtual Rectangle   CalcAnchorRect( const XclObjAnchor& rAnchor, bool bDffAnchor ) const;
    /** Called whenever an object has been inserted into the draw page. */
    virtual void        OnObjectInserted( const XclImpDrawObjBase& rDrawObj );

private:
    Rectangle           maChartRect;        /// Position and size of the chart shape in 1/100 mm.
    SCTAB               mnScTab;            /// Index of the sheet that contains the chart.
    bool                mbOwnTab;           /// True = own sheet, false = embedded object.
};

// ----------------------------------------------------------------------------

/** Represents the entire chart substream (all records in BOF/EOF block). */
class XclImpChart : protected XclImpRoot
{
public:
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > XModelRef;

public:
    /** Constructs a new chart object.
        @param bOwnTab  True = chart is on an own sheet; false = chart is an embedded object. */
    explicit            XclImpChart( const XclImpRoot& rRoot, bool bOwnTab );
    virtual             ~XclImpChart();

    /** Reads the complete chart substream (BOF/EOF block).
        @descr  The passed stream must be located in the BOF record of the chart substream. */
    void                ReadChartSubStream( XclImpStream& rStrm );
    /** Sets formatting from BIFF3-BIFF5 OBJ record, if own formatting is invisible. */
    void                UpdateObjFrame( const XclObjLineData& rLineData, const XclObjFillData& rFillData );

    /** Returns the number of units on the progress bar needed for the chart. */
    sal_Size            GetProgressSize() const;
    /** Returns true, if the chart is based on a pivot table. */
    inline bool         IsPivotChart() const { return mbIsPivotChart; }

    /** Creates the chart object in the passed component. */
    void                Convert( XModelRef xModel,
                            XclImpDffConverter& rDffConv,
                            const OUString& rObjName,
                            const Rectangle& rChartRect ) const;

private:
    /** Returns (initially creates) the drawing container for embedded shapes. **/
    XclImpChartDrawing& GetChartDrawing();
    /** Reads the CHCHART group (entire chart data). */
    void                ReadChChart( XclImpStream& rStrm );

private:
    typedef boost::shared_ptr< XclImpChartDrawing > XclImpChartDrawingRef;

    XclImpChChartRef    mxChartData;        /// The chart data (CHCHART group).
    XclImpChartDrawingRef mxChartDrawing;   /// Drawing container for embedded shapes.
    bool                mbOwnTab;           /// true = own sheet; false = embedded object.
    bool                mbIsPivotChart;     /// true = chart is based on a pivot table.
};

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
