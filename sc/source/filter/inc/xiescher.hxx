/*************************************************************************
 *
 *  $RCSfile: xiescher.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2003-07-24 11:56:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// ============================================================================

#ifndef SC_XIESCHER_HXX
#define SC_XIESCHER_HXX

#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif
#ifndef _MSDFFIMP_HXX
#include <svx/msdffimp.hxx>
#endif

#ifndef SC_XLESCHER_HXX
#include "xlescher.hxx"
#endif
#ifndef SC_XIHELPER_HXX
#include "xihelper.hxx"
#endif

#ifndef _CHRTDEFS_HXX
#include "chrtdefs.hxx"
#endif


class ScfProgressBar;
class XclImpStream;


// Escher stream consumer =====================================================

struct XclImpStreamNode
{
    sal_uInt32                  nPos;
    sal_uInt32                  nSize;
    XclImpStreamNode*           pPrev;
};


// ----------------------------------------------------------------------------

class XclImpStreamConsumer : ScfNoCopy
{
private:
    SvMemoryStream              aStrm;
    DffRecordHeader             aHd;
    XclImpStreamNode*           pNode;
    sal_uInt32                  nBytesLeft;

    void                        UpdateNode( const DffRecordHeader& rHd );
    void                        RemoveNode();

public:
                                XclImpStreamConsumer();
                                ~XclImpStreamConsumer();

    const DffRecordHeader*      ConsumeRecord( XclImpStream& rSrcStrm );
    bool                        AppendData( sal_Char* pBuf, sal_uInt32 nLen );

    inline bool                 HasData() const { return aStrm.Tell() > 0; }
    inline sal_uInt32           Tell() const    { return aStrm.Tell(); }
    inline SvStream&            GetStream()     { return aStrm; }
};


// Escher objects =============================================================

/** Base class for Escher objects. Does not contain any specialized data. */
class XclImpEscherObj : protected XclImpRoot, ScfNoCopy
{
public:
    TYPEINFO();

    /** Constructs a dummy Escher object with an invalid Escher stream position. */
    explicit                    XclImpEscherObj( const XclImpRoot& rRoot );
    /** Constructs a new Escher object at the specified Escher stream position. */
    explicit                    XclImpEscherObj(
                                    const XclImpRoot& rRoot,
                                    sal_uInt32 nStrmBegin, sal_uInt32 nStrmEnd );
    /** Constructor takes ownership of the members of rSrcObj, which will be invalidated. */
    explicit                    XclImpEscherObj( XclImpEscherObj& rSrcObj );

    virtual                     ~XclImpEscherObj();

    /** Returns the SdrObj of this Escher object. */
    inline const SdrObject*     GetSdrObj() const       { return mpSdrObj.get(); }
    /** Returns the position of the object in the draw page. */
    inline const Rectangle&     GetAnchor() const       { return maAnchorRect; }
    /** Returns the start position in the Escher stream of this object. */
    inline sal_uInt32           GetStrmBegin() const    { return mnStrmBegin; }
    /** Returns the end position in the Escher stream of this object (last position + 1). */
    inline sal_uInt32           GetStrmEnd() const      { return mnStrmEnd; }
    /** Returns the Calc sheet index of this object. */
    inline sal_uInt16           GetTab() const          { return mnScTab; }
    /** Returns the Excel object identifier. */
    inline sal_uInt16           GetObjId() const        { return mnObjId; }
    /** Returns true, if this Escher object contains an SdrObj and a valid anchor position. */
    inline bool                 IsValid() const         { return !mbSkip && GetSdrObj() && !GetAnchor().IsEmpty(); }

    /** Sets the position of this object in the draw page. */
    inline void                 SetAnchor( const Rectangle& rRect ) { maAnchorRect = rRect; }
    /** Sets the Excel object identifier (unique per sheet). */
    inline void                 SetObjId( sal_uInt16 nObjId ) { mnObjId = nObjId; }
    /** Marks this Esher object to be skipped on export. */
    inline void                 SetSkip() { mbSkip = true; }

    /** Sets a new SdrObj for this Escher object. This object owns the passed SdrObj. */
    virtual void                SetSdrObj( SdrObject* pSdrObj );
    /** Initializes the progress bar according to the current object type and size.
        @descr  Currently used by charts and OLE objects. */
    virtual void                InitProgress( ScfProgressBar& rProgress );
    /** Inserts the contained SdrObj into the draw page. */
    virtual void                Apply( ScfProgressBar& rProgress );

protected:
    typedef ::std::auto_ptr< SdrObject > SdrObjectPtr;

    Rectangle                   maAnchorRect;   /// Location of the object in the draw page.
    SdrObjectPtr                mpSdrObj;       /// SdrObj representing this Escher object.
    sal_uInt32                  mnStrmBegin;    /// Start position in Escher stream.
    sal_uInt32                  mnStrmEnd;      /// End position in Escher stream (last + 1).
    sal_uInt16                  mnScTab;        /// Calc sheet index of the object.
    sal_uInt16                  mnObjId;        /// The Excel object identifier (from OBJ record).
    bool                        mbSkip;         /// true = Skip creation (ignore this object).
};


// ----------------------------------------------------------------------------

/** A simple drawing object, i.e. line, rectangle, or bitmap. */
class XclImpEscherDrawing : public XclImpEscherObj
{
public:
    TYPEINFO();

    /** Constructor takes ownership of the members of rSrcObj, which will be invalidated. */
    explicit                    XclImpEscherDrawing( XclImpEscherObj& rSrcObj );
};


// ----------------------------------------------------------------------------

/** A common text box object. */
class XclImpEscherTxo : public XclImpEscherDrawing
{
public:
    TYPEINFO();

    /** Constructor takes ownership of the members of rSrcObj, which will be invalidated. */
    explicit                    XclImpEscherTxo( XclImpEscherObj& rSrcObj );

    /** Returns the string data, if there is any. */
    inline const XclImpString*  GetString() const { return mpString.get(); }
    /** Sets a new plain or rich string. This object will own the string. */
    inline void                 SetString( XclImpString* pString ) { mpString.reset( pString ); }

    /** Sets the horizontal and vertical text alignment from the passed Excel TXO flags. */
    void                        SetAlignment( sal_uInt16 nAlign );
    /** Returns the horizontal text alignment. */
    inline XclTxoHorAlign       GetHorAlign() const { return meHorAlign; }
    /** Returns the vertical text alignment. */
    inline XclTxoVerAlign       GetVerAlign() const { return meVerAlign; }

    /** Sets the text of this Escher text object to the passed SdrObj, if it can take text. */
    void                        ApplyTextOnSdrObj( SdrObject& rSdrObj );

    /** Sets a new SdrObj for this Escher object. This object owns the passed SdrObj. */
    virtual void                SetSdrObj( SdrObject* pSdrObj );

private:
    /** Sets the text of this Escher object to the own SdrObj. */
    void                        ApplyText();

private:
    typedef ::std::auto_ptr< XclImpString > XclImpStringPtr;

    XclImpStringPtr             mpString;       /// Plain or rich string.
    XclTxoHorAlign              meHorAlign;     /// Horizontal alignment.
    XclTxoVerAlign              meVerAlign;     /// Vertical alignment.
};


// ----------------------------------------------------------------------------

/** A note object, which is a specialized text box objext. */
class XclImpEscherNote : public XclImpEscherTxo
{
public:
    TYPEINFO();

    /** Constructor takes ownership of the members of rSrcObj, which will be invalidated. */
    explicit                    XclImpEscherNote( XclImpEscherObj& rSrcObj );

    /** Empty. Prevents insertion of a drawing object for a note (done via ScDetectiveFunc). */
    virtual void                Apply( ScfProgressBar& rProgress );
};


// ----------------------------------------------------------------------------

/** Helper to manage controls linked to the sheet. */
class XclImpCtrlLinkHelper
{
public:
    explicit                    XclImpCtrlLinkHelper( ScDocument& rDoc );

    /** Reads the formula for the linked cell from the current position of the stream. */
    void                        ReadCellLinkFormula( XclImpStream& rStrm );
    /** Reads the formula for the source range from the current position of the stream. */
    void                        ReadSrcRangeFormula( XclImpStream& rStrm );

    /** Inserts a string list property from the document to the property set. */
    void                        InsertStringList(
                                    ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertySet >& rxPropSet ) const;
    /** Inserts a tag property containing the link range address to the property set. */
    void                        InsertLinkTag(
                                    ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertySet >& rxPropSet ) const;

    /** Returns the string from the document, which appears at the passed position in the listbox. */
    String                      GetString( sal_Int16 nPos ) const;

private:
    ScDocument&                 mrDoc;          /// Source Calc document.
    ::std::auto_ptr< ScAddress > mpCellLink;    /// Linked cell in the Calc document.
    ::std::auto_ptr< ScRange >  mpSrcRange;     /// Source data range in the Calc document.
};


// ----------------------------------------------------------------------------

/** An old form control object (does not use the OLE mechanism, but is a "simple" drawing object). */
class XclImpEscherTbxCtrl : public XclImpEscherTxo
{
public:
    TYPEINFO();

    /** Constructor takes ownership of the members of rSrcObj, which will be invalidated. */
    explicit                    XclImpEscherTbxCtrl( XclImpEscherObj& rSrcObj, sal_uInt16 nCtrlType );

    /** Returns the type of the control, which is the object type from the OBJ record. */
    inline sal_uInt16           GetType() const { return mnCtrlType; }

    /** Reads the contents of the ftCbls sub structure in an OBJ record. */
    void                        ReadCbls( XclImpStream& rStrm );
    /** Reads the contents of the ftCblsFmla sub structure in an OBJ record. */
    void                        ReadCblsFmla( XclImpStream& rStrm );
    /** Reads the contents of the ftLbsData sub structure in an OBJ record. */
    void                        ReadLbsData( XclImpStream& rStrm );

    /** Returns the complete component service name for this control. */
    ::rtl::OUString             GetServiceName() const;

    /** Sets control specific properties at the passed property set. */
    void                        SetProperties( ::com::sun::star::uno::Reference<
                                    ::com::sun::star::beans::XPropertySet >& rxPropSet ) const;

    /** Adds a segment to the progress bar. */
    virtual void                InitProgress( ScfProgressBar& rProgress );
    /** Inserts the contained SdrObj into the draw page. */
    virtual void                Apply( ScfProgressBar& rProgress );

private:
    ScfInt16Vec                 maMultiSel;     /// Indexes of all selected entries in a multi selection.
    XclImpCtrlLinkHelper        maLinkHelper;   /// Helper for linked ranges.
    sal_Int32                   mnProgressSeg;  /// Progress bar segment identifier.
    sal_uInt16                  mnCtrlType;     /// Type of the control from OBJ record.
    sal_uInt16                  mnState;        /// Checked/unchecked state.
    sal_Int16                   mnSelEntry;     /// Index of selected entry (1-based).
    sal_Int16                   mnSelType;      /// Selection type.
    sal_Int16                   mnLineCount;    /// Combobox dropdown line count.
    bool                        mb3DStyle;      /// true = 3D style.
};


// ----------------------------------------------------------------------------

class XclImpDffManager;

/** A common Escher OLE object, or an OLE form control. */
class XclImpEscherOle : public XclImpEscherObj
{
public:
    TYPEINFO();

    /** Constructor takes ownership of the members of rSrcObj, which will be invalidated. */
    explicit                    XclImpEscherOle( XclImpEscherObj& rSrcObj );

    /** Returns true, if this object is a form control, and false, if it is a common OLE object. */
    inline bool                 IsControl() const { return mbControl; }

    /** Returns the OLE storage name used in the Excel document. */
    inline const String&        GetStorageName() const { return maStorageName; }
    /** Returns the BLIP identifier for the meta file. */
    inline sal_uInt32           GetBlipId() const { return mnBlipId; }
    /** Returns the position in Ctrl stream for additional form control data. */
    inline sal_uInt32           GetCtrlStreamPos() const { return mnCtrlStrmPos; }

    /** Sets the BLIP identifier for the meta file. */
    inline void                 SetBlipId( sal_uInt32 nBlipId ) { mnBlipId = nBlipId; }

    /** Reads the contents of the ftPioGrbit sub structure in an OBJ record. */
    void                        ReadPioGrbit( XclImpStream& rStrm );
    /** Reads the contents of the ftPictFmla sub structure in an OBJ record. */
    void                        ReadPictFmla( XclImpStream& rStrm, sal_uInt16 nRecSize );

    /** Sets control specific properties at the passed property set. */
    void                        SetProperties( ::com::sun::star::uno::Reference<
                                    ::com::sun::star::beans::XPropertySet >& rxPropSet ) const;

    /** Adds a segment to the progress bar. */
    virtual void                InitProgress( ScfProgressBar& rProgress );
    /** Inserts the contained SdrObj into the draw page. */
    virtual void                Apply( ScfProgressBar& rProgress );

private:
    String                      maStorageName;  /// Name of the OLE storage for this object.
    XclImpCtrlLinkHelper        maLinkHelper;   /// Helper for linked ranges.
    sal_uInt32                  mnBlipId;       /// The BLIP identifier (meta file).
    sal_uInt32                  mnCtrlStrmPos;  /// Position in Ctrl stream for controls.
    sal_Int32                   mnProgressSeg;  /// Progress bar segment identifier.
    bool                        mbAsSymbol;     /// true = Show as symbol.
    bool                        mbLinked;       /// true = Linked; false = Embedded.
    bool                        mbControl;      /// true = Form control, false = OLE object.
};

// ----------------------------------------------------------------------------

class XclImpChart;

/** A chart object. */
class XclImpEscherChart : public XclImpEscherObj
{
public:
    TYPEINFO();

    /** Constructor takes ownership of the members of rSrcObj, which will be invalidated. */
    explicit                    XclImpEscherChart( XclImpEscherObj& rSrcObj );

    /** Returns the chart this object contains. */
    inline XclImpChart*         GetChartData() { return mpChart.get(); }
    /** Sets a new chart at this object. */
    void                        SetChartData( XclImpChart* pChart );

    /** Adds a segment to the progress bar according to the current chart data. */
    virtual void                InitProgress( ScfProgressBar& rProgress );
    /** Inserts the contained chart into the document. */
    virtual void                Apply( ScfProgressBar& rProgress );

private:
    typedef ::std::auto_ptr< XclImpChart > XclImpChartPtr;

    XclImpChartPtr              mpChart;        /// The chart itself.
};


// Escher object data =========================================================

/** Represents the position (anchor) of an Escher object in the Calc document. */
struct XclImpEscherAnchor
{
    sal_uInt16                  mnScTab;    /// Calc sheet index of the object.

    sal_uInt16                  mnLCol;     /// Left column index.
    sal_uInt16                  mnLX;       /// X offset in left column (1/1024 of column width).
    sal_uInt16                  mnTRow;     /// Top row index.
    sal_uInt16                  mnTY;       /// Y offset in top row (1/256 of row height).
    sal_uInt16                  mnRCol;     /// Right column index.
    sal_uInt16                  mnRX;       /// X offset in right column (1/1024 of column width).
    sal_uInt16                  mnBRow;     /// Bottom row index.
    sal_uInt16                  mnBY;       /// Y offset in bottom row (1/256 of row height).

    explicit                    XclImpEscherAnchor( sal_uInt16 nScTab );
};

SvStream& operator>>( SvStream& rStrm, XclImpEscherAnchor& rAnchor );


// ----------------------------------------------------------------------------

/** Contains all information of an Escher object.
    @descr  This is the Escher object itself (XclImpEscherObj) and the position
    in the Calc document (XclImpEscherAnchor). */
class XclImpObjData
{
public:
    /** Takes ownership of pObj. */
    explicit                    XclImpObjData( XclImpEscherObj* pEscherObj );

    /** Sets the passed Escher object, deletes the old. */
    void                        SetObj( XclImpEscherObj* pEscherObj );

    /** Returns the Escher object, if present. */
    inline XclImpEscherObj*     GetObj() { return mpEscherObj.get(); }
    /** Returns the anchor data. */
    inline XclImpEscherAnchor&  GetAnchor() { return maAnchor; }

    /** Returns true, if the passed stream position is part of the current object. */
    bool                        ContainsStrmPos( sal_uInt32 nStrmPos ) const;

private:
    typedef ::std::auto_ptr< XclImpEscherObj > XclImpEscherObjPtr;

    XclImpEscherAnchor          maAnchor;       /// The sheet position of the object.
    XclImpEscherObjPtr          mpEscherObj;    /// The Escher object itself.
};


// ----------------------------------------------------------------------------

/** This list contains all read Escher objects with their anchor positions. */
class XclImpEscherObjList : ScfNoCopy
{
public:
    /** Returns the number of contained objects. */
    inline sal_uInt32           GetObjCount() const { return maObjDataList.Count(); }

    /** Appends the passed object to the list. Takes ownership of the object. */
    void                        AppendObj( XclImpEscherObj* pEscherObj );
    /** Appends the passed object as dummy (does not use it later). Takes ownership of the object. */
    void                        AppendDummyObj( XclImpEscherObj* pEscherObj );
    /** Replaces the Escher object of the last inserted object data (or appends, if list is empty). */
    void                        ReplaceLastObj( XclImpEscherObj* pEscherObj );

    /** Returns the object in the specified sheet by Excel object identifier. */
    XclImpEscherObj*            GetObj( sal_uInt16 nScTab, sal_uInt16 nObjId ) const;
    /** Returns the object at the specified Escher stream position. */
    XclImpEscherObj*            GetObj( sal_uInt32 nStrmPos ) const;
    /** Returns the last inserted Escher object in the list. */
    XclImpEscherObj*            GetLastObj() const;

    /** Returns the anchor of the object at the passed Escher stream position. */
    XclImpEscherAnchor*         GetAnchor( sal_uInt32 nStrmPos ) const;

    /** Initializes the progress bar for all objects. */
    void                        InitProgress( ScfProgressBar& rProgress );
    /** Inserts all objects into the Calc document. */
    void                        Apply( ScfProgressBar& rProgress );

private:
    /** Updates the cache data with the last inserted object for searching. */
    void                        UpdateCache();
    /** Returns the object data of the Escher object at the specified Escher stream position. */
    XclImpObjData*              FindObjData( sal_uInt32 nStrmPos ) const;

private:
    /** Stores data for each sheet to speed up the search for objects. */
    struct XclCacheEntry
    {
        sal_uInt32                  mnListIndex;    /// List index of first object in the sheet.
        sal_uInt32                  mnStrmPos;      /// First Escher stream position of the objects in the sheet.
        inline explicit             XclCacheEntry( sal_uInt32 nListIndex, sal_uInt32 nStrmPos ) :
                                        mnListIndex( nListIndex ), mnStrmPos( nStrmPos ) {}
    };

    typedef ScfDelList< XclImpObjData >     XclImpObjDataList;
    typedef ::std::vector< XclCacheEntry >  XclCacheVec;

    XclImpObjDataList           maObjDataList;      /// The list of all object data structs.
    XclCacheVec                 maObjCache;         /// Caches data to speed up object search.
};


// Escher stream conversion ===================================================

class XclImpObjectManager;

/** Derived from SvxMSDffManager, contains core implementation of Escher stream import. */
class XclImpDffManager : public SvxMSDffManager, protected XclImpRoot
{
public:
    explicit                    XclImpDffManager(
                                    const XclImpRoot& rRoot, XclImpObjectManager& rObjManager,
                                    sal_Int32 nOffsDgg, SvStream* pStData, SdrModel* pSdrModel,
                                    sal_Int32 nApplicationScale );

    /** Converts an Escher object and creates and attaches the corresponding SdrObj. */
    void                        SetSdrObject(
                                    XclImpEscherObj* pEscherObj,
                                    sal_uInt32 nId,
                                    SvxMSDffImportData& rData );

    /** Creates the SdrOle2Obj for the passed Escher OLE object from the OLE storage in the Excel file.
        @return  true = SdrOle2Obj successfully created. */
    bool                        CreateSdrOleObj( XclImpEscherOle& rOleObj );

protected:
    /** Reads the client anchor from the Escher stream and sets it at the correct Escher object. */
    virtual void                ProcessClientAnchor2( SvStream& rStrm, DffRecordHeader&, void*, DffObjData& rObjData );
    /** Processes an Escher object, reads properties from Escher stream. */
    virtual SdrObject*          ProcessObj(
                                    SvStream&,
                                    DffObjData& rObjData,
                                    void* pData,
                                    Rectangle& rTextRect,
                                    SdrObject* pRetSdrObj = NULL );
    /** Returns the BLIP stream position, based on the passed Escher stream position. */
    virtual sal_uInt32          Calc_nBLIPPos( sal_uInt32 nOrgVal, sal_uInt32 nStreamPos ) const;
    /** Returns a color from the Excel color palette. */
    virtual FASTBOOL            GetColorFromPalette( sal_uInt16 nIndex, Color& rColor ) const;
    /** Returns true, if the object with the passed shape ID contains any text data. */
    virtual sal_Bool            ShapeHasText( sal_uInt32 nShapeId, sal_uInt32 nFilePos ) const;

private:
    XclImpObjectManager&        mrObjManager;   /// The Excel object manager.
    sal_uInt32                  mnOleImpFlags;  /// Application OLE import settings.
};


// The object manager =========================================================

class XclImpOcxConverter;

/** Stores all drawing and OLE objects and additional data related to these objects. */
class XclImpObjectManager : protected XclImpRoot
{
public:
    explicit                    XclImpObjectManager( const XclImpRoot& rRoot );
                                ~XclImpObjectManager();

// *** Escher stream *** ------------------------------------------------------

    /** Returns true, if the Escher stream contains any data. */
    inline bool                 HasEscherStream()   { return maStreamConsumer.HasData(); }
    /** Returns the Escher stream. */
    inline SvStream&            GetEscherStream()   { return maStreamConsumer.GetStream(); }

// *** Escher objects *** -----------------------------------------------------

    /** Returns the number of contained objects. */
    inline sal_uInt32           GetEscherObjCount() const { return maEscherObjList.GetObjCount(); }

    /** Returns the object in the specified sheet by Excel object identifier. */
    const XclImpEscherObj*      GetEscherObj( sal_uInt16 nScTab, sal_uInt16 nObjId ) const;
    /** Returns access to the object in the specified sheet by Excel object identifier. */
    XclImpEscherObj*            GetEscherObjAcc( sal_uInt16 nScTab, sal_uInt16 nObjId );

    /** Returns the object at the specified Escher stream position. */
    const XclImpEscherObj*      GetEscherObj( sal_uInt32 nStrmPos ) const;
    /** Returns access to the object at the specified Escher stream position. */
    XclImpEscherObj*            GetEscherObjAcc( sal_uInt32 nStrmPos );

    /** Returns the last inserted object in the list. */
    const XclImpEscherObj*      GetLastEscherObj() const;
    /** Returns access to the last inserted object in the list. */
    XclImpEscherObj*            GetLastEscherObjAcc();

    /** Returns the anchor of the object at the passed Escher stream position. */
    const XclImpEscherAnchor*   GetEscherAnchor( sal_uInt32 nStrmPos ) const;
    /** Returns access to the anchor of the object at the passed Escher stream position. */
    XclImpEscherAnchor*         GetEscherAnchorAcc( sal_uInt32 nStrmPos );

// *** Text boxes *** ---------------------------------------------------------

    /** Returns the textbox object at the specified Escher stream position. */
    const XclImpEscherTxo*      GetEscherTxo( sal_uInt32 nStrmPos ) const;
    /** Returns access to the textbox object at the specified Escher stream position. */
    XclImpEscherTxo*            GetEscherTxoAcc( sal_uInt32 nStrmPos );

    /** Returns the note object in the specified sheet by Excel object identifier. */
    const XclImpEscherNote*     GetEscherNote( sal_uInt16 nScTab, sal_uInt16 nObjId ) const;

// *** Chart *** --------------------------------------------------------------

    /** Returns true, if the current object is a chart. */
    bool                        IsCurrObjChart() const;

    /** Returns the chart data of the current chart object, or 0, if there is no chart. */
    XclImpChart*                GetCurrChartData();
    /** Sets a new chart data (with the passed chart type) at the current object.
        @descr  Reads additional properies of the chart type from stream.
        @return  The new chart data. The old chart data, returned by GetCurrChartData(),
        is no longer valid. */
    XclImpChart*                ReplaceChartData( XclImpStream& rStrm, XclChartType eNewType );

    /** Inserts a new chart object.
        @descr  Used to import sheet charts, which do not have a corresponding OBJ record. */
    void                        StartNewChartObj();

// *** OLE / controls *** -----------------------------------------------------

    /** Creates the SdrObj for an OLE Escher object.
        @descr  The passed object may be a common OLE object, then this function creates
        the internal OLE stream. Or it is a form control, then this function reads the
        control formatting data from the 'Ctls' stream. */
    bool                        CreateSdrObj( XclImpEscherOle& rOleObj );
    /** Creates the SdrObj for an old-fashioned Escher control object. */
    bool                        CreateSdrObj( XclImpEscherTbxCtrl& rCtrlObj );

// *** Read Excel records *** -------------------------------------------------

    /** Reads the MSODRAWINGGROUP record. */
    void                        ReadMsodrawinggroup( XclImpStream& rStrm );
    /** Reads the MSODRAWING record. */
    void                        ReadMsodrawing( XclImpStream& rStrm );
    /** Reads the MSODRAWINGSELECTION record. */
    void                        ReadMsodrawingselection( XclImpStream& rStrm );

    /** Reads the OBJ record. */
    void                        ReadObj( XclImpStream& rStrm );
    /** Reads the TXO record. */
    void                        ReadTxo( XclImpStream& rStrm );

// *** Misc *** ---------------------------------------------------------------

    /** Lets the object manager add a dummy object, before the next object is read. */
    inline void                 InsertDummyObj() { mbStartWithDummy = true; }

    /** Returns the DFF manager (Escher stream converter). Don't call before the Escher stream is read. */
    XclImpDffManager&           GetDffManager();
    /** Updates the connector rules of the passed object in the solver container. */
    void                        UpdateConnectorRules( const DffObjData& rObjData, SdrObject* pSdrObj );

    /** Sets the object with the passed identification to be ignored on import. */
    void                        SetSkipObj( sal_uInt16 nScTab, sal_uInt16 nObjId );

    /** Inserts all objects into the Calc document. */
    void                        Apply();

private:
    /** Appends the passed object to the list. */
    void                        AppendEscherObj( XclImpEscherObj* pEscherObj );
    /** Replaces the last object in the list with the passed object. */
    void                        ReplaceEscherObj( XclImpEscherObj* pEscherObj );

    /** Reads the ftCmo sub structure (common object data) in an OBJ record.
        @return  The new Escher object. */
    void                        ReadObjFtCmo( XclImpStream& rStrm );
    /** Reads the ftPioGrbit sub structure (object option flags) in an OBJ record. */
    void                        ReadObjFtPioGrbit( XclImpStream& rStrm );
    /** Reads the ftPictFmla sub structure (OLE link formula) in an OBJ record. */
    void                        ReadObjFtPictFmla( XclImpStream& rStrm, sal_uInt16 nRecSize );

    /** Reads a sub record for TBX form controls in an OBJ record. */
    void                        ReadObjTbxSubRec( XclImpStream& rStrm, sal_uInt16 nSubRecId );

    /** Returns the solver container (for connector rules). */
    SvxMSDffSolverContainer&    GetSolverContainer();
    /** Returns the OCX converter (OCX form controls converter). */
    XclImpOcxConverter&         GetOcxConverter();

    /** Identifies an Escher object to skip on import (will not be inserted into the Calc document). */
    struct XclSkipObj
    {
        sal_uInt16                  mnScTab;        /// Calc sheet index.
        sal_uInt16                  mnObjId;        /// Excel object identifier.
        inline explicit             XclSkipObj( sal_uInt16 nScTab, sal_uInt16 nObjId ) :
                                        mnScTab( nScTab ), mnObjId( nObjId ) {}
    };

    typedef ::std::auto_ptr< XclImpDffManager >         XclImpDffManagerPtr;
    typedef ::std::auto_ptr< SvxMSDffSolverContainer >  SolverContainerPtr;
    typedef ::std::auto_ptr< XclImpOcxConverter >       XclImpOcxConverterPtr;
    typedef ::std::vector< XclSkipObj >                 XclSkipObjVec;

    XclImpStreamConsumer        maStreamConsumer;   /// Collects the entire Escher stream.
    XclImpEscherObjList         maEscherObjList;    /// Contains all Escher objects.
    SolverContainerPtr          mpSolverContainer;  /// The solver container for connector rules.
    XclImpDffManagerPtr         mpDffManager;       /// The Escher stream converter.
    XclImpOcxConverterPtr       mpOcxConverter;     /// The form controls converter.
    XclSkipObjVec               maSkipObjVec;       /// All Escher objects to skip.

    bool                        mbStartWithDummy;   /// true = Insert a dummy object before the next Escher object.

};


// Escher property set helper =================================================

/** This class reads an Escher property set (msofbtOPT record).
    @descr  It can return separate property values or an item set which contains
    items translated from these properties. */
class XclImpEscherPropSet
{
public:
    explicit                    XclImpEscherPropSet( const XclImpDffManager& rDffManager );

    /** Reads an Escher property set from the stream.
        @descr  The stream must point to the start of an Escher record containing properties. */
    void                        Read( XclImpStream& rStrm );

    /** Returns the specified property or the default value, if not extant. */
    sal_uInt32                  GetPropertyValue( sal_uInt16 nPropId, sal_uInt32 nDefault = 0 ) const;

    /** Translates the properties and fills the item set. */
    void                        FillToItemSet( SfxItemSet& rItemSet ) const;

private:
    typedef ::std::auto_ptr< SvMemoryStream > SvMemoryStreamPtr;

    DffPropertyReader           maPropReader;   /// Reads the properties from an SvStream.
    SvMemoryStreamPtr           mpMemStrm;      /// Helper stream.
};

XclImpStream& operator>>( XclImpStream& rStrm, XclImpEscherPropSet& rPropSet );


// ============================================================================

#endif

