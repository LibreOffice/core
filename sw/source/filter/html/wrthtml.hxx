/*************************************************************************
 *
 *  $RCSfile: wrthtml.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:56 $
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

#ifndef _WRTHTML_HXX
#define _WRTHTML_HXX


#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORM_HPP_
#include <com/sun/star/form/XForm.hpp>
#endif
#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#define _SVSTDARR_STRINGSDTOR
#define _SVSTDARR_STRINGSSORTDTOR
#define _SVSTDARR_ULONGS
#include <svtools/svstdarr.hxx>

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

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
class SwNoTxtNode;
class SwPageDesc;
class SwNodeIndex;
class ImageMap;
class SwNumRule;
class SdrObject;
class SvxBrushItem;
class SvxFontItem;
class SwHTMLNumRuleInfo;
class SwHTMLPosFlyFrms;
class SwHTMLTxtFtns;

extern SwAttrFnTab aHTMLAttrFnTab;

//#define HTML_PARSPACE ((MM50 * 7) / 10)
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
#define CSS1_OUTMODE_SPAN_NO_ON     0x0000
#define CSS1_OUTMODE_SPAN_TAG_ON    0x0001
#define CSS1_OUTMODE_STYLE_OPT_ON   0x0002
#define CSS1_OUTMODE_RULE_ON        0x0003
#define CSS1_OUTMODE_SPAN_TAG1_ON   0x0004
#define CSS1_OUTMODE_ANY_ON         0x0007

#define CSS1_OUTMODE_SPAN_NO_OFF    0x0000
#define CSS1_OUTMODE_SPAN_TAG_OFF   (0x0001 << 3)
#define CSS1_OUTMODE_STYLE_OPT_OFF  (0x0002 << 3)
#define CSS1_OUTMODE_RULE_OFF       (0x0003 << 3)
#define CSS1_OUTMODE_SPAN_TAG1_OFF  (0x0004 << 3)
#define CSS1_OUTMODE_ANY_OFF        (0x0007 << 3)

#define CSS1_OUTMODE_ONOFF(a) (CSS1_OUTMODE_##a##_ON|CSS1_OUTMODE_##a##_OFF)
#define CSS1_OUTMODE_SPAN_TAG       CSS1_OUTMODE_ONOFF(SPAN_TAG)
#define CSS1_OUTMODE_STYLE_OPT      CSS1_OUTMODE_ONOFF(STYLE_OPT)
#define CSS1_OUTMODE_RULE           CSS1_OUTMODE_ONOFF(RULE)
#define CSS1_OUTMODE_SPAN_TAG1      CSS1_OUTMODE_ONOFF(TAG1)

// Die folgenden Flags legen fest, was ausgegeben wird
#define CSS1_OUTMODE_TEMPLATE       0x0000
#define CSS1_OUTMODE_BODY           (0x0001 << 6)
#define CSS1_OUTMODE_PARA           (0x0002 << 6)
#define CSS1_OUTMODE_HINT           (0x0003 << 6)
#define CSS1_OUTMODE_FRAME          (0x0004 << 6)
#define CSS1_OUTMODE_TABLE          (0x0005 << 6)
#define CSS1_OUTMODE_TABLEBOX       (0x0006 << 6)
#define CSS1_OUTMODE_DROPCAP        (0x0007 << 6)
#define CSS1_OUTMODE_SECTION        (0x0008 << 6)
#define CSS1_OUTMODE_SOURCE         (0x000f << 6)

#define CSS1_OUTMODE_ENCODE         (0x0001 << 10)


// der HTML-Writer
struct HTMLControl;
SV_DECL_PTRARR_SORT_DEL( HTMLControls, HTMLControl*, 1, 1 )
SV_DECL_PTRARR( INetFmts, SwFmtINetFmt*, 1, 1 )

struct SwHTMLFmtInfo;
SV_DECL_PTRARR_SORT_DEL( SwHTMLFmtInfos, SwHTMLFmtInfo*, 1, 1 )

class SwHTMLWriter : public Writer
{
    String aCId;                    // Content-Id fuer Mail-Export

    SwHTMLPosFlyFrms *pHTMLPosFlyFrms;
    SwHTMLNumRuleInfo *pNumRuleInfo;// aktuelle Numerierung
    SwHTMLNumRuleInfo *pNextNumRuleInfo;
    const SwFlyFrmFmt* pFlyFrmFmt;  // liegt der Node in einem FlyFrame, ist
                                    // das Format gesetzt, sonst 0. In Tabellen
                                    // nur dann, wenn die Tabelle alleine
                                    // im Rahmen ist.

    sal_uInt32 nHTMLMode;               // Beschreibung der Export-Konfiguration

    FieldUnit eCSS1Unit;

    sal_uInt16 OutHeaderAttrs();
    const SwPageDesc *MakeHeader( sal_uInt16& rHeaderAtrs );
    void GetControls();

    void AddLinkTarget( const String& rURL );
    void CollectLinkTargets();

protected:
    sal_uInt32 WriteStream();

public:
#if defined(MAC) || defined(UNX)
    static const sal_Char sNewLine;     // nur \012 oder \015
#else
    static const sal_Char __FAR_DATA sNewLine[];    // \015\012
#endif

    SvStringsDtor aImgMapNames;     // geschriebene Image Maps
    SvStringsSortDtor aImplicitMarks;// implizite Stprungmarken
    SvStringsDtor aOutlineMarks;        // geschriebene Image Maps
    SvULongs aOutlineMarkPoss;
    HTMLControls aHTMLControls;     // die zu schreibenden ::com::sun::star::form::Forms
    SwHTMLFmtInfos aChrFmtInfos;
    SwHTMLFmtInfos aTxtCollInfos;
    INetFmts aINetFmts;             // die "offenen" INet-Attribute
    SwHTMLTxtFtns *pFootEndNotes;

    String aCSS1Selector;           // der Selektor eines Styles
    String aBulletGrfs[MAXLEVEL];   // die Grafiken fuer Listen

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >  *pxFormComps; // die aktuelle Form

    SwDoc *pTemplate;               // die HTML-Vorlage
    Color *pDfltColor;              // default Farbe
    SwNodeIndex *pStartNdIdx;       // Index des ersten Absatz
    const SwPageDesc *pCurrPageDesc;// Die aktuelle Seiten-Vorlage
    const SwFmtFtn *pFmtFtn;

    sal_uInt32 aFontHeights[7];         // die Font-Hoehen 1-7

    sal_uInt32 nWarn;                   // Result-Code fuer Warnungen
    xub_StrLen nLastLFPos;              // letzte Position eines LF

    sal_uInt16 nLastParaToken;          // fuers Absaetze zusammenhalten
    sal_uInt16 nBkmkTabPos;             // akt. Position in der Bookmark-Tabelle
    sal_uInt16 nImgMapCnt;              // zum eindeutig
    sal_uInt16 nFormCntrlCnt;
    sal_uInt16 nEndNote;
    sal_uInt16 nFootNote;
    sal_uInt16 nLeftMargin;             // linker Einzug (z.B. aus Listen)
    sal_uInt16 nDfltLeftMargin;         // die dafaults, der nicht geschrieben
    sal_uInt16 nDfltRightMargin;        // werden muessen (aus der Vorlage)
    short  nFirstLineIndent;        // Erstzeilen-Einzug (aus Listen)
    short  nDfltFirstLineIndent;    // nicht zu schreibender default
    sal_uInt16 nDfltTopMargin;          // die defaults, der nicht geschrieben
    sal_uInt16 nDfltBottomMargin;       // werden muessen (aus der Vorlage)
    sal_uInt16 nIndentLvl;              // wie weit ist eingerueckt?
    xub_StrLen nWhishLineLen;           // wie lang darf eine Zeile werden?
    sal_uInt16 nDefListLvl;             // welcher DL-Level existiert gerade
    sal_uInt16 nDefListMargin;          // Wie weit wird in DL eingerueckt
    sal_uInt16 nHeaderFooterSpace;
    sal_uInt16 nTxtAttrsToIgnore;
    sal_uInt16 nExportMode;
    sal_uInt16 nCSS1OutMode;

    rtl_TextEncoding    eDestEnc;

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

    // Flags fuer Style-Export

    sal_Bool bFirstCSS1Rule : 1;        // wurde schon eine Property ausgegeben
    sal_Bool bFirstCSS1Property : 1;    // wurde schon eine Property ausgegeben
    sal_Bool bPoolCollTextModified : 1; // die Textkoerper-Vorlage wurde
                                    // modifiziert.
    sal_Bool bCSS1IgnoreFirstPageDesc : 1;
    // 16

    // was muss/kann/darf nicht ausgegeben werden?

    sal_Bool bNoAlign : 1;              // HTML-Tag erlaubt kein ALIGN=...
    sal_Bool bClearLeft : 1;            // <BR CLEAR=LEFT> am Absatz-Ende ausg.
    sal_Bool bClearRight : 1;           // <BR CLEAR=RIGHT> am Absatz-Ende ausg.
    sal_Bool bLFPossible : 1;           // ein Zeilenumbruch darf eingef. werden

    // sonstiges

    sal_Bool bPreserveForm : 1;         // die aktuelle Form beibehalten

    sal_Bool bCfgNetscape4 : 1;         // Netscape4 Hacks
    // 22

    SwHTMLWriter();
    virtual ~SwHTMLWriter();

    void Out_SwDoc( SwPaM* );       // schreibe den makierten Bereich

    // gebe alle an in aktuellen Ansatz stehenden ::com::sun::star::text::Bookmarks aus
    void OutAnchor( const String& rName );
    void OutBookmarks();
    void OutImplicitMark( const String& rMark, const sal_Char *pMarkType );

    // gebe die evt. an der akt. Position stehenden FlyFrame aus.
    sal_Bool OutFlyFrm( sal_uInt32 nNdIdx, xub_StrLen nCntntIdx,
                        sal_uInt8 nPos );
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
    xub_StrLen GetFootEndNoteSymLen( const SwFmtFtn& rFmtFtn );
    void OutFootEndNoteSym( const SwFmtFtn& rFmtFtn );

#ifdef JAVA_BASIC_IDE
    void OutBasicModule( const String& rName, const String& rLanguage );
#endif
    void OutBasic();

    void OutAndSetDefList( sal_uInt16 nNewLvl );

    void OutStyleSheet( const SwPageDesc& rPageDesc, sal_Bool bUsed=sal_True );

    inline void OutCSS1_PropertyAscii( const sal_Char *pProp,
                                       const sal_Char *pVal );
    inline void OutCSS1_PropertyAscii( const sal_Char *pProp,
                                       const ByteString& rVal );
    inline void OutCSS1_Property( const sal_Char *pProp, const String& rVal );
    void OutCSS1_Property( const sal_Char *pProp, const sal_Char *pVal,
                           const String *pSVal );
    void OutCSS1_UnitProperty( const sal_Char *pProp, long nVal );
    void OutCSS1_PixelProperty( const sal_Char *pProp, long nVal, sal_Bool bVert );
    void OutCSS1_SfxItemSet( const SfxItemSet& rItemSet, sal_Bool bDeep=sal_True );
#if USED
    void OutCSS1_SfxItem( const SfxPoolItem *pItem1,
                          const SfxPoolItem *pItem2 = 0 );
#endif

    // BODY-Tag-Events aus der SFX-Konfigaurion
    void OutBasicBodyEvents();

    // BACKGROUND/BGCOLOR-Option
    void OutBackground( const SvxBrushItem *pBrushItem, String& rEmbGrfNm,
                        sal_Bool bGraphic );
    void OutBackground( const SfxItemSet& rItemSet, String &rEmbGrfName,
                        sal_Bool bGraphic );

    // ALT/ALIGN/WIDTH/HEIGHT/HSPACE/VSPACE-Optionen des aktuellen
    // Frame-Formats ausgeben und ggf. ein <BR CLEAR=...> vorne an
    // rEndTags anhaengen
    void OutFrmFmtOptions( const SwFrmFmt& rFrmFmt, const String& rAltTxt,
                            ByteString &rEndTags, sal_uInt32 nFrmOpts );
    void OutCSS1_TableFrmFmtOptions( const SwFrmFmt& rFrmFmt );
    void OutCSS1_SectionFmtOptions( const SwFrmFmt& rFrmFmt );
    void OutCSS1_FrmFmtOptions( const SwFrmFmt& rFrmFmt, sal_uInt32 nFrmOpts,
                                const SdrObject *pSdrObj=0,
                                const SfxItemSet *pItemSet=0 );
    void OutCSS1_FrmFmtBackground( const SwFrmFmt& rFrmFmt );

    void ChangeParaToken( sal_uInt16 nNew );

    // Umgebendes FlyFrmFmt
    const SwFlyFrmFmt *GetFlyFrmFmt() const { return pFlyFrmFmt; }
    void SetFlyFrmFmt( const SwFlyFrmFmt *pFly ) { pFlyFrmFmt = pFly; }

    void IncIndentLevel() { nIndentLvl++; }
    void DecIndentLevel() { if ( nIndentLvl ) nIndentLvl--; }
    void GetIndentString( ByteString& rStr, sal_uInt16 nIncLvl=0 );

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
    static sal_uInt16 GetCSS1Selector( const SwFmt *pFmt, ByteString& rToken,
                                   String& rClass, sal_uInt16& rRefPoolId,
                                   String *pPseudo=0 );

    static const SwFmt *GetTemplateFmt( sal_uInt16 nPoolId, SwDoc *pTemplate );
    static const SwFmt *GetParentFmt( const SwFmt& rFmt, sal_uInt16 nDeep );

    static void SubtractItemSet( SfxItemSet& rItemSet,
                                 const SfxItemSet& rRefItemSet,
                                 sal_Bool bSetDefaults,
                                 sal_Bool bClearSame = sal_True );

    static void GetEEAttrsFromDrwObj( SfxItemSet& rItemSet,
                                      const SdrObject *pObj,
                                      sal_Bool bSetDefaults );

    static sal_uInt16 GetDefListLvl( const String& rNm, sal_uInt16 nPoolId );

    sal_uInt32 GetHTMLMode() const { return nHTMLMode; }
    sal_Bool IsHTMLMode( sal_uInt32 nMode ) const { return (nHTMLMode & nMode) != 0; }

    inline sal_Bool IsCSS1Source( sal_uInt16 n ) const;
    sal_Bool HasCId() const { return aCId.Len() > 0; }
    void MakeCIdURL( String& rURL );

    static const sal_Char *GetNumFormat( sal_uInt16 nFmt );
    static void PrepareFontList( const SvxFontItem& rFontItem, String& rNames,
                                 sal_Unicode cQuote, sal_Bool bGeneric );

    FieldUnit GetCSS1Unit() const { return eCSS1Unit; }
};

inline sal_Bool SwHTMLWriter::IsCSS1Source( sal_uInt16 n ) const
{
    return n == (nCSS1OutMode & CSS1_OUTMODE_SOURCE);
}

inline void SwHTMLWriter::OutCSS1_PropertyAscii( const sal_Char *pProp,
                                                 const sal_Char *pVal )
{
    OutCSS1_Property( pProp, pVal, 0 );
}

inline void SwHTMLWriter::OutCSS1_PropertyAscii( const sal_Char *pProp,
                                                 const ByteString& rVal )
{
    OutCSS1_Property( pProp, rVal.GetBuffer(), 0 );
}

inline void SwHTMLWriter::OutCSS1_Property( const sal_Char *pProp,
                                            const String& rVal )
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
    sal_Bool bOldWriteAll : 1;
    sal_Bool bOldOutHeader : 1;
    sal_Bool bOldOutFooter : 1;
    const SwFlyFrmFmt* pOldFlyFmt;

    HTMLSaveData( SwHTMLWriter&, sal_uInt32 nStt, sal_uInt32 nEnd,
                  const SwFlyFrmFmt* pFly, sal_Bool bSaveNum=sal_True );
    ~HTMLSaveData();
};


// einige Funktions-Deklarationen
Writer& OutHTML_FrmFmtOLENode( Writer& rWrt, const SwFrmFmt& rFmt,
                               sal_Bool bInCntnr );
Writer& OutHTML_FrmFmtOLENodeGrf( Writer& rWrt, const SwFrmFmt& rFmt,
                                  sal_Bool bInCntnr );

Writer& OutHTML_SwTxtNode( Writer&, const SwCntntNode& );
Writer& OutHTML_SwTblNode( Writer& , SwTableNode &, const SwFrmFmt *,
                           const String* pCaption=0, sal_Bool bTopCaption=sal_False );

Writer& OutHTML_DrawFrmFmtAsControl( Writer& rWrt, const SwDrawFrmFmt& rFmt,
                                     const SdrObject& rSdrObj, sal_Bool bInCntnr );
Writer& OutHTML_DrawFrmFmtAsMarquee( Writer& rWrt, const SwDrawFrmFmt& rFmt,
                                     const SdrObject& rSdrObj );

Writer& OutHTML_HeaderFooter( Writer& rWrt, const SwFrmFmt& rFrmFmt,
                              sal_Bool bHeader );

Writer& OutHTML_Image( Writer&, const SwFrmFmt& rFmt,
                       const String& rGrfName, const String& rAlternateTxt,
                       const Size& rRealSize, sal_uInt32 nFrmOpts,
                       const sal_Char *pMarkType = 0,
                       const ImageMap *pGenImgMap = 0 );
Writer& OutHTML_BulletImage( Writer& rWrt, const sal_Char *pTag,
                             const SvxBrushItem* pBrush, String &rGrfName,
                             const Size &rSize,
                             const SwFmtVertOrient* pVertOrient );

Writer& OutHTML_SwFmtFld( Writer& rWrt, const SfxPoolItem& rHt );
Writer& OutHTML_SwFmtFtn( Writer& rWrt, const SfxPoolItem& rHt );
Writer& OutHTML_INetFmt( Writer&, const SwFmtINetFmt& rINetFmt, sal_Bool bOn );

Writer& OutCSS1_BodyTagStyleOpt( Writer& rWrt, const SfxItemSet& rItemSet,
                                 String aEmbBGGrfName );
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


#endif  //  _WRTHTML_HXX

