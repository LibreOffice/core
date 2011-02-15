/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_XCL97REC_HXX
#define SC_XCL97REC_HXX

#include "excrecds.hxx"
#include "xcl97esc.hxx"
#include "xlstyle.hxx"

// ============================================================================

class XclObj;
class XclExpMsoDrawing;

class XclExpObjList : public List, public ExcEmptyRec, protected XclExpRoot
{
public:
    explicit            XclExpObjList( const XclExpRoot& rRoot, XclEscherEx& rEscherEx );
    virtual             ~XclExpObjList();

    XclObj*             First() { return (XclObj*) List::First(); }
    XclObj*             Next() { return (XclObj*) List::Next(); }

    /// return: 1-based ObjId
    ///! count>=0xFFFF: Obj will be deleted, return 0
    sal_uInt16              Add( XclObj* );

    inline XclExpMsoDrawing* GetMsodrawingPerSheet() { return pMsodrawingPerSheet; }

                                /// close groups and DgContainer opened in ctor
    void                EndSheet();

    virtual void        Save( XclExpStream& rStrm );

private:
    XclEscherEx&        mrEscherEx;
    XclExpMsoDrawing*   pMsodrawingPerSheet;
    XclExpMsoDrawing*   pSolverContainer;
};


// --- class XclObj --------------------------------------------------

class XclTxo;
class SdrTextObj;

class XclObj : public XclExpRecord
{
protected:
        XclEscherEx&        mrEscherEx;
        XclExpMsoDrawing*   pMsodrawing;
        XclExpMsoDrawing*   pClientTextbox;
        XclTxo*             pTxo;
        sal_uInt16          mnObjType;
        sal_uInt16              nObjId;
        sal_uInt16              nGrbit;
        sal_Bool                bFirstOnSheet;

        bool                    mbOwnEscher;    /// true = Escher part created on the fly.

    /** @param bOwnEscher  If set to true, this object will create its escher data.
        See SetOwnEscher() for details. */
    explicit                    XclObj( XclExpObjectManager& rObjMgr, sal_uInt16 nObjType, bool bOwnEscher = false );

    void                        ImplWriteAnchor( const XclExpRoot& rRoot, const SdrObject* pSdrObj, const Rectangle* pChildAnchor );

                                // overwritten for writing MSODRAWING record
    virtual void                WriteBody( XclExpStream& rStrm );
    virtual void                WriteSubRecs( XclExpStream& rStrm );
            void                SaveTextRecs( XclExpStream& rStrm );

public:
    virtual                     ~XclObj();

    inline sal_uInt16           GetObjType() const { return mnObjType; }

    inline  void                SetId( sal_uInt16 nId ) { nObjId = nId; }

    inline  void                SetLocked( sal_Bool b )
                                    { b ? nGrbit |= 0x0001 : nGrbit &= ~0x0001; }
    inline  void                SetPrintable( sal_Bool b )
                                    { b ? nGrbit |= 0x0010 : nGrbit &= ~0x0010; }
    inline  void                SetAutoFill( sal_Bool b )
                                    { b ? nGrbit |= 0x2000 : nGrbit &= ~0x2000; }
    inline  void                SetAutoLine( sal_Bool b )
                                    { b ? nGrbit |= 0x4000 : nGrbit &= ~0x4000; }

                                // set corresponding Excel object type in OBJ/ftCmo
            void                SetEscherShapeType( sal_uInt16 nType );
    inline  void                SetEscherShapeTypeGroup() { mnObjType = EXC_OBJTYPE_GROUP; }

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

    virtual void                Save( XclExpStream& rStrm );
};

// --- class XclObjComment -------------------------------------------

class XclObjComment : public XclObj
{
public:
                                XclObjComment( XclExpObjectManager& rObjMgr,
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
    sal_Bool                        bIsFiltered;

    virtual void                WriteSubRecs( XclExpStream& rStrm );

protected:
public:
                                XclObjDropDown( XclExpObjectManager& rObjMgr, const ScAddress& rPos, sal_Bool bFilt );
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

    inline void                 SetHorAlign( sal_uInt8 nHorAlign ) { mnHorAlign = nHorAlign; }
    inline void                 SetVerAlign( sal_uInt8 nVerAlign ) { mnVerAlign = nVerAlign; }

    virtual void                Save( XclExpStream& rStrm );

    virtual sal_uInt16              GetNum() const;
    virtual sal_Size            GetLen() const;

private:
    virtual void                SaveCont( XclExpStream& rStrm );

private:
    XclExpStringRef             mpString;       /// Text and formatting data.
    sal_uInt16                  mnRotation;     /// Text rotation.
    sal_uInt8                   mnHorAlign;     /// Horizontal alignment.
    sal_uInt8                   mnVerAlign;     /// Vertical alignment.
};


// --- class XclObjOle -----------------------------------------------

class XclObjOle : public XclObj
{
private:

        const SdrObject&    rOleObj;
        SotStorage*         pRootStorage;

    virtual void                WriteSubRecs( XclExpStream& rStrm );

public:
                                XclObjOle( XclExpObjectManager& rObjMgr, const SdrObject& rObj );
    virtual                     ~XclObjOle();

    virtual void                Save( XclExpStream& rStrm );
};


// --- class XclObjAny -----------------------------------------------

class XclObjAny : public XclObj
{
private:
    virtual void                WriteSubRecs( XclExpStream& rStrm );

public:
                                XclObjAny( XclExpObjectManager& rObjMgr );
    virtual                     ~XclObjAny();

    virtual void                Save( XclExpStream& rStrm );
};


// --- class ExcBof8_Base --------------------------------------------

class ExcBof8_Base : public ExcBof_Base
{
protected:
        sal_uInt32              nFileHistory;       // bfh
        sal_uInt32              nLowestBiffVer;     // sfo

    virtual void                SaveCont( XclExpStream& rStrm );

public:
                                ExcBof8_Base();

    virtual sal_uInt16              GetNum() const;
    virtual sal_Size            GetLen() const;
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


// --- class ExcBundlesheet8 -----------------------------------------

class ExcBundlesheet8 : public ExcBundlesheetBase
{
private:
    String                      sUnicodeName;
    XclExpString                GetName() const;

    virtual void                SaveCont( XclExpStream& rStrm );

public:
                                ExcBundlesheet8( RootData& rRootData, SCTAB nTab );
                                ExcBundlesheet8( const String& rString );

    virtual sal_Size            GetLen() const;

    virtual void                SaveXml( XclExpXmlStream& rStrm );
};


// --- class XclObproj -----------------------------------------------

class XclObproj : public ExcRecord
{
public:
    virtual sal_uInt16              GetNum() const;
    virtual sal_Size            GetLen() const;
};


// ---- class XclCodename --------------------------------------------

class XclCodename : public ExcRecord
{
private:
    XclExpString                aName;
    virtual void                SaveCont( XclExpStream& rStrm );
public:
                                XclCodename( const String& );

    virtual sal_uInt16              GetNum() const;
    virtual sal_Size            GetLen() const;
};


// ---- Scenarios ----------------------------------------------------
// - ExcEScenarioCell           a cell of a scenario range
// - ExcEScenario               all ranges of a scenario table
// - ExcEScenarioManager        list of scenario tables

class ExcEScenarioCell
{
private:
    sal_uInt16                      nCol;
    sal_uInt16                      nRow;
    XclExpString                sText;

protected:
public:
                                ExcEScenarioCell( sal_uInt16 nC, sal_uInt16 nR, const String& rTxt );

    inline sal_Size             GetStringBytes()
                                    { return sText.GetSize(); }

    void                        WriteAddress( XclExpStream& rStrm );
    void                        WriteText( XclExpStream& rStrm );

    void                        SaveXml( XclExpXmlStream& rStrm );
};



class ExcEScenario : public ExcRecord, private List
{
private:
    sal_Size                    nRecLen;
    XclExpString                sName;
    XclExpString                sComment;
    XclExpString                sUserName;
    sal_uInt8                       nProtected;

    inline ExcEScenarioCell*    _First()    { return (ExcEScenarioCell*) List::First(); }
    inline ExcEScenarioCell*    _Next()     { return (ExcEScenarioCell*) List::Next(); }

    sal_Bool                        Append( sal_uInt16 nCol, sal_uInt16 nRow, const String& rTxt );

    virtual void                SaveCont( XclExpStream& rStrm );

protected:
public:
                                ExcEScenario( const XclExpRoot& rRoot, SCTAB nTab );
    virtual                     ~ExcEScenario();

    virtual sal_uInt16              GetNum() const;
    virtual sal_Size            GetLen() const;

    virtual void                SaveXml( XclExpXmlStream& rStrm );
};



class ExcEScenarioManager : public ExcRecord, private List
{
private:
    sal_uInt16                      nActive;

    inline ExcEScenario*        _First()    { return (ExcEScenario*) List::First(); }
    inline ExcEScenario*        _Next()     { return (ExcEScenario*) List::Next(); }

    inline void                 Append( ExcEScenario* pScen )
                                    { List::Insert( pScen, LIST_APPEND ); }

    virtual void                SaveCont( XclExpStream& rStrm );

protected:
public:
                                ExcEScenarioManager( const XclExpRoot& rRoot, SCTAB nTab );
    virtual                     ~ExcEScenarioManager();

    virtual void                Save( XclExpStream& rStrm );
    virtual void                SaveXml( XclExpXmlStream& rStrm );

    virtual sal_uInt16              GetNum() const;
    virtual sal_Size            GetLen() const;
};

// ============================================================================

/** Represents a SHEETPROTECTION record that stores sheet protection
    options.  Note that a sheet still needs to save its sheet protection
    options even when it's not protected. */
class XclExpSheetProtectOptions : public XclExpRecord
{
public:
    explicit            XclExpSheetProtectOptions( const XclExpRoot& rRoot, SCTAB nTab );

private:
    virtual void        WriteBody( XclExpStream& rStrm );

private:
    sal_uInt16      mnOptions;      /// Encoded sheet protection options.
};

// ============================================================================

class XclCalccount : public ExcRecord
{
private:
    sal_uInt16                      nCount;
protected:
    virtual void                SaveCont( XclExpStream& rStrm );
public:
                                XclCalccount( const ScDocument& );

    virtual sal_uInt16              GetNum() const;
    virtual sal_Size            GetLen() const;

    virtual void                SaveXml( XclExpXmlStream& rStrm );
};




class XclIteration : public ExcRecord
{
private:
    sal_uInt16                      nIter;
protected:
    virtual void                SaveCont( XclExpStream& rStrm );
public:
                                XclIteration( const ScDocument& );

    virtual sal_uInt16              GetNum() const;
    virtual sal_Size            GetLen() const;

    virtual void                SaveXml( XclExpXmlStream& rStrm );
};




class XclDelta : public ExcRecord
{
private:
    double                      fDelta;
protected:
    virtual void                SaveCont( XclExpStream& rStrm );
public:
                                XclDelta( const ScDocument& );

    virtual sal_uInt16              GetNum() const;
    virtual sal_Size            GetLen() const;

    virtual void                SaveXml( XclExpXmlStream& rStrm );
};




class XclRefmode : public XclExpBoolRecord
{
public:
                                XclRefmode( const ScDocument& );

    virtual void                SaveXml( XclExpXmlStream& rStrm );
};

// ============================================================================

class XclExpFileEncryption : public XclExpRecord
{
public:
    explicit XclExpFileEncryption( const XclExpRoot& rRoot );
    virtual ~XclExpFileEncryption();

private:
    virtual void WriteBody( XclExpStream& rStrm );

private:
    const XclExpRoot& mrRoot;
};

// ============================================================================

/** Beginning of User Interface Records */
class XclExpInterfaceHdr : public XclExpUInt16Record
{
public:
    explicit            XclExpInterfaceHdr( sal_uInt16 nCodePage );

private:
    virtual void        WriteBody( XclExpStream& rStrm );
};

// ============================================================================

/** End of User Interface Records */
class XclExpInterfaceEnd : public XclExpRecord
{
public:
    explicit XclExpInterfaceEnd();
    virtual ~XclExpInterfaceEnd();

private:
    virtual void WriteBody( XclExpStream& rStrm );
};

// ============================================================================

/** Write Access User Name - This record contains the user name, which is
    the name you type when you install Excel. */
class XclExpWriteAccess : public XclExpRecord
{
public:
    explicit XclExpWriteAccess();
    virtual ~XclExpWriteAccess();

private:
    virtual void WriteBody( XclExpStream& rStrm );
};

// ============================================================================

class XclExpFileSharing : public XclExpRecord
{
public:
    explicit            XclExpFileSharing( const XclExpRoot& rRoot, sal_uInt16 nPasswordHash, bool bRecommendReadOnly );

    virtual void        Save( XclExpStream& rStrm );

private:
    virtual void        WriteBody( XclExpStream& rStrm );

private:
    XclExpString        maUserName;
    sal_uInt16          mnPasswordHash;
    bool                mbRecommendReadOnly;
};

// ============================================================================

class XclExpProt4Rev : public XclExpRecord
{
public:
    explicit XclExpProt4Rev();
    virtual ~XclExpProt4Rev();

private:
    virtual void WriteBody( XclExpStream& rStrm );
};

// ============================================================================

class XclExpProt4RevPass : public XclExpRecord
{
public:
    explicit XclExpProt4RevPass();
    virtual ~XclExpProt4RevPass();

private:
    virtual void WriteBody( XclExpStream& rStrm );
};

// ============================================================================

class XclExpRecalcId : public XclExpDummyRecord
{
public:
    explicit XclExpRecalcId();
};

// ============================================================================

class XclExpBookExt : public XclExpDummyRecord
{
public:
    explicit XclExpBookExt();
};


#endif // _XCL97REC_HXX
