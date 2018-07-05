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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_XECHART_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_XECHART_HXX

#include "xerecord.hxx"
#include "xlchart.hxx"
#include "xlformula.hxx"
#include "xlstyle.hxx"
#include "xeroot.hxx"
#include "xestring.hxx"

#include <memory>
#include <map>

class Size;
namespace tools { class Rectangle; }

namespace com { namespace sun { namespace star {
    namespace awt
    {
        struct Rectangle;
    }
    namespace frame
    {
        class XModel;
    }
    namespace chart
    {
        class XAxis;
    }
    namespace chart2
    {
        struct ScaleData;
        class XChartDocument;
        class XDiagram;
        class XCoordinateSystem;
        class XChartType;
        class XDataSeries;
        class XAxis;
        class XTitle;
        class XFormattedString;
        class XRegressionCurve;
        namespace data
        {
            class XDataSequence;
            class XLabeledDataSequence;
        }
    }
} } }

// Common =====================================================================

struct XclExpChRootData;
class XclExpChChart;

/** Base class for complex chart classes, provides access to other components
    of the chart.

    Keeps also track of future record levels and writes the needed future
    records on demand.
 */
class XclExpChRoot : public XclExpRoot
{
public:
    explicit            XclExpChRoot( const XclExpRoot& rRoot, XclExpChChart& rChartData );
    virtual             ~XclExpChRoot() override;

    XclExpChRoot(XclExpChRoot const &) = default;
    XclExpChRoot(XclExpChRoot &&) = default;
    XclExpChRoot & operator =(XclExpChRoot const &) = default;
    XclExpChRoot & operator =(XclExpChRoot &&) = default;

    /** Returns this root instance - for code readability in derived classes. */
    const XclExpChRoot& GetChRoot() const { return *this; }
    /** Returns the API Chart document model. */
    css::uno::Reference< css::chart2::XChartDocument > const &
                        GetChartDocument() const;
    /** Returns a reference to the parent chart data object. */
    XclExpChChart&      GetChartData() const;
    /** Returns chart type info for a unique chart type identifier. */
    const XclChTypeInfo& GetChartTypeInfo( XclChTypeId eType ) const;
    /** Returns the first fitting chart type info for the passed service name. */
    const XclChTypeInfo& GetChartTypeInfo( const OUString& rServiceName ) const;

    /** Returns an info struct about auto formatting for the passed object type. */
    const XclChFormatInfo& GetFormatInfo( XclChObjectType eObjType ) const;

    /** Starts the API chart document conversion. Must be called once before all API conversion. */
    void                InitConversion( css::uno::Reference< css::chart2::XChartDocument > const & xChartDoc,
                                        const tools::Rectangle& rChartRect ) const;
    /** Finishes the API chart document conversion. Must be called once after all API conversion. */
    void                FinishConversion() const;

    /** Returns true, if the passed color equals to the specified system color. */
    bool                IsSystemColor( const Color& rColor, sal_uInt16 nSysColorIdx ) const;
    /** Sets a system color and the respective color identifier. */
    void                SetSystemColor( Color& rColor, sal_uInt32& rnColorId, sal_uInt16 nSysColorIdx ) const;

    /** Converts the passed horizontal coordinate from 1/100 mm to Excel chart units. */
    sal_Int32           CalcChartXFromHmm( sal_Int32 nPosX ) const;
    /** Converts the passed vertical coordinate from 1/100 mm to Excel chart units. */
    sal_Int32           CalcChartYFromHmm( sal_Int32 nPosY ) const;
    /** Converts the passed rectangle from 1/100 mm to Excel chart units. */
    XclChRectangle      CalcChartRectFromHmm( const css::awt::Rectangle& rRect ) const;

    /** Reads all line properties from the passed property set. */
    void                ConvertLineFormat(
                            XclChLineFormat& rLineFmt,
                            const ScfPropertySet& rPropSet,
                            XclChPropertyMode ePropMode ) const;
    /** Reads solid area properties from the passed property set.
        @return  true = object contains complex fill properties. */
    bool                ConvertAreaFormat(
                            XclChAreaFormat& rAreaFmt,
                            const ScfPropertySet& rPropSet,
                            XclChPropertyMode ePropMode ) const;
    /** Reads gradient or bitmap area properties from the passed property set. */
    void                ConvertEscherFormat(
                            XclChEscherFormat& rEscherFmt,
                            XclChPicFormat& rPicFmt,
                            const ScfPropertySet& rPropSet,
                            XclChPropertyMode ePropMode ) const;
    /** Reads font properties from the passed property set. */
    sal_uInt16          ConvertFont(
                            const ScfPropertySet& rPropSet,
                            sal_Int16 nScript ) const;

    /** Reads the pie rotation property and returns the converted angle. */
    static sal_uInt16   ConvertPieRotation( const ScfPropertySet& rPropSet );

protected:
    /** Called from XclExpChGroupBase::Save, registers a new future record level. */
    void                RegisterFutureRecBlock( const XclChFrBlock& rFrBlock );
    /** Called from XclExpChFutureRecordBase::Save, Initializes the current future record level. */
    void                InitializeFutureRecBlock( XclExpStream& rStrm );
    /** Called from XclExpChGroupBase::Save, finalizes the current future record level. */
    void                FinalizeFutureRecBlock( XclExpStream& rStrm );

private:
    typedef std::shared_ptr< XclExpChRootData > XclExpChRootDataRef;
    XclExpChRootDataRef mxChData;           /// Reference to the root data object.
};

/** Base class for chart record groups. Provides helper functions to write sub records.

    A chart record group consists of a header record, followed by a CHBEGIN
    record, followed by group sub records, and finished with a CHEND record.
 */
class XclExpChGroupBase : public XclExpRecord, protected XclExpChRoot
{
public:
    explicit            XclExpChGroupBase(
                            const XclExpChRoot& rRoot, sal_uInt16 nFrType,
                            sal_uInt16 nRecId, std::size_t nRecSize = 0 );
    virtual             ~XclExpChGroupBase() override;

    /** Saves the header record. Calls WriteSubRecords() to let derived classes write sub records. */
    virtual void        Save( XclExpStream& rStrm ) override;
    /** Derived classes return whether there are any records embedded in this group. */
    virtual bool        HasSubRecords() const;
    /** Derived classes implement writing any records embedded in this group. */
    virtual void        WriteSubRecords( XclExpStream& rStrm ) = 0;

protected:
    /** Sets context information for future record blocks. */
    void                SetFutureRecordContext( sal_uInt16 nFrContext,
                            sal_uInt16 nFrValue1 = 0, sal_uInt16 nFrValue2 = 0 );

private:
    XclChFrBlock        maFrBlock;          /// Future records block settings.
};

/** Base class for chart future records. On saving, the record writes missing
    CHFRBLOCKBEGIN records automatically.
 */
class XclExpChFutureRecordBase : public XclExpFutureRecord, protected XclExpChRoot
{
public:
    explicit            XclExpChFutureRecordBase( const XclExpChRoot& rRoot,
                            XclFutureRecType eRecType, sal_uInt16 nRecId, std::size_t nRecSize );

    /** Writes missing CHFRBLOCKBEGIN records and this record. */
    virtual void        Save( XclExpStream& rStrm ) override;
};

// Frame formatting ===========================================================

class XclExpChFramePos : public XclExpRecord
{
public:
    explicit            XclExpChFramePos( sal_uInt16 nTLMode );

    /** Returns read/write access to the frame position data. */
    XclChFramePos& GetFramePosData() { return maData; }

private:
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    XclChFramePos       maData;             /// Position of the frame.
};

typedef std::shared_ptr< XclExpChFramePos > XclExpChFramePosRef;

class XclExpChLineFormat : public XclExpRecord
{
public:
    explicit            XclExpChLineFormat( const XclExpChRoot& rRoot );

    /** Converts line formatting properties from the passed property set. */
    void                Convert( const XclExpChRoot& rRoot,
                            const ScfPropertySet& rPropSet, XclChObjectType eObjType );
    /** Sets or clears the automatic flag. */
    void         SetAuto( bool bAuto ) { ::set_flag( maData.mnFlags, EXC_CHLINEFORMAT_AUTO, bAuto ); }
    /** Sets flag to show or hide an axis. */
    void         SetShowAxis( bool bShowAxis )
                            { ::set_flag( maData.mnFlags, EXC_CHLINEFORMAT_SHOWAXIS, bShowAxis ); }
    /** Sets the line format to the specified default type. */
    void                SetDefault( XclChFrameType eDefFrameType );

    /** Returns true, if the line format is set to automatic. */
    bool         IsAuto() const { return ::get_flag( maData.mnFlags, EXC_CHLINEFORMAT_AUTO ); }
    /** Returns true, if the line style is set to something visible. */
    bool         HasLine() const { return maData.mnPattern != EXC_CHLINEFORMAT_NONE; }
    /** Returns true, if the line contains default formatting according to the passed frame type. */
    bool                IsDefault( XclChFrameType eDefFrameType ) const;

private:
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    XclChLineFormat     maData;             /// Contents of the CHLINEFORMAT record.
    sal_uInt32          mnColorId;          /// Line color identifier.
};

typedef std::shared_ptr< XclExpChLineFormat > XclExpChLineFormatRef;

class XclExpChAreaFormat : public XclExpRecord
{
public:
    explicit            XclExpChAreaFormat( const XclExpChRoot& rRoot );

    /** Converts area formatting properties from the passed property set.
        @return  true = object contains complex fill properties. */
    bool                Convert( const XclExpChRoot& rRoot,
                            const ScfPropertySet& rPropSet, XclChObjectType eObjType );
    /** Sets or clears the automatic flag. */
    void         SetAuto( bool bAuto ) { ::set_flag( maData.mnFlags, EXC_CHAREAFORMAT_AUTO, bAuto ); }
    /** Sets the area format to the specified default type. */
    void                SetDefault( XclChFrameType eDefFrameType );

    /** Returns true, if the area format is set to automatic. */
    bool         IsAuto() const { return ::get_flag( maData.mnFlags, EXC_CHAREAFORMAT_AUTO ); }
    /** Returns true, if the area style is set to something visible. */
    bool         HasArea() const { return maData.mnPattern != EXC_PATT_NONE; }
    /** Returns true, if the area contains default formatting according to the passed frame type. */
    bool                IsDefault( XclChFrameType eDefFrameType ) const;

private:
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    XclChAreaFormat     maData;             /// Contents of the CHAREAFORMAT record.
    sal_uInt32          mnPattColorId;      /// Pattern color identifier.
    sal_uInt32          mnBackColorId;      /// Pattern background color identifier.
};

typedef std::shared_ptr< XclExpChAreaFormat > XclExpChAreaFormatRef;

class XclExpChEscherFormat : public XclExpChGroupBase
{
public:
    explicit            XclExpChEscherFormat( const XclExpChRoot& rRoot );

    /** Converts complex area formatting from the passed property set. */
    void                Convert( const ScfPropertySet& rPropSet, XclChObjectType eObjType );

    /** Returns true, if the object contains valid formatting data. */
    bool                IsValid() const;

    /** Writes the CHESCHERFORMAT record group to the stream, if complex formatting is extant. */
    virtual void        Save( XclExpStream& rStrm ) override;
    /** Returns true, if this record group contains a CHPICFORMAT record. */
    virtual bool        HasSubRecords() const override;
    /** Writes all embedded records. */
    virtual void        WriteSubRecords( XclExpStream& rStrm ) override;

private:
    /** Inserts a color from the contained Escher property set into the color palette. */
    sal_uInt32          RegisterColor( sal_uInt16 nPropId );

    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    XclChEscherFormat   maData;             /// Fill properties for complex areas (CHESCHERFORMAT record).
    XclChPicFormat      maPicFmt;           /// Image options, e.g. stretched, stacked (CHPICFORMAT record).
    sal_uInt32          mnColor1Id;         /// First fill color identifier.
    sal_uInt32          mnColor2Id;         /// Second fill color identifier.
};

typedef std::shared_ptr< XclExpChEscherFormat > XclExpChEscherFormatRef;

/** Base class for record groups containing frame formatting.

    Frame formatting can be part of several record groups, e.g. CHFRAME,
    CHDATAFORMAT, CHDROPBAR. It consists of CHLINEFORMAT, CHAREAFORMAT, and
    CHESCHERFORMAT group.
 */
class XclExpChFrameBase
{
public:
    explicit            XclExpChFrameBase();
    virtual             ~XclExpChFrameBase();

protected:
    /** Converts frame formatting properties from the passed property set. */
    void                ConvertFrameBase( const XclExpChRoot& rRoot,
                            const ScfPropertySet& rPropSet, XclChObjectType eObjType );
    /** Sets the frame formatting to the specified default type. */
    void                SetDefaultFrameBase( const XclExpChRoot& rRoot,
                            XclChFrameType eDefFrameType, bool bIsFrame );

    /** Returns true, if the frame contains default formatting (as if the frame is missing). */
    bool                IsDefaultFrameBase( XclChFrameType eDefFrameType ) const;

    /** Writes all contained frame records to the passed stream. */
    void                WriteFrameRecords( XclExpStream& rStrm );

private:
    XclExpChLineFormatRef mxLineFmt;        /// Line format (CHLINEFORMAT record).
    XclExpChAreaFormatRef mxAreaFmt;        /// Area format (CHAREAFORMAT record).
    XclExpChEscherFormatRef mxEscherFmt;    /// Complex area format (CHESCHERFORMAT record).
};

/** Represents the CHFRAME record group containing object frame properties.

    The CHFRAME group consists of: CHFRAME, CHBEGIN, CHLINEFORMAT,
    CHAREAFORMAT, CHESCHERFORMAT group, CHEND.
 */
class XclExpChFrame : public XclExpChGroupBase, public XclExpChFrameBase
{
public:
    explicit            XclExpChFrame( const XclExpChRoot& rRoot, XclChObjectType eObjType );

    /** Converts frame formatting properties from the passed property set. */
    void                Convert( const ScfPropertySet& rPropSet );
    /** Sets the specified automatic flags. */
    void                SetAutoFlags( bool bAutoPos, bool bAutoSize );

    /** Returns true, if the frame object contains default formats. */
    bool                IsDefault() const;
    /** Returns true, if the frame object can be deleted because it contains default formats. */
    bool                IsDeleteable() const;

    /** Writes the entire record group. */
    virtual void        Save( XclExpStream& rStrm ) override;
    /** Writes all embedded records. */
    virtual void        WriteSubRecords( XclExpStream& rStrm ) override;

private:
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    XclChFrame          maData;             /// Contents of the CHFRAME record.
    XclChObjectType     meObjType;          /// Type of the represented object.
};

typedef std::shared_ptr< XclExpChFrame > XclExpChFrameRef;

// Source links ===============================================================

class XclExpChSourceLink : public XclExpRecord, protected XclExpChRoot
{
public:
    explicit            XclExpChSourceLink( const XclExpChRoot& rRoot, sal_uInt8 nDestType );

    void                ConvertString( const OUString& aString );
    /** Converts the passed source link, returns the number of linked values. */
    sal_uInt16          ConvertDataSequence( css::uno::Reference< css::chart2::data::XDataSequence > const & xDataSeq,
                                             bool bSplitToColumns, sal_uInt16 nDefCount = 0 );
    /** Converts the passed sequence of formatted string objects, returns leading font index. */
    sal_uInt16          ConvertStringSequence( const css::uno::Sequence< css::uno::Reference< css::chart2::XFormattedString > >& rStringSeq );
    /** Converts the number format from the passed property set. */
    void                ConvertNumFmt( const ScfPropertySet& rPropSet, bool bPercent );

    void                AppendString( const OUString& rStr );

    /** Returns true, if this source link contains explicit string data. */
    bool         HasString() const { return mxString && !mxString->IsEmpty(); }

    /** Writes the CHSOURCELINK record and optionally a CHSTRING record with explicit string data. */
    virtual void        Save( XclExpStream& rStrm ) override;

private:
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    XclChSourceLink     maData;             /// Contents of the CHSOURCELINK record.
    XclTokenArrayRef    mxLinkFmla;         /// Formula with link to source data.
    XclExpStringRef     mxString;           /// Text data (CHSTRING record).
};

typedef std::shared_ptr< XclExpChSourceLink > XclExpChSourceLinkRef;

// Text =======================================================================

/** The CHFONT record containing a font index for text objects. */
class XclExpChFont : public XclExpUInt16Record
{
public:
    explicit            XclExpChFont( sal_uInt16 nFontIdx );
};

typedef std::shared_ptr< XclExpChFont > XclExpChFontRef;

/** The CHOBJECTLINK record linking a text object to a specific chart object. */
class XclExpChObjectLink : public XclExpRecord
{
public:
    explicit            XclExpChObjectLink( sal_uInt16 nLinkTarget, const XclChDataPointPos& rPointPos );

private:
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    XclChObjectLink     maData;             /// Contents of the CHOBJECTLINK record.
};

typedef std::shared_ptr< XclExpChObjectLink > XclExpChObjectLinkRef;

/** Additional data label settings in the future record CHFRLABELPROPS. */
class XclExpChFrLabelProps : public XclExpChFutureRecordBase
{
public:
    explicit            XclExpChFrLabelProps( const XclExpChRoot& rRoot );

    /** Converts separator and the passed data label flags. */
    void                Convert(
                            const ScfPropertySet& rPropSet,
                            bool bShowCateg, bool bShowValue,
                            bool bShowPercent, bool bShowBubble );

private:
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    XclChFrLabelProps   maData;             /// Contents of the CHFRLABELPROPS record.
};

typedef std::shared_ptr< XclExpChFrLabelProps > XclExpChFrLabelPropsRef;

/** Base class for objects with font settings. Provides font conversion helper functions. */
class XclExpChFontBase
{
public:
    virtual             ~XclExpChFontBase();

    /** Derived classes set font color and color identifier to internal data structures. */
    virtual void        SetFont( XclExpChFontRef xFont, const Color& rColor, sal_uInt32 nColorId ) = 0;
    /** Derived classes set text rotation to internal data structures. */
    virtual void        SetRotation( sal_uInt16 nRotation ) = 0;

    /** Creates a CHFONT record from the passed font index, calls virtual function SetFont(). */
    void                ConvertFontBase( const XclExpChRoot& rRoot, sal_uInt16 nFontIdx );
    /** Creates a CHFONT record from the passed font index, calls virtual function SetFont(). */
    void                ConvertFontBase( const XclExpChRoot& rRoot, const ScfPropertySet& rPropSet );
    /** Converts rotation settings, calls virtual function SetRotation(). */
    void                ConvertRotationBase( const ScfPropertySet& rPropSet, bool bSupportsStacked );
};

/** Represents the CHTEXT record group containing text object properties.

    The CHTEXT group consists of: CHTEXT, CHBEGIN, CHFRAMEPOS, CHFONT,
    CHFORMATRUNS, CHSOURCELINK, CHSTRING, CHFRAME group, CHOBJECTLINK, and CHEND.
 */
class XclExpChText : public XclExpChGroupBase, public XclExpChFontBase
{
public:
    explicit            XclExpChText( const XclExpChRoot& rRoot );

    /** Sets font color and color identifier to internal data structures. */
    virtual void        SetFont( XclExpChFontRef xFont, const Color& rColor, sal_uInt32 nColorId ) override;
    /** Sets text rotation to internal data structures. */
    virtual void        SetRotation( sal_uInt16 nRotation ) override;

    /** Converts all text settings of the passed title text object. */
    void                ConvertTitle( css::uno::Reference< css::chart2::XTitle > const & xTitle, sal_uInt16 nTarget, const OUString* pSubTitle );
    /** Converts all text settings of the passed legend. */
    void                ConvertLegend( const ScfPropertySet& rPropSet );
    /** Converts all settings of the passed data point caption text object. */
    bool                ConvertDataLabel( const ScfPropertySet& rPropSet,
                            const XclChTypeInfo& rTypeInfo, const XclChDataPointPos& rPointPos );
    /** Converts all settings of the passed trend line equation box. */
    void                ConvertTrendLineEquation( const ScfPropertySet& rPropSet, const XclChDataPointPos& rPointPos );

    /** Returns true, if the string object does not contain any text data. */
    bool         HasString() const { return mxSrcLink && mxSrcLink->HasString(); }
    /** Returns the flags needed for the CHATTACHEDLABEL record. */
    sal_uInt16          GetAttLabelFlags() const;

    /** Writes all embedded records. */
    virtual void        WriteSubRecords( XclExpStream& rStrm ) override;

private:
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    XclChText           maData;             /// Contents of the CHTEXT record.
    XclExpChFramePosRef mxFramePos;         /// Relative text frame position (CHFRAMEPOS record).
    XclExpChSourceLinkRef mxSrcLink;        /// Linked data (CHSOURCELINK with CHSTRING record).
    XclExpChFrameRef    mxFrame;            /// Text object frame properties (CHFRAME group).
    XclExpChFontRef     mxFont;             /// Index into font buffer (CHFONT record).
    XclExpChObjectLinkRef mxObjLink;        /// Link target for this text object.
    XclExpChFrLabelPropsRef mxLabelProps;   /// Extended data label properties (CHFRLABELPROPS record).
    sal_uInt32          mnTextColorId;      /// Text color identifier.
};

typedef std::shared_ptr< XclExpChText > XclExpChTextRef;

// Data series ================================================================

/** The CHMARKERFORMAT record containing data point marker formatting data. */
class XclExpChMarkerFormat : public XclExpRecord
{
public:
    explicit            XclExpChMarkerFormat( const XclExpChRoot& rRoot );

    /** Converts symbol properties from the passed property set. */
    void                Convert( const XclExpChRoot& rRoot,
                            const ScfPropertySet& rPropSet, sal_uInt16 nFormatIdx );
    /** Converts symbol properties for stock charts from the passed property set. */
    void                ConvertStockSymbol( const XclExpChRoot& rRoot,
                            const ScfPropertySet& rPropSet, bool bCloseSymbol );

    /** Returns true, if markers are enabled. */
    bool         HasMarker() const { return maData.mnMarkerType != EXC_CHMARKERFORMAT_NOSYMBOL; }
    /** Returns true, if border line of markers is visible. */
    bool         HasLineColor() const { return !::get_flag( maData.mnFlags, EXC_CHMARKERFORMAT_NOLINE ); }
    /** Returns true, if fill area of markers is visible. */
    bool         HasFillColor() const { return !::get_flag( maData.mnFlags, EXC_CHMARKERFORMAT_NOFILL ); }

private:
    /** Registers marker colors in palette and stores color identifiers. */
    void                RegisterColors( const XclExpChRoot& rRoot );

    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    XclChMarkerFormat   maData;             /// Contents of the CHMARKERFORMAT record.
    sal_uInt32          mnLineColorId;      /// Border line color identifier.
    sal_uInt32          mnFillColorId;      /// Fill color identifier.
};

typedef std::shared_ptr< XclExpChMarkerFormat > XclExpChMarkerFormatRef;

/** The CHPIEFORMAT record containing data point formatting data for pie segments. */
class XclExpChPieFormat : public XclExpUInt16Record
{
public:
    explicit            XclExpChPieFormat();

    /** Sets pie segment properties from the passed property set. */
    void                Convert( const ScfPropertySet& rPropSet );
};

typedef std::shared_ptr< XclExpChPieFormat > XclExpChPieFormatRef;

/** The CH3DDATAFORMAT record containing the bar type in 3D bar charts. */
class XclExpCh3dDataFormat : public XclExpRecord
{
public:
    explicit            XclExpCh3dDataFormat();

    /** Sets 3d bar properties from the passed property set. */
    void                Convert( const ScfPropertySet& rPropSet );

private:
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    XclCh3dDataFormat   maData;             /// Contents of the CH3DDATAFORMAT record.
};

typedef std::shared_ptr< XclExpCh3dDataFormat > XclExpCh3dDataFormatRef;

/** The CHATTACHEDLABEL record that contains the type of a data point label. */
class XclExpChAttachedLabel : public XclExpUInt16Record
{
public:
    explicit            XclExpChAttachedLabel( sal_uInt16 nFlags );
};

typedef std::shared_ptr< XclExpChAttachedLabel > XclExpChAttLabelRef;

/** Represents the CHDATAFORMAT record group containing data point properties.

    The CHDATAFORMAT group consists of: CHDATAFORMAT, CHBEGIN, CHFRAME group,
    CHMARKERFORMAT, CHPIEFORMAT, CH3DDATAFORMAT, CHSERIESFORMAT,
    CHATTACHEDLABEL, CHEND.
 */
class XclExpChDataFormat : public XclExpChGroupBase, public XclExpChFrameBase
{
public:
    explicit            XclExpChDataFormat( const XclExpChRoot& rRoot,
                            const XclChDataPointPos& rPointPos, sal_uInt16 nFormatIdx );

    /** Converts the passed data series or data point formatting. */
    void                ConvertDataSeries( const ScfPropertySet& rPropSet, const XclChExtTypeInfo& rTypeInfo );
    /** Sets default formatting for a series in a stock chart. */
    void                ConvertStockSeries( const ScfPropertySet& rPropSet, bool bCloseSymbol );
    /** Converts line formatting for the specified object (e.g. trend lines, error bars). */
    void                ConvertLine( const ScfPropertySet& rPropSet, XclChObjectType eObjType );

    /** Returns true, if this objects describes the formatting of an entire series. */
    bool         IsSeriesFormat() const { return maData.maPointPos.mnPointIdx == EXC_CHDATAFORMAT_ALLPOINTS; }

    /** Writes all embedded records. */
    virtual void        WriteSubRecords( XclExpStream& rStrm ) override;

private:
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    XclChDataFormat     maData;             /// Contents of the CHDATAFORMAT record.
    XclExpChMarkerFormatRef mxMarkerFmt;    /// Data point marker (CHMARKERFORMAT record).
    XclExpChPieFormatRef mxPieFmt;          /// Pie segment format (CHPIEFORMAT record).
    XclExpRecordRef     mxSeriesFmt;        /// Series properties (CHSERIESFORMAT record).
    XclExpCh3dDataFormatRef mx3dDataFmt;    /// 3D bar format (CH3DDATAFORMAT record).
    XclExpChAttLabelRef mxAttLabel;         /// Data point label type (CHATTACHEDLABEL record).
};

typedef std::shared_ptr< XclExpChDataFormat > XclExpChDataFormatRef;

/** Represents the CHSERTRENDLINE record containing settings for a trend line. */
class XclExpChSerTrendLine : public XclExpRecord, protected XclExpChRoot
{
public:
    explicit            XclExpChSerTrendLine( const XclExpChRoot& rRoot );

    /** Converts the passed trend line, returns true if trend line type is supported. */
    bool                Convert( css::uno::Reference< css::chart2::XRegressionCurve > const & xRegCurve,
                                 sal_uInt16 nSeriesIdx );

    /** Returns formatting information of the trend line, created in Convert(). */
    const XclExpChDataFormatRef& GetDataFormat() const { return mxDataFmt; }
    /** Returns formatting of the equation text box, created in Convert(). */
    const XclExpChTextRef& GetDataLabel() const { return mxLabel; }

private:
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    XclChSerTrendLine   maData;             /// Contents of the CHSERTRENDLINE record.
    XclExpChDataFormatRef mxDataFmt;        /// Formatting settings of the trend line.
    XclExpChTextRef     mxLabel;            /// Formatting of the equation text box.
};

typedef std::shared_ptr< XclExpChSerTrendLine > XclExpChSerTrendLineRef;

/** Represents the CHSERERRORBAR record containing settings for error bars. */
class XclExpChSerErrorBar : public XclExpRecord, protected XclExpChRoot
{
public:
    explicit            XclExpChSerErrorBar( const XclExpChRoot& rRoot, sal_uInt8 nBarType );

    /** Converts the passed error bar settings, returns true if error bar type is supported. */
    bool                Convert( XclExpChSourceLink& rValueLink, sal_uInt16& rnValueCount, const ScfPropertySet& rPropSet );

private:
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    XclChSerErrorBar    maData;             /// Contents of the CHSERERRORBAR record.
};

typedef std::shared_ptr< XclExpChSerErrorBar > XclExpChSerErrorBarRef;

/** Represents the CHSERIES record group describing a data series in a chart.

    The CHSERIES group consists of: CHSERIES, CHBEGIN, CHSOURCELINK groups,
    CHDATAFORMAT groups, CHSERGROUP, CHSERPARENT, CHSERERRORBAR,
    CHSERTRENDLINE, CHEND.
 */
class XclExpChSeries : public XclExpChGroupBase
{
public:
    explicit            XclExpChSeries( const XclExpChRoot& rRoot, sal_uInt16 nSeriesIdx );

    /** Converts the passed data series (source links and formatting). */
    bool                ConvertDataSeries(
                            css::uno::Reference< css::chart2::XDiagram > const & xDiagram,
                            css::uno::Reference< css::chart2::XDataSeries > const & xDataSeries,
                            const XclChExtTypeInfo& rTypeInfo,
                            sal_uInt16 nGroupIdx, sal_uInt16 nFormatIdx );
    /** Converts the passed data series for stock charts. */
    bool                ConvertStockSeries(
                            css::uno::Reference< css::chart2::XDataSeries > const & xDataSeries,
                            const OUString& rValueRole,
                            sal_uInt16 nGroupIdx, sal_uInt16 nFormatIdx, bool bCloseSymbol );
    /** Converts the passed error bar settings (called at trend line child series). */
    bool                ConvertTrendLine( const XclExpChSeries& rParent,
                                          css::uno::Reference< css::chart2::XRegressionCurve > const & xRegCurve );
    /** Converts the passed error bar settings (called at error bar child series). */
    bool                ConvertErrorBar( const XclExpChSeries& rParent, const ScfPropertySet& rPropSet, sal_uInt8 nBarId );
    /** Converts and inserts category ranges for all inserted series. */
    void                ConvertCategSequence( css::uno::Reference< css::chart2::data::XLabeledDataSequence > const & xCategSeq );

    /** Writes all embedded records. */
    virtual void        WriteSubRecords( XclExpStream& rStrm ) override;

private:
    /** Initializes members of this series to represent a child of the passed series. */
    void                InitFromParent( const XclExpChSeries& rParent );
    /** Tries to create trend line series objects (called at parent series). */
    void                CreateTrendLines( css::uno::Reference< css::chart2::XDataSeries > const & xDataSeries );
    /** Tries to create positive and negative error bar series objects (called at parent series). */
    void                CreateErrorBars( const ScfPropertySet& rPropSet,
                            const OUString& rBarPropName,
                            sal_uInt8 nPosBarId, sal_uInt8 nNegBarId );
    /** Tries to create an error bar series object (called at parent series). */
    void                CreateErrorBar( const ScfPropertySet& rPropSet,
                            const OUString& rShowPropName, sal_uInt8 nBarId );

    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    typedef XclExpRecordList< XclExpChDataFormat > XclExpChDataFormatList;

private:
    XclChSeries         maData;             /// Contents of the CHSERIES record.
    XclExpChSourceLinkRef mxTitleLink;      /// Link data for series title.
    XclExpChSourceLinkRef mxValueLink;      /// Link data for series values.
    XclExpChSourceLinkRef mxCategLink;      /// Link data for series category names.
    XclExpChSourceLinkRef mxBubbleLink;     /// Link data for series bubble sizes.
    XclExpChDataFormatRef mxSeriesFmt;      /// CHDATAFORMAT group for series format.
    XclExpChDataFormatList maPointFmts;     /// CHDATAFORMAT groups for data point formats.
    XclExpChSerTrendLineRef mxTrendLine;    /// Trend line settings (CHSERTRENDLINE record).
    XclExpChSerErrorBarRef mxErrorBar;      /// Error bar settings (CHSERERRORBAR record).
    sal_uInt16          mnGroupIdx;         /// Chart type group (CHTYPEGROUP group) this series is assigned to.
    sal_uInt16          mnSeriesIdx;        /// 0-based series index.
    sal_uInt16          mnParentIdx;        /// 0-based index of parent series (trend lines and error bars).
};

typedef std::shared_ptr< XclExpChSeries > XclExpChSeriesRef;

// Chart type groups ==========================================================

/** Represents the chart type record for all supported chart types. */
class XclExpChType : public XclExpRecord, protected XclExpChRoot
{
public:
    explicit            XclExpChType( const XclExpChRoot& rRoot );

    /** Converts the passed chart type and the contained data series. */
    void                Convert( css::uno::Reference< css::chart2::XDiagram > const & xDiagram,
                                 css::uno::Reference< css::chart2::XChartType > const & xChartType,
                                 sal_Int32 nApiAxesSetIdx, bool bSwappedAxesSet, bool bHasXLabels );
    /** Sets stacking mode (standard or percent) for the series in this chart type group. */
    void                SetStacked( bool bPercent );

    /** Returns true, if this is object represents a valid chart type. */
    bool         IsValidType() const { return maTypeInfo.meTypeId != EXC_CHTYPEID_UNKNOWN; }
    /** Returns the chart type info struct for the contained chart type. */
    const XclChTypeInfo& GetTypeInfo() const { return maTypeInfo; }

private:
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    XclChType           maData;             /// Contents of the chart type record.
    XclChTypeInfo       maTypeInfo;         /// Chart type info for the contained type.
};

/** Represents the CHCHART3D record that contains 3D view settings. */
class XclExpChChart3d : public XclExpRecord
{
public:
    explicit            XclExpChChart3d();

    /** Converts 3d settings for the passed chart type. */
    void                Convert( const ScfPropertySet& rPropSet, bool b3dWallChart );
    /** Sets flag that the data points are clustered on the X axis. */
    void         SetClustered() { ::set_flag( maData.mnFlags, EXC_CHCHART3D_CLUSTER ); }

    /** Returns true, if the data points are clustered on the X axis. */
    bool         IsClustered() const { return ::get_flag( maData.mnFlags, EXC_CHCHART3D_CLUSTER ); }

private:
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    XclChChart3d        maData;             /// Contents of the CHCHART3D record.
};

typedef std::shared_ptr< XclExpChChart3d > XclExpChChart3dRef;

/** Represents the CHLEGEND record group describing the chart legend.

    The CHLEGEND group consists of: CHLEGEND, CHBEGIN, CHFRAMEPOS, CHFRAME
    group, CHTEXT group, CHEND.
 */
class XclExpChLegend : public XclExpChGroupBase
{
public:
    explicit            XclExpChLegend( const XclExpChRoot& rRoot );

    /** Converts all legend settings from the passed property set. */
    void                Convert( const ScfPropertySet& rPropSet );

    /** Writes all embedded records. */
    virtual void        WriteSubRecords( XclExpStream& rStrm ) override;

private:
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    XclChLegend         maData;             /// Contents of the CHLEGEND record.
    XclExpChFramePosRef mxFramePos;         /// Legend frame position (CHFRAMEPOS record).
    XclExpChTextRef     mxText;             /// Legend text format (CHTEXT group).
    XclExpChFrameRef    mxFrame;            /// Legend frame format (CHFRAME group).
};

typedef std::shared_ptr< XclExpChLegend > XclExpChLegendRef;

/** Represents the CHDROPBAR record group describing pos/neg bars in line charts.

    The CHDROPBAR group consists of: CHDROPBAR, CHBEGIN, CHLINEFORMAT,
    CHAREAFORMAT, CHESCHERFORMAT group, CHEND.
 */
class XclExpChDropBar : public XclExpChGroupBase, public XclExpChFrameBase
{
public:
    explicit            XclExpChDropBar( const XclExpChRoot& rRoot, XclChObjectType eObjType );

    /** Converts and writes the contained frame data to the passed property set. */
    void                Convert( const ScfPropertySet& rPropSet );

    /** Writes all embedded records. */
    virtual void        WriteSubRecords( XclExpStream& rStrm ) override;

private:
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    XclChObjectType     meObjType;          /// Type of the dropbar.
};

typedef std::shared_ptr< XclExpChDropBar > XclExpChDropBarRef;

/** Represents the CHTYPEGROUP record group describing a group of series.

    The CHTYPEGROUP group consists of: CHTYPEGROUP, CHBEGIN, a chart type
    record (e.g. CHBAR, CHLINE, CHAREA, CHPIE, ...), CHCHART3D, CHLEGEND group,
    CHDROPBAR groups, CHCHARTLINE groups (CHCHARTLINE with CHLINEFORMAT),
    CHDATAFORMAT group, CHEND.
 */
class XclExpChTypeGroup : public XclExpChGroupBase
{
public:
    explicit            XclExpChTypeGroup( const XclExpChRoot& rRoot, sal_uInt16 nGroupIdx );

    /** Converts the passed chart type to Excel type settings. */
    void                ConvertType( css::uno::Reference< css::chart2::XDiagram > const & xDiagram,
                            css::uno::Reference< css::chart2::XChartType > const & xChartType,
                            sal_Int32 nApiAxesSetIdx, bool b3dChart, bool bSwappedAxesSet, bool bHasXLabels );
    /** Converts and inserts all series from the passed chart type. */
    void                ConvertSeries( css::uno::Reference< css::chart2::XDiagram > const & xDiagram,
                            css::uno::Reference< css::chart2::XChartType > const & xChartType,
                            sal_Int32 nGroupAxesSetIdx, bool bPercent, bool bConnectorLines );
    /** Converts and inserts category ranges for all inserted series. */
    void                ConvertCategSequence( css::uno::Reference< css::chart2::data::XLabeledDataSequence > const & xCategSeq );
    /** Creates a legend object and converts all legend settings. */
    void                ConvertLegend( const ScfPropertySet& rPropSet );

    /** Returns true, if this chart type group contains at least one valid series. */
    bool         IsValidGroup() const { return !maSeries.IsEmpty() && maType.IsValidType(); }
    /** Returns the index of this chart type group format. */
    sal_uInt16   GetGroupIdx() const { return maData.mnGroupIdx; }
    /** Returns the chart type info struct for the contained chart type. */
    const XclChExtTypeInfo& GetTypeInfo() const { return maTypeInfo; }
    /** Returns true, if the chart is three-dimensional. */
    bool         Is3dChart() const { return maTypeInfo.mb3dChart; }
    /** Returns true, if chart type supports wall and floor format. */
    bool         Is3dWallChart() const { return Is3dChart() && (maTypeInfo.meTypeCateg != EXC_CHTYPECATEG_PIE); }
    /** Returns true, if the series in this chart type group are ordered on the Z axis. */
    bool         Is3dDeepChart() const { return Is3dWallChart() && mxChart3d && !mxChart3d->IsClustered(); }
    /** Returns true, if this chart type can be combined with other types. */
    bool         IsCombinable2d() const { return !Is3dChart() && maTypeInfo.mbCombinable2d; }

    /** Writes all embedded records. */
    virtual void        WriteSubRecords( XclExpStream& rStrm ) override;

private:
    /** Returns an unused format index to be used for the next created series. */
    sal_uInt16          GetFreeFormatIdx() const;
    /** Creates all data series of any chart type except stock charts. */
    void                CreateDataSeries( css::uno::Reference< css::chart2::XDiagram > const & xDiagram,
                            css::uno::Reference< css::chart2::XDataSeries > const & xDataSeries );
    /** Creates all data series of a stock chart. */
    void                CreateAllStockSeries( css::uno::Reference< css::chart2::XChartType > const & xChartType,
                            css::uno::Reference< css::chart2::XDataSeries > const & xDataSeries );
    /** Creates a single data series of a stock chart. */
    bool                CreateStockSeries( css::uno::Reference< css::chart2::XDataSeries > const & xDataSeries,
                            const OUString& rValueRole, bool bCloseSymbol );

    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    typedef XclExpRecordList< XclExpChSeries >          XclExpChSeriesList;
    typedef ::std::map<sal_uInt16, std::unique_ptr<XclExpChLineFormat>> XclExpChLineFormatMap;

    XclChTypeGroup      maData;             /// Contents of the CHTYPEGROUP record.
    XclExpChType        maType;             /// Chart type (e.g. CHBAR, CHLINE, ...).
    XclChExtTypeInfo    maTypeInfo;         /// Extended chart type info.
    XclExpChSeriesList  maSeries;           /// List of series data (CHSERIES groups).
    XclExpChChart3dRef  mxChart3d;          /// 3D settings (CHCHART3D record).
    XclExpChLegendRef   mxLegend;           /// Chart legend (CHLEGEND group).
    XclExpChDropBarRef  mxUpBar;            /// White dropbars (CHDROPBAR group).
    XclExpChDropBarRef  mxDownBar;          /// Black dropbars (CHDROPBAR group).
    XclExpChLineFormatMap m_ChartLines;     /// Global line formats (CHCHARTLINE group).
};

typedef std::shared_ptr< XclExpChTypeGroup > XclExpChTypeGroupRef;

// Axes =======================================================================

class XclExpChLabelRange : public XclExpRecord, protected XclExpChRoot
{
public:
    explicit            XclExpChLabelRange( const XclExpChRoot& rRoot );

    /** Converts category axis scaling settings. */
    void                Convert( const css::chart2::ScaleData& rScaleData,
                            const ScfPropertySet& rChart1Axis, bool bMirrorOrient );
    /** Converts position settings of a crossing axis at this axis. */
    void                ConvertAxisPosition( const ScfPropertySet& rPropSet );
    /** Sets flag for tickmark position between categories or on categories. */
    void         SetTicksBetweenCateg( bool bTicksBetween )
                            { ::set_flag( maLabelData.mnFlags, EXC_CHLABELRANGE_BETWEEN, bTicksBetween ); }

private:
    virtual void        Save( XclExpStream& rStrm ) override;
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    XclChLabelRange     maLabelData;        /// Contents of the CHLABELRANGE record.
    XclChDateRange      maDateData;         /// Contents of the CHDATERANGE record.
};

typedef std::shared_ptr< XclExpChLabelRange > XclExpChLabelRangeRef;

class XclExpChValueRange : public XclExpRecord, protected XclExpChRoot
{
public:
    explicit            XclExpChValueRange( const XclExpChRoot& rRoot );

    /** Converts value axis scaling settings. */
    void                Convert( const css::chart2::ScaleData& rScaleData );
    /** Converts position settings of a crossing axis at this axis. */
    void                ConvertAxisPosition( const ScfPropertySet& rPropSet );

private:
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    XclChValueRange     maData;             /// Contents of the CHVALUERANGE record.
};

typedef std::shared_ptr< XclExpChValueRange > XclExpChValueRangeRef;

class XclExpChTick : public XclExpRecord, protected XclExpChRoot
{
public:
    explicit            XclExpChTick( const XclExpChRoot& rRoot );

    /** Converts axis tick mark settings. */
    void                Convert( const ScfPropertySet& rPropSet, const XclChExtTypeInfo& rTypeInfo, sal_uInt16 nAxisType );
    /** Sets font color and color identifier to internal data structures. */
    void                SetFontColor( const Color& rColor, sal_uInt32 nColorId );
    /** Sets text rotation to internal data structures. */
    void                SetRotation( sal_uInt16 nRotation );

private:
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    XclChTick           maData;             /// Contents of the CHTICK record.
    sal_uInt32          mnTextColorId;      /// Axis labels text color identifier.
};

typedef std::shared_ptr< XclExpChTick > XclExpChTickRef;

/** Represents the CHAXIS record group describing an entire chart axis.

    The CHAXIS group consists of: CHAXIS, CHBEGIN, CHLABELRANGE, CHEXTRANGE,
    CHVALUERANGE, CHFORMAT, CHTICK, CHFONT, CHAXISLINE groups (CHAXISLINE with
    CHLINEFORMAT, CHAREAFORMAT, and CHESCHERFORMAT group), CHEND.
 */
class XclExpChAxis : public XclExpChGroupBase, public XclExpChFontBase
{
public:
    explicit            XclExpChAxis( const XclExpChRoot& rRoot, sal_uInt16 nAxisType );

    /** Sets font color and color identifier to internal data structures. */
    virtual void        SetFont( XclExpChFontRef xFont, const Color& rColor, sal_uInt32 nColorId ) override;
    /** Sets text rotation to internal data structures. */
    virtual void        SetRotation( sal_uInt16 nRotation ) override;

    /** Converts formatting and scaling settings from the passed axis. */
    void                Convert( css::uno::Reference< css::chart2::XAxis > const & xAxis,
                            css::uno::Reference< css::chart2::XAxis > const & xCrossingAxis,
                            css::uno::Reference< css::chart::XAxis > const & xChart1Axis,
                            const XclChExtTypeInfo& rTypeInfo );
    /** Converts and writes 3D wall/floor properties from the passed diagram. */
    void                ConvertWall( css::uno::Reference< css::chart2::XDiagram > const & xDiagram );

    /** Returns the type of this axis. */
    sal_uInt16   GetAxisType() const { return maData.mnType; }
    /** Returns the axis dimension index used by the chart API. */
    sal_Int32    GetApiAxisDimension() const { return maData.GetApiAxisDimension(); }

    /** Writes all embedded records. */
    virtual void        WriteSubRecords( XclExpStream& rStrm ) override;

private:
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    XclChAxis           maData;             /// Contents of the CHAXIS record.
    XclExpChLabelRangeRef mxLabelRange;     /// Category scaling (CHLABELRANGE record).
    XclExpChValueRangeRef mxValueRange;     /// Value scaling (CHVALUERANGE record).
    XclExpChTickRef     mxTick;             /// Axis ticks (CHTICK record).
    XclExpChFontRef     mxFont;             /// Index into font buffer (CHFONT record).
    XclExpChLineFormatRef mxAxisLine;       /// Axis line format (CHLINEFORMAT record).
    XclExpChLineFormatRef mxMajorGrid;      /// Major grid line format (CHLINEFORMAT record).
    XclExpChLineFormatRef mxMinorGrid;      /// Minor grid line format (CHLINEFORMAT record).
    XclExpChFrameRef    mxWallFrame;        /// Wall/floor format (sub records of CHFRAME group).
    sal_uInt16          mnNumFmtIdx;        /// Index into number format buffer (CHFORMAT record).
};

typedef std::shared_ptr< XclExpChAxis > XclExpChAxisRef;

/** Represents the CHAXESSET record group describing an axes set (X/Y/Z axes).

    The CHAXESSET group consists of: CHAXESSET, CHBEGIN, CHFRAMEPOS, CHAXIS
    groups, CHTEXT groups, CHPLOTFRAME group (CHPLOTFRAME with CHFRAME group),
    CHTYPEGROUP group, CHEND.
 */
class XclExpChAxesSet : public XclExpChGroupBase
{
public:
    explicit            XclExpChAxesSet( const XclExpChRoot& rRoot, sal_uInt16 nAxesSetId );

    /** Converts the passed diagram to chart record data.
        @return  First unused chart type group index. */
    sal_uInt16          Convert( css::uno::Reference< css::chart2::XDiagram > const & xDiagram,
                                 sal_uInt16 nFirstGroupIdx );

    /** Returns true, if this axes set exists (returns false if this is a dummy object). */
    bool         IsValidAxesSet() const { return !maTypeGroups.IsEmpty(); }
    /** Returns the index of the axes set (primary/secondary). */
    sal_uInt16   GetAxesSetId() const { return maData.mnAxesSetId; }
    /** Returns the axes set index used by the chart API. */
    sal_Int32    GetApiAxesSetIndex() const { return maData.GetApiAxesSetIndex(); }
    /** Returns true, if the chart is three-dimensional. */
    bool                Is3dChart() const;

    /** Writes all embedded records. */
    virtual void        WriteSubRecords( XclExpStream& rStrm ) override;

private:
    /** Returns first inserted chart type group. */
    XclExpChTypeGroupRef GetFirstTypeGroup() const;
    /** Returns last inserted chart type group. */
    XclExpChTypeGroupRef GetLastTypeGroup() const;

    /** Converts a complete axis object including axis title. */
    void                ConvertAxis( XclExpChAxisRef& rxChAxis, sal_uInt16 nAxisType,
                            XclExpChTextRef& rxChAxisTitle, sal_uInt16 nTitleTarget,
                            css::uno::Reference< css::chart2::XCoordinateSystem > const & xCoordSystem,
                            const XclChExtTypeInfo& rTypeInfo,
                            sal_Int32 nCrossingAxisDim );

    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    typedef XclExpRecordList< XclExpChTypeGroup > XclExpChTypeGroupList;

    XclChAxesSet        maData;             /// Contents of the CHAXESSET record.
    XclExpChFramePosRef mxFramePos;         /// Outer plot area position (CHFRAMEPOS record).
    XclExpChAxisRef     mxXAxis;            /// The X axis (CHAXIS group).
    XclExpChAxisRef     mxYAxis;            /// The Y axis (CHAXIS group).
    XclExpChAxisRef     mxZAxis;            /// The Z axis (CHAXIS group).
    XclExpChTextRef     mxXAxisTitle;       /// The X axis title (CHTEXT group).
    XclExpChTextRef     mxYAxisTitle;       /// The Y axis title (CHTEXT group).
    XclExpChTextRef     mxZAxisTitle;       /// The Z axis title (CHTEXT group).
    XclExpChFrameRef    mxPlotFrame;        /// Plot area (CHPLOTFRAME group).
    XclExpChTypeGroupList maTypeGroups;     /// Chart type groups (CHTYPEGROUP group).
};

typedef std::shared_ptr< XclExpChAxesSet > XclExpChAxesSetRef;

// The chart object ===========================================================

/** Represents the CHCHART record group describing the chart contents.

    The CHCHART group consists of: CHCHART, CHBEGIN, SCL, CHPLOTGROWTH, CHFRAME
    group, CHSERIES groups, CHPROPERTIES, CHDEFAULTTEXT groups (CHDEFAULTTEXT
    with CHTEXT groups), CHUSEDAXESSETS, CHAXESSET groups, CHTEXT groups, CHEND.
 */
class XclExpChChart : public XclExpChGroupBase
{
public:
    explicit            XclExpChChart( const XclExpRoot& rRoot,
                            css::uno::Reference< css::chart2::XChartDocument > const & xChartDoc,
                            const tools::Rectangle& rChartRect );

    /** Creates, registers and returns a new data series object. */
    XclExpChSeriesRef   CreateSeries();
    /** Removes the last created data series object from the series list. */
    void                RemoveLastSeries();
    /** Stores a CHTEXT group that describes a data point label. */
    void                SetDataLabel( XclExpChTextRef const & xText );
    /** Sets the plot area position and size to manual mode. */
    void                SetManualPlotArea();

    /** Writes all embedded records. */
    virtual void        WriteSubRecords( XclExpStream& rStrm ) override;

private:
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    typedef XclExpRecordList< XclExpChSeries >  XclExpChSeriesList;
    typedef XclExpRecordList< XclExpChText >    XclExpChTextList;

    XclChRectangle      maRect;             /// Position of the chart on the sheet (CHCHART record).
    XclExpChSeriesList  maSeries;           /// List of series data (CHSERIES groups).
    XclExpChFrameRef    mxFrame;            /// Chart frame format (CHFRAME group).
    XclChProperties     maProps;            /// Chart properties (CHPROPERTIES record).
    XclExpChAxesSetRef  mxPrimAxesSet;      /// Primary axes set (CHAXESSET group).
    XclExpChAxesSetRef  mxSecnAxesSet;      /// Secondary axes set (CHAXESSET group).
    XclExpChTextRef     mxTitle;            /// Chart title (CHTEXT group).
    XclExpChTextList    maLabels;           /// Data point labels (CHTEXT groups).
};

/** Represents the group of DFF and OBJ records containing all drawing shapes
    embedded in the chart object.
 */
class XclExpChartDrawing : public XclExpRecordBase, protected XclExpRoot
{
public:
    explicit            XclExpChartDrawing(
                            const XclExpRoot& rRoot,
                            const css::uno::Reference< css::frame::XModel >& rxModel,
                            const Size& rChartSize );
    virtual             ~XclExpChartDrawing() override;

    virtual void        Save( XclExpStream& rStrm ) override;

private:
    std::shared_ptr< XclExpObjectManager > mxObjMgr;
    std::shared_ptr< XclExpRecordBase > mxObjRecs;
};

/** Represents the entire chart substream (all records in BOF/EOF block). */
class XclExpChart : public XclExpSubStream, protected XclExpRoot
{
public:
    explicit            XclExpChart( const XclExpRoot& rRoot,
                            css::uno::Reference< css::frame::XModel > const & xModel,
                            const tools::Rectangle& rChartRect );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
