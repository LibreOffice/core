/*************************************************************************
 *
 *  $RCSfile: xcl97rec.hxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: dr $ $Date: 2001-07-30 11:33:30 $
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

// --- class XclSstList ----------------------------------------------

class XclSstList : public List, public ExcEmptyRec
{
private:
    inline XclExpUniString*     _First()    { return (XclExpUniString*) List::First(); }
    inline XclExpUniString*     _Next()     { return (XclExpUniString*) List::Next(); }

public:
    inline                      XclSstList() {}
    virtual                     ~XclSstList();

            UINT32              Add( XclExpUniString* pStr );

    virtual void                Save( XclExpStream& rStrm );
};


// --- class XclMsodrawing_Base --------------------------------------

class XclMsodrawing_Base
{
protected:
        XclEscher*              pEscher;
        ULONG                   nStartPos;      // position in OffsetMap
        ULONG                   nStopPos;       // position in OffsetMap

public:
                                XclMsodrawing_Base( XclEscher& rEscher );
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
                                    UINT16 nEscherType = 0 );
    virtual                     ~XclMsodrawing();

    virtual UINT16              GetNum() const;
    virtual ULONG               GetLen() const;
};


// --- class XclObjList ----------------------------------------------

class XclObj;
class XclMsodrawing;

class XclObjList : public List, public ExcEmptyRec
{
private:
        XclMsodrawing*          pMsodrawingPerSheet;

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

                                XclObj( ObjType eObjType, RootData& rRoot );

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
                                    const ScAddress& rPos, const String& rStr );
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


// --- class XclNoteList ---------------------------------------------

class XclNote;

class XclNoteList : public List, public ExcEmptyRec
{
private:
public:
                                XclNoteList();
    virtual                     ~XclNoteList();

            XclNote*            First() { return (XclNote*) List::First(); }
            XclNote*            Next()  { return (XclNote*) List::Next(); }

            void                Add( XclNote* );

    virtual void                Save( XclExpStream& rStrm );
};


// --- class XclNote -------------------------------------------------

class XclNote : public ExcRecord
{
private:
        XclExpUniString     aAuthor;
        ScAddress           aPos;
        UINT16              nGrbit;
        UINT16              nObjId;

    virtual void            SaveCont( XclExpStream& rStrm );

public:
                            XclNote( RootData& rD, const ScAddress& rPos,
                                const String& rNoteText, const String& rNoteAuthor );
    virtual                 ~XclNote();

    virtual void            Save( XclExpStream& rStrm );

    virtual UINT16          GetNum() const;
    virtual ULONG           GetLen() const;
};


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

#if 0
//! unused
// --- class ExcLabel8 -----------------------------------------------

class ExcLabel8 : public ExcCell
{
private:
    XclExpRichString            aText;

    virtual void                SaveDiff( XclExpStream& rStrm );    // instead of SaveCont()
    virtual ULONG               GetDiffLen() const;

public:
                                ExcLabel8(
                                    const ScAddress&        rPos,
                                    const ScPatternAttr*    pAttr,
                                    RootData&               rRoot,
                                    const String&           rText );
                                ExcLabel8(
                                    const ScAddress&        rPos,
                                    const ScPatternAttr*    pAttr,
                                    RootData&               rRoot,
                                    const ScEditCell&       rEdCell );
    virtual                     ~ExcLabel8();

    virtual UINT16              GetNum() const;
};

#endif
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


// --- class ExcXf8 --------------------------------------------------

class ExcXf8 : public ExcXf
{
private:
        UINT16              nTrot;
        UINT16              nCIndent;
        UINT16              nIReadingOrder;

        UINT16              nGrbitDiag;
        UINT32              nIcvDiagSer;
        UINT16              nDgDiag;

        BOOL                bFShrinkToFit;
        BOOL                bFMergeCell;

    virtual void                SaveCont( XclExpStream& rStrm );

public:
                                ExcXf8( UINT16 nFont, UINT16 nForm,
                                    const ScPatternAttr* pPattAttr,
                                    BOOL& rbLineBreak, BOOL bStyle = FALSE );

    virtual UINT16              GetNum() const;
    virtual ULONG               GetLen() const;
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

class ExcWindow28 : public ExcRecord
{
private:
    ExcPalette2&                rPalette;
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
                                ExcWindow28( RootData& rRootData, UINT16 nTab );
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

class XclCondFormat : public ExcEmptyRec, protected List
{
// writes multiple cf _and_ condfmt records!
private:
    const ScConditionalFormat&  rCF;
    ScRangeList*                pRL;
    UINT16                      nTabNum;
    ULONG                       nComplLen;

    inline XclCf*               _First()    { return (XclCf*) List::First(); }
    inline XclCf*               _Next()     { return (XclCf*) List::Next(); }

    void                        WriteCondfmt( XclExpStream& rStrm );

public:
                                XclCondFormat( const ScConditionalFormat&, ScRangeList*, RootData& );
                                    // takes ScRangeList, don't use it after this Ctor!
    virtual                     ~XclCondFormat();

    virtual void                Save( XclExpStream& rStrm );
};



class XclCf : public ExcRecord
{
private:
    friend XclCondFormat;

    ExcPalette2&                rPalette2;

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
    UINT8                       nLineData1;
    UINT8                       nLineData2;
    UINT32                      nIcvTopSer;
    UINT32                      nIcvBotSer;
    UINT32                      nIcvLftSer;
    UINT32                      nIcvRigSer;

    BOOL                        bHasPattern;
    UINT16                      nPatt;
    UINT32                      nIcvForeSer;
    UINT32                      nIcvBackSer;

    virtual void                SaveCont( XclExpStream& rStrm );

public:
                                XclCf( const ScCondFormatEntry&, RootData& );
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
    UINT16                  nCol1;
    UINT16                  nCol2;
    UINT16                  nRow1;
    UINT16                  nRow2;
    UINT16                  nXF;

    inline                  XclExpMergedCell( UINT16 nC1, UINT16 nC2, UINT16 nR1, UINT16 nR2, UINT16 _nXF ) :
                                nCol1( nC1 ), nCol2( nC2 ), nRow1( nR1 ), nRow2( nR2 ), nXF( _nXF ) {}
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
    void                        Append( UINT16 nCol1, UINT16 nColCnt, UINT16 nRow1, UINT16 nRowCnt, UINT16 nXF );
    BOOL                        FindNextMerge( const ScAddress& rPos, UINT16& rnCol );
    BOOL                        FindMergeBaseXF( const ScAddress& rPos, UINT16& rnXF, UINT16& rnColCount );
    inline BOOL                 FindMergeBaseXF( const ScAddress& rPos, UINT16& rnXF );

    virtual void                Save( XclExpStream& rStrm );
};

inline BOOL XclExpCellMerging::FindMergeBaseXF( const ScAddress& rPos, UINT16& rnXF )
{
    UINT16 nCols;
    return FindMergeBaseXF( rPos, rnXF, nCols );
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
                                    { return sText.GetByteCount(); }

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


// ---- class XclHlink -----------------------------------------------

class SvMemoryStream;
class SvxURLField;

class XclHlink : public ExcRecord
{
private:
    UINT16                      nColFirst;
    UINT16                      nRowFirst;
    UINT32                      nFlags;
    String*                     pRepr;

    static const BYTE           pStaticData[];
    SvMemoryStream*             pVarData;

    static inline ULONG         GetStaticLen();             // -> xcl97rec.cxx!
    inline ULONG                GetVarLen() const;          // -> xcl97rec.cxx!

    virtual void                SaveCont( XclExpStream& rStrm );

public:
                                XclHlink( RootData& rRootData, const SvxURLField& rField );
    virtual                     ~XclHlink();

    inline void                 SetPosition( const ScAddress& rPos );

    inline const String*        GetRepr() const     { return pRepr; }

    virtual UINT16              GetNum() const;
    virtual ULONG               GetLen() const;
};

inline void XclHlink::SetPosition( const ScAddress& rPos )
{
    nColFirst = rPos.Col();
    nRowFirst = rPos.Row();
}


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


// ---- class XclBGPic -----------------------------------------------

class XclBGPic : public ExcEmptyRec
{
private:
    const Graphic*              pGr;

public:
                                XclBGPic( RootData& );
    virtual                     ~XclBGPic();

    virtual void                Save( XclExpStream& rStrm );
};


// ---- class XclExpPageBreaks8 --------------------------------------

class XclExpPageBreaks8 : public XclExpPageBreaks
{
private:
    UINT16                      nRangeMax;

    virtual void                SaveCont( XclExpStream& rStrm );

public:
                                XclExpPageBreaks8( RootData& rRootData, UINT16 nScTab, ExcPBOrientation eOrient );
    virtual                     ~XclExpPageBreaks8();

    virtual ULONG               GetLen() const;
};


// ---- class XclExpWebQuery -----------------------------------------

class XclExpWebQuery : public ExcEmptyRec
{
private:
    XclExpUniString             aRangeName;
    XclExpUniString             aURL;
    XclExpUniString*            pQryTables;
    INT16                       nRefresh;
    BOOL                        bEntireDoc;

// mode 1 - entire document:    pQryTables==NULL, bEntireDoc==TRUE
// mode 2 - all tables:         pQryTables==NULL, bEntireDoc==FALSE
// mode 3 - range list:         pQryTables!=NULL, bEntireDoc==FALSE

public:
                                XclExpWebQuery(
                                    const String& rRangeName,
                                    const String& rURL,
                                    const String& rSource,
                                    sal_Int32 nRefrSecs );
    virtual                     ~XclExpWebQuery();

    virtual void                Save( XclExpStream& rStrm );
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
