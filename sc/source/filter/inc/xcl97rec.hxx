/*************************************************************************
 *
 *  $RCSfile: xcl97rec.hxx,v $
 *
 *  $Revision: 1.27 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-08 16:28:23 $
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
                                XclMsodrawing( RootData& rRoot,
                                    UINT16 nEscherType = 0,
                                    ULONG nInitialSize = 0 );
    virtual                     ~XclMsodrawing();

    virtual UINT16              GetNum() const;
    virtual ULONG               GetLen() const;
};


// --- class XclObjList ----------------------------------------------

class XclObj;
class XclMsodrawing;

class XclObjList : public List, public ExcEmptyRec, public ExcRoot
{
private:
        XclMsodrawing*          pMsodrawingPerSheet;
        XclMsodrawing*          pSolverContainer;

public:
                                XclObjList( RootData& rRoot );
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

class XclObj : public ExcRecord
{
protected:

    enum FtType {
        ftEnd       = 0x0000,
        // reserved: 0x01-0x03
        ftMacro     = 0x0004,
        ftButton    = 0x0005,
        ftGmo       = 0x0006,
        ftCf        = 0x0007,
        ftPioGrbit  = 0x0008,
        ftPictFmla  = 0x0009,
        ftCbls      = 0x000A,
        ftRbo       = 0x000B,
        ftSbs       = 0x000C,
        ftNts       = 0x000D,
        ftSbsFmla   = 0x000E,
        ftGboData   = 0x000F,
        ftEdoData   = 0x0010,
        ftRboData   = 0x0011,
        ftCblsdata  = 0x0012,
        ftLbsData   = 0x0013,
        ftCbsFmls   = 0x0014,
        ftCmo       = 0x0015
    };

    enum ObjType {
        otGroup         = 0x0000,
        otLine          = 0x0001,
        otRectangle     = 0x0002,
        otOval          = 0x0003,
        otArc           = 0x0004,
        otChart         = 0x0005,
        otText          = 0x0006,
        otButton        = 0x0007,
        otPicture       = 0x0008,
        otPolygon       = 0x0009,
        // reserved: 0x0A
        otCheckBox      = 0x000B,
        otOptionButtton = 0x000C,
        otEditBox       = 0x000D,
        otLabel         = 0x000E,
        otDialogBox     = 0x000F,
        otSpinner       = 0x0010,
        otScrollBar     = 0x0011,
        otListBox       = 0x0012,
        otGroupBox      = 0x0013,
        otComboBox      = 0x0014,
        // reserved: 0x15-0x18
        otComment       = 0x0019,
        // reserved: 0x1A-0x1D
        otMsOffDrawing  = 0x001E,
        // SC internal
        otUnknown       = 0xFFFF    // only for temporary use
    };

        XclMsodrawing*      pMsodrawing;
        XclMsodrawing*      pClientTextbox;
        XclTxo*             pTxo;
        ObjType             eObjType;
        UINT16              nObjId;
        UINT16              nGrbit;
        BOOL                bFirstOnSheet;

        bool                    mbOwnEscher;    /// true = Escher part created on the fly.

    /** @param bOwnEscher  If set to true, this object will create its escher data.
        See SetOwnEscher() for details. */
    explicit                    XclObj( ObjType eObjType, RootData& rRoot, bool bOwnEscher = false );

                                // overwritten for writing MSODRAWING record
    virtual void                SaveCont( XclExpStream& rStrm );
            void                SaveTextRecs( XclExpStream& rStrm );

public:
    virtual                     ~XclObj();

    inline  void                SetId( UINT16 nId ) { nObjId = nId; }

    inline  void                SetLocked( BOOL b )
                                    { b ? nGrbit |= 0x0001 : nGrbit &= ~0x0001; }
    inline  void                SetPrintable( BOOL b )
                                    { b ? nGrbit |= 0x0010 : nGrbit &= ~0x0010; }
    inline  void                SetAutoFill( BOOL b )
                                    { b ? nGrbit |= 0x2000 : nGrbit &= ~0x2000; }
    inline  void                SetAutoLine( BOOL b )
                                    { b ? nGrbit |= 0x4000 : nGrbit &= ~0x4000; }

                                // set corresponding ObjType
            void                SetEscherShapeType( UINT16 nType );
    inline  void                SetEscherShapeTypeGroup() { eObjType = otGroup; }

    /** If set to true, this object has created its own escher data. This causes the
        function EscherEx::EndShape() to not post process this object. This is used
        i.e. for form controls. They are not handled in the svx base code, so the
        XclExpObjControl c'tor creates the escher data itself. The svx base code does
        not receive the correct shape ID after the EscherEx::StartShape() call, which
        would result in deleting the object in EscherEx::EndShape(). */
    inline void                 SetOwnEscher( bool bOwnEscher = true ) { mbOwnEscher = bOwnEscher; }
    /** Returns true, if the object has created the escher data itself.
        See SetOwnEscher() for details. */
    inline bool                 IsOwnEscher() const { return mbOwnEscher; }

                                //! actually writes ESCHER_ClientTextbox
            void                SetText( RootData& rRoot, const SdrTextObj& rObj );

    inline  void                UpdateStopPos();

    virtual void                Save( XclExpStream& rStrm );

    virtual UINT16              GetNum() const;
    virtual ULONG               GetLen() const;
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
private:
    virtual void                SaveCont( XclExpStream& rStrm );

public:
                                XclObjComment( RootData& rRoot,
                                    const ScAddress& rPos, const String& rStr, bool bVisible );
    virtual                     ~XclObjComment();

    virtual void                Save( XclExpStream& rStrm );
    virtual ULONG               GetLen() const;
};


// --- class XclObjDropDown ------------------------------------------

class XclObjDropDown : public XclObj
{
private:
    BOOL                        bIsFiltered;

    virtual void                SaveCont( XclExpStream& rStrm );

protected:
public:
                                XclObjDropDown( RootData& rRoot, const ScAddress& rPos, BOOL bFilt );
    virtual                     ~XclObjDropDown();

    virtual ULONG               GetLen() const;     // GetNum() done by XclObj
};


// --- class XclTxo --------------------------------------------------

class SdrTextObj;

class XclTxo : public ExcRecord
{
    friend void XclObjComment::SaveCont( XclExpStream& );

private:

        XclExpUniString     aText;
        UINT16              nGrbit;
        UINT16              nRot;

    virtual void                SaveCont( XclExpStream& rStrm );

public:
                                XclTxo( const String& rStr );
                                XclTxo( const SdrTextObj& rEditObj );
    virtual                     ~XclTxo();

    virtual void                Save( XclExpStream& rStrm );

    virtual UINT16              GetNum() const;
    virtual ULONG               GetLen() const;
};


// --- class XclObjOle -----------------------------------------------

class XclObjOle : public XclObj
{
private:

        const SdrObject&    rOleObj;
        SvStorage*          pRootStorage;

    virtual void                SaveCont( XclExpStream& rStrm );

public:
                                XclObjOle( RootData& rRoot, const SdrObject& rObj );
    virtual                     ~XclObjOle();

    virtual void                Save( XclExpStream& rStrm );
    virtual ULONG               GetLen() const;
};


// ----------------------------------------------------------------------------

#if EXC_INCL_EXP_OCX

/** Represents an OBJ record for a form control. */
class XclExpObjControl : public XclObj
{
private:
    String                      maClassName;        /// Class name of the control.
    sal_uInt32                  mnStrmStart;        /// Start position in 'Ctls' stream.
    sal_uInt32                  mnStrmSize;         /// Size in 'Ctls' stream.

public:
                                XclExpObjControl(
                                    const XclRoot& rRoot,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::drawing::XShape >& rxShape,
                                    const String& rClassName,
                                    sal_uInt32 nStrmStart, sal_uInt32 nStrmSize );

    virtual sal_uInt32          GetLen() const;

private:
    virtual void                SaveCont( XclExpStream& rStrm );
};

#endif

// --- class XclObjAny -----------------------------------------------

class XclObjAny : public XclObj
{
private:
    virtual void                SaveCont( XclExpStream& rStrm );

public:
                                XclObjAny( RootData& rRoot );
    virtual                     ~XclObjAny();

    virtual void                Save( XclExpStream& rStrm );
    virtual ULONG               GetLen() const;
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

// --- class ExcLabelSst ---------------------------------------------

class ExcLabelSst : public ExcCell
{
private:
        UINT32              nIsst;      // Index in SST

    virtual void                SaveDiff( XclExpStream& rStrm );    // instead of SaveCont()
    virtual ULONG               GetDiffLen() const;

public:
                                ExcLabelSst(
                                    const ScAddress&        rPos,
                                    const ScPatternAttr*    pAttr,
                                    RootData&               rRoot,
                                    const String&           rText );
                                ExcLabelSst(
                                    const ScAddress&        rPos,
                                    const ScPatternAttr*    pAttr,
                                    RootData&               rRoot,
                                    const ScEditCell&       rEdCell );
    virtual                     ~ExcLabelSst();

    virtual UINT16              GetNum() const;
};


// --- class ExcBundlesheet8 -----------------------------------------

class ExcBundlesheet8 : public ExcBundlesheetBase
{
private:
    XclExpUniString             aUnicodeName;

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


// --- class XclCondFormat -------------------------------------------

class ScConditionalFormat;
class ScCondFormatEntry;
class ScRangeList;
struct RootData;
class XclCf;

class XclCondFormat : public ExcEmptyRec, protected ScfDelList< XclCf >
{
// writes multiple cf _and_ condfmt records!
private:
    const ScConditionalFormat&  rCF;
    ScRangeList*                pRL;
    UINT16                      nTabNum;
    ULONG                       nComplLen;

    void                        WriteCondfmt( XclExpStream& rStrm );

public:
                                XclCondFormat( const ScConditionalFormat&, ScRangeList*, RootData& );
                                    // takes ScRangeList, don't use it after this Ctor!
    virtual                     ~XclCondFormat();

    virtual void                Save( XclExpStream& rStrm );
};



class XclCf : public ExcRecord, protected XclExpRoot
{
private:
    friend XclCondFormat;

    sal_Char*                   pVarData;       // formats + formulas
    UINT16                      nVarLen;        // len of attributes + formulas
    UINT16                      nFormatLen;
    UINT16                      nFormLen1;
    UINT16                      nFormLen2;

    UINT8                       nType;          // formatting type
    UINT8                       nOp;            // formatting operator

    BOOL                        bHasStyle;
    UINT32                      nStart;

    BOOL                        bHasFont;
    UINT32                      nFontData1;
    UINT32                      nFontData2;
    UINT32                      nFontData3;
    UINT32                      nFontData4;
    UINT32                      nFontData5;
    UINT8                       nFontData6;
    BOOL                        bHasColor;
    UINT32                      nIcvTextSer;

    BOOL                        bHasLine;
    XclExpCellBorder            maBorder;

    BOOL                        bHasPattern;
    XclExpCellArea              maArea;

    virtual void                SaveCont( XclExpStream& rStrm );

public:
                                XclCf( const XclExpRoot& rRoot, const ScCondFormatEntry& );
    virtual                     ~XclCf();

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
    XclExpUniString*        pWorkbook;

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
    XclExpUniString             aName;
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
    XclExpUniString             sText;

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
    XclExpUniString             sName;
    XclExpUniString             sComment;
    static XclExpUniString      sUsername;

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
