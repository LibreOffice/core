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
#ifndef _SWHTML_HXX
#define _SWHTML_HXX

#include <sfx2/sfxhtml.hxx>
#include <svl/macitem.hxx>
#include <editeng/svxenum.hxx>
#include <fmtornt.hxx>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/form/XFormComponent.hpp>

#include "calbck.hxx"
#include "htmlvsh.hxx"

class SfxMedium;
class SfxViewFrame;
class SdrObject;
class SvxMacroTableDtor;
class SwDoc;
class SwPaM;
class ViewShell;
class SwStartNode;
class SwFmtColl;
class SwField;
class SwHTMLForm_Impl;
class SwApplet_Impl;
struct SwHTMLFootEndNote_Impl;
class HTMLTableCnts;
struct SwPendingStack;
class SvxCSS1PropertyInfo;

#define HTML_PARSPACE (MM50)
#define HTML_CJK_PARSPACE (MM50/2)
#define HTML_CTL_PARSPACE (MM50/2)

#define HTML_DFLT_IMG_WIDTH (MM50*4)
#define HTML_DFLT_IMG_HEIGHT (MM50*2)

// ein par Sachen, die man oefter mal braucht
extern HTMLOptionEnum aHTMLPAlignTable[];
extern HTMLOptionEnum aHTMLImgHAlignTable[];
extern HTMLOptionEnum aHTMLImgVAlignTable[];


// der Attribut Stack:

class _HTMLAttr;
typedef std::deque<_HTMLAttr *> _HTMLAttrs;

class _HTMLAttr
{
    friend class SwHTMLParser;
    friend class _CellSaveStruct;

    SwNodeIndex nSttPara, nEndPara;
    xub_StrLen nSttCntnt, nEndCntnt;
    sal_Bool bInsAtStart : 1;
    sal_Bool bLikePara : 1; // Attribut ueber dem gesamten Absatz setzen
    sal_Bool bValid : 1;    // ist das Attribut gueltig?

    SfxPoolItem* pItem;
    sal_uInt16 nCount;      // Anzahl noch zu schliessender Attrs mit einem Wert
    _HTMLAttr *pNext;   // noch zu schliessene Attrs mit unterschiedl. Werten
    _HTMLAttr *pPrev;   // bereits geschlossene aber noch nicht gesetze Attrs
    _HTMLAttr **ppHead; // der Listenkopf

    _HTMLAttr( const SwPosition& rPos, const SfxPoolItem& rItem,
               _HTMLAttr **pHd=0 );

    _HTMLAttr( const _HTMLAttr &rAttr, const SwNodeIndex &rEndPara,
               xub_StrLen nEndCnt, _HTMLAttr **pHd );

public:

    ~_HTMLAttr();

    _HTMLAttr *Clone( const SwNodeIndex& rEndPara, xub_StrLen nEndCnt ) const;
    void Reset( const SwNodeIndex& rSttPara, xub_StrLen nSttCnt,
                _HTMLAttr **pHd );
    inline void SetStart( const SwPosition& rPos );

    sal_uInt32 GetSttParaIdx() const { return nSttPara.GetIndex(); }
    sal_uInt32 GetEndParaIdx() const { return nEndPara.GetIndex(); }

    const SwNodeIndex& GetSttPara() const { return nSttPara; }
    const SwNodeIndex& GetEndPara() const { return nEndPara; }

    xub_StrLen GetSttCnt() const { return nSttCntnt; }
    xub_StrLen GetEndCnt() const { return nEndCntnt; }

    sal_Bool IsLikePara() const { return bLikePara; }
    void SetLikePara( sal_Bool bPara=sal_True ) { bLikePara = bPara; }

          SfxPoolItem& GetItem()        { return *pItem; }
    const SfxPoolItem& GetItem() const  { return *pItem; }

    _HTMLAttr *GetNext() const { return pNext; }
    void InsertNext( _HTMLAttr *pNxt ) { pNext = pNxt; }

    _HTMLAttr *GetPrev() const { return pPrev; }
    void InsertPrev( _HTMLAttr *pPrv );
    void ClearPrev() { pPrev = 0; }

    void SetHead( _HTMLAttr **ppHd ) { ppHead = ppHd; }

    // Beim Setzen von Attributen aus Vorlagen kann es passieren,
    // dass Attribute doch nicht mehr gesetzt werden sollen. Die zu loeschen
    // waere sehr aufwendig, da man nicht so genau weiss, wo sie eingekettet
    // sind. Sie werden deshalb einfach invalidiert und erst beim naechsten
    // _SetAttr() geloescht.
    void Invalidate() { bValid = sal_False; }
    sal_Bool IsValid() const { return bValid; }
};

// Tabelle der Attribute: Hier ist die Reihenfolge wichtig: Die Attribute
// vorne in der Tabelle werden in EndAllAttrs auch zuerst gesetzt.
struct _HTMLAttrTable
{
    _HTMLAttr
                *pKeep,         // ::com::sun::star::frame::Frame-Attribure
                *pBox,
                *pBrush,
                *pBreak,
                *pPageDesc,

                *pLRSpace,      // Absatz-Attribute
                *pULSpace,
                *pLineSpacing,
                *pAdjust,
                *pDropCap,
                *pSplit,
                *pWidows,
                *pOrphans,
                *pDirection,

                *pCharFmts,     // Text-Attribute
                *pINetFmt,

                *pBold,         // Zeichen-Attribute
                *pBoldCJK,
                *pBoldCTL,
                *pItalic,
                *pItalicCJK,
                *pItalicCTL,
                *pStrike,
                *pUnderline,
                *pBlink,
                *pFont,
                *pFontCJK,
                *pFontCTL,
                *pFontHeight,
                *pFontHeightCJK,
                *pFontHeightCTL,
                *pFontColor,
                *pEscapement,
                *pCaseMap,
                *pKerning,      // (nur fuer SPACER)
                *pCharBrush,    // Zeichen-Hintergrund
                *pLanguage,
                *pLanguageCJK,
                *pLanguageCTL
                ;
};

class _HTMLAttrContext_SaveDoc;

enum SwHTMLAppendMode {
     AM_NORMAL,         // keine Absatz-Abstand-Behandlung
     AM_NOSPACE,        // Abstand hart auf 0cm setzen
     AM_SPACE,          // Abstand hart auf 0.5cm setzen
     AM_SOFTNOSPACE,    // Abstand nicht setzen aber 0cm merken
     AM_NONE            // gar kein Append
};

class _HTMLAttrContext
{
    _HTMLAttrs aAttrs;      // die in dem Kontext gestarteten Attribute

    String aClass;          // die Klasse des Kontexts

    _HTMLAttrContext_SaveDoc *pSaveDocContext;
    SfxItemSet *pFrmItemSet;

    sal_uInt16  nToken;         // das Token, zu dem der Kontext gehoehrt

    sal_uInt16  nTxtFmtColl;    // eine in dem Kontext begonnene Vorlage oder 0

    sal_uInt16  nLeftMargin;        // ein veraenderter linker Rand
    sal_uInt16  nRightMargin;       // ein veraenderter rechter Rand
    sal_uInt16  nFirstLineIndent;   // ein veraenderter Erstzeilen-Einzug

    sal_uInt16  nUpperSpace;
    sal_uInt16  nLowerSpace;

    SwHTMLAppendMode eAppend;

    sal_Bool    bLRSpaceChanged : 1;// linker/rechtr Rand, Einzug veraendert?
    sal_Bool    bULSpaceChanged : 1;// oberer/unterer Rand veraendert?
    sal_Bool    bDfltTxtFmtColl : 1;// nTxtFmtColl ist nur ein default
    sal_Bool    bSpansSection : 1;  // Der Kontext spannt eine SwSection auf
    sal_Bool    bPopStack : 1;      // Oberhalb liegende Stack-Elemente entf.
    sal_Bool    bFinishPREListingXMP : 1;
    sal_Bool    bRestartPRE : 1;
    sal_Bool    bRestartXMP : 1;
    sal_Bool    bRestartListing : 1;

public:
    void ClearSaveDocContext();

    _HTMLAttrContext( sal_uInt16 nTokn, sal_uInt16 nPoolId, const String& rClass,
                      sal_Bool bDfltColl=sal_False ) :
        aClass( rClass ),
        pSaveDocContext( 0 ),
        pFrmItemSet( 0 ),
        nToken( nTokn ),
        nTxtFmtColl( nPoolId ),
        nLeftMargin( 0 ),
        nRightMargin( 0 ),
        nFirstLineIndent( 0 ),
        nUpperSpace( 0 ),
        nLowerSpace( 0 ),
        eAppend( AM_NONE ),
        bLRSpaceChanged( sal_False ),
        bULSpaceChanged( sal_False ),
        bDfltTxtFmtColl( bDfltColl ),
        bSpansSection( sal_False ),
        bPopStack( sal_False ),
        bFinishPREListingXMP( sal_False ),
        bRestartPRE( sal_False ),
        bRestartXMP( sal_False ),
        bRestartListing( sal_False )
    {}

    _HTMLAttrContext( sal_uInt16 nTokn ) :
        pSaveDocContext( 0 ),
        pFrmItemSet( 0 ),
        nToken( nTokn ),
        nTxtFmtColl( 0 ),
        nLeftMargin( 0 ),
        nRightMargin( 0 ),
        nFirstLineIndent( 0 ),
        nUpperSpace( 0 ),
        nLowerSpace( 0 ),
        eAppend( AM_NONE ),
        bLRSpaceChanged( sal_False ),
        bULSpaceChanged( sal_False ),
        bDfltTxtFmtColl( sal_False ),
        bSpansSection( sal_False ),
        bPopStack( sal_False ),
        bFinishPREListingXMP( sal_False ),
        bRestartPRE( sal_False ),
        bRestartXMP( sal_False ),
        bRestartListing( sal_False )
    {}

    ~_HTMLAttrContext() { ClearSaveDocContext(); delete pFrmItemSet; }

    sal_uInt16 GetToken() const { return nToken; }

    sal_uInt16 GetTxtFmtColl() const { return bDfltTxtFmtColl ? 0 : nTxtFmtColl; }
    sal_uInt16 GetDfltTxtFmtColl() const { return bDfltTxtFmtColl ? nTxtFmtColl : 0; }

    const String& GetClass() const { return aClass; }

    inline void SetMargins( sal_uInt16 nLeft, sal_uInt16 nRight, short nIndent );

    inline sal_Bool IsLRSpaceChanged() const { return bLRSpaceChanged; }
    inline void GetMargins( sal_uInt16& nLeft, sal_uInt16& nRight,
                            short &nIndent ) const;

    inline void SetULSpace( sal_uInt16 nUpper, sal_uInt16 nLower );
    inline sal_Bool IsULSpaceChanged() const { return bULSpaceChanged; }
    inline void GetULSpace( sal_uInt16& rUpper, sal_uInt16& rLower ) const;

    sal_Bool HasAttrs() const { return !aAttrs.empty(); }
    const _HTMLAttrs& GetAttrs() const { return aAttrs; }
    _HTMLAttrs& GetAttrs() { return aAttrs; }

    void SetSpansSection( sal_Bool bSet ) { bSpansSection = bSet; }
    sal_Bool GetSpansSection() const { return bSpansSection; }

    void SetPopStack( sal_Bool bSet ) { bPopStack = bSet; }
    sal_Bool GetPopStack() const { return bPopStack; }

    sal_Bool HasSaveDocContext() const { return pSaveDocContext!=0; }
    _HTMLAttrContext_SaveDoc *GetSaveDocContext( sal_Bool bCreate=sal_False );

    const SfxItemSet *GetFrmItemSet() const { return pFrmItemSet; }
    SfxItemSet *GetFrmItemSet( SwDoc *pCreateDoc );

    void SetFinishPREListingXMP( sal_Bool bSet ) { bFinishPREListingXMP = bSet; }
    sal_Bool IsFinishPREListingXMP() const { return bFinishPREListingXMP; }

    void SetRestartPRE( sal_Bool bSet ) { bRestartPRE = bSet; }
    sal_Bool IsRestartPRE() const { return bRestartPRE; }

    void SetRestartXMP( sal_Bool bSet ) { bRestartXMP = bSet; }
    sal_Bool IsRestartXMP() const { return bRestartXMP; }

    void SetRestartListing( sal_Bool bSet ) { bRestartListing = bSet; }
    sal_Bool IsRestartListing() const { return bRestartListing; }

    void SetAppendMode( SwHTMLAppendMode eMode=AM_NORMAL ) { eAppend = eMode; }
    SwHTMLAppendMode GetAppendMode() const { return eAppend; }
};

typedef std::vector<_HTMLAttrContext *> _HTMLAttrContexts;

class HTMLTable;
class SwCSS1Parser;
class SwHTMLNumRuleInfo;

typedef boost::ptr_vector<ImageMap> ImageMaps;
typedef std::vector<SwFrmFmt *> SwHTMLFrmFmts;

#define HTML_CNTXT_PROTECT_STACK    0x0001
#define HTML_CNTXT_STRIP_PARA       0x0002
#define HTML_CNTXT_KEEP_NUMRULE     0x0004
#define HTML_CNTXT_HEADER_DIST      0x0008
#define HTML_CNTXT_FOOTER_DIST      0x0010
#define HTML_CNTXT_KEEP_ATTRS       0x0020

#define CONTEXT_FLAGS_ABSPOS    \
    (HTML_CNTXT_PROTECT_STACK | \
     HTML_CNTXT_STRIP_PARA)

#define HTML_FF_BOX                 0x0001
#define HTML_FF_BACKGROUND          0x0002
#define HTML_FF_PADDING             0x0004
#define HTML_FF_DIRECTION           0x0008

class SwHTMLParser : public SfxHTMLParser, public SwClient
{
    friend class _SectionSaveStruct;
    friend class _CellSaveStruct;
    friend class _CaptionSaveStruct;

    OUString      aPathToFile;
    OUString      sBaseURL;
    OUString      sSaveBaseURL;
    OUString      aBasicLib;
    OUString      aBasicModule;
    OUString      aScriptSource;  // Inhalt des aktuellen Script-Blocks
    OUString      aScriptType;    // Type des gelesenen Scripts (StarBasic/VB/JAVA)
    OUString      aScriptURL;     // URL eines Scripts
    OUString      aStyleSource;   // Inhalt des aktuellen Style-Sheets
    OUString      aContents;      // Text des akteullen Marquee, Feldes etc.
    OUString      sTitle;
    OUString      aUnknownToken;  // ein gestartetes unbekanntes Token
    OUString      aBulletGrfs[MAXLEVEL];
    OUString      sJmpMark;

    std::vector<sal_uInt16>   aBaseFontStack; // Stack fuer <BASEFONT>
                                // Bit 0-2: Fontgroesse (1-7)
    std::vector<sal_uInt16>   aFontStack;     // Stack fuer <FONT>, <BIG>, <SMALL>
                                // Bit 0-2: Fontgroesse (1-7)
                                // Bit 15: Fontfarbe wurde gesetzt

    _HTMLAttrs      aSetAttrTab;// "geschlossene", noch nicht gesetzte Attr.
    _HTMLAttrs      aParaAttrs; // vorlauefige Absatz-Attribute
    _HTMLAttrTable  aAttrTab;   // "offene" Attribute
    _HTMLAttrContexts aContexts;// der aktuelle Attribut/Token-Kontext
    SwHTMLFrmFmts   aMoveFlyFrms;// Fly-Frames, the anchor is moved
    std::deque<xub_StrLen> aMoveFlyCnts;// and the Content-Positions

    SwApplet_Impl *pAppletImpl; // das aktuelle Applet

    SwCSS1Parser    *pCSS1Parser;   // der Style-Sheet-Parser
    SwHTMLNumRuleInfo *pNumRuleInfo;
    SwPendingStack  *pPendStack;

    SwDoc           *pDoc;
    SwPaM           *pPam;      // SwPosition duerfte doch reichen, oder ??
    ViewShell       *pActionViewShell;  // ViewShell, an der das StartAction
                                        // gerufen wurde.
    SwNodeIndex     *pSttNdIdx;

    HTMLTable       *pTable;    // die aktuelle "auesserste" Tabelle
    SwHTMLForm_Impl *pFormImpl;// die aktuelle Form
    SdrObject       *pMarquee;  // aktuelles Marquee
    SwField         *pField;    // aktuelles Feld
    ImageMap        *pImageMap; // aktuelle Image-Map
    ImageMaps       *pImageMaps;// alle gelesenen Image-Maps
    SwHTMLFootEndNote_Impl *pFootEndNoteImpl;

    Size    aHTMLPageSize;      // die Seitengroesse der HTML-Vorlage

    sal_uInt32  aFontHeights[7];    // die Font-Hoehen 1-7
    sal_uInt32  nScriptStartLineNr; // Zeilennummer eines Script-Blocks
    sal_uLong       nEventId;

    sal_uInt16  nBaseFontStMin;     //
    sal_uInt16  nFontStMin;         //
    sal_uInt16  nDefListDeep;       //
    sal_uInt16  nFontStHeadStart;   // Elemente im Font-Stack bei <Hn>
    sal_uInt16  nSBModuleCnt;       // Zaehler fuer Basic-Module
    sal_uInt16  nMissingImgMaps;    // Wie viele Image-Maps fehlen noch?
    size_t nParaCnt;
    sal_uInt16  nContextStMin;      // Untergrenze fuer PopContext
    sal_uInt16  nContextStAttrMin;  // Untergrenze fuer Attributierung
    sal_uInt16  nSelectEntryCnt;    // Anzahl der Eintraege der akt. Listbox
    sal_uInt16  nOpenParaToken;     // ein geoeffnetes Absatz-Element

    enum JumpToMarks { JUMPTO_NONE, JUMPTO_MARK, JUMPTO_TABLE, JUMPTO_FRAME,
                        JUMPTO_REGION, JUMPTO_GRAPHIC } eJumpTo;

#ifdef DBG_UTIL
    sal_uInt16  m_nContinue;        // depth of Continue calls
#endif

    SvxAdjust   eParaAdjust;    // Ausrichtung des aktuellen Absatz
    HTMLScriptLanguage eScriptLang; // die aktuelle Script-Language

    sal_Bool bOldIsHTMLMode : 1;    // War's mal ein HTML-Dokument?

    sal_Bool bDocInitalized : 1;    // Dokument bzw. Shell wurden initialisiert
                                // Flag um doppeltes init durch Rekursion
                                // zu verhindern.
    sal_Bool bViewCreated : 1;      // die View wurde schon erzeugt (asynchron)
    sal_Bool bSetCrsr : 1;          // Crsr wieder auf den Anfang setzen
    sal_Bool bSetModEnabled : 1;

    sal_Bool bInFloatingFrame : 1;  // Wir sind in einen Floating ::com::sun::star::frame::Frame
    sal_Bool bInField : 1;
    sal_Bool bKeepUnknown : 1;      // unbekannte/nicht unterstuetze Tokens beh.
    // 8
    sal_Bool bCallNextToken : 1;    // In Tabellen: NextToken in jedem Fall rufen
    sal_Bool bIgnoreRawData : 1;    // Inhalt eines Scripts/Styles ignorieren.
    sal_Bool bLBEntrySelected : 1;  // Ist der aktuelle Listbox-Eintrag selekt.
    sal_Bool bTAIgnoreNewPara : 1;  // naechstes LF in TextArea ignorieren?
    sal_Bool bFixMarqueeWidth : 1;  // Groesse einer Laufschrift anpassen?
    sal_Bool bFixMarqueeHeight : 1;

    sal_Bool bUpperSpace : 1;       // obererer Absatz-Abstand wird benoetigt
    sal_Bool bNoParSpace : 1;
    // 16

    sal_Bool bAnyStarBasic : 1;     // gibt es ueberhaupt ein StarBasic-Modul
    sal_Bool bInNoEmbed : 1;        // Wir sind in einem NOEMBED-Bereich

    sal_Bool bInTitle : 1;          // Wir sind im Titel

    sal_Bool bChkJumpMark : 1;      // springe ggfs. zu einem vorgegebenem Mark
    sal_Bool bUpdateDocStat : 1;
    sal_Bool bFixSelectWidth : 1;   // Breite eines Selects neu setzen?
    sal_Bool bFixSelectHeight : 1;  // Breite eines Selects neu setzen?
    sal_Bool bTextArea : 1;
    // 24
    sal_Bool bSelect : 1;
    sal_Bool bInFootEndNoteAnchor : 1;
    sal_Bool bInFootEndNoteSymbol : 1;
    sal_Bool bIgnoreHTMLComments : 1;
    sal_Bool bRemoveHidden : 1; // the filter implementation might set the hidden flag

    /// the names corresponding to the DOCINFO field subtypes INFO[1-4]
    OUString m_InfoNames[4];

    SfxViewFrame* pTempViewFrame;

    void DeleteFormImpl();

    void DocumentDetected();
    void Show();
    void ShowStatline();
    ViewShell *CallStartAction( ViewShell *pVSh = 0, sal_Bool bChkPtr = sal_True );
    ViewShell *CallEndAction( sal_Bool bChkAction = sal_False, sal_Bool bChkPtr = sal_True );
    ViewShell *CheckActionViewShell();

    DECL_LINK( AsyncCallback, void* );

    // Attribute am Dok setzen
    void _SetAttr( sal_Bool bChkEnd, sal_Bool bBeforeTable, _HTMLAttrs *pPostIts );
    inline void SetAttr( sal_Bool bChkEnd = sal_True, sal_Bool bBeforeTable = sal_False,
                         _HTMLAttrs *pPostIts = 0 )
    {
        if( !aSetAttrTab.empty() || !aMoveFlyFrms.empty() )
            _SetAttr( bChkEnd, bBeforeTable, pPostIts );
    }

    _HTMLAttr **GetAttrTabEntry( sal_uInt16 nWhich );

    // Einen neuen Textknoten an PaM-Position anlegen
    sal_Bool AppendTxtNode( SwHTMLAppendMode eMode=AM_NORMAL, sal_Bool bUpdateNum=sal_True );
    void AddParSpace();

    // Ein Attribut beginnen/beenden
    // ppDepAttr gibt einen Attribut-Tabellen-Eintrag an, dessen Attribute
    // gesetzt sein muessen, bevor das Attribut beendet werden darf
    void NewAttr( _HTMLAttr **ppAttr, const SfxPoolItem& rItem );
    void EndAttr( _HTMLAttr *pAttr, _HTMLAttr **ppDepAttr=0,
                  sal_Bool bChkEmpty=sal_True );
    void DeleteAttr( _HTMLAttr* pAttr );

    void EndContextAttrs( _HTMLAttrContext *pContext, sal_Bool bRemove=sal_False );
    void SaveAttrTab( _HTMLAttrTable& rNewAttrTab );
    void SplitAttrTab( const SwPosition& rNewPos );
    void SplitAttrTab( _HTMLAttrTable& rNewAttrTab, sal_Bool bMoveEndBack = sal_True );
    void RestoreAttrTab( const _HTMLAttrTable& rNewAttrTab,
                         sal_Bool bSetNewStart = sal_False );
    void InsertAttr( const SfxPoolItem& rItem, sal_Bool bLikePara = sal_False,
                     sal_Bool bInsAtStart=sal_False );
    void InsertAttrs( _HTMLAttrs& rAttrs );

    sal_Bool DoPositioning( SfxItemSet &rItemSet,
                        SvxCSS1PropertyInfo &rPropInfo,
                        _HTMLAttrContext *pContext );
    sal_Bool CreateContainer( const OUString& rClass, SfxItemSet &rItemSet,
                          SvxCSS1PropertyInfo &rPropInfo,
                          _HTMLAttrContext *pContext );
    sal_Bool EndSection( sal_Bool bLFStripped=sal_False );

    void InsertAttrs( SfxItemSet &rItemSet, SvxCSS1PropertyInfo &rPropInfo,
                      _HTMLAttrContext *pContext, sal_Bool bCharLvl=sal_False );
    void InsertAttr( _HTMLAttr **ppAttr, const SfxPoolItem & rItem,
                     _HTMLAttrContext *pCntxt );
    void SplitPREListingXMP( _HTMLAttrContext *pCntxt );
    void FixHeaderFooterDistance( sal_Bool bHeader, const SwPosition *pOldPos );

    void EndContext( _HTMLAttrContext *pContext );
    void ClearContext( _HTMLAttrContext *pContext );

    const SwFmtColl *GetCurrFmtColl() const;

    SwTwips GetCurrentBrowseWidth();

    SwHTMLNumRuleInfo& GetNumInfo() { return *pNumRuleInfo; }
    // add parameter <bCountedInList>
    void SetNodeNum( sal_uInt8 nLevel, bool bCountedInList );

    // Verwalten von Absatz-Vorlagen

    // die Vorlagen auf dem Stack bzw. deren Attribute setzen
    void SetTxtCollAttrs( _HTMLAttrContext *pContext = 0 );

    void InsertParaAttrs( const SfxItemSet& rItemSet );

    // Verwalten des Attribut-Kontexts

    // aktuellen Kontext merken
    inline void PushContext( _HTMLAttrContext *pCntxt );

    // den obersten/spezifizierten Kontext holen, aber nicht ausserhalb
    // des Kontexts mit Token nLimit suchen. Wenn bRemove gesetzt ist,
    // wird er entfernt
    _HTMLAttrContext *PopContext( sal_uInt16 nToken=0, sal_uInt16 nLimit=0,
                                  sal_Bool bRemove=sal_True );
    inline const _HTMLAttrContext *GetTopContext() const;

    sal_Bool GetMarginsFromContext( sal_uInt16 &nLeft, sal_uInt16 &nRight, short& nIndent,
                                sal_Bool bIgnoreCurrent=sal_False ) const;
    sal_Bool GetMarginsFromContextWithNumBul( sal_uInt16 &nLeft, sal_uInt16 &nRight,
                                          short& nIndent ) const;
    void GetULSpaceFromContext( sal_uInt16 &rUpper, sal_uInt16 &rLower ) const;


    void MovePageDescAttrs( SwNode *pSrcNd, sal_uLong nDestIdx, sal_Bool bFmtBreak );

    // Behandlung von Tags auf Absatz-Ebene

    // <P> und <H1> bis <H6>
    void NewPara();
    void EndPara( sal_Bool bReal = sal_False );
    void NewHeading( int nToken );
    void EndHeading();

    // <ADDRESS>, <BLOCKQUOTE> und <PRE>
    void NewTxtFmtColl( int nToken, sal_uInt16 nPoolId );
    void EndTxtFmtColl( int nToken );

    // <DIV> und <CENTER>
    void NewDivision( int nToken );
    void EndDivision( int nToken );

    // Fly-Frames einfuegen/verlassen
    void InsertFlyFrame( const SfxItemSet& rItemSet, _HTMLAttrContext *pCntxt,
                         const String& rId, sal_uInt16 nFlags );

    void SaveDocContext( _HTMLAttrContext *pCntxt, sal_uInt16 nFlags,
                       const SwPosition *pNewPos );
    void RestoreDocContext( _HTMLAttrContext *pCntxt );

    // alle durch <DIV> aufgespannten Bereiche verlassen
    sal_Bool EndSections( sal_Bool bLFStripped );

    // <MULTICOL>
    void NewMultiCol( sal_uInt16 columnsFromCss=0 );
    void EndMultiCol();

    // <MARQUEE>
    void NewMarquee( HTMLTable *pCurTable=0 );
    void EndMarquee();
    void InsertMarqueeText();

    // Behandluung von Listen

    // Numerierungs <OL> und Aufzaehlungs-Listen <UL> mit <LI>
    void NewNumBulList( int nToken );
    void EndNumBulList( int nToken=0 );
    void NewNumBulListItem( int nToken );
    void EndNumBulListItem( int nToken=0, sal_Bool bSetColl=sal_True,
                            sal_Bool bLastPara=sal_False );

    // Definitions-Listen <DL> mit <DD>, <DT>
    void NewDefList();
    void EndDefList();
    void NewDefListItem( int nToken );
    void EndDefListItem( int nToken=0, sal_Bool bSetColl=sal_True,
                         sal_Bool bLastPara=sal_False );


    // Behandlung von Tags auf Zeichen-Ebene

    // Tags wie <B>, <I> etc behandeln, die ein bestimmtes Attribut
    // an und ausschalten, oder die wie SPAN nur Attribute aus Styles holen
    void NewStdAttr( int nToken );
    void NewStdAttr( int nToken,
                     _HTMLAttr **ppAttr, const SfxPoolItem & rItem,
                     _HTMLAttr **ppAttr2=0, const SfxPoolItem *pItem2=0,
                     _HTMLAttr **ppAttr3=0, const SfxPoolItem *pItem3=0 );
    void EndTag( int nToken );

    // Font-Attribute behandeln
    void NewBasefontAttr();             // fuer <BASEFONT>
    void EndBasefontAttr();
    void NewFontAttr( int nToken ); // fuer <FONT>, <BIG> und <SMALL>
    void EndFontAttr( int nToken );

    // Tags, die durch Zeichenvorlagen realisiert werden
    void NewCharFmt( int nToken );

    // <SDFIELD>
public:
    static sal_uInt16 GetNumType( const String& rStr, sal_uInt16 eDfltType );
private:
    void NewField();
    void EndField();
    void InsertFieldText();

    // <SPACER>
    void InsertSpacer();

    // Einfuegen von Grafiken, Plugins und Applets

    // Image-Maps suchen und mit Grafik-Nodes verbinden
    ImageMap *FindImageMap( const String& rURL ) const;
    void ConnectImageMaps();

    // Verankerung eines Fly-Frames bestimmen und entsprechende Attribute
    // in den Attrset setzen (htmlgrin.cxx)
    void SetAnchorAndAdjustment( sal_Int16 eVertOri,
                                 sal_Int16 eHoriOri,
                                 const SfxItemSet &rItemSet,
                                 const SvxCSS1PropertyInfo &rPropInfo,
                                 SfxItemSet& rFrmSet );
    void SetAnchorAndAdjustment( sal_Int16 eVertOri,
                                 sal_Int16 eHoriOri,
                                 SfxItemSet& rFrmSet,
                                 sal_Bool bDontAppend=sal_False );
    void SetAnchorAndAdjustment( const SfxItemSet &rItemSet,
                                 const SvxCSS1PropertyInfo &rPropInfo,
                                 SfxItemSet &rFrmItemSet );

    void SetFrmFmtAttrs( SfxItemSet &rItemSet, SvxCSS1PropertyInfo &rPropInfo,
                         sal_uInt16 nFlags, SfxItemSet &rFrmItemSet );

    // Frames anlegen und Auto-gebundene Rahmen registrieren
    void RegisterFlyFrm( SwFrmFmt *pFlyFrm );

    // Die Groesse des Fly-Frames an die Vorgaben und Gegebenheiten anpassen
    // (nicht fuer Grafiken, deshalb htmlplug.cxx)
    void SetFixSize( const Size& rPixSize, const Size& rTwipDfltSize,
                     sal_Bool bPrcWidth, sal_Bool bPrcHeight,
                     SfxItemSet &rItemSet, SvxCSS1PropertyInfo &rPropInfo,
                     SfxItemSet& rFlyItemSet );
    void SetVarSize( SfxItemSet &rItemSet, SvxCSS1PropertyInfo &rPropInfo,
                     SfxItemSet& rFlyItemSet, SwTwips nDfltWidth=MINLAY,
                     sal_uInt8 nDltPrcWidth=0 );
    void SetSpace( const Size& rPixSpace, SfxItemSet &rItemSet,
                   SvxCSS1PropertyInfo &rPropInfo, SfxItemSet& rFlyItemSet );

    sal_uInt16 IncGrfsThatResizeTable();

    void GetDefaultScriptType( ScriptType& rType,
                               OUString& rTypeStr ) const;

    // die eigentlichen Einfuege-Methoden fuer <IMG>, <EMBED> und <APPLET>
    // und <PARAM>
    void InsertImage();     // htmlgrin.cxx
    void InsertEmbed();     // htmlplug.cxx

#ifdef SOLAR_JAVA
    void NewObject();   // htmlplug.cxx
#endif
    void EndObject();       // CommandLine mit Applet verkn. (htmlplug.cxx)
#ifdef SOLAR_JAVA
    void InsertApplet();    // htmlplug.cxx
#endif
    void EndApplet();       // CommandLine mit Applet verkn. (htmlplug.cxx)
    void InsertParam();     // htmlplug.cxx

    void InsertFloatingFrame();
    void EndFloatingFrame() { bInFloatingFrame = sal_False; }

    // <BODY>-Tag auswerten: Hintergrund-Grafiken und -Farben setzen (htmlgrin.cxx)
    void InsertBodyOptions();


    // Einfuegen von Links und ::com::sun::star::text::Bookmarks (htmlgrin.cxx)

    // <A>-Tag auswerten: einen Link bzw. eine ::com::sun::star::text::Bookmark einfuegen
    void NewAnchor();
    void EndAnchor();

    // eine ::com::sun::star::text::Bookmark einfuegen
    void InsertBookmark( const String& rName );


    void InsertCommentText( const sal_Char *pTag = 0 );
    void InsertComment( const String& rName, const sal_Char *pTag = 0 );

    // sind im aktuellen Absatz ::com::sun::star::text::Bookmarks vorhanden?
    sal_Bool HasCurrentParaBookmarks( sal_Bool bIgnoreStack=sal_False ) const;


    // Einfuegen von Script/Basic-Elementen

    // das zueletzt gelsene Basic-Modul parsen (htmlbas.cxx)
    void NewScript();
    void EndScript();

    void AddScriptSource();

    // ein Event in die SFX-Konfiguation eintragen (htmlbas.cxx)
    void InsertBasicDocEvent( OUString aEventName, const OUString& rName,
                              ScriptType eScrType, const OUString& rScrType );

    // ein Event an ein VC-Control anhaengen (htmlform.cxx)
    void InsertBasicCtrlEvent( sal_uInt16 nEvent, const String& rName );

    // Inserting styles

    // <STYLE>
    void NewStyle();
    void EndStyle();

    inline sal_Bool HasStyleOptions( const String &rStyle, const String &rId,
                                 const String &rClass, const String *pLang=0,
                                    const String *pDir=0 );
    sal_Bool ParseStyleOptions( const String &rStyle, const String &rId,
                            const String &rClass, SfxItemSet &rItemSet,
                            SvxCSS1PropertyInfo &rPropInfo,
                             const String *pLang=0, const String *pDir=0 );


    // Inserting Controls and ::com::sun::star::form::Forms (htmlform.cxx)

    // Insert draw object into document
    void InsertDrawObject( SdrObject* pNewDrawObj, const Size& rSpace,
                           sal_Int16 eVertOri,
                           sal_Int16 eHoriOri,
                           SfxItemSet& rCSS1ItemSet,
                           SvxCSS1PropertyInfo& rCSS1PropInfo,
                           sal_Bool bHidden=sal_False );
                        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >  InsertControl( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent > & rFormComp,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rFCompPropSet,
                        const Size& rSize,
                        sal_Int16 eVertOri,
                        sal_Int16 eHoriOri,
                        SfxItemSet& rCSS1ItemSet,
                        SvxCSS1PropertyInfo& rCSS1PropInfo,
                        const SvxMacroTableDtor& rMacroTbl,
                        const std::vector<OUString>& rUnoMacroTbl,
                        const std::vector<OUString>& rUnoMacroParamTbl,
                        sal_Bool bSetPropSet = sal_True,
                        sal_Bool bHidden = sal_False );
    void SetControlSize( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & rShape, const Size& rTextSz,
                         sal_Bool bMinWidth, sal_Bool bMinHeight );
    void SetPendingControlSize();

public:
    void ResizeDrawObject( SdrObject* pObj, SwTwips nWidth );
private:
    void RegisterDrawObjectToTable( HTMLTable *pCurTable, SdrObject* pObj,
                                    sal_uInt8 nWidth );

    void NewForm( sal_Bool bAppend=sal_True );
    void EndForm( sal_Bool bAppend=sal_True );

    // Insert methods for <INPUT>, <TEXTAREA> und <SELECT>
    void InsertInput();

    void NewTextArea();
    void InsertTextAreaText( sal_uInt16 nToken );
    void EndTextArea();

    void NewSelect();
    void InsertSelectOption();
    void InsertSelectText();
    void EndSelect();

    // Inserting tables (htmltab.cxx)
public:

    // Insert box content after the given node
    const SwStartNode *InsertTableSection( const SwStartNode *pPrevStNd );

    // Insert box content at the end of the table containing the PaM
    // and move the PaM into the cell
    const SwStartNode *InsertTableSection( sal_uInt16 nPoolId );

    // Insert methods for various table tags
    HTMLTableCnts *InsertTableContents( sal_Bool bHead );

private:
    // Create a section for the temporary storage of the table caption
    SwStartNode *InsertTempTableCaptionSection();

    void BuildTableCell( HTMLTable *pTable, sal_Bool bReadOptions, sal_Bool bHead );
    void BuildTableRow( HTMLTable *pTable, sal_Bool bReadOptions,
                        SvxAdjust eGrpAdjust, sal_Int16 eVertOri );
    void BuildTableSection( HTMLTable *pTable, sal_Bool bReadOptions, sal_Bool bHead );
    void BuildTableColGroup( HTMLTable *pTable, sal_Bool bReadOptions );
    void BuildTableCaption( HTMLTable *pTable );
    HTMLTable *BuildTable( SvxAdjust eCellAdjust,
                           sal_Bool bIsParentHead = sal_False,
                           sal_Bool bHasParentSection=sal_True,
                           sal_Bool bIsInMulticol = sal_False,
                           sal_Bool bHasToFlow = sal_False );


    // misc ...

    void ParseMoreMetaOptions();

    sal_Bool FileDownload( const String& rURL, OUString& rStr );
    void InsertLink();

    void InsertIDOption();
    void InsertLineBreak();
    void InsertHorzRule();

    void FillEndNoteInfo( const OUString& rContent );
    void FillFootNoteInfo( const OUString& rContent );
    void InsertFootEndNote( const OUString& rName, sal_Bool bEndNote, sal_Bool bFixed );
    void FinishFootEndNote();
    void InsertFootEndNoteText();
    SwNodeIndex *GetFootEndNoteSection( const String& rName );
    void DeleteFootEndNoteImpl();

    xub_StrLen StripTrailingLF();

    // Remove empty paragraph at the PaM position
    void StripTrailingPara();

    // Are there fly frames in the current paragraph?
    sal_Bool HasCurrentParaFlys( sal_Bool bNoSurroundOnly = sal_False,
                             sal_Bool bSurroundOnly = sal_False ) const;

public:         // used in tables

    // Create brush item (with new) or 0
    SvxBrushItem* CreateBrushItem( const Color *pColor,
                                   const String &rImageURL,
                                   const String &rStyle,
                                   const String &rId,
                                   const String &rClass );

protected:
    // Executed for each token recognized by CallParser
    virtual void NextToken( int nToken );
    virtual ~SwHTMLParser();

    // If the document is removed, remove the parser as well
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew );

    virtual void AddMetaUserDefined( OUString const & i_rMetaName );

public:

    SwHTMLParser( SwDoc* pD, SwPaM & rCrsr, SvStream& rIn,
                    const OUString& rFileName,
                    const OUString& rBaseURL,
                    int bReadNewDoc = sal_True,
                    SfxMedium* pMed = 0, sal_Bool bReadUTF8 = sal_False,
                    sal_Bool bIgnoreHTMLComments = sal_False );

    virtual SvParserState CallParser();


    sal_uInt16 ToTwips( sal_uInt16 nPixel ) const;

    // for reading asynchronously from SvStream
    virtual void Continue( int nToken );

    virtual bool ParseMetaOptions( const ::com::sun::star::uno::Reference<
                ::com::sun::star::document::XDocumentProperties>&,
            SvKeyValueIterator* );
};


struct SwPendingStackData
{
    virtual ~SwPendingStackData() {}
};

struct SwPendingStack
{
    int nToken;
    SwPendingStackData* pData;
    SwPendingStack* pNext;

    SwPendingStack( int nTkn, SwPendingStack* pNxt )
        : nToken( nTkn ), pData( 0 ), pNext( pNxt )
        {}
};

inline void _HTMLAttr::SetStart( const SwPosition& rPos )
{
    nSttPara = rPos.nNode;
    nSttCntnt = rPos.nContent.GetIndex();
    nEndPara = nSttPara;
    nEndCntnt = nSttCntnt;
}

inline void _HTMLAttrContext::SetMargins( sal_uInt16 nLeft, sal_uInt16 nRight,
                                          short nIndent )
{
    nLeftMargin = nLeft;
    nRightMargin = nRight;
    nFirstLineIndent = nIndent;
    bLRSpaceChanged = sal_True;
}

inline void _HTMLAttrContext::GetMargins( sal_uInt16& nLeft,
                                          sal_uInt16& nRight,
                                          short& nIndent ) const
{
    if( bLRSpaceChanged )
    {
        nLeft = nLeftMargin;
        nRight = nRightMargin;
        nIndent = nFirstLineIndent;
    }
}

inline void _HTMLAttrContext::SetULSpace( sal_uInt16 nUpper, sal_uInt16 nLower )
{
    nUpperSpace = nUpper;
    nLowerSpace = nLower;
    bULSpaceChanged = sal_True;
}

inline void _HTMLAttrContext::GetULSpace( sal_uInt16& rUpper,
                                          sal_uInt16& rLower ) const
{
    if( bULSpaceChanged )
    {
        rUpper = nUpperSpace;
        rLower = nLowerSpace;
    }
}

inline sal_Bool SwHTMLParser::HasStyleOptions( const String &rStyle,
                                            const String &rId,
                                            const String &rClass,
                                            const String *pLang,
                                               const String *pDir )
{
    return rStyle.Len() || rId.Len() || rClass.Len() ||
           (pLang && pLang->Len()) || (pDir && pDir->Len());
}

inline const _HTMLAttrContext *SwHTMLParser::GetTopContext() const
{
    return aContexts.size() > nContextStMin
            ? aContexts.back() : 0;
}

inline void SwHTMLParser::PushContext( _HTMLAttrContext *pCntxt )
{
    aContexts.push_back( pCntxt );
}


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
