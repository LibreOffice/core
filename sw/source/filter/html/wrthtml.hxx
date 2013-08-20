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
#ifndef _WRTHTML_HXX
#define _WRTHTML_HXX

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/ptr_container/ptr_set.hpp>
#include <vector>

#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <vcl/field.hxx>
#include <i18nlangtag/lang.h>
#include <tools/stream.hxx>
#include <o3tl/sorted_vector.hxx>

#include "shellio.hxx"
#include "wrt_fn.hxx"

// einige Forward Deklarationen
class Color;
class SwFrmFmt;
class SwFlyFrmFmt;
class SwDrawFrmFmt;
class SwFmtINetFmt;
class SwFmtVertOrient;
class SwFmtFtn;
class SwStartNode;
class SwTableNode;
class SwPageDesc;
class SwNodeIndex;
class ImageMap;
class SwNumRule;
class SdrObject;
class SvxBrushItem;
class SvxFontItem;
class SwHTMLNumRuleInfo;
class SwHTMLPosFlyFrms;
class SwTxtFtn;

typedef std::vector<SwTxtFtn *> SwHTMLTxtFtns;

extern SwAttrFnTab aHTMLAttrFnTab;

#define HTML_PARSPACE (MM50)

// Flags fuer die Ausgabe von Rahmen aller Art
// BORDER geht nur bei OutHTML_Image
// ANYSIZE gibt an, ob auch VAR_SIZE und MIN_SIZE angaben exportiert werden
// ABSSIZE gibt an, ob Abstand und Umrandung ignoriert werden sollen
const sal_uInt32 HTML_FRMOPT_ALIGN      = 1<<0;
const sal_uInt32 HTML_FRMOPT_S_ALIGN    = 1<<1;

const sal_uInt32 HTML_FRMOPT_WIDTH      = 1<<2;
const sal_uInt32 HTML_FRMOPT_HEIGHT         = 1<<3;
const sal_uInt32 HTML_FRMOPT_SIZE       = HTML_FRMOPT_WIDTH|HTML_FRMOPT_HEIGHT;
const sal_uInt32 HTML_FRMOPT_S_WIDTH    = 1<<4;
const sal_uInt32 HTML_FRMOPT_S_HEIGHT   = 1<<5;
const sal_uInt32 HTML_FRMOPT_S_SIZE     = HTML_FRMOPT_S_WIDTH|HTML_FRMOPT_S_HEIGHT;
const sal_uInt32 HTML_FRMOPT_ANYSIZE    = 1<<6;
const sal_uInt32 HTML_FRMOPT_ABSSIZE    = 1<<7;
const sal_uInt32 HTML_FRMOPT_MARGINSIZE     = 1<<8;

const sal_uInt32 HTML_FRMOPT_SPACE      = 1<<9;
const sal_uInt32 HTML_FRMOPT_S_SPACE    = 1<<10;

const sal_uInt32 HTML_FRMOPT_BORDER     = 1<<11;
const sal_uInt32 HTML_FRMOPT_S_BORDER   = 1<<12;
const sal_uInt32 HTML_FRMOPT_S_NOBORDER     = 1<<13;

const sal_uInt32 HTML_FRMOPT_S_BACKGROUND = 1<<14;

const sal_uInt32 HTML_FRMOPT_NAME           = 1<<15;
const sal_uInt32 HTML_FRMOPT_ALT        = 1<<16;
const sal_uInt32 HTML_FRMOPT_BRCLEAR    = 1<<17;
const sal_uInt32 HTML_FRMOPT_S_PIXSIZE  = 1<<18;
const sal_uInt32 HTML_FRMOPT_ID             = 1<<19;
const sal_uInt32 HTML_FRMOPT_DIR            = 1<<20;


const sal_uInt32 HTML_FRMOPTS_GENIMG_ALL    =
    HTML_FRMOPT_ALT     |
    HTML_FRMOPT_SIZE    |
    HTML_FRMOPT_ABSSIZE |
    HTML_FRMOPT_NAME;
const sal_uInt32 HTML_FRMOPTS_GENIMG_CNTNR = HTML_FRMOPTS_GENIMG_ALL;
const sal_uInt32 HTML_FRMOPTS_GENIMG    =
    HTML_FRMOPTS_GENIMG_ALL |
    HTML_FRMOPT_ALIGN       |
    HTML_FRMOPT_SPACE       |
    HTML_FRMOPT_BRCLEAR;

#define HTMLMODE_BLOCK_SPACER       0x00010000
#define HTMLMODE_FLOAT_FRAME        0x00020000
#define HTMLMODE_VERT_SPACER        0x00040000
#define HTMLMODE_NBSP_IN_TABLES     0x00080000
#define HTMLMODE_LSPACE_IN_NUMBUL   0x00100000
#define HTMLMODE_NO_BR_AT_PAREND    0x00200000
#define HTMLMODE_PRINT_EXT          0x00400000
#define HTMLMODE_ABS_POS_FLY        0x00800000
#define HTMLMODE_ABS_POS_DRAW       0x01000000
#define HTMLMODE_FLY_MARGINS        0x02000000
#define HTMLMODE_BORDER_NONE        0x04000000
#define HTMLMODE_FONT_GENERIC       0x08000000
#define HTMLMODE_FRSTLINE_IN_NUMBUL 0x10000000
#define HTMLMODE_NO_CONTROL_CENTERING 0x20000000

#define HTML_DLCOLL_DD 0x4000
#define HTML_DLCOLL_DT 0x8000

#define CSS1_FMT_ISTAG (USHRT_MAX)
#define CSS1_FMT_CMPREF (USHRT_MAX-1)
#define CSS1_FMT_SPECIAL (USHRT_MAX-1)

// Die folgenden Flags bestimmen nur, welche Descriptoren, Tags, Optionen etc.
// ausgegeben werden ...
// bit 0,1,2
#define CSS1_OUTMODE_SPAN_NO_ON     0x0000U
#define CSS1_OUTMODE_SPAN_TAG_ON    0x0001U
#define CSS1_OUTMODE_STYLE_OPT_ON   0x0002U
#define CSS1_OUTMODE_RULE_ON        0x0003U
#define CSS1_OUTMODE_SPAN_TAG1_ON   0x0004U
#define CSS1_OUTMODE_ANY_ON         0x0007U

// bit 3,4,5
#define CSS1_OUTMODE_SPAN_NO_OFF    0x0000U
#define CSS1_OUTMODE_SPAN_TAG_OFF   ((sal_uInt16)(0x0001U << 3))
#define CSS1_OUTMODE_STYLE_OPT_OFF  ((sal_uInt16)(0x0002U << 3))
#define CSS1_OUTMODE_RULE_OFF       ((sal_uInt16)(0x0003U << 3))
#define CSS1_OUTMODE_SPAN_TAG1_OFF  ((sal_uInt16)(0x0004U << 3))
#define CSS1_OUTMODE_ANY_OFF        ((sal_uInt16)(0x0007U << 3))

#define CSS1_OUTMODE_ONOFF(a) (CSS1_OUTMODE_##a##_ON|CSS1_OUTMODE_##a##_OFF)
#define CSS1_OUTMODE_SPAN_TAG       CSS1_OUTMODE_ONOFF(SPAN_TAG)
#define CSS1_OUTMODE_STYLE_OPT      CSS1_OUTMODE_ONOFF(STYLE_OPT)
#define CSS1_OUTMODE_RULE           CSS1_OUTMODE_ONOFF(RULE)
#define CSS1_OUTMODE_SPAN_TAG1      CSS1_OUTMODE_ONOFF(TAG1)

// Die folgenden Flags legen fest, was ausgegeben wird
// bit 6,7,8,9
#define CSS1_OUTMODE_TEMPLATE       0x0000U
#define CSS1_OUTMODE_BODY           ((sal_uInt16)(0x0001U << 6))
#define CSS1_OUTMODE_PARA           ((sal_uInt16)(0x0002U << 6))
#define CSS1_OUTMODE_HINT           ((sal_uInt16)(0x0003U << 6))
#define CSS1_OUTMODE_FRAME          ((sal_uInt16)(0x0004U << 6))
#define CSS1_OUTMODE_TABLE          ((sal_uInt16)(0x0005U << 6))
#define CSS1_OUTMODE_TABLEBOX       ((sal_uInt16)(0x0006U << 6))
#define CSS1_OUTMODE_DROPCAP        ((sal_uInt16)(0x0007U << 6))
#define CSS1_OUTMODE_SECTION        ((sal_uInt16)(0x0008U << 6))
#define CSS1_OUTMODE_SOURCE         ((sal_uInt16)(0x000fU << 6))

// bit 10
#define CSS1_OUTMODE_ENCODE         ((sal_uInt16)(0x0001U << 10))

// bit 11,12,13
// don't care about script
#define CSS1_OUTMODE_ANY_SCRIPT     0x0000U
// no cjk or ctl items
#define CSS1_OUTMODE_WESTERN        ((sal_uInt16)(0x0001U << 11))
// no western or ctl items
#define CSS1_OUTMODE_CJK            ((sal_uInt16)(0x0002U << 11))
// no western or cjk items
#define CSS1_OUTMODE_CTL            ((sal_uInt16)(0x0003U << 11))
// no western, cjk or ctl items
#define CSS1_OUTMODE_NO_SCRIPT      ((sal_uInt16)(0x0004U << 11))
#define CSS1_OUTMODE_SCRIPT         ((sal_uInt16)(0x0007U << 11))

// der HTML-Writer
struct HTMLControl
{
    // die Form, zu der das Control gehoert
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > xFormComps;
    sal_uLong nNdIdx;                   // der Node, in dem es verankert ist
    xub_StrLen nCount;              // wie viele Controls sind in dem Node

    HTMLControl( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > & rForm,
                 sal_uInt32 nIdx );
    ~HTMLControl();

    // operatoren fuer das Sort-Array
    bool operator==( const HTMLControl& rCtrl ) const
    {
        return nNdIdx == rCtrl.nNdIdx;
    }
    bool operator<( const HTMLControl& rCtrl ) const
    {
        return nNdIdx < rCtrl.nNdIdx;
    }
};

class HTMLControls : public o3tl::sorted_vector<HTMLControl*, o3tl::less_ptr_to<HTMLControl> > {
public:
    // will free any items still in the vector
    ~HTMLControls() { DeleteAndDestroyAll(); }
};


typedef std::vector<SwFmtINetFmt*> INetFmts;

struct SwHTMLFmtInfo
{
    const SwFmt *pFmt;      // das Format selbst
    const SwFmt *pRefFmt;   // das Vergleichs-Format

    OString aToken;          // das auszugebende Token
    OUString aClass;          // die auszugebende Klasse

    SfxItemSet *pItemSet;   // der auszugebende Attribut-Set

    sal_Int32 nLeftMargin;      // ein par default-Werte fuer
    sal_Int32 nRightMargin; // Absatz-Vorlagen
    short nFirstLineIndent;

    sal_uInt16 nTopMargin;
    sal_uInt16 nBottomMargin;

    bool bScriptDependent;

    // Konstruktor fuer einen Dummy zum Suchen
    SwHTMLFmtInfo( const SwFmt *pF ) :
        pFmt( pF ), pRefFmt(0), pItemSet( 0 ), nFirstLineIndent(0), bScriptDependent(false)
    {}


    // Konstruktor zum Erstellen der Format-Info
    SwHTMLFmtInfo( const SwFmt *pFmt, SwDoc *pDoc, SwDoc *pTemlate,
                   sal_Bool bOutStyles, LanguageType eDfltLang=LANGUAGE_DONTKNOW,
                   sal_uInt16 nScript=CSS1_OUTMODE_ANY_SCRIPT,
                   sal_Bool bHardDrop=sal_False );
    ~SwHTMLFmtInfo();

    friend bool operator==( const SwHTMLFmtInfo& rInfo1,
                            const SwHTMLFmtInfo& rInfo2 )
    {
        return (sal_IntPtr)rInfo1.pFmt == (sal_IntPtr)rInfo2.pFmt;
    }

    friend bool operator<( const SwHTMLFmtInfo& rInfo1,
                            const SwHTMLFmtInfo& rInfo2 )
    {
        return (sal_IntPtr)rInfo1.pFmt < (sal_IntPtr)rInfo2.pFmt;
    }

};

typedef boost::ptr_set<SwHTMLFmtInfo> SwHTMLFmtInfos;

class IDocumentStylePoolAccess;

class SwHTMLWriter : public Writer
{
    SwHTMLPosFlyFrms *pHTMLPosFlyFrms;
    SwHTMLNumRuleInfo *pNumRuleInfo;// aktuelle Numerierung
    SwHTMLNumRuleInfo *pNextNumRuleInfo;
    sal_uInt32 nHTMLMode;               // Beschreibung der Export-Konfiguration

    FieldUnit eCSS1Unit;

    sal_uInt16 OutHeaderAttrs();
    const SwPageDesc *MakeHeader( sal_uInt16& rHeaderAtrs );
    void GetControls();

    void AddLinkTarget( const String& rURL );
    void CollectLinkTargets();

protected:
    sal_uLong WriteStream();

public:
#if defined(UNX)
    static const sal_Char sNewLine;     // nur \012 oder \015
#else
    static const sal_Char sNewLine[];   // \015\012
#endif

    std::vector<String> aImgMapNames;     // geschriebene Image Maps
    std::set<String> aImplicitMarks;// implizite Stprungmarken
    std::set<String> aNumRuleNames;// Names of exported num rules
    std::set<String> aScriptParaStyles;// script dependent para styles
    std::set<OUString> aScriptTextStyles;// script dependent text styles
    boost::ptr_vector<String> aOutlineMarks;
    std::vector<sal_uInt32> aOutlineMarkPoss;
    HTMLControls aHTMLControls;     // die zu schreibenden ::com::sun::star::form::Forms
    SwHTMLFmtInfos aChrFmtInfos;
    SwHTMLFmtInfos aTxtCollInfos;
    INetFmts aINetFmts;             // die "offenen" INet-Attribute
    SwHTMLTxtFtns *pFootEndNotes;

    String aCSS1Selector;           // der Selektor eines Styles
    OUString aNonConvertableCharacters;
    String aBulletGrfs[MAXLEVEL];   // die Grafiken fuer Listen

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >  *pxFormComps; // die aktuelle Form

    SwDoc *pTemplate;               // die HTML-Vorlage
    Color *pDfltColor;              // default Farbe
    SwNodeIndex *pStartNdIdx;       // Index des ersten Absatz
    const SwPageDesc *pCurrPageDesc;// Die aktuelle Seiten-Vorlage
    const SwFmtFtn *pFmtFtn;

    sal_uInt32 aFontHeights[7];         // die Font-Hoehen 1-7

    sal_uInt32 nWarn;                   // Result-Code fuer Warnungen
    sal_uInt32 nLastLFPos;              // letzte Position eines LF

    sal_uInt16 nLastParaToken;          // fuers Absaetze zusammenhalten
    sal_Int32 nBkmkTabPos;              // akt. Position in der Bookmark-Tabelle
    sal_uInt16 nImgMapCnt;              // zum eindeutig
    sal_uInt16 nFormCntrlCnt;
    sal_uInt16 nEndNote;
    sal_uInt16 nFootNote;
    sal_Int32 nLeftMargin;              // linker Einzug (z.B. aus Listen)
    sal_Int32 nDfltLeftMargin;          // die dafaults, der nicht geschrieben
    sal_Int32 nDfltRightMargin;     // werden muessen (aus der Vorlage)
    short  nFirstLineIndent;        // Erstzeilen-Einzug (aus Listen)
    short  nDfltFirstLineIndent;    // nicht zu schreibender default
    sal_uInt16 nDfltTopMargin;          // die defaults, der nicht geschrieben
    sal_uInt16 nDfltBottomMargin;       // werden muessen (aus der Vorlage)
    sal_uInt16 nIndentLvl;              // wie weit ist eingerueckt?
    xub_StrLen nWhishLineLen;           // wie lang darf eine Zeile werden?
    sal_uInt16 nDefListLvl;             // welcher DL-Level existiert gerade
    sal_Int32  nDefListMargin;          // Wie weit wird in DL eingerueckt
    sal_uInt16 nHeaderFooterSpace;
    sal_uInt16 nTxtAttrsToIgnore;
    sal_uInt16 nExportMode;
    sal_uInt16 nCSS1OutMode;
    sal_uInt16 nCSS1Script;         // contains default script (that's the one
                                    // that is not contained in class names)
    sal_uInt16 nDirection;          // the current direction

    rtl_TextEncoding    eDestEnc;
    LanguageType        eLang;

    // Beschreibung der Export-Konfiguration
    // 0
    sal_Bool bCfgOutStyles : 1;         // Styles exportieren
    sal_Bool bCfgPreferStyles : 1;      // Styles herkoemmlichen Tags vorziehen
    sal_Bool bCfgFormFeed : 1;          // Form-Feeds exportieren
    sal_Bool bCfgStarBasic : 1;         // StarBasic exportieren
    sal_Bool bCfgCpyLinkedGrfs : 1;

    // Beschreibung dessen, was exportiert wird

    sal_Bool bFirstLine : 1;            // wird die 1. Zeile ausgegeben ?
    sal_Bool bTagOn : 1;                // Tag an oder aus/Attr-Start oder -Ende

    // Die folgenden beiden Flags geben an, wir Attribute exportiert werden:
    // bTxtAttr bOutOpts
    // 0        0           Style-Sheets
    // 1        0           Hints: Jedes Attribut wird als eignes Tag
    //                          geschrieben und es gibt ein End-Tag
    // 0        1           (Absatz-)Attribute: Das Attribut wird als Option
    //                          eines bereits geschrieben Tags exportiert. Es
    //                          gibt kein End-Tag.
    sal_Bool bTxtAttr : 1;
    // 8
    sal_Bool bOutOpts : 1;

    sal_Bool bOutTable : 1;             // wird der Tabelleninhalt geschrieben?
    sal_Bool bOutHeader : 1;
    sal_Bool bOutFooter : 1;
    sal_Bool bOutFlyFrame : 1;

    // Flags fuer Style-Export

    sal_Bool bFirstCSS1Rule : 1;        // wurde schon eine Property ausgegeben
    sal_Bool bFirstCSS1Property : 1;    // wurde schon eine Property ausgegeben
    sal_Bool bPoolCollTextModified : 1; // die Textkoerper-Vorlage wurde
                                    // modifiziert.
    // 16
    sal_Bool bCSS1IgnoreFirstPageDesc : 1;

    // was muss/kann/darf nicht ausgegeben werden?

    sal_Bool bNoAlign : 1;              // HTML-Tag erlaubt kein ALIGN=...
    sal_Bool bClearLeft : 1;            // <BR CLEAR=LEFT> am Absatz-Ende ausg.
    sal_Bool bClearRight : 1;           // <BR CLEAR=RIGHT> am Absatz-Ende ausg.
    sal_Bool bLFPossible : 1;           // ein Zeilenumbruch darf eingef. werden

    // sonstiges

    sal_Bool bPreserveForm : 1;         // die aktuelle Form beibehalten

    sal_Bool bCfgNetscape4 : 1;         // Netscape4 Hacks
    // 23

    SwHTMLWriter( const String& rBaseURL );
    virtual ~SwHTMLWriter();

    void Out_SwDoc( SwPaM* );       // schreibe den makierten Bereich

    // gebe alle an in aktuellen Ansatz stehenden ::com::sun::star::text::Bookmarks aus
    void OutAnchor( const String& rName );
    void OutBookmarks();
    void OutImplicitMark( const String& rMark, const sal_Char *pMarkType );
    void OutHyperlinkHRefValue( const String& rURL );

    // gebe die evt. an der akt. Position stehenden FlyFrame aus.
    sal_Bool OutFlyFrm( sal_uLong nNdIdx, xub_StrLen nCntntIdx,
                        sal_uInt8 nPos, HTMLOutContext *pContext = 0 );
    void OutFrmFmt( sal_uInt8 nType, const SwFrmFmt& rFmt,
                    const SdrObject *pSdrObj );

    void OutForm( sal_Bool bTagOn=sal_True, const SwStartNode *pStNd=0 );
    void OutHiddenForms();
    void OutHiddenForm( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::form::XForm > & rForm );

    void OutForm( sal_Bool bOn, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > & rFormComps );
    void OutHiddenControls( const ::com::sun::star::uno::Reference<
                ::com::sun::star::container::XIndexContainer > & rFormComps,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > & rPropSet );
    sal_Bool HasControls() const;

    void OutFootEndNoteInfo();
    void OutFootEndNotes();
    String GetFootEndNoteSym( const SwFmtFtn& rFmtFtn );
    void OutFootEndNoteSym( const SwFmtFtn& rFmtFtn, const String& rNum,
                             sal_uInt16 nScript );

    void OutBasic();

    void OutAndSetDefList( sal_uInt16 nNewLvl );

    void OutStyleSheet( const SwPageDesc& rPageDesc, sal_Bool bUsed=sal_True );

    inline void OutCSS1_PropertyAscii( const sal_Char *pProp,
                                       const sal_Char *pVal );
    inline void OutCSS1_PropertyAscii( const sal_Char *pProp,
                                       const OString& rVal );
    inline void OutCSS1_Property( const sal_Char *pProp, const OUString& rVal );
    void OutCSS1_Property( const sal_Char *pProp, const sal_Char *pVal,
                           const OUString *pSVal );
    void OutCSS1_UnitProperty( const sal_Char *pProp, long nVal );
    void OutCSS1_PixelProperty( const sal_Char *pProp, long nVal, sal_Bool bVert );
    void OutCSS1_SfxItemSet( const SfxItemSet& rItemSet, sal_Bool bDeep=sal_True );

    // BODY-Tag-Events aus der SFX-Konfigaurion
    void OutBasicBodyEvents();

    // BACKGROUND/BGCOLOR-Option
    void OutBackground( const SvxBrushItem *pBrushItem, sal_Bool bGraphic );
    void OutBackground( const SfxItemSet& rItemSet, sal_Bool bGraphic );

    void OutLanguage( LanguageType eLang );
    sal_uInt16 GetHTMLDirection( sal_uInt16 nDir ) const;
    sal_uInt16 GetHTMLDirection( const SfxItemSet& rItemSet ) const;
    void OutDirection( sal_uInt16 nDir );

    // ALT/ALIGN/WIDTH/HEIGHT/HSPACE/VSPACE-Optionen des aktuellen
    // Frame-Formats ausgeben und ggf. ein <BR CLEAR=...> vorne an
    // rEndTags anhaengen
    OString OutFrmFmtOptions( const SwFrmFmt& rFrmFmt, const String& rAltTxt,
        sal_uInt32 nFrmOpts, const OString& rEndTags = OString() );
    void OutCSS1_TableFrmFmtOptions( const SwFrmFmt& rFrmFmt );
    void OutCSS1_TableCellBorderHack(const SwFrmFmt& rFrmFmt);
    void OutCSS1_SectionFmtOptions( const SwFrmFmt& rFrmFmt, const SwFmtCol *pCol );
    void OutCSS1_FrmFmtOptions( const SwFrmFmt& rFrmFmt, sal_uInt32 nFrmOpts,
                                const SdrObject *pSdrObj=0,
                                const SfxItemSet *pItemSet=0 );
    void OutCSS1_FrmFmtBackground( const SwFrmFmt& rFrmFmt );

    void ChangeParaToken( sal_uInt16 nNew );

    void IncIndentLevel() { nIndentLvl++; }
    void DecIndentLevel() { if ( nIndentLvl ) nIndentLvl--; }
    OString GetIndentString(sal_uInt16 nIncLvl = 0);

    xub_StrLen GetLineLen() { return (xub_StrLen)(Strm().Tell()-nLastLFPos); }
    void OutNewLine( sal_Bool bCheck=sal_False );

    // fuer HTMLSaveData
    SwPaM* GetEndPaM() { return pOrigPam; }
    void SetEndPaM( SwPaM* pPam ) { pOrigPam = pPam; }

    sal_uInt32 ToPixel( sal_uInt32 nVal ) const;

    sal_uInt16 GuessFrmType( const SwFrmFmt& rFrmFmt,
                         const SdrObject*& rpStrObj );
    sal_uInt16 GuessOLENodeFrmType( const SwNode& rNd );

    void CollectFlyFrms();

    sal_uInt16 GetHTMLFontSize( sal_uInt32 nFontHeight ) const;

    // Die aktuelle Numerierungs-Information holen.
    SwHTMLNumRuleInfo& GetNumInfo() { return *pNumRuleInfo; }

    // Die Numerierungs-Information des naechsten Absatz holen. Sie
    // muss noch nicht vorhanden sein!
    SwHTMLNumRuleInfo *GetNextNumInfo() { return pNextNumRuleInfo; }

    // Die Numerierungs-Information des naechsten Absatz setzen.
    void SetNextNumInfo( SwHTMLNumRuleInfo *pNxt ) { pNextNumRuleInfo=pNxt; }

    // Die Numerierungs-Information des naeschten Absatz fuellen.
    void FillNextNumInfo();

    // Die Numerierungs-Information des naeschten Absatz loeschen.
    void ClearNextNumInfo();

    static const SdrObject *GetHTMLControl( const SwDrawFrmFmt& rFmt );
    static const SdrObject *GetMarqueeTextObj( const SwDrawFrmFmt& rFmt );
    static sal_uInt16 GetCSS1Selector( const SwFmt *pFmt, OString& rToken,
                                   OUString& rClass, sal_uInt16& rRefPoolId,
                                   OUString *pPseudo=0 );

    static const SwFmt *GetTemplateFmt( sal_uInt16 nPoolId, IDocumentStylePoolAccess* /*SwDoc*/ pTemplate );
    static const SwFmt *GetParentFmt( const SwFmt& rFmt, sal_uInt16 nDeep );

    static void SubtractItemSet( SfxItemSet& rItemSet,
                                 const SfxItemSet& rRefItemSet,
                                 sal_Bool bSetDefaults,
                                 sal_Bool bClearSame = sal_True,
                                   const SfxItemSet *pRefScriptItemSet=0 );
    static sal_Bool HasScriptDependentItems( const SfxItemSet& rItemSet,
                                               sal_Bool bCheckDropCap );

    static void GetEEAttrsFromDrwObj( SfxItemSet& rItemSet,
                                      const SdrObject *pObj,
                                      sal_Bool bSetDefaults );

    static sal_uInt16 GetDefListLvl( const OUString& rNm, sal_uInt16 nPoolId );

    sal_uInt32 GetHTMLMode() const { return nHTMLMode; }
    sal_Bool IsHTMLMode( sal_uInt32 nMode ) const { return (nHTMLMode & nMode) != 0; }

    inline bool IsCSS1Source( sal_uInt16 n ) const;
    inline bool IsCSS1Script( sal_uInt16 n ) const;

    static const sal_Char *GetNumFormat( sal_uInt16 nFmt );
    static void PrepareFontList( const SvxFontItem& rFontItem, OUString& rNames,
                                 sal_Unicode cQuote, sal_Bool bGeneric );
    static sal_uInt16 GetCSS1ScriptForScriptType( sal_uInt16 nScriptType );
    static sal_uInt16 GetLangWhichIdFromScript( sal_uInt16 nScript );

    FieldUnit GetCSS1Unit() const { return eCSS1Unit; }
};

inline bool SwHTMLWriter::IsCSS1Source( sal_uInt16 n ) const
{
    return n == (nCSS1OutMode & CSS1_OUTMODE_SOURCE);
}

inline bool SwHTMLWriter::IsCSS1Script( sal_uInt16 n ) const
{
    sal_uInt16 nScript = (nCSS1OutMode & CSS1_OUTMODE_SCRIPT);
    return CSS1_OUTMODE_ANY_SCRIPT == nScript || n == nScript;
}

inline void SwHTMLWriter::OutCSS1_PropertyAscii( const sal_Char *pProp,
                                                 const sal_Char *pVal )
{
    OutCSS1_Property( pProp, pVal, 0 );
}

inline void SwHTMLWriter::OutCSS1_PropertyAscii( const sal_Char *pProp,
                                                 const OString& rVal )
{
    OutCSS1_Property( pProp, rVal.getStr(), 0 );
}

inline void SwHTMLWriter::OutCSS1_Property( const sal_Char *pProp,
                                            const OUString& rVal )
{
    OutCSS1_Property( pProp, 0, &rVal );
}

// Struktur speichert die aktuellen Daten des Writers zwischen, um
// einen anderen Dokument-Teil auszugeben, wie z.B. Header/Footer
// Mit den beiden USHORTs im CTOR wird ein neuer PaM erzeugt und auf
// die Position im Dokument gesetzt.
// Im Destructor werden alle Daten wieder restauriert und der angelegte
// Pam wieder geloescht.

struct HTMLSaveData
{
    SwHTMLWriter& rWrt;
    SwPaM* pOldPam, *pOldEnd;
    SwHTMLNumRuleInfo *pOldNumRuleInfo;     // Owner = this
    SwHTMLNumRuleInfo *pOldNextNumRuleInfo; // Owner = HTML-Writer
    sal_uInt16 nOldDefListLvl;
    sal_uInt16 nOldDirection;
    sal_Bool bOldWriteAll : 1;
    sal_Bool bOldOutHeader : 1;
    sal_Bool bOldOutFooter : 1;
    sal_Bool bOldOutFlyFrame : 1;
    const SwFlyFrmFmt* pOldFlyFmt;

    HTMLSaveData( SwHTMLWriter&, sal_uLong nStt, sal_uLong nEnd,
                  sal_Bool bSaveNum=sal_True,
                     const SwFrmFmt *pFrmFmt=0  );
    ~HTMLSaveData();
};


// einige Funktions-Deklarationen
Writer& OutHTML_FrmFmtOLENode( Writer& rWrt, const SwFrmFmt& rFmt,
                               sal_Bool bInCntnr );
Writer& OutHTML_FrmFmtOLENodeGrf( Writer& rWrt, const SwFrmFmt& rFmt,
                                  sal_Bool bInCntnr );

Writer& OutHTML_SwTxtNode( Writer&, const SwCntntNode& );
Writer& OutHTML_SwTblNode( Writer& , SwTableNode &, const SwFrmFmt *,
                           const OUString* pCaption=0, sal_Bool bTopCaption=sal_False );

Writer& OutHTML_DrawFrmFmtAsControl( Writer& rWrt, const SwDrawFrmFmt& rFmt,
                                     const SdrObject& rSdrObj, sal_Bool bInCntnr );
Writer& OutHTML_DrawFrmFmtAsMarquee( Writer& rWrt, const SwDrawFrmFmt& rFmt,
                                     const SdrObject& rSdrObj );

Writer& OutHTML_HeaderFooter( Writer& rWrt, const SwFrmFmt& rFrmFmt,
                              sal_Bool bHeader );

Writer& OutHTML_Image( Writer&, const SwFrmFmt& rFmt,
                       Graphic& rGraphic, const OUString& rAlternateTxt,
                       const Size& rRealSize, sal_uInt32 nFrmOpts,
                       const sal_Char *pMarkType = 0,
                       const ImageMap *pGenImgMap = 0 );

Writer& OutHTML_BulletImage( Writer& rWrt, const sal_Char *pTag,
                             const SvxBrushItem* pBrush,
                             const Size &rSize,
                             const SwFmtVertOrient* pVertOrient );

Writer& OutHTML_SwFmtFld( Writer& rWrt, const SfxPoolItem& rHt );
Writer& OutHTML_SwFmtFtn( Writer& rWrt, const SfxPoolItem& rHt );
Writer& OutHTML_INetFmt( Writer&, const SwFmtINetFmt& rINetFmt, sal_Bool bOn );

Writer& OutCSS1_BodyTagStyleOpt( Writer& rWrt, const SfxItemSet& rItemSet );
Writer& OutCSS1_ParaTagStyleOpt( Writer& rWrt, const SfxItemSet& rItemSet );

Writer& OutCSS1_HintSpanTag( Writer& rWrt, const SfxPoolItem& rHt );
Writer& OutCSS1_HintStyleOpt( Writer& rWrt, const SfxPoolItem& rHt );

Writer& OutCSS1_TableBGStyleOpt( Writer& rWrt, const SfxPoolItem& rHt );
Writer& OutCSS1_NumBulListStyleOpt( Writer& rWrt, const SwNumRule& rNumRule,
                                    sal_uInt8 nLevel );

Writer& OutHTML_NumBulListStart( SwHTMLWriter& rWrt,
                                 const SwHTMLNumRuleInfo& rInfo );
Writer& OutHTML_NumBulListEnd( SwHTMLWriter& rWrt,
                               const SwHTMLNumRuleInfo& rNextInfo );

Writer& OutCSS1_SvxBox( Writer& rWrt, const SfxPoolItem& rHt );

#endif  //  _WRTHTML_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
