/*************************************************************************
 *
 *  $RCSfile: xcl97rec.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: gt $ $Date: 2000-11-27 15:24:45 $
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
#include "xcl97exp.hxx"
#include "xcl97esc.hxx"
#ifndef SC_RANGELST_HXX
#include "rangelst.hxx"
#endif

struct SingleRefData;

// --- class XclSstList ----------------------------------------------

class XclSstList : public List, public ExcRecord
{
private:
                                // overwritten to be able to not save empty list
            void                _Save( SvStream& );

    virtual void                SaveCont( SvStream& );

public:
                                XclSstList();
    virtual                     ~XclSstList();

            UINT32              Add( XclUnicodeString* );

    virtual UINT16              GetNum() const;
    virtual UINT16              GetLen() const;
};


// --- class XclCrn --------------------------------------------------

class XclCrn : public ExcRecord
{
private:
    UINT16                      nCol;
    UINT16                      nRow;

protected:
    void                        SaveHeadings( SvStream& rStrm );

public:
    inline                      XclCrn( UINT16 nC, UINT16 nR ) :
                                    nCol( nC ), nRow( nR )  {}

    inline BOOL                 IsAddress( UINT16 nC, UINT16 nR )
                                    { return (nC == nCol) && (nR == nRow); }

    virtual UINT16              GetNum() const;
};


// --- class XclCrnDouble ---------------------------------------------

class XclCrnDouble : public XclCrn
{
private:
    double                      fVal;
    virtual void                SaveCont( SvStream& rStrm );

protected:
public:
                                XclCrnDouble( UINT16 nC, UINT16 nR, double fV );
    virtual UINT16              GetLen() const;
};


// --- class XclCrnString ---------------------------------------------

class XclCrnString : public XclCrn
{
private:
    XclRawUnicodeString         sText;
    virtual void                SaveCont( SvStream& rStrm );

protected:
public:
                                XclCrnString( UINT16 nC, UINT16 nR, const String& rTxt );
    virtual UINT16              GetLen() const;
};


// --- class XclCrnBool -----------------------------------------------

class XclCrnBool : public XclCrn
{
private:
    UINT16                      nBool;
    virtual void                SaveCont( SvStream& rStrm );

protected:
public:
                                XclCrnBool( UINT16 nC, UINT16 nR, BOOL b );
    virtual UINT16              GetLen() const;
};


// --- class XclXct --------------------------------------------------

class XclXct : public ExcRecord, private List
{
private:
    XclUnicodeString            sTable;
    UINT16                      nTabNum;

    inline XclCrn*              _First()    { return (XclCrn*) List::First(); }
    inline XclCrn*              _Next()     { return (XclCrn*) List::Next(); }

    BOOL                        Exists( UINT16 nCol, UINT16 nRow );
    inline void                 Append( XclCrn* pCrn )
                                    { List::Insert( pCrn, LIST_APPEND ); }

    virtual void                SaveCont( SvStream& rStrm );

protected:
public:
    inline                      XclXct( const String& rTab ) :
                                    sTable( rTab )  {   }
    virtual                     ~XclXct();

    inline UINT16               GetTableBytes() const   { return ( UINT16 ) sTable.GetByteCount(); }
    inline const XclUnicodeString& GetTableName() const { return sTable; }

    inline void                 SetTableNum( UINT16 nTab )  { nTabNum = nTab; }
    void                        StoreCellRange( RootData& rRoot, const ScRange& rRange );

    virtual void                Save( SvStream& rStrm );

    virtual UINT16              GetNum() const;
    virtual UINT16              GetLen() const;
};


// --- class XclSupbook ----------------------------------------------

class XclSupbook : public ExcRecord, private List
{
private:
    UINT16                      nTables;
    String                      sDocName;
    XclRawUnicodeString         sEncoded;
    UINT32                      nLen;
    BOOL                        bSelf;

    inline XclXct*              _First()    { return (XclXct*) List::First(); }
    inline XclXct*              _Next()     { return (XclXct*) List::Next(); }
    inline XclXct*              _Get( UINT16 nTab ) const
                                            { return (XclXct*) List::GetObject( nTab ); }

    virtual void                SaveCont( SvStream& rStrm );

public:
                                XclSupbook( UINT16 nTabs );             // own book
                                XclSupbook( const String& rDocName );   // ext book
    virtual                     ~XclSupbook();

    inline const String&        GetName() const     { return sDocName; }

    UINT16                      AddTableName( const String& rTabName );
    void                        StoreCellRange( RootData& rRoot, const ScRange& rRange,
                                                UINT16 nXct );

    virtual void                Save( SvStream& rStrm );

    virtual UINT16              GetNum() const;
    virtual UINT16              GetLen() const;
};


// --- class XclSupbookList ------------------------------------------

class XclSupbookList : public ExcEmptyRec, ExcRoot, private List
{
private:
    UINT16*                     pSupbookBuffer; // supbook number for every xcl table
    UINT16*                     pTableBuffer;   // table numbers in supbook
    UINT16                      nRefdCnt;       // array size for p***Buffer

    inline XclSupbook*          _First()    { return (XclSupbook*) List::First(); }
    inline XclSupbook*          _Next()     { return (XclSupbook*) List::Next(); }
    inline XclSupbook*          _Get( UINT32 nInd )
                                            { return (XclSupbook*) List::GetObject( nInd ); }

    inline XclSupbook*          GetSupbook( UINT16 nExcTab ) const;

    UINT32                      Append( XclSupbook* pBook );
    void                        AddExt( UINT16 nScTab );

public:
                                XclSupbookList( RootData* pRoot );
    virtual                     ~XclSupbookList();

    void                        StoreCellRange( const ScRange& rRange );

    void                        WriteXtiInfo( SvStream& rStrm, UINT16 nTabFirst, UINT16 nTabLast );
    virtual void                Save( SvStream& rStrm );
};

inline XclSupbook* XclSupbookList::GetSupbook( UINT16 nExcTab ) const
{
    return (XclSupbook*) List::GetObject( pSupbookBuffer[ nExcTab ] );
}


// --- class XclXti --------------------------------------------------

class XclXti
{
private:
    UINT16                      nTabFirst;
    UINT16                      nTabLast;

public:
    inline                      XclXti( UINT16 nFirst, UINT16 nLast ) :
                                    nTabFirst( nFirst ), nTabLast( nLast )  {}

    static inline UINT16        GetSize()           { return 3 * sizeof(UINT16); }
    inline UINT16               GetTabFirst() const { return nTabFirst; }
    inline UINT16               GetTabLast() const  { return nTabLast; }
};


// --- class XclExternsheetList --------------------------------------

class XclExternsheetList : public ExcRecord, ExcRoot, private List
{
private:
    XclSupbookList              aSupbookList;

    XclXti*                     _First()    { return (XclXti*) List::First(); }
    XclXti*                     _Next()     { return (XclXti*) List::Next(); }

    static inline UINT16        GetVal16( UINT32 nVal )
                                    { return (nVal <= 0xFFFF) ? (UINT16) nVal : 0xFFFF; }

    inline void                 WriteXtiInfo( SvStream& rStrm, XclXti& rXti );
    virtual void                SaveCont( SvStream& rStrm );

public:
                                XclExternsheetList( RootData* pRoot );
    virtual                     ~XclExternsheetList();

                                // Xcl design error
    inline UINT16               GetCount16() const  { return GetVal16( List::Count() ); }

    UINT16                      Add( XclXti* pXti );

                                // add new XclXti if not found - expects Excel tabnums
    UINT16                      Find( UINT16 nTabFirst, UINT16 nTabLast );

                                // cell contents -> CRN - expects SC tabnums
    void                        StoreCellCont( const SingleRefData& rRef );
    void                        StoreCellRange( const SingleRefData& rRef1, const SingleRefData& rRef2 );

    virtual void                Save( SvStream& rStrm );

    virtual UINT16              GetNum() const;
    virtual UINT16              GetLen() const;
};

inline void XclExternsheetList::WriteXtiInfo( SvStream& rStrm, XclXti& rXti )
{
    aSupbookList.WriteXtiInfo( rStrm, rXti.GetTabFirst(), rXti.GetTabLast() );
}


// --- class XclMsodrawing_Base --------------------------------------

class XclMsodrawing_Base
{
protected:
        XclEscher*          pEscher;
        ULONG               nStartPos;      // position in OffsetMap
        ULONG               nStopPos;       // position in OffsetMap

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

    virtual void                SaveCont( SvStream& );

public:
                                XclMsodrawinggroup( RootData& rRoot,
                                    UINT16 nEscherType = 0 );
    virtual                     ~XclMsodrawinggroup();

    virtual UINT16              GetNum() const;
    virtual UINT16              GetLen() const;
};


// --- class XclMsodrawing -------------------------------------------

class XclMsodrawing : public XclMsodrawing_Base, public ExcRecord
{
private:

    virtual void                SaveCont( SvStream& );

public:
                                XclMsodrawing( RootData& rRoot,
                                    UINT16 nEscherType = 0 );
    virtual                     ~XclMsodrawing();

    virtual UINT16              GetNum() const;
    virtual UINT16              GetLen() const;
};


// --- class XclObjList ----------------------------------------------

class XclObj;
class XclMsodrawing;

class XclObjList : public List, public ExcRecord
{
private:
        XclMsodrawing*      pMsodrawingPerSheet;

                                // overwritten to be able to write nothing
            void                _Save( SvStream& );
    virtual void                SaveCont( SvStream& );

public:
                                XclObjList( RootData& rRoot );
    virtual                     ~XclObjList();

            XclObj*             First() { return (XclObj*) List::First(); }
            XclObj*             Next()  { return (XclObj*) List::Next(); }

                                /// return: 1-based ObjId
                                ///! count>=0xFFFF: Obj will be deleted, return 0
            UINT16              Add( XclObj* );

    inline  XclMsodrawing*      GetMsodrawingPerSheet();
                                /// close groups and DgContainer opened in ctor
            void                EndSheet();

    virtual UINT16              GetNum() const;
    virtual UINT16              GetLen() const;
};


inline XclMsodrawing* XclObjList::GetMsodrawingPerSheet()
{
    return pMsodrawingPerSheet;
}


// --- class XclObj --------------------------------------------------

class XclTxo;
class SdrTextObj;

class XclObj : public ExcRecord
{
    friend void XclObjList::SaveCont( SvStream& );

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
            void                _Save( SvStream& );
    virtual void                SaveCont( SvStream& );
            void                SaveText( SvStream& );

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

    virtual UINT16              GetNum() const;
    virtual UINT16              GetLen() const;
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

    virtual void                SaveCont( SvStream& );

public:
                                XclObjComment( RootData& rRoot,
                                    const ScAddress& rPos, const String& rStr );
    virtual                     ~XclObjComment();

//  virtual UINT16              GetNum() const;     // done by XclObj
    virtual UINT16              GetLen() const;
};


// --- class XclObjDropDown ------------------------------------------

class XclObjDropDown : public XclObj
{
private:
    BOOL                        bIsFiltered;

    virtual void                SaveCont( SvStream& rStrm );

protected:
public:
                                XclObjDropDown( RootData& rRoot, const ScAddress& rPos, BOOL bFilt );
    virtual                     ~XclObjDropDown();

    virtual UINT16              GetLen() const;     // GetNum() done by XclObj
};


// --- class XclTxo --------------------------------------------------

class SdrTextObj;

class XclTxo : public ExcRecord
{
    friend void XclObjComment::SaveCont( SvStream& );

private:

        XclRawUnicodeString aText;
        UINT16              nGrbit;
        UINT16              nRot;

    virtual void                SaveCont( SvStream& );

public:
                                XclTxo( const String& rStr );
                                XclTxo( const SdrTextObj& rEditObj );
    virtual                     ~XclTxo();

    virtual UINT16              GetNum() const;
    virtual UINT16              GetLen() const;
};


// --- class XclObjOle -----------------------------------------------

class XclObjOle : public XclObj
{
private:

        const SdrObject&    rOleObj;
        SvStorage*          pRootStorage;

    virtual void                SaveCont( SvStream& );

public:
                                XclObjOle( RootData& rRoot, const SdrObject& rObj );
    virtual                     ~XclObjOle();

//  virtual UINT16              GetNum() const;     // done by XclObj
    virtual UINT16              GetLen() const;
};


// --- class XclObjAny -----------------------------------------------

class XclObjAny : public XclObj
{
private:

    virtual void                SaveCont( SvStream& );

public:
                                XclObjAny( RootData& rRoot );
    virtual                     ~XclObjAny();

//  virtual UINT16              GetNum() const;     // done by XclObj
    virtual UINT16              GetLen() const;
};


// --- class XclNoteList ---------------------------------------------

class XclNote;

class XclNoteList : public List, public ExcRecord
{
private:
                                // overwritten to be able to write nothing
            void                _Save( SvStream& );
    virtual void                SaveCont( SvStream& );

public:
                                XclNoteList();
    virtual                     ~XclNoteList();

            XclNote*            First() { return (XclNote*) List::First(); }
            XclNote*            Next()  { return (XclNote*) List::Next(); }

            void                Add( XclNote* );

    virtual UINT16              GetNum() const;
    virtual UINT16              GetLen() const;
};


// --- class XclNote -------------------------------------------------

class XclNote : public ExcRecord
{
private:
        XclUnicodeString    aAuthor;
        ScAddress           aPos;
        UINT16              nGrbit;
        UINT16              nObjId;

                            // overwritten to be able to not save any data
            void            _Save( SvStream& );
    virtual void            SaveCont( SvStream& );

public:
                            XclNote( RootData& rD, const ScAddress& rPos,
                                const ScPostIt& rNote );
    virtual                 ~XclNote();

    virtual UINT16          GetNum() const;
    virtual UINT16          GetLen() const;
};


// --- class ExcBof8_Base --------------------------------------------

class ExcBof8_Base : public ExcBof_Base
{
protected:
        UINT32              nFileHistory;       // bfh
        UINT32              nLowestBiffVer;     // sfo

    virtual void                SaveCont( SvStream& rStrm );

public:
                                ExcBof8_Base();

    virtual UINT16              GetNum() const;
    virtual UINT16              GetLen() const;
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


// --- class ExcLabel8 -----------------------------------------------

class ExcLabel8 : public ExcCell
{
private:
        XclRichString       aText;

    virtual void                SaveDiff( SvStream& );  // statt SaveCont()

public:
                                ExcLabel8(
                                    const ScAddress&        rPos,
                                    const ScPatternAttr*    pAttr,
                                    const String&           rText );
                                ExcLabel8(
                                    const ScAddress&        rPos,
                                    const ScPatternAttr*    pAttr,
                                    const ScEditCell&       rEdCell,
                                    RootData&               rRoot );
    virtual                     ~ExcLabel8();

    virtual UINT16              GetNum() const;
    virtual UINT16              GetLen() const;
};


// --- class ExcLabelSst ---------------------------------------------

class ExcLabelSst : public ExcCell
{
private:
        UINT32              nIsst;      // Index in SST

    virtual void                SaveDiff( SvStream& );  // statt SaveCont()

public:
                                ExcLabelSst(
                                    const ScAddress&        rPos,
                                    const ScPatternAttr*    pAttr,
                                    const String&           rText,
                                    RootData&               rRoot );
                                ExcLabelSst(
                                    const ScAddress&        rPos,
                                    const ScPatternAttr*    pAttr,
                                    const ScEditCell&       rEdCell,
                                    RootData&               rRoot );
    virtual                     ~ExcLabelSst();

    virtual UINT16              GetNum() const;
    virtual UINT16              GetLen() const;
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

    virtual void                SaveCont( SvStream& rStrm );

public:
                                ExcXf8( UINT16 nFont, UINT16 nForm,
                                    const ScPatternAttr* pPattAttr,
                                    BOOL& rbLineBreak, BOOL bStyle = FALSE );

    virtual UINT16              GetNum() const;
    virtual UINT16              GetLen() const;
};


// --- class ExcBundlesheet8 -----------------------------------------

class ExcBundlesheet8 : public ExcBundlesheet
{
private:
        const XclRawUnicodeString   aUnicodeName;

    virtual void                SaveCont( SvStream& );

public:
                                ExcBundlesheet8( const String &rNewName );

    virtual UINT16              GetNum() const;
    virtual UINT16              GetLen() const;
};


// --- class ExcTabid8 -----------------------------------------------

class ExcTabid8 : public ExcRecord
{
private:
        UINT16              nTabs;

            void                SaveCont( SvStream& );

public:
                                ExcTabid8( UINT16 nTabsP ) : nTabs( nTabsP ) {}

    virtual UINT16              GetNum() const;
    virtual UINT16              GetLen() const;
};


// --- class ExcWindow28 ---------------------------------------------

class ExcWindow28 : public ExcWindow2
{
private:

            void                SaveCont( SvStream& );

public:
                                ExcWindow28( UINT16 nTab ) : ExcWindow2( nTab ) {}

    virtual UINT16              GetNum() const;
    virtual UINT16              GetLen() const;
};


// --- class XclCondFormat -------------------------------------------

class ScConditionalFormat;
class ScCondFormatEntry;
class ScRangeList;
struct RootData;
class XclCf;

class XclCondFormat : public ExcRecord, protected List
{
// writes multiple cf _and_ condfmt records!
private:
    const ScConditionalFormat&  rCF;
    ScRangeList*                pRL;
    UINT16                      nTabNum;
    UINT16                      nOwnLen;
    UINT16                      nComplLen;
protected:
    virtual void                _Save( SvStream& );     // because multiple records are written!
public:
                                XclCondFormat( const ScConditionalFormat&, ScRangeList*, RootData& );
                                    // takes ScRangeList, don't use it after this Ctor!
    virtual                     ~XclCondFormat();
    virtual UINT16              GetNum() const;
    virtual UINT16              GetLen() const;
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

protected:
    virtual void                SaveCont( SvStream& );
public:
                                XclCf( const ScCondFormatEntry&, RootData& );
    virtual                     ~XclCf();
    virtual UINT16              GetNum() const;
    virtual UINT16              GetLen() const;
};


class XclObproj : public ExcRecord
{
public:
    virtual UINT16              GetNum() const;
    virtual UINT16              GetLen() const;
};


//============================================================================
// data consolidation reference


class XclDConRef : public ExcRecord
{
private:
    ScRange                 aSourceRange;
    XclRawUnicodeString*    pWorkbook;

protected:
    virtual void            SaveCont( SvStream& rStrm );

public:
                            XclDConRef( const ScRange& rSrcR, const String& rWB );
    virtual                 ~XclDConRef();

    virtual UINT16          GetNum() const;
    virtual UINT16          GetLen() const;
};



//============================================================================
// merged cells
//_________________________________________________________
// class XclCellMerging - merged cells (max. 1024)

class XclCellMerging : public ExcRecord
{
private:
    UINT16                  nCount;
    UINT16List              aCoordList;

    virtual void            SaveCont( SvStream& rStrm );

protected:
public:
    inline                  XclCellMerging();
    virtual                 ~XclCellMerging();

    inline BOOL             IsListFull() const  { return nCount >= 1024; }
    void                    Append( UINT16 nCol1, UINT16 nColCnt,
                                    UINT16 nRow1, UINT16 nRowCnt );

    virtual UINT16          GetNum() const;
    virtual UINT16          GetLen() const;
};


inline XclCellMerging::XclCellMerging() :
        nCount( 0 )
{   }



//_________________________________________________________
// class XclCellMergingList - list of XclCellMerging

class XclCellMergingList : public ExcEmptyRec, private List
{
private:
    XclCellMerging*         pCurrRec;

    inline XclCellMerging*  _First()    { return (XclCellMerging*) List::First(); }
    inline XclCellMerging*  _Next()     { return (XclCellMerging*) List::Next(); }

    XclCellMerging*         InsertNewRec();

protected:
public:
    inline                  XclCellMergingList();
    virtual                 ~XclCellMergingList();

    void                    Append( UINT16 nCol1, UINT16 nColCnt,
                                    UINT16 nRow1, UINT16 nRowCnt );

    virtual void            Save( SvStream& rStrm );
};


inline XclCellMergingList::XclCellMergingList() :
        pCurrRec( NULL )
{   }



//_________________________________________________________
// class XclCodename

class XclCodename : public ExcRecord
{
private:
    XclUnicodeString            aName;
    virtual void                SaveCont( SvStream& );
public:
                                XclCodename( const String& );

    virtual UINT16              GetNum() const;
    virtual UINT16              GetLen() const;
};




class XclBuildInName : public ExcNameListEntry, private ExcRoot
{
private:
    UINT8                   nKey;
    UINT8*                  pData;
    UINT16                  nFormLen;
    UINT16                  nTabNum;
    ScRangeList             aRL;

    void                    _Save( SvStream& rStrm );

    inline ScRange*         _First( void );
    inline ScRange*         _Next( void );
protected:
    inline void             Append( const ScRange& rNew );

    void                    Add( const ScRange& rRange );
    void                    CreateFormula( void );
public:
                            XclBuildInName( RootData*, UINT16 nTabNum, UINT8 nKey );
    virtual                 ~XclBuildInName();

    virtual UINT16          GetLen() const;
};


inline ScRange* XclBuildInName::_First( void )
{
    return ( ScRange* ) aRL.First();
}


inline ScRange* XclBuildInName::_Next( void )
{
    return ( ScRange* ) aRL.Next();
}


inline void XclBuildInName::Append( const ScRange& r )
{
    aRL.Append( r );
}




class XclPrintRange : public XclBuildInName
{
public:
                            XclPrintRange( RootData*, UINT16 nTabNum, ScDocument& rDoc );
};




class XclTitleRange : public XclBuildInName
{
public:
                            XclTitleRange( RootData*, UINT16 nTabNum, ScDocument& rDoc );
};




//___________________________________________________________________
// Scenario export
// - ExcEScenarioCell           a cell of a scenario range
// - ExcEScenario               all ranges of a scenario table
// - ExcEScenarioManager        list of scenario tables

class ExcEScenarioCell
{
private:
    UINT16                      nCol;
    UINT16                      nRow;
    XclRawUnicodeString         sText;

protected:
public:
                                ExcEScenarioCell( UINT16 nC, UINT16 nR, const String& rTxt );

    inline UINT16               GetStringBytes()
                                    { return (UINT16)(sText.GetByteCount() + 3); }

    inline void                 WriteAddress( SvStream& rStrm ) { rStrm << nRow << nCol; }
    void                        WriteText( SvStream& rStrm );
};



class ExcEScenario : public ExcRecord, private List
{
private:
    UINT16                      nRecLen;
    XclRawUnicodeString         sName;
    XclRawUnicodeString         sComment;
    static XclRawUnicodeString  sUsername;

    inline ExcEScenarioCell*    _First()    { return (ExcEScenarioCell*) List::First(); }
    inline ExcEScenarioCell*    _Next()     { return (ExcEScenarioCell*) List::Next(); }

    BOOL                        Append( UINT16 nCol, UINT16 nRow, const String& rTxt );

    virtual void                SaveCont( SvStream& rStrm );

protected:
public:
                                ExcEScenario( ScDocument& rDoc, UINT16 nTab );
    virtual                     ~ExcEScenario();

    virtual UINT16              GetNum() const;
    virtual UINT16              GetLen() const;
};



class ExcEScenarioManager : public ExcRecord, private List
{
private:
    UINT16                      nActive;

    inline ExcEScenario*        _First()    { return (ExcEScenario*) List::First(); }
    inline ExcEScenario*        _Next()     { return (ExcEScenario*) List::Next(); }

    inline void                 Append( ExcEScenario* pScen )
                                    { List::Insert( pScen, LIST_APPEND ); }

    virtual void                SaveCont( SvStream& rStrm );

protected:
public:
                                ExcEScenarioManager( ScDocument& rDoc, UINT16 nTab );
    virtual                     ~ExcEScenarioManager();

    virtual void                Save( SvStream& rStrm );

    virtual UINT16              GetNum() const;
    virtual UINT16              GetLen() const;
};




class SvMemoryStream;
class SvxURLField;


class XclHlink : public ExcRecord
{
private:
    UINT16                      nColFirst;
    UINT16                      nRowFirst;
    UINT32                      nFlags;

    String*                     pRepr;      // to B displayed in cell text

    static const BYTE           pStaticData1[];
    SvMemoryStream*             pVarData;

    static inline UINT32        StaticLen( void );          // -> xcl97rec.cxx!
    inline UINT32               VarLen( void ) const;       // -> xcl97rec.cxx!

    virtual void                SaveCont( SvStream& );
public:
                                XclHlink( RootData&, const SvxURLField& );
    virtual                     ~XclHlink();

    virtual UINT16              GetNum() const;
    virtual UINT16              GetLen() const;

    inline void                 Set( const ScAddress& rPos );

    inline BOOL                 IsValid( void ) const;
    inline const String*        GetRepr( void ) const;
};


inline void XclHlink::Set( const ScAddress& r )
{
    nColFirst = r.Col();
    nRowFirst = r.Row();
}


inline BOOL XclHlink::IsValid( void ) const
{
    return pVarData != NULL;
}


inline const String* XclHlink::GetRepr( void ) const
{
    return pRepr;
}




class XclProtection : public ExcDummyRec
{
    // replacement for records PROTECT, SCENPROTECT, OBJPROTECT...
private:
    static const BYTE           pMyData[];
    static const UINT16         nMyLen;
public:
    virtual UINT16              GetLen( void ) const;
    virtual const BYTE*         GetData( void ) const;
};




class XclBGPic : public ExcRecord
{
private:
    const Graphic*              pGr;
    virtual void                _Save( SvStream& );
public:
                                XclBGPic( RootData& );
    virtual                     ~XclBGPic();

    virtual UINT16              GetNum() const;
    virtual UINT16              GetLen() const;
};


#endif // _XCL97REC_HXX
