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

#ifndef _WW8PAR_HXX
#define _WW8PAR_HXX

#include <tools/string.hxx>
#include <filter/msfilter/msdffimp.hxx>
#include <editeng/frmdir.hxx>
#include <fltshell.hxx>         // fuer den Attribut Stack

#include <vector>
#include <stack>
#include <deque>
#include <map>
#include <utility>

#include "ww8struc.hxx"     // WW8_BRC
#include "ww8scan.hxx"  // WW8Fib
#include "ww8glsy.hxx"
#include "wrtww8.hxx"
#include "../inc/msfilter.hxx"
#include <xmloff/odffields.hxx>
#include <IMark.hxx>

#include <swtypes.hxx>  // enum RndStdIds
#include <fmtfsize.hxx>
#include <fmtornt.hxx>
#include <fmtsrnd.hxx>
#include <editeng/lrspitem.hxx>
#include <oox/ole/olehelper.hxx>

#include <boost/noncopyable.hpp>

class SwDoc;
class SwPaM;
class SfxPoolItem;
class SwTxtFmtColl;
class SwPageDesc;
class SvxBoxItem;
class SwFmt;
class SwNodeIndex;
class SwFlyFrmFmt;
class SwAttrSet;
class SwNumRule;
class SwFrmFmt;
class Writer;
class SwFmtFld;
class WW8Fib;
class WW8PLCFMan;
struct WW8PLCFManResult;
class WW8RStyle;
class WW8PLCF_HdFt;
class WW8Dop;
class WW8ScannerBase;
struct WW8FieldDesc;
struct WW8FlyPara;
struct WW8SwFlyPara;
struct WW8_PIC;
class WW8TabDesc;
struct WW8_SHD;
struct WW8_OLST;
class SwNumFmt;
struct WW8_ANLD;
struct WW8_ANLV;
struct WW8_DO;
struct WW8_DPHEAD;
struct WW8_FSPA;
class SdrModel;
class SdrPage;
class SdrObject;
class SdrTextObj;
class Size;
class EditEngine;
struct SwPosition;
class WW8ReaderSave;
struct WW8PicDesc;
class Graphic;
class SwFieldType;
class SvStorage;
// alt: class SvStorageRef;
class SwAttrSet;
class GDIMetaFile;
struct ESelection;
class SfxItemSet;
class WW8ReadFieldParams;
class wwZOrderer;
class OutlinerParaObject;

namespace com{namespace sun {namespace star{
    namespace beans{ class XPropertySet;}
    namespace form { class XFormComponent;}
    namespace drawing{class XShape;}
    namespace lang{class XMultiServiceFactory;}
}}}

// defines nur fuer die WW8-variable der INI-Datei
#define WW8FL_NO_TEXT        1
#define WW8FL_NO_STYLES      2
#define WW8FL_NO_ZSTYLES     4  // keine Zeichenstyles importieren
#define WW8FL_NO_GRAF     0x80

// falls gestetzt, werden fuer Writer-Def-Styles neue Styles mit den
// WW8-Def-Style-Eigenschaften erzeugt, statt die Writer-Standards zu
// ueberschreiben
#define WW8FL_NO_DEFSTYLES        0x400

#define WW8FL_NO_OUTLINE         0x1000
#define WW8FL_NO_IMPLPASP        0x4000  // no implicit para space
#define WW8FL_NO_GRAFLAYER       0x8000

// Zusatz-Filter-Flags, gueltig ab Winword 8
#define WW8FL_NO_FLY_FOR_TXBX         1

//-----------------------------------------
//            Listen-Manager (ab Ver8)
//-----------------------------------------

struct WW8LFOInfo;

class WW8Reader : public StgReader
{
    virtual sal_uLong Read(SwDoc &, const String& rBaseURL, SwPaM &,const String &);
    sal_uLong OpenMainStream( SvStorageStreamRef& rRef, sal_uInt16& rBuffSize );
public:
    virtual int GetReaderType();

    virtual sal_Bool HasGlossaries() const;
    virtual sal_Bool ReadGlossaries( SwTextBlocks&, sal_Bool bSaveRelFiles ) const;
};

class SwWW8ImplReader;
struct WW8LSTInfo;
class WW8ListManager
{
public:
    WW8ListManager(SvStream& rSt_, SwWW8ImplReader& rReader_);
    //Min and Max possible List Levels in Word
    enum ListLevel {nMinLevel=1, nMaxLevel=9};
    //the rParaSprms returns back the original word paragraph indent
    //sprms which were attached to the original numbering format
    SwNumRule* GetNumRuleForActivation(sal_uInt16 nLFOPosition, const sal_uInt8 nLevel,
        std::vector<sal_uInt8> &rParaSprms, SwTxtNode *pNode=0);
    SwNumRule* CreateNextRule(bool bSimple);
    ~WW8ListManager();
    SwNumRule* GetNumRule(size_t i);
    size_t GetWW8LSTInfoNum() const{return maLSTInfos.size();}
private:
    wwSprmParser maSprmParser;
    SwWW8ImplReader& rReader;
    SwDoc&           rDoc;
    const WW8Fib&    rFib;
    SvStream&        rSt;
    std::vector<WW8LSTInfo* > maLSTInfos;
    boost::ptr_vector<WW8LFOInfo > pLFOInfos;// D. aus PLF LFO, sortiert genau wie im WW8 Stream
    sal_uInt16       nUniqueList; // current number for creating unique list names
    sal_uInt8* GrpprlHasSprm(sal_uInt16 nId, sal_uInt8& rSprms, sal_uInt8 nLen);
    WW8LSTInfo* GetLSTByListId(    sal_uInt32  nIdLst     ) const;
    //the rParaSprms returns back the original word paragraph indent
    //sprms which are attached to this numbering level
    bool ReadLVL(SwNumFmt& rNumFmt, SfxItemSet*& rpItemSet, sal_uInt16 nLevelStyle,
        bool bSetStartNo, std::deque<bool> &rNotReallyThere, sal_uInt16 nLevel,
        ww::bytes &rParaSprms);

    // Zeichenattribute aus GrpprlChpx
    typedef SfxItemSet* WW8aISet[nMaxLevel];
    // Zeichen Style Pointer
    typedef SwCharFmt* WW8aCFmt[nMaxLevel];

    void AdjustLVL(sal_uInt8 nLevel, SwNumRule& rNumRule, WW8aISet& rListItemSet,
        WW8aCFmt& aCharFmt, bool& bNewCharFmtCreated,
        String aPrefix = aEmptyStr);

    //No copying
    WW8ListManager(const WW8ListManager&);
    WW8ListManager& operator=(const WW8ListManager&);
    sal_uInt16 nLastLFOPosition;
};

struct WW8FlyPara
{                       // WinWord-Attribute
                        // Achtung: *Nicht* umsortieren, da Teile mit
                        // memcmp verglichen werden
    bool bVer67;
    sal_Int16 nSp26, nSp27;         // rohe Position
    sal_Int16 nSp45, nSp28;         // Breite / Hoehe
    sal_Int16 nLeMgn, nRiMgn, nUpMgn, nLoMgn;           // Raender
    sal_uInt8 nSp29;                 // rohe Bindung + Alignment
    sal_uInt8 nSp37;                 // Wrap-Mode ( 1 / 2; 0 = no Apo ? )
    WW8_BRC5 brc;               // Umrandung Top, Left, Bottom, Right, Between
    bool bBorderLines;          // Umrandungslinien
    bool bGrafApo;              // true: Dieser Rahmen dient allein dazu, die
                                // enthaltene Grafik anders als zeichengebunden
                                // zu positionieren
    bool mbVertSet;             // true if vertical positioning has been set

    WW8FlyPara(bool bIsVer67, const WW8FlyPara* pSrc = 0);
    bool operator==(const WW8FlyPara& rSrc) const;
    void Read(sal_uInt8 nSprm29, WW8PLCFx_Cp_FKP* pPap);
    void ReadFull(sal_uInt8 nSprm29, SwWW8ImplReader* pIo);
    void Read(sal_uInt8 nSprm29, WW8RStyle* pStyle);
    void ApplyTabPos(const WW8_TablePos *pTabPos);
    bool IsEmpty() const;
};

class SwWW8StyInf
{
    String      sWWStyleName;
    sal_uInt16      nWWStyleId;
public:
    rtl_TextEncoding eLTRFontSrcCharSet;    // rtl_TextEncoding fuer den Font
    rtl_TextEncoding eRTLFontSrcCharSet;    // rtl_TextEncoding fuer den Font
    rtl_TextEncoding eCJKFontSrcCharSet;    // rtl_TextEncoding fuer den Font
    SwFmt*      pFmt;
    WW8FlyPara* pWWFly;
    SwNumRule*  pOutlineNumrule;
    long        nFilePos;
    sal_uInt16      nBase;
    sal_uInt16      nFollow;
    sal_uInt16      nLFOIndex;
    sal_uInt8        nListLevel;
    sal_uInt8        nOutlineLevel;      // falls Gliederungs-Style
    sal_uInt16  n81Flags;           // Fuer Bold, Italic, ...
    sal_uInt16  n81BiDiFlags;       // Fuer Bold, Italic, ...
    SvxLRSpaceItem maWordLR;
    bool bValid;            // leer oder Valid
    bool bImported;         // fuers rekursive Importieren
    bool bColl;             // true-> pFmt ist SwTxtFmtColl
    bool bImportSkipped;    // nur true bei !bNewDoc && vorh. Style
    bool bHasStyNumRule;    // true-> Benannter NumRule in Style
    bool bHasBrokenWW6List; // true-> WW8+ style has a WW7- list
    bool bListReleventIndentSet; //true if this style's indent has
                                 //been explicitly set, it's set to the value
                                 //of pFmt->GetItemState(RES_LR_SPACE, false)
                                 //if it was possible to get the ItemState
                                 //for L of the LR space independantly
    bool bParaAutoBefore;   // For Auto spacing before a paragraph
    bool bParaAutoAfter;    // For Auto Spacing after a paragraph

    SwWW8StyInf() :
        sWWStyleName( aEmptyStr ),
        nWWStyleId( 0 ),
        eLTRFontSrcCharSet(0),
        eRTLFontSrcCharSet(0),
        eCJKFontSrcCharSet(0),
        pFmt( 0 ),
        pWWFly( 0 ),
        pOutlineNumrule( 0 ),
        nFilePos( 0 ),
        nBase( 0 ),
        nFollow( 0 ),
        nLFOIndex( USHRT_MAX ),
        nListLevel(WW8ListManager::nMaxLevel),
        nOutlineLevel( MAXLEVEL ),
        n81Flags( 0 ),
        n81BiDiFlags(0),
        maWordLR( RES_LR_SPACE ),
        bValid(false),
        bImported(false),
        bColl(false),
        bImportSkipped(false),
        bHasStyNumRule(false),
        bHasBrokenWW6List(false),
        bListReleventIndentSet(false),
        bParaAutoBefore(false),
        bParaAutoAfter(false)

    {}

    ~SwWW8StyInf()
    {
        delete pWWFly;
    }

    void SetOrgWWIdent( const String& rName, const sal_uInt16 nId )
    {
        sWWStyleName = rName;
        nWWStyleId = nId;
    }
    sal_uInt16 GetWWStyleId() const { return nWWStyleId; }
    const String& GetOrgWWName() const
    {
        return sWWStyleName;
    }
    bool IsOutline() const
    {
        return (pFmt && (MAXLEVEL > nOutlineLevel));
    }
    bool IsOutlineNumbered() const
    {
        return pOutlineNumrule && IsOutline();
    }
    const SwNumRule* GetOutlineNumrule() const
    {
        return pOutlineNumrule;
    }
    CharSet GetCharSet() const;
    CharSet GetCJKCharSet() const;
};

//-----------------------------------------
//            Stack
//-----------------------------------------
class SwWW8FltControlStack : public SwFltControlStack
{
private:
    SwWW8ImplReader& rReader;
    sal_uInt16 nToggleAttrFlags;
    sal_uInt16 nToggleBiDiAttrFlags;
    //No copying
    SwWW8FltControlStack(const SwWW8FltControlStack&);
    SwWW8FltControlStack& operator=(const SwWW8FltControlStack&);
    const SwNumFmt* GetNumFmtFromStack(const SwPosition &rPos,
        const SwTxtNode &rTxtNode);
protected:
    virtual void SetAttrInDoc(const SwPosition& rTmpPos,
        SwFltStackEntry& rEntry);

    virtual sal_Int32 GetCurrAttrCP() const;
    virtual bool IsParaEndInCPs(sal_Int32 nStart,sal_Int32 nEnd,bool bSdOD=true) const;
    //Clear the para end position recorded in reader intermittently for the least impact on loading performance
    virtual void ClearParaEndPosition();
    virtual bool CheckSdOD(sal_Int32 nStart,sal_Int32 nEnd);

public:
    SwWW8FltControlStack(SwDoc* pDo, sal_uLong nFieldFl, SwWW8ImplReader& rReader_ )
        : SwFltControlStack( pDo, nFieldFl ), rReader( rReader_ ),
        nToggleAttrFlags(0), nToggleBiDiAttrFlags(0)
    {}

    void NewAttr(const SwPosition& rPos, const SfxPoolItem& rAttr);

    virtual SwFltStackEntry* SetAttr(const SwPosition& rPos, sal_uInt16 nAttrId=0, sal_Bool bTstEnde=sal_True, long nHand=LONG_MAX, sal_Bool consumedByField=sal_False);

    void SetToggleAttr(sal_uInt8 nId, bool bOn)
    {
        if( bOn )
            nToggleAttrFlags |= (1 << nId);
        else
            nToggleAttrFlags &= ~(1 << nId);
    }

    sal_uInt16 GetToggleAttrFlags() const { return nToggleAttrFlags; }

    void SetToggleBiDiAttr(sal_uInt8 nId, bool bOn)
    {
        if( bOn )
            nToggleBiDiAttrFlags |= (1 << nId);
        else
            nToggleBiDiAttrFlags &= ~(1 << nId);
    }

    sal_uInt16 GetToggleBiDiAttrFlags() const { return nToggleBiDiAttrFlags; }
    void SetToggleAttrFlags(sal_uInt16 nFlags) { nToggleAttrFlags = nFlags; }
    void SetToggleBiDiAttrFlags(sal_uInt16 nFlags) {nToggleBiDiAttrFlags = nFlags;}

    const SfxPoolItem* GetFmtAttr(const SwPosition& rPos, sal_uInt16 nWhich);
    const SfxPoolItem* GetStackAttr(const SwPosition& rPos, sal_uInt16 nWhich);
};

//The only thing this is for is RES_FLTR_ANCHOR, anything else is an error.
//For graphics whose anchoring position would otherwise be automatically moved
//along by the insertion of text.
class SwWW8FltAnchorStack : public SwFltControlStack
{
public:
    SwWW8FltAnchorStack(SwDoc* pDo, sal_uLong nFieldFl)
        : SwFltControlStack( pDo, nFieldFl ) {}
    void AddAnchor(const SwPosition& rPos,SwFrmFmt *pFmt);
    void Flush();
private:
    //No copying
    SwWW8FltAnchorStack(const SwWW8FltAnchorStack&);
    SwWW8FltAnchorStack& operator=(const SwWW8FltAnchorStack&);
};

//For fields whose handling cannot be fully resolved until we hit the end of
//the document.
class Position
{
public:
    SwNodeIndex maMkNode;
    SwNodeIndex maPtNode;
    xub_StrLen mnMkCntnt;
    xub_StrLen mnPtCntnt;
    Position(const SwPaM &rPaM);
    Position(const Position &rEntry);
private:
    //No assignment
    Position& operator=(const Position&);
};

class SwWW8FltRefStack : public SwFltEndStack
{
public:
    SwWW8FltRefStack(SwDoc* pDo, sal_uLong nFieldFl)
        : SwFltEndStack( pDo, nFieldFl )
    {}
    bool IsFtnEdnBkmField(const SwFmtFld& rFmtFld, sal_uInt16& rBkmNo);

    struct ltstr
    {
        bool operator()(const String &r1, const String &r2) const
        {
            return r1.CompareIgnoreCaseToAscii(r2) == COMPARE_LESS;
        }
    };
    //Keep track of variable names created with fields, and the bookmark
    //mapped to their position, hopefully the same, but very possibly
    //an additional pseudo bookmark
    std::map<String, String, ltstr> aFieldVarNames;
protected:
    SwFltStackEntry *RefToVar(const SwField* pFld,SwFltStackEntry& rEntry);
    virtual void SetAttrInDoc(const SwPosition& rTmpPos,
        SwFltStackEntry& rEntry);
private:
    //No copying
    SwWW8FltRefStack(const SwWW8FltRefStack&);
    SwWW8FltRefStack& operator=(const SwWW8FltRefStack&);
};


namespace sw
{
    namespace hack
    {
        class Position
        {
        private:
            SwNodeIndex maPtNode;
            xub_StrLen mnPtCntnt;
        public:
            Position(const SwPosition &rPos);
            Position(const Position &rPos);
            operator SwPosition() const;
            SwNodeIndex GetPtNode() { return maPtNode; };
            xub_StrLen GetPtCntnt() { return mnPtCntnt; };
        };
    }
}

class WW8FieldEntry
{
    private:
        OUString msBookmarkName;
        OUString msMarkType;
        OUString msMarkCode;
        ::sw::mark::IFieldmark::parameter_map_t maParams;

    public:
        sw::hack::Position maStartPos;
        sal_uInt16 mnFieldId;
        sal_uLong mnObjLocFc;
        WW8FieldEntry(SwPosition &rPos, sal_uInt16 nFieldId) throw();
        WW8FieldEntry(const WW8FieldEntry &rOther) throw();
        WW8FieldEntry &operator=(const WW8FieldEntry &rOther) throw();
        void Swap(WW8FieldEntry &rOther) throw();

        SwNodeIndex GetPtNode() { return maStartPos.GetPtNode(); };
        xub_StrLen GetPtCntnt() { return maStartPos.GetPtCntnt(); };

        OUString GetBookmarkName();
        OUString GetBookmarkCode();
        void SetBookmarkName(OUString bookmarkName);
        void SetBookmarkType(OUString bookmarkType);
        void SetBookmarkCode(OUString bookmarkCode);
        ::sw::mark::IFieldmark::parameter_map_t& getParameters();
};


//-----------------------------------------
//    Mini-Merker fuer einige Flags
//-----------------------------------------
class WW8ReaderSave
{
private:
    WW8PLCFxSaveAll maPLCFxSave;
    SwPosition maTmpPos;
    std::deque<bool> maOldApos;
    std::deque<WW8FieldEntry> maOldFieldStack;
    SwWW8FltControlStack* mpOldStck;
    SwWW8FltAnchorStack* mpOldAnchorStck;
    sw::util::RedlineStack *mpOldRedlines;
    WW8PLCFMan* mpOldPlcxMan;
    WW8FlyPara* mpWFlyPara;
    WW8SwFlyPara* mpSFlyPara;
    SwPaM* mpPreviousNumPaM;
    const SwNumRule* mpPrevNumRule;
    WW8TabDesc* mpTableDesc;
    int mnInTable;
    sal_uInt16 mnAktColl;
    sal_Unicode mcSymbol;
    bool mbIgnoreText;
    bool mbSymbol;
    bool mbHdFtFtnEdn;
    bool mbTxbxFlySection;
    bool mbAnl;
    bool mbInHyperlink;
    bool mbPgSecBreak;
    bool mbWasParaEnd;
    bool mbHasBorder;
    bool mbFirstPara;
public:
    WW8ReaderSave(SwWW8ImplReader* pRdr, WW8_CP nStart=-1);
    void Restore(SwWW8ImplReader* pRdr);
    const SwPosition &GetStartPos() const { return maTmpPos; }
};

enum eF_ResT{ FLD_OK, FLD_TEXT, FLD_TAGIGN, FLD_TAGTXT, FLD_READ_FSPA };

class SwWW8Shade
{
public:
    Color aColor;
    SwWW8Shade(bool bVer67, const WW8_SHD& rSHD);
    SwWW8Shade(ColorData nFore, ColorData nBack, sal_uInt16 nIndex)
    {
        SetShade(nFore, nBack, nIndex);
    }
private:
    void SetShade(ColorData nFore, ColorData nBack, sal_uInt16 nIndex);
};


//-----------------------------------------
//    Formulas
//-----------------------------------------
enum SwWw8ControlType
{
    WW8_CT_EDIT,
    WW8_CT_CHECKBOX,
    WW8_CT_DROPDOWN
};

class WW8FormulaControl : private ::boost::noncopyable
{
protected:
    SwWW8ImplReader &rRdr;
public:
    WW8FormulaControl(const OUString& rN, SwWW8ImplReader &rR)
        : rRdr(rR), fUnknown(0), fDropdownIndex(0),
        fToolTip(0), fNoMark(0), fUseSize(0), fNumbersOnly(0), fDateOnly(0),
        fUnused(0), nSize(0), hpsCheckBox(20), nChecked(0), sName( rN )
    {
    }
    sal_uInt8 fUnknown:2;
    sal_uInt8 fDropdownIndex:6;
    sal_uInt8 fToolTip:1;
    sal_uInt8 fNoMark:1;
    sal_uInt8 fUseSize:1;
    sal_uInt8 fNumbersOnly:1;
    sal_uInt8 fDateOnly:1;
    sal_uInt8 fUnused:3;
    sal_uInt16 nSize;

    sal_uInt16 hpsCheckBox;
    sal_uInt16 nChecked;
    sal_uInt16 nDefaultChecked;

    OUString sTitle;
    OUString sDefault;
    OUString sFormatting;
    OUString sHelp;
    OUString sToolTip;
    std::vector<OUString> maListEntries;
    virtual ~WW8FormulaControl() {}
    void FormulaRead(SwWw8ControlType nWhich,SvStream *pD);
    virtual sal_Bool Import(const com::sun::star::uno::Reference <
        com::sun::star::lang::XMultiServiceFactory> &rServiceFactory,
        com::sun::star::uno::Reference <
        com::sun::star::form::XFormComponent> &rFComp,
        com::sun::star::awt::Size &rSz) = 0;
    OUString sName;
};

class WW8FormulaCheckBox : public WW8FormulaControl
{
private:
    //No copying
    WW8FormulaCheckBox(const WW8FormulaCheckBox&);
    WW8FormulaCheckBox& operator=(const WW8FormulaCheckBox&);

public:
    WW8FormulaCheckBox(SwWW8ImplReader &rR);

    virtual sal_Bool Import(const com::sun::star::uno::Reference <
        com::sun::star::lang::XMultiServiceFactory> &rServiceFactory,
        com::sun::star::uno::Reference <
        com::sun::star::form::XFormComponent> &rFComp,
        com::sun::star::awt::Size &rSz);
};

class WW8FormulaListBox : public WW8FormulaControl
{
private:
    //No copying
    WW8FormulaListBox(const WW8FormulaListBox&);
    WW8FormulaListBox& operator=(const WW8FormulaListBox&);

public:
    WW8FormulaListBox(SwWW8ImplReader &rR);

    virtual sal_Bool Import(const com::sun::star::uno::Reference <
        com::sun::star::lang::XMultiServiceFactory> &rServiceFactory,
        com::sun::star::uno::Reference <
        com::sun::star::form::XFormComponent> &rFComp,
        com::sun::star::awt::Size &rSz);
};

class WW8FormulaEditBox : public WW8FormulaControl
{
private:
    //No copying
    WW8FormulaEditBox(const WW8FormulaEditBox&);
    WW8FormulaEditBox& operator=(const WW8FormulaEditBox&);
public:
    WW8FormulaEditBox(SwWW8ImplReader &rR);
    //no real implementation, return false
    virtual sal_Bool Import(const com::sun::star::uno::Reference <
        com::sun::star::lang::XMultiServiceFactory> & /* rServiceFactory */,
        com::sun::star::uno::Reference <
        com::sun::star::form::XFormComponent> & /* rFComp */,
        com::sun::star::awt::Size & /* rSz */) { return sal_False; }
};

class SwMSConvertControls: public oox::ole::MSConvertOCXControls
{
public:
    SwMSConvertControls( SfxObjectShell *pDSh,SwPaM *pP );
    virtual sal_Bool InsertFormula( WW8FormulaControl &rFormula);
    virtual sal_Bool InsertControl(const com::sun::star::uno::Reference<
        com::sun::star::form::XFormComponent >& rFComp,
        const ::com::sun::star::awt::Size& rSize,
        com::sun::star::uno::Reference <
        com::sun::star::drawing::XShape > *pShape,sal_Bool bFloatingCtrl);
    bool ExportControl(WW8Export &rWrt, const SdrObject *pObj);
    virtual sal_Bool ReadOCXStream( SotStorageRef& rSrc1,
        com::sun::star::uno::Reference<
        com::sun::star::drawing::XShape > *pShapeRef=0,
        sal_Bool bFloatingCtrl=false );
private:
    sal_uInt32 GenerateObjectID();
    SwPaM *pPaM;
    sal_uInt32 mnObjectId;
};

class SwMSDffManager : public SvxMSDffManager
{
private:
    SwWW8ImplReader& rReader;
    SvStream *pFallbackStream;
    std::map<sal_uInt32,OString> aOldEscherBlipCache;

    virtual sal_Bool GetOLEStorageName( long nOLEId, OUString& rStorageName,
        SvStorageRef& rSrcStorage, com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& rDestStorage ) const;
    virtual sal_Bool ShapeHasText( sal_uLong nShapeId, sal_uLong nFilePos ) const;
    // #i32596# - new parameter <_nCalledByGroup>, which
    // indicates, if the OLE object is imported inside a group object
    virtual SdrObject* ImportOLE( long nOLEId,
                                  const Graphic& rGrf,
                                  const Rectangle& rBoundRect,
                                  const Rectangle& rVisArea,
                                  const int _nCalledByGroup,
                                  sal_Int64 nAspect ) const;

    //No copying
    SwMSDffManager(const SwMSDffManager&);
    SwMSDffManager& operator=(const SwMSDffManager&);
public:
    static sal_uInt32 GetFilterFlags();
    static sal_Int32 GetEscherLineMatch(MSO_LineStyle eStyle, MSO_SPT eShapeType,
        sal_Int32 &rThick);
    SwMSDffManager( SwWW8ImplReader& rRdr );
    void DisableFallbackStream();
    void EnableFallbackStream();
protected:
    virtual SdrObject* ProcessObj( SvStream& rSt, DffObjData& rObjData, void* pData, Rectangle& rTextRect, SdrObject* pObj );
};


class wwSection
{
public:
    wwSection(const SwPosition &rPos);
    SEPr maSep;
    WW8_BRC brc[4];
    SwNodeIndex maStart;
    SwSection *mpSection;
    SwPageDesc *mpPage;
    SvxFrameDirection meDir;
    short mLinkId;

    sal_uInt32 nPgWidth;
    sal_uInt32 nPgLeft;
    sal_uInt32 nPgRight;

    sal_uInt8 mnBorders;
    bool mbHasFootnote;
    void SetDirection();
    void SetLinkId(short sLinkId) { mLinkId = sLinkId; }
    bool DoesContainFootnote() const { return mbHasFootnote; }
    bool IsContinuous() const { return maSep.bkc == 0; }
    bool IsLinked() const { return mLinkId != 0; }
    bool IsNotProtected() const { return maSep.fUnlocked != 0; }
    bool IsVertical() const;
    sal_Int16 NoCols() const { return maSep.ccolM1 + 1; }
    sal_Int32 StandardColSeparation() const { return maSep.dxaColumns; }
    bool HasTitlePage() const { return maSep.fTitlePage ? true : false; }
    sal_uInt16 PageStartAt() const { return maSep.pgnStart; }
    bool PageRestartNo() const { return maSep.fPgnRestart ? true : false; }
    bool IsBiDi() const { return maSep.fBiDi ? true : false; }
    sal_uInt32 GetPageWidth() const { return nPgWidth; }
    sal_uInt32 GetTextAreaWidth() const
        { return GetPageWidth() - GetPageLeft() - GetPageRight(); }
    sal_uInt32 GetPageHeight() const { return maSep.yaPage; }
    sal_uInt32 GetPageLeft() const { return nPgLeft; }
    sal_uInt32 GetPageRight() const { return nPgRight; }
    bool IsLandScape() const { return maSep.dmOrientPage ? true : false; }
    bool IsFixedHeightHeader() const { return maSep.dyaTop < 0; }
    bool IsFixedHeightFooter() const { return maSep.dyaBottom < 0; }
};

class wwSectionManager
{
private:
    /*
    A queue of the ms sections in the document
    */
    SwWW8ImplReader& mrReader;
    std::deque<wwSection> maSegments;
    typedef ::std::deque<wwSection>::iterator mySegIter;
    typedef ::std::deque<wwSection>::reverse_iterator mySegrIter;

    //Num of page desc's entered into the document
    sal_uInt16 mnDesc;

    struct wwULSpaceData
    {
        bool bHasHeader, bHasFooter;
        sal_uInt32 nSwHLo, nSwFUp, nSwUp,  nSwLo;
        wwULSpaceData() : bHasHeader(false), bHasFooter(false) {}
    };

    void SetSegmentToPageDesc(const wwSection &rSection, bool bIgnoreCols);

    void GetPageULData(const wwSection &rNewSection,
        wwULSpaceData& rData) const;
    void SetPageULSpaceItems(SwFrmFmt &rFmt, wwULSpaceData& rData,
        const wwSection &rSection) const;

    void SetPage(SwPageDesc &rPageDesc, SwFrmFmt &rFmt,
        const wwSection &rSection, bool bIgnoreCols) const;

    void SetNumberingType(const wwSection &rNewSection, SwPageDesc &rPageDesc)
        const;

    void SetUseOn(wwSection &rSection);
    void SetHdFt(wwSection &rSection, int nSect, const wwSection *pPrevious);

    SwSectionFmt *InsertSection(SwPaM& rMyPaM, wwSection &rSection);
    bool SetCols(SwFrmFmt &rFmt, const wwSection &rSection,
        sal_uInt32 nNettoWidth) const;
    bool SectionIsProtected(const wwSection &rSection) const;
    void SetLeftRight(wwSection &rSection);
    bool IsNewDoc() const;
    /*
     The segment we're inserting, the start of the segments container, and the
     nodeindex of where we want the page break to (normally the segments start
     position
    */
    SwFmtPageDesc SetSwFmtPageDesc(mySegIter &rIter, mySegIter &rStart,
        bool bIgnoreCols);

    //No copying
    wwSectionManager(const wwSectionManager&);
    wwSectionManager& operator=(const wwSectionManager&);
public:
    wwSectionManager(SwWW8ImplReader &rReader) : mrReader(rReader), mnDesc(0)
        {};
    void SetCurrentSectionHasFootnote();
    bool CurrentSectionIsVertical() const;
    bool CurrentSectionIsProtected() const;
    void PrependedInlineNode(const SwPosition &rPos, const SwNode &rNode);
    sal_uInt16 CurrentSectionColCount() const;
    bool WillHavePageDescHere(SwNodeIndex aIdx) const;
    void CreateSep(const long nTxtPos, bool bMustHaveBreak);
    void InsertSegments();
    void JoinNode(const SwPosition &rPos, const SwNode &rNode);
    sal_uInt32 GetPageLeft() const;
    sal_uInt32 GetPageRight() const;
    sal_uInt32 GetPageWidth() const;
    sal_uInt32 GetWWPageTopMargin() const;
    bool empty() const { return maSegments.empty(); }
    sal_uInt32 GetTextAreaWidth() const;
};

//Various writer elements like frames start off containing a blank paragraph,
//sometimes this paragraph turns out to be extraneous, e.g. the frame should
//only contain a table with no trailing paragraph.
//
//We want to remove these extra paragraphs, but removing them during the parse
//is problematic, because we don't want to remove any paragraphs that are still
//addressed by property entries in a SwFltControlStack which have not yet been
//committed to the document.
//
//Safest thing is to not delete SwTxtNodes from a document during import, and
//remove these extraneous paragraphs at the end after all SwFltControlStack are
//destroyed.
class wwExtraneousParas : private ::boost::noncopyable
{
private:
    /*
    A vector of SwTxtNodes to erase from a document after import is complete
    */
    std::vector<SwTxtNode*> m_aTxtNodes;
    SwDoc& m_rDoc;
public:
    wwExtraneousParas(SwDoc &rDoc) : m_rDoc(rDoc) {}
    ~wwExtraneousParas() { delete_all_from_doc(); }
    void push_back(SwTxtNode *pTxtNode) { m_aTxtNodes.push_back(pTxtNode); }
    void delete_all_from_doc();
};

class wwFrameNamer : private ::boost::noncopyable
{
private:
    OUString msSeed;
    sal_Int32 mnImportedGraphicsCount;
    bool mbIsDisabled;
public:
    void SetUniqueGraphName(SwFrmFmt *pFrmFmt, const OUString &rFixedPart);
    wwFrameNamer(bool bIsDisabled, const OUString &rSeed)
        : msSeed(rSeed), mnImportedGraphicsCount(0), mbIsDisabled(bIsDisabled)
    {
    }
};

class wwSectionNamer
{
private:
    const SwDoc &mrDoc;
    String msFileLinkSeed;
    int mnFileSectionNo;
    //No copying
    wwSectionNamer(const wwSectionNamer&);
    wwSectionNamer& operator=(const wwSectionNamer&);
public:
    String UniqueName();
    wwSectionNamer(const SwDoc &rDoc, const String &rSeed)
        : mrDoc(rDoc), msFileLinkSeed(rSeed), mnFileSectionNo(0)
        { }
};

class FtnDescriptor
{
public:
    ManTypes meType;
    bool mbAutoNum;
    WW8_CP mnStartCp;
    WW8_CP mnLen;
};

struct ApoTestResults
{
    bool mbStartApo;
    bool mbStopApo;
    bool m_bHasSprm37;
    bool m_bHasSprm29;
    sal_uInt8 m_nSprm29;
    WW8FlyPara* mpStyleApo;
    ApoTestResults()
        : mbStartApo(false), mbStopApo(false), m_bHasSprm37(false)
        , m_bHasSprm29(false), m_nSprm29(0), mpStyleApo(0) {}
    bool HasStartStop() const { return (mbStartApo || mbStopApo); }
    bool HasFrame() const { return (m_bHasSprm29 || m_bHasSprm37 || mpStyleApo); }
};

struct ANLDRuleMap
{
    SwNumRule* mpOutlineNumRule;    // WinWord 6 numbering, varient 1
    SwNumRule* mpNumberingNumRule;  // WinWord 6 numbering, varient 2
    SwNumRule* GetNumRule(sal_uInt8 nNumType);
    void SetNumRule(SwNumRule*, sal_uInt8 nNumType);
    ANLDRuleMap() : mpOutlineNumRule(0), mpNumberingNumRule(0) {}
};

struct SprmReadInfo;
class SwDocShell;
struct WW8PostProcessAttrsInfo
{
    bool mbCopy;
    WW8_CP mnCpStart;
    WW8_CP mnCpEnd;
    SwPaM mPaM;
    SfxItemSet mItemSet;

    WW8PostProcessAttrsInfo(WW8_CP nCpStart, WW8_CP nCpEnd, SwPaM & rPaM);
};

#define MAX_COL 64  // WW6-description: 32, WW6-UI: 31 & WW8-UI: 63!

struct WW8TabBandDesc
{
    WW8TabBandDesc* pNextBand;
    short nGapHalf;
    short mnDefaultLeft;
    short mnDefaultTop;
    short mnDefaultRight;
    short mnDefaultBottom;
    bool mbHasSpacing;
    short nLineHeight;
    short nRows;
    sal_uInt16 maDirections[MAX_COL + 1];
    short nCenter[MAX_COL + 1]; // X-edge of all cells of this band
    short nWidth[MAX_COL + 1];  // length of all cells of this band
    short nWwCols;      // sal_uInt8 would be sufficient, alignment -> short
    short nSwCols;      // SW: number of columns for the writer
    bool bLEmptyCol;    // SW: an additional empty column at the left
    bool bREmptyCol;    // SW: same at the right
    bool bCantSplit;
    bool bCantSplit90;
    WW8_TCell* pTCs;
    sal_uInt8 nOverrideSpacing[MAX_COL + 1];
    short nOverrideValues[MAX_COL + 1][4];
    WW8_SHD* pSHDs;
    sal_uInt32* pNewSHDs;
    WW8_BRC aDefBrcs[6];

    bool bExist[MAX_COL];           // does this cell exist ?
    sal_uInt8 nTransCell[MAX_COL + 2];  // translation WW-Index -> SW-Index

    sal_uInt8 transCell(sal_uInt8 nWwCol) const
    {
        return nWwCol < SAL_N_ELEMENTS(nTransCell) ? nTransCell[nWwCol] : 0xFF;
    }

    WW8TabBandDesc();
    WW8TabBandDesc(WW8TabBandDesc& rBand);    // deep copy
    ~WW8TabBandDesc();
    static void setcelldefaults(WW8_TCell *pCells, short nCells);
    void ReadDef(bool bVer67, const sal_uInt8* pS);
    void ProcessDirection(const sal_uInt8* pParams);
    void ProcessSprmTSetBRC(bool bVer67, const sal_uInt8* pParamsTSetBRC);
    void ProcessSprmTTableBorders(bool bVer67, const sal_uInt8* pParams);
    void ProcessSprmTDxaCol(const sal_uInt8* pParamsTDxaCol);
    void ProcessSprmTDelete(const sal_uInt8* pParamsTDelete);
    void ProcessSprmTInsert(const sal_uInt8* pParamsTInsert);
    void ProcessSpacing(const sal_uInt8* pParamsTInsert);
    void ProcessSpecificSpacing(const sal_uInt8* pParamsTInsert);
    void ReadShd(const sal_uInt8* pS );
    void ReadNewShd(const sal_uInt8* pS, bool bVer67);

    enum wwDIR {wwTOP = 0, wwLEFT = 1, wwBOTTOM = 2, wwRIGHT = 3};
};

//-----------------------------------------
//            Storage-Reader
//-----------------------------------------
typedef std::vector<WW8_CP> cp_vector;

class SwWW8ImplReader
{
private:
    SwDocShell *mpDocShell;         // The Real DocShell

friend class WW8RStyle;
friend class WW8TabDesc;
friend class WW8ReaderSave;
friend struct WW8FlyPara;
friend struct WW8SwFlyPara;
friend class WW8FlySet;
friend class SwMSDffManager;
friend class SwWW8FltControlStack;
friend class WW8FormulaControl;
friend class wwSectionManager;

private:

    SvStorage* pStg;                // Input-Storage
    SvStream* pStrm;                // Input-(Storage)Stream
    SvStream* pTableStream;         // Input-(Storage)Stream
    SvStream* pDataStream;          // Input-(Storage)Stream

// allgemeines
    SwDoc& rDoc;
    SwPaM* pPaM;

    SwWW8FltControlStack* pCtrlStck;    // Stack fuer die Attribute

    /*
    This stack is for redlines, because their sequence of discovery can
    be out of order of their order of insertion into the document.
    */
    sw::util::RedlineStack *mpRedlineStack;

    /*
    This stack is for fields that get referneced later, e.g. BookMarks and TOX.
    They get inserted at the end of the document, it is the same stack for
    headers/footers/main text/textboxes/tables etc...
    */
    SwFltEndStack *pReffedStck;

    /*
    This stack is for fields whose true conversion cannot be determined until
    the end of the document, it is the same stack for headers/footers/main
    text/textboxes/tables etc... They are things that reference other things
    e.g. NoteRef and Ref, they are processed after pReffedStck
    */
    SwWW8FltRefStack *pReffingStck;

    /*
    For graphics anchors. Determines the graphics whose anchors are in the
    current paragraph, and works around the difficulty in inserting a graphic
    anchored to character before a character to be anchored to has been
    inserted. Is emptied at the end of each paragraph.
    */
    SwWW8FltAnchorStack* pAnchorStck;

    /*
    A stack of fields identifiers to keep track of any open fields that need
    to be closed. Generally word fields are inserted as writer fields as soon
    as they are encountered, and so their end point is normally unimportant.
    But hyperlink fields need to be applied as attributes to text and it is
    far easier and safer to set the end point of an attribute when we
    encounter the end marker of the field instead of caculating in advance
    where the end point will fall, to do so fully correctly duplicates the
    main logic of the filter itself.
    */
    std::deque<WW8FieldEntry> maFieldStack;
    typedef std::deque<WW8FieldEntry>::const_iterator mycFieldIter;

    /*
    A stack of open footnotes. Should only be one in it at any time.
    */
    std::deque<FtnDescriptor> maFtnStack;

    /*
    A queue of the ms sections in the document
    */
    wwSectionManager maSectionManager;

    /*
    A vector of surplus-to-requirements paragraph in the final document,
    that exist because of quirks of the SwDoc document model and/or API,
    which need to be removed.
    */
    wwExtraneousParas m_aExtraneousParas;

    /*
    A map of of tables to their follow nodes for use in inserting tables into
    already existing document, i.e. insert file
    */
    sw::util::InsertedTablesManager maInsertedTables;

    /*
    Creates unique names to give to (file link) sections (WW1/WW2/...)
    */
    wwSectionNamer maSectionNameGenerator;

    /*
    Knows how to split a series of bytes into sprms and their arguments
    */
    wwSprmParser *mpSprmParser;

    /*
    Creates unique names to give to graphics
    */
    wwFrameNamer maGrfNameGenerator;

    /*
    Knows which writer style a given word style should be imported as.
    */
    sw::util::ParaStyleMapper maParaStyleMapper;
    sw::util::CharStyleMapper maCharStyleMapper;

    /*
     Stack of textencoding being used as we progress through the document text
    */
    std::stack<rtl_TextEncoding> maFontSrcCharSets;
    std::stack<rtl_TextEncoding> maFontSrcCJKCharSets;

    /*
     Winword numbering gets imported as SwNumRules, there is a problem that
     winword can have multiple outline numberings, only one gets chosen as
     the writer outline numbering rule. The one that gets chosen is set here
     as mpChosenOutlineNumRule
    */
    SwNumRule *mpChosenOutlineNumRule;

    SwMSConvertControls *pFormImpl; // Control-Implementierung

    SwFlyFrmFmt* pFlyFmtOfJustInsertedGraphic;
    SwFrmFmt* pFmtOfJustInsertedApo;
    SwPaM* pPreviousNumPaM;
    const SwNumRule* pPrevNumRule;

    //Keep track of APO environments
    std::deque<bool> maApos;
    typedef std::deque<bool>::const_iterator mycApoIter;
    /*
    Keep track of generated Ruby character formats we we can minimize the
    number of character formats created
    */
    std::vector<const SwCharFmt*> aRubyCharFmts;

    WW8PostProcessAttrsInfo * mpPostProcessAttrsInfo;

    WW8Fib* pWwFib;
    WW8Fonts* pFonts;
    WW8Dop* pWDop;
    WW8ListManager* pLstManager;
    WW8ScannerBase* pSBase;
    WW8PLCFMan* pPlcxMan;
    std::map<short, String> aLinkStringMap;

    std::set<const SwNode*> maTxtNodesHavingFirstLineOfstSet; // #i103711#
    std::set<const SwNode*> maTxtNodesHavingLeftIndentSet; // #i105414#

    WW8RStyle* pStyles;     // Pointer auf die Style-Einleseklasse
    SwFmt* pAktColl;        // gerade zu erzeugende Collection
                            // ( ist ausserhalb einer Style-Def immer 0 )
    SfxItemSet* pAktItemSet;// gerade einzulesende Zeichenattribute
                            // (ausserhalb des WW8ListManager Ctor's immer 0)
    std::vector<SwWW8StyInf> vColl;
    const SwTxtFmtColl* pDfltTxtFmtColl;    // Default
    SwFmt* pStandardFmtColl;// "Standard"

    WW8PLCF_HdFt* pHdFt;        // Pointer auf Header / Footer - Scannerklasse

    WW8FlyPara* pWFlyPara;      // WW-Parameter
    WW8SwFlyPara* pSFlyPara;    // daraus erzeugte Sw-Parameter

    WW8TabDesc* pTableDesc;     // Beschreibung der Tabelleneigenschaften
    //Keep track of tables within tables
    std::stack<WW8TabDesc*> maTableStack;

    ANLDRuleMap maANLDRules;
    WW8_OLST* pNumOlst;         // Gliederung im Text

    SwNode* pNode_FLY_AT_PARA; // set: WW8SwFlyPara()   read: CreateSwTable()

    SdrModel* pDrawModel;
    SdrPage* pDrawPg;
    EditEngine* mpDrawEditEngine;
    wwZOrderer *pWWZOrder;

    SwFieldType* pNumFldType;   // fuer Nummernkreis

    SwMSDffManager* pMSDffManager;

    std::vector<String>* mpAtnNames;
    boost::shared_ptr< std::map<sal_uInt32, int> > mpAtnIndexes;
    boost::shared_ptr<WW8PLCFspecial> mpAtnStarts;
    boost::shared_ptr<WW8PLCFspecial> mpAtnEnds;

    sw::util::AuthorInfos m_aAuthorInfos;
    String sBaseURL;

                                // Ini-Flags:
    sal_uLong nIniFlags;            // Flags aus der writer.ini
    sal_uLong nIniFlags1;           // dito ( zusaetzliche Flags )
    sal_uLong nFieldFlags;          // dito fuer Feldern
    sal_uLong nFieldTagAlways[3];   // dito fuers Taggen von Feldern
    sal_uLong nFieldTagBad[3];      // dito fuers Taggen von nicht importierbaren F.
    bool m_bRegardHindiDigits;  // import digits in CTL scripts as Hindi numbers

    WW8_CP nDrawCpO;            // Anfang der Txbx-SubDocs

    sal_uLong nPicLocFc;            // Picture Location in File (FC)
    sal_uLong nObjLocFc;            // Object Location in File (FC)

    sal_Int32 nIniFlyDx;            // X-Verschiebung von Flys
    sal_Int32 nIniFlyDy;            // Y-Verschiebung von Flys

    rtl_TextEncoding eTextCharSet;    // Default charset for Text
    rtl_TextEncoding eStructCharSet;  // rtl_TextEncoding for structures
    rtl_TextEncoding eHardCharSet;    // Hard rtl_TextEncoding-Attribute
    sal_uInt16 nProgress;           // %-Angabe fuer Progressbar
    sal_uInt16 nAktColl;            // gemaess WW-Zaehlung
    sal_uInt16 nFldNum;             // laufende Nummer dafuer
    sal_uInt16 nLFOPosition;

    short nCharFmt;             // gemaess WW-Zaehlung, <0 fuer keine

    short nDrawXOfs, nDrawYOfs;
    short nDrawXOfs2, nDrawYOfs2;

    sal_Unicode cSymbol;        // aktuell einzulesendes Symbolzeichen


    sal_uInt8 nWantedVersion;        // urspruenglich vom Writer
                                // angeforderte WW-Doc-Version


    sal_uInt8 nSwNumLevel;           // LevelNummer fuer Outline / Nummerierung
    sal_uInt8 nWwNumType;            // Gliederung / Nummerg / Aufzaehlg
    sal_uInt8 nListLevel;

    sal_uInt8 nPgChpDelim;           // ChapterDelim from PageNum
    sal_uInt8 nPgChpLevel;           // ChapterLevel of Heading from PageNum

    bool mbNewDoc;          // Neues Dokument ?
    bool bReadNoTbl;        // Keine Tabellen
    bool bPgSecBreak;       // Page- oder Sectionbreak ist noch einzufuegen
    bool bSpec;             // Special-Char im Text folgt
    bool bObj;              // Obj im Text
    bool bTxbxFlySection;   // FlyFrame, der als Ersatz fuer Winword Textbox eingefuegt wurde
    bool bHasBorder;        // fuer Buendelung der Border
    bool bSymbol;           // z.B. Symbol statt Times
    bool bIgnoreText;       // z.B. fuer FieldVanish
    int  nInTable;          // wird gerade eine Tabelle eingelesen
    bool bWasTabRowEnd;     // Tabelle : Row End Mark
    bool bWasTabCellEnd;    // table: Cell End Mark

    bool bShdTxtCol;        // Textfarbe indirekt gesetzt ( Hintergrund sw )
    bool bCharShdTxtCol;    // Textfarbe indirekt gesetzt ( Zeichenhintergrund sw )
    bool bAnl;              // Nummerierung in Bearbeitung
                                // Anl heisst Autonumber level

    bool bHdFtFtnEdn;       // Spezialtext: Kopf- Fuss- usw.
    bool bFtnEdn;           // Fussnote oder Endnote
    bool bIsHeader;         // Text aus Header wird gelesen ( Zeilenhoehe )
    bool bIsFooter;         // Text aus Footer wird gelesen ( Zeilenhoehe )

    bool bIsUnicode;            // aktuelles Text-Stueck ist als 2-Bytiger-Unicode kodiert
                                // bitte NICHT als Bitfeld kodieren!

    bool bCpxStyle;         // Style im Complex Part
    bool bStyNormal;        // Style mit Id 0 wird gelesen
    bool bWWBugNormal;      // WW-Version nit Bug Dya in Style Normal
    bool bNoAttrImport;     // Attribute ignorieren zum Ignorieren v. Styles
    bool bInHyperlink;      // Sonderfall zum einlesen eines 0x01
                                   // siehe: SwWW8ImplReader::Read_F_Hyperlink()
    bool bWasParaEnd;

    // praktische Hilfsvariablen:
    bool bVer67;            // ( (6 == nVersion) || (7 == nVersion) );
    bool bVer6;             //   (6 == nVersion);
    bool bVer7;             //   (7 == nVersion);
    bool bVer8;             //   (8 == nVersion);

    bool bEmbeddObj;        // EmbeddField gelesen

    bool bAktAND_fNumberAcross; // current active Annotated List Deskriptor - ROW flag

    bool bNoLnNumYet;       // no Line Numbering has been activated yet (we import
                            //     the very 1st Line Numbering and ignore the rest)

    bool bFirstPara;        // first paragraph?
    bool bFirstParaOfPage;//cs2c--xushanchuan add for bug11210
    bool bParaAutoBefore;
    bool bParaAutoAfter;

    bool bDropCap;
    sal_Int32 nDropCap;

    int nIdctHint;
    bool bBidi;
    bool bReadTable;
    boost::shared_ptr<SwPaM> mpTableEndPaM;

    cp_vector maEndParaPos;
    WW8_CP maCurrAttrCP;
    bool mbOnLoadingMain:1;
//---------------------------------------------

    const SprmReadInfo& GetSprmReadInfo(sal_uInt16 nId) const;

    bool StyleExists(unsigned int nColl) const { return (nColl < vColl.size()); }
    SwWW8StyInf *GetStyle(sal_uInt16 nColl) const;
    void AppendTxtNode(SwPosition& rPos);

    void Read_HdFt(int nSect, const SwPageDesc *pPrev,
        const wwSection &rSection);
    void Read_HdFtText(long nStartCp, long nLen, SwFrmFmt* pHdFtFmt);
    void Read_HdFtTextAsHackedFrame(long nStart, long nLen,
        SwFrmFmt &rHdFtFmt, sal_uInt16 nPageWidth);

    bool isValid_HdFt_CP(WW8_CP nHeaderCP) const;

    bool HasOwnHeaderFooter(sal_uInt8 nWhichItems, sal_uInt8 grpfIhdt, int nSect);

    void HandleLineNumbering(const wwSection &rSection);

    void CopyPageDescHdFt( const SwPageDesc* pOrgPageDesc,
                           SwPageDesc* pNewPageDesc, sal_uInt8 nCode );

    void DeleteStk(SwFltControlStack* prStck);
    void DeleteCtrlStk()    { DeleteStk( pCtrlStck  ); pCtrlStck   = 0; }
    void DeleteRefStks()
    {
        DeleteStk( pReffedStck );
        pReffedStck = 0;
        DeleteStk( pReffingStck );
        pReffingStck = 0;
    }
    void DeleteAnchorStk()  { DeleteStk( pAnchorStck ); pAnchorStck = 0; }
    void emulateMSWordAddTextToParagraph(const OUString& rAddString);
    void simpleAddTextToParagraph(const String& rAddString);
    bool HandlePageBreakChar();
    bool ReadChar(long nPosCp, long nCpOfs);
    bool ReadPlainChars(WW8_CP& rPos, long nEnd, long nCpOfs);
    bool ReadChars(WW8_CP& rPos, WW8_CP nNextAttr, long nTextEnd, long nCpOfs);
    bool LangUsesHindiNumbers(sal_uInt16 nLang);
    sal_Unicode TranslateToHindiNumbers(sal_Unicode);

    void SetDocumentGrid(SwFrmFmt &rFmt, const wwSection &rSection);

    void ProcessAktCollChange(WW8PLCFManResult& rRes, bool* pStartAttr,
        bool bCallProcessSpecial);
    long ReadTextAttr(WW8_CP& rTxtPos, bool& rbStartLine);
    void ReadAttrs(WW8_CP& rNext, WW8_CP& rTxtPos, bool& rbStartLine);
    void CloseAttrEnds();
    bool ReadText(long nStartCp, long nTextLen, ManTypes nType);

    void ReadRevMarkAuthorStrTabl( SvStream& rStrm, sal_Int32 nTblPos,
        sal_Int32 nTblSiz, SwDoc& rDoc );

    void Read_HdFtFtnText( const SwNodeIndex* pSttIdx, long nStartCp,
                           long nLen, ManTypes nType );

    void ImportTox( int nFldId, String aStr );

    void EndSprm( sal_uInt16 nId );
    // #i103711#
    // #i105414#
    void NewAttr( const SfxPoolItem& rAttr,
                  const bool bFirstLineOfStSet = false,
                  const bool bLeftIndentSet = false );

    bool GetFontParams(sal_uInt16, FontFamily&, String&, FontPitch&,
        rtl_TextEncoding&);
    bool SetNewFontAttr(sal_uInt16 nFCode, bool bSetEnums, sal_uInt16 nWhich);
    void ResetCharSetVars();
    void ResetCJKCharSetVars();

    const SfxPoolItem* GetFmtAttr( sal_uInt16 nWhich );
    bool JoinNode(SwPaM &rPam, bool bStealAttr = false);

    bool IsBorder(const WW8_BRC* pbrc, bool bChkBtwn = false) const;

    //Set closest writer border equivalent into rBox from pbrc, optionally
    //recording true winword dimensions in pSizeArray. nSetBorders to mark a
    //border which has been previously set to a value and for which becoming
    //empty is valid. Set bCheBtwn to work with paragraphs that have a special
    //between paragraphs border

    // Note #i20672# we can't properly support between lines so best to ignore
    // them for now
    bool SetBorder(SvxBoxItem& rBox, const WW8_BRC* pbrc, short *pSizeArray=0,
        sal_uInt8 nSetBorders=0xFF) const;
    void GetBorderDistance(const WW8_BRC* pbrc, Rectangle& rInnerDist) const;
    sal_uInt16 GetParagraphAutoSpace(bool fDontUseHTMLAutoSpacing);
    bool SetShadow(SvxShadowItem& rShadow, const short *pSizeArray,
        const WW8_BRC *pbrc) const;
    //returns true is a shadow was set
    bool SetFlyBordersShadow(SfxItemSet& rFlySet, const WW8_BRC *pbrc,
        short *SizeArray=0) const;
    void SetPageBorder(SwFrmFmt &rFmt, const wwSection &rSection) const;

    sal_Int32 MatchSdrBoxIntoFlyBoxItem( const Color& rLineColor,
        MSO_LineStyle eLineStyle, MSO_LineDashing eDashing, MSO_SPT eShapeType, sal_Int32 &rLineWidth,
        SvxBoxItem& rBox );
    void MatchSdrItemsIntoFlySet( SdrObject*    pSdrObj, SfxItemSet &aFlySet,
        MSO_LineStyle eLineStyle, MSO_LineDashing eDashing, MSO_SPT eShapeType, Rectangle &rInnerDist );
    void AdjustLRWrapForWordMargins(const SvxMSDffImportRec &rRecord,
        SvxLRSpaceItem &rLR);
    void AdjustULWrapForWordMargins(const SvxMSDffImportRec &rRecord,
        SvxULSpaceItem &rUL);
    void MapWrapIntoFlyFmt(SvxMSDffImportRec* pRecord, SwFrmFmt* pFlyFmt);

    void SetAttributesAtGrfNode(SvxMSDffImportRec const* pRecord,
            SwFrmFmt *pFlyFmt, WW8_FSPA *pF);

    bool IsDropCap();
    bool IsListOrDropcap() { return (!pAktItemSet  || bDropCap); };

    WW8FlyPara *ConstructApo(const ApoTestResults &rApo,
        const WW8_TablePos *pTabPos);
    bool StartApo(const ApoTestResults &rApo, const WW8_TablePos *pTabPos);
    void StopApo();
    bool TestSameApo(const ApoTestResults &rApo, const WW8_TablePos *pTabPos);
    ApoTestResults TestApo(int nCellLevel, bool bTableRowEnd,
        const WW8_TablePos *pTabPos);

    void EndSpecial();
    bool ProcessSpecial(bool &rbReSync, WW8_CP nStartCp);
    sal_uInt16 TabRowSprm(int nLevel) const;

    bool ReadGrafFile(String& rFileName, Graphic*& rpGraphic,
       const WW8_PIC& rPic, SvStream* pSt, sal_uLong nFilePos, bool* pDelIt);

    void ReplaceObj(const SdrObject &rReplaceTextObj,
        SdrObject &rSubObj);

    SwFlyFrmFmt* MakeGrafNotInCntnt(const WW8PicDesc& rPD,
        const Graphic* pGraph, const String& rFileName,
        const SfxItemSet& rGrfSet);

    SwFrmFmt* MakeGrafInCntnt(const WW8_PIC& rPic, const WW8PicDesc& rPD,
        const Graphic* pGraph, const String& rFileName,
        const SfxItemSet& rGrfSet);

    SwFrmFmt *AddAutoAnchor(SwFrmFmt *pFmt);
    SwFrmFmt* ImportGraf1(WW8_PIC& rPic, SvStream* pSt, sal_uLong nFilePos);
    SwFrmFmt* ImportGraf(SdrTextObj* pTextObj = 0, SwFrmFmt* pFlyFmt = 0);

    SdrObject* ImportOleBase( Graphic& rGraph, const Graphic* pGrf=0,
        const SfxItemSet* pFlySet=0, const Rectangle& aVisArea = Rectangle() );

    SwFrmFmt* ImportOle( const Graphic* = 0, const SfxItemSet* pFlySet = 0,
        const SfxItemSet* pGrfSet = 0, const Rectangle& aVisArea = Rectangle() );
    SwFlyFrmFmt* InsertOle(SdrOle2Obj &rObject, const SfxItemSet &rFlySet,
        const SfxItemSet &rGrfSet);

    bool ImportFormulaControl(WW8FormulaControl &rBox,WW8_CP nStart,
        SwWw8ControlType nWhich);

    void ImportDop();

    //This converts MS Asian Typography information into OOo's
    void ImportDopTypography(const WW8DopTypography &rTypo);

    sal_uLong LoadThroughDecryption(SwPaM& rPaM ,WW8Glossary *pGloss);
    sal_uLong SetSubStreams(SvStorageStreamRef &rTableStream, SvStorageStreamRef &rDataStream);
    sal_uLong CoreLoad(WW8Glossary *pGloss, const SwPosition &rPos);

    void ReadDocVars();

    bool StartTable(WW8_CP nStartCp);
    bool InEqualApo(int nLvl) const;
    bool InLocalApo() const { return InEqualApo(nInTable); }
    bool InEqualOrHigherApo(int nLvl) const;
    bool InAnyApo() const { return InEqualOrHigherApo(1); }
    void TabCellEnd();
    void StopTable();
    short GetTableLeft();
    bool IsInvalidOrToBeMergedTabCell() const;

// Nummerierungen / Aufzaehlungen ( Autonumbered List Data Descriptor )
// Liste:        ANLD ( Autonumbered List Data Descriptor )
//   eine Ebene: ANLV ( Autonumber Level Descriptor )
//
// Chg7-8:
// Listen sind in WW8 eigene Strukturen, die ueber folgende drei Tabellen
// verwaltet werden: rglst, hpllfo und hsttbListNames
// die Strukturen hierfuer sind: LSTF, LVLF, LFO LFOLVL

    void SetAnlvStrings(SwNumFmt &rNum, WW8_ANLV &rAV, const sal_uInt8* pTxt,
        bool bOutline);
    void SetAnld(SwNumRule* pNumR, WW8_ANLD* pAD, sal_uInt8 nSwLevel, bool bOutLine);
    void SetNumOlst( SwNumRule* pNumR, WW8_OLST* pO, sal_uInt8 nSwLevel );
    SwNumRule* GetStyRule();

    void StartAnl(const sal_uInt8* pSprm13);
    void NextAnlLine(const sal_uInt8* pSprm13);
    void StopAllAnl(bool bGoBack = true);
    void StopAnlToRestart(sal_uInt8 nType, bool bGoBack = true);

// GrafikLayer

    bool ReadGrafStart(void* pData, short nDataSiz, WW8_DPHEAD* pHd,
        const WW8_DO* pDo, SfxAllItemSet &rSet);
    SdrObject *ReadLine(WW8_DPHEAD* pHd, const WW8_DO* pDo,
        SfxAllItemSet &rSet);
    SdrObject *ReadRect(WW8_DPHEAD* pHd, const WW8_DO* pDo,
        SfxAllItemSet &rSet);
    SdrObject *ReadElipse(WW8_DPHEAD* pHd, const WW8_DO* pDo,
        SfxAllItemSet &rSet);
    SdrObject *ReadArc(WW8_DPHEAD* pHd, const WW8_DO* pDo,
        SfxAllItemSet &rSet);
    SdrObject *ReadPolyLine(WW8_DPHEAD* pHd, const WW8_DO* pDo,
        SfxAllItemSet &rSet);
    ESelection GetESelection( long nCpStart, long nCpEnd );
    void InsertTxbxStyAttrs( SfxItemSet& rS, sal_uInt16 nColl );
    void InsertAttrsAsDrawingAttrs(long nStartCp, long nEndCp, ManTypes eType, bool bONLYnPicLocFc=false);

    bool GetTxbxTextSttEndCp(WW8_CP& rStartCp, WW8_CP& rEndCp, sal_uInt16 nTxBxS,
        sal_uInt16 nSequence);
    bool GetRangeAsDrawingString(String& rString, long StartCp, long nEndCp, ManTypes eType);
    OutlinerParaObject* ImportAsOutliner(String &rString, WW8_CP nStartCp, WW8_CP nEndCp, ManTypes eType);
    SwFrmFmt* InsertTxbxText(SdrTextObj* pTextObj, Size* pObjSiz,
        sal_uInt16 nTxBxS, sal_uInt16 nSequence, long nPosCp, SwFrmFmt* pFlyFmt,
        bool bMakeSdrGrafObj, bool& rbEraseTextObj,
        bool* pbTestTxbxContainsText = 0, long* pnStartCp = 0,
        long* pnEndCp = 0, bool* pbContainsGraphics = 0,
        SvxMSDffImportRec* pRecord = 0);
    bool TxbxChainContainsRealText( sal_uInt16 nTxBxS,
                                    long&  rStartCp,
                                    long&  rEndCp );
    SdrObject *ReadTxtBox(WW8_DPHEAD* pHd, const WW8_DO* pDo,
        SfxAllItemSet &rSet);
    SdrObject *ReadCaptionBox(WW8_DPHEAD* pHd, const WW8_DO* pDo,
        SfxAllItemSet &rSet);
    SdrObject *ReadGroup(WW8_DPHEAD* pHd, const WW8_DO* pDo,
        SfxAllItemSet &rSet);
    SdrObject *ReadGrafPrimitive(short& rLeft, const WW8_DO* pDo,
        SfxAllItemSet &rSet);
    void ReadGrafLayer1( WW8PLCFspecial* pPF, long nGrafAnchorCp );
    SdrObject* CreateContactObject(SwFrmFmt* pFlyFmt);
    RndStdIds ProcessEscherAlign(SvxMSDffImportRec* pRecord, WW8_FSPA *pFSPA,
        SfxItemSet &rFlySet, bool bOrgObjectWasReplace);
    bool MiserableRTLGraphicsHack(SwTwips &rLeft, SwTwips nWidth,
        sal_Int16 eHoriOri, sal_Int16 eHoriRel);
    SwFrmFmt* Read_GrafLayer( long nGrafAnchorCp );
    SwFlyFrmFmt* ImportReplaceableDrawables( SdrObject* &rpObject,
        SdrObject* &rpOurNewObject, SvxMSDffImportRec* pRecord, WW8_FSPA *pF,
        SfxItemSet &rFlySet );
    SwFlyFrmFmt *ConvertDrawTextToFly( SdrObject* &rpObject,
        SdrObject* &rpOurNewObject, SvxMSDffImportRec* pRecord,
        RndStdIds eAnchor, WW8_FSPA *pF, SfxItemSet &rFlySet );
    SwFrmFmt* MungeTextIntoDrawBox(SdrObject* pTrueObject,
        SvxMSDffImportRec *pRecord, long nGrafAnchorCp, SwFrmFmt *pRetFrmFmt);

    void GrafikCtor();
    void GrafikDtor();

// anderes
    String GetFieldResult( WW8FieldDesc* pF );
    void MakeTagString( String& rStr, const String& rOrg );
    void UpdateFields();
    OUString ConvertFFileName(const OUString& rRaw);
    long Read_F_Tag( WW8FieldDesc* pF );
    void InsertTagField( const sal_uInt16 nId, const String& rTagText );
    long ImportExtSprm(WW8PLCFManResult* pRes);
    void EndExtSprm(sal_uInt16 nSprmId);
    void ReadDocInfo();

// Ver8-Listen

    void RegisterNumFmtOnTxtNode(sal_uInt16 nActLFO, sal_uInt8 nActLevel,
        bool bSetAttr = true);
    void RegisterNumFmtOnStyle(sal_uInt16 nStyle);
    void SetStylesList(sal_uInt16 nStyle, sal_uInt16 nActLFO,
        sal_uInt8 nActLevel);
    void RegisterNumFmt(sal_uInt16 nActLFO, sal_uInt8 nActLevel);

// spaeter zu ersetzen durch Aufruf in entsprechend erweiterten SvxMSDffManager

    const String* GetAnnotationAuthor(sal_uInt16 nIdx);
    int GetAnnotationIndex(sal_uInt32 nTag);
    WW8_CP GetAnnotationStart(int nIndex);
    WW8_CP GetAnnotationEnd(int nIndex);

    // Schnittstellen fuer die Toggle-Attribute
    void SetToggleAttr(sal_uInt8 nAttrId, bool bOn);
    void SetToggleBiDiAttr(sal_uInt8 nAttrId, bool bOn);
    void _ChkToggleAttr( sal_uInt16 nOldStyle81Mask, sal_uInt16 nNewStyle81Mask );

    void ChkToggleAttr( sal_uInt16 nOldStyle81Mask, sal_uInt16 nNewStyle81Mask )
    {
        if( nOldStyle81Mask != nNewStyle81Mask &&
            pCtrlStck->GetToggleAttrFlags() )
            _ChkToggleAttr( nOldStyle81Mask, nNewStyle81Mask );
    }

    void _ChkToggleBiDiAttr( sal_uInt16 nOldStyle81Mask, sal_uInt16 nNewStyle81Mask );

    void ChkToggleBiDiAttr( sal_uInt16 nOldStyle81Mask, sal_uInt16 nNewStyle81Mask )
    {
        if( nOldStyle81Mask != nNewStyle81Mask &&
            pCtrlStck->GetToggleBiDiAttrFlags() )
            _ChkToggleBiDiAttr( nOldStyle81Mask, nNewStyle81Mask );
    }

    void PopTableDesc();
    void MoveInsideFly(const SwFrmFmt *pFlyFmt);
    SwTwips MoveOutsideFly(SwFrmFmt *pFlyFmt, const SwPosition &rPos,
        bool bTableJoin = true);

    void SetOutLineStyles();

    bool SetSpacing(SwPaM &rMyPam, int nSpace, bool bIsUpper);
    bool SetUpperSpacing(SwPaM &pMyPam, int nSpace);
    bool SetLowerSpacing(SwPaM &rMyPam, int nSpace);

    bool IsInlineEscherHack() const
        {return !maFieldStack.empty() ? maFieldStack.back().mnFieldId == 95 : false; };

    void StoreMacroCmds();

    // #i84783#
    // determine object attribute "Layout in Table Cell"
    bool IsObjectLayoutInTableCell( const sal_uInt32 nLayoutInTableCell ) const;
    bool ReadGlobalTemplateSettings( const OUString& sCreatedFrom, const com::sun::star::uno::Reference< com::sun::star::container::XNameContainer >& xPrjNameMap );
    //No copying
    SwWW8ImplReader(const SwWW8ImplReader &);
    SwWW8ImplReader& operator=(const SwWW8ImplReader&);
public:     // eigentlich private, geht aber leider nur public
    sal_uInt16 GetToggleAttrFlags() const;
    sal_uInt16 GetToggleBiDiAttrFlags() const;
    void SetToggleAttrFlags(sal_uInt16 nFlags);
    void SetToggleBiDiAttrFlags(sal_uInt16 nFlags);
    WW8_CP GetCurrAttrCP() const {return maCurrAttrCP;}
    bool IsParaEndInCPs(sal_Int32 , sal_Int32,bool bSdOD=true) const;
    //Clear the para end position recorded in reader intermittently for the least impact on loading performance
    void ClearParaEndPosition();

    long Read_Ftn(WW8PLCFManResult* pRes);
    sal_uInt16 End_Ftn();
    long Read_Field(WW8PLCFManResult* pRes);
    sal_uInt16 End_Field();
    long Read_Book(WW8PLCFManResult*);
    long Read_And(WW8PLCFManResult* pRes);

    // Attribute

    void Read_Special(sal_uInt16, const sal_uInt8*, short nLen);
    void Read_Obj(sal_uInt16, const sal_uInt8*, short nLen);
    void Read_PicLoc(sal_uInt16, const sal_uInt8* pData, short nLen );
    void Read_BoldUsw(sal_uInt16 nId, const sal_uInt8*, short nLen);
    void Read_Bidi(sal_uInt16 nId, const sal_uInt8*, short nLen);
    void Read_BoldBiDiUsw(sal_uInt16 nId, const sal_uInt8*, short nLen);
    void Read_SubSuper(         sal_uInt16, const sal_uInt8*, short nLen );
    bool ConvertSubToGraphicPlacement();
    SwFrmFmt *ContainsSingleInlineGraphic(const SwPaM &rRegion);
    void Read_SubSuperProp(     sal_uInt16, const sal_uInt8*, short nLen );
    void Read_Underline(        sal_uInt16, const sal_uInt8*, short nLen );
    void Read_TxtColor(         sal_uInt16, const sal_uInt8*, short nLen );
    void openFont(sal_uInt16 nFCode, sal_uInt16 nId);
    void closeFont(sal_uInt16 nId);
    void Read_FontCode(         sal_uInt16, const sal_uInt8*, short nLen );
    void Read_FontSize(         sal_uInt16, const sal_uInt8*, short nLen );
    void Read_CharSet(sal_uInt16 , const sal_uInt8* pData, short nLen);
    void Read_Language(         sal_uInt16, const sal_uInt8*, short nLen );
    void Read_CColl(            sal_uInt16, const sal_uInt8*, short nLen );
    void Read_Kern(             sal_uInt16, const sal_uInt8* pData, short nLen );
    void Read_FontKern(         sal_uInt16, const sal_uInt8* pData, short nLen );
    void Read_Emphasis(         sal_uInt16, const sal_uInt8* pData, short nLen );
    void Read_ScaleWidth(       sal_uInt16, const sal_uInt8* pData, short nLen );
    void Read_Relief(           sal_uInt16, const sal_uInt8* pData, short nLen);
    void Read_TxtAnim(      sal_uInt16, const sal_uInt8* pData, short nLen);

    void Read_NoLineNumb(       sal_uInt16 nId, const sal_uInt8* pData, short nLen );

    void Read_LR(               sal_uInt16 nId, const sal_uInt8*, short nLen );
    void Read_UL(               sal_uInt16 nId, const sal_uInt8*, short nLen );
    void Read_ParaAutoBefore(sal_uInt16 , const sal_uInt8 *pData, short nLen);
    void Read_ParaAutoAfter(sal_uInt16 , const sal_uInt8 *pData, short nLen);
    void Read_ParaContextualSpacing( sal_uInt16 nId, const sal_uInt8* pData, short nLen );
    void Read_LineSpace(        sal_uInt16, const sal_uInt8*, short nLen );
    void Read_Justify(sal_uInt16, const sal_uInt8*, short nLen);
    void Read_IdctHint(sal_uInt16, const sal_uInt8*, short nLen);
    bool IsRightToLeft();
    void Read_RTLJustify(sal_uInt16, const sal_uInt8*, short nLen);
    void Read_Hyphenation(      sal_uInt16, const sal_uInt8* pData, short nLen );
    void Read_WidowControl(     sal_uInt16, const sal_uInt8* pData, short nLen );
    void Read_AlignFont(        sal_uInt16, const sal_uInt8* pData, short nLen );
    void Read_UsePgsuSettings(  sal_uInt16, const sal_uInt8* pData, short nLen );
    void Read_KeepLines(        sal_uInt16, const sal_uInt8* pData, short nLen );
    void Read_KeepParas(        sal_uInt16, const sal_uInt8* pData, short nLen );
    void Read_BreakBefore(      sal_uInt16, const sal_uInt8* pData, short nLen );
    void Read_Apo(sal_uInt16 nId, const sal_uInt8* pData, short nLen);
    void Read_ApoPPC(sal_uInt16, const sal_uInt8* pData, short);

    void Read_BoolItem(         sal_uInt16 nId, const sal_uInt8*, short nLen );

    void Read_Border(           sal_uInt16 nId, const sal_uInt8* pData, short nLen );
    void Read_Tab(              sal_uInt16 nId, const sal_uInt8* pData, short nLen );
    void Read_Symbol(sal_uInt16, const sal_uInt8* pData, short nLen);
    void Read_FldVanish(        sal_uInt16 nId, const sal_uInt8* pData, short nLen );

    // Revision Marks ( == Redlining )

    // insert or delete content (change char attributes resp.)
    void Read_CRevisionMark(RedlineType_t eType, const sal_uInt8* pData, short nLen);
    // insert new content
    void Read_CFRMark(sal_uInt16 , const sal_uInt8* pData, short nLen);
    // delete old content
    void Read_CFRMarkDel(sal_uInt16 , const sal_uInt8* pData, short nLen);
    // change properties of content (e.g. char formating)
    void Read_CPropRMark(sal_uInt16 , const sal_uInt8* pData, short nLen); // complex!


    void Read_TabRowEnd(        sal_uInt16, const sal_uInt8* pData, short nLen );
    void Read_TabCellEnd(        sal_uInt16, const sal_uInt8* pData, short nLen );
    bool ParseTabPos(WW8_TablePos *aTabPos, WW8PLCFx_Cp_FKP* pPap);
    void Read_Shade(            sal_uInt16, const sal_uInt8* pData, short nLen );
    void Read_ANLevelNo(        sal_uInt16, const sal_uInt8* pData, short nLen );
    void Read_ANLevelDesc(      sal_uInt16, const sal_uInt8* pData, short nLen );

    // Gliederungsebene Ver8
    void Read_POutLvl(sal_uInt16, const sal_uInt8* pData, short nLen);

    void Read_OLST(             sal_uInt16, const sal_uInt8* pData, short nLen );

    void Read_CharShadow(       sal_uInt16, const sal_uInt8* pData, short nLen );
    void Read_CharHighlight(    sal_uInt16, const sal_uInt8* pData, short nLen );
                                        // Ver8-Listen

    void Read_ListLevel(        sal_uInt16 nId, const sal_uInt8* pData, short nLen);

    /**
     * read and interpret the sprmPIlfo used to determine which list
     * contains the paragraph.
     * @param     nId   unused (sprm value, 0x460b for sprmPIlfo).
     * @param[in] pData operand.
     * @param[in] nLen  size of the operand (pData) in byte, should be 2.
     *                  -1 to indicate the actual level is finished.
     */
    void Read_LFOPosition(      sal_uInt16 nId, const sal_uInt8* pData, short nLen);
    bool SetTxtFmtCollAndListLevel(const SwPaM& rRg, SwWW8StyInf& rStyleInfo);

    void Read_StyleCode(sal_uInt16, const sal_uInt8* pData, short nLen);
    void Read_Majority(sal_uInt16, const sal_uInt8* , short );
    void Read_DoubleLine_Rotate( sal_uInt16, const sal_uInt8* pDATA, short nLen);

    void Read_TxtForeColor(sal_uInt16, const sal_uInt8* pData, short nLen);
    void Read_TxtBackColor(sal_uInt16, const sal_uInt8* pData, short nLen);
    void Read_ParaBackColor(sal_uInt16, const sal_uInt8* pData, short nLen);
    void Read_ParaBiDi(sal_uInt16, const sal_uInt8* pData, short nLen);
    static sal_uInt32 ExtractColour(const sal_uInt8* &rpData, bool bVer67);

    void Read_UnderlineColor(sal_uInt16, const sal_uInt8* pData, short nLen);
    long MapBookmarkVariables(const WW8FieldDesc* pF,String &rOrigName,
        const String &rData);
    String GetMappedBookmark(const String &rOrigName);

    // Felder
    eF_ResT Read_F_Input(WW8FieldDesc*, String& rStr);
    eF_ResT Read_F_InputVar(WW8FieldDesc*, String& rStr);
    eF_ResT Read_F_ANumber( WW8FieldDesc*, String& );
    eF_ResT Read_F_DocInfo( WW8FieldDesc* pF, String& rStr );
    eF_ResT Read_F_Author( WW8FieldDesc*, String& );
    eF_ResT Read_F_TemplName( WW8FieldDesc*, String& );
    short GetTimeDatePara(String& rStr, sal_uInt32& rFormat, sal_uInt16 &rLang,
        int nWhichDefault, bool bHijri = false);
    bool ForceFieldLanguage(SwField &rFld, sal_uInt16 nLang);
    eF_ResT Read_F_DateTime( WW8FieldDesc*, String& rStr );
    eF_ResT Read_F_FileName( WW8FieldDesc*, String& rStr);
    eF_ResT Read_F_Anz( WW8FieldDesc* pF, String& );
    eF_ResT Read_F_CurPage( WW8FieldDesc*, String& );
    eF_ResT Read_F_Ref( WW8FieldDesc* pF, String& );

    eF_ResT Read_F_Set( WW8FieldDesc*, String& rStr );
    eF_ResT Read_F_PgRef( WW8FieldDesc*, String& rStr );
    eF_ResT Read_F_NoteReference( WW8FieldDesc* pF, String& rStr );

    eF_ResT Read_F_Tox( WW8FieldDesc* pF, String& rStr );
    bool AddExtraOutlinesAsExtraStyles(SwTOXBase& rBase);
    eF_ResT Read_F_Symbol( WW8FieldDesc*, String& rStr );
    eF_ResT Read_F_Embedd( WW8FieldDesc*, String& rStr );
    eF_ResT Read_F_FormTextBox( WW8FieldDesc* pF, String& rStr);
    eF_ResT Read_F_FormCheckBox( WW8FieldDesc* pF, String& rStr );
    eF_ResT Read_F_FormListBox( WW8FieldDesc* pF, String& rStr);
    com::sun::star::awt::Size MiserableDropDownFormHack(const String &rString,
        com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>&
        rPropSet);

    eF_ResT Read_F_Macro( WW8FieldDesc*, String& rStr);
    eF_ResT Read_F_DBField( WW8FieldDesc*, String& rStr );
    eF_ResT Read_F_DBNext( WW8FieldDesc*, String& );
    eF_ResT Read_F_DBNum( WW8FieldDesc*, String& );
    eF_ResT Read_F_Equation( WW8FieldDesc*, String& );
    void Read_SubF_Ruby( WW8ReadFieldParams& rReadParam);
    void Read_SubF_Combined( WW8ReadFieldParams& rReadParam);
    eF_ResT Read_F_IncludePicture( WW8FieldDesc*, String& rStr );
    eF_ResT Read_F_IncludeText(    WW8FieldDesc*, String& rStr );
    eF_ResT Read_F_Seq( WW8FieldDesc*, String& rStr );

    eF_ResT Read_F_OCX(WW8FieldDesc*, String&);
    eF_ResT Read_F_Hyperlink(WW8FieldDesc*, String& rStr);
        eF_ResT Read_F_Shape(WW8FieldDesc* pF, String& rStr);
    eF_ResT Read_F_HTMLControl( WW8FieldDesc* pF, String& rStr);


    void DeleteFormImpl();

    short ImportSprm( const sal_uInt8* pPos, sal_uInt16 nId = 0 );

    bool SearchRowEnd(WW8PLCFx_Cp_FKP* pPap,WW8_CP &rStartCp, int nLevel) const;

    const WW8Fib& GetFib() const    { return *pWwFib; }
    SwDoc& GetDoc() const           { return rDoc; }
    sal_uInt16 GetNAktColl()  const     { return nAktColl; }
    void SetNAktColl( sal_uInt16 nColl ) { nAktColl = nColl;    }
    void SetAktItemSet( SfxItemSet* pItemSet ) { pAktItemSet = pItemSet; }
    sal_uInt16 StyleUsingLFO( sal_uInt16 nLFOIndex ) const ;
    const SwFmt* GetStyleWithOrgWWName( String& rName ) const ;

    static bool GetPictGrafFromStream(Graphic& rGraphic, SvStream& rSrc);
    static void PicRead( SvStream *pDataStream, WW8_PIC *pPic, bool bVer67);
    static bool ImportOleWMF( SvStorageRef xSrc1, GDIMetaFile &rWMF,
        long &rX, long &rY);
    static ColorData GetCol(sal_uInt8 nIco);

    SwWW8ImplReader( sal_uInt8 nVersionPara, SvStorage* pStorage, SvStream* pSt,
        SwDoc& rD, const String& rBaseURL, bool bNewDoc );

    const String& GetBaseURL() const { return sBaseURL; }
    // Laden eines kompletten DocFiles
    sal_uLong LoadDoc( SwPaM&,WW8Glossary *pGloss=0);
    CharSet GetCurrentCharSet();
    CharSet GetCurrentCJKCharSet();

    void PostProcessAttrs();
};

bool CanUseRemoteLink(const String &rGrfName);
void UseListIndent(SwWW8StyInf &rStyle, const SwNumFmt &rFmt);
void SetStyleIndent(SwWW8StyInf &rStyleInfo, const SwNumFmt &rFmt);
// #i103711#
// #i105414#
void SyncIndentWithList( SvxLRSpaceItem &rLR,
                         const SwNumFmt &rFmt,
                         const bool bFirstLineOfStSet,
                         const bool bLeftIndentSet );
long GetListFirstLineIndent(const SwNumFmt &rFmt);
String BookmarkToWriter(const String &rBookmark);
bool RTLGraphicsHack(SwTwips &rLeft, SwTwips nWidth,
    sal_Int16 eHoriOri, sal_Int16 eHoriRel, SwTwips nPageLeft,
    SwTwips nPageRight, SwTwips nPageSize);
void MatchEscherMirrorIntoFlySet(const SvxMSDffImportRec &rRecord,
    SfxItemSet &rFlySet);
bool RTLDrawingsHack(long &rLeft, long nWidth,
    sal_Int16 eHoriOri, sal_Int16 eHoriRel, SwTwips nPageLeft,
    SwTwips nPageRight, SwTwips nPageSize);
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
