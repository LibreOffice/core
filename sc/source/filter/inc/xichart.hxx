/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xichart.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 13:58:48 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_XICHART_HXX
#define SC_XICHART_HXX

#include <vector>
#include <map>
#include <set>

#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

#ifndef SC_RANGELST_HXX
#include "rangelst.hxx"
#endif

#ifndef SC_XLCHART_HXX
#include "xlchart.hxx"
#endif
#ifndef SC_XISTRING_HXX
#include "xistring.hxx"
#endif
#ifndef SC_XIROOT_HXX
#include "xiroot.hxx"
#endif

namespace com { namespace sun { namespace star {
    namespace chart
    {
        class XChartDocument;
        class XDiagram;
    }
} } }

// Common =====================================================================

/** Template for a map of ref-counted objects with additional accessor functions. */
template< typename KeyType, typename ObjType >
class XclImpChMap : public ::std::map< KeyType, ScfRef< ObjType > >
{
public:
    typedef KeyType                             key_type;
    typedef ScfRef< ObjType >                   ref_type;
    typedef ::std::map< key_type, ref_type >    map_type;

    /** Returns true, if the object accossiated to the passed key exists. */
    inline bool         has( key_type nKey ) const
                        {
                            typename map_type::const_iterator aIt = find( nKey );
                            return (aIt != this->end()) && aIt->second.is();
                        }

    /** Returns a reference to the object accossiated to the passed key, or 0 on error. */
    inline ref_type     get( key_type nKey ) const
                        {
                            typename map_type::const_iterator aIt = find( nKey );
                            if( aIt != this->end() ) return aIt->second;
                            return ref_type();
                        }
};

// ----------------------------------------------------------------------------

class ScfProgressBar;
struct XclImpChRootData;
class XclImpChChart;

/** Base class for complex chart classes, provides access to other components of the chart. */
class XclImpChRoot : public XclImpRoot
{
public:
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::chart::XChartDocument > XChartDocRef;

public:
    explicit            XclImpChRoot( const XclImpRoot& rRoot, XclImpChChart* pChartData );
    virtual             ~XclImpChRoot();

    /** Returns this root instance - for code readability in derived classes. */
    inline const XclImpChRoot& GetChRoot() const { return *this; }
    /** Returns a reference to the parent chart data object. */
    XclImpChChart&      GetChartData() const;

    /** Returns the default line color for charts. */
    Color               GetLineAutoColor() const;
    /** Returns the default fill color for charts. */
    Color               GetFillAutoColor() const;
    /** Returns the default text color for charts. */
    Color               GetFontAutoColor() const;
    /** Returns the automatic line color of linear series. */
    Color               GetSeriesLineAutoColor( sal_uInt16 nFormatIdx ) const;
    /** Returns the automatic fill color of filled series. */
    Color               GetSeriesFillAutoColor( sal_uInt16 nFormatIdx ) const;

    /** Starts the API chart document conversion. Must be called once before all API conversion. */
    void                InitConversion( XChartDocRef xChartDoc ) const;
    /** Finishes the API chart document conversion. Must be called once after all API conversion. */
    void                FinishConversion( ScfProgressBar& rProgress ) const;

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
    void                ConvertEscherForrmat(
                            ScfPropertySet& rPropSet,
                            const SfxItemSet& rItemSet,
                            const XclChPicFormat& rPicFmt ) const;
    /** Writes font properties to the passed property set. */
    void                ConvertFont(
                            ScfPropertySet& rPropSet,
                            sal_uInt16 nFontIdx,
                            const Color* pFontColor = 0 ) const;

private:
    typedef ScfRef< XclImpChRootData > XclImpChRootDataRef;
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

// Formatting =================================================================

class XclImpChFramePos
{
public:
    /** Reads the CHFRAMEPOS record (frame position and size). */
    void                ReadChFramePos( XclImpStream& rStrm );

private:
    XclChFramePos       maData;             /// Position of the frame.
};

typedef ScfRef< XclImpChFramePos > XclImpChFramePosRef;

// ----------------------------------------------------------------------------

class XclImpChLineFormat
{
public:
    /** Creates a new line format object with specific default formatting. */
    explicit            XclImpChLineFormat( sal_uInt16 nLinkTarget = EXC_CHOBJLINK_NONE );

    /** Reads the CHLINEFORMAT record (basic line properties). */
    void                ReadChLineFormat( XclImpStream& rStrm );

    /** Returns true, if the line format is set to automatic. */
    inline bool         IsAuto() const { return ::get_flag( maData.mnFlags, EXC_CHLINEFORMAT_AUTO ); }
    /** Returns true, if the line style is set to something visible. */
    inline bool         HasLine() const { return IsAuto() || (maData.mnPattern != EXC_CHLINEFORMAT_NONE); }
    /** Returns the line width of this line format (returns 'single', if the line is invisible). */
    inline sal_uInt16   GetWeight() const { return (IsAuto() || !HasLine()) ? EXC_CHLINEFORMAT_SINGLE : maData.mnWeight; }
    /** Returns true, if the "show axis" flag is set. */
    inline bool         IsShowAxis() const { return ::get_flag( maData.mnFlags, EXC_CHLINEFORMAT_SHOWAXIS ); }

    /** Converts and writes the contained data to the passed property set. */
    void                Convert( const XclImpChRoot& rRoot, ScfPropertySet& rPropSet,
                            sal_uInt16 nAutoWeight = EXC_CHLINEFORMAT_SINGLE ) const;
    /** Converts and writes the contained data to the passed property set of a series or data point. */
    void                ConvertSeries( const XclImpChRoot& rRoot, ScfPropertySet& rPropSet,
                            sal_uInt16 nFormatIdx, bool bLinear2dType ) const;

private:
    XclChLineFormat     maData;             /// Contents of the CHLINEFORMAT record.
};

typedef ScfRef< XclImpChLineFormat > XclImpChLineFormatRef;

// ----------------------------------------------------------------------------

class XclImpChAreaFormat
{
public:
    /** Creates a new area format object with specific default formatting. */
    explicit            XclImpChAreaFormat( sal_uInt16 nLinkTarget = EXC_CHOBJLINK_NONE );

    /** Reads the CHAREAFORMAT record (basic fill properties, e.g. transparent or colored). */
    void                ReadChAreaFormat( XclImpStream& rStrm );

    /** Returns true, if the area format is set to automatic. */
    inline bool         IsAuto() const { return ::get_flag( maData.mnFlags, EXC_CHAREAFORMAT_AUTO ); }
    /** Returns true, if the area style is set to something visible. */
    inline bool         HasArea() const { return IsAuto() || (maData.mnPattern != EXC_CHAREAFORMAT_NONE); }

    /** Converts and writes the contained data to the passed property set. */
    void                Convert( const XclImpChRoot& rRoot, ScfPropertySet& rPropSet ) const;
    /** Converts and writes the contained data to the passed property set of a series or data point. */
    void                ConvertSeries( const XclImpChRoot& rRoot,
                            ScfPropertySet& rPropSet, sal_uInt16 nFormatIdx ) const;

private:
    XclChAreaFormat     maData;             /// Contents of the CHAREAFORMAT record.
};

typedef ScfRef< XclImpChAreaFormat > XclImpChAreaFormatRef;

// ----------------------------------------------------------------------------

class XclImpChEscherFormat : public XclImpChGroupBase
{
public:
    explicit            XclImpChEscherFormat( const XclImpRoot& rRoot );

    /** Reads the CHESCHERFORMAT record (complex fill data) (called by base class). */
    virtual void        ReadHeaderRecord( XclImpStream& rStrm );
    /** Reads a record from the CHESCHERFORMAT group (called by base class). */
    virtual void        ReadSubRecord( XclImpStream& rStrm );

    /** Converts and writes the contained data to the passed property set. */
    void                Convert( const XclImpChRoot& rRoot, ScfPropertySet& rPropSet ) const;

private:
    SfxItemSet          maItemSet;          /// Translated fill items for complex areas (CHESCHERFORMAT record).
    XclChPicFormat      maPicFmt;           /// Image options, e.g. stretched, stacked (CHPICFORMAT record).
};

typedef ScfRef< XclImpChEscherFormat > XclImpChEscherFormatRef;

// ----------------------------------------------------------------------------

/** Base class for record groups containing frame formatting.

    Frame formatting can be part of several record groups, e.g. CHFRAME,
    CHDATAFORMAT, CHDROPBAR. It consists of CHLINEFORMAT, CHAREAFORMAT, and
    CHESCHERFORMAT group.
 */
class XclImpChFrameBase : public XclImpChGroupBase
{
public:
    /** Creates a new frame object with specific default formatting. */
    explicit            XclImpChFrameBase( sal_uInt16 nLinkTarget = EXC_CHOBJLINK_NONE );

    /** Reads a frame formatting record (called by base class). */
    virtual void        ReadSubRecord( XclImpStream& rStrm );

    /** Returns true, if the line format is set to automatic. */
    inline bool         IsAutoLine() const { return !mxLineFmt || mxLineFmt->IsAuto(); }
    /** Returns true, if the line style is set to something visible. */
    inline bool         HasLine() const { return IsAutoLine() || mxLineFmt->HasLine(); }
    /** Returns true, if the line style is set to something visible. */
    inline sal_uInt16   GetLineWeight() const { return mxLineFmt.is() ? mxLineFmt->GetWeight() : EXC_CHLINEFORMAT_SINGLE; }

    /** Returns true, if the area format is set to automatic. */
    inline bool         IsAutoArea() const { return !mxEscherFmt && (!mxAreaFmt || mxAreaFmt->IsAuto()); }
    /** Returns true, if the area style is set to something visible. */
    inline bool         HasArea() const { return mxEscherFmt.is() || IsAutoArea() || mxAreaFmt->HasArea(); }

protected:
    /** Converts and writes the contained data to the passed property set. */
    void                ConvertFrameBase( const XclImpChRoot& rRoot,
                            ScfPropertySet& rPropSet ) const;
    /** Converts and writes the contained data to the passed property set of a series or data point. */
    void                ConvertSeriesFrameBase(
                            const XclImpChRoot& rRoot, ScfPropertySet& rPropSet,
                            sal_uInt16 nFormatIdx, bool bLinear2dType ) const;

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
class XclImpChFrame : public XclImpChFrameBase
{
public:
    /** Creates a new frame object with specific default formatting. */
    explicit            XclImpChFrame( sal_uInt16 nLinkTarget = EXC_CHOBJLINK_NONE );

    /** Reads the CHFRAME record (called by base class). */
    virtual void        ReadHeaderRecord( XclImpStream& rStrm );

    /** Converts and writes the contained data to the passed property set. */
    void                Convert( const XclImpChRoot& rRoot, ScfPropertySet& rPropSet ) const;

private:
    /** Updates frame objects according to current link target. */
    void                UpdateObjects();

private:
    XclChFrame          maData;             /// Contents of the CHFRAME record.
};

typedef ScfRef< XclImpChFrame > XclImpChFrameRef;

// ----------------------------------------------------------------------------

class XclImpChMarkerFormat
{
public:
    /** Reads the CHMARKERFORMAT record (data point marker properties). */
    void                ReadChMarkerFormat( XclImpStream& rStrm );

    /** Returns true, if the marker format is set to automatic. */
    inline bool         IsAuto() const { return ::get_flag( maData.mnFlags, EXC_CHMARKERFORMAT_AUTO ); }
    /** Returns true, if markers are enabled. */
    inline bool         HasMarker() const { return IsAuto() || (maData.mnMarkerType != EXC_CHMARKERFORMAT_NOSYMBOL); }

    /** Converts and writes the contained data to the passed property set. */
    void                Convert( const XclImpChRoot& rRoot, ScfPropertySet& rPropSet,
                            sal_uInt16 nFormatIdx, sal_uInt16 nLineWeight ) const;
    /** Sets the marker fill color as main color to the passed property set. */
    void                ConvertColor( const XclImpChRoot& rRoot,
                            ScfPropertySet& rPropSet, sal_uInt16 nFormatIdx ) const;

private:
    XclChMarkerFormat   maData;             /// Contents of the CHMARKERFORMAT record.
};

typedef ScfRef< XclImpChMarkerFormat > XclImpChMarkerFormatRef;

// ----------------------------------------------------------------------------

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

typedef ScfRef< XclImpChPieFormat > XclImpChPieFormatRef;

// ----------------------------------------------------------------------------

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

typedef ScfRef< XclImpChSeriesFormat > XclImpChSeriesFormatRef;

// ----------------------------------------------------------------------------

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

typedef ScfRef< XclImpCh3dDataFormat > XclImpCh3dDataFormatRef;

// Text =======================================================================

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

typedef ScfRef< XclImpChFont > XclImpChFontRef;

// ----------------------------------------------------------------------------

class XclImpChFormat
{
public:
    explicit            XclImpChFormat();
    /** Reads the CHFORMAT record (number format index). */
    void                ReadChFormat( XclImpStream& rStrm );
    /** Converts and writes the contained data to the passed property set. */
    void                Convert( const XclImpChRoot& rRoot, ScfPropertySet& rPropSet ) const;

private:
    sal_uInt16          mnNumFmtIdx;        /// Index into number format buffer.
};

typedef ScfRef< XclImpChFormat > XclImpChFormatRef;

// ----------------------------------------------------------------------------

/** Represents linked or literal string data attached to a title or series. */
class XclImpChString
{
public:
    /** Reads the CHSTRING record (string data). */
    void                ReadChString( XclImpStream& rStrm );
    /** Reads the CHFORMATRUNS record (more font indexes). */
    void                ReadChFormatRuns( XclImpStream& rStrm );

    /** Returns true, if the object contains any text data. */
    inline bool         HasString() const { return !maData.IsEmpty(); }

    /** Returns the string read from the CHSTRING record. */
    inline const String& GetString() const { return maData.GetText(); }
    /** Writes the contained string to the passed property set. */
    void                Convert( ScfPropertySet& rPropSet ) const;

private:
    XclImpString        maData;             /// Contents of the CHSTRING record.
};

typedef ScfRef< XclImpChString > XclImpChStringRef;

// ----------------------------------------------------------------------------

/** Base class for objects with font settings. Provides font conversion helper functions. */
class XclImpChFontBase
{
public:
    virtual             ~XclImpChFontBase();

    /** Derived classes return the leading font index for the text object. */
    virtual sal_uInt16  GetFontIndex() const = 0;
    /** Derived classes return the leading font color for the text object. */
    virtual Color       GetFontColor() const = 0;

    /** Converts and writes the contained font settings to the passed property set. */
    void                ConvertFontBase( const XclImpChRoot& rRoot, ScfPropertySet& rPropSet ) const;
};

// ----------------------------------------------------------------------------

/** Base class for objects with text rotation. Provides conversion helper functions. */
class XclImpChRotationBase
{
public:
    virtual             ~XclImpChRotationBase();

    /** Derived classes return the rotation value for the text. */
    virtual sal_uInt16  GetRotation() const = 0;
};

// ----------------------------------------------------------------------------

/** Represents the CHTEXT record group containing text object properties.

    The CHTEXT group consists of: CHTEXT, CHBEGIN, CHFRAMEPOS, CHFONT,
    CHFORMATRUNS, CHSOURCELINK, CHSTRING, CHFRAME group, CHOBJECTLINK, and CHEND.
 */
class XclImpChText : public XclImpChGroupBase, public XclImpChFontBase, protected XclImpChRoot
{
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

    /** Updates missing parts of this text object from the passed object. */
    void                UpdateText( const XclImpChText* pParentText );
    /** Updates display type of this data point label text object. */
    void                UpdateDataLabel( bool bCateg, bool bValue, bool bPercent );

    /** Returns the target object this text is linked to. */
    inline sal_uInt16   GetLinkTarget() const { return maObjLink.mnTarget; }
    /** Returns the position of the data point label this text is linked to. */
    inline const XclChDataPointPos& GetPointPos() const { return maObjLink.maPointPos; }
    /** Returns true, if this text group contains string data. */
    inline bool         HasString() const { return mxString.is() && mxString->HasString(); }

    /** Converts and writes the contained font settings to the passed property set. */
    void                ConvertFont( ScfPropertySet& rPropSet ) const;
    /** Converts and writes the contained frame data to the passed property set. */
    void                ConvertFrame( ScfPropertySet& rPropSet ) const;
    /** Converts and writes text rotation settings to the passed property set. */
    void                ConvertRotation( ScfPropertySet& rPropSet ) const;
    /** Converts and writes the contained string to the passed property set. */
    void                ConvertString( ScfPropertySet& rPropSet ) const;
    /** Converts and writes all contained data to the passed data point label property set. */
    void                ConvertDataLabel( ScfPropertySet& rPropSet, sal_uInt16 nTypeId ) const;
    /** Converts and writes all contained data to the passed title property set. */
    void                ConvertTitle( ScfPropertySet& rPropSet ) const;

private:
    XclChText           maData;             /// Contents of the CHTEXT record.
    XclChObjectLink     maObjLink;          /// Link target for this text object.
    XclImpChFrameRef    mxFrame;            /// Text object frame properties (CHFRAME group).
    XclImpChFontRef     mxFont;             /// Index into font buffer (CHFONT record).
    XclImpChStringRef   mxString;           /// Text data (CHSTRING, CHFORMATRUNS records).
};

typedef ScfRef< XclImpChText > XclImpChTextRef;

// Linked source data =========================================================

/** Represents the main position of a source range.
    @descr  The main position always contains the sheet index. For vertical
        ranges it will contain the column index, and for horizontal ranges the
        row index. */
struct XclImpChMainPos
{
    SCTAB               mnScTab;            /// Sheet index of source ranges.
    SCCOLROW            mnScPos;            /// Main position of source ranges (column/row).

    inline explicit     XclImpChMainPos() : mnScTab( -1 ), mnScPos( -1 ) {}
    inline explicit     XclImpChMainPos( SCTAB nScTab, SCCOLROW nScPos ) :
                            mnScTab( nScTab ), mnScPos( nScPos ) {}
    inline bool         operator==( const XclImpChMainPos& rCmp ) const
                            { return (mnScTab == rCmp.mnScTab) && (mnScPos == rCmp.mnScPos); }
    inline bool         operator<( const XclImpChMainPos& rCmp ) const
                            { return (mnScTab < rCmp.mnScTab) || ((mnScTab == rCmp.mnScTab) && (mnScPos < rCmp.mnScPos)); }
};

/** Represents a range of Calc rows or columns with begin and end position. */
struct XclImpChInterval
{
    SCCOLROW            mnScPos1;           /// First column/row of the interval.
    SCCOLROW            mnScPos2;           /// Last column/row of the interval.

    inline explicit     XclImpChInterval( SCCOLROW nScPos1, SCCOLROW nScPos2 ) :
                            mnScPos1( nScPos1 ), mnScPos2( nScPos2 ) {}
    inline bool         operator==( const XclImpChInterval& rCmp ) const
                            { return (mnScPos1 == rCmp.mnScPos1) && (mnScPos2 == rCmp.mnScPos2); }
};

// ----------------------------------------------------------------------------

class XclImpChSourceLink : protected XclImpChRoot
{
public:
    explicit            XclImpChSourceLink( const XclImpChRoot& rRoot );

    /** Reads the CHSOURCELINK record (link to source data). */
    void                ReadChSourceLink( XclImpStream& rStrm );
    /** Sets the final orientation for a single-cell source range. */
    void                SetFinalOrientation( XclChOrientation eOrient );
    /** Inserts the title position. Useful for value or category ranges. */
    void                InsertTitlePos( SCCOLROW nScPos );

    /** Returns the destination object (title, values, category, ...). */
    inline sal_uInt8    GetDestType() const { return maData.mnDestType; }
    /** Returns the link type (to worksheet, directly, default, ...). */
    inline sal_uInt8    GetLinkType() const { return maData.mnLinkType; }

    /** Returns the orientation of the linked range. */
    inline XclChOrientation GetOrientation() const { return meOrient; }
    /** Returns true, if the orientation of the linked range is valid. */
    bool                HasValidOrientation() const;
    /** Returns the main position of the source ranges. */
    inline const XclImpChMainPos& GetMainPos() const { return maMainPos; }
    /** Returns the column index of the first range. */
    inline SCCOL        GetFirstScCol() const { return mnScCol; }
    /** Returns the row index of the first range. */
    inline SCROW        GetFirstScRow() const { return mnScRow; }
    /** Returns the number of data points of this source link. */
    inline sal_uInt16   GetCellCount() const { return mnCells; }

    /** Returns true, if the own source links and the passed source links are equal. */
    bool                IsEqualLink( const XclImpChSourceLink& rSrcLink ) const;
    /** Returns true, if the passed link is a valid value range (use only if own link is a value link). */
    bool                IsValidValueLink( const XclImpChSourceLink& rValueLink ) const;
    /** Returns true, if the passed link is a valid category range (use only if own link is a value link). */
    bool                IsValidCategLink( const XclImpChSourceLink& rCategLink ) const;
    /** Returns true, if the passed link is a valid title range (use only if own link is a value link). */
    bool                IsValidTitleLink( const XclImpChSourceLink& rTitleLink ) const;

    /** Inserts (joins) the ranges into the passed Calc range list. */
    void                JoinRanges( ScRangeList& rScRanges ) const;

private:
    /** Calculates the orientation of the source range. */
    void                CalcOrientation( const ScRangeList& rScRanges );
    /** Used while iterating over the range list. Update state with a single cell position. */
    void                AppendSingleCell( SCCOL nScCol, SCROW nScRow );
    /** Used while iterating over the range list. Update state with a column range. */
    void                AppendColumnRange( SCCOL nScCol, SCROW nScRow1, SCROW nScRow2 );
    /** Used while iterating over the range list. Update state with a row range. */
    void                AppendRowRange( SCCOL nScCol1, SCCOL nScCol2, SCROW nScRow );
    /** Changes orientation from single to the passed, and inserts the passed interval. */
    void                UpdateOrientation( XclChOrientation eOrient, SCCOLROW nScPos1, SCCOLROW nScPos2 );
    /** Appends the passed interval to the interval list. */
    void                AppendInterval( SCCOLROW nScPos1, SCCOLROW nScPos2 );

private:
    typedef ::std::vector< XclImpChInterval > XclImpChIntervalVec;

    XclChSourceLink     maData;             /// Contents of the CHSOURCELINK record.
    XclImpChStringRef   mxString;           /// Text data (CHSTRING record).
    XclChOrientation    meOrient;           /// Orientation of source ranges.
    XclImpChMainPos     maMainPos;          /// Main position of the source ranges.
    XclImpChIntervalVec maIntervals;        /// Intervals of source ranges (start/end rows/columns).
    SCCOL               mnScCol;            /// Column index of first source range.
    SCROW               mnScRow;            /// Row index of first source range.
    sal_uInt16          mnCells;            /// Number of cells linked by this source range.
};

typedef ScfRef< XclImpChSourceLink > XclImpChSourceLinkRef;

// ----------------------------------------------------------------------------

class XclImpChAttachedLabel : protected XclImpChRoot
{
public:
    explicit            XclImpChAttachedLabel( const XclImpChRoot& rRoot );
    /** Reads the CHATTACHEDLABEL record (data series/point labels). */
    void                ReadChAttachedLabel( XclImpStream& rStrm );
    /** Creates a CHTEXT group for the label. Clones xParentText and sets additional label settings */
    XclImpChTextRef     CreateDataLabel( XclImpChTextRef xParent ) const;

private:
    sal_uInt16          mnFlags;            /// Additional flags.
};

typedef ScfRef< XclImpChAttachedLabel > XclImpChAttLabelRef;

// ----------------------------------------------------------------------------

class XclImpChChartGroup;

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
    void                SetPointPos( sal_uInt16 nSeriesIdx, sal_uInt16 nPointIdx, sal_uInt16 nFormatIdx );
    /** Sets type and text formatting for a data point label (CHTEXT group). */
    inline void         SetDataLabel( XclImpChTextRef xLabel ) { mxLabel = xLabel; }

    /** Updates default data format for series group. */
    void                UpdateGroupFormat( const XclImpChChartGroup& rChartGroup );
    /** Updates missing series settings from the passed chart group format. */
    void                UpdateSeriesFormat( const XclImpChChartGroup& rChartGroup );
    /** Updates missing data point settings from the passed series format. */
    void                UpdatePointFormat( const XclImpChChartGroup& rChartGroup, const XclImpChDataFormat* pSeriesFmt );

    /** Returns the position of the data point described by this group. */
    inline const XclChDataPointPos& GetPointPos() const { return maData.maPointPos; }
    /** Returns the format index of the data point described by this group. */
    inline sal_uInt16   GetFormatIdx() const { return maData.mnFormatIdx; }
    /** Returns true, if markers are set to automatic format. */
    inline bool         IsAutoMarker() const { return !mxMarkerFmt || mxMarkerFmt->IsAuto(); }
    /** Returns true, if markers are enabled. */
    inline bool         HasMarker() const { return !mxMarkerFmt || mxMarkerFmt->HasMarker(); }
    /** Returns true, if the series line is smoothed. */
    inline bool         HasSpline() const { return mxSeriesFmt.is() && mxSeriesFmt->HasSpline(); }
    /** Returns the data label text object. */
    inline XclImpChTextRef GetDataLabel() const { return mxLabel; }

    /** Converts and writes the contained data to the passed property set. */
    void                Convert( ScfPropertySet& rPropSet, const XclImpChChartGroup& rChartGroup ) const;
    /** Writes the area format for a data point in a series with automatic point colors. */
    void                ConvertVarPoint( ScfPropertySet& rPropSet, sal_uInt16 nFormatIdx ) const;
    /** Writes the line format only, e.g. for trend lines or error bars. */
    void                ConvertLine( ScfPropertySet& rPropSet, sal_uInt16 nAutoWeight ) const;

private:
    /** Removes unused formatting (e.g. pie distance in a bar chart). */
    void                RemoveUnusedFormats( const XclImpChChartGroup& rChartGroup );
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

typedef ScfRef< XclImpChDataFormat > XclImpChDataFormatRef;

// ----------------------------------------------------------------------------

/** Represents the CHSERTRENDLINE record containing settings for a trend line. */
class XclImpChSerTrendLine : protected XclImpChRoot
{
public:
    explicit            XclImpChSerTrendLine( const XclImpChRoot& rRoot );

    /** Reads the CHSERTRENDLINE record. */
    void                ReadChSerTrendLine( XclImpStream& rStrm );
    /** Sets formatting information for the error bars. */
    inline void         SetLineFormat( XclImpChDataFormatRef xLineFmt ) { mxLineFmt = xLineFmt; }

    /** Returns true, if the type of the trend line is supported in OOChart. */
    bool                IsValidType() const;

    /** Converts and writes the contained data to the passed property set. */
    void                Convert( ScfPropertySet& rPropSet ) const;

private:
    XclChSerTrendLine   maData;             /// Contents of the CHSERTRENDLINE record.
    XclImpChDataFormatRef mxLineFmt;        /// Formatting settings of the trend line.
};

typedef ScfRef< XclImpChSerTrendLine > XclImpChSerTrendLineRef;

// ----------------------------------------------------------------------------

/** Represents the CHSERERRORBAR record containing settings for error bars. */
class XclImpChSerErrorBar : protected XclImpChRoot
{
public:
    explicit            XclImpChSerErrorBar( const XclImpChRoot& rRoot );

    /** Reads the CHSERERRORBAR record. */
    void                ReadChSerErrorBar( XclImpStream& rStrm );
    /** Sets formatting information for the error bars. */
    inline void         SetBarFormat( XclImpChDataFormatRef xBarFmt ) { mxBarFmt = xBarFmt; }
    /** Adds the passed error bar settings to the own settings. */
    void                AddErrorBar( const XclImpChSerErrorBar& rErrorBar );

    /** Returns true, if this object describes X error bars. */
    bool                IsXErrorBar() const;
    /** Returns true, if this object describes Y error bars. */
    bool                IsYErrorBar() const;

    /** Converts and writes the contained data to the passed property set. */
    void                Convert( ScfPropertySet& rPropSet ) const;

private:
    XclChSerErrorBar    maData;             /// Contents of the CHSERERRORBAR record.
    XclImpChDataFormatRef mxBarFmt;         /// Formatting settings of the error bars.
};

typedef ScfRef< XclImpChSerErrorBar > XclImpChSerErrorBarRef;

// ----------------------------------------------------------------------------

/** Represents the CHSERIES record group describing a data series in a chart.

    The CHSERIES group consists of: CHSERIES, CHBEGIN, CHSOURCELINK groups,
    CHDATAFORMAT groups, CHSERGROUP, CHSERPARENT, CHSERERRORBAR,
    CHSERTRENDLINE, CHEND.
 */
class XclImpChSeries : public XclImpChGroupBase, protected XclImpChRoot
{
public:
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::chart::XChartDocument > XChartDocRef;

public:
    explicit            XclImpChSeries( const XclImpChRoot& rRoot, sal_uInt16 nSeriesIdx );

    /** Reads the CHSERIES record (called by base class). */
    virtual void        ReadHeaderRecord( XclImpStream& rStrm );
    /** Reads a record from the CHSERIES group (called by base class). */
    virtual void        ReadSubRecord( XclImpStream& rStrm );

    /** If own orientation is single-cell, tries to update it from the passed series.
    @return  True, if the passed series has compatible value ranges. */
    bool                CheckAndUpdateOrientation( const XclImpChSeries& rSeries );
    /** Sets the final orientation of the chart. Removes invalid titles and categories. */
    void                SetFinalOrientation( XclChOrientation eOrient );
    /** Tries to get the categories from the passed series, if they are valid. */
    void                UpdateCategories( const XclImpChSeries& rSeries );
    /** Removes own title, if the title of the passed series does not match. */
    void                UpdateTitle( const XclImpChSeries& rSeries );
    /** Adds the title cell to the value ranges. */
    void                AddTitleToValues();

    /** Sets a data point or series format (CHDATAFORMAT group) for this series. */
    void                SetDataFormat( XclImpChDataFormatRef xDataFmt );
    /** Sets a label text (CHTEXT group) attached  to a series or data point. */
    void                SetDataLabel( XclImpChTextRef xLabel );
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
    inline sal_uInt16   GetFormatIdx() const { return mxSeriesFmt.is() ? mxSeriesFmt->GetFormatIdx() : EXC_CHDATAFORMAT_DEFAULT; }
    /** Returns the number of data points of this series. */
    inline sal_uInt16   GetPointCount() const { return mbHasValue ? mxValueLink->GetCellCount() : 0; }
    /** Returns the record identifier of the chart type this series is attached to. */
    sal_uInt16          GetChartTypeId() const;

    /** Returns the orientation of the linked range. */
    inline XclChOrientation GetOrientation() const { return meOrient; }
    /** Returns true, if the series is child of another series (e.g. trend line). */
    inline bool         HasParentSeries() const { return mnParentIdx != EXC_CHSERIES_INVALID; }
    /** Returns true, if the series has a valid source link for the values. */
    inline bool         HasValidValues() const { return mbHasValue; }
    /** Returns true, if the series has a valid source link for the categories. */
    inline bool         HasValidCategories() const { return mbHasCateg; }
    /** Returns true, if the series has a valid title. */
    inline bool         HasValidTitle() const { return mbHasTitle; }
    /** Returns true, if the own categories and the categories of the passed series are equal. */
    bool                HasEqualCategories( const XclImpChSeries& rSeries ) const;
    /** Returns the main position of the value source ranges. */
    const XclImpChMainPos& GetMainPos() const;

    /** Returns true, if the line style of the series is visible. */
    inline bool         HasLine() const { return !mxSeriesFmt || mxSeriesFmt->HasLine(); }
    /** Returns true, if the series line is smoothed. */
    inline bool         HasSpline() const { return mxSeriesFmt.is() && mxSeriesFmt->HasSpline(); }
    /** Returns true, if markers are enabled for at least one data point. */
    bool                HasMarker() const;

    /** Inserts (joins) the value ranges into the passed Calc range list. */
    void                JoinValueRanges( ScRangeList& rScRanges ) const;
    /** Inserts (joins) the category ranges into the passed Calc range list. */
    void                JoinCategoryRanges( ScRangeList& rScRanges ) const;

    /** Converts and writes the contained series formatting to the passed chart.
        @param nSeriesIdx  0-based series index used by the chart. */
    void                Convert( XChartDocRef xChartDoc, size_t nSeriesIdx ) const;

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
    /** Finalizes the series after reading the CHSERIES group. */
    void                ReadChEnd( XclImpStream& rStrm );

    /** Calculates the orientation of the series source ranges. */
    void                CalcOrientation();

    /** Returns the chart type that contains this series. */
    XclImpChChartGroup* GetChartGroup() const;

    /** Creates a new CHDATAFORMAT group with the specified point index. */
    XclImpChDataFormatRef CreateDataFormat( sal_uInt16 nPointIdx, sal_uInt16 nFormatIdx );
    /** Returns the pointer to a CHDATAFORMAT group reference or 0 for invalid pointer index. */
    XclImpChDataFormatRef* GetDataFormatRef( sal_uInt16 nPointIdx );
    /** Returns the pointer to a CHTEXT group reference or 0 for invalid pointer index. */
    XclImpChTextRef*    GetDataLabelRef( sal_uInt16 nPointIdx );

private:
    typedef XclImpChMap< sal_uInt16, XclImpChDataFormat >   XclImpChDataFormatMap;
    typedef XclImpChMap< sal_uInt16, XclImpChText >         XclImpChTextMap;

    XclChSeries         maData;             /// Contents of the CHSERIES record.
    XclImpChSourceLinkRef mxValueLink;      /// Link data for series values.
    XclImpChSourceLinkRef mxCategLink;      /// Link data for series category names.
    XclImpChSourceLinkRef mxTitleLink;      /// Link data for series title.
    XclImpChSourceLinkRef mxBubbleLink;     /// Link data for series bubble sizes.
    XclImpChDataFormatRef mxSeriesFmt;      /// CHDATAFORMAT group for series format.
    XclImpChDataFormatMap maPointFmts;      /// CHDATAFORMAT groups for data point formats.
    XclImpChTextMap     maLabels;           /// All text labels for series and points (CHTEXT groups).
    XclImpChSerTrendLineRef mxTrendLine;    /// Settings for a trend line (CHSERTRENDLINE record).
    XclImpChSerErrorBarRef mxXErrorBar;     /// Settings for X error bar (CHSERERRORBAR record).
    XclImpChSerErrorBarRef mxYErrorBar;     /// Settings for Y error bar (CHSERERRORBAR record).
    XclChOrientation    meOrient;           /// Orientation of the series.
    sal_uInt16          mnGroupIdx;         /// Chart group (CHCHARTGROUP group) this series is assigned to.
    sal_uInt16          mnSeriesIdx;        /// 0-based series index.
    sal_uInt16          mnParentIdx;        /// 0-based index of parent series (trend lines and error bars).
    bool                mbHasValue;         /// true = Has valid series values.
    bool                mbHasCateg;         /// true = Has valid category range.
    bool                mbHasTitle;         /// true = Has valid series title.
};

typedef ScfRef< XclImpChSeries > XclImpChSeriesRef;

// Chart structure ============================================================

class XclImpChType : protected XclImpChRoot
{
public:
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::chart::XChartDocument > XChartDocRef;

public:
    explicit            XclImpChType( const XclImpChRoot& rRoot );

    /** Reads a chart type record (e.g. CHBAR, CHLINE, CHPIE, ...). */
    void                ReadChType( XclImpStream& rStrm );
    /** Sets the chart group index. */
    inline void         SetGroupIndex( sal_uInt16 nGroupIdx ) { mnGroupIdx = nGroupIdx; }
    /** Final processing after reading the entire chart. */
    void                Finalize();

    /** Returns the record identifier of the chart type record. */
    inline sal_uInt16   GetTypeId() const { return mnTypeId; }
    /** Returns true, if the series in this chart group are stacked on each other. */
    bool                IsStackedChart() const;
    /** Returns true, if the series in this chart group are stacked on each other as percentage. */
    bool                IsPercentChart() const;

    /** Creates the diagram with the correct type in the passed chart. */
    void                CreateDiagram( XChartDocRef xChartDoc ) const;
    /** Converts and writes the contained data to the passed chart. */
    void                Convert( XChartDocRef xChartDoc ) const;

private:
    XclChType           maData;             /// Contents of the chart type record.
    sal_uInt16          mnTypeId;           /// Record identifier for chart type.
    sal_uInt16          mnGroupIdx;         /// Chart group index.
};

// ----------------------------------------------------------------------------

class XclImpChChart3d
{
public:
    /** Reads the CHCHART3D record (properties for 3D charts). */
    void                ReadChChart3d( XclImpStream& rStrm );
    /** Returns true, if the data points are clustered on the X axis. */
    inline bool         IsClustered() const { return ::get_flag( maData.mnFlags, EXC_CHCHART3D_CLUSTER ); }
    /** Converts and writes the contained data to the passed property set. */
    void                Convert( ScfPropertySet& rPropSet ) const;

private:
    XclChChart3d        maData;             /// Contents of the CHCHART3D record.
};

typedef ScfRef< XclImpChChart3d > XclImpChChart3dRef;

// ----------------------------------------------------------------------------

/** Represents the CHLEGEND record group describing the chart legend.

    The CHLEGEND group consists of: CHLEGEND, CHBEGIN, CHFRAME group,
    CHTEXT group, CHEND.
 */
class XclImpChLegend : public XclImpChGroupBase, protected XclImpChRoot
{
public:
    explicit            XclImpChLegend( const XclImpChRoot& rRoot );

    /** Reads the CHLEGEND record (called by base class). */
    virtual void        ReadHeaderRecord( XclImpStream& rStrm );
    /** Reads a record from the CHLEGEND group (called by base class). */
    virtual void        ReadSubRecord( XclImpStream& rStrm );
    /** Final processing after reading the entire chart. */
    void                Finalize();

    /** Converts and writes legend properties to the passed property set. */
    void                Convert( ScfPropertySet& rPropSet ) const;

private:
    XclChLegend         maData;             /// Contents of the CHLEGEND record.
    XclImpChTextRef     mxText;             /// Legend text format (CHTEXT group).
    XclImpChFrameRef    mxFrame;            /// Legend frame format (CHFRAME group).
};

typedef ScfRef< XclImpChLegend > XclImpChLegendRef;

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
    sal_uInt16          mnBarDist;          /// Distance between bars (CHDROPBAR record).
};

typedef ScfRef< XclImpChDropBar > XclImpChDropBarRef;

// ----------------------------------------------------------------------------

/** Represents the CHCHARTGROUP record group describing formats of an axes set.

    The CHCHARTGROUP group consists of: CHCHARTGROUP, CHBEGIN, a chart type
    record (e.g. CHBAR, CHLINE, CHAREA, CHPIE, ...), CHCHART3D, CHLEGEND group,
    CHDROPBAR groups, CHCHARTLINE groups (CHCHARTLINE with CHLINEFORMAT),
    CHDATAFORMAT group, CHEND.
 */
class XclImpChChartGroup : public XclImpChGroupBase, protected XclImpChRoot
{
public:
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::chart::XChartDocument > XChartDocRef;

public:
    explicit            XclImpChChartGroup( const XclImpChRoot& rRoot );

    /** Reads the CHCHARTGROUP record (called by base class). */
    virtual void        ReadHeaderRecord( XclImpStream& rStrm );
    /** Reads a record from the CHCHARTGROUP group (called by base class). */
    virtual void        ReadSubRecord( XclImpStream& rStrm );
    /** Final processing after reading the entire chart. */
    void                Finalize();

    /** Marks the passed format index as used. PopUnusedFormatIndex() will not return this index. */
    void                SetUsedFormatIndex( sal_uInt16 nFormatIdx );
    /** Returns the next unused format index and marks it as used. */
    sal_uInt16          PopUnusedFormatIndex();

    /** Returns the chart group index of this chart group format. */
    inline sal_uInt16   GetGroupIdx() const { return maData.mnGroupIdx; }
    /** Returns the record identifier of the chart type record. */
    inline sal_uInt16   GetChartTypeId() const { return maType.GetTypeId(); }
    /** Returns true, if the series in this chart group are stacked on each other. */
    inline bool         IsStackedChart() const { return maType.IsStackedChart(); }
    /** Returns true, if the series in this chart group are stacked on each other as percentage. */
    inline bool         IsPercentChart() const { return maType.IsPercentChart(); }
    /** Returns true, if this chart group is a linear 2d chart type. */
    inline bool         IsLinear2dChart() const { return !Is3dChart() && XclChartHelper::HasLinearSeries( GetChartTypeId() ); }
    /** Returns true, if the chart is three-dimensional. */
    inline bool         Is3dChart() const { return mxChart3d.is(); }
    /** Returns true, if the series in this chart group are ordered on the Z axis. */
    inline bool         IsDeep3dChart() const { return mxChart3d.is() && !mxChart3d->IsClustered(); }
    /** Returns true, if the chart group contains a hi-lo line format. */
    inline bool         HasHiLoLine() const { return maChartLines.has( EXC_CHCHARTLINE_HILO ); }
    /** Returns true, if the chart group contains drop bar formats. */
    inline bool         HasDropBars() const { return !maDropBars.empty(); }
    /** Looks for a legend in all chart groups and returns it. */
    inline XclImpChLegendRef GetLegend() const { return mxLegend; }

    /** Returns true, if points of a series have varying automatic area format (only for a single series in this group). */
    inline bool         IsVarPointFormat() const { return ::get_flag( maData.mnFlags, EXC_CHCHARTGROUP_VARIED ); }
    /** Returns the default series data format. */
    inline XclImpChDataFormatRef GetGroupFormat() const { return mxGroupFmt; }

    /** Creates the diagram with the correct type in the passed chart. */
    void                CreateDiagram( XChartDocRef xChartDoc ) const;
    /** Converts and writes all remaining formatting to the passed chart. */
    void                Convert( XChartDocRef xChartDoc ) const;

private:
    /** Reads a CHDROPBAR record group. */
    void                ReadChDropBar( XclImpStream& rStrm );
    /** Reads a CHCHARTLINE record group. */
    void                ReadChChartLine( XclImpStream& rStrm );
    /** Reads a CHDATAFORMAT record group (default series format). */
    void                ReadChDataFormat( XclImpStream& rStrm );

    /** Converts and writes chart type dependent settings to the passed chart. */
    void                ConvertTypeSettings( XChartDocRef xChartDoc ) const;

private:
    typedef XclImpChMap< sal_uInt16, XclImpChDropBar >      XclImpChDropBarMap;
    typedef XclImpChMap< sal_uInt16, XclImpChLineFormat >   XclImpChLineFormatMap;
    typedef ::std::set< sal_uInt16 >                        UInt16Set;

    XclChChartGroup     maData;             /// Contents of the CHCHARTGROUP record.
    XclImpChType        maType;             /// Chart type (e.g. CHBAR, CHLINE, ...).
    XclImpChChart3dRef  mxChart3d;          /// 3D settings (CHCHART3D record).
    XclImpChLegendRef   mxLegend;           /// Chart legend (CHLEGEND group).
    XclImpChDropBarMap  maDropBars;         /// Dropbars (CHDROPBAR group).
    XclImpChLineFormatMap maChartLines;     /// Global line formats (CHCHARTLINE group).
    XclImpChDataFormatRef mxGroupFmt;       /// Default format for all series (CHDATAFORMAT group).
    UInt16Set           maUnusedFormats;    /// Contains unused format indexes for automatic colors.
};

typedef ScfRef< XclImpChChartGroup > XclImpChChartGroupRef;

// Axes =======================================================================

class XclImpChLabelRange : protected XclImpChRoot
{
public:
    explicit            XclImpChLabelRange( const XclImpChRoot& rRoot );
    /** Reads the CHLABELRANGE record (category axis scaling properties). */
    void                ReadChLabelRange( XclImpStream& rStrm );
    /** Converts and writes the contained data to the passed property set. */
    void                Convert( ScfPropertySet& rPropSet ) const;

private:
    XclChLabelRange     maData;             /// Contents of the CHLABELRANGE record.
};

typedef ScfRef< XclImpChLabelRange > XclImpChLabelRangeRef;

// ----------------------------------------------------------------------------

class XclImpChValueRange : protected XclImpChRoot
{
public:
    explicit            XclImpChValueRange( const XclImpChRoot& rRoot );
    /** Reads the CHVALUERANGE record (numeric axis scaling properties). */
    void                ReadChValueRange( XclImpStream& rStrm );
    /** Converts and writes the contained data to the passed property set. */
    void                Convert( ScfPropertySet& rPropSet ) const;

private:
    XclChValueRange     maData;             /// Contents of the CHVALUERANGE record.
};

typedef ScfRef< XclImpChValueRange > XclImpChValueRangeRef;

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
    /** Converts and writes the contained font color to the passed property set. */
    void                ConvertFontColor( ScfPropertySet& rPropSet ) const;

private:
    XclChTick           maData;             /// Contents of the CHTICK record.
};

typedef ScfRef< XclImpChTick > XclImpChTickRef;

// ----------------------------------------------------------------------------

/** Represents the CHAXIS record group describing an entire chart axis.

    The CHAXIS group consists of: CHAXIS, CHBEGIN, CHLABELRANGE, CHEXTRANGE,
    CHVALUERANGE, CHFORMAT, CHTICK, CHFONT, CHAXISLINE groups (CHAXISLINE with
    CHLINEFORMAT, CHAREAFORMAT, and CHESCHERFORMAT group), CHEND.
 */
class XclImpChAxis : public XclImpChGroupBase, public XclImpChFontBase, public XclImpChRotationBase, protected XclImpChRoot
{
public:
    explicit            XclImpChAxis( const XclImpChRoot& rRoot, sal_uInt16 nAxesSetId );

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
    /** Returns true, if the axis is active. */
    inline bool         IsAlive() const { return !mxAxisLine || mxAxisLine->IsShowAxis(); }
    /** Returns true, if the axis contains caption labels. */
    inline bool         HasLabels() const { return !mxTick || mxTick->HasLabels(); }
    /** Returns true, if the axis shows its major grid lines. */
    inline bool         HasMajorGrid() const { return mxMajorGrid.is(); }
    /** Returns true, if the axis shows its minor grid lines. */
    inline bool         HasMinorGrid() const { return mxMinorGrid.is(); }

    /** Converts and writes axis and grid properties to the passed property sets. */
    void                Convert( ScfPropertySet& rAxisProp, ScfPropertySet& rMajorProp, ScfPropertySet& rMinorProp ) const;
    /** Converts and writes wall/floor properties to the passed property set. */
    void                ConvertWall( ScfPropertySet& rPropSet ) const;

private:
    /** Reads a CHAXISLINE record specifying the target for following line properties. */
    void                ReadChAxisLine( XclImpStream& rStrm );

private:
    XclChAxis           maData;             /// Contents of the CHAXIS record.
    XclImpChLabelRangeRef mxLabelRange;     /// X axis scaling (CHLABELRANGE record).
    XclImpChValueRangeRef mxValueRange;     /// Y axis scaling (CHVALUERANGE record).
    XclImpChFormatRef   mxFormat;           /// Index to number format (CHFORMAT record).
    XclImpChTickRef     mxTick;             /// Axis ticks (CHTICK record).
    XclImpChFontRef     mxFont;             /// Index into font buffer (CHFONT record).
    XclImpChLineFormatRef mxAxisLine;       /// Axis line format (CHLINEFORMAT record).
    XclImpChLineFormatRef mxMajorGrid;      /// Major grid line format (CHLINEFORMAT record).
    XclImpChLineFormatRef mxMinorGrid;      /// Minor grid line format (CHLINEFORMAT record).
    XclImpChFrameRef    mxWallFrame;        /// Wall/floor format (sub records of CHFRAME group).
    sal_uInt16          mnAxesSetId;        /// Axes set identifier (primary/secondary).
};

typedef ScfRef< XclImpChAxis > XclImpChAxisRef;

// ----------------------------------------------------------------------------

/** A helper struct containing the correct property sets for an axis. */
struct XclImpChAxisHelper
{
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::chart::XDiagram > XDiagramRef;

    XclImpChAxisRef     mxAxis;             /// The axis (CHAXIS group).
    XclImpChTextRef     mxTitle;            /// The axis title (CHTEXT group).
    ScfPropertySet      maDiaProp;          /// Diagram properties.
    ScfPropertySet      maAxisProp;         /// Axis properties.
    ScfPropertySet      maTitleProp;        /// Axis title properties.
    ScfPropertySet      maMajorProp;        /// Major grid line properties.
    ScfPropertySet      maMinorProp;        /// Major grid line properties.
    ::rtl::OUString     maNameHasAxis;      /// Property name for axis on/off.
    ::rtl::OUString     maNameHasLabels;    /// Property name for axis labels on/off.
    ::rtl::OUString     maNameHasMajor;     /// Property name for major grid on/off.
    ::rtl::OUString     maNameHasMinor;     /// Property name for minor grid on/off.

    explicit            XclImpChAxisHelper(
                            XclImpChAxisRef xAxis,
                            XclImpChTextRef xTitle,
                            XDiagramRef xDiagram );

    void                Convert( const XclImpChRoot& rRoot );
};

// ----------------------------------------------------------------------------

/** Represents the CHAXESSET record group describing an axes set (X/Y/Z axes).

    The CHAXESSET group consists of: CHAXESSET, CHBEGIN, CHFRAMEPOS, CHAXIS
    groups, CHTEXT groups, CHPLOTFRAME group (CHPLOTFRAME with CHFRAME group),
    CHCHARTGROUP group, CHEND.
 */
class XclImpChAxesSet : public XclImpChGroupBase, protected XclImpChRoot
{
public:
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::chart::XChartDocument > XChartDocRef;
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::chart::XDiagram >       XDiagramRef;

public:
    explicit            XclImpChAxesSet( const XclImpChRoot& rRoot, sal_uInt16 nAxesSetId );

    /** Reads the CHAXESSET record (called by base class). */
    virtual void        ReadHeaderRecord( XclImpStream& rStrm );
    /** Reads a record from the CHAXESSET group (called by base class). */
    virtual void        ReadSubRecord( XclImpStream& rStrm );
    /** Final processing after reading the entire chart. */
    void                Finalize();

    /** Returns true, if this axes set exists (returns false if this is a dummy object). */
    inline bool         IsAlive() const { return mbAlive; }
    /** Returns the index of the axes set (primary/secondary). */
    inline sal_uInt16   GetAxesSetId() const { return maData.mnAxesSetId; }

    /** Returns true, if the axes set contains the specified chart group. */
    inline bool         HasChartGroup( sal_uInt16 nGroupIdx ) const { return maChartGroups.has( nGroupIdx ); }
    /** Returns the specified chart format group. */
    inline XclImpChChartGroupRef GetChartGroup( sal_uInt16 nGroupIdx ) const { return maChartGroups.get( nGroupIdx ); }
    /** Returns the default chart format group of this axes set. */
    inline XclImpChChartGroup& GetDefChartGroup() const { return *maChartGroups.begin()->second; }

    /** Looks for a legend in all chart groups and returns it. */
    XclImpChLegendRef   GetLegend() const;

    /** Creates the diagram with the correct type in the passed chart. */
    void                CreateDiagram( XChartDocRef xChartDoc ) const;
    /** Converts and writes all axes formatting to the passed chart. */
    void                Convert( XChartDocRef xChartDoc ) const;

private:
    /** Reads a CHAXIS record group containing a single axis. */
    void                ReadChAxis( XclImpStream& rStrm );
    /** Reads a CHTEXT record group containing an axis title. */
    void                ReadChText( XclImpStream& rStrm );
    /** Reads the CHPLOTFRAME record group containing diagram area formatting. */
    void                ReadChPlotFrame( XclImpStream& rStrm );
    /** Reads a CHCHARTGROUP record group containing chart type and chart settings. */
    void                ReadChChartGroup( XclImpStream& rStrm );

    /** Updates text formatting of the passed axis title with global text formatting. */
    void                UpdateAxisTitle( XclImpChTextRef xTitle );

    /** Writes all properties of the primary X axis to the passed diagram. */
    void                ConvertXAxis( XDiagramRef xDiagram ) const;
    /** Writes all properties of the primary Y axis to the passed diagram. */
    void                ConvertYAxis( XDiagramRef xDiagram ) const;
    /** Writes all properties of the secondary Y axis to the passed diagram. */
    void                ConvertSecYAxis( XDiagramRef xDiagram ) const;
    /** Writes all properties of the primary Z axis to the passed diagram. */
    void                ConvertZAxis( XDiagramRef xDiagram ) const;
    /** Writes all properties of the background area to the passed diagram. */
    void                ConvertBackground( XDiagramRef xDiagram ) const;

private:
    typedef XclImpChMap< sal_uInt16, XclImpChChartGroup > XclImpChChartGroupMap;

    XclChAxesSet        maData;             /// Contents of the CHAXESSET record.
    XclImpChFramePosRef mxPos;              /// Position of the axes set (CHFRAMEPOS record).
    XclImpChAxisRef     mxXAxis;            /// The X axis (CHAXIS group).
    XclImpChAxisRef     mxYAxis;            /// The Y axis (CHAXIS group).
    XclImpChAxisRef     mxZAxis;            /// The Z axis (CHAXIS group).
    XclImpChTextRef     mxXAxisTitle;       /// The X axis title (CHTEXT group).
    XclImpChTextRef     mxYAxisTitle;       /// The Y axis title (CHTEXT group).
    XclImpChTextRef     mxZAxisTitle;       /// The Z axis title (CHTEXT group).
    XclImpChFrameRef    mxPlotFrame;        /// Plot area (CHPLOTFRAME group).
    XclImpChChartGroupMap maChartGroups;    /// Chart group (chart type) (CHCHARTGROUP group).
    bool                mbAlive;            /// true = Axes set exists; false = dummy object.
};

typedef ScfRef< XclImpChAxesSet > XclImpChAxesSetRef;

// The chart object ===========================================================

/** Represents the CHCHART record group describing the chart contents.

    The CHCHART group consists of: CHCHART, CHBEGIN, SCL, CHPLOTGROWTH, CHFRAME
    group, CHSERIES groups, CHPROPERTIES, CHDEFAULTTEXT groups (CHDEFAULTTEXT
    with CHTEXT groups), CHUSEDAXESSETS, CHAXESSET groups, CHTEXT groups, CHEND.
 */
class XclImpChChart : public XclImpChGroupBase, protected XclImpChRoot
{
public:
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::chart::XChartDocument > XChartDocRef;

public:
    explicit            XclImpChChart( const XclImpRoot& rRoot );
    virtual             ~XclImpChChart();

    /** Reads the CHCHART record (called by base class). */
    virtual void        ReadHeaderRecord( XclImpStream& rStrm );
    /** Reads a record from the CHCHART group (called by base class). */
    virtual void        ReadSubRecord( XclImpStream& rStrm );
    /** Reads a CHDATAFORMAT group describing a series format or a data point format. */
    void                ReadChDataFormat( XclImpStream& rStrm );

    /** Checks validity of scatter X value ranges (must be equal for all series).
        @return  true = chart contains valid X value ranges for all series. */
    bool                UpdateScatterXRanges();

    /** Returns true, if the series have valid source links for the categories. */
    inline bool         HasValidCategories() const { return mbHasCateg; }
    /** Returns true, if the series have valid source links for titles. */
    inline bool         HasValidTitles() const { return mbHasTitle; }

    /** Returns a range list with all source ranges of the chart. */
    ScRangeListRef      GetSourceData() const;
    /** returns true, if the source range contains a header column. */
    bool                HasHeaderColumn() const;
    /** returns true, if the source range contains a header row. */
    bool                HasHeaderRow() const;

    /** Returns the final chart type identifier. */
    inline sal_uInt16   GetChartTypeId() const { return mxPrimAxesSet->GetDefChartGroup().GetChartTypeId(); }
    /** Returns true, if the chart is a three-dimensional chart. */
    bool                Is3dChart() const;
    /** Returns true, the chart is a stock chart. */
    bool                IsStockChart() const;

    /** Returns true, if the specified axes set is alive. */
    bool                HasAxesSet( sal_uInt16 nAxesSetId ) const;

    /** Returns the axes set that contains the passed chart group. */
    XclImpChAxesSet&    GetChartGroupAxesSet( sal_uInt16 nGroupIdx ) const;
    /** Returns the axes set identifier of the passed chart group. */
    sal_uInt16          GetChartGroupAxesSetId( sal_uInt16 nGroupIdx ) const;

    /** Returns the specified chart format group. */
    XclImpChChartGroupRef GetChartGroup( sal_uInt16 nGroupIdx ) const;
    /** Returns the default chart format group from one of the axes sets. */
    XclImpChChartGroup& GetDefChartGroup( sal_uInt16 nGroupIdx ) const;
    /** Returns the number of series for the specified chart format group. */
    sal_uInt16          GetChartGroupSeriesCount( sal_uInt16 nGroupIdx ) const;

    /** Returns the specified default text. */
    XclImpChTextRef     GetDefaultText( sal_uInt16 nTextId ) const;

    /** Returns true, if at least one series contains a visible line format. */
    bool                HasAnySeriesLine() const;
    /** Returns the number of units on the progress bar needed for the chart. */
    inline sal_Size     GetProgressSize() const { return 2 * EXC_CHART_PROGRESS_SIZE; }

    /** Converts and writes all properties to the passed chart. */
    void                Convert( XChartDocRef xChartDoc, ScfProgressBar& rProgress ) const;

private:
    /** Reads a CHSERIES group (data series source and formatting). */
    void                ReadChSeries( XclImpStream& rStrm );
    /** Reads a CHDEFAULTTEXT group (default text formats). */
    void                ReadChDefaultText( XclImpStream& rStrm );
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

    /** Converts and writes all series and data point formatting to the passed chart. */
    void                ConvertSeries( XChartDocRef xChartDoc ) const;
    /** Converts and writes the order of the visible series to the passed chart. */
    void                ConvertSeriesOrder( XChartDocRef xChartDoc ) const;

private:
    typedef ::std::vector< XclImpChSeriesRef >                      XclImpChSeriesVec;
    typedef XclImpChMap< XclChDataPointPos, XclImpChDataFormat >    XclImpChDataFormatMap;
    typedef XclImpChMap< sal_uInt16, XclImpChText >                 XclImpChTextMap;

    XclChRectangle      maRect;             /// Position of the chart on the sheet (CHCHART record).
    XclImpChSeriesVec   maSeries;           /// List of series data (CHSERIES groups).
    XclImpChSeriesVec   maValidSeries;      /// All valid series (in original order).
    XclImpChDataFormatMap maDataFmts;       /// All series and point formats (CHDATAFORMAT groups).
    XclImpChFrameRef    mxFrame;            /// Chart frame format (CHFRAME group).
    XclImpChTextRef     mxTitle;            /// Chart title (CHTEXT group).
    XclChProperties     maProps;            /// Chart properties (CHPROPERTIES record).
    XclImpChTextMap     maDefTexts;         /// Default text objects (CHDEFAULTTEXT groups).
    XclImpChAxesSetRef  mxPrimAxesSet;      /// Primary axes set (CHAXESSET group).
    XclImpChAxesSetRef  mxSecnAxesSet;      /// Secondary axes set (CHAXESSET group).
    XclImpChLegendRef   mxLegend;           /// Chart legend (CHLEGEND group).
    XclChOrientation    meOrient;           /// Final orientation of the series.
    bool                mbHasValue;         /// true = Has valid series values.
    bool                mbHasCateg;         /// true = Has valid category ranges.
    bool                mbHasTitle;         /// true = Has valid series titles.
};

typedef ScfRef< XclImpChChart > XclImpChChartRef;

// ----------------------------------------------------------------------------

/** THE chart object. Represents the entire chart sub stream (BOF/EOF block). */
class XclImpChart : protected XclImpRoot
{
public:
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > XInterfaceRef;

public:
    /** Constructs a new chart object.
        @param bOwnTab  True = chart is on an own sheet; false = chart is an embedded object. */
    explicit            XclImpChart( const XclImpRoot& rRoot, bool bOwnTab );

    /** Reads the complete chart substream (BOF/EOF block).
        @descr  The passed stream must be located in the BOF record of the chart substream. */
    void                ReadChartSubStream( XclImpStream& rStrm );

    /** Returns a range list with all source ranges of the chart. */
    ScRangeListRef      GetSourceData() const;
    /** returns true, if the source range contains a header column. */
    bool                HasHeaderColumn() const;
    /** returns true, if the source range contains a header row. */
    bool                HasHeaderRow() const;
    /** Returns the number of units on the progress bar needed for the chart. */
    sal_Size            GetProgressSize() const;

    /** Creates the chart object in the pased component. */
    void                Convert( XInterfaceRef xComp, ScfProgressBar& rProgress ) const;

private:
    /** Reads the CHCHART group (entire chart data). */
    void                ReadChChart( XclImpStream& rStrm );

private:
    XclImpChChartRef    mxChartData;        /// The chart data (CHCHART group).
    bool                mbOwnTab;           /// true = own sheet; false = embedded object.
};

// ============================================================================

#endif

