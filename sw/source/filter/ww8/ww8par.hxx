/*************************************************************************
 *
 *  $RCSfile: ww8par.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: cmc $ $Date: 2001-02-21 13:49:03 $
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


//#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
//#include <com/sun/star/drawing/XShape.hpp>
//#endif
//#ifndef _COM_SUN_STAR_FORM_XFORMCOMPONENT_HPP_
//#include <com/sun/star/form/XFormComponent.hpp>
//#endif
//#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
//#include <com/sun/star/beans/XPropertySet.hpp>
//#endif

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _VCL_COLOR_HXX
#include <vcl/color.hxx>
#endif
#ifndef _SV_FONTTYPE_HXX //autogen
#include <vcl/fonttype.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif

#ifndef _FLTSHELL_HXX
#include <fltshell.hxx>         // fuer den Attribut Stack
#endif
#ifndef _WW8STRUC_HXX
#include <ww8struc.hxx>     // WW8_BRC ( Forward-Declaration mit anschliessender
#endif                      // Benutzung von WW8_BRC[4] in Deklaration will der
                            // doofe Os2Blci nicht )
#ifndef _WW8SCAN_HXX
#include <ww8scan.hxx>  // WW8Fib
#endif

#ifndef _WW8GLSY_HXX
#include <ww8glsy.hxx>
#endif

#ifndef _MSDFFIMP_HXX
#include <svx/msdffimp.hxx>
#endif

#ifndef _MSOCXIMEX_HXX
#include <svx/msocximex.hxx>
#endif

#define WW8_ASCII2STR(s) String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(s))

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

//class WW8CtrlStack;
class SwFltControlStack;
//class WW8EndStack;
class SwFltEndStack;
class SwWW8StyInf;
class WW8Fib;
class WW8PLCFMan;
struct WW8PLCFManResult;
class WW8Fonts;
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
class WW8ReaderSave;
struct WW8PicDesc;
class Graphic;
//class String;
class SwFieldType;
class SvStorage;
// alt: class SvStorageRef;
class SwFlyFrmFmt;
class SwAttrSet;
class GDIMetaFile;
struct ESelection;
class SfxItemSet;
struct WW8PLCFxDesc;

struct WW8ULSpaceData;
class SdrAttrObj;
struct WW8ULSpaceData;

namespace com{namespace sun {namespace star{
    namespace beans{ class XPropertySet;}
    namespace form { class XFormComponent;}
    namespace drawing{class XShape;}
    namespace lang{class XMultiServiceFactory;}
}}}

//#define WW8_CHARBUF_SIZE 1040 // Lese-Puffer fuer nackten Text

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

struct WW8LSTInfo;
struct WW8LFOInfo;

typedef WW8LSTInfo* WW8LSTInfo_Ptr;
typedef WW8LFOInfo* WW8LFOInfo_Ptr;
// Zeichenattribute aus GrpprlChpx
typedef SfxItemSet* WW8aISet[nWW8MaxListLevel ];
// Zeichen Style Pointer
typedef SwCharFmt* WW8aCFmt[ nWW8MaxListLevel ];
// Redlining: match WinWord author ids to StarWriter author ids
struct WW8AuthorInfo;
typedef WW8AuthorInfo* WW8AuthorInfo_Ptr;

SV_DECL_PTRARR_SORT_DEL(WW8LSTInfos,    WW8LSTInfo_Ptr,     16,16);
SV_DECL_PTRARR_DEL(     WW8LFOInfos,    WW8LFOInfo_Ptr,     16,16);
SV_DECL_PTRARR_SORT_DEL(WW8AuthorInfos, WW8AuthorInfo_Ptr,  16,16);

class SwWW8ImplReader;
class WW8ListManager
{
    SwWW8ImplReader& rReader;
    SwDoc&           rDoc;
    const WW8Fib&    rFib;
    SvStream&        rSt;
    WW8LSTInfos* pLSTInfos;// Daten aus PLCF LST, sortiert nach LST Id
    WW8LFOInfos* pLFOInfos;// D. aus PLF LFO, sortiert genau wie im WW8 Stream
    USHORT       nLSTInfos;// geht schneller als Abfrage von pLSTInfos->Count()
    USHORT       nLFOInfos;// dito
    BYTE* GrpprlHasSprm(USHORT nId, BYTE& rSprms, BYTE nLen);
    WW8LSTInfo* GetLSTByStreamPos( USHORT nStreamPos ) const;
    WW8LSTInfo* GetLSTByListId(    ULONG  nIdLst     ) const;
    BOOL ReadLVL(   BYTE nLevel,
                    SwNumFmt& rNumFmt,
                    SfxItemSet*& rpItemSet,
                    USHORT nLevelStyle,
                    BOOL bSetStartNo );
    void AdjustLVL( BYTE       nLevel,
                    SwNumRule& rNumRule,
                    WW8aISet&  rListItemSet,
                    WW8aCFmt&  aCharFmt,
                    BOOL&      bNewCharFmtCreated,  // Ausgabe-Parameter
                    String     aPrefix = aEmptyStr );
    BOOL LFOequaltoLST(WW8LFOInfo& rLFOInfo);
public:
    WW8ListManager(SvStream& rSt_, SwWW8ImplReader& rReader_);
    ~WW8ListManager();
    SwNumRule* GetNumRuleForActivation(USHORT nLFOPosition) const;
    BOOL IsSimpleList(USHORT nLFOPosition) const;
};



//-----------------------------------------
//            Stack
//-----------------------------------------
class SwWW8FltControlStack : public SwFltControlStack
{
    SwWW8ImplReader& rReader;
    USHORT nToggleAttrFlags;
protected:
    virtual void SetAttrInDoc(const SwPosition& rTmpPos, SwFltStackEntry* pEntry);

public:
    SwWW8FltControlStack( SwDoc* pDo, ULONG nFieldFl, SwWW8ImplReader& rReader_ ) :
        SwFltControlStack( pDo, nFieldFl ),
        rReader( rReader_ ),
        nToggleAttrFlags( 0 )
    {}

    BOOL IsFtnEdnBkmField(SwFmtFld& rFmtFld, USHORT& nBkmNo);
    void NewAttr(const SwPosition& rPos, const SfxPoolItem& rAttr);
    void SetToggleAttr( BYTE nId, BOOL bOn )
    {
        if( bOn )
            nToggleAttrFlags |= (1 << nId);
        else
            nToggleAttrFlags &= ~(1 << nId);
    }
    USHORT GetToggleAttrFlags() const { return nToggleAttrFlags; }
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
    WW8PLCFxSaveAll aPLCFxSave;
    WW8PLCFMan* pOldPlcxMan;

    WW8FlyPara* pWFlyPara;
    WW8SwFlyPara* pSFlyPara;
    WW8TabDesc* pTableDesc;
    USHORT nAktColl;
    USHORT nNoAttrScan;
    sal_Unicode cSymbol;
    BOOL bIgnoreText    : 1;
    BOOL bDontCreateSep : 1;
    BOOL bSymbol        : 1;
    BOOL bHdFtFtnEdn    : 1;
    BOOL bApo           : 1;
    BOOL bTxbxFlySection: 1;
    BOOL bTable         : 1;
    BOOL bTableInApo    : 1;
    BOOL bAnl           : 1;
    BOOL bNeverCallProcessSpecial: 1;
public:
    WW8ReaderSave( SwWW8ImplReader* pRdr, WW8_CP nStart=-1 );
    void Restore( SwWW8ImplReader* pRdr );
};



enum eF_ResT{ F_OK, F_TEXT, F_TAGIGN, F_TAGTXT, F_READ_FSPA };

struct SwWW8Shade{
    Color aColor;
    BOOL  bWhiteText;
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
    WW8FormulaControl(const UniString& sN,SwWW8ImplReader &rR)
        : OCX_Control(sN), rRdr(rR) {}
    void WW8FormulaControl::SetOthersFromDoc(com::sun::star::uno::Reference <
        com::sun::star::form::XFormComponent> &rFComp,
        com::sun::star::awt::Size &rSz,
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

    UniString sTitle;
    UniString sDefault;
    UniString sFormatting;
    UniString sHelp;
    UniString sToolTip;

    void Read(SwWw8ControlType nWhich,SvStream *pD);
private:
    SwWW8ImplReader &rRdr;
};

class WW8FormulaCheckBox: public WW8FormulaControl
{
public:
    WW8FormulaCheckBox(SwWW8ImplReader &rR)
        : WW8FormulaControl( WW8_ASCII2STR( "CheckBox" ), rR)
        {}
    BOOL Import(const com::sun::star::uno::Reference <
        com::sun::star::lang::XMultiServiceFactory> &rServiceFactory,
        com::sun::star::uno::Reference <
        com::sun::star::form::XFormComponent> &rFComp,
        com::sun::star::awt::Size &rSz);
};

class WW8FormulaEditBox: public WW8FormulaControl
{
public:
    WW8FormulaEditBox(SwWW8ImplReader &rR)
        : WW8FormulaControl( WW8_ASCII2STR( "TextField" ) ,rR)
        {}
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
    BOOL InsertFormula( WW8FormulaControl &rFormula,
        com::sun::star::uno::Reference <
        com::sun::star::drawing::XShape> *pShapeRef=0 );
    BOOL InsertControl(const com::sun::star::uno::Reference<
        com::sun::star::form::XFormComponent >& rFComp,
        const ::com::sun::star::awt::Size& rSize,
        com::sun::star::uno::Reference <
        com::sun::star::drawing::XShape > *pShape,BOOL bFloatingCtrl);
    BOOL SwMSConvertControls::ExportControl(Writer &rWrt,const SdrObject *pObj);
};

class SwMSDffManager : public SvxMSDffManager
{
    SwWW8ImplReader& rReader;
    virtual BOOL GetOLEStorageName( long nOLEId, String& rStorageName,
                                    SvStorageRef& rSrcStorage,
                                    SvStorageRef& rDestStorage ) const;
    virtual BOOL ShapeHasText( ULONG nShapeId, ULONG nFilePos ) const;
    virtual SdrObject* ImportOLE( long nOLEId, const Graphic& rGrf,
                                    const Rectangle& rBoundRect ) const;

//  virtual void ProcessClientAnchor2( SvStream& rStData, DffRecordHeader& rHd, void* pData, DffObjData& );

public:
    static UINT32 GetFilterFlags();
    SwMSDffManager( SwWW8ImplReader& rRdr );
    const com::sun::star::uno::Reference< com::sun::star::drawing::XShape >
        GetLastOCXShape() const {return xShape;}
private:
// If we convert an OCX through this manager we will store the uno XShape
// reference created through the conversion
    mutable com::sun::star::uno::Reference<
        com::sun::star::drawing::XShape > xShape;
};


//-----------------------------------------
//            Storage-Reader
//-----------------------------------------
class SwWW8ImplReader
{
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
    SwFltEndStack*        pEndStck;     // End-Stack fuer die Attribute
    SwWW8FltControlStack* pRefFldStck;  // for Reference Fields

//  BYTE* pCharBuf;             // Puffer fuer nackten Text
    SwMSConvertControls *pFormImpl; // Control-Implementierung

    SwFlyFrmFmt* pFlyFmtOfJustInsertedGraphic;
    SwFrmFmt* pFmtOfJustInsertedGraphicOrOLE;
    WW8Fib* pWwFib;
    WW8Fonts* pFonts;
    WW8Dop* pWDop;
    WW8ListManager* pLstManager;
    WW8ScannerBase* pSBase;
    WW8PLCFMan* pPlcxMan;

    WW8RStyle* pStyles;             // Pointer auf die Style-Einleseklasse
    SwFmt* pAktColl;                // gerade zu erzeugende Collection
                                    // ( ist ausserhalb einer Style-Def immer 0 )
    SfxItemSet* pAktItemSet;        // gerade einzulesende Zeichenattribute
                                    // ( ausserhalb des WW8ListManager Ctor's immer 0 )
    SwWW8StyInf* pCollA;            // UEbersetzungs-Array der Styles
    const SwTxtFmtColl* pDfltTxtFmtColl;    // Default
    SwFmt* pStandardFmtColl;        // "Standard"

    SwPageDesc* pPageDesc;      // fuer uebernommene KF-Zeilen
    WW8PLCF_HdFt* pHdFt;        // Pointer auf Header / Footer - Scannerklasse

    WW8FlyPara* pWFlyPara;      // WW-Parameter
    WW8SwFlyPara* pSFlyPara;    // daraus erzeugte Sw-Parameter

    WW8TabDesc* pTableDesc;     // Beschreibung der Tabelleneigenschaften

    SwNumRule* pNumRule;        // fuer Nummerierung / Aufzaehlungen im Text
    WW8_OLST* pNumOlst;         // Gliederung im Text
    SwNodeIndex* pBehindSection;// Node-Index zum Zuruecksetzen des PaM nach einem Bereich
    SwSection*   pNewSection;   // last Section that was inserted into the doc

    SwNode*      pNode_FLY_AT_CNTNT; // set: WW8SwFlyPara()   read: CreateSwTable()

    SwDrawFrmFmt *pDrawFmt;     // wie FlyFrmFmt
    SdrModel* pDrawModel;
    SdrPage* pDrawPg;
    SdrObjList* pDrawGroup;
    SvShorts* pDrawHeight;      // Welches Objekt ueberdeckt welches ?
    EditEngine* pDrawEditEngine;

    SwFieldType* pNumFldType;   // fuer Nummernkreis

    SvxMSDffManager* pMSDffManager;

    SvStringsDtor* pAtnNames;

    WW8AuthorInfos* pAuthorInfos;

    SwNodeIndex* pLastPgDeskIdx;// for inserting a section when Ft-/End-Note
                                // with flag 'on end of section' set

                                // Ini-Flags:
    ULONG nIniFlags;            // Flags aus der writer.ini
    ULONG nIniFlags1;           // dito ( zusaetzliche Flags )
//  ULONG nIniHdSiz;            // dito fuer Header
    ULONG nIniFtSiz;            // dito fuer Default-Size Footer
    ULONG nFieldFlags;          // dito fuer Feldern
    ULONG nFieldTagAlways[3];   // dito fuers Taggen von Feldern
    ULONG nFieldTagBad[3];      // dito fuers Taggen von nicht importierbaren F.

    ULONG nLastFlyNode;         // Node number of last imported Fly

    ULONG nImportedGraphicsCount;   // benoetigt fuer MakeUniqueGraphName()

    ULONG nHdTextHeight;        // Hoehe des eingelesenen Headers
    ULONG nFtTextHeight;        // Hoehe des eingelesenen Footers

    ULONG nDrawObjOfs;
    long nDrawCpO;              // Anfang der Txbx-SubDocs

    long nPicLocFc;             // Picture Location in File (FC)
    long nObjLocFc;             // Object Location in File (FC)

    INT32 nIniFlyDx;            // X-Verschiebung von Flys
    INT32 nIniFlyDy;            // Y-Verschiebung von Flys

    rtl_TextEncoding eTextCharSet;      // Default-Charset fuer Text
    rtl_TextEncoding eStructCharSet;        // rtl_TextEncoding fuer interne Strukturen
    rtl_TextEncoding eFontSrcCharSet;   // Source-rtl_TextEncoding fuer aktuellen Font
    rtl_TextEncoding eFontDstCharSet;   // Dest-rtl_TextEncoding fuer aktuellen Font
    rtl_TextEncoding eHardCharSet;       // Hartes rtl_TextEncoding-Attribut

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
    BYTE nDrawHeaven, nDrawHell;
    BYTE nListLevel;

    BYTE nNfcPgn;               // Formatting of PageNum
    BYTE nPgChpDelim;           // ChapterDelim from PageNum
    BYTE nPgChpLevel;           // ChapterLevel of Heading from PageNum

    BYTE nCorrIhdt;             // used in CreateSep()
    BOOL bSectionHasATitlePage; // used in CreateSep()

    BOOL bNew;              // Neues Dokument ?
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
     BOOL bTable;           // wird gerade eine Tabelle eingelesen
    BOOL bTableInApo;       // Table is contained in Apo
    BOOL bWasTabRowEnd;     // Tabelle : Row End Mark
    BOOL bTxtCol;           // TextFarbe direkt gesetzt
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
    BOOL bNeverCallProcessSpecial; // Sonderfall zum einlesen eines 0x01
                                   // siehe: SwWW8ImplReader::Read_F_Hyperlink()

    // praktische Hilfsvariablen:
    BOOL bVer67;            // ( (6 == nVersion) || (7 == nVersion) );
    BOOL bVer6;             //   (6 == nVersion);
    BOOL bVer7;             //   (7 == nVersion);
    BOOL bVer8;             //   (8 == nVersion);

    BOOL bPgChpLevel;       // ChapterLevel of Heading from PageNum
    BOOL bEmbeddObj;        // EmbeddField gelesen
    BOOL bFloatingCtrl;     // Whether to anchor imported controls as char or para

    BOOL bAktAND_fNumberAcross; // current active Annotated List Deskriptor - ROW flag

    BOOL bNoLnNumYet;       // no Line Numbering has been activated yet (we import
                            //     the very 1st Line Numbering and ignore the rest)
    BOOL bRestartLnNumPerSection;




//---------------------------------------------

    void SetLastPgDeskIdx();

    SwPageDesc* CreatePageDesc( SwPageDesc* pFirstPageDesc,
                                SwPaM** ppPaMWanted = 0 );
    void RemoveCols( SwPageDesc& rPageDesc, SwFmtCol*& rpCol );
    BOOL SetCols( SwFrmFmt* pFmt, const WW8PLCFx_SEPX* pSep,
                    USHORT nNettoWidth,
                    BOOL bTestOnly = FALSE );
    void SetPage1( SwPageDesc* pPageDesc, SwFrmFmt &rFmt,
                   const WW8PLCFx_SEPX* pSep, USHORT nLIdx,
                   BOOL bIgnoreCols );
    void SetHdFt( SwPageDesc* pPageDesc0, SwPageDesc* pPageDesc1,
                  const WW8PLCFx_SEPX* pSep, BYTE nIPara );
    void GetPageULData( const  WW8PLCFx_SEPX* pSep,
                        USHORT nLIdx,
                        BOOL   bFirst,
                        WW8ULSpaceData& rData );
    void SetPageULSpaceItems( SwFrmFmt &rFmt, WW8ULSpaceData& rData );

    void SetPageBorder( SwPageDesc* pPageDesc0, SwPageDesc* pPageDesc1,
                        const WW8PLCFx_SEPX* pSep, USHORT nLIdx );
    void SetUseOn( SwPageDesc* pPageDesc0, SwPageDesc* pPageDesc1,
                   const WW8PLCFx_SEPX* pSep, BYTE nHdFt );
    void InsertSectionWithWithoutCols( SwPaM& rMyPaM, const SwFmtCol* pCol );
    void CreateSep( const long nTxtPos );

    void CopyPageDescHdFt( const SwPageDesc* pOrgPageDesc,
                           SwPageDesc* pNewPageDesc, BYTE nCode );

    void UpdatePageDescs( USHORT nPageDescOffset );

    void DeleteStk(SwFltControlStack* prStck);
    void DeleteCtrlStk()    { DeleteStk( pCtrlStck  ); pCtrlStck   = 0; }
    void DeleteEndStk()     { DeleteStk( pEndStck   ); pEndStck    = 0; }
    void DeleteRefFldStk()  { DeleteStk( pRefFldStck); pRefFldStck = 0; }

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
    void ReadText( long nStartCp, long nTextLen, short nType );

    void ReadRevMarkAuthorStrTabl( SvStream& rStrm, INT32 nTblPos, INT32 nTblSiz, SwDoc& rDoc );

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
    BOOL SetBorder( SvxBoxItem& pBox, const WW8_BRC* pbrc, BOOL bChkBtwn=FALSE );
    void GetBorderDistance( WW8_BRC* pbrc, Rectangle& rInnerDist );
    BOOL SetShadow( SvxShadowItem& rShadow, const SvxBoxItem& rBox, const WW8_BRC pbrc[4] );

    void SetFlyBordersShadow( SfxItemSet& rFlySet,
                              const WW8_BRC pbrc[4], USHORT nInnerMgn );

    BOOL MatchSdrBoxIntoFlyBoxItem( const Color& rLineColor,
                                    MSO_LineStyle eLineStyle,
                                    USHORT      nLineWidth,
                                    SvxBoxItem& rBox );
    void MatchSdrItemsIntoFlySet( SdrObject*    pSdrObj,
                                  SfxItemSet&   aFlySet,
                                  MSO_LineStyle eLineStyle,
                                  Rectangle&    rInnerDist,
                                  BOOL          bFixSize );
    void MatchWrapDistancesIntoFlyFmt( SvxMSDffImportRec* pRecord,
                                       SwFrmFmt*          pFlyFmt );

    void SetCropAtGrfNode( SvxMSDffImportRec* pRecord,
                           SwFrmFmt*          pFlyFmt,
                           WW8_FSPA*          pF );

    BOOL StartApo( BYTE* pSprm29, BOOL bNowStyleApo );
    void StopApo();
    BOOL TestSameApo( BYTE* pSprm29, BOOL bNowStyleApo );
    BYTE* TestApo( BOOL& rbStartApo, BOOL& rbStopApo, BOOL& rbNowStyleApo,
                   BOOL bInTable,    BOOL bTableRowEnd,
                   BOOL bStillInTable );
    BOOL ProcessSpecial( BOOL bAllEnd, BOOL* pbReSync );

    ULONG ReadWmfHeader( WmfFileHd* pHd, long nPos );
    BOOL ReadGrafFile( String& rFileName, Graphic*& rpGraphic,
       const WW8_PIC& rPic, SvStream* pSt, ULONG nFilePos, BOOL* pDelIt );

    SwFrmFmt* MakeGrafByFlyFmt( SdrTextObj* pReplaceTextObj,
                                const SwFrmFmt& rOldFmt,const WW8PicDesc& rPD,
                                const Graphic*  pGraph, const String& rFileName,
                                const String&   rGrName,const SfxItemSet& rGrfSet,
                                const BOOL  bSetToBackground );
    SwFrmFmt* MakeGrafNotInCntnt(const WW8PicDesc& rPD,
                                 const Graphic* pGraph, const String& rFileName,
                                 const String&  rGrName,const SfxItemSet& rGrfSet );
    SwFrmFmt* MakeGrafInCntnt(const WW8_PIC& rPic,   const WW8PicDesc& rPD,
                              const Graphic* pGraph, const String& rFileName,
                              const String&  rGrName,const SfxItemSet& rGrfSet );

    SwFrmFmt* ImportGraf1( WW8_PIC& rPic, SvStream* pSt, ULONG nFilePos );
    SwFrmFmt* ImportGraf(  SdrTextObj* pTextObj = 0,
                           SwFrmFmt*   pFlyFmt = 0,
                           BOOL        bSetToBackground = FALSE );
    BOOL ImportURL(String &sURL,String &sMark,WW8_CP nStart);

    SwFrmFmt* ImportOle( const Graphic* = 0, const SfxItemSet* pFlySet = 0 );

    BOOL ImportFormulaControl(WW8FormulaControl &rBox,WW8_CP nStart,
        SwWw8ControlType nWhich);

    void ImportDop( BOOL bNewDoc );

    //This converts MS Asian Typography information into OOo's
    void ImportDopTypography(const WW8DopTypography &rTypo);

    void SetImplicitTab();
    ULONG LoadDoc1( SwPaM& rPaM ,WW8Glossary *pGloss);

    BOOL StartTable();
    void TabCellEnd();
    void StopTable();
    short GetTableLeft();
    BOOL IsInvalidOrToBeMergedTabCell() const;

//  Brush* Shade1( SwWW8Shade& rRet, WW8_SHD* pS );

    ColorData GetCol( BYTE nIco );

// Nummerierungen / Aufzaehlungen ( Autonumbered List Data Descriptor )
// Liste:        ANLD ( Autonumbered List Data Descriptor )
//   eine Ebene: ANLV ( Autonumber Level Descriptor )
//
// Chg7-8:
// Listen sind in WW8 eigene Strukturen, die ueber folgende drei Tabellen
// verwaltet werden: rglst, hpllfo und hsttbListNames
// die Strukturen hierfuer sind: LSTF, LVLF, LFO LFOLVL

    void SetAnlvStrings( SwNumFmt* pNum, WW8_ANLV* pAV, BYTE* pTxt,
                            BOOL bOutline );
    void SetAnld( SwNumRule* pNumR, WW8_ANLD* pAD, BYTE nSwLevel, BOOL bOutLine );
    void SetNumOlst( SwNumRule* pNumR, WW8_OLST* pO, BYTE nSwLevel );
    SwNumRule* GetStyRule();

    void StartAnl( BYTE* pSprm13 );
    void NextAnlLine( BYTE* pSprm13, BYTE* pS12 = 0 );
    void StopAnl( BOOL bGoBack = TRUE );

// GrafikLayer

    BOOL MakeUniqueGraphName(String& rName, const String& rFixedPart);

    BOOL ReadGrafStart( void* pData, short nDataSiz, WW8_DPHEAD* pHd,
                        WW8_DO* pDo );
    void InsertObj( SdrObject* pObj, short nWwHeight );
    void ReadLine( WW8_DPHEAD* pHd, WW8_DO* pDo );
    void ReadRect( WW8_DPHEAD* pHd, WW8_DO* pDo );
    void ReadElipse( WW8_DPHEAD* pHd, WW8_DO* pDo );
    void ReadArc( WW8_DPHEAD* pHd, WW8_DO* pDo );
    void ReadPolyLine( WW8_DPHEAD* pHd, WW8_DO* pDo );
    ESelection GetESelection( long nCpStart, long nCpEnd );
    /*
    void GetTxbxCharAttrs( SfxItemSet& rS, const WW8PLCFxDesc& rD, BOOL bONLYnPicLocFc );
    void GetTxbxParaAttrs( SfxItemSet& rS, const WW8PLCFxDesc& rD );
    */
    void GetTxbxPapAndCharAttrs( SfxItemSet& rS, const WW8PLCFManResult& rRes );
    //void InsertTxbxCharAttrs( long nStartCp, long nEndCp, BOOL bONLYnPicLocFc );
    void InsertTxbxStyAttrs( SfxItemSet& rS, USHORT nColl );
    //void InsertTxbxParaAttrs( long nStartCp, long nEndCp );
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
                        SvxMSDffImportRec* pRecord = 0);
    BOOL TxbxChainContainsRealText( USHORT nTxBxS,
                                    long&  rStartCp,
                                    long&  rEndCp );
    void ReadTxtBox( WW8_DPHEAD* pHd, WW8_DO* pDo );
    void ReadCaptionBox( WW8_DPHEAD* pHd, WW8_DO* pDo );
    void ReadGroup( WW8_DPHEAD* pHd, WW8_DO* pDo );
    void ReadGrafPrimitive( short& rLeft, WW8_DO* pDo );
    void ReadGrafLayer1( WW8PLCFspecial* pPF, long nGrafAnchorCp );
    SdrObject* CreateContactObject( SwFlyFrmFmt* pFlyFmt );
    void ProcessEscherAlign( SvxMSDffImportRec* pRecord,
                             WW8_FSPA&          rFSPA,
                             SfxItemSet&        rFlySet );
    SwFrmFmt* Read_GrafLayer( long nGrafAnchorCp );
    void GrafikCtor();
    void GrafikDtor();

// anderes
    String GetFieldResult( WW8FieldDesc* pF );
    void QuoteChar( String& rStr, const sal_Unicode _c, BOOL bAllowCr );
    void QuoteString( String& rStr, const String& rOrg );
    void MakeTagString( String& rStr, const String& rOrg );
    void UpdateFields();
//  void ConvertFName( String& rName );
    void ConvertFFileName( String& rName, const String& rRaw );
    long Read_F_Tag( WW8FieldDesc* pF );
    void InsertTagField( const USHORT nId, const String& rTagText );
    long ImportExtSprm( WW8PLCFManResult* pRes, BOOL bStart );
    void ReadDocInfo();

// Ver8-Listen

    void RegisterNumFmtOnTxtNode(   USHORT nActLFO,
                                    BYTE   nActLevel,
                                    BOOL   bSetAttr = TRUE );
    void RegisterNumFmtOnStyle(     SwWW8StyInf& rStyleInfo,
                                    USHORT nActLFO   = USHRT_MAX,
                                    BYTE   nActLevel = nWW8MaxListLevel );
    void RegisterNumFmt(USHORT nActLFO, BYTE nActLevel);

// Pictures mit Kode 100, die ueber MsoftbSpContainer abgespeichert sind.
    BOOL WW8QuickHackForMSDFF_DirectBLIPImport( SvStream&   rSt,
                                                WW8_PIC&    rPic,
                                                Graphic&    rData,
                                                String&     rGraphName);
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

public:     // eigentlich private, geht aber leider nur public
    void ConvertUFName( String& rName );

    long Read_Ftn( WW8PLCFManResult* pRes, BOOL );
    long Read_Field( WW8PLCFManResult* pRes, BOOL );
    long Read_Book( WW8PLCFManResult* pRes, BOOL bStartAttr );
    long Read_Piece( WW8PLCFManResult* pRes, BOOL bStartAttr );
    long Read_And( WW8PLCFManResult* pRes, BOOL bStartAttr );

                                        // Attribute

    void Read_Special(          USHORT, BYTE*, short nLen );
    void Read_Obj(              USHORT, BYTE*, short nLen );
    void Read_PicLoc(           USHORT nId, BYTE* pData, short nLen );
    void Read_BoldUsw(          USHORT nId, BYTE*, short nLen );
    void Read_SubSuper(         USHORT, BYTE*, short nLen );
    void Read_SubSuperProp(     USHORT, BYTE*, short nLen );
    void Read_Underline(        USHORT, BYTE*, short nLen );
    void Read_TxtColor(         USHORT, BYTE*, short nLen );
    void Read_FontCode(         USHORT, BYTE*, short nLen );
    void Read_FontSize(         USHORT, BYTE*, short nLen );
    void Read_CharSet(          USHORT nId, BYTE* pData, short nLen );
    void Read_Language(         USHORT, BYTE*, short nLen );
    void Read_CColl(            USHORT, BYTE*, short nLen );
    void Read_Kern(             USHORT, BYTE* pData, short nLen );
    void Read_FontKern(         USHORT, BYTE* pData, short nLen );
    void Read_Invisible(        USHORT, BYTE* pData, short nLen );
    void Read_Emphasis(         USHORT, BYTE* pData, short nLen );
    void Read_ScaleWidth(       USHORT, BYTE* pData, short nLen );

    void Read_NoLineNumb(       USHORT nId, BYTE* pData, short nLen );

    void Read_LR(               USHORT nId, BYTE*, short nLen );
    void Read_UL(               USHORT nId, BYTE*, short nLen );
    void Read_LineSpace(        USHORT, BYTE*, short nLen );
    void Read_Justify(          USHORT, BYTE*, short nLen );
    void Read_Hyphenation(      USHORT, BYTE* pData, short nLen );
    void Read_WidowControl(     USHORT, BYTE* pData, short nLen );
    void Read_KeepLines(        USHORT, BYTE* pData, short nLen );
    void Read_KeepParas(        USHORT, BYTE* pData, short nLen );
    void Read_BreakBefore(      USHORT, BYTE* pData, short nLen );
    void Read_ApoPPC(           USHORT, BYTE* pData, short nLen );

    void Read_BoolItem(         USHORT nId, BYTE*, short nLen );

    void Read_Border(           USHORT nId, BYTE* pData, short nLen );
    void Read_Tab(              USHORT nId, BYTE* pData, short nLen );
    void Read_TabCellDelete(    USHORT nId, BYTE* pData, short nLen );
    void Read_TabCellInsert(    USHORT nId, BYTE* pData, short nLen );
    void Read_Symbol(           USHORT nId, BYTE* pData, short nLen );
    void Read_FldVanish(        USHORT nId, BYTE* pData, short nLen );

    // Revision Marks ( == Redlining )

    // insert or delete content (change char attributes resp.)
    void Read_CRevisionMark(SwRedlineType eType,
                            USHORT nId,
                            BYTE* pData,
                            short nLen );
    // insert new content
    void Read_CFRMark(          USHORT nId, BYTE* pData, short nLen );
    // delete old content
    void Read_CFRMarkDel(       USHORT nId, BYTE* pData, short nLen );
    // change properties of content (e.g. char formating)
    void Read_CPropRMark(       USHORT nId, BYTE* pData, short nLen ); // complex!


    void Read_TabRowEnd(        USHORT, BYTE* pData, short nLen );
    void Read_Shade(            USHORT, BYTE* pData, short nLen );
    void Read_ANLevelNo(        USHORT, BYTE* pData, short nLen );
    void Read_ANLevelDesc(      USHORT, BYTE* pData, short nLen );

    void Read_POutLvl( USHORT nId, BYTE* pData, short nLen ); // Gliederungsebene Ver8

    void Read_OLST(             USHORT, BYTE* pData, short nLen );

    void Read_CharShadow(       USHORT, BYTE* pData, short nLen );
    void Read_CharHighlight(    USHORT, BYTE* pData, short nLen );
                                        // Ver8-Listen

    void Read_ListLevel(        USHORT nId, BYTE* pData, short nLen);
    void Read_LFOPosition(      USHORT nId, BYTE* pData, short nLen);
    BOOL SetTxtFmtCollAndListLevel( const SwPaM& rRg,
                                    SwWW8StyInf& rStyleInfo,
                                    BOOL         bReset = TRUE );

                                        // FastSave-Attribute

    void Read_StyleCode(        USHORT, BYTE* pData, short nLen );
    void Read_Majority(         USHORT, BYTE* pData, short nLen );
    void Read_DoubleLine_Rotate( USHORT, BYTE* pDATA, short nLen);

                                        // Felder

    eF_ResT Read_F_Nul( WW8FieldDesc*, String& rStr );
    eF_ResT Read_F_Input( WW8FieldDesc*, String& rStr );
    eF_ResT Read_F_InputVar( WW8FieldDesc*, String& rStr );
    eF_ResT Read_F_ANumber( WW8FieldDesc*, String& );
    eF_ResT Read_F_DocInfo( WW8FieldDesc* pF, String& rStr );
    eF_ResT Read_F_Author( WW8FieldDesc*, String& );
    eF_ResT Read_F_TemplName( WW8FieldDesc*, String& );
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
    eF_ResT Read_F_FormTextBox( WW8FieldDesc* pF, String& rStr );
    eF_ResT Read_F_FormCheckBox( WW8FieldDesc* pF, String& rStr );
    eF_ResT Read_F_FormListBox( WW8FieldDesc* pF, String& rStr );
    eF_ResT Read_F_Macro( WW8FieldDesc*, String& rStr );
    eF_ResT Read_F_DBField( WW8FieldDesc*, String& rStr );
    eF_ResT Read_F_DBNext( WW8FieldDesc*, String& );
    eF_ResT Read_F_DBNum( WW8FieldDesc*, String& );
    eF_ResT Read_F_Equation( WW8FieldDesc*, String& );
    eF_ResT Read_F_IncludePicture( WW8FieldDesc*, String& rStr );
    eF_ResT Read_F_IncludeText(    WW8FieldDesc*, String& rStr );
    eF_ResT Read_F_Seq( WW8FieldDesc*, String& rStr );

    eF_ResT Read_F_OCX( WW8FieldDesc*, String& rStr );
    eF_ResT Read_F_Hyperlink( WW8FieldDesc*, String& rStr );

    BOOL InsertControl(const com::sun::star::uno::Reference<
        com::sun::star::form::XFormComponent>& rFComp,
        const ::com::sun::star::awt::Size& rSize,
        com::sun::star::uno::Reference<
        com::sun::star::drawing::XShape> *pShape=NULL,
        BOOL bFloatingCtrl = FALSE );

    void BuildInputField( USHORT eType, const String& rParam );
    void DeleteFormImpl();

                                // Ver8: Listen Manager
    short ImportSprm( BYTE* pPos, short nSprmsLen, USHORT nId = 0 );

    const WW8Fib& GetFib() const    { return *pWwFib; }
    SwDoc& GetDoc() const           { return rDoc;          }
    USHORT GetNAktColl()  const     { return nAktColl;  }
    void SetNAktColl( USHORT nColl ) { nAktColl = nColl;    }
    //SfxItemSet* GetAktItemSet()   { return pAktItemSet;   }
    void SetAktItemSet( SfxItemSet* pItemSet ) { pAktItemSet = pItemSet;    }
    const USHORT StyleUsingLFO(      USHORT nLFOIndex ) const ;
    const SwFmt* GetStyleWithOrgWWName( String& rName    ) const ;

    static BOOL GetPictGrafFromStream( Graphic& rGraphic, SvStream& rSrc,
                                    ULONG nLen = ULONG_MAX );


    SwWW8ImplReader( BYTE nVersionPara, SvStorage* pStorage,
                     SvStream* pSt, SwDoc& rD, BOOL bNewDoc );

    const ULONG GetFieldFlags() const{ return nFieldFlags; }
    const ULONG GetIniFlags()   const{ return nIniFlags; }
    const ULONG GetIniFlags1()  const{ return nIniFlags1; }

    // Laden eines kompletten DocFiles
    ULONG LoadDoc( SwPaM&,WW8Glossary *pGloss=0);
};



/*************************************************************************

    Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/ww8/ww8par.hxx,v 1.13 2001-02-21 13:49:03 cmc Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.12  2001/02/15 20:08:10  jp
      im-/export the Rotate-/ScaleWidth-Character attribut

      Revision 1.11  2001/02/06 17:28:21  cmc
      #83581# CJK Two Lines in One {Im|Ex}port for Word

      Revision 1.10  2001/01/30 20:11:06  cmc
      #83362# CJK Forbidden Character {Im|Ex}port

      Revision 1.9  2000/12/15 15:33:06  cmc
      #79055# OCX FormControls changes

      Revision 1.8  2000/12/04 14:08:08  khz
      #78930# Pictures in Hyperlinks will be imported as Graphics with Hyperlink

      Revision 1.7  2000/12/01 11:22:52  jp
      Task #81077#: im-/export of CJK documents

      Revision 1.6  2000/11/23 13:37:53  khz
      #79474# Save/restore PLCF state before/after reading header or footer data

      Revision 1.5  2000/11/20 14:09:17  jp
      Read_FieldIniFlags removed

      Revision 1.4  2000/11/01 12:12:16  jp
      optimize: use the same code to read MAC-Pict

      Revision 1.3  2000/10/16 10:54:58  khz
      read extended WW9-Frame-Alignment (stored in Escher record 0xF122)

      Revision 1.2  2000/10/10 16:54:06  cmc
      MSOffice 97/2000 Controls {Im|Ex}port

      Revision 1.1.1.1  2000/09/18 17:14:58  hr
      initial import

      Revision 1.95  2000/09/18 16:05:00  willem.vandorp
      OpenOffice header added.

      Revision 1.94  2000/08/22 17:40:57  cmc
      #77315# 95 Formulas, bad string read

      Revision 1.93  2000/08/22 17:08:58  cmc
      #77743# OLE Import, bad seek & bad FilterOptions

      Revision 1.92  2000/08/18 09:48:28  khz
      Import Line Numbering (restart on new section)

      Revision 1.91  2000/08/18 06:47:26  khz
      Import Line Numbering

      Revision 1.90  2000/07/28 15:37:19  khz
      #73796# don't delete NumRule from Attr but set it into pDoc

      Revision 1.89  2000/07/27 10:21:26  khz
      #73796# stop ANList when opening next cell in a row and !pAktANLD->fNumberAcross

      Revision 1.88  2000/07/25 15:16:25  khz
      #76811# read/write AutoHyphenation flag from/into Document Properties

      Revision 1.87  2000/07/17 13:46:56  khz
      #73987# check if sprmSNfcPgn should cause section change or not

      Revision 1.86  2000/07/11 11:39:10  khz
      #76673# prepare implementation of sprmTDelete and sprmTInsert

      Revision 1.85  2000/07/07 12:48:54  jp
      must changes VCL

      Revision 1.84  2000/06/28 08:07:39  khz
      #70915# Insert Section if end-note with flag 'on end of section' found

      Revision 1.83  2000/06/15 15:23:39  cmc
      #75669# 97Controls Import fixes

      Revision 1.82  2000/05/31 12:22:57  khz
      Changes for Unicode

      Revision 1.81  2000/05/25 08:06:45  khz
      Piece Table optimization, Unicode changes, Bugfixes

      Revision 1.80  2000/05/24 08:56:06  jp
      Bugfixes for Unicode

      Revision 1.79  2000/05/17 16:41:43  khz
      Changes for unicode (2)

      Revision 1.78  2000/05/16 11:06:17  khz
      Unicode code-conversion

      Revision 1.77  2000/03/27 15:55:24  cmc
      #74329# Ported WW97 Controls to uno codebase

      Revision 1.76  2000/03/24 10:50:36  khz
      Task #74368# IsInvalidOrToBeMergedTabCell() stops duplicate reset of NumRule

      Revision 1.75  2000/03/21 15:04:57  os
      UNOIII

      Revision 1.74  2000/03/10 14:29:32  khz
      Task #65529# improve detection if Section is identical with previous

      Revision 1.73  2000/03/03 15:20:02  os
      StarView remainders removed

      Revision 1.72  2000/02/18 09:36:56  cmc
      #69372# Improved Hyperlink Importing for WW97

      Revision 1.71  2000/02/14 14:39:02  jp
      #70473# changes for unicode

      Revision 1.70  2000/02/14 09:10:28  cmc
      #72579# working glossary implementation

      Revision 1.68  2000/02/09 08:57:23  khz
      Task #72647# Read SPRMs that are stored in piece table grpprls (2)

      Revision 1.67  2000/02/02 18:07:39  khz
      Task #69885# Read SPRMs that are stored in piece table grpprls

      Revision 1.66  2000/01/06 15:23:45  khz
      Task #71411# Let last Section be unbalanced

      Revision 1.65  1999/12/23 14:03:10  khz
      Task #68143# avoid attributes atached solely to Cell-End marks

      Revision 1.64  1999/12/22 18:03:16  khz
      Task #70919# look if ParaStyle is different behind filed than it was before

      Revision 1.63  1999/12/14 14:22:38  khz
      Task #70815# Avoid typecast when calling MatchSdrItemsIntoFlySet()

      Revision 1.62  1999/12/09 19:12:20  khz
      Task #65551# nSkipPos had erroneously been a member instead of local variable

      Revision 1.61  1999/12/09 16:46:29  khz
      Task #69180# allow Tabs if right of 1st-line-start OR right of paragraph margin

      Revision 1.60  1999/12/01 14:35:45  khz
      Task #68488# Graphics in Sections with more than one Column

      Revision 1.59  1999/11/29 11:09:36  khz
      Task #70177# SetDerivedFrom( RES_POOL_COL_STANDARD )

      Revision 1.58  1999/11/19 14:57:07  khz
      Task #68651# Store state of bSymbol and nSymbol in WW8ReaderSave class

      Revision 1.57  1999/11/12 13:04:59  jp
      new: read sprmCHighlight

      Revision 1.56  1999/11/05 17:05:28  khz
      prevent from erroneous Table-Restart when sprm24 missing

      Revision 1.55  1999/11/02 15:59:34  khz
      import new TOX_CONTENT and TOX_INDEX features (2)

      Revision 1.54  1999/10/29 12:07:35  khz
      import new TOX_CONTENT and TOX_INDEX features

      Revision 1.53  1999/10/08 09:25:39  khz
      Import Redlining

      Revision 1.52  1999/08/30 19:52:58  JP
      Bug #68219#: no static members - be reentrant


      Rev 1.51   30 Aug 1999 21:52:58   JP
   Bug #68219#: no static members - be reentrant

      Rev 1.50   12 Aug 1999 13:57:52   KHZ
   Task #67668# TestApo() modified

      Rev 1.49   09 Aug 1999 14:16:08   JP
   read W95 format from stream

      Rev 1.48   12 Jul 1999 16:58:20   KHZ
   Task #66194# 1. Sections with cols, 2. App.Node before PgBreak if Fly in node

      Rev 1.47   05 Jul 1999 19:30:52   KHZ
   Task #66194# Sections with columns

      Rev 1.46   21 Jun 1999 10:24:40   KHZ
   Reference field (page, bookmark, footnote) part#2

      Rev 1.45   18 Jun 1999 15:53:50   KHZ
   Reference field (page, bookmark, footnote) part#1

      Rev 1.44   10 Jun 1999 17:58:52   KHZ
   use FOUR different inner distances with SvxBoxItem

      Rev 1.43   09 Jun 1999 18:25:00   KHZ
   use FOUR different inner distances with SvxBoxItem

      Rev 1.42   26 Apr 1999 17:40:20   KHZ
   Task #61381# Attribute von Textboxen in denen NUR EINE Grafik steht

      Rev 1.41   09 Apr 1999 18:01:38   KHZ
   Task #64348# Bei Listen-Overrides nur bei Ungleichheit neue Liste anlegen!

      Rev 1.40   18 Mar 1999 09:56:52   JP
   Task #63049#: Numerierung mit rel. Einzuegen

      Rev 1.39   10 Mar 1999 15:11:10   KHZ
   Task #62521# Einlesen von Sonderzeichen

      Rev 1.38   08 Mar 1999 11:51:30   KHZ
   Task #57749# Writer kann jetzt spaltige Bereiche

      Rev 1.37   26 Feb 1999 14:43:38   KHZ
   Task #59715# Behandlung von Section breaks

      Rev 1.36   17 Feb 1999 15:23:42   KHZ
   Task #61382# jetzt auch Textbox-Grafiken im Hintergrund ( SvxOpaqItem(FALSE) )

*************************************************************************/

#endif

