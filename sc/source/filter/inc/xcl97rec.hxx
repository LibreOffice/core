/*************************************************************************
 *
 *  $RCSfile: xcl97rec.hxx,v $
 *
 *  $Revision: 1.32 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 13:39:35 $
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

#ifndef _XCL97REC_HXX
#define _XCL97REC_HXX

#ifndef SC_XLESCHER_HXX
#include "xlescher.hxx"
#endif

#include "excrecds.hxx"
#include "xcl97esc.hxx"

struct SingleRefData;
struct ScExtTabOptions;

//___________________________________________________________________

// --- class XclMsodrawing_Base --------------------------------------

class XclMsodrawing_Base
{
protected:
        XclEscher*              pEscher;
        ULONG                   nStartPos;      // position in OffsetMap
        ULONG                   nStopPos;       // position in OffsetMap

public:
                                XclMsodrawing_Base( XclEscher& rEscher, ULONG nInitialSize = 0 );
    virtual                     ~XclMsodrawing_Base();

    inline  XclEscher*          GetEscher() const   { return pEscher; }
    inline  XclEscherEx*        GetEscherEx() const { return pEscher->GetEx(); }
            void                UpdateStopPos();
            ULONG               GetDataLen() const;
};


// --- class XclMsodrawinggroup --------------------------------------

class XclMsodrawinggroup : public XclMsodrawing_Base, public ExcRecord
{
private:

    virtual void                SaveCont( XclExpStream& rStrm );

public:
                                XclMsodrawinggroup( RootData& rRoot,
                                    UINT16 nEscherType = 0 );
    virtual                     ~XclMsodrawinggroup();

    virtual UINT16              GetNum() const;
    virtual ULONG               GetLen() const;
};


// --- class XclMsodrawing -------------------------------------------

class XclMsodrawing : public XclMsodrawing_Base, public ExcRecord
{
private:

    virtual void                SaveCont( XclExpStream& rStrm );

public:
                                XclMsodrawing(
                                    const XclExpRoot& rRoot,
                                    UINT16 nEscherType = 0,
                                    ULONG nInitialSize = 0 );
    virtual                     ~XclMsodrawing();

    virtual UINT16              GetNum() const;
    virtual ULONG               GetLen() const;
};


// --- class XclObjList ----------------------------------------------

class XclObj;
class XclMsodrawing;

class XclObjList : public List, public ExcEmptyRec, protected XclExpRoot
{
private:
        XclMsodrawing*          pMsodrawingPerSheet;
        XclMsodrawing*          pSolverContainer;

public:
                                XclObjList( const XclExpRoot& rRoot );
    virtual                     ~XclObjList();

            XclObj*             First() { return (XclObj*) List::First(); }
            XclObj*             Next()  { return (XclObj*) List::Next(); }

                                /// return: 1-based ObjId
                                ///! count>=0xFFFF: Obj will be deleted, return 0
            UINT16              Add( XclObj* );

    inline  XclMsodrawing*      GetMsodrawingPerSheet() { return pMsodrawingPerSheet; }

                                /// close groups and DgContainer opened in ctor
            void                EndSheet();

    virtual void                Save( XclExpStream& rStrm );
};


// --- class XclObj --------------------------------------------------

class XclTxo;
class SdrTextObj;

class XclObj : public XclExpRecord
{
protected:
        XclMsodrawing*      pMsodrawing;
        XclMsodrawing*      pClientTextbox;
        XclTxo*             pTxo;
        sal_uInt16          mnObjType;
        UINT16              nObjId;
        UINT16              nGrbit;
        BOOL                bFirstOnSheet;

        bool                    mbOwnEscher;    /// true = Escher part created on the fly.

    /** @param bOwnEscher  If set to true, this object will create its escher data.
        See SetOwnEscher() for details. */
    explicit                    XclObj( const XclExpRoot& rRoot, sal_uInt16 nObjType, bool bOwnEscher = false );

                                // overwritten for writing MSODRAWING record
    virtual void                WriteBody( XclExpStream& rStrm );
    virtual void                WriteSubRecs( XclExpStream& rStrm );
            void                SaveTextRecs( XclExpStream& rStrm );

public:
    virtual                     ~XclObj();

    inline sal_uInt16           GetObjType() const { return mnObjType; }

    inline  void                SetId( UINT16 nId ) { nObjId = nId; }

    inline  void                SetLocked( BOOL b )
                                    { b ? nGrbit |= 0x0001 : nGrbit &= ~0x0001; }
    inline  void                SetPrintable( BOOL b )
                                    { b ? nGrbit |= 0x0010 : nGrbit &= ~0x0010; }
    inline  void                SetAutoFill( BOOL b )
                                    { b ? nGrbit |= 0x2000 : nGrbit &= ~0x2000; }
    inline  void                SetAutoLine( BOOL b )
                                    { b ? nGrbit |= 0x4000 : nGrbit &= ~0x4000; }

                                // set corresponding Excel object type in OBJ/ftCmo
            void                SetEscherShapeType( UINT16 nType );
    inline  void                SetEscherShapeTypeGroup() { mnObjType = EXC_OBJ_CMO_GROUP; }

    /** If set to true, this object has created its own escher data.
        @descr  This causes the function EscherEx::EndShape() to not post process
        this object. This is used i.e. for form controls. They are not handled in
        the svx base code, so the XclExpEscherOcxCtrl c'tor creates the escher data
        itself. The svx base code does not receive the correct shape ID after the
        EscherEx::StartShape() call, which would result in deleting the object in
        EscherEx::EndShape(). */
    inline void                 SetOwnEscher( bool bOwnEscher = true ) { mbOwnEscher = bOwnEscher; }
    /** Returns true, if the object has created the escher data itself.
        @descr  See SetOwnEscher() for details. */
    inline bool                 IsOwnEscher() const { return mbOwnEscher; }

                                //! actually writes ESCHER_ClientTextbox
            void                SetText( const XclExpRoot& rRoot, const SdrTextObj& rObj );

    inline  void                UpdateStopPos();

    virtual void                Save( XclExpStream& rStrm );
};


inline void XclObj::UpdateStopPos()
{
    if ( pClientTextbox )
        pClientTextbox->UpdateStopPos();
    else
        pMsodrawing->UpdateStopPos();
}


// --- class XclObjComment -------------------------------------------

class XclObjComment : public XclObj
{
public:
                                XclObjComment( const XclExpRoot& rRoot,
                                    const ScAddress& rPos, const String& rStr, bool bVisible );
    virtual                     ~XclObjComment();

    virtual void                Save( XclExpStream& rStrm );
};


// --- class XclObjDropDown ------------------------------------------

class XclObjDropDown : public XclObj
{
private:
    BOOL                        bIsFiltered;

    virtual void                WriteSubRecs( XclExpStream& rStrm );

protected:
public:
                                XclObjDropDown( const XclExpRoot& rRoot, const ScAddress& rPos, BOOL bFilt );
    virtual                     ~XclObjDropDown();
};


// --- class XclTxo --------------------------------------------------

class SdrTextObj;

class XclTxo : public ExcRecord
{
public:
                                XclTxo( const String& rString, sal_uInt16 nFontIx = EXC_FONT_APP );
                                XclTxo( const XclExpRoot& rRoot, const SdrTextObj& rEditObj );

    inline void                 SetHorAlign( XclTxoHorAlign eHorAlign ) { meHorAlign = eHorAlign; }
    inline void                 SetVerAlign( XclTxoVerAlign eVerAlign ) { meVerAlign = eVerAlign; }

    virtual void                Save( XclExpStream& rStrm );

    virtual UINT16              GetNum() const;
    virtual ULONG               GetLen() const;

private:
    virtual void                SaveCont( XclExpStream& rStrm );

private:
    XclExpStringPtr             mpString;       /// Text and formatting data.
    XclTxoHorAlign              meHorAlign;     /// Horizontal alignment.
    XclTxoVerAlign              meVerAlign;     /// Vertical alignment.
    XclTxoRotation              meRotation;     /// Text rotation.
};


// --- class XclObjOle -----------------------------------------------

class XclObjOle : public XclObj
{
private:

        const SdrObject&    rOleObj;
        SvStorage*          pRootStorage;

    virtual void                WriteSubRecs( XclExpStream& rStrm );

public:
                                XclObjOle( const XclExpRoot& rRoot, const SdrObject& rObj );
    virtual                     ~XclObjOle();

    virtual void                Save( XclExpStream& rStrm );
};


// ----------------------------------------------------------------------------

/** Helper to manage controls linked to the sheet. */
class XclExpCtrlLinkHelper : protected XclExpRoot
{
public:
    explicit                    XclExpCtrlLinkHelper( const XclExpRoot& rRoot );

    /** Sets the address of the control's linked cell. */
    void                        SetCellLink( const ScAddress& rCellLink );
    /** Sets the address of the control's linked source cell range. */
    void                        SetSourceRange( const ScRange& rSrcRange );

protected:
    /** Returns the Excel token array of the cell link, or 0, if no link present. */
    inline const ExcUPN*        GetCellLinkTokArr() const { return mpCellLink.get(); }
    /** Returns the Excel token array of the source range, or 0, if no link present. */
    inline const ExcUPN*        GetSourceRangeTokArr() const { return mpSrcRange.get(); }
    /** Returns the number of entries in the source range, or 0, if no source set. */
    inline sal_uInt16           GetSourceEntryCount() const { return mnEntryCount; }

    /** Writes a sheet link formula with special style only valid in OBJ records. */
    void                        WriteFormula( XclExpStream& rStrm, const ExcUPN& rTokArr ) const;

private:
    typedef ::std::auto_ptr< ExcUPN > XclExpTokArrPtr;

    XclExpTokArrPtr             CreateTokenArray( const ScTokenArray& rScTokArr ) const;
    XclExpTokArrPtr             CreateTokenArray( const ScAddress& rPos ) const;
    XclExpTokArrPtr             CreateTokenArray( const ScRange& rRange ) const;

private:
    XclExpTokArrPtr             mpCellLink;     /// Formula for linked cell.
    XclExpTokArrPtr             mpSrcRange;     /// Formula for source data range.
    sal_uInt16                  mnEntryCount;   /// Number of entries in source range.
};


// ----------------------------------------------------------------------------

#if EXC_EXP_OCX_CTRL

/** Represents an OBJ record for an OCX form control. */
class XclExpObjOcxCtrl : public XclObj, public XclExpCtrlLinkHelper
{
public:
                                XclExpObjOcxCtrl(
                                    const XclExpRoot& rRoot,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::drawing::XShape >& rxShape,
                                    const String& rClassName,
                                    sal_uInt32 nStrmStart, sal_uInt32 nStrmSize );

private:
    virtual void                WriteSubRecs( XclExpStream& rStrm );

private:
    String                      maClassName;        /// Class name of the control.
    sal_uInt32                  mnStrmStart;        /// Start position in 'Ctls' stream.
    sal_uInt32                  mnStrmSize;         /// Size in 'Ctls' stream.
};

#else

/** Represents an OBJ record for an TBX form control. */
class XclExpObjTbxCtrl : public XclObj, public XclExpCtrlLinkHelper
{
public:
                                XclExpObjTbxCtrl(
                                    const XclExpRoot& rRoot,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::drawing::XShape >& rxShape,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::awt::XControlModel >& rxCtrlModel );
    virtual                     ~XclExpObjTbxCtrl();

private:
    virtual void                WriteSubRecs( XclExpStream& rStrm );

private:
    ScfInt16Vec                 maMultiSel;     /// Indexes of all selected entries in a multi selection.
    sal_Int32                   mnHeight;       /// Height of the control.
    sal_uInt16                  mnState;        /// Checked/unchecked state.
    sal_Int16                   mnLineCount;    /// Combobox dropdown line count.
    sal_Int16                   mnSelEntry;     /// Selected entry in combobox (1-based).
    bool                        mb3DStyle;      /// true = 3D style.
    bool                        mbMultiSel;     /// true = Multi selection in listbox.
};

#endif

// --- class XclObjAny -----------------------------------------------

class XclObjAny : public XclObj
{
private:
    virtual void                WriteSubRecs( XclExpStream& rStrm );

public:
                                XclObjAny( const XclExpRoot& rRoot );
    virtual                     ~XclObjAny();

    virtual void                Save( XclExpStream& rStrm );
};


// ============================================================================

/** Represents a NOTE record containing the author and the note object ID.
    @descr  Creates the note Escher object internally. */
class XclExpNote : public XclExpRecord
{
private:
    XclExpString                maAuthor;       /// Name of the author.
    ScAddress                   maPos;          /// Cell address of the note.
    sal_uInt16                  mnObjId;        /// Escher object ID.
    bool                        mbVisible;      /// true = permanently visible.

public:
    /** Constructs a NOTE record from the passed note object and/or the text.
        @descr  The additional text will be separated from the note text with an empty line.
        Creates the Escher object containing the drawing information and the note text.
        @param rPos  The Calc cell address of the note.
        @param pScNote  The Calc note object.
        @param rAddText  Additional text appended to the note text. */
    explicit                    XclExpNote(
                                    const XclExpRoot& rRoot,
                                    const ScAddress& rPos,
                                    const ScPostIt* pScNote,
                                    const String& rAddText );

    /** Writes the NOTE record, if the respective Escher object is present. */
    virtual void                Save( XclExpStream& rStrm );

private:
    /** Writes the body of the NOTE record. */
    virtual void                WriteBody( XclExpStream& rStrm );
};


/** A list of NOTE record objects. */
typedef XclExpRecordList< XclExpNote > XclExpNoteList;


// ============================================================================


// --- class ExcBof8_Base --------------------------------------------

class ExcBof8_Base : public ExcBof_Base
{
protected:
        UINT32              nFileHistory;       // bfh
        UINT32              nLowestBiffVer;     // sfo

    virtual void                SaveCont( XclExpStream& rStrm );

public:
                                ExcBof8_Base();

    virtual UINT16              GetNum() const;
    virtual ULONG               GetLen() const;
};


// --- class ExcBofW8 ------------------------------------------------
// Header Record fuer WORKBOOKS

class ExcBofW8 : public ExcBof8_Base
{
public:
                                ExcBofW8();
};


// --- class ExcBof8 -------------------------------------------------
// Header Record fuer WORKSHEETS

class ExcBof8 : public ExcBof8_Base
{
public:
                                ExcBof8();
};


// --- class ExcBofC8 ------------------------------------------------
// Header Record fuer CHARTs

class ExcBofC8 : public ExcBof8_Base
{
public:
                                ExcBofC8();
};

// ============================================================================

/** Represents a LABELSST record for a formatted or unformatted text cell. */
class XclExpLabelSst : public ExcCell
{
public:
    explicit                    XclExpLabelSst(
                                    const XclExpRoot& rRoot,
                                    const ScAddress& rPos,
                                    const String& rText,
                                    const ScPatternAttr* pPattern );
    explicit                    XclExpLabelSst(
                                    const XclExpRoot& rRoot,
                                    const ScAddress& rPos,
                                    const ScEditCell& rEditCell,
                                    const ScPatternAttr* pPattern );

    virtual UINT16              GetNum() const;

private:
    virtual void                SaveDiff( XclExpStream& rStrm );    // instead of SaveCont()
    virtual ULONG               GetDiffLen() const;

private:
    sal_uInt32                  mnSstIndex;     /// Index to string in SST.
};


// ============================================================================

// --- class ExcBundlesheet8 -----------------------------------------

class ExcBundlesheet8 : public ExcBundlesheetBase
{
private:
    XclExpString                aUnicodeName;

    virtual void                SaveCont( XclExpStream& rStrm );

public:
                                ExcBundlesheet8( RootData& rRootData, UINT16 nTab );
                                ExcBundlesheet8( const String& rString );

    virtual ULONG               GetLen() const;
};


// --- class ExcWindow18 ---------------------------------------------

class ExcWindow18 : public ExcRecord
{
private:
    UINT16                  nCurrTable;
    UINT16                  nSelTabs;

    virtual void            SaveCont( XclExpStream& rStrm );

public:
                            ExcWindow18( RootData& rRootData );

    virtual UINT16          GetNum( void ) const;
    virtual ULONG           GetLen( void ) const;
};


// --- class ExcPane8 ------------------------------------------------

class ExcPane8 : public ExcRecord
{
private:
    UINT16                      nSplitX;
    UINT16                      nSplitY;
    UINT16                      nLeftCol;
    UINT16                      nTopRow;
    UINT16                      nActivePane;

    virtual void                SaveCont( XclExpStream& rStrm );

public:
                                ExcPane8( const ScExtTabOptions& rTabOptions );

    virtual UINT16              GetNum() const;
    virtual ULONG               GetLen() const;
};


// --- class ExcWindow28 ---------------------------------------------

class ExcWindow28 : public ExcRecord, protected XclExpRoot
{
private:
    ExcPane8*                   pPaneRec;
    UINT32                      nGridColorSer;
    UINT16                      nFlags;
    UINT16                      nLeftCol;
    UINT16                      nTopRow;
    UINT16                      nActiveCol;
    UINT16                      nActiveRow;
    BOOL                        bHorSplit       : 1;
    BOOL                        bVertSplit      : 1;

    virtual void                SaveCont( XclExpStream& rStrm );

public:
                                ExcWindow28( const XclExpRoot& rRoot, UINT16 nTab );
    virtual                     ~ExcWindow28();

    virtual void                Save( XclExpStream& rStrm );
    virtual UINT16              GetNum() const;
    virtual ULONG               GetLen() const;
};


// --- class XclObproj -----------------------------------------------

class XclObproj : public ExcRecord
{
public:
    virtual UINT16              GetNum() const;
    virtual ULONG               GetLen() const;
};


// --- class XclDConRef ----------------------------------------------

class XclDConRef : public ExcRecord
{
private:
    ScRange                 aSourceRange;
    XclExpString*           pWorkbook;

    virtual void            SaveCont( XclExpStream& rStrm );

public:
                            XclDConRef( const ScRange& rSrcR, const String& rWB );
    virtual                 ~XclDConRef();

    virtual UINT16          GetNum() const;
    virtual ULONG           GetLen() const;
};


// --- class XclExpCellMerging ---------------------------------------

struct XclExpMergedCell
{
    sal_uInt32              mnXFId;
    UINT16                  nCol1;
    UINT16                  nCol2;
    UINT16                  nRow1;
    UINT16                  nRow2;

    inline                  XclExpMergedCell( UINT16 nC1, UINT16 nC2, UINT16 nR1, UINT16 nR2, sal_uInt32 nXFId ) :
                                nCol1( nC1 ), nCol2( nC2 ), nRow1( nR1 ), nRow2( nR2 ), mnXFId( nXFId ) {}
};

inline XclExpStream& operator<<( XclExpStream& rStrm, const XclExpMergedCell& rCell )
{
    return (rStrm << rCell.nRow1 << rCell.nRow2 << rCell.nCol1 << rCell.nCol2);
}



class XclExpCellMerging : public ExcEmptyRec
{
private:
    List                        aCellList;

    inline XclExpMergedCell*    FirstCell() { return (XclExpMergedCell*) aCellList.First(); }
    inline XclExpMergedCell*    NextCell()  { return (XclExpMergedCell*) aCellList.Next(); }
    inline XclExpMergedCell*    GetCell( ULONG nIndex )
                                    { return (XclExpMergedCell*) aCellList.GetObject( nIndex ); }
    inline void                 AppendCell( XclExpMergedCell* pNewCell )
                                    { aCellList.Insert( pNewCell, LIST_APPEND ); }

public:
    void                        Append( UINT16 nCol1, UINT16 nColCnt, UINT16 nRow1, UINT16 nRowCnt, sal_uInt32 nXFId );
    BOOL                        FindNextMerge( const ScAddress& rPos, UINT16& rnCol );
    BOOL                        FindMergeBaseXF( const ScAddress& rPos, sal_uInt32& rnXFId, UINT16& rnColCount );
    inline BOOL                 FindMergeBaseXF( const ScAddress& rPos, sal_uInt32& rnXFId );

    virtual void                Save( XclExpStream& rStrm );
};

inline BOOL XclExpCellMerging::FindMergeBaseXF( const ScAddress& rPos, sal_uInt32& rnXFId )
{
    UINT16 nCols;
    return FindMergeBaseXF( rPos, rnXFId, nCols );
}



// ---- class XclCodename --------------------------------------------

class XclCodename : public ExcRecord
{
private:
    XclExpString                aName;
    virtual void                SaveCont( XclExpStream& rStrm );
public:
                                XclCodename( const String& );

    virtual UINT16              GetNum() const;
    virtual ULONG               GetLen() const;
};


// ---- Scenarios ----------------------------------------------------
// - ExcEScenarioCell           a cell of a scenario range
// - ExcEScenario               all ranges of a scenario table
// - ExcEScenarioManager        list of scenario tables

class ExcEScenarioCell
{
private:
    UINT16                      nCol;
    UINT16                      nRow;
    XclExpString                sText;

protected:
public:
                                ExcEScenarioCell( UINT16 nC, UINT16 nR, const String& rTxt );

    inline ULONG                GetStringBytes()
                                    { return sText.GetSize(); }

    void                        WriteAddress( XclExpStream& rStrm );
    void                        WriteText( XclExpStream& rStrm );
};



class ExcEScenario : public ExcRecord, private List
{
private:
    ULONG                       nRecLen;
    XclExpString                sName;
    XclExpString                sComment;
    static XclExpString         sUsername;

    inline ExcEScenarioCell*    _First()    { return (ExcEScenarioCell*) List::First(); }
    inline ExcEScenarioCell*    _Next()     { return (ExcEScenarioCell*) List::Next(); }

    BOOL                        Append( UINT16 nCol, UINT16 nRow, const String& rTxt );

    virtual void                SaveCont( XclExpStream& rStrm );

protected:
public:
                                ExcEScenario( ScDocument& rDoc, UINT16 nTab );
    virtual                     ~ExcEScenario();

    virtual UINT16              GetNum() const;
    virtual ULONG               GetLen() const;
};



class ExcEScenarioManager : public ExcRecord, private List
{
private:
    UINT16                      nActive;

    inline ExcEScenario*        _First()    { return (ExcEScenario*) List::First(); }
    inline ExcEScenario*        _Next()     { return (ExcEScenario*) List::Next(); }

    inline void                 Append( ExcEScenario* pScen )
                                    { List::Insert( pScen, LIST_APPEND ); }

    virtual void                SaveCont( XclExpStream& rStrm );

protected:
public:
                                ExcEScenarioManager( ScDocument& rDoc, UINT16 nTab );
    virtual                     ~ExcEScenarioManager();

    virtual void                Save( XclExpStream& rStrm );

    virtual UINT16              GetNum() const;
    virtual ULONG               GetLen() const;
};


// ---- class XclProtection ------------------------------------------

class XclProtection : public ExcDummyRec
{
    // replacement for records PROTECT, SCENPROTECT, OBJPROTECT...
private:
    static const BYTE           pMyData[];
    static const ULONG          nMyLen;
public:
    virtual ULONG               GetLen( void ) const;
    virtual const BYTE*         GetData( void ) const;
};


// -------------------------------------------------------------------


class XclCalccount : public ExcRecord
{
private:
    UINT16                      nCount;
protected:
    virtual void                SaveCont( XclExpStream& rStrm );
public:
                                XclCalccount( const ScDocument& );

    virtual UINT16              GetNum() const;
    virtual ULONG               GetLen() const;
};




class XclIteration : public ExcRecord
{
private:
    UINT16                      nIter;
protected:
    virtual void                SaveCont( XclExpStream& rStrm );
public:
                                XclIteration( const ScDocument& );

    virtual UINT16              GetNum() const;
    virtual ULONG               GetLen() const;
};




class XclDelta : public ExcRecord
{
private:
    double                      fDelta;
protected:
    virtual void                SaveCont( XclExpStream& rStrm );
public:
                                XclDelta( const ScDocument& );

    virtual UINT16              GetNum() const;
    virtual ULONG               GetLen() const;
};


#endif // _XCL97REC_HXX
