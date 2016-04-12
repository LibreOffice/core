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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_XCL97REC_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_XCL97REC_HXX

#include "excrecds.hxx"
#include "xcl97esc.hxx"
#include "xlstyle.hxx"
#include "tabprotection.hxx"

class XclObj;
class XclExpMsoDrawing;
class SdrCaptionObj;

class XclExpObjList : public ExcEmptyRec, protected XclExpRoot
{
public:

    typedef std::vector<XclObj*>::iterator iterator;

    explicit            XclExpObjList( const XclExpRoot& rRoot, XclEscherEx& rEscherEx );
    virtual             ~XclExpObjList();

    /// return: 1-based ObjId
    ///! count>=0xFFFF: Obj will be deleted, return 0
    sal_uInt16              Add( XclObj* );

    XclObj* back () { return maObjs.empty() ? nullptr : maObjs.back(); }

    /**
     *
     * @brief Remove last element in the list.
     *
     */

    void pop_back ();

    inline bool empty () const { return maObjs.empty(); }

    inline size_t size () const { return maObjs.size(); }

    inline iterator begin () { return maObjs.begin(); }

    inline iterator end () { return maObjs.end(); }

    inline XclExpMsoDrawing* GetMsodrawingPerSheet() { return pMsodrawingPerSheet; }

                                /// close groups and DgContainer opened in ctor
    void                EndSheet();

    virtual void        Save( XclExpStream& rStrm ) override;
    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;

    static void        ResetCounters();

private:
    static  sal_Int32   mnDrawingMLCount, mnVmlCount;
    SCTAB               mnScTab;

    XclEscherEx&        mrEscherEx;
    XclExpMsoDrawing*   pMsodrawingPerSheet;
    XclExpMsoDrawing*   pSolverContainer;

    std::vector<XclObj*> maObjs;
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
        sal_uInt16          nObjId;
        sal_uInt16          nGrbit;
        SCTAB               mnScTab;
        bool                bFirstOnSheet;

        bool                    mbOwnEscher;    /// true = Escher part created on the fly.

    /** @param bOwnEscher  If set to true, this object will create its escher data.
        See SetOwnEscher() for details. */
    explicit                    XclObj( XclExpObjectManager& rObjMgr, sal_uInt16 nObjType, bool bOwnEscher = false );

    void                        ImplWriteAnchor( const XclExpRoot& rRoot, const SdrObject* pSdrObj, const Rectangle* pChildAnchor );

                                // overwritten for writing MSODRAWING record
    virtual void                WriteBody( XclExpStream& rStrm ) override;
    virtual void                WriteSubRecs( XclExpStream& rStrm );
            void                SaveTextRecs( XclExpStream& rStrm );

public:
    virtual                     ~XclObj();

    inline sal_uInt16           GetObjType() const { return mnObjType; }

    inline  void                SetId( sal_uInt16 nId ) { nObjId = nId; }

    inline  void                SetTab( SCTAB nScTab )  { mnScTab = nScTab; }
    inline  SCTAB               GetTab() const          { return mnScTab; }

    inline  void                SetLocked( bool b )
                                    { b ? nGrbit |= 0x0001 : nGrbit &= ~0x0001; }
    inline  void                SetPrintable( bool b )
                                    { b ? nGrbit |= 0x0010 : nGrbit &= ~0x0010; }
    inline  void                SetAutoFill( bool b )
                                    { b ? nGrbit |= 0x2000 : nGrbit &= ~0x2000; }
    inline  void                SetAutoLine( bool b )
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
    /** Returns true, if the object has created the escher data itself.
        @descr  See SetOwnEscher() for details. */
    inline bool                 IsOwnEscher() const { return mbOwnEscher; }

                                //! actually writes ESCHER_ClientTextbox
            void                SetText( const XclExpRoot& rRoot, const SdrTextObj& rObj );

    virtual void                Save( XclExpStream& rStrm ) override;
};

// --- class XclObjComment -------------------------------------------

class XclObjComment : public XclObj
{
    ScAddress                   maScPos;
    std::unique_ptr< SdrCaptionObj >
                                mpCaption;
    bool                        mbVisible;
    Rectangle                   maFrom;
    Rectangle                   maTo;

public:
                                XclObjComment( XclExpObjectManager& rObjMgr,
                                    const Rectangle& rRect, const EditTextObject& rEditObj, SdrCaptionObj* pCaption, bool bVisible, const ScAddress& rAddress, Rectangle &rFrom, Rectangle &To );
    virtual                     ~XclObjComment();

    /** c'tor process for formatted text objects above .
       @descr used to construct the MSODRAWING Escher object properties. */
    void                        ProcessEscherObj( const XclExpRoot& rRoot,
                                    const Rectangle& rRect, SdrObject* pCaption, bool bVisible );

    virtual void                Save( XclExpStream& rStrm ) override;
    virtual void                SaveXml( XclExpXmlStream& rStrm ) override;
};

// --- class XclObjDropDown ------------------------------------------

class XclObjDropDown : public XclObj
{
private:
    bool                        bIsFiltered;

    virtual void                WriteSubRecs( XclExpStream& rStrm ) override;

protected:
public:
                                XclObjDropDown( XclExpObjectManager& rObjMgr, const ScAddress& rPos, bool bFilt );
    virtual                     ~XclObjDropDown();
};

// --- class XclTxo --------------------------------------------------

class SdrTextObj;

class XclTxo : public ExcRecord
{
public:
                                XclTxo( const OUString& rString, sal_uInt16 nFontIx = EXC_FONT_APP );
                                XclTxo( const XclExpRoot& rRoot, const SdrTextObj& rEditObj );
                                XclTxo( const XclExpRoot& rRoot, const EditTextObject& rEditObj, SdrObject* pCaption );

    inline void                 SetHorAlign( sal_uInt8 nHorAlign ) { mnHorAlign = nHorAlign; }
    inline void                 SetVerAlign( sal_uInt8 nVerAlign ) { mnVerAlign = nVerAlign; }

    virtual void                Save( XclExpStream& rStrm ) override;

    virtual sal_uInt16              GetNum() const override;
    virtual sal_Size            GetLen() const override;

private:
    virtual void                SaveCont( XclExpStream& rStrm ) override;

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

    virtual void                WriteSubRecs( XclExpStream& rStrm ) override;

public:
                                XclObjOle( XclExpObjectManager& rObjMgr, const SdrObject& rObj );
    virtual                     ~XclObjOle();

    virtual void                Save( XclExpStream& rStrm ) override;
};

// --- class XclObjAny -----------------------------------------------

class XclObjAny : public XclObj
{
protected:
    virtual void                WriteSubRecs( XclExpStream& rStrm ) override;

public:
                                XclObjAny( XclExpObjectManager& rObjMgr,
                                    const css::uno::Reference< css::drawing::XShape >& rShape,
                                    ScDocument* pDoc);
    virtual                     ~XclObjAny();

    const css::uno::Reference< css::drawing::XShape >&
                                GetShape() const { return mxShape; }

    virtual void                Save( XclExpStream& rStrm ) override;
    virtual void                SaveXml( XclExpXmlStream& rStrm ) override;
    static void                 WriteFromTo( XclExpXmlStream& rStrm, const XclObjAny& rObj );
    static void                 WriteFromTo( XclExpXmlStream& rStrm, const css::uno::Reference< css::drawing::XShape >& rShape, SCTAB nTab );

private:
    css::uno::Reference< css::drawing::XShape >
                                mxShape;
    ScDocument* mpDoc;
};

// --- class ExcBof8_Base --------------------------------------------

class ExcBof8_Base : public ExcBof_Base
{
protected:
        sal_uInt32              nFileHistory;       // bfh
        sal_uInt32              nLowestBiffVer;     // sfo

    virtual void                SaveCont( XclExpStream& rStrm ) override;

public:
                                ExcBof8_Base();

    virtual sal_uInt16              GetNum() const override;
    virtual sal_Size            GetLen() const override;
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
    OUString                    sUnicodeName;
    XclExpString                GetName() const { return XclExpString( sUnicodeName, EXC_STR_8BITLENGTH );}

    virtual void                SaveCont( XclExpStream& rStrm ) override;

public:
                                ExcBundlesheet8( RootData& rRootData, SCTAB nTab );
                                ExcBundlesheet8( const OUString& rString );

    virtual sal_Size            GetLen() const override;

    virtual void                SaveXml( XclExpXmlStream& rStrm ) override;
};

// --- class XclObproj -----------------------------------------------

class XclObproj : public ExcRecord
{
public:
    virtual sal_uInt16              GetNum() const override;
    virtual sal_Size            GetLen() const override;
};

// ---- class XclCodename --------------------------------------------

class XclCodename : public ExcRecord
{
private:
    XclExpString                aName;
    virtual void                SaveCont( XclExpStream& rStrm ) override;
public:
                                XclCodename( const OUString& );

    virtual sal_uInt16              GetNum() const override;
    virtual sal_Size            GetLen() const override;
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
                                ExcEScenarioCell( sal_uInt16 nC, sal_uInt16 nR, const OUString& rTxt );

    inline sal_Size             GetStringBytes() const
                                    { return sText.GetSize(); }

    void                        WriteAddress( XclExpStream& rStrm ) const ;
    void                        WriteText( XclExpStream& rStrm ) const;

    void                        SaveXml( XclExpXmlStream& rStrm ) const;
};

class ExcEScenario : public ExcRecord
{
private:
    sal_Size                    nRecLen;
    XclExpString                sName;
    XclExpString                sComment;
    XclExpString                sUserName;
    bool                        bProtected;

    std::vector<ExcEScenarioCell> aCells;

    bool                        Append( sal_uInt16 nCol, sal_uInt16 nRow, const OUString& rTxt );

    virtual void                SaveCont( XclExpStream& rStrm ) override;

protected:
public:
                                ExcEScenario( const XclExpRoot& rRoot, SCTAB nTab );
    virtual                     ~ExcEScenario();

    virtual sal_uInt16              GetNum() const override;
    virtual sal_Size            GetLen() const override;

    virtual void                SaveXml( XclExpXmlStream& rStrm ) override;
};

class ExcEScenarioManager : public ExcRecord
{
private:
    sal_uInt16                      nActive;
    std::vector<ExcEScenario*> aScenes;

    virtual void                SaveCont( XclExpStream& rStrm ) override;

protected:
public:
                                ExcEScenarioManager( const XclExpRoot& rRoot, SCTAB nTab );
    virtual                     ~ExcEScenarioManager();

    virtual void                Save( XclExpStream& rStrm ) override;
    virtual void                SaveXml( XclExpXmlStream& rStrm ) override;

    virtual sal_uInt16              GetNum() const override;
    virtual sal_Size            GetLen() const override;
};

/** Represents a FEATHDR (SHEETPROTECTION) record that stores sheet protection
    options.  Note that a sheet still needs to save its sheet protection
    options even when it's not protected. */
class XclExpSheetProtectOptions : public XclExpRecord
{
public:
    explicit            XclExpSheetProtectOptions( const XclExpRoot& rRoot, SCTAB nTab );

private:
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    sal_uInt16      mnOptions;      /// Encoded sheet protection options.
};

/** Represents one EnhancedProtection feature in a FEAT record.
    To be written only if such feature exists. */
class XclExpSheetEnhancedProtection : public XclExpRecord
{
public:
    explicit            XclExpSheetEnhancedProtection( const XclExpRoot& rRoot, const ScEnhancedProtection& rProt );

private:
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    const XclExpRoot&       mrRoot;
    ScEnhancedProtection    maEnhancedProtection;
};

class XclCalccount : public ExcRecord
{
private:
    sal_uInt16                      nCount;
protected:
    virtual void                SaveCont( XclExpStream& rStrm ) override;
public:
                                XclCalccount( const ScDocument& );

    virtual sal_uInt16              GetNum() const override;
    virtual sal_Size            GetLen() const override;

    virtual void                SaveXml( XclExpXmlStream& rStrm ) override;
};

class XclIteration : public ExcRecord
{
private:
    sal_uInt16                      nIter;
protected:
    virtual void                SaveCont( XclExpStream& rStrm ) override;
public:
                                XclIteration( const ScDocument& );

    virtual sal_uInt16              GetNum() const override;
    virtual sal_Size            GetLen() const override;

    virtual void                SaveXml( XclExpXmlStream& rStrm ) override;
};

class XclDelta : public ExcRecord
{
private:
    double                      fDelta;
protected:
    virtual void                SaveCont( XclExpStream& rStrm ) override;
public:
                                XclDelta( const ScDocument& );

    virtual sal_uInt16              GetNum() const override;
    virtual sal_Size            GetLen() const override;

    virtual void                SaveXml( XclExpXmlStream& rStrm ) override;
};

class XclRefmode : public XclExpBoolRecord
{
public:
                                XclRefmode( const ScDocument& );

    virtual void                SaveXml( XclExpXmlStream& rStrm ) override;
};

class XclExpFileEncryption : public XclExpRecord
{
public:
    explicit XclExpFileEncryption( const XclExpRoot& rRoot );
    virtual ~XclExpFileEncryption();

private:
    virtual void WriteBody( XclExpStream& rStrm ) override;

private:
    const XclExpRoot& mrRoot;
};

/** Beginning of User Interface Records */
class XclExpInterfaceHdr : public XclExpUInt16Record
{
public:
    explicit            XclExpInterfaceHdr( sal_uInt16 nCodePage );

private:
    virtual void        WriteBody( XclExpStream& rStrm ) override;
};

/** End of User Interface Records */
class XclExpInterfaceEnd : public XclExpRecord
{
public:
    explicit XclExpInterfaceEnd();
    virtual ~XclExpInterfaceEnd();

private:
    virtual void WriteBody( XclExpStream& rStrm ) override;
};

/** Write Access User Name - This record contains the user name, which is
    the name you type when you install Excel. */
class XclExpWriteAccess : public XclExpRecord
{
public:
    explicit XclExpWriteAccess();
    virtual ~XclExpWriteAccess();

private:
    virtual void WriteBody( XclExpStream& rStrm ) override;
};

class XclExpFileSharing : public XclExpRecord
{
public:
    explicit            XclExpFileSharing( const XclExpRoot& rRoot, sal_uInt16 nPasswordHash, bool bRecommendReadOnly );

    virtual void        Save( XclExpStream& rStrm ) override;

private:
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    XclExpString        maUserName;
    sal_uInt16          mnPasswordHash;
    bool                mbRecommendReadOnly;
};

class XclExpProt4Rev : public XclExpRecord
{
public:
    explicit XclExpProt4Rev();
    virtual ~XclExpProt4Rev();

private:
    virtual void WriteBody( XclExpStream& rStrm ) override;
};

class XclExpProt4RevPass : public XclExpRecord
{
public:
    explicit XclExpProt4RevPass();
    virtual ~XclExpProt4RevPass();

private:
    virtual void WriteBody( XclExpStream& rStrm ) override;
};

class XclExpRecalcId : public XclExpDummyRecord
{
public:
    explicit XclExpRecalcId();
};

class XclExpBookExt : public XclExpDummyRecord
{
public:
    explicit XclExpBookExt();
};

#endif // _XCL97REC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
