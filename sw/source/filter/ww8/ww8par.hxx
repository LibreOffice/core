/*************************************************************************
 *
 *  $RCSfile: ww8par.hxx,v $
 *
 *  $Revision: 1.72 $
 *
 *  last change: $Author: cmc $ $Date: 2002-06-10 10:33:56 $
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

#ifndef _FLTSHELL_HXX
#include <fltshell.hxx>         // fuer den Attribut Stack
#endif

#ifndef __SGI_STL_VECTOR
#include <vector>
#endif
#ifndef __SGI_STLSTACK
#include <stack>
#endif
#ifndef __SGI_STL_MAP
#include <map>
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

class SvStringsDtor;
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
class SvShorts;
class SvUShorts;
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

class SdrAttrObj;
struct WW8ULSpaceData;
class _ReadFieldParams;
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
#define WW8FL_NO_APO         8
#define WW8FL_NO_HDFT     0x10
#define WW8FL_NO_FTN      0x20
#define WW8FL_NO_FLD      0x40  // keine Felder
#define WW8FL_NO_GRAF     0x80
#define WW8FL_NO_DEFTABS 0x100  // keine impliziten Tabs
#define WW8FL_NO_LRUL    0x200


#define WW8FL_NO_DEFSTYLES        0x400 // falls gestetzt, werden fuer Writer-Def-Styles
                                        // neue Styles mit den WW8-Def-Style-Eigenschaften
                                        // erzeugt, statt die Writer-Standards zu ueberschreiben

#define WW8FL_NO_TABLE            0x800
#define WW8FL_NO_OUTLINE         0x1000
#define WW8FL_NO_NUMRULE         0x2000
#define WW8FL_NO_IMPLPASP        0x4000  // no implicit para space
#define WW8FL_NO_GRAFLAYER       0x8000
#define WW8FL_NO_COMPLEX        0x10000
#define WW8FL_NO_OLE            0x20000
#define WW8FL_OLE_TO_GRAF       0x40000
#define WW8FL_NO_VCCONTROLS     0x80000
#define WW8FL_NO_STD_STY_DYA   0x100000
#define WW8FL_NO_COLS          0x200000
#define WW8FL_NO_TOX           0x400000
#define WW8FL_NO_IMPORT      0x20000000
#define WW8FL_WRITE_TO_FILE  0x40000000
#define WW8FL_NOWARN_COMPLEX 0x80000000

// Zusatz-Filter-Flags, gueltig ab Winword 8
#define WW8FL_NO_FLY_FOR_TXBX         1
#define EQUAL_TAB_BORDERDISTS         2


//-----------------------------------------
//            Listen-Manager (ab Ver8)
//-----------------------------------------

struct WW8LFOInfo;
typedef WW8LFOInfo* WW8LFOInfo_Ptr;
// Zeichenattribute aus GrpprlChpx
typedef SfxItemSet* WW8aISet[nWW8MaxListLevel ];
// Zeichen Style Pointer
typedef SwCharFmt* WW8aCFmt[ nWW8MaxListLevel ];
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
    UINT32 nWWid;
    SvInPlaceObject *pWriterRef;

    WW8OleMap(UINT32 nWWid_ , SvInPlaceObject *pWriterRef_ = 0):
        nWWid( nWWid_), pWriterRef (pWriterRef_) {}

    BOOL operator==( const WW8OleMap & rEntry ) const
    {
        return (nWWid == rEntry.nWWid);
    }
    BOOL operator<( const WW8OleMap & rEntry ) const
    {
        return (nWWid < rEntry.nWWid);
    }
};

class SwWW8ImplReader;
struct WW8LSTInfo;
class WW8ListManager
{
    SwWW8ImplReader& rReader;
    SwDoc&           rDoc;
    const WW8Fib&    rFib;
    SvStream&        rSt;
    ::std::vector<WW8LSTInfo* > maLSTInfos;
    WW8LFOInfos* pLFOInfos;// D. aus PLF LFO, sortiert genau wie im WW8 Stream
    USHORT       nUniqueList; // current number for creating unique list names
    BYTE* GrpprlHasSprm(USHORT nId, BYTE& rSprms, BYTE nLen);
    WW8LSTInfo* GetLSTByListId(    ULONG  nIdLst     ) const;
    BOOL ReadLVL(SwNumFmt& rNumFmt, SfxItemSet*& rpItemSet, USHORT nLevelStyle,
        BOOL bSetStartNo );
    void AdjustLVL(BYTE nLevel, SwNumRule& rNumRule, WW8aISet& rListItemSet,
        WW8aCFmt& aCharFmt, BOOL& bNewCharFmtCreated,
        String aPrefix = aEmptyStr );
    BOOL LFOequaltoLST(WW8LFOInfo& rLFOInfo);

    //No copying
    WW8ListManager(const WW8ListManager&);
    WW8ListManager& operator=(const WW8ListManager&);
public:
    WW8ListManager(SvStream& rSt_, SwWW8ImplReader& rReader_);
    ~WW8ListManager();
    SwNumRule* GetNumRuleForActivation(USHORT nLFOPosition, BYTE nLevel) const;
    BOOL IsSimpleList(USHORT nLFOPosition) const;
    SwNumRule* CreateNextRule(BOOL bSimple);
};



//-----------------------------------------
//            Stack
//-----------------------------------------
class SwWW8FltControlStack : public SwFltControlStack
{
private:
    SwWW8ImplReader& rReader;
    USHORT nToggleAttrFlags;
    //No copying
    SwWW8FltControlStack(const SwWW8FltControlStack&);
    SwWW8FltControlStack& operator=(const SwWW8FltControlStack&);
protected:
    virtual void SetAttrInDoc(const SwPosition& rTmpPos,
        SwFltStackEntry* pEntry);
public:
    SwWW8FltControlStack(SwDoc* pDo, ULONG nFieldFl, SwWW8ImplReader& rReader_ )
        : SwFltControlStack( pDo, nFieldFl ), rReader( rReader_ ),
        nToggleAttrFlags( 0 )
    {}

    void NewAttr(const SwPosition& rPos, const SfxPoolItem& rAttr);
    void SetAttr(const SwPosition& rPos, USHORT nAttrId=0, BOOL bTstEnde=TRUE,
        long nHand=LONG_MAX);
    void SetToggleAttr( BYTE nId, BOOL bOn )
    {
        if( bOn )
            nToggleAttrFlags |= (1 << nId);
        else
            nToggleAttrFlags &= ~(1 << nId);
    }
    USHORT GetToggleAttrFlags() const { return nToggleAttrFlags; }

    const SfxPoolItem* GetFmtAttr(const SwPosition& rPos, USHORT nWhich);
    const SfxPoolItem* GetStackAttr(const SwPosition& rPos, USHORT nWhich);
    void Delete(const SwPaM &rPam);
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
};

//For fields whose handling cannot be fully resolved until we hit the end of
//the document.
class SwWW8FltRefStack : public SwFltEndStack
{
public:
    SwWW8FltRefStack(SwDoc* pDo, ULONG nFieldFl)
        : SwFltEndStack( pDo, nFieldFl )
    {}
    BOOL IsFtnEdnBkmField(const SwFmtFld& rFmtFld, USHORT& rBkmNo);

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
    ::std::map<String, String, ltstr> aFieldVarNames;
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
    BOOL operator==( const WW8AuthorInfo& rEntry ) const
    {
        return (nWWAuthorId == rEntry.nWWAuthorId);
    }
    BOOL operator<( const WW8AuthorInfo& rEntry ) const
    {
        return (nWWAuthorId < rEntry.nWWAuthorId);
    }
};



//-----------------------------------------
//    Mini-Merker fuer einige Flags
//-----------------------------------------
class WW8ReaderSave
{
    SwPosition aTmpPos;
    SwWW8FltControlStack* pOldStck;
    SwWW8FltAnchorStack* pOldAnchorStck;
    WW8PLCFxSaveAll aPLCFxSave;
    WW8PLCFMan* pOldPlcxMan;

    WW8FlyPara* pWFlyPara;
    WW8SwFlyPara* pSFlyPara;
    WW8TabDesc* pTableDesc;
    int nTable;
    USHORT nAktColl;
    USHORT nNoAttrScan;
    sal_Unicode cSymbol;
    BOOL bIgnoreText    : 1;
    BOOL bDontCreateSep : 1;
    BOOL bSymbol        : 1;
    BOOL bHdFtFtnEdn    : 1;
    BOOL bApo           : 1;
    BOOL bTxbxFlySection: 1;
    BOOL bTableInApo    : 1;
    BOOL bAnl           : 1;
    BOOL bInHyperlink : 1;
    BOOL bPgSecBreak : 1;
    BOOL bVerticalEnviron : 1;
    BOOL bWasParaEnd : 1;
public:
    WW8ReaderSave( SwWW8ImplReader* pRdr, WW8_CP nStart=-1 );
    void Restore( SwWW8ImplReader* pRdr );
    const SwPosition &GetStartPos() const { return aTmpPos; }
};

enum eF_ResT{ FLD_OK, FLD_TEXT, FLD_TAGIGN, FLD_TAGTXT, FLD_READ_FSPA };

struct SwWW8Shade{
    Color aColor;
    SwWW8Shade( BOOL bVer67, const WW8_SHD& rSHD );
};


//-----------------------------------------
//    Formulas
//-----------------------------------------
enum SwWw8ControlType
{
    WW8_CT_EDIT,
    WW8_CT_CHECKBOX,
    WW8_CT_COMBOBOX
};


class WW8FormulaControl : public OCX_Control
{
public:
    WW8FormulaControl(const String& sN,SwWW8ImplReader &rR)
        : OCX_Control(sN), rRdr(rR) {}
    void WW8FormulaControl::SetOthersFromDoc(com::sun::star::awt::Size &rSz,
        com::sun::star::uno::Reference <
        com::sun::star::beans::XPropertySet> &rPropSet);

    UINT8   fToolTip:1;
    UINT8   fNoMark:1;
    UINT8   fUseSize:1;
    UINT8   fNumbersOnly:1;
    UINT8   fDateOnly:1;
    UINT8   fUnused:3;
    UINT16  nSize;

    UINT16  hpsCheckBox;
    UINT16 nChecked;

    String sTitle;
    String sDefault;
    String sFormatting;
    String sHelp;
    String sToolTip;

    void FormulaRead(SwWw8ControlType nWhich,SvStream *pD);
private:
    //No copying
    WW8FormulaControl(const WW8FormulaControl&);
    WW8FormulaControl& operator=(const WW8FormulaControl&);

    SwWW8ImplReader &rRdr;
};

class WW8FormulaCheckBox : public WW8FormulaControl
{
private:
    //No copying
    WW8FormulaCheckBox(const WW8FormulaCheckBox&);
    WW8FormulaCheckBox& operator=(const WW8FormulaCheckBox&);
public:
    WW8FormulaCheckBox(SwWW8ImplReader &rR);
    BOOL Import(const com::sun::star::uno::Reference <
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
    BOOL Import(const com::sun::star::uno::Reference <
        com::sun::star::lang::XMultiServiceFactory> &rServiceFactory,
        com::sun::star::uno::Reference <
        com::sun::star::form::XFormComponent> &rFComp,
        com::sun::star::awt::Size &rSz);
};

class SwMSConvertControls: public SvxMSConvertOCXControls
{
public:
    SwMSConvertControls( SfxObjectShell *pDSh,SwPaM *pP ) :
        SvxMSConvertOCXControls( pDSh,pP ) {}
    BOOL InsertFormula( WW8FormulaControl &rFormula);
    BOOL InsertControl(const com::sun::star::uno::Reference<
        com::sun::star::form::XFormComponent >& rFComp,
        const ::com::sun::star::awt::Size& rSize,
        com::sun::star::uno::Reference <
        com::sun::star::drawing::XShape > *pShape,BOOL bFloatingCtrl);
    BOOL SwMSConvertControls::ExportControl(Writer &rWrt,const SdrObject *pObj);
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

//-----------------------------------------
//            Storage-Reader
//-----------------------------------------
class SwWW8ImplReader
{
private:
friend class WW8RStyle;
friend class WW8TabDesc;
friend class WW8ReaderSave;
friend struct WW8FlyPara;
friend struct WW8SwFlyPara;
friend class WW8FlySet;
friend class SwMSDffManager;
friend class SwWW8FltControlStack;
friend class WW8FormulaControl;

    SvStorage* pStg;                // Input-Storage
    SvStream* pStrm;                // Input-(Storage)Stream
    SvStream* pTableStream;         // Input-(Storage)Stream
    SvStream* pDataStream;          // Input-(Storage)Stream

// allgemeines
    SwDoc& rDoc;
    SwPaM* pPaM;

    SwWW8FltControlStack* pCtrlStck;    // Stack fuer die Attribute

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
    inserted.
    */
    SwWW8FltAnchorStack* pAnchorStck;

    /*
    Creates unique names to give to graphics
    */
    wwFrameNamer aGrfNameGenerator;

    SwMSConvertControls *pFormImpl; // Control-Implementierung

    SwFlyFrmFmt* pFlyFmtOfJustInsertedGraphic;
    SwFrmFmt* pFmtOfJustInsertedGraphicOrOLE;

    /*
    Keep track of generated Ruby character formats we we can minimize the
    number of character formats created
    */
    ::std::vector<const SwCharFmt*> aRubyCharFmts;

    WW8Fib* pWwFib;
    WW8Fonts* pFonts;
    WW8Dop* pWDop;
    WW8ListManager* pLstManager;
    WW8ScannerBase* pSBase;
    WW8PLCFMan* pPlcxMan;

    WW8RStyle* pStyles;     // Pointer auf die Style-Einleseklasse
    SwFmt* pAktColl;        // gerade zu erzeugende Collection
                            // ( ist ausserhalb einer Style-Def immer 0 )
    SfxItemSet* pAktItemSet;// gerade einzulesende Zeichenattribute
                            // (ausserhalb des WW8ListManager Ctor's immer 0)
    SwWW8StyInf* pCollA;    // UEbersetzungs-Array der Styles
    const SwTxtFmtColl* pDfltTxtFmtColl;    // Default
    SwFmt* pStandardFmtColl;// "Standard"

    SwPageDesc* pPageDesc;      // fuer uebernommene KF-Zeilen
    WW8PLCF_HdFt* pHdFt;        // Pointer auf Header / Footer - Scannerklasse

    WW8FlyPara* pWFlyPara;      // WW-Parameter
    WW8SwFlyPara* pSFlyPara;    // daraus erzeugte Sw-Parameter

    WW8TabDesc* pTableDesc;     // Beschreibung der Tabelleneigenschaften
    //Keep track of tables within tables
    ::std::stack<WW8TabDesc*> aTableStack;

    SwNumRule* pNumRule;        // fuer Nummerierung / Aufzaehlungen im Text
    WW8_OLST* pNumOlst;         // Gliederung im Text
    SwNodeIndex* pAfterSection;
    SwSection*   pNewSection;   // last Section that was inserted into the doc

    SwNode* pNode_FLY_AT_CNTNT; // set: WW8SwFlyPara()   read: CreateSwTable()

    SdrModel* pDrawModel;
    SdrPage* pDrawPg;
    EditEngine* pDrawEditEngine;
    wwZOrderer *pWWZOrder;

    SwFieldType* pNumFldType;   // fuer Nummernkreis

    SwMSDffManager* pMSDffManager;

    SvStringsDtor* pAtnNames;

    WW8AuthorInfos* pAuthorInfos;
    WW8OleMaps* pOleMap;

    /*
    Tabstops on a paragraph need to be adjusted when the lrspace has been
    changed. To safely collect all lrchanges we process them at the closing of
    the lr attribute, and store the start pos for the newly modified tabstop
    here.
    */
    SwNodeIndex *pTabNode;
    xub_StrLen nTabCntnt;

    SwNodeIndex* pLastPgDeskIdx;// for inserting a section when Ft-/End-Note
                                // with flag 'on end of section' set

                                // Ini-Flags:
    ULONG nIniFlags;            // Flags aus der writer.ini
    ULONG nIniFlags1;           // dito ( zusaetzliche Flags )
    ULONG nIniFtSiz;            // dito fuer Default-Size Footer
    ULONG nFieldFlags;          // dito fuer Feldern
    ULONG nFieldTagAlways[3];   // dito fuers Taggen von Feldern
    ULONG nFieldTagBad[3];      // dito fuers Taggen von nicht importierbaren F.

    ULONG nLastFlyNode;         // Node number of last imported Fly

    ULONG nHdTextHeight;        // Hoehe des eingelesenen Headers
    ULONG nFtTextHeight;        // Hoehe des eingelesenen Footers

    ULONG nDrawObjOfs;
    WW8_CP nDrawCpO;            // Anfang der Txbx-SubDocs

    ULONG nPicLocFc;            // Picture Location in File (FC)
    ULONG nObjLocFc;            // Object Location in File (FC)

    INT32 nIniFlyDx;            // X-Verschiebung von Flys
    INT32 nIniFlyDy;            // Y-Verschiebung von Flys

    rtl_TextEncoding eTextCharSet;    // Default charset for Text
    rtl_TextEncoding eStructCharSet;  // rtl_TextEncoding for structures
    rtl_TextEncoding eHardCharSet;    // Hard rtl_TextEncoding-Attribute
    // Source rtl_TextEncoding character encodings stack for word text
    SvUShorts *pFontSrcCharSets;
    USHORT nProgress;           // %-Angabe fuer Progressbar
    USHORT nColls;              // Groesse des Arrays
    USHORT nAktColl;            // gemaess WW-Zaehlung
    USHORT nPageDescOffset;     // fuer UpdatePageDescs
    USHORT nDrawTxbx;           // Nummer der Textbox ( noetig ?? )
    USHORT nFldNum;             // laufende Nummer dafuer
    USHORT nLFOPosition;

    short nActSectionNo;        // aktuelle Abschnitts-Nummer (in CreateSep() gesetzt)
    short nCharFmt;             // gemaess WW-Zaehlung, <0 fuer keine
    short nAlign2;
    short nPgWidth;
    short nPgTop;               // fuer APOs
    short nPgLeft;              // Seitenraender, z.B. fuer APOs, Tabellen
    short nPgRight;             // dito

    short nLeftParaMgn;         // Absatz L-Space
    short nTxtFirstLineOfst;    // Absatz 1st line ofset

    short nDrawXOfs, nDrawYOfs;
    short nDrawXOfs2, nDrawYOfs2;

    sal_Unicode cSymbol;        // aktuell einzulesendes Symbolzeichen


    BYTE nWantedVersion;        // urspruenglich vom Writer
                                // angeforderte WW-Doc-Version


    BYTE nSwNumLevel;           // LevelNummer fuer Outline / Nummerierung
    BYTE nWwNumType;            // Gliederung / Nummerg / Aufzaehlg
    sal_Int8 nDrawHeaven, nDrawHell;
    BYTE nListLevel;

    BYTE nNfcPgn;               // Formatting of PageNum
    BYTE nPgChpDelim;           // ChapterDelim from PageNum
    BYTE nPgChpLevel;           // ChapterLevel of Heading from PageNum

    BYTE nCorrIhdt;             // used in CreateSep()
    BOOL bSectionHasATitlePage; // used in CreateSep()

    bool mbNewDoc;          // Neues Dokument ?
    BOOL bReadNoTbl;        // Keine Tabellen
    BOOL bPgSecBreak;       // Page- oder Sectionbreak ist noch einzufuegen
    BOOL bSpec;             // Special-Char im Text folgt
    BOOL bObj;              // Obj im Text
    BOOL bApo;              // FlyFrame, der wegen Winword APO eingefuegt wurde
    BOOL bTxbxFlySection;   // FlyFrame, der als Ersatz fuer Winword Textbox eingefuegt wurde
    BOOL bHasBorder;        // fuer Buendelung der Border
    BOOL bSymbol;           // z.B. Symbol statt Times
    BOOL bIgnoreText;       // z.B. fuer FieldVanish
    BOOL bDontCreateSep;    // e.g. when skipping result of multi-column index-field
     int  nTable;           // wird gerade eine Tabelle eingelesen
    BOOL bTableInApo;       // Table is contained in Apo
    BOOL bWasTabRowEnd;     // Tabelle : Row End Mark
    BOOL bShdTxtCol;        // Textfarbe indirekt gesetzt ( Hintergrund sw )
    BOOL bCharShdTxtCol;    // Textfarbe indirekt gesetzt ( Zeichenhintergrund sw )
    BOOL bAnl;              // Nummerierung in Bearbeitung
                                // Anl heisst Autonumber level

    BOOL bHdFtFtnEdn;       // Spezialtext: Kopf- Fuss- usw.
    BOOL bFtnEdn;           // Fussnote oder Endnote
    BOOL bIsHeader;         // Text aus Header wird gelesen ( Zeilenhoehe )
    BOOL bIsFooter;         // Text aus Footer wird gelesen ( Zeilenhoehe )

    BOOL bIsUnicode;            // aktuelles Text-Stueck ist als 2-Bytiger-Unicode kodiert
                                // bitte NICHT als Bitfeld kodieren!

    BOOL bCpxStyle;         // Style im Complex Part
    BOOL bStyNormal;        // Style mit Id 0 wird gelesen
    BOOL bWWBugNormal;      // WW-Version nit Bug Dya in Style Normal
    BOOL bNoAttrImport;     // Attribute ignorieren zum Ignorieren v. Styles
    BOOL bInHyperlink;      // Sonderfall zum einlesen eines 0x01
                                   // siehe: SwWW8ImplReader::Read_F_Hyperlink()
    BOOL bVerticalEnviron;
    BOOL bWasParaEnd;

    // praktische Hilfsvariablen:
    BOOL bVer67;            // ( (6 == nVersion) || (7 == nVersion) );
    BOOL bVer6;             //   (6 == nVersion);
    BOOL bVer7;             //   (7 == nVersion);
    BOOL bVer8;             //   (8 == nVersion);

    BOOL bPgChpLevel;       // ChapterLevel of Heading from PageNum
    BOOL bEmbeddObj;        // EmbeddField gelesen

    BOOL bAktAND_fNumberAcross; // current active Annotated List Deskriptor - ROW flag

    BOOL bNoLnNumYet;       // no Line Numbering has been activated yet (we import
                            //     the very 1st Line Numbering and ignore the rest)
    BOOL bRestartLnNumPerSection;




//---------------------------------------------

    void AppendTxtNode(SwPosition& rPos);
    void GetNoninlineNodeAttribs(const SwTxtNode *pNode,
        ::std::vector<const xub_StrLen*> &rPositions);
    void SetLastPgDeskIdx();

    SwPageDesc* CreatePageDesc( SwPageDesc* pFirstPageDesc,
                                SwPaM** ppPaMWanted = 0 );
    void RemoveCols( SwPageDesc& rPageDesc, SwFmtCol*& rpCol );
    BOOL SetCols( SwFrmFmt* pFmt, const WW8PLCFx_SEPX* pSep, USHORT nNettoWidth,
        BOOL bTestOnly = FALSE );
    void SetPage1( SwPageDesc* pPageDesc, SwFrmFmt &rFmt,
                   const WW8PLCFx_SEPX* pSep, USHORT nLIdx,
                   BOOL bIgnoreCols );
    void SetHdFt(SwPageDesc* pPageDesc0, SwPageDesc* pPageDesc1, BYTE nIPara);
    void GetPageULData( const  WW8PLCFx_SEPX* pSep,
                        USHORT nLIdx,
                        BOOL   bFirst,
                        WW8ULSpaceData& rData );
    void SetPageULSpaceItems( SwFrmFmt &rFmt, WW8ULSpaceData& rData );
    void SetDocumentGrid(SwFrmFmt &rFmt,const WW8PLCFx_SEPX* pSep);

    void SetPageBorder( SwPageDesc* pPageDesc0, SwPageDesc* pPageDesc1,
                        const WW8PLCFx_SEPX* pSep, USHORT nLIdx );
    void SetUseOn(SwPageDesc* pPageDesc0, SwPageDesc* pPageDesc1, BYTE nHdFt);
    void InsertSectionWithWithoutCols( SwPaM& rMyPaM, const SwFmtCol* pCol );
    void CreateSep( const long nTxtPos, BOOL bMustHaveBreak );
    BOOL MustCloseSection(long nTxtPos);

    void CopyPageDescHdFt( const SwPageDesc* pOrgPageDesc,
                           SwPageDesc* pNewPageDesc, BYTE nCode );

    void UpdatePageDescs( USHORT nPageDescOffset );

    void DeleteStk(SwFltControlStack* prStck);
    void DeleteCtrlStk()    { DeleteStk( pCtrlStck  ); pCtrlStck   = 0; }
    void DeleteRefStk()     { DeleteStk( pRefStck ); pRefStck = 0; }
    void DeleteAnchorStk()  { DeleteStk( pAnchorStck ); pAnchorStck = 0; }

    BOOL ReadChar( long nPosCp, long nCpOfs );
    BOOL ReadPlainChars( long& rPos, long nEnd, long nCpOfs );
    BOOL ReadChars( long& rPos, long nNextAttr, long nTextEnd, long nCpOfs );


    void ReadPlainText( long nStartCp, long nTextLen );
    void ProcessAktCollChange(  WW8PLCFManResult& rRes,
                                BOOL* pStartAttr,
                                BOOL bCallProcessSpecial );
    long ReadTextAttr( long& rTxtPos, BOOL& rbStartLine );
    void ReadAttrs( long& rNext, long& rTxtPos, BOOL& rbStartLine );
    void ReadAttrEnds( long& rNext, long& rTxtPos );
    BOOL ReadText( long nStartCp, long nTextLen, short nType );

    void ReadRevMarkAuthorStrTabl( SvStream& rStrm, INT32 nTblPos,
        INT32 nTblSiz, SwDoc& rDoc );

    void Read_HdFtFtnText( const SwNodeIndex* pSttIdx, long nStartCp,
                           long nLen, short nType );
    void Read_HdFt1( BYTE nPara, BYTE nWhichItems, SwPageDesc* pPD );
    void Read_HdFtText( long nStartCp, long nLen, SwPageDesc* pPD,
                             BOOL bUseLeft, BOOL bFooter );

    BYTE* ReadUntilToken( USHORT& rStrLen, USHORT nMaxLen, BYTE nToken );
    void ImportTox( int nFldId, String aStr );

    void EndSprm( USHORT nId );
    void NewAttr( const SfxPoolItem& rAttr );

    BOOL GetFontParams( USHORT, FontFamily&, String&, FontPitch&, rtl_TextEncoding& );
    BOOL SetNewFontAttr( USHORT nFCode, BOOL bSetEnums, USHORT nWhich );
    void ResetCharSetVars();

    const SfxPoolItem* GetFmtAttr( USHORT nWhich );
    BYTE HdFtCorrectPara( BYTE nPara );
    BOOL JoinNode( SwPaM* pPam, BOOL bStealAttr = FALSE );

    BOOL IsBorder( const WW8_BRC* pbrc, BOOL bChkBtwn=FALSE );

    //Set closest writer border equivalent into rBox from pbrc, optionally
    //recording true winword dimensions in pSizeArray. nSetBorders to mark a
    //border which has been previously set to a value and for which becoming
    //empty is valid. Set bCheBtwn to work with paragraphs that have a special
    //between paragraphs border
    BOOL SetBorder( SvxBoxItem& rBox, const WW8_BRC* pbrc, short *pSizeArray=0,
        BYTE nSetBorders=0xFF, BOOL bChkBtwn=FALSE);

    void GetBorderDistance( WW8_BRC* pbrc, Rectangle& rInnerDist );

    BOOL SetShadow( SvxShadowItem& rShadow, const SvxBoxItem& rBox,
        const WW8_BRC pbrc[4] );

    //returns true is a shadow was set
    BOOL SetFlyBordersShadow( SfxItemSet& rFlySet, const WW8_BRC pbrc[4],
        short *SizeArray=0 );

    INT32 MatchSdrBoxIntoFlyBoxItem( const Color& rLineColor,
        MSO_LineStyle eLineStyle, MSO_SPT eShapeType, INT32 &rLineWidth,
        SvxBoxItem& rBox );
    void MatchSdrItemsIntoFlySet( SdrObject*    pSdrObj, SfxItemSet &aFlySet,
        MSO_LineStyle eLineStyle, MSO_SPT eShapeType, Rectangle &rInnerDist );
    void AdjustLRWrapForWordMargins(SvxMSDffImportRec* pRecord,
        SvxLRSpaceItem *pLR);
    void AdjustULWrapForWordMargins(SvxMSDffImportRec* pRecord,
        SvxULSpaceItem *pUL);
    void MatchWrapDistancesIntoFlyFmt( SvxMSDffImportRec* pRecord,
                                       SwFrmFmt*          pFlyFmt );

    void SetAttributesAtGrfNode( SvxMSDffImportRec* pRecord, SwFrmFmt *pFlyFmt,
        WW8_FSPA *pF );

    BOOL StartApo(const BYTE* pSprm29, const WW8FlyPara *pNowStyleApo,
        WW8_TablePos *pTabPos);
    void StopApo();
    BOOL TestSameApo( const BYTE* pSprm29, const WW8FlyPara *pNowStyleApo,
        WW8_TablePos *pTabPos);
    const BYTE* TestApo( BOOL& rbStartApo, BOOL& rbStopApo,
        WW8FlyPara* &pbNowStyleApo, int nInTable, BOOL bTableRowEnd,
        WW8_TablePos *pTabPos);

    BOOL ProcessSpecial( BOOL bAllEnd, BOOL* pbReSync, WW8_CP nStartCp );
    USHORT TabCellSprm(int nLevel) const;
    USHORT TabRowSprm(int nLevel) const;

    ULONG ReadWmfHeader( WmfFileHd* pHd, long nPos );
    BOOL ReadGrafFile( String& rFileName, Graphic*& rpGraphic,
       const WW8_PIC& rPic, SvStream* pSt, ULONG nFilePos, BOOL* pDelIt );

    void ReplaceObjWithGraphicLink(const SdrObject &rReplaceTextObj,
        const String& rFileName);

    SwFlyFrmFmt* MakeGrafNotInCntnt(const WW8PicDesc& rPD,
        const Graphic* pGraph, const String& rFileName,
        const SfxItemSet& rGrfSet);

    SwFrmFmt* MakeGrafInCntnt(const WW8_PIC& rPic, const WW8PicDesc& rPD,
        const Graphic* pGraph, const String& rFileName,
        const SfxItemSet& rGrfSet);

    SwFrmFmt *AddAutoAnchor(SwFrmFmt *pFmt);
    void RemoveAutoAnchor(const SwFrmFmt *pFmt);
    SwFrmFmt* ImportGraf1( WW8_PIC& rPic, SvStream* pSt, ULONG nFilePos );
    SwFrmFmt* ImportGraf(  SdrTextObj* pTextObj = 0, SwFrmFmt* pFlyFmt = 0,
        BOOL bSetToBackground = FALSE );
    BOOL ImportURL(String &sURL,String &sMark,WW8_CP nStart);

    SdrObject* ImportOleBase( Graphic& rGraph, const Graphic* pGrf=0,
        const SfxItemSet* pFlySet=0 );

    SwFrmFmt* ImportOle( const Graphic* = 0, const SfxItemSet* pFlySet = 0 );
    SwFlyFrmFmt* InsertOle(SdrOle2Obj &rObject, const SfxItemSet &rFlySet);

    BOOL ImportFormulaControl(WW8FormulaControl &rBox,WW8_CP nStart,
        SwWw8ControlType nWhich);

    void ImportDop();

    //This converts MS Asian Typography information into OOo's
    void ImportDopTypography(const WW8DopTypography &rTypo);

    ULONG LoadDoc1( SwPaM& rPaM ,WW8Glossary *pGloss);

    BOOL StartTable(WW8_CP nStartCp);
    void TabCellEnd();
    void StopTable();
    short GetTableLeft();
    BOOL IsInvalidOrToBeMergedTabCell() const;

    ColorData GetCol( BYTE nIco );

// Nummerierungen / Aufzaehlungen ( Autonumbered List Data Descriptor )
// Liste:        ANLD ( Autonumbered List Data Descriptor )
//   eine Ebene: ANLV ( Autonumber Level Descriptor )
//
// Chg7-8:
// Listen sind in WW8 eigene Strukturen, die ueber folgende drei Tabellen
// verwaltet werden: rglst, hpllfo und hsttbListNames
// die Strukturen hierfuer sind: LSTF, LVLF, LFO LFOLVL

    void SetAnlvStrings( SwNumFmt* pNum, WW8_ANLV* pAV, const BYTE* pTxt,
                            BOOL bOutline );
    void SetAnld(SwNumRule* pNumR, WW8_ANLD* pAD, BYTE nSwLevel, BOOL bOutLine);
    void SetNumOlst( SwNumRule* pNumR, WW8_OLST* pO, BYTE nSwLevel );
    SwNumRule* GetStyRule();

    void StartAnl( const BYTE* pSprm13 );
    void NextAnlLine( const BYTE* pSprm13, const BYTE* pS12 = 0 );
    void StopAnl( BOOL bGoBack = TRUE );

// GrafikLayer

    BOOL ReadGrafStart( void* pData, short nDataSiz, WW8_DPHEAD* pHd,
        const WW8_DO* pDo, SfxAllItemSet &rSet );
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
    void InsertTxbxAttrs( long nStartCp, long nEndCp, BOOL bONLYnPicLocFc );

    BOOL GetTxbxTextSttEndCp( long& rStartCp, long& rEndCp,
                                USHORT nTxBxS, USHORT nSequence );
    BOOL GetTxbxText( String& rString, long StartCp, long nEndCp );
    SwFrmFmt* InsertTxbxText(SdrTextObj*    pTextObj,
                        Size*       pObjSiz,
                        USHORT      nTxBxS,
                        USHORT      nSequence,
                        long        nPosCp,
                        SwFrmFmt*   pFlyFmt,
                        BOOL        bMakeSdrGrafObj,
                        BOOL&       rbEraseTextObj,
                        BOOL*       pbTestTxbxContainsText = 0,
                        long*       pnStartCp      = 0,
                        long*       pnEndCp        = 0,
                        BOOL*       pbContainsGraphics = 0,
                        SvxMSDffImportRec* pRecord = 0);
    BOOL TxbxChainContainsRealText( USHORT nTxBxS,
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
    void ProcessEscherAlign( SvxMSDffImportRec* pRecord, WW8_FSPA *pFSPA,
        SfxItemSet &rFlySet, BOOL bOrgObjectWasReplace );
    SwFrmFmt* Read_GrafLayer( long nGrafAnchorCp );
    SwFlyFrmFmt* ImportReplaceableDrawables( SdrObject* &rpObject,
        SdrObject* &rpOurNewObject, SvxMSDffImportRec* pRecord, WW8_FSPA *pF,
        SfxItemSet &rFlySet );
    SwFlyFrmFmt *ConvertDrawTextToFly( SdrObject* &rpObject,
        SdrObject* &rpOurNewObject, SvxMSDffImportRec* pRecord,
        RndStdIds eAnchor, WW8_FSPA *pF, SfxItemSet &rFlySet );
    void MungeTextIntoDrawBox(SdrObject* pTrueObject,
        SvxMSDffImportRec *pRecord, long nGrafAnchorCp, SwFrmFmt *pRetFrmFmt);
#if 0
    void EmbeddedFlyFrameSizeLock(SwNodeIndex &rStart,SwFrmFmt *pFrmFmt);
#endif
    void GrafikCtor();
    void GrafikDtor();

// anderes
    String GetFieldResult( WW8FieldDesc* pF );
    void MakeTagString( String& rStr, const String& rOrg );
    void UpdateFields();
    void ConvertFFileName( String& rName, const String& rRaw );
    long Read_F_Tag( WW8FieldDesc* pF );
    void InsertTagField( const USHORT nId, const String& rTagText );
    long ImportExtSprm( WW8PLCFManResult* pRes, BOOL bStart );
    void ReadDocInfo();

// Ver8-Listen

    void RegisterNumFmtOnTxtNode(   USHORT nActLFO,
                                    BYTE   nActLevel,
                                    BOOL   bSetAttr = TRUE );
    void RegisterNumFmtOnStyle(     USHORT nStyle,
                                    USHORT nActLFO   = USHRT_MAX,
                                    BYTE   nActLevel = nWW8MaxListLevel );
    void RegisterNumFmt(USHORT nActLFO, BYTE nActLevel);

    SwNumRule* SyncStyleIndentWithList(SwWW8StyInf &rStyleInfo,
        SwNumRule* pRule, BYTE nLevel);

// spaeter zu ersetzen durch Aufruf in entsprechend erweiterten SvxMSDffManager

    const String* GetAnnotationAuthor( short nId );

    // Schnittstellen fuer die Toggle-Attribute
    void SetToggleAttr( BYTE nAttrId, BOOL bOn );
    void _ChkToggleAttr( USHORT nOldStyle81Mask, USHORT nNewStyle81Mask );

    void ChkToggleAttr( USHORT nOldStyle81Mask, USHORT nNewStyle81Mask )
    {
        if( nOldStyle81Mask != nNewStyle81Mask &&
            pCtrlStck->GetToggleAttrFlags() )
            _ChkToggleAttr( nOldStyle81Mask, nNewStyle81Mask );
    }

    void PopTableDesc();
    void MoveInsideFly(const SwFrmFmt *pFlyFmt);
    void MoveOutsideFly(const SwFrmFmt *pFlyFmt, const SwPosition &rPos,
        BOOL bTableJoin=TRUE);

    //No copying
    SwWW8ImplReader(const SwWW8ImplReader &);
    SwWW8ImplReader& operator=(const SwWW8ImplReader&);
public:     // eigentlich private, geht aber leider nur public
    void ConvertUFName( String& rName );

    long Read_Ftn(WW8PLCFManResult* pRes, BOOL);
    long Read_Field(WW8PLCFManResult* pRes, BOOL);
    long Read_Book(WW8PLCFManResult*, BOOL bStartAttr);
    long Read_And(WW8PLCFManResult* pRes, BOOL bStartAttr);

                                        // Attribute

    void Read_Special(USHORT, const BYTE*, short nLen);
    void Read_Obj(USHORT, const BYTE*, short nLen);
    void Read_PicLoc(USHORT, const BYTE* pData, short nLen );
    void Read_BoldUsw(          USHORT nId, const BYTE*, short nLen );
    void Read_SubSuper(         USHORT, const BYTE*, short nLen );
    BOOL ConvertSubToGraphicPlacement();
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
    void Read_Invisible(USHORT, const BYTE*, short nLen);
    void Read_Emphasis(         USHORT, const BYTE* pData, short nLen );
    void Read_ScaleWidth(       USHORT, const BYTE* pData, short nLen );
    void Read_Relief(           USHORT, const BYTE* pData, short nLen);

    void Read_NoLineNumb(       USHORT nId, const BYTE* pData, short nLen );

    void Read_LR(               USHORT nId, const BYTE*, short nLen );
    void NeedAdjustStyleTabStops(short nLeft, short nFirstLineOfst,
        SwWW8StyInf *pSty);
    void NeedAdjustTextTabStops(short nLeft, short nFirstLineOfst,
        SwNodeIndex *pPos,xub_StrLen nIndex);
    void Read_UL(               USHORT nId, const BYTE*, short nLen );
    void Read_LineSpace(        USHORT, const BYTE*, short nLen );
    void Read_Justify(          USHORT, const BYTE*, short nLen );
    void Read_Hyphenation(      USHORT, const BYTE* pData, short nLen );
    void Read_WidowControl(     USHORT, const BYTE* pData, short nLen );
    void Read_AlignFont(        USHORT, const BYTE* pData, short nLen );
    void Read_UsePgsuSettings(  USHORT, const BYTE* pData, short nLen );
    void Read_KeepLines(        USHORT, const BYTE* pData, short nLen );
    void Read_KeepParas(        USHORT, const BYTE* pData, short nLen );
    void Read_BreakBefore(      USHORT, const BYTE* pData, short nLen );
    void Read_ApoPPC(           USHORT, const BYTE* pData, short nLen );

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
    static BOOL ParseTabPos(WW8_TablePos *aTabPos, WW8PLCFx_Cp_FKP* pPap);
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
    BOOL SetTxtFmtCollAndListLevel(const SwPaM& rRg, SwWW8StyInf& rStyleInfo);

    void Read_StyleCode(USHORT, const BYTE* pData, short nLen);
    void Read_Majority(USHORT, const BYTE* , short );
    void Read_DoubleLine_Rotate( USHORT, const BYTE* pDATA, short nLen);

    void Read_TxtForeColor(USHORT, const BYTE* pData, short nLen);
    void Read_TxtBackColor(USHORT, const BYTE* pData, short nLen);
    void Read_ParaBackColor(USHORT, const BYTE* pData, short nLen);
    static sal_uInt32 ExtractColour(const BYTE* &rpData);

    long MapBookmarkVariables(const WW8FieldDesc* pF,String &rOrigName,
        const String &rData);
    const String &GetMappedBookmark(String &rOrigName);

    // Felder
    eF_ResT Read_F_Nul(WW8FieldDesc*, String& );
    eF_ResT Read_F_Input(WW8FieldDesc*, String& rStr);
    eF_ResT Read_F_InputVar(WW8FieldDesc*, String& rStr);
    eF_ResT Read_F_ANumber( WW8FieldDesc*, String& );
    eF_ResT Read_F_DocInfo( WW8FieldDesc* pF, String& rStr );
    eF_ResT Read_F_Author( WW8FieldDesc*, String& );
    eF_ResT Read_F_TemplName( WW8FieldDesc*, String& );
    short GetTimeDatePara(String& rStr, ULONG& rFormat);
    eF_ResT Read_F_DateTime( WW8FieldDesc*, String& rStr );
    eF_ResT Read_F_FileName( WW8FieldDesc*, String& );
    eF_ResT Read_F_Anz( WW8FieldDesc* pF, String& );
    eF_ResT Read_F_CurPage( WW8FieldDesc*, String& );
    eF_ResT Read_F_Ref( WW8FieldDesc* pF, String& );

    eF_ResT Read_F_Set( WW8FieldDesc*, String& rStr );
    eF_ResT Read_F_PgRef( WW8FieldDesc*, String& rStr );
    eF_ResT Read_F_NoteReference( WW8FieldDesc* pF, String& rStr );

    eF_ResT Read_F_Tox( WW8FieldDesc* pF, String& rStr );
    eF_ResT Read_F_Symbol( WW8FieldDesc*, String& rStr );
    eF_ResT Read_F_Embedd( WW8FieldDesc*, String& rStr );
    eF_ResT Read_F_FormTextBox( WW8FieldDesc* pF, String& rStr);
    eF_ResT Read_F_FormCheckBox( WW8FieldDesc* pF, String& rStr );
    eF_ResT Read_F_FormListBox( WW8FieldDesc* pF, String& );
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

    eF_ResT Read_F_OCX( WW8FieldDesc*, String& );
    eF_ResT Read_F_Hyperlink( WW8FieldDesc*, String& rStr );

    void DeleteFormImpl();

    short ImportSprm( const BYTE* pPos, USHORT nId = 0 );

    BOOL SearchRowEnd(WW8PLCFx_Cp_FKP* pPap,WW8_CP &rStartCp, int nLevel) const;

    const WW8Fib& GetFib() const    { return *pWwFib; }
    SwDoc& GetDoc() const           { return rDoc; }
    USHORT GetNAktColl()  const     { return nAktColl; }
    void SetNAktColl( USHORT nColl ) { nAktColl = nColl;    }
    void SetAktItemSet( SfxItemSet* pItemSet ) { pAktItemSet = pItemSet; }
    const USHORT StyleUsingLFO( USHORT nLFOIndex ) const ;
    const SwFmt* GetStyleWithOrgWWName( String& rName ) const ;

    static BOOL GetPictGrafFromStream( Graphic& rGraphic, SvStream& rSrc,
        ULONG nLen = ULONG_MAX );
    static void PicRead( SvStream *pDataStream, WW8_PIC *pPic, BOOL bVer67);
    static BOOL ImportOleWMF( SvStorageRef xSrc1, GDIMetaFile &rWMF,
        long &rX, long &rY);

    SwWW8ImplReader( BYTE nVersionPara, SvStorage* pStorage, SvStream* pSt,
        SwDoc& rD, bool bNewDoc );

    const ULONG GetFieldFlags() const{ return nFieldFlags; }
    const ULONG GetIniFlags()   const{ return nIniFlags; }
    const ULONG GetIniFlags1()  const{ return nIniFlags1; }

    // Laden eines kompletten DocFiles
    ULONG LoadDoc( SwPaM&,WW8Glossary *pGloss=0);
};

#endif
