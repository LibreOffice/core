/*************************************************************************
 *
 *  $RCSfile: xiescher.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-14 12:11:32 $
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

namespace com { namespace sun { namespace star {
    namespace script { struct ScriptEventDescriptor; }
} } }

class ScfProgressBar;

// Escher stream consumer =====================================================

struct XclImpStreamNode
{
    ULONG               mnPos;
    ULONG               mnSize;
    XclImpStreamNode*   mpPrev;
};

// ----------------------------------------------------------------------------

class XclImpStreamConsumer : ScfNoCopy
{
public:
    explicit            XclImpStreamConsumer();
                        ~XclImpStreamConsumer();

    const DffRecordHeader* ConsumeRecord( XclImpStream& rSrcStrm );
    bool                AppendData( sal_Char* pBuf, ULONG nLen );

    inline bool         HasData() const { return maStrm.Tell() > 0; }
    inline ULONG        Tell() const { return maStrm.Tell(); }
    inline SvStream&    GetStream() { return maStrm; }

private:
    void                UpdateNode( const DffRecordHeader& rHd );
    void                RemoveNode();

private:
    SvMemoryStream      maStrm;
    DffRecordHeader     maHd;
    XclImpStreamNode*   mpNode;
    ULONG               mnBytesLeft;
};

// Escher objects =============================================================

/** Base class for Escher objects. Does not contain any specialized data. */
class XclImpEscherObj : protected XclImpRoot, ScfNoCopy
{
public:
    TYPEINFO();

    /** Constructs a dummy Escher object with an invalid Escher stream position. */
    explicit            XclImpEscherObj( const XclImpRoot& rRoot );
    /** Constructs a new Escher object at the specified Escher stream position. */
    explicit            XclImpEscherObj(
                            const XclImpRoot& rRoot,
                            ULONG nStrmBegin, ULONG nStrmEnd );
    /** Constructor takes ownership of the members of rSrcObj, which will be invalidated. */
    explicit            XclImpEscherObj( XclImpEscherObj& rSrcObj );

    virtual             ~XclImpEscherObj();

    /** Returns the SdrObj of this Escher object. */
    inline const SdrObject* GetSdrObj() const { return mxSdrObj.get(); }
    /** Returns the position of the object in the draw page. */
    inline const Rectangle& GetAnchor() const { return maAnchorRect; }
    /** Returns the start position in the Escher stream of this object. */
    inline ULONG        GetStrmBegin() const { return mnStrmBegin; }
    /** Returns the end position in the Escher stream of this object (last position + 1). */
    inline ULONG        GetStrmEnd() const { return mnStrmEnd; }
    /** Returns the Calc sheet index of this object. */
    inline SCTAB        GetScTab() const { return mnScTab; }
    /** Returns the Excel object identifier. */
    inline sal_uInt16   GetObjId() const { return mnObjId; }
    /** Returns true, if Escher object will be skipped on creating the draw page. */
    inline bool         GetIsSkip() const { return mbSkip; }
    /** Returns true, if Escher object is printable. */
    inline bool         GetPrintable() const { return mbPrintable; }

    /** Returns true, if the passed width and height would be valid for this object. */
    bool                IsValidSize( const Rectangle& rRect ) const;

    /** Returns true, if the width and height of the object are valid. */
    bool                IsValidSize() const;
    /** Returns true, if this Escher object contains an SdrObj and a valid anchor position. */
    bool                IsValid() const;

    /** Sets the position of this object in the draw page. */
    inline void         SetAnchor( const Rectangle& rRect ) { maAnchorRect = rRect; }
    /** Sets the Excel object identifier (unique per sheet). */
    inline void         SetObjId( sal_uInt16 nObjId ) { mnObjId = nObjId; }
    /** Sets whether this is an area object (then its width and height must be greater than 0). */
    inline void         SetAreaObj( bool bAreaObj ) { mbAreaObj = bAreaObj; }
    /** Marks this Esher object to be skipped on export. */
    inline void         SetSkip() { mbSkip = true; }

    /** Sets a new SdrObj for this Escher object. This object owns the passed SdrObj. */
    virtual void        SetSdrObj( SdrObject* pSdrObj );
    /** Set true if Escher object is printable. */
    inline void         SetPrintable( bool bPrint ) { mbPrintable = bPrint; }

    /** Returns the needed size on the progress bar. */
    virtual sal_uInt32  GetProgressSize() const;
    /** Inserts the contained SdrObj into the draw page. */
    virtual void        Apply( ScfProgressBar& rProgress );

protected:
    typedef ::std::auto_ptr< SdrObject > SdrObjectPtr;

    Rectangle           maAnchorRect;   /// Location of the object in the draw page.
    SdrObjectPtr        mxSdrObj;       /// SdrObj representing this Escher object.
    ULONG               mnStrmBegin;    /// Start position in Escher stream.
    ULONG               mnStrmEnd;      /// End position in Escher stream (last + 1).
    SCTAB               mnScTab;        /// Calc sheet index of the object.
    sal_uInt16          mnObjId;        /// The Excel object identifier (from OBJ record).
    bool                mbAreaObj;      /// true = Width and height must be greater than 0.
    bool                mbSkip;         /// true = Skip creation (ignore this object).
    bool                mbPrintable;    /// true = Print this object.
};

// ----------------------------------------------------------------------------

/** A simple drawing object, i.e. line, rectangle, or bitmap. */
class XclImpEscherDrawing : public XclImpEscherObj
{
public:
    TYPEINFO();

    /** Constructor takes ownership of the members of rSrcObj, which will be invalidated.
        @param bAreaObj  true = Width and height of the object must be greater than 0. */
    explicit            XclImpEscherDrawing( XclImpEscherObj& rSrcObj, bool bAreaObj );
};

// ----------------------------------------------------------------------------

/** A common text box object. */
class XclImpEscherTxo : public XclImpEscherDrawing
{
public:
    TYPEINFO();

    /** Constructor takes ownership of the members of rSrcObj, which will be invalidated. */
    explicit            XclImpEscherTxo( XclImpEscherObj& rSrcObj );

    /** Returns the string data, if there is any. */
    inline const XclImpString* GetString() const { return mxString.get(); }
    /** Sets a new plain or rich string. This object will own the string. */
    inline void         SetString( XclImpString* pString ) { mxString.reset( pString ); }

    /** Sets the horizontal and vertical text alignment from the passed Excel TXO flags. */
    void                SetAlignment( sal_uInt16 nAlign );
    /** Sets the rotation text alignment from the passed Excel TXO orientation flags. */
    void                SetRotation( sal_uInt16 nOrient );
    /** Gets the rotation text alignment. */
    inline XclTxoRotation      GetRotation() const { return meRotation; }

    /** Sets the text of this Escher text object to the passed SdrObj, if it can take text. */
    void                ApplyTextOnSdrObj( SdrObject& rSdrObj ) const;

    /** Sets a new SdrObj for this Escher object. This object owns the passed SdrObj. */
    virtual void        SetSdrObj( SdrObject* pSdrObj );

private:
    /** Sets the text of this Escher object to the own SdrObj. */
    void                ApplyText();

private:
    typedef ::std::auto_ptr< XclImpString > XclImpStringPtr;

    XclImpStringPtr     mxString;       /// Plain or rich string.
    XclTxoHorAlign      meHorAlign;     /// Horizontal alignment.
    XclTxoVerAlign      meVerAlign;     /// Vertical alignment.
    XclTxoRotation      meRotation;     /// Text rotation.
};

// ----------------------------------------------------------------------------

/** A note object, which is a specialized text box objext. */
class XclImpEscherNote : public XclImpEscherTxo
{
public:
    TYPEINFO();

    /** Constructor takes ownership of the members of rSrcObj, which will be invalidated. */
    explicit            XclImpEscherNote( XclImpEscherObj& rSrcObj );

    virtual void                Apply( ScfProgressBar& rProgress );

    inline SCCOL                GetCol() const          { return mnCol; }
    inline SCROW                GetRow() const          { return mnRow; }
    inline void                 SetCol(SCCOL nCol) { mnCol = nCol; }
    inline void                 SetRow(SCROW nRow) { mnRow = nRow; }

private:
    SCCOL                  mnCol;        /// Calc source column index of the cell note.
    SCROW                  mnRow;        /// Row source column index of the cell note.
};

// ----------------------------------------------------------------------------

/** Helper to manage controls linked to the sheet. */
class XclImpCtrlLinkHelper
{
public:
    explicit            XclImpCtrlLinkHelper( XclCtrlBindMode eBindMode );

    /** Returns the linked cell address, or 0, if not present. */
    inline const ScAddress* GetCellLink() const { return mxCellLink.get(); }
    /** Returns the value binding mode for linked cells. */
    inline XclCtrlBindMode GetBindingMode() const { return meBindMode; }
    /** Returns the linked source cell range, or 0, if not present. */
    inline const ScRange* GetSourceRange() const { return mxSrcRange.get(); }

protected:
    /** Reads the formula for the linked cell from the current position of the stream. */
    void                ReadCellLinkFormula( XclImpStream& rStrm );
    /** Reads the formula for the source range from the current position of the stream. */
    void                ReadSrcRangeFormula( XclImpStream& rStrm );

private:
    ::std::auto_ptr< ScAddress > mxCellLink;    /// Linked cell in the Calc document.
    ::std::auto_ptr< ScRange >  mxSrcRange;     /// Source data range in the Calc document.
    XclCtrlBindMode     meBindMode;             /// Value binding mode.
};

// ----------------------------------------------------------------------------

/** An old form control object (does not use the OLE mechanism, but is a "simple" drawing object). */
class XclImpEscherTbxCtrl : public XclImpEscherTxo, public XclImpCtrlLinkHelper
{
public:
    TYPEINFO();

    /** Constructor takes ownership of the members of rSrcObj, which will be invalidated. */
    explicit            XclImpEscherTbxCtrl( XclImpEscherObj& rSrcObj, sal_uInt16 nCtrlType );

    /** Returns the type of the control, which is the object type from the OBJ record. */
    inline sal_uInt16   GetType() const { return mnCtrlType; }

    /** Reads the contents of the ftCbls sub structure in an OBJ record. */
    void                ReadCbls( XclImpStream& rStrm );
    /** Reads the contents of the ftCblsFmla sub structure in an OBJ record. */
    void                ReadCblsFmla( XclImpStream& rStrm );
    /** Reads the contents of the ftLbsData sub structure in an OBJ record. */
    void                ReadLbsData( XclImpStream& rStrm );
    /** Reads the contents of the ftSbs sub structure in an OBJ record. */
    void                ReadSbs( XclImpStream& rStrm );
    /** Reads the contents of the ftGboData sub structure in an OBJ record. */
    void                ReadGboData( XclImpStream& rStrm );
    /** Reads the contents of the ftMacro sub structure in an OBJ record. */
    void                ReadMacro( XclImpStream& rStrm );

    /** Returns the complete component service name for this control. */
    ::rtl::OUString     GetServiceName() const;

    /** Sets form control specific properties. */
    void                SetProperties( ::com::sun::star::uno::Reference<
                            ::com::sun::star::beans::XPropertySet >& rxPropSet ) const;
    /** Tries to fill the passed descriptor with imported macro data.
        @return  true = Control is associated with a macro, rEvent contains valid data. */
    bool                FillMacroDescriptor(
                            ::com::sun::star::script::ScriptEventDescriptor& rEvent ) const;

    /** Inserts the contained SdrObj into the draw page. */
    virtual void        Apply( ScfProgressBar& rProgress );

private:
    ScfInt16Vec         maMultiSel;     /// Indexes of all selected entries in a multi selection.
    String              maMacroName;    /// Name of an attached macro.
    sal_uInt16          mnCtrlType;     /// Type of the control from OBJ record.
    sal_uInt16          mnState;        /// Checked/unchecked state.
    sal_Int16           mnSelEntry;     /// Index of selected entry (1-based).
    sal_Int16           mnSelType;      /// Selection type.
    sal_Int16           mnLineCount;    /// Combobox dropdown line count.
    sal_Int16           mnScrollValue;  /// Scrollbar: Current value.
    sal_Int16           mnScrollMin;    /// Scrollbar: Minimum value.
    sal_Int16           mnScrollMax;    /// Scrollbar: Maximum value.
    sal_Int16           mnScrollStep;   /// Scrollbar: Single step.
    sal_Int16           mnScrollPage;   /// Scrollbar: Page step.
    bool                mbFlatButton;   /// False = 3D button style; True = Flat button style.
    bool                mbFlatBorder;   /// False = 3D border style; True = Flat border style.
    bool                mbScrollHor;    /// Scrollbar: true = horizontal.
};

// ----------------------------------------------------------------------------

class XclImpDffManager;

/** A common Escher OLE object, or an OLE form control. */
class XclImpEscherOle : public XclImpEscherObj, public XclImpCtrlLinkHelper
{
public:
    TYPEINFO();

    /** Constructor takes ownership of the members of rSrcObj, which will be invalidated. */
    explicit            XclImpEscherOle( XclImpEscherObj& rSrcObj );

    /** Returns true, if this object is a form control, and false, if it is a common OLE object. */
    inline bool         IsControl() const { return mbControl; }

    /** Returns the OLE storage name used in the Excel document. */
    inline const String& GetStorageName() const { return maStorageName; }
    /** Returns the BLIP identifier for the meta file. */
    inline sal_uInt32   GetBlipId() const { return mnBlipId; }
    /** Returns the position in Ctrl stream for additional form control data. */
    inline sal_uInt32   GetCtrlStreamPos() const { return mnCtrlStrmPos; }
    /** Returns control name indicated by MsofbtOPT property DFF_Prop_wzName
        if present*/
    inline ::rtl::OUString      GetName() { return msName; }
    /** Sets the BLIP identifier for the meta file. */

    inline void         SetBlipId( sal_uInt32 nBlipId ) { mnBlipId = nBlipId; }
    /** Sets name indicated by MsofbtOPT property DFF_Prop_wzName */
    inline void                 SetName( const rtl::OUString& sName ) { msName = sName; }
    /** Reads the contents of the ftPioGrbit sub structure in an OBJ record. */
    void                ReadPioGrbit( XclImpStream& rStrm );
    /** Reads the contents of the ftPictFmla sub structure in an OBJ record. */
    void                ReadPictFmla( XclImpStream& rStrm, sal_uInt16 nRecSize );

    /** Sets form control specific properties. */
    void                SetProperties( ::com::sun::star::uno::Reference<
                            ::com::sun::star::beans::XPropertySet >& rxPropSet ) const;

    /** Inserts the contained SdrObj into the draw page. */
    virtual void        Apply( ScfProgressBar& rProgress );

private:
    String              maStorageName;  /// Name of the OLE storage for this object.
    ::rtl::OUString     msName;         /// control name if form control
    sal_uInt32          mnBlipId;       /// The BLIP identifier (meta file).
    sal_uInt32          mnCtrlStrmPos;  /// Position in Ctrl stream for controls.
    bool                mbAsSymbol;     /// true = Show as symbol.
    bool                mbLinked;       /// true = Linked; false = Embedded.
    bool                mbControl;      /// true = Form control, false = OLE object.
};

// ----------------------------------------------------------------------------

class XclImpChart;

/** A chart object. */
class XclImpEscherChart : public XclImpEscherObj
{
public:
    TYPEINFO();

    /** Constructor takes ownership of the members of rSrcObj, which will be invalidated. */
    explicit            XclImpEscherChart( XclImpEscherObj& rSrcObj );

    /** Returns the chart this object contains. */
    inline XclImpChart* GetChartData() { return mxChart.get(); }
    /** Sets a new chart at this object. */
    void                SetChartData( XclImpChart* pChart );

    /** Returns the needed size on the progress bar. */
    virtual sal_uInt32  GetProgressSize() const;
    /** Inserts the contained chart into the document. */
    virtual void        Apply( ScfProgressBar& rProgress );

private:
    typedef ::std::auto_ptr< XclImpChart > XclImpChartPtr;

    XclImpChartPtr      mxChart;        /// The chart itself.
};

// Escher object data =========================================================

/** Represents the position (anchor) of an Escher object in the Calc document. */
struct XclImpEscherAnchor
{
    SCTAB               mnScTab;    /// Calc sheet index of the object.

    sal_uInt16          mnLCol;     /// Left column index.
    sal_uInt16          mnLX;       /// X offset in left column (1/1024 of column width).
    sal_uInt16          mnTRow;     /// Top row index.
    sal_uInt16          mnTY;       /// Y offset in top row (1/256 of row height).
    sal_uInt16          mnRCol;     /// Right column index.
    sal_uInt16          mnRX;       /// X offset in right column (1/1024 of column width).
    sal_uInt16          mnBRow;     /// Bottom row index.
    sal_uInt16          mnBY;       /// Y offset in bottom row (1/256 of row height).

    explicit            XclImpEscherAnchor( SCTAB nScTab );
};

SvStream& operator>>( SvStream& rStrm, XclImpEscherAnchor& rAnchor );

// ----------------------------------------------------------------------------

/** Contains all information of an Escher object.
    @descr  This is the Escher object itself (XclImpEscherObj) and the position
    in the Calc document (XclEscherAnchor). */
class XclImpObjData
{
public:
    /** Takes ownership of pObj. */
    explicit            XclImpObjData( XclImpEscherObj* pEscherObj );

    /** Sets the passed Escher object, deletes the old. */
    void                SetObj( XclImpEscherObj* pEscherObj );

    /** Returns the Escher object, if present. */
    inline XclImpEscherObj* GetObj() { return mxEscherObj.get(); }
    /** Returns the anchor data. */
    inline XclEscherAnchor& GetAnchor() { return maAnchor; }

    /** Returns true, if the passed stream position is part of the current object. */
    bool                ContainsStrmPos( ULONG nStrmPos ) const;

private:
    typedef ::std::auto_ptr< XclImpEscherObj > XclImpEscherObjPtr;

    XclEscherAnchor     maAnchor;       /// The sheet position of the object.
    XclImpEscherObjPtr  mxEscherObj;    /// The Escher object itself.
};

// ----------------------------------------------------------------------------

/** This list contains all read Escher objects with their anchor positions. */
class XclImpEscherObjList : ScfNoCopy
{
public:
    /** Returns the number of contained objects. */
    inline ULONG        GetObjCount() const { return maObjDataList.Count(); }

    /** Appends the passed object to the list. Takes ownership of the object. */
    void                AppendObj( XclImpEscherObj* pEscherObj );
    /** Appends the passed object as dummy (does not use it later). Takes ownership of the object. */
    void                AppendDummyObj( XclImpEscherObj* pEscherObj );
    /** Replaces the Escher object of the last inserted object data (or appends, if list is empty). */
    void                ReplaceLastObj( XclImpEscherObj* pEscherObj );

    /** Returns the object in the specified sheet by Excel object identifier. */
    XclImpEscherObj*    GetObj( SCTAB nScTab, sal_uInt16 nObjId ) const;
    /** Returns the object at the specified Escher stream position. */
    XclImpEscherObj*    GetObj( ULONG nStrmPos ) const;
    /** Returns the last inserted Escher object in the list. */
    XclImpEscherObj*    GetLastObj() const;

    /** Returns the anchor of the object at the passed Escher stream position. */
    XclEscherAnchor*    GetAnchor( ULONG nStrmPos ) const;

    /** Inserts all objects into the Calc document. */
    void                Apply( ScfProgressBar& rProgress );

private:
    /** Updates the cache data with the last inserted object for searching. */
    void                UpdateCache();
    /** Returns the object data of the Escher object at the specified Escher stream position. */
    XclImpObjData*      FindObjData( ULONG nStrmPos ) const;

private:
    /** Stores data for each sheet to speed up the search for objects. */
    struct XclCacheEntry
    {
        ULONG               mnListIdx;      /// List index of first object in the sheet.
        ULONG               mnStrmPos;      /// First Escher stream position of the objects in the sheet.
        inline explicit     XclCacheEntry( ULONG nListIdx, ULONG nStrmPos ) :
                                mnListIdx( nListIdx ), mnStrmPos( nStrmPos ) {}
    };

    typedef ScfDelList< XclImpObjData >     XclImpObjDataList;
    typedef ::std::vector< XclCacheEntry >  XclCacheVec;

    XclImpObjDataList   maObjDataList;      /// The list of all object data structs.
    XclCacheVec         maObjCache;         /// Caches data to speed up object search.
};

// Escher stream conversion ===================================================

class XclImpObjectManager;

/** Derived from SvxMSDffManager, contains core implementation of Escher stream import. */
class XclImpDffManager : public SvxMSDffManager, protected XclImpRoot
{
public:
    explicit            XclImpDffManager(
                            const XclImpRoot& rRoot, XclImpObjectManager& rObjManager,
                            long nOffsDgg, SvStream* pStData, SdrModel* pSdrModel,
                            long nApplicationScale );

    /** Converts an Escher object and creates and attaches the corresponding SdrObj. */
    void                SetSdrObject(
                            XclImpEscherObj* pEscherObj,
                            ULONG nId,
                            SvxMSDffImportData& rData );

    /** Creates the SdrOle2Obj for the passed Escher OLE object from the OLE storage in the Excel file.
        @return  true = SdrOle2Obj successfully created. */
    bool                CreateSdrOleObj( XclImpEscherOle& rOleObj );

protected:
    /** Reads the client anchor from the Escher stream and sets it at the correct Escher object. */
    virtual void        ProcessClientAnchor2( SvStream& rStrm, DffRecordHeader&, void*, DffObjData& rObjData );
    /** Processes an Escher object, reads properties from Escher stream. */
    virtual SdrObject*  ProcessObj(
                            SvStream&,
                            DffObjData& rObjData,
                            void* pData,
                            Rectangle& rTextRect,
                            SdrObject* pRetSdrObj = 0 );
    /** Returns the BLIP stream position, based on the passed Escher stream position. */
    virtual ULONG       Calc_nBLIPPos( ULONG nOrgVal, ULONG nStreamPos ) const;
    /** Returns a color from the Excel color palette. */
    virtual FASTBOOL    GetColorFromPalette( USHORT nIndex, Color& rColor ) const;
    /** Returns true, if the object with the passed shape ID contains any text data. */
    virtual sal_Bool    ShapeHasText( ULONG nShapeId, ULONG nFilePos ) const;

private:
    XclImpObjectManager& mrObjManager;  /// The Excel object manager.
    sal_uInt32          mnOleImpFlags;  /// Application OLE import settings.
};

// The object manager =========================================================

class XclImpOcxConverter;

/** Stores all drawing and OLE objects and additional data related to these objects. */
class XclImpObjectManager : protected XclImpRoot
{
public:
    explicit            XclImpObjectManager( const XclImpRoot& rRoot );
    virtual             ~XclImpObjectManager();

    // *** Escher stream *** --------------------------------------------------

    /** Returns true, if the Escher stream contains any data. */
    inline bool         HasEscherStream() { return maStreamConsumer.HasData(); }
    /** Returns the Escher stream. */
    inline SvStream&    GetEscherStream() { return maStreamConsumer.GetStream(); }

    // *** Escher objects *** -------------------------------------------------

    /** Returns the number of contained objects. */
    inline ULONG        GetEscherObjCount() const { return maEscherObjList.GetObjCount(); }

    /** Returns the object in the specified sheet by Excel object identifier. */
    const XclImpEscherObj* GetEscherObj( SCTAB nScTab, sal_uInt16 nObjId ) const;
    /** Returns access to the object in the specified sheet by Excel object identifier. */
    XclImpEscherObj*    GetEscherObjAcc( SCTAB nScTab, sal_uInt16 nObjId );

    /** Returns the object at the specified Escher stream position. */
    const XclImpEscherObj* GetEscherObj( ULONG nStrmPos ) const;
    /** Returns access to the object at the specified Escher stream position. */
    XclImpEscherObj*    GetEscherObjAcc( ULONG nStrmPos );

    /** Returns the last inserted object in the list. */
    const XclImpEscherObj* GetLastEscherObj() const;
    /** Returns access to the last inserted object in the list. */
    XclImpEscherObj*    GetLastEscherObjAcc();

    /** Returns the anchor of the object at the passed Escher stream position. */
    const XclEscherAnchor* GetEscherAnchor( ULONG nStrmPos ) const;
    /** Returns access to the anchor of the object at the passed Escher stream position. */
    XclEscherAnchor*    GetEscherAnchorAcc( ULONG nStrmPos );

    // *** Text boxes *** -----------------------------------------------------

    /** Returns the textbox object at the specified Escher stream position. */
    const XclImpEscherTxo* GetEscherTxo( ULONG nStrmPos ) const;
    /** Returns access to the textbox object at the specified Escher stream position. */
    XclImpEscherTxo*    GetEscherTxoAcc( ULONG nStrmPos );

    /** Returns the note object in the specified sheet by Excel object identifier. */
    const XclImpEscherNote* GetEscherNote( SCTAB nScTab, sal_uInt16 nObjId ) const;

    // *** Chart *** ----------------------------------------------------------

    /** Returns true, if the current object is a chart. */
    bool                IsCurrObjChart() const;

    /** Returns the chart data of the current chart object, or 0, if there is no chart. */
    XclImpChart*        GetCurrChartData();
    /** Sets a new chart data (with the passed chart type) at the current object.
        @descr  Reads additional properies of the chart type from stream.
        @return  The new chart data. The old chart data, returned by GetCurrChartData(),
        is no longer valid. */
    XclImpChart*        ReplaceChartData( XclImpStream& rStrm, XclChartType eNewType );

    /** Inserts a new chart object.
        @descr  Used to import sheet charts, which do not have a corresponding OBJ record. */
    void                StartNewChartObj();

    // *** OLE / controls *** -------------------------------------------------

    /** Creates the SdrObj for an OLE Escher object.
        @descr  The passed object may be a common OLE object, then this function creates
        the internal OLE stream. Or it is a form control, then this function reads the
        control formatting data from the 'Ctls' stream. */
    bool                CreateSdrObj( XclImpEscherOle& rOleObj );
    /** Creates the SdrObj for an old-fashioned Escher control object. */
    bool                CreateSdrObj( XclImpEscherTbxCtrl& rCtrlObj );

    // *** Read Excel records *** ---------------------------------------------

    /** Reads the MSODRAWINGGROUP record. */
    void                ReadMsodrawinggroup( XclImpStream& rStrm );
    /** Reads the MSODRAWING record. */
    void                ReadMsodrawing( XclImpStream& rStrm );
    /** Reads the MSODRAWINGSELECTION record. */
    void                ReadMsodrawingselection( XclImpStream& rStrm );

    /** Reads the OBJ record. */
    void                ReadObj( XclImpStream& rStrm );
    /** Reads the TXO record. */
    void                ReadTxo( XclImpStream& rStrm );

    // *** Misc *** -----------------------------------------------------------

    /** Lets the object manager add a dummy object, before the next object is read. */
    inline void         InsertDummyObj() { mbStartWithDummy = true; }

    /** Returns the DFF manager (Escher stream converter). Don't call before the Escher stream is read. */
    XclImpDffManager&   GetDffManager();
    /** Updates the connector rules of the passed object in the solver container. */
    void                UpdateConnectorRules( const DffObjData& rObjData, SdrObject& rSdrObj );

    /** Sets the object with the passed identification to be ignored on import. */
    void                SetSkipObj( SCTAB nScTab, sal_uInt16 nObjId );

    /** Inserts all objects into the Calc document. */
    void                Apply( ScfProgressBar& rProgress );

    // ------------------------------------------------------------------------
private:
    /** Appends the passed object to the list. */
    void                AppendEscherObj( XclImpEscherObj* pEscherObj );
    /** Replaces the last object in the list with the passed object. */
    void                ReplaceEscherObj( XclImpEscherObj* pEscherObj );

    /** Reads the ftCmo sub structure (common object data) in an OBJ record.
        @return  The new Escher object. */
    void                ReadObjFtCmo( XclImpStream& rStrm );
    /** Reads the ftPioGrbit sub structure (object option flags) in an OBJ record. */
    void                ReadObjFtPioGrbit( XclImpStream& rStrm );
    /** Reads the ftPictFmla sub structure (OLE link formula) in an OBJ record. */
    void                ReadObjFtPictFmla( XclImpStream& rStrm, sal_uInt16 nRecSize );

    /** Reads a sub record for TBX form controls in an OBJ record. */
    void                ReadObjTbxSubRec( XclImpStream& rStrm, sal_uInt16 nSubRecId );

    /** Returns the OCX converter (OCX form controls converter). */
    XclImpOcxConverter& GetOcxConverter();

    /** Returns the solver container (for connector rules). */
    SvxMSDffSolverContainer& GetSolverContainer();
    /** Returns the first connector rule from the solver container. */
    SvxMSDffConnectorRule* GetFirstConnectorRule();
    /** Returns the next connector rule from the solver container. */
    SvxMSDffConnectorRule* GetNextConnectorRule();

    /** Identifies an Escher object to skip on import (will not be inserted into the Calc document). */
    struct XclSkipObj
    {
        SCTAB               mnScTab;        /// Calc sheet index.
        sal_uInt16          mnObjId;        /// Excel object identifier.
        inline explicit     XclSkipObj( SCTAB nScTab, sal_uInt16 nObjId ) :
                                mnScTab( nScTab ), mnObjId( nObjId ) {}
    };

    typedef ::std::auto_ptr< XclImpDffManager >         XclImpDffManagerPtr;
    typedef ::std::auto_ptr< SvxMSDffSolverContainer >  SolverContainerPtr;
    typedef ::std::auto_ptr< XclImpOcxConverter >       XclImpOcxConverterPtr;
    typedef ::std::vector< XclSkipObj >                 XclSkipObjVec;

    XclImpStreamConsumer maStreamConsumer;  /// Collects the entire Escher stream.
    XclImpEscherObjList maEscherObjList;    /// Contains all Escher objects.
    SolverContainerPtr  mxSolverContainer;  /// The solver container for connector rules.
    XclImpDffManagerPtr mxDffManager;       /// The Escher stream converter.
    XclImpOcxConverterPtr mxOcxConverter;   /// The form controls converter.
    XclSkipObjVec       maSkipObjVec;       /// All Escher objects to skip.

    bool                mbStartWithDummy;   /// true = Insert a dummy object before the next Escher object.
};

// Escher property set helper =================================================

/** This class reads an Escher property set (msofbtOPT record).
    @descr  It can return separate property values or an item set which contains
    items translated from these properties. */
class XclImpEscherPropSet
{
public:
    explicit            XclImpEscherPropSet( const XclImpDffManager& rDffManager );

    /** Reads an Escher property set from the stream.
        @descr  The stream must point to the start of an Escher record containing properties. */
    void                Read( XclImpStream& rStrm );

    /** Returns the specified property or the default value, if not extant. */
    sal_uInt32          GetPropertyValue( sal_uInt16 nPropId, sal_uInt32 nDefault = 0 ) const;

    /** Translates the properties and fills the item set. */
    void                FillToItemSet( SfxItemSet& rItemSet ) const;

private:
    typedef ::std::auto_ptr< SvMemoryStream > SvMemoryStreamPtr;

    DffPropertyReader   maPropReader;   /// Reads the properties from an SvStream.
    SvMemoryStreamPtr   mxMemStrm;      /// Helper stream.
};

XclImpStream& operator>>( XclImpStream& rStrm, XclImpEscherPropSet& rPropSet );

// ============================================================================

#endif

