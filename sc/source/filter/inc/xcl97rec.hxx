/*************************************************************************
 *
 *  $RCSfile: xcl97rec.hxx,v $
 *
 *  $Revision: 1.41 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-14 12:08:36 $
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

#ifndef SC_XLSTYLE_HXX
#include "xlstyle.hxx"
#endif

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
                                    const Rectangle& rRect, const EditTextObject& rEditObj, SdrObject* pCaption, bool bVisible );
    virtual                     ~XclObjComment();

    /** c'tor process for formatted text objects above .
       @descr used to construct the MSODRAWING Escher object properties. */
    void                        ProcessEscherObj( const XclExpRoot& rRoot,
                                    const Rectangle& rRect, SdrObject* pCaption, bool bVisible );


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
                                XclTxo( const XclExpRoot& rRoot, const EditTextObject& rEditObj, SdrObject* pCaption );

    inline void                 SetHorAlign( XclTxoHorAlign eHorAlign ) { meHorAlign = eHorAlign; }
    inline void                 SetVerAlign( XclTxoVerAlign eVerAlign ) { meVerAlign = eVerAlign; }

    virtual void                Save( XclExpStream& rStrm );

    virtual UINT16              GetNum() const;
    virtual ULONG               GetLen() const;

private:
    virtual void                SaveCont( XclExpStream& rStrm );

private:
    XclExpStringRef             mpString;       /// Text and formatting data.
    XclTxoHorAlign              meHorAlign;     /// Horizontal alignment.
    XclTxoVerAlign              meVerAlign;     /// Vertical alignment.
    XclTxoRotation              meRotation;     /// Text rotation.
};


// --- class XclObjOle -----------------------------------------------

class XclObjOle : public XclObj
{
private:

        const SdrObject&    rOleObj;
        SotStorage*         pRootStorage;

    virtual void                WriteSubRecs( XclExpStream& rStrm );

public:
                                XclObjOle( const XclExpRoot& rRoot, const SdrObject& rObj );
    virtual                     ~XclObjOle();

    virtual void                Save( XclExpStream& rStrm );
};


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

// --- class ExcBundlesheet8 -----------------------------------------

class ExcBundlesheet8 : public ExcBundlesheetBase
{
private:
    XclExpString                aUnicodeName;

    virtual void                SaveCont( XclExpStream& rStrm );

public:
                                ExcBundlesheet8( RootData& rRootData, SCTAB nTab );
                                ExcBundlesheet8( const String& rString );

    virtual ULONG               GetLen() const;
};


// ============================================================================

/** Represents the WINDOW1 record containing workbook global view settings. */
class XclExpWindow1 : public XclExpRecord
{
public:
    explicit                XclExpWindow1( const XclExpRoot& rRoot );

private:
    /** Writes the contents of the WINDOW1 record. */
    virtual void            WriteBody( XclExpStream& rStrm );

private:
    sal_uInt16              mnActiveTab;        /// Index to active (visible) sheet.
    sal_uInt16              mnFirstVisTab;      /// Index to first visible sheet.
    sal_uInt16              mnSelectedTabs;     /// Number of selected sheets.
};


// ============================================================================
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

class XclExpWindow28 : public XclExpRecord, protected XclExpRoot
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

    virtual void                WriteBody( XclExpStream& rStrm );

public:
                                XclExpWindow28( const XclExpRoot& rRoot, SCTAB nScTab );
    virtual                     ~XclExpWindow28();

    virtual void                Save( XclExpStream& rStrm );
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
    UINT8                       nProtected;

    inline ExcEScenarioCell*    _First()    { return (ExcEScenarioCell*) List::First(); }
    inline ExcEScenarioCell*    _Next()     { return (ExcEScenarioCell*) List::Next(); }

    BOOL                        Append( UINT16 nCol, UINT16 nRow, const String& rTxt );

    virtual void                SaveCont( XclExpStream& rStrm );

protected:
public:
                                ExcEScenario( ScDocument& rDoc, SCTAB nTab );
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
                                ExcEScenarioManager( ScDocument& rDoc, SCTAB nTab );
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
