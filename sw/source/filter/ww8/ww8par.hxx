/*************************************************************************
 *
 *  $RCSfile: ww8par.hxx,v $
 *
 *  $Revision: 1.129 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 09:12:42 $
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
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#ifndef _WW8PAR_HXX
#define _WW8PAR_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _MSDFFIMP_HXX
#include <svx/msdffimp.hxx>
#endif
#ifndef _MSOCXIMEX_HXX
#include <svx/msocximex.hxx>
#endif
#ifndef _SVX_FRMDIR_HXX
#include <svx/frmdir.hxx>
#endif

#ifndef _FLTSHELL_HXX
#include <fltshell.hxx>         // fuer den Attribut Stack
#endif

#ifndef __SGI_STL_VECTOR
#include <vector>
#endif
#ifndef __SGI_STL_STACK
#include <stack>
#endif
#ifndef __SGI_STL_DEQUE
#include <deque>
#endif
#ifndef __SGI_STL_MAP
#include <map>
#endif
#ifndef __SGI_STL_UTILITY
#include <utility>
#endif

#ifndef SW_TRACER
#include "tracer.hxx"
#endif
#ifndef _WW8STRUC_HXX
#include "ww8struc.hxx"     // WW8_BRC
#endif
#ifndef _WW8SCAN_HXX
#include "ww8scan.hxx"  // WW8Fib
#endif
#ifndef _WW8GLSY_HXX
#include "ww8glsy.hxx"
#endif
#ifndef SW_MS_MSFILTER_HXX
#include "../inc/msfilter.hxx"
#endif

class SwDoc;
class SwPaM;
class SfxPoolItem;
class SwTxtFmtColl;
class SwPageDesc;
class SvxBoxItem;
class SwFmt;
class SwNodeIndex;
class SwFlyFrmFmt;
class SvxTabStopItem;
class SwAttrSet;
class SwNumRule;
class SwFrmFmt;
class Writer;

class SwWW8StyInf;
class WW8Fib;
class WW8PLCFMan;
struct WW8PLCFManResult;
class WW8RStyle;
class WW8PLCF_HdFt;
class Plcx_Fkp;
class WW8PLCFx_SEPX;
class WW8Dop;
class WW8ScannerBase;
struct WW8FieldDesc;
struct WW8FlyPara;
struct WW8SwFlyPara;
struct WmfFileHd;
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
class Plc1;
class SwDrawFrmFmt;
class SdrModel;
class SdrPage;
class SdrObjList;
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
struct WW8PLCFxDesc;
class _ReadFieldParams;
class SdrAttrObj;
class wwZOrderer;
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
typedef WW8LFOInfo* WW8LFOInfo_Ptr;
// Redlining: match WinWord author ids to StarWriter author ids
struct WW8AuthorInfo;
typedef WW8AuthorInfo* WW8AuthorInfo_Ptr;
struct WW8OleMap;
typedef WW8OleMap* WW8OleMap_Ptr;

SV_DECL_PTRARR_DEL(WW8LFOInfos,WW8LFOInfo_Ptr,16,16)
SV_DECL_PTRARR_SORT_DEL(WW8AuthorInfos, WW8AuthorInfo_Ptr,16,16)
SV_DECL_PTRARR_SORT_DEL(WW8OleMaps, WW8OleMap_Ptr,16,16)

struct WW8OleMap
{
    sal_uInt32 mnWWid;
    String msStorageName;

    WW8OleMap(sal_uInt32 nWWid)
        : mnWWid(nWWid) {}

     WW8OleMap(sal_uInt32 nWWid, String sStorageName)
        : mnWWid(nWWid), msStorageName(sStorageName) {}

    bool operator==(const WW8OleMap & rEntry) const
    {
        return (mnWWid == rEntry.mnWWid);
    }
    bool operator<(const WW8OleMap & rEntry) const
    {
        return (mnWWid < rEntry.mnWWid);
    }
};

class SwWW8ImplReader;
struct WW8LSTInfo;
class SwNodeNum;
class WW8ListManager
{
public:
    WW8ListManager(SvStream& rSt_, SwWW8ImplReader& rReader_);
    //Min and Max possible List Levels in Word
    enum ListLevel {nMinLevel=1, nMaxLevel=9};
    //the rParaSprms returns back the original word paragraph indent
    //sprms which were attached to the original numbering format
    SwNumRule* GetNumRuleForActivation(USHORT nLFOPosition, BYTE nLevel,
        std::vector<sal_uInt8> &rParaSprms, SwNodeNum *pNodeNum=0) const;
    SwNumRule* CreateNextRule(bool bSimple);
    ~WW8ListManager();
private:
    wwSprmParser maSprmParser;
    SwWW8ImplReader& rReader;
    SwDoc&           rDoc;
    const WW8Fib&    rFib;
    SvStream&        rSt;
    std::vector<WW8LSTInfo* > maLSTInfos;
    WW8LFOInfos* pLFOInfos;// D. aus PLF LFO, sortiert genau wie im WW8 Stream
    USHORT       nUniqueList; // current number for creating unique list names
    BYTE* GrpprlHasSprm(USHORT nId, BYTE& rSprms, BYTE nLen);
    WW8LSTInfo* GetLSTByListId(    ULONG  nIdLst     ) const;
    //the rParaSprms returns back the original word paragraph indent
    //sprms which are attached to this numbering level
    bool ReadLVL(SwNumFmt& rNumFmt, SfxItemSet*& rpItemSet, USHORT nLevelStyle,
        bool bSetStartNo, std::deque<bool> &rNotReallyThere, sal_uInt16 nLevel,
        std::vector<sal_uInt8> &rParaSprms);

    // Zeichenattribute aus GrpprlChpx
    typedef SfxItemSet* WW8aISet[nMaxLevel];
    // Zeichen Style Pointer
    typedef SwCharFmt* WW8aCFmt[nMaxLevel];

    void AdjustLVL(BYTE nLevel, SwNumRule& rNumRule, WW8aISet& rListItemSet,
        WW8aCFmt& aCharFmt, bool& bNewCharFmtCreated,
        String aPrefix = aEmptyStr);

    //No copying
    WW8ListManager(const WW8ListManager&);
    WW8ListManager& operator=(const WW8ListManager&);
};

//-----------------------------------------
//            Stack
//-----------------------------------------
class SwWW8FltControlStack : public SwFltControlStack
{
private:
    SwWW8ImplReader& rReader;
    USHORT nToggleAttrFlags;
    USHORT nToggleBiDiAttrFlags;
    //No copying
    SwWW8FltControlStack(const SwWW8FltControlStack&);
    SwWW8FltControlStack& operator=(const SwWW8FltControlStack&);
    const SwNumFmt* GetNumFmtFromStack(const SwPosition &rPos,
        const SwTxtNode &rTxtNode);
protected:
    virtual void SetAttrInDoc(const SwPosition& rTmpPos,
        SwFltStackEntry* pEntry);
public:
    SwWW8FltControlStack(SwDoc* pDo, ULONG nFieldFl, SwWW8ImplReader& rReader_ )
        : SwFltControlStack( pDo, nFieldFl ), rReader( rReader_ ),
        nToggleAttrFlags(0), nToggleBiDiAttrFlags(0)
    {}

    void NewAttr(const SwPosition& rPos, const SfxPoolItem& rAttr);
    void SetAttr(const SwPosition& rPos, USHORT nAttrId=0, bool bTstEnde=true,
        long nHand=LONG_MAX);
    void SetToggleAttr(BYTE nId, bool bOn)
    {
        if( bOn )
            nToggleAttrFlags |= (1 << nId);
        else
            nToggleAttrFlags &= ~(1 << nId);
    }
    USHORT GetToggleAttrFlags() const { return nToggleAttrFlags; }
    void SetToggleBiDiAttr(BYTE nId, bool bOn)
    {
        if( bOn )
            nToggleBiDiAttrFlags |= (1 << nId);
        else
            nToggleBiDiAttrFlags &= ~(1 << nId);
    }
    USHORT GetToggleBiDiAttrFlags() const { return nToggleBiDiAttrFlags; }
    void SetToggleAttrFlags(USHORT nFlags) { nToggleAttrFlags = nFlags; }
    void SetToggleBiDiAttrFlags(USHORT nFlags) {nToggleBiDiAttrFlags = nFlags;}

    const SfxPoolItem* GetFmtAttr(const SwPosition& rPos, USHORT nWhich);
    const SfxPoolItem* GetStackAttr(const SwPosition& rPos, USHORT nWhich);
};

class wwRedlineStack
{
private:
    std::vector<SwFltStackEntry *> maStack;
    typedef std::vector<SwFltStackEntry *>::reverse_iterator myriter;
    SwDoc &mrDoc;
public:
    explicit wwRedlineStack(SwDoc &rDoc) : mrDoc(rDoc) {}
    void open(const SwPosition& rPos, const SfxPoolItem& rAttr);
    void close(const SwPosition& rPos, SwRedlineType eType);
    void closeall(const SwPosition& rPos);
    ~wwRedlineStack();
private:
    //No copying
    wwRedlineStack(const wwRedlineStack&);
    wwRedlineStack& operator=(const wwRedlineStack&);
};

//The only thing this is for is RES_FLTR_ANCHOR, anything else is an error.
//For graphics whose anchoring position would otherwise be automatically moved
//along by the insertion of text.
class SwWW8FltAnchorStack : public SwFltControlStack
{
public:
    SwWW8FltAnchorStack(SwDoc* pDo, ULONG nFieldFl)
        : SwFltControlStack( pDo, nFieldFl ) {}
    void AddAnchor(const SwPosition& rPos,SwFrmFmt *pFmt);
    void RemoveAnchor(const SwFrmFmt *pFmt);
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
    SwWW8FltRefStack(SwDoc* pDo, ULONG nFieldFl)
        : SwFltEndStack( pDo, nFieldFl )
    {}
    bool IsFtnEdnBkmField(const SwFmtFld& rFmtFld, USHORT& rBkmNo);

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
    SwFltStackEntry *RefToVar(const SwField* pFld,SwFltStackEntry *pEntry);
    virtual void SetAttrInDoc(const SwPosition& rTmpPos,
        SwFltStackEntry* pEntry);
private:
    //No copying
    SwWW8FltRefStack(const SwWW8FltRefStack&);
    SwWW8FltRefStack& operator=(const SwWW8FltRefStack&);
};

//-----------------------------------------
//     Redlining Authors
//-----------------------------------------
struct WW8AuthorInfo
{
    USHORT nWWAuthorId;
    USHORT nOurId;

    WW8AuthorInfo(USHORT nWWAuthorId_, USHORT nOurId_ = 0):
        nWWAuthorId( nWWAuthorId_ ),
        nOurId(      nOurId_ )
        {}
    bool operator==(const WW8AuthorInfo& rEntry) const
    {
        return (nWWAuthorId == rEntry.nWWAuthorId);
    }
    bool operator<(const WW8AuthorInfo& rEntry) const
    {
        return (nWWAuthorId < rEntry.nWWAuthorId);
    }
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
        };
    }
}

class FieldEntry
{
public:
    sw::hack::Position maStartPos;
    sal_uInt16 mnFieldId;
    FieldEntry(SwPosition &rPos, sal_uInt16 nFieldId) throw();
    FieldEntry(const FieldEntry &rOther) throw();
    FieldEntry &operator=(const FieldEntry &rOther) throw();
    void Swap(FieldEntry &rOther) throw();
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
    std::deque<FieldEntry> maOldFieldStack;
    SwWW8FltControlStack* mpOldStck;
    SwWW8FltAnchorStack* mpOldAnchorStck;
    wwRedlineStack *mpOldRedlines;
    WW8PLCFMan* mpOldPlcxMan;
    WW8FlyPara* mpWFlyPara;
    WW8SwFlyPara* mpSFlyPara;
    SwPaM* mpPreviousNumPaM;
    const SwNumRule* mpPrevNumRule;
    WW8TabDesc* mpTableDesc;
    int mnInTable;
    USHORT mnAktColl;
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


class WW8FormulaControl : public OCX_Control
{
protected:
    SwWW8ImplReader &rRdr;
public:
    WW8FormulaControl(const String& rN, SwWW8ImplReader &rR)
        : OCX_Control(rN), rRdr(rR), fUnknown(0), fDropdownIndex(0),
        fToolTip(0), fNoMark(0), fUseSize(0), fNumbersOnly(0), fDateOnly(0),
        fUnused(0), nSize(0), hpsCheckBox(20), nChecked(0)
    {
    }
    UINT8 fUnknown:2;
    UINT8 fDropdownIndex:6;
    UINT8 nDefaultChecked;
    UINT8 fToolTip:1;
    UINT8 fNoMark:1;
    UINT8 fUseSize:1;
    UINT8 fNumbersOnly:1;
    UINT8 fDateOnly:1;
    UINT8 fUnused:3;
    UINT16 nSize;

    UINT16 hpsCheckBox;
    UINT16 nChecked;

    String sTitle;
    String sDefault;
    String sFormatting;
    String sHelp;
    String sToolTip;
    std::vector<String> maListEntries;
    virtual ~WW8FormulaControl() {}
    void FormulaRead(SwWw8ControlType nWhich,SvStream *pD);
private:
    //No copying
    WW8FormulaControl(const WW8FormulaControl&);
    WW8FormulaControl& operator=(const WW8FormulaControl&);
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
#if 0
    //#i3029# we are no longer importing editboxes as uno textboxes, using
    //input fields instead for superior layout.
    virtual sal_Bool Import(const com::sun::star::uno::Reference <
        com::sun::star::lang::XMultiServiceFactory> &rServiceFactory,
        com::sun::star::uno::Reference <
        com::sun::star::form::XFormComponent> &rFComp,
        com::sun::star::awt::Size &rSz);
#endif
};

class SwMSConvertControls: public SvxMSConvertOCXControls
{
public:
    SwMSConvertControls( SfxObjectShell *pDSh,SwPaM *pP ) :
        SvxMSConvertOCXControls( pDSh,pP ) {}
    virtual sal_Bool InsertFormula( WW8FormulaControl &rFormula);
    virtual sal_Bool InsertControl(const com::sun::star::uno::Reference<
        com::sun::star::form::XFormComponent >& rFComp,
        const ::com::sun::star::awt::Size& rSize,
        com::sun::star::uno::Reference <
        com::sun::star::drawing::XShape > *pShape,BOOL bFloatingCtrl);
    bool ExportControl(Writer &rWrt, const SdrObject *pObj);
};

class SwMSDffManager : public SvxMSDffManager
{
private:
    SwWW8ImplReader& rReader;
    SvStream *pFallbackStream;
    List *pOldEscherBlipCache;

    virtual BOOL GetOLEStorageName( long nOLEId, String& rStorageName,
        SvStorageRef& rSrcStorage, SvStorageRef& rDestStorage ) const;
    virtual BOOL ShapeHasText( ULONG nShapeId, ULONG nFilePos ) const;
    virtual SdrObject* ImportOLE( long nOLEId, const Graphic& rGrf,
        const Rectangle& rBoundRect ) const;

    //No copying
    SwMSDffManager(const SwMSDffManager&);
    SwMSDffManager& operator=(const SwMSDffManager&);
public:
    static UINT32 GetFilterFlags();
    static INT32 GetEscherLineMatch(MSO_LineStyle eStyle, MSO_SPT eShapeType,
        INT32 &rThick);
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
    SwPageDesc *mpTitlePage;
    SwPageDesc *mpPage;
    SvxFrameDirection meDir;
    short mLinkId;

    sal_uInt32 nPgWidth;
    sal_uInt32 nPgLeft;
    sal_uInt32 nPgRight;

    BYTE mnBorders;
    bool mbHasFootnote;
    void SetDirection();
    void SetLinkId(short sLinkId) { mLinkId = sLinkId; }
    bool DoesContainFootnote() const { return mbHasFootnote; }
    bool IsContinous() const { return maSep.bkc == 0; }
    bool IsLinked() const { return mLinkId != 0; }
    bool IsNotProtected() const { return maSep.fUnlocked != 0; }
    bool IsVertical() const;
    sal_Int16 NoCols() const { return maSep.ccolM1 + 1; }
    sal_Int32 StandardColSeperation() const { return maSep.dxaColumns; }
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

    void SetSegmentToPageDesc(const wwSection &rSection, bool bTitlePage,
        bool bIgnoreCols);

    void GetPageULData(const wwSection &rNewSection, bool bFirst,
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
    USHORT CurrentSectionColCount() const;
    bool WillHavePageDescHere(SwNodeIndex aIdx) const;
    void CreateSep(const long nTxtPos, bool bMustHaveBreak);
    void InsertSegments();
    void JoinNode(const SwPosition &rPos, const SwNode &rNode);
    sal_uInt32 GetPageLeft() const;
    sal_uInt32 GetPageRight() const;
    sal_uInt32 GetPageWidth() const;
    sal_uInt32 GetTextAreaWidth() const;
};

class wwFrameNamer
{
private:
    String msSeed;
    int mnImportedGraphicsCount;
    bool mbIsDisabled;
    //No copying
    wwFrameNamer(const wwFrameNamer&);
    wwFrameNamer& operator=(const wwFrameNamer&);
public:
    void SetUniqueGraphName(SwFrmFmt *pFrmFmt,const String &rFixedPart);
    wwFrameNamer(bool bIsDisabled, const String &rSeed)
        : msSeed(rSeed), mnImportedGraphicsCount(0), mbIsDisabled(bIsDisabled)
        { }
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
    const BYTE* mpSprm37;
    const BYTE* mpSprm29;
    WW8FlyPara* mpStyleApo;
    ApoTestResults() :
        mbStartApo(false), mbStopApo(false), mpSprm37(0), mpSprm29(0),
        mpStyleApo(0) {}
    bool HasStartStop() const { return (mbStartApo || mbStopApo); }
    bool HasFrame() const { return (mpSprm29 || mpSprm37 || mpStyleApo); }
};

struct ANLDRuleMap
{
    SwNumRule* mpOutlineNumRule;    // WinWord 6 numbering, varient 1
    SwNumRule* mpNumberingNumRule;  // WinWord 6 numbering, varient 2
    SwNumRule* GetNumRule(BYTE nNumType);
    void SetNumRule(SwNumRule*, BYTE nNumType);
    ANLDRuleMap() : mpOutlineNumRule(0), mpNumberingNumRule(0) {}
};

//-----------------------------------------
//            Storage-Reader
//-----------------------------------------
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

public:
    /*
    To log unimplemented features
    */
    sw::log::Tracer maTracer;
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
    wwRedlineStack *mpRedlineStack;

    /*
    This stack is for fields whose true conversion cannot be determined until
    the end of the document, it is the same stack for headers/footers/main
    text/textboxes/tables etc...
    */
    SwWW8FltRefStack *pRefStck;

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
    std::deque<FieldEntry> maFieldStack;
    typedef std::deque<FieldEntry>::const_iterator mycFieldIter;

    /*
    A stack of open footnotes. Should only be one in it at any time.
    */
    std::deque<FtnDescriptor> maFtnStack;

    /*
    A queue of the ms sections in the document
    */
    wwSectionManager maSectionManager;

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

    WW8Fib* pWwFib;
    WW8Fonts* pFonts;
    WW8Dop* pWDop;
    WW8ListManager* pLstManager;
    WW8ScannerBase* pSBase;
    WW8PLCFMan* pPlcxMan;
    std::map<short, String> aLinkStringMap;

    WW8RStyle* pStyles;     // Pointer auf die Style-Einleseklasse
    SwFmt* pAktColl;        // gerade zu erzeugende Collection
                            // ( ist ausserhalb einer Style-Def immer 0 )
    SfxItemSet* pAktItemSet;// gerade einzulesende Zeichenattribute
                            // (ausserhalb des WW8ListManager Ctor's immer 0)
    SwWW8StyInf* pCollA;    // UEbersetzungs-Array der Styles
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

    SwNode* pNode_FLY_AT_CNTNT; // set: WW8SwFlyPara()   read: CreateSwTable()

    SdrModel* pDrawModel;
    SdrPage* pDrawPg;
    EditEngine* pDrawEditEngine;
    wwZOrderer *pWWZOrder;

    SwFieldType* pNumFldType;   // fuer Nummernkreis

    SwMSDffManager* pMSDffManager;

    std::vector<String>* mpAtnNames;

    WW8AuthorInfos* pAuthorInfos;

                                // Ini-Flags:
    ULONG nIniFlags;            // Flags aus der writer.ini
    ULONG nIniFlags1;           // dito ( zusaetzliche Flags )
    ULONG nFieldFlags;          // dito fuer Feldern
    ULONG nFieldTagAlways[3];   // dito fuers Taggen von Feldern
    ULONG nFieldTagBad[3];      // dito fuers Taggen von nicht importierbaren F.

    WW8_CP nDrawCpO;            // Anfang der Txbx-SubDocs

    ULONG nPicLocFc;            // Picture Location in File (FC)
    ULONG nObjLocFc;            // Object Location in File (FC)

    INT32 nIniFlyDx;            // X-Verschiebung von Flys
    INT32 nIniFlyDy;            // Y-Verschiebung von Flys

    rtl_TextEncoding eTextCharSet;    // Default charset for Text
    rtl_TextEncoding eStructCharSet;  // rtl_TextEncoding for structures
    rtl_TextEncoding eHardCharSet;    // Hard rtl_TextEncoding-Attribute
    USHORT nProgress;           // %-Angabe fuer Progressbar
    USHORT nColls;              // Groesse des Arrays
    USHORT nAktColl;            // gemaess WW-Zaehlung
    USHORT nDrawTxbx;           // Nummer der Textbox ( noetig ?? )
    USHORT nFldNum;             // laufende Nummer dafuer
    USHORT nLFOPosition;

    short nCharFmt;             // gemaess WW-Zaehlung, <0 fuer keine

    short nDrawXOfs, nDrawYOfs;
    short nDrawXOfs2, nDrawYOfs2;

    sal_Unicode cSymbol;        // aktuell einzulesendes Symbolzeichen


    BYTE nWantedVersion;        // urspruenglich vom Writer
                                // angeforderte WW-Doc-Version


    BYTE nSwNumLevel;           // LevelNummer fuer Outline / Nummerierung
    BYTE nWwNumType;            // Gliederung / Nummerg / Aufzaehlg
    BYTE nListLevel;

    BYTE nPgChpDelim;           // ChapterDelim from PageNum
    BYTE nPgChpLevel;           // ChapterLevel of Heading from PageNum

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

    bool bPgChpLevel;       // ChapterLevel of Heading from PageNum
    bool bEmbeddObj;        // EmbeddField gelesen

    bool bAktAND_fNumberAcross; // current active Annotated List Deskriptor - ROW flag

    bool bNoLnNumYet;       // no Line Numbering has been activated yet (we import
                            //     the very 1st Line Numbering and ignore the rest)

    bool bFirstPara;        // first paragraph?

    bool bParaAutoBefore;
    bool bParaAutoAfter;

    bool bDropCap;
    int nDropCap;

//---------------------------------------------

    bool StyleExists(int nColl) const { return (nColl < nColls); }
    SwWW8StyInf *GetStyle(USHORT nColl) const;
    void AppendTxtNode(SwPosition& rPos);
    void GetNoninlineNodeAttribs(const SwTxtNode *pNode,
        std::vector<const xub_StrLen*> &rPositions);

    void Read_HdFt(bool bIsTitle, int nSect, const SwPageDesc *pPrev,
        const wwSection &rSection);
    void Read_HdFtText(long nStartCp, long nLen, SwFrmFmt* pHdFtFmt);
    void Read_HdFtTextAsHackedFrame(long nStart, long nLen,
        SwFrmFmt &rHdFtFmt, sal_uInt16 nPageWidth);

    bool HasOwnHeaderFooter(BYTE nWhichItems, BYTE grpfIhdt, int nSect);

    void HandleLineNumbering(const wwSection &rSection);

    void CopyPageDescHdFt( const SwPageDesc* pOrgPageDesc,
                           SwPageDesc* pNewPageDesc, BYTE nCode );

    void DeleteStk(SwFltControlStack* prStck);
    void DeleteCtrlStk()    { DeleteStk( pCtrlStck  ); pCtrlStck   = 0; }
    void DeleteRefStk()     { DeleteStk( pRefStck ); pRefStck = 0; }
    void DeleteAnchorStk()  { DeleteStk( pAnchorStck ); pAnchorStck = 0; }
    bool AddTextToParagraph(const String& sAddString);
    bool HandlePageBreakChar();
    bool ReadChar(long nPosCp, long nCpOfs);
    bool ReadPlainChars(long& rPos, long nEnd, long nCpOfs);
    bool ReadChars(long& rPos, long nNextAttr, long nTextEnd, long nCpOfs);

    void SetDocumentGrid(SwFrmFmt &rFmt, const wwSection &rSection);

    void ProcessAktCollChange(WW8PLCFManResult& rRes, bool* pStartAttr,
        bool bCallProcessSpecial);
    long ReadTextAttr(long& rTxtPos, bool& rbStartLine);
    void ReadAttrs(long& rNext, long& rTxtPos, bool& rbStartLine);
    void CloseAttrEnds();
    bool ReadText(long nStartCp, long nTextLen, short nType);

    void ReadRevMarkAuthorStrTabl( SvStream& rStrm, INT32 nTblPos,
        INT32 nTblSiz, SwDoc& rDoc );

    void Read_HdFtFtnText( const SwNodeIndex* pSttIdx, long nStartCp,
                           long nLen, short nType );

    void ImportTox( int nFldId, String aStr );

    void EndSprm( USHORT nId );
    void NewAttr( const SfxPoolItem& rAttr );

    bool GetFontParams(USHORT, FontFamily&, String&, FontPitch&,
        rtl_TextEncoding&);
    bool SetNewFontAttr(USHORT nFCode, bool bSetEnums, USHORT nWhich);
    void ResetCharSetVars();

    const SfxPoolItem* GetFmtAttr( USHORT nWhich );
    bool JoinNode(SwPaM &rPam, bool bStealAttr = false);

    bool IsBorder(const WW8_BRC* pbrc, bool bChkBtwn = false) const;

    //Set closest writer border equivalent into rBox from pbrc, optionally
    //recording true winword dimensions in pSizeArray. nSetBorders to mark a
    //border which has been previously set to a value and for which becoming
    //empty is valid. Set bCheBtwn to work with paragraphs that have a special
    //between paragraphs border
#if 0
    // #i20672# we can't properly support between lines so best to ignore
    // them for now
    bool SetBorder(SvxBoxItem& rBox, const WW8_BRC* pbrc, short *pSizeArray=0,
        BYTE nSetBorders=0xFF, bool bChkBtwn = false) const;
#endif
    bool SetBorder(SvxBoxItem& rBox, const WW8_BRC* pbrc, short *pSizeArray=0,
        BYTE nSetBorders=0xFF) const;
    void GetBorderDistance(const WW8_BRC* pbrc, Rectangle& rInnerDist) const;
    sal_uInt16 GetParagraphAutoSpace(bool fDontUseHTMLAutoSpacing);
    bool SetShadow(SvxShadowItem& rShadow, const short *pSizeArray,
        const WW8_BRC *pbrc) const;
    //returns true is a shadow was set
    bool SetFlyBordersShadow(SfxItemSet& rFlySet, const WW8_BRC *pbrc,
        short *SizeArray=0) const;
    void SetPageBorder(SwFrmFmt &rFmt, const wwSection &rSection) const;

    INT32 MatchSdrBoxIntoFlyBoxItem( const Color& rLineColor,
        MSO_LineStyle eLineStyle, MSO_SPT eShapeType, INT32 &rLineWidth,
        SvxBoxItem& rBox );
    void MatchSdrItemsIntoFlySet( SdrObject*    pSdrObj, SfxItemSet &aFlySet,
        MSO_LineStyle eLineStyle, MSO_SPT eShapeType, Rectangle &rInnerDist );
    void AdjustLRWrapForWordMargins(const SvxMSDffImportRec &rRecord,
        SvxLRSpaceItem &rLR);
    void AdjustULWrapForWordMargins(const SvxMSDffImportRec &rRecord,
        SvxULSpaceItem &rUL);
    void MapWrapIntoFlyFmt(SvxMSDffImportRec* pRecord, SwFrmFmt* pFlyFmt);

    void SetAttributesAtGrfNode( SvxMSDffImportRec* pRecord, SwFrmFmt *pFlyFmt,
        WW8_FSPA *pF );

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
    USHORT TabRowSprm(int nLevel) const;

    bool ReadGrafFile(String& rFileName, Graphic*& rpGraphic,
       const WW8_PIC& rPic, SvStream* pSt, ULONG nFilePos, bool* pDelIt);

    void ReplaceObj(const SdrObject &rReplaceTextObj,
        SdrObject &rSubObj);

    SwFlyFrmFmt* MakeGrafNotInCntnt(const WW8PicDesc& rPD,
        const Graphic* pGraph, const String& rFileName,
        const SfxItemSet& rGrfSet);

    SwFrmFmt* MakeGrafInCntnt(const WW8_PIC& rPic, const WW8PicDesc& rPD,
        const Graphic* pGraph, const String& rFileName,
        const SfxItemSet& rGrfSet);

    SwFrmFmt *AddAutoAnchor(SwFrmFmt *pFmt);
    SwFrmFmt* ImportGraf1(WW8_PIC& rPic, SvStream* pSt, ULONG nFilePos);
    SwFrmFmt* ImportGraf(SdrTextObj* pTextObj = 0, SwFrmFmt* pFlyFmt = 0);

    SdrObject* ImportOleBase( Graphic& rGraph, const Graphic* pGrf=0,
        const SfxItemSet* pFlySet=0 );

    SwFrmFmt* ImportOle( const Graphic* = 0, const SfxItemSet* pFlySet = 0,
        const SfxItemSet* pGrfSet = 0);
    SwFlyFrmFmt* InsertOle(SdrOle2Obj &rObject, const SfxItemSet &rFlySet,
        const SfxItemSet &rGrfSet);

    bool ImportFormulaControl(WW8FormulaControl &rBox,WW8_CP nStart,
        SwWw8ControlType nWhich);

    void ImportDop();

    //This converts MS Asian Typography information into OOo's
    void ImportDopTypography(const WW8DopTypography &rTypo);

    ULONG LoadThroughDecryption(SwPaM& rPaM ,WW8Glossary *pGloss);
    ULONG SetSubStreams(SvStorageStreamRef &rTableStream, SvStorageStreamRef &rDataStream);
    ULONG CoreLoad(WW8Glossary *pGloss, const SwPosition &rPos);

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

    void SetAnlvStrings(SwNumFmt &rNum, WW8_ANLV &rAV, const BYTE* pTxt,
        bool bOutline);
    void SetAnld(SwNumRule* pNumR, WW8_ANLD* pAD, BYTE nSwLevel, bool bOutLine);
    void SetNumOlst( SwNumRule* pNumR, WW8_OLST* pO, BYTE nSwLevel );
    SwNumRule* GetStyRule();

    void StartAnl(const BYTE* pSprm13);
    void NextAnlLine(const BYTE* pSprm13);
    void StopAllAnl(bool bGoBack = true);
    void StopAnlToRestart(BYTE nType, bool bGoBack = true);

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
    void InsertTxbxStyAttrs( SfxItemSet& rS, USHORT nColl );
    void InsertTxbxAttrs(long nStartCp, long nEndCp, bool bONLYnPicLocFc);

    bool GetTxbxTextSttEndCp(long& rStartCp, long& rEndCp, USHORT nTxBxS,
        USHORT nSequence);
    bool GetTxbxText(String& rString, long StartCp, long nEndCp);
    SwFrmFmt* InsertTxbxText(SdrTextObj* pTextObj, Size* pObjSiz,
        USHORT nTxBxS, USHORT nSequence, long nPosCp, SwFrmFmt* pFlyFmt,
        bool bMakeSdrGrafObj, bool& rbEraseTextObj,
        bool* pbTestTxbxContainsText = 0, long* pnStartCp = 0,
        long* pnEndCp = 0, bool* pbContainsGraphics = 0,
        SvxMSDffImportRec* pRecord = 0);
    bool TxbxChainContainsRealText( USHORT nTxBxS,
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
    bool MiserableRTLGraphicsHack(long &rLeft, long nWidth,
        SwHoriOrient eHoriOri, SwRelationOrient eHoriRel);
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
    void ConvertFFileName( String& rName, const String& rRaw );
    long Read_F_Tag( WW8FieldDesc* pF );
    void InsertTagField( const USHORT nId, const String& rTagText );
    long ImportExtSprm(WW8PLCFManResult* pRes);
    void EndExtSprm(USHORT nSprmId);
    void ReadDocInfo();

// Ver8-Listen

    void RegisterNumFmtOnTxtNode(USHORT nActLFO, BYTE nActLevel,
        bool bSetAttr = true);
    void RegisterNumFmtOnStyle(USHORT nStyle);
    void SetStylesList(sal_uInt16 nStyle, sal_uInt16 nActLFO,
        sal_uInt8 nActLevel);
    void RegisterNumFmt(USHORT nActLFO, BYTE nActLevel);

// spaeter zu ersetzen durch Aufruf in entsprechend erweiterten SvxMSDffManager

    const String* GetAnnotationAuthor(sal_uInt16 nIdx);

    // Schnittstellen fuer die Toggle-Attribute
    void SetToggleAttr(BYTE nAttrId, bool bOn);
    void SetToggleBiDiAttr(BYTE nAttrId, bool bOn);
    void _ChkToggleAttr( USHORT nOldStyle81Mask, USHORT nNewStyle81Mask );

    void ChkToggleAttr( USHORT nOldStyle81Mask, USHORT nNewStyle81Mask )
    {
        if( nOldStyle81Mask != nNewStyle81Mask &&
            pCtrlStck->GetToggleAttrFlags() )
            _ChkToggleAttr( nOldStyle81Mask, nNewStyle81Mask );
    }

    void _ChkToggleBiDiAttr( USHORT nOldStyle81Mask, USHORT nNewStyle81Mask );

    void ChkToggleBiDiAttr( USHORT nOldStyle81Mask, USHORT nNewStyle81Mask )
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

    //No copying
    SwWW8ImplReader(const SwWW8ImplReader &);
    SwWW8ImplReader& operator=(const SwWW8ImplReader&);
public:     // eigentlich private, geht aber leider nur public
    void ConvertUFName( String& rName );


    USHORT GetToggleAttrFlags() const;
    USHORT GetToggleBiDiAttrFlags() const;
    void SetToggleAttrFlags(USHORT nFlags);
    void SetToggleBiDiAttrFlags(USHORT nFlags);


    long Read_Ftn(WW8PLCFManResult* pRes);
    sal_uInt16 End_Ftn();
    long Read_Field(WW8PLCFManResult* pRes);
    sal_uInt16 End_Field();
    long Read_Book(WW8PLCFManResult*);
    long Read_And(WW8PLCFManResult* pRes);

    // Attribute

    void Read_Special(USHORT, const BYTE*, short nLen);
    void Read_Obj(USHORT, const BYTE*, short nLen);
    void Read_PicLoc(USHORT, const BYTE* pData, short nLen );
    void Read_BoldUsw(USHORT nId, const BYTE*, short nLen);
    void Read_BoldBiDiUsw(USHORT nId, const BYTE*, short nLen);
    void Read_SubSuper(         USHORT, const BYTE*, short nLen );
    bool ConvertSubToGraphicPlacement();
    SwFrmFmt *ContainsSingleInlineGraphic(const SwPaM &rRegion);
    void Read_SubSuperProp(     USHORT, const BYTE*, short nLen );
    void Read_Underline(        USHORT, const BYTE*, short nLen );
    void Read_TxtColor(         USHORT, const BYTE*, short nLen );
    void Read_FontCode(         USHORT, const BYTE*, short nLen );
    void Read_FontSize(         USHORT, const BYTE*, short nLen );
    void Read_CharSet(USHORT , const BYTE* pData, short nLen);
    void Read_Language(         USHORT, const BYTE*, short nLen );
    void Read_CColl(            USHORT, const BYTE*, short nLen );
    void Read_Kern(             USHORT, const BYTE* pData, short nLen );
    void Read_FontKern(         USHORT, const BYTE* pData, short nLen );
    void Read_Emphasis(         USHORT, const BYTE* pData, short nLen );
    void Read_ScaleWidth(       USHORT, const BYTE* pData, short nLen );
    void Read_Relief(           USHORT, const BYTE* pData, short nLen);
    void Read_TxtAnim(      USHORT, const BYTE* pData, short nLen);

    void Read_NoLineNumb(       USHORT nId, const BYTE* pData, short nLen );

    void Read_LR(               USHORT nId, const BYTE*, short nLen );
    void AdjustStyleTabStops(long nLeft, SwWW8StyInf *pSty);
    void Read_UL(               USHORT nId, const BYTE*, short nLen );
    void Read_ParaAutoBefore(USHORT , const BYTE *pData, short nLen);
    void Read_ParaAutoAfter(USHORT , const BYTE *pData, short nLen);
    void Read_DontAddEqual(USHORT , const BYTE *pData, short nLen);
    void Read_LineSpace(        USHORT, const BYTE*, short nLen );
    void Read_Justify(USHORT, const BYTE*, short nLen);
    bool IsRightToLeft();
    void Read_RTLJustify(USHORT, const BYTE*, short nLen);
    void Read_Hyphenation(      USHORT, const BYTE* pData, short nLen );
    void Read_WidowControl(     USHORT, const BYTE* pData, short nLen );
    void Read_AlignFont(        USHORT, const BYTE* pData, short nLen );
    void Read_UsePgsuSettings(  USHORT, const BYTE* pData, short nLen );
    void Read_KeepLines(        USHORT, const BYTE* pData, short nLen );
    void Read_KeepParas(        USHORT, const BYTE* pData, short nLen );
    void Read_BreakBefore(      USHORT, const BYTE* pData, short nLen );
    void Read_Apo(USHORT nId, const BYTE* pData, short nLen);
    void Read_ApoPPC(USHORT, const BYTE* pData, short);

    void Read_BoolItem(         USHORT nId, const BYTE*, short nLen );

    void Read_Border(           USHORT nId, const BYTE* pData, short nLen );
    void Read_Tab(              USHORT nId, const BYTE* pData, short nLen );
    void Read_Symbol(USHORT, const BYTE* pData, short nLen);
    void Read_FldVanish(        USHORT nId, const BYTE* pData, short nLen );

    // Revision Marks ( == Redlining )

    // insert or delete content (change char attributes resp.)
    void Read_CRevisionMark(SwRedlineType eType, const BYTE* pData, short nLen);
    // insert new content
    void Read_CFRMark(USHORT , const BYTE* pData, short nLen);
    // delete old content
    void Read_CFRMarkDel(USHORT , const BYTE* pData, short nLen);
    // change properties of content (e.g. char formating)
    void Read_CPropRMark(USHORT , const BYTE* pData, short nLen); // complex!


    void Read_TabRowEnd(        USHORT, const BYTE* pData, short nLen );
    static bool ParseTabPos(WW8_TablePos *aTabPos, WW8PLCFx_Cp_FKP* pPap);
    void Read_Shade(            USHORT, const BYTE* pData, short nLen );
    void Read_ANLevelNo(        USHORT, const BYTE* pData, short nLen );
    void Read_ANLevelDesc(      USHORT, const BYTE* pData, short nLen );

    // Gliederungsebene Ver8
    void Read_POutLvl(USHORT, const BYTE* pData, short nLen);

    void Read_OLST(             USHORT, const BYTE* pData, short nLen );

    void Read_CharShadow(       USHORT, const BYTE* pData, short nLen );
    void Read_CharHighlight(    USHORT, const BYTE* pData, short nLen );
                                        // Ver8-Listen

    void Read_ListLevel(        USHORT nId, const sal_uInt8* pData, short nLen);
    void Read_LFOPosition(      USHORT nId, const sal_uInt8* pData, short nLen);
    bool SetTxtFmtCollAndListLevel(const SwPaM& rRg, SwWW8StyInf& rStyleInfo);

    void Read_StyleCode(USHORT, const BYTE* pData, short nLen);
    void Read_Majority(USHORT, const BYTE* , short );
    void Read_DoubleLine_Rotate( USHORT, const BYTE* pDATA, short nLen);

    void Read_TxtForeColor(USHORT, const BYTE* pData, short nLen);
    void Read_TxtBackColor(USHORT, const BYTE* pData, short nLen);
    void Read_ParaBackColor(USHORT, const BYTE* pData, short nLen);
    void Read_ParaBiDi(USHORT, const BYTE* pData, short nLen);
    static sal_uInt32 ExtractColour(const BYTE* &rpData, bool bVer67);

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
    short GetTimeDatePara(String& rStr, ULONG& rFormat, USHORT &rLang,
        int nWhichDefault, bool bHijri = false);
    bool ForceFieldLanguage(SwField &rFld, USHORT nLang);
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

    eF_ResT Read_F_Macro( WW8FieldDesc*, String& rStr );
    eF_ResT Read_F_DBField( WW8FieldDesc*, String& rStr );
    eF_ResT Read_F_DBNext( WW8FieldDesc*, String& );
    eF_ResT Read_F_DBNum( WW8FieldDesc*, String& );
    eF_ResT Read_F_Equation( WW8FieldDesc*, String& );
    void Read_SubF_Ruby( _ReadFieldParams& rReadParam);
    void Read_SubF_Combined( _ReadFieldParams& rReadParam);
    eF_ResT Read_F_IncludePicture( WW8FieldDesc*, String& rStr );
    eF_ResT Read_F_IncludeText(    WW8FieldDesc*, String& rStr );
    eF_ResT Read_F_Seq( WW8FieldDesc*, String& rStr );

    eF_ResT Read_F_OCX(WW8FieldDesc*, String&);
    eF_ResT Read_F_Hyperlink(WW8FieldDesc*, String& rStr);
        eF_ResT Read_F_Shape(WW8FieldDesc* pF, String& rStr);

    void DeleteFormImpl();

    short ImportSprm( const BYTE* pPos, USHORT nId = 0 );

    bool SearchRowEnd(WW8PLCFx_Cp_FKP* pPap,WW8_CP &rStartCp, int nLevel) const;

    const WW8Fib& GetFib() const    { return *pWwFib; }
    SwDoc& GetDoc() const           { return rDoc; }
    USHORT GetNAktColl()  const     { return nAktColl; }
    void SetNAktColl( USHORT nColl ) { nAktColl = nColl;    }
    void SetAktItemSet( SfxItemSet* pItemSet ) { pAktItemSet = pItemSet; }
    const USHORT StyleUsingLFO( USHORT nLFOIndex ) const ;
    const SwFmt* GetStyleWithOrgWWName( String& rName ) const ;

    static bool GetPictGrafFromStream(Graphic& rGraphic, SvStream& rSrc);
    static void PicRead( SvStream *pDataStream, WW8_PIC *pPic, bool bVer67);
    static bool ImportOleWMF( SvStorageRef xSrc1, GDIMetaFile &rWMF,
        long &rX, long &rY);
    static ColorData GetCol(BYTE nIco);

    SwWW8ImplReader( BYTE nVersionPara, SvStorage* pStorage, SvStream* pSt,
        SwDoc& rD, bool bNewDoc );

    // Laden eines kompletten DocFiles
    ULONG LoadDoc( SwPaM&,WW8Glossary *pGloss=0);
    CharSet GetCurrentCharSet();
};

bool CanUseRemoteLink(const String &rGrfName);
void UseListIndent(SwWW8StyInf &rStyle, const SwNumFmt &rFmt);
void SetStyleIndent(SwWW8StyInf &rStyleInfo, const SwNumFmt &rFmt);
void SyncIndentWithList(SvxLRSpaceItem &rLR, const SwNumFmt &rFmt);
long GetListFirstLineIndent(const SwNumFmt &rFmt);
String BookmarkToWriter(const String &rBookmark);
bool RTLGraphicsHack(long &rLeft, long nWidth,
    SwHoriOrient eHoriOri, SwRelationOrient eHoriRel, SwTwips nPageLeft,
    SwTwips nPageRight, SwTwips nPageSize);
void MatchEscherMirrorIntoFlySet(const SvxMSDffImportRec &rRecord,
    SfxItemSet &rFlySet);
bool RTLDrawingsHack(long &rLeft, long nWidth,
    SwHoriOrient eHoriOri, SwRelationOrient eHoriRel, SwTwips nPageLeft,
    SwTwips nPageRight, SwTwips nPageSize);
#endif

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
