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
#ifndef INCLUDED_SW_SOURCE_FILTER_HTML_WRTHTML_HXX
#define INCLUDED_SW_SOURCE_FILTER_HTML_WRTHTML_HXX

#include <memory>
#include <vector>
#include <set>

#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <vcl/field.hxx>
#include <i18nlangtag/lang.h>
#include <comphelper/stl_types.hxx>
#include <o3tl/sorted_vector.hxx>

#include "shellio.hxx"
#include "wrt_fn.hxx"

// einige Forward Deklarationen
class Color;
class SwFrameFormat;
class SwFlyFrameFormat;
class SwDrawFrameFormat;
class SwFormatINetFormat;
class SwFormatVertOrient;
class SwFormatFootnote;
class SwStartNode;
class SwTableNode;
class SwPageDesc;
class SwNodeIndex;
class ImageMap;
class SwNumRule;
class SdrObject;
class SdrUnoObj;
class SvxBrushItem;
class SvxFontItem;
class SwHTMLNumRuleInfo;
class SwHTMLPosFlyFrms;
class SwTextFootnote;

typedef std::vector<SwTextFootnote*> SwHTMLTextFootnotes;

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
    css::uno::Reference<css::container::XIndexContainer> xFormComps;
    sal_uLong nNdIdx;                   // der Node, in dem es verankert ist
    sal_Int32 nCount;              // wie viele Controls sind in dem Node

    HTMLControl( const css::uno::Reference<css::container::XIndexContainer>& rForm, sal_uInt32 nIdx );
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

struct SwHTMLFormatInfo
{
    const SwFormat *pFormat;      // das Format selbst
    const SwFormat *pRefFormat;   // das Vergleichs-Format

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
    explicit SwHTMLFormatInfo( const SwFormat *pF ) :
        pFormat( pF ),
        pRefFormat(nullptr),
        pItemSet( nullptr ),
        nLeftMargin( 0 ),
        nRightMargin( 0 ),
        nFirstLineIndent(0),
        nTopMargin( 0 ),
        nBottomMargin( 0 ),
        bScriptDependent(false)
    {}

    // Konstruktor zum Erstellen der Format-Info
    SwHTMLFormatInfo( const SwFormat *pFormat, SwDoc *pDoc, SwDoc *pTemlate,
                   bool bOutStyles, LanguageType eDfltLang=LANGUAGE_DONTKNOW,
                   sal_uInt16 nScript=CSS1_OUTMODE_ANY_SCRIPT,
                   bool bHardDrop=false );
    ~SwHTMLFormatInfo();

    friend bool operator<( const SwHTMLFormatInfo& rInfo1,
                            const SwHTMLFormatInfo& rInfo2 )
    {
        return reinterpret_cast<sal_IntPtr>(rInfo1.pFormat) < reinterpret_cast<sal_IntPtr>(rInfo2.pFormat);
    }

};

typedef std::set<std::unique_ptr<SwHTMLFormatInfo>,
        comphelper::UniquePtrValueLess<SwHTMLFormatInfo>> SwHTMLFormatInfos;

class IDocumentStylePoolAccess;

class SwHTMLWriter : public Writer
{
    SwHTMLPosFlyFrms *m_pHTMLPosFlyFrms;
    SwHTMLNumRuleInfo *m_pNumRuleInfo;// aktuelle Numerierung
    SwHTMLNumRuleInfo *m_pNextNumRuleInfo;
    sal_uInt32 m_nHTMLMode;               // Beschreibung der Export-Konfiguration

    FieldUnit m_eCSS1Unit;

    sal_uInt16 OutHeaderAttrs();
    const SwPageDesc *MakeHeader( sal_uInt16& rHeaderAtrs );
    void GetControls();

    void AddLinkTarget( const OUString& rURL );
    void CollectLinkTargets();

protected:
    sal_uLong WriteStream() override;
    void SetupFilterOptions(SfxMedium& rMedium) override;

public:
    std::vector<OUString> m_aImgMapNames;     // geschriebene Image Maps
    std::set<OUString> m_aImplicitMarks;// implizite Stprungmarken
    std::set<OUString> m_aNumRuleNames;// Names of exported num rules
    std::set<OUString> m_aScriptParaStyles;// script dependent para styles
    std::set<OUString> m_aScriptTextStyles;// script dependent text styles
    std::vector<OUString> m_aOutlineMarks;
    std::vector<sal_uInt32> m_aOutlineMarkPoss;
    HTMLControls m_aHTMLControls;     // die zu schreibenden Forms
    SwHTMLFormatInfos m_CharFormatInfos;
    SwHTMLFormatInfos m_TextCollInfos;
    std::vector<SwFormatINetFormat*> m_aINetFormats; // die "offenen" INet-Attribute
    SwHTMLTextFootnotes *m_pFootEndNotes;

    OUString m_aCSS1Selector;           // der Selektor eines Styles
    OUString m_aNonConvertableCharacters;
    OUString m_aBulletGrfs[MAXLEVEL];   // die Grafiken fuer Listen

    css::uno::Reference<css::container::XIndexContainer> mxFormComps; // die aktuelle Form

    SwDoc *m_pTemplate;               // die HTML-Vorlage
    Color *m_pDfltColor;              // default Farbe
    SwNodeIndex *m_pStartNdIdx;       // Index des ersten Absatz
    const SwPageDesc *m_pCurrPageDesc;// Die aktuelle Seiten-Vorlage
    const SwFormatFootnote *m_pFormatFootnote;

    sal_uInt32 m_aFontHeights[7];         // die Font-Hoehen 1-7

    sal_uInt32 m_nWarn;                   // Result-Code fuer Warnungen
    sal_uInt32 m_nLastLFPos;              // letzte Position eines LF

    sal_uInt16 m_nLastParaToken;          // fuers Absaetze zusammenhalten
    sal_Int32 m_nBkmkTabPos;              // akt. Position in der Bookmark-Tabelle
    sal_uInt16 m_nImgMapCnt;              // zum eindeutig
    sal_uInt16 m_nFormCntrlCnt;
    sal_uInt16 m_nEndNote;
    sal_uInt16 m_nFootNote;
    sal_Int32 m_nLeftMargin;              // linker Einzug (z.B. aus Listen)
    sal_Int32 m_nDfltLeftMargin;          // die defaults, der nicht geschrieben
    sal_Int32 m_nDfltRightMargin;     // werden muessen (aus der Vorlage)
    short  m_nFirstLineIndent;        // Erstzeilen-Einzug (aus Listen)
    short  m_nDfltFirstLineIndent;    // nicht zu schreibender default
    sal_uInt16 m_nDfltTopMargin;          // die defaults, der nicht geschrieben
    sal_uInt16 m_nDfltBottomMargin;       // werden muessen (aus der Vorlage)
    sal_uInt16 m_nIndentLvl;              // wie weit ist eingerueckt?
    sal_Int32 m_nWhishLineLen;           // wie lang darf eine Zeile werden?
    sal_uInt16 m_nDefListLvl;             // welcher DL-Level existiert gerade
    sal_Int32  m_nDefListMargin;          // Wie weit wird in DL eingerueckt
    sal_uInt16 m_nHeaderFooterSpace;
    sal_uInt16 m_nTextAttrsToIgnore;
    sal_uInt16 m_nExportMode;
    sal_uInt16 m_nCSS1OutMode;
    sal_uInt16 m_nCSS1Script;         // contains default script (that's the one
                                    // that is not contained in class names)
    sal_uInt16 m_nDirection;          // the current direction

    rtl_TextEncoding    m_eDestEnc;
    LanguageType        m_eLang;

    // Beschreibung der Export-Konfiguration
    // 0
    bool m_bCfgOutStyles : 1;         // Styles exportieren
    bool m_bCfgPreferStyles : 1;      // Styles herkoemmlichen Tags vorziehen
    bool m_bCfgFormFeed : 1;          // Form-Feeds exportieren
    bool m_bCfgStarBasic : 1;         // StarBasic exportieren
    bool m_bCfgCpyLinkedGrfs : 1;

    // Beschreibung dessen, was exportiert wird

    bool m_bFirstLine : 1;            // wird die 1. Zeile ausgegeben ?
    bool m_bTagOn : 1;                // Tag an oder aus/Attr-Start oder -Ende

    // Die folgenden beiden Flags geben an, wir Attribute exportiert werden:
    // bTextAttr bOutOpts
    // 0        0           Style-Sheets
    // 1        0           Hints: Jedes Attribut wird als eignes Tag
    //                          geschrieben und es gibt ein End-Tag
    // 0        1           (Absatz-)Attribute: Das Attribut wird als Option
    //                          eines bereits geschrieben Tags exportiert. Es
    //                          gibt kein End-Tag.
    bool m_bTextAttr : 1;
    // 8
    bool m_bOutOpts : 1;

    bool m_bOutTable : 1;             // wird der Tabelleninhalt geschrieben?
    bool m_bOutHeader : 1;
    bool m_bOutFooter : 1;
    bool m_bOutFlyFrame : 1;

    // Flags fuer Style-Export

    bool m_bFirstCSS1Rule : 1;        // wurde schon eine Property ausgegeben
    bool m_bFirstCSS1Property : 1;    // wurde schon eine Property ausgegeben
    bool m_bPoolCollTextModified : 1; // die Textkoerper-Vorlage wurde
                                    // modifiziert.
    // 16
    bool m_bCSS1IgnoreFirstPageDesc : 1;

    // was muss/kann/darf nicht ausgegeben werden?

    bool m_bNoAlign : 1;              // HTML-Tag erlaubt kein ALIGN=...
    bool m_bClearLeft : 1;            // <BR CLEAR=LEFT> am Absatz-Ende ausg.
    bool m_bClearRight : 1;           // <BR CLEAR=RIGHT> am Absatz-Ende ausg.
    bool m_bLFPossible : 1;           // ein Zeilenumbruch darf eingef. werden

    // sonstiges

    bool m_bPreserveForm : 1;         // die aktuelle Form beibehalten

    bool m_bCfgNetscape4 : 1;         // Netscape4 Hacks

    bool mbSkipImages : 1;
    /// If HTML header and footer should be written as well, or just the content itself.
    bool mbSkipHeaderFooter : 1;

#define sCSS2_P_CLASS_leaders "leaders"
    bool m_bCfgPrintLayout : 1;       // PrintLayout option for TOC dot leaders
    bool m_bParaDotLeaders : 1;       // for TOC dot leaders
    // 25

    explicit SwHTMLWriter( const OUString& rBaseURL );
    virtual ~SwHTMLWriter();

    void Out_SwDoc( SwPaM* );       // schreibe den makierten Bereich

    // gebe alle an in aktuellen Ansatz stehenden Bookmarks aus
    void OutAnchor( const OUString& rName );
    void OutBookmarks();
    void OutPointFieldmarks( const SwPosition& rPos );
    void OutImplicitMark( const OUString& rMark, const sal_Char *pMarkType );

    OUString convertHyperlinkHRefValue(const OUString& rURL);

    void OutHyperlinkHRefValue( const OUString& rURL );

    // gebe die evt. an der akt. Position stehenden FlyFrame aus.
    bool OutFlyFrm( sal_uLong nNdIdx, sal_Int32 nContentIdx,
                        sal_uInt8 nPos, HTMLOutContext *pContext = nullptr );
    void OutFrameFormat( sal_uInt8 nType, const SwFrameFormat& rFormat,
                    const SdrObject *pSdrObj );

    void OutForm( bool bTagOn=true, const SwStartNode *pStNd=nullptr );
    void OutHiddenForms();
    void OutHiddenForm( const css::uno::Reference<css::form::XForm>& rForm );

    void OutForm( bool bOn, const css::uno::Reference<css::container::XIndexContainer>& rFormComps );
    void OutHiddenControls( const css::uno::Reference<css::container::XIndexContainer>& rFormComps,
                            const css::uno::Reference<css::beans::XPropertySet>& rPropSet );
    bool HasControls() const;

    void OutFootEndNoteInfo();
    void OutFootEndNotes();
    OUString GetFootEndNoteSym( const SwFormatFootnote& rFormatFootnote );
    void OutFootEndNoteSym( const SwFormatFootnote& rFormatFootnote, const OUString& rNum,
                             sal_uInt16 nScript );

    void OutBasic();

    void OutAndSetDefList( sal_uInt16 nNewLvl );

    void OutStyleSheet( const SwPageDesc& rPageDesc, bool bUsed=true );

    inline void OutCSS1_PropertyAscii( const sal_Char *pProp,
                                       const sal_Char *pVal );
    inline void OutCSS1_PropertyAscii( const sal_Char *pProp,
                                       const OString& rVal );
    inline void OutCSS1_Property( const sal_Char *pProp, const OUString& rVal );
    void OutCSS1_Property( const sal_Char *pProp, const sal_Char *pVal,
                           const OUString *pSVal );
    void OutCSS1_UnitProperty( const sal_Char *pProp, long nVal );
    void OutCSS1_PixelProperty( const sal_Char *pProp, long nVal, bool bVert );
    void OutCSS1_SfxItemSet( const SfxItemSet& rItemSet, bool bDeep=true );

    // BODY-Tag-Events aus der SFX-Konfigaurion
    void OutBasicBodyEvents();

    // BACKGROUND/BGCOLOR-Option
    void OutBackground( const SvxBrushItem *pBrushItem, bool bGraphic );
    void OutBackground( const SfxItemSet& rItemSet, bool bGraphic );

    void OutLanguage( LanguageType eLang );
    sal_uInt16 GetHTMLDirection( sal_uInt16 nDir ) const;
    sal_uInt16 GetHTMLDirection( const SfxItemSet& rItemSet ) const;
    void OutDirection( sal_uInt16 nDir );
    static OString convertDirection(sal_uInt16 nDirection);

    // ALT/ALIGN/WIDTH/HEIGHT/HSPACE/VSPACE-Optionen des aktuellen
    // Frame-Formats ausgeben und ggf. ein <BR CLEAR=...> vorne an
    // rEndTags anhaengen
    OString OutFrameFormatOptions( const SwFrameFormat& rFrameFormat, const OUString& rAltText,
        sal_uInt32 nFrmOpts, const OString& rEndTags = OString() );

    void writeFrameFormatOptions(HtmlWriter& aHtml, const SwFrameFormat& rFrameFormat, const OUString& rAltText, sal_uInt32 nFrmOpts);

    void OutCSS1_TableFrameFormatOptions( const SwFrameFormat& rFrameFormat );
    void OutCSS1_TableCellBorderHack(const SwFrameFormat& rFrameFormat);
    void OutCSS1_SectionFormatOptions( const SwFrameFormat& rFrameFormat, const SwFormatCol *pCol );
    void OutCSS1_FrameFormatOptions( const SwFrameFormat& rFrameFormat, sal_uInt32 nFrmOpts,
                                const SdrObject *pSdrObj=nullptr,
                                const SfxItemSet *pItemSet=nullptr );
    void OutCSS1_FrameFormatBackground( const SwFrameFormat& rFrameFormat );

    void ChangeParaToken( sal_uInt16 nNew );

    void IncIndentLevel()
    {
        m_nIndentLvl++;
    }
    void DecIndentLevel()
    {
        if ( m_nIndentLvl ) m_nIndentLvl--;
    }
    OString GetIndentString(sal_uInt16 nIncLvl = 0);

    sal_Int32 GetLineLen()
    {
        return (sal_Int32)(Strm().Tell()-m_nLastLFPos);
    }
    void OutNewLine( bool bCheck=false );

    // fuer HTMLSaveData
    SwPaM* GetEndPaM() { return pOrigPam; }
    void SetEndPaM( SwPaM* pPam ) { pOrigPam = pPam; }

    static sal_uInt32 ToPixel( sal_uInt32 nVal, const bool bVert );

    sal_uInt16 GuessFrmType( const SwFrameFormat& rFrameFormat,
                         const SdrObject*& rpStrObj );
    static sal_uInt16 GuessOLENodeFrmType( const SwNode& rNd );

    void CollectFlyFrms();

    sal_uInt16 GetHTMLFontSize( sal_uInt32 nFontHeight ) const;

    // Die aktuelle Numerierungs-Information holen.
    SwHTMLNumRuleInfo& GetNumInfo() { return *m_pNumRuleInfo; }

    // Die Numerierungs-Information des naechsten Absatz holen. Sie
    // muss noch nicht vorhanden sein!
    SwHTMLNumRuleInfo *GetNextNumInfo() { return m_pNextNumRuleInfo; }

    // Die Numerierungs-Information des naechsten Absatz setzen.
    void SetNextNumInfo( SwHTMLNumRuleInfo *pNxt ) { m_pNextNumRuleInfo=pNxt; }

    // Die Numerierungs-Information des naeschten Absatz fuellen.
    void FillNextNumInfo();

    // Die Numerierungs-Information des naeschten Absatz loeschen.
    void ClearNextNumInfo();

    static const SdrObject* GetHTMLControl( const SwDrawFrameFormat& rFormat );
    static const SdrObject* GetMarqueeTextObj( const SwDrawFrameFormat& rFormat );
    static sal_uInt16 GetCSS1Selector( const SwFormat *pFormat, OString& rToken,
                                   OUString& rClass, sal_uInt16& rRefPoolId,
                                   OUString *pPseudo=nullptr );

    static const SwFormat *GetTemplateFormat( sal_uInt16 nPoolId, IDocumentStylePoolAccess* /*SwDoc*/ pTemplate );
    static const SwFormat *GetParentFormat( const SwFormat& rFormat, sal_uInt16 nDeep );

    static void SubtractItemSet( SfxItemSet& rItemSet,
                                 const SfxItemSet& rRefItemSet,
                                 bool bSetDefaults,
                                 bool bClearSame = true,
                                   const SfxItemSet *pRefScriptItemSet=nullptr );
    static bool HasScriptDependentItems( const SfxItemSet& rItemSet,
                                               bool bCheckDropCap );

    static void GetEEAttrsFromDrwObj( SfxItemSet& rItemSet,
                                      const SdrObject *pObj,
                                      bool bSetDefaults );

    static sal_uInt16 GetDefListLvl( const OUString& rNm, sal_uInt16 nPoolId );

    sal_uInt32 GetHTMLMode() const
    {
        return m_nHTMLMode;
    }
    bool IsHTMLMode( sal_uInt32 nMode ) const
    {
        return (m_nHTMLMode & nMode) != 0;
    }

    inline bool IsCSS1Source( sal_uInt16 n ) const;
    inline bool IsCSS1Script( sal_uInt16 n ) const;

    static const sal_Char *GetNumFormat( sal_uInt16 nFormat );
    static void PrepareFontList( const SvxFontItem& rFontItem, OUString& rNames,
                                 sal_Unicode cQuote, bool bGeneric );
    static sal_uInt16 GetCSS1ScriptForScriptType( sal_uInt16 nScriptType );
    static sal_uInt16 GetLangWhichIdFromScript( sal_uInt16 nScript );

    FieldUnit GetCSS1Unit() const { return m_eCSS1Unit; }

    sal_Int32 indexOfDotLeaders( sal_uInt16 nPoolId, const OUString& rText );
};

inline bool SwHTMLWriter::IsCSS1Source( sal_uInt16 n ) const
{
    return n == (m_nCSS1OutMode & CSS1_OUTMODE_SOURCE);
}

inline bool SwHTMLWriter::IsCSS1Script( sal_uInt16 n ) const
{
    sal_uInt16 nScript = (m_nCSS1OutMode & CSS1_OUTMODE_SCRIPT);
    return CSS1_OUTMODE_ANY_SCRIPT == nScript || n == nScript;
}

inline void SwHTMLWriter::OutCSS1_PropertyAscii( const sal_Char *pProp,
                                                 const sal_Char *pVal )
{
    OutCSS1_Property( pProp, pVal, nullptr );
}

inline void SwHTMLWriter::OutCSS1_PropertyAscii( const sal_Char *pProp,
                                                 const OString& rVal )
{
    OutCSS1_Property( pProp, rVal.getStr(), nullptr );
}

inline void SwHTMLWriter::OutCSS1_Property( const sal_Char *pProp,
                                            const OUString& rVal )
{
    OutCSS1_Property( pProp, nullptr, &rVal );
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
    bool bOldWriteAll : 1;
    bool bOldOutHeader : 1;
    bool bOldOutFooter : 1;
    bool bOldOutFlyFrame : 1;
    const SwFlyFrameFormat* pOldFlyFormat;

    HTMLSaveData( SwHTMLWriter&, sal_uLong nStt, sal_uLong nEnd,
                  bool bSaveNum=true,
                  const SwFrameFormat *pFrameFormat=nullptr  );
    ~HTMLSaveData();
};

// einige Funktions-Deklarationen
Writer& OutHTML_FrameFormatOLENode( Writer& rWrt, const SwFrameFormat& rFormat,
                               bool bInCntnr );
Writer& OutHTML_FrameFormatOLENodeGrf( Writer& rWrt, const SwFrameFormat& rFormat,
                                  bool bInCntnr );

Writer& OutHTML_SwTextNode( Writer&, const SwContentNode& );
Writer& OutHTML_SwTableNode( Writer& , SwTableNode &, const SwFrameFormat *,
                           const OUString* pCaption=nullptr, bool bTopCaption=false );

Writer& OutHTML_DrawFrameFormatAsControl( Writer& rWrt, const SwDrawFrameFormat& rFormat,
                                     const SdrUnoObj& rSdrObj, bool bInCntnr );
Writer& OutHTML_DrawFrameFormatAsMarquee( Writer& rWrt, const SwDrawFrameFormat& rFormat,
                                     const SdrObject& rSdrObj );

Writer& OutHTML_HeaderFooter( Writer& rWrt, const SwFrameFormat& rFrameFormat,
                              bool bHeader );

Writer& OutHTML_Image( Writer&, const SwFrameFormat& rFormat,
                       Graphic& rGraphic, const OUString& rAlternateText,
                       const Size& rRealSize, sal_uInt32 nFrmOpts,
                       const sal_Char *pMarkType = nullptr,
                       const ImageMap *pGenImgMap = nullptr );

Writer& OutHTML_BulletImage( Writer& rWrt, const sal_Char *pTag,
                             const SvxBrushItem* pBrush );

Writer& OutHTML_SwFormatField( Writer& rWrt, const SfxPoolItem& rHt );
Writer& OutHTML_SwFormatFootnote( Writer& rWrt, const SfxPoolItem& rHt );
Writer& OutHTML_INetFormat( Writer&, const SwFormatINetFormat& rINetFormat, bool bOn );

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

#endif // INCLUDED_SW_SOURCE_FILTER_HTML_WRTHTML_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
