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
#ifndef INCLUDED_SW_SOURCE_FILTER_HTML_SWHTML_HXX
#define INCLUDED_SW_SOURCE_FILTER_HTML_SWHTML_HXX

#include <config_features.h>

#include <sfx2/sfxhtml.hxx>
#include <svl/macitem.hxx>
#include <editeng/svxenum.hxx>
#include <fmtornt.hxx>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/form/XFormComponent.hpp>

#include "calbck.hxx"
#include "htmlvsh.hxx"

#include <memory>
#include <vector>
#include <deque>

class SfxMedium;
class SfxViewFrame;
class SdrObject;
class SvxMacroTableDtor;
class SwDoc;
class SwPaM;
class SwViewShell;
class SwStartNode;
class SwFormatColl;
class SwField;
class SwHTMLForm_Impl;
class SwApplet_Impl;
struct SwHTMLFootEndNote_Impl;
class HTMLTableCnts;
struct SwPendingStack;
class SvxCSS1PropertyInfo;
struct ImplSVEvent;

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
    sal_Int32 nSttContent, nEndContent;
    bool bInsAtStart : 1;
    bool bLikePara : 1; // Attribut ueber dem gesamten Absatz setzen
    bool bValid : 1;    // ist das Attribut gueltig?

    SfxPoolItem* pItem;
    sal_uInt16 nCount;      // Anzahl noch zu schliessender Attrs mit einem Wert
    _HTMLAttr *pNext;   // noch zu schliessene Attrs mit unterschiedl. Werten
    _HTMLAttr *pPrev;   // bereits geschlossene aber noch nicht gesetze Attrs
    _HTMLAttr **ppHead; // der Listenkopf

    _HTMLAttr( const SwPosition& rPos, const SfxPoolItem& rItem,
               _HTMLAttr **pHd=0 );

    _HTMLAttr( const _HTMLAttr &rAttr, const SwNodeIndex &rEndPara,
               sal_Int32 nEndCnt, _HTMLAttr **pHd );

public:

    ~_HTMLAttr();

    _HTMLAttr *Clone( const SwNodeIndex& rEndPara, sal_Int32 nEndCnt ) const;
    void Reset( const SwNodeIndex& rSttPara, sal_Int32 nSttCnt,
                _HTMLAttr **pHd );
    inline void SetStart( const SwPosition& rPos );

    sal_uInt32 GetSttParaIdx() const { return nSttPara.GetIndex(); }
    sal_uInt32 GetEndParaIdx() const { return nEndPara.GetIndex(); }

    const SwNodeIndex& GetSttPara() const { return nSttPara; }
    const SwNodeIndex& GetEndPara() const { return nEndPara; }

    sal_Int32 GetSttCnt() const { return nSttContent; }
    sal_Int32 GetEndCnt() const { return nEndContent; }

    bool IsLikePara() const { return bLikePara; }
    void SetLikePara( bool bPara=true ) { bLikePara = bPara; }

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
    void Invalidate() { bValid = false; }
};

// Tabelle der Attribute: Hier ist die Reihenfolge wichtig: Die Attribute
// vorne in der Tabelle werden in EndAllAttrs auch zuerst gesetzt.
struct _HTMLAttrTable
{
    _HTMLAttr
                *pKeep,         // Frame-Attribute
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

                *pCharFormats,     // Text-Attribute
                *pINetFormat,

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
                *pLanguageCTL,
                *pCharBox
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

    OUString    aClass;          // die Klasse des Kontexts

    _HTMLAttrContext_SaveDoc *pSaveDocContext;
    SfxItemSet *pFrmItemSet;

    sal_uInt16  nToken;         // das Token, zu dem der Kontext gehoehrt

    sal_uInt16  nTextFormatColl;    // eine in dem Kontext begonnene Vorlage oder 0

    sal_uInt16  nLeftMargin;        // ein veraenderter linker Rand
    sal_uInt16  nRightMargin;       // ein veraenderter rechter Rand
    sal_uInt16  nFirstLineIndent;   // ein veraenderter Erstzeilen-Einzug

    sal_uInt16  nUpperSpace;
    sal_uInt16  nLowerSpace;

    SwHTMLAppendMode eAppend;

    bool    bLRSpaceChanged : 1;// linker/rechtr Rand, Einzug veraendert?
    bool    bULSpaceChanged : 1;// oberer/unterer Rand veraendert?
    bool    bDfltTextFormatColl : 1;// nTextFormatColl ist nur ein default
    bool    bSpansSection : 1;  // Der Kontext spannt eine SwSection auf
    bool    bPopStack : 1;      // Oberhalb liegende Stack-Elemente entf.
    bool    bFinishPREListingXMP : 1;
    bool    bRestartPRE : 1;
    bool    bRestartXMP : 1;
    bool    bRestartListing : 1;

public:
    void ClearSaveDocContext();

    _HTMLAttrContext( sal_uInt16 nTokn, sal_uInt16 nPoolId, const OUString& rClass,
                      bool bDfltColl=false ) :
        aClass( rClass ),
        pSaveDocContext( 0 ),
        pFrmItemSet( 0 ),
        nToken( nTokn ),
        nTextFormatColl( nPoolId ),
        nLeftMargin( 0 ),
        nRightMargin( 0 ),
        nFirstLineIndent( 0 ),
        nUpperSpace( 0 ),
        nLowerSpace( 0 ),
        eAppend( AM_NONE ),
        bLRSpaceChanged( false ),
        bULSpaceChanged( false ),
        bDfltTextFormatColl( bDfltColl ),
        bSpansSection( false ),
        bPopStack( false ),
        bFinishPREListingXMP( false ),
        bRestartPRE( false ),
        bRestartXMP( false ),
        bRestartListing( false )
    {}

    explicit _HTMLAttrContext( sal_uInt16 nTokn ) :
        pSaveDocContext( 0 ),
        pFrmItemSet( 0 ),
        nToken( nTokn ),
        nTextFormatColl( 0 ),
        nLeftMargin( 0 ),
        nRightMargin( 0 ),
        nFirstLineIndent( 0 ),
        nUpperSpace( 0 ),
        nLowerSpace( 0 ),
        eAppend( AM_NONE ),
        bLRSpaceChanged( false ),
        bULSpaceChanged( false ),
        bDfltTextFormatColl( false ),
        bSpansSection( false ),
        bPopStack( false ),
        bFinishPREListingXMP( false ),
        bRestartPRE( false ),
        bRestartXMP( false ),
        bRestartListing( false )
    {}

    ~_HTMLAttrContext() { ClearSaveDocContext(); delete pFrmItemSet; }

    sal_uInt16 GetToken() const { return nToken; }

    sal_uInt16 GetTextFormatColl() const { return bDfltTextFormatColl ? 0 : nTextFormatColl; }
    sal_uInt16 GetDfltTextFormatColl() const { return bDfltTextFormatColl ? nTextFormatColl : 0; }

    const OUString& GetClass() const { return aClass; }

    inline void SetMargins( sal_uInt16 nLeft, sal_uInt16 nRight, short nIndent );

    inline bool IsLRSpaceChanged() const { return bLRSpaceChanged; }
    inline void GetMargins( sal_uInt16& nLeft, sal_uInt16& nRight,
                            short &nIndent ) const;

    inline void SetULSpace( sal_uInt16 nUpper, sal_uInt16 nLower );
    inline bool IsULSpaceChanged() const { return bULSpaceChanged; }
    inline void GetULSpace( sal_uInt16& rUpper, sal_uInt16& rLower ) const;

    bool HasAttrs() const { return !aAttrs.empty(); }
    const _HTMLAttrs& GetAttrs() const { return aAttrs; }
    _HTMLAttrs& GetAttrs() { return aAttrs; }

    void SetSpansSection( bool bSet ) { bSpansSection = bSet; }
    bool GetSpansSection() const { return bSpansSection; }

    void SetPopStack( bool bSet ) { bPopStack = bSet; }
    bool GetPopStack() const { return bPopStack; }

    bool HasSaveDocContext() const { return pSaveDocContext!=0; }
    _HTMLAttrContext_SaveDoc *GetSaveDocContext( bool bCreate=false );

    const SfxItemSet *GetFrmItemSet() const { return pFrmItemSet; }
    SfxItemSet *GetFrmItemSet( SwDoc *pCreateDoc );

    void SetFinishPREListingXMP( bool bSet ) { bFinishPREListingXMP = bSet; }
    bool IsFinishPREListingXMP() const { return bFinishPREListingXMP; }

    void SetRestartPRE( bool bSet ) { bRestartPRE = bSet; }
    bool IsRestartPRE() const { return bRestartPRE; }

    void SetRestartXMP( bool bSet ) { bRestartXMP = bSet; }
    bool IsRestartXMP() const { return bRestartXMP; }

    void SetRestartListing( bool bSet ) { bRestartListing = bSet; }
    bool IsRestartListing() const { return bRestartListing; }

    void SetAppendMode( SwHTMLAppendMode eMode=AM_NORMAL ) { eAppend = eMode; }
    SwHTMLAppendMode GetAppendMode() const { return eAppend; }
};

typedef std::vector<_HTMLAttrContext *> _HTMLAttrContexts;

class HTMLTable;
class SwCSS1Parser;
class SwHTMLNumRuleInfo;

typedef ::std::vector<std::unique_ptr<ImageMap>> ImageMaps;

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

    OUString      m_aPathToFile;
    OUString      m_sBaseURL;
    OUString      m_sSaveBaseURL;
    OUString      m_aBasicLib;
    OUString      m_aBasicModule;
    OUString      m_aScriptSource;  // Inhalt des aktuellen Script-Blocks
    OUString      m_aScriptType;    // Type des gelesenen Scripts (StarBasic/VB/JAVA)
    OUString      m_aScriptURL;     // URL eines Scripts
    OUString      m_aStyleSource;   // Inhalt des aktuellen Style-Sheets
    OUString      m_aContents;      // Text des akteullen Marquee, Feldes etc.
    OUString      m_sTitle;
    OUString      m_aUnknownToken;  // ein gestartetes unbekanntes Token
    OUString      m_aBulletGrfs[MAXLEVEL];
    OUString      m_sJmpMark;

    std::vector<sal_uInt16>   m_aBaseFontStack; // Stack fuer <BASEFONT>
                                // Bit 0-2: Fontgroesse (1-7)
    std::vector<sal_uInt16>   m_aFontStack;     // Stack fuer <FONT>, <BIG>, <SMALL>
                                // Bit 0-2: Fontgroesse (1-7)
                                // Bit 15: Fontfarbe wurde gesetzt

    _HTMLAttrs      m_aSetAttrTab;// "geschlossene", noch nicht gesetzte Attr.
    _HTMLAttrs      m_aParaAttrs; // vorlauefige Absatz-Attribute
    _HTMLAttrTable  m_aAttrTab;   // "offene" Attribute
    _HTMLAttrContexts m_aContexts;// der aktuelle Attribut/Token-Kontext
    std::vector<SwFrameFormat *> m_aMoveFlyFrms;// Fly-Frames, the anchor is moved
    std::deque<sal_Int32> m_aMoveFlyCnts;// and the Content-Positions

    SwApplet_Impl *m_pAppletImpl; // das aktuelle Applet

    SwCSS1Parser    *m_pCSS1Parser;   // der Style-Sheet-Parser
    SwHTMLNumRuleInfo *m_pNumRuleInfo;
    SwPendingStack  *m_pPendStack;

    SwDoc           *m_pDoc;
    SwPaM           *m_pPam;      // SwPosition duerfte doch reichen, oder ??
    SwViewShell       *m_pActionViewShell;  // SwViewShell, an der das StartAction
                                        // gerufen wurde.
    SwNodeIndex     *m_pSttNdIdx;

    HTMLTable       *m_pTable;    // die aktuelle "auesserste" Tabelle
    SwHTMLForm_Impl *m_pFormImpl;// die aktuelle Form
    SdrObject       *m_pMarquee;  // aktuelles Marquee
    SwField         *m_pField;    // aktuelles Feld
    ImageMap        *m_pImageMap; // aktuelle Image-Map
    ImageMaps       *m_pImageMaps; ///< all Image-Maps that have been read
    SwHTMLFootEndNote_Impl *m_pFootEndNoteImpl;

    Size    m_aHTMLPageSize;      // die Seitengroesse der HTML-Vorlage

    sal_uInt32  m_aFontHeights[7];    // die Font-Hoehen 1-7
    sal_uInt32  m_nScriptStartLineNr; // Zeilennummer eines Script-Blocks
    ImplSVEvent * m_nEventId;

    sal_uInt16  m_nBaseFontStMin;
    sal_uInt16  m_nFontStMin;
    sal_uInt16  m_nDefListDeep;
    sal_uInt16  m_nFontStHeadStart;   // Elemente im Font-Stack bei <Hn>
    sal_uInt16  m_nSBModuleCnt;       // Zaehler fuer Basic-Module
    sal_uInt16  m_nMissingImgMaps;    // Wie viele Image-Maps fehlen noch?
    size_t m_nParaCnt;
    size_t m_nContextStMin;           // Untergrenze fuer PopContext
    size_t m_nContextStAttrMin;       // Untergrenze fuer Attributierung
    sal_uInt16  m_nSelectEntryCnt;    // Anzahl der Eintraege der akt. Listbox
    sal_uInt16  m_nOpenParaToken;     // ein geoeffnetes Absatz-Element

    enum JumpToMarks { JUMPTO_NONE, JUMPTO_MARK, JUMPTO_TABLE, JUMPTO_FRAME,
                        JUMPTO_REGION, JUMPTO_GRAPHIC } m_eJumpTo;

#ifdef DBG_UTIL
    sal_uInt16  m_nContinue;        // depth of Continue calls
#endif

    SvxAdjust   m_eParaAdjust;    // Ausrichtung des aktuellen Absatz
    HTMLScriptLanguage m_eScriptLang; // die aktuelle Script-Language

    bool m_bOldIsHTMLMode : 1;    // War's mal ein HTML-Dokument?

    bool m_bDocInitalized : 1;    // Dokument bzw. Shell wurden initialisiert
                                // Flag um doppeltes init durch Rekursion
                                // zu verhindern.
    bool m_bViewCreated : 1;      // die View wurde schon erzeugt (asynchron)
    bool m_bSetCrsr : 1;          // Crsr wieder auf den Anfang setzen
    bool m_bSetModEnabled : 1;

    bool m_bInFloatingFrame : 1;  // Wir sind in einen Floating Frame
    bool m_bInField : 1;
    bool m_bKeepUnknown : 1;      // unbekannte/nicht unterstuetze Tokens beh.
    // 8
    bool m_bCallNextToken : 1;    // In Tabellen: NextToken in jedem Fall rufen
    bool m_bIgnoreRawData : 1;    // Inhalt eines Scripts/Styles ignorieren.
    bool m_bLBEntrySelected : 1;  // Ist der aktuelle Listbox-Eintrag selekt.
    bool m_bTAIgnoreNewPara : 1;  // naechstes LF in TextArea ignorieren?
    bool m_bFixMarqueeWidth : 1;  // Groesse einer Laufschrift anpassen?
    bool m_bFixMarqueeHeight : 1;

    bool m_bUpperSpace : 1;       // obererer Absatz-Abstand wird benoetigt
    bool m_bNoParSpace : 1;
    // 16

    bool m_bAnyStarBasic : 1;     // gibt es ueberhaupt ein StarBasic-Modul
    bool m_bInNoEmbed : 1;        // Wir sind in einem NOEMBED-Bereich

    bool m_bInTitle : 1;          // Wir sind im Titel

    bool m_bChkJumpMark : 1;      // springe ggfs. zu einem vorgegebenem Mark
    bool m_bUpdateDocStat : 1;
    bool m_bFixSelectWidth : 1;   // Breite eines Selects neu setzen?
    bool m_bFixSelectHeight : 1;  // Breite eines Selects neu setzen?
    bool m_bTextArea : 1;
    // 24
    bool m_bSelect : 1;
    bool m_bInFootEndNoteAnchor : 1;
    bool m_bInFootEndNoteSymbol : 1;
    bool m_bIgnoreHTMLComments : 1;
    bool m_bRemoveHidden : 1; // the filter implementation might set the hidden flag

    /// the names corresponding to the DOCINFO field subtypes INFO[1-4]
    OUString m_InfoNames[4];

    SfxViewFrame* m_pTempViewFrame;

    void DeleteFormImpl();

    void DocumentDetected();
    void Show();
    void ShowStatline();
    SwViewShell *CallStartAction( SwViewShell *pVSh = 0, bool bChkPtr = true );
    SwViewShell *CallEndAction( bool bChkAction = false, bool bChkPtr = true );
    SwViewShell *CheckActionViewShell();

    DECL_LINK_TYPED( AsyncCallback, void*, void );

    // Attribute am Dok setzen
    void _SetAttr( bool bChkEnd, bool bBeforeTable, _HTMLAttrs *pPostIts );
    inline void SetAttr( bool bChkEnd = true, bool bBeforeTable = false,
                         _HTMLAttrs *pPostIts = 0 )
    {
        if( !m_aSetAttrTab.empty() || !m_aMoveFlyFrms.empty() )
            _SetAttr( bChkEnd, bBeforeTable, pPostIts );
    }

    _HTMLAttr **GetAttrTabEntry( sal_uInt16 nWhich );

    // Einen neuen Textknoten an PaM-Position anlegen
    bool AppendTextNode( SwHTMLAppendMode eMode=AM_NORMAL, bool bUpdateNum=true );
    void AddParSpace();

    // Ein Attribut beginnen/beenden
    // ppDepAttr gibt einen Attribut-Tabellen-Eintrag an, dessen Attribute
    // gesetzt sein muessen, bevor das Attribut beendet werden darf
    void NewAttr( _HTMLAttr **ppAttr, const SfxPoolItem& rItem );
    bool EndAttr( _HTMLAttr *pAttr, _HTMLAttr **ppDepAttr=0,
                  bool bChkEmpty=true );
    void DeleteAttr( _HTMLAttr* pAttr );

    void EndContextAttrs( _HTMLAttrContext *pContext, bool bRemove=false );
    void SaveAttrTab( _HTMLAttrTable& rNewAttrTab );
    void SplitAttrTab( const SwPosition& rNewPos );
    void SplitAttrTab( _HTMLAttrTable& rNewAttrTab, bool bMoveEndBack = true );
    void RestoreAttrTab( _HTMLAttrTable& rNewAttrTab,
                         bool bSetNewStart = false );
    void InsertAttr( const SfxPoolItem& rItem, bool bLikePara = false,
                     bool bInsAtStart=false );
    void InsertAttrs( _HTMLAttrs& rAttrs );

    bool DoPositioning( SfxItemSet &rItemSet,
                        SvxCSS1PropertyInfo &rPropInfo,
                        _HTMLAttrContext *pContext );
    bool CreateContainer( const OUString& rClass, SfxItemSet &rItemSet,
                          SvxCSS1PropertyInfo &rPropInfo,
                          _HTMLAttrContext *pContext );
    bool EndSection( bool bLFStripped=false );

    void InsertAttrs( SfxItemSet &rItemSet, SvxCSS1PropertyInfo &rPropInfo,
                      _HTMLAttrContext *pContext, bool bCharLvl=false );
    void InsertAttr( _HTMLAttr **ppAttr, const SfxPoolItem & rItem,
                     _HTMLAttrContext *pCntxt );
    void SplitPREListingXMP( _HTMLAttrContext *pCntxt );
    void FixHeaderFooterDistance( bool bHeader, const SwPosition *pOldPos );

    void EndContext( _HTMLAttrContext *pContext );
    void ClearContext( _HTMLAttrContext *pContext );

    const SwFormatColl *GetCurrFormatColl() const;

    SwTwips GetCurrentBrowseWidth();

    SwHTMLNumRuleInfo& GetNumInfo() { return *m_pNumRuleInfo; }
    // add parameter <bCountedInList>
    void SetNodeNum( sal_uInt8 nLevel, bool bCountedInList );

    // Verwalten von Absatz-Vorlagen

    // die Vorlagen auf dem Stack bzw. deren Attribute setzen
    void SetTextCollAttrs( _HTMLAttrContext *pContext = 0 );

    void InsertParaAttrs( const SfxItemSet& rItemSet );

    // Verwalten des Attribut-Kontexts

    // aktuellen Kontext merken
    inline void PushContext( _HTMLAttrContext *pCntxt );

    // den obersten/spezifizierten Kontext holen, aber nicht ausserhalb
    // des Kontexts mit Token nLimit suchen. Wenn bRemove gesetzt ist,
    // wird er entfernt
    _HTMLAttrContext *PopContext( sal_uInt16 nToken=0, sal_uInt16 nLimit=0,
                                  bool bRemove=true );

    bool GetMarginsFromContext( sal_uInt16 &nLeft, sal_uInt16 &nRight, short& nIndent,
                                bool bIgnoreCurrent=false ) const;
    bool GetMarginsFromContextWithNumBul( sal_uInt16 &nLeft, sal_uInt16 &nRight,
                                          short& nIndent ) const;
    void GetULSpaceFromContext( sal_uInt16 &rUpper, sal_uInt16 &rLower ) const;

    void MovePageDescAttrs( SwNode *pSrcNd, sal_uLong nDestIdx, bool bFormatBreak );

    // Behandlung von Tags auf Absatz-Ebene

    // <P> und <H1> bis <H6>
    void NewPara();
    void EndPara( bool bReal = false );
    void NewHeading( int nToken );
    void EndHeading();

    // <ADDRESS>, <BLOCKQUOTE> und <PRE>
    void NewTextFormatColl( int nToken, sal_uInt16 nPoolId );
    void EndTextFormatColl( int nToken );

    // <DIV> und <CENTER>
    void NewDivision( int nToken );
    void EndDivision( int nToken );

    // Fly-Frames einfuegen/verlassen
    void InsertFlyFrame( const SfxItemSet& rItemSet, _HTMLAttrContext *pCntxt,
                         const OUString& rId, sal_uInt16 nFlags );

    void SaveDocContext( _HTMLAttrContext *pCntxt, sal_uInt16 nFlags,
                       const SwPosition *pNewPos );
    void RestoreDocContext( _HTMLAttrContext *pCntxt );

    // alle durch <DIV> aufgespannten Bereiche verlassen
    bool EndSections( bool bLFStripped );

    // <MULTICOL>
    void NewMultiCol( sal_uInt16 columnsFromCss=0 );

    // <MARQUEE>
    void NewMarquee( HTMLTable *pCurTable=0 );
    void EndMarquee();
    void InsertMarqueeText();

    // Behandluung von Listen

    // Numerierungs <OL> und Aufzaehlungs-Listen <UL> mit <LI>
    void NewNumBulList( int nToken );
    void EndNumBulList( int nToken=0 );
    void NewNumBulListItem( int nToken );
    void EndNumBulListItem( int nToken=0, bool bSetColl=true,
                            bool bLastPara=false );

    // Definitions-Listen <DL> mit <DD>, <DT>
    void NewDefList();
    void EndDefList();
    void NewDefListItem( int nToken );
    void EndDefListItem( int nToken=0, bool bSetColl=true,
                         bool bLastPara=false );

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
    void NewCharFormat( int nToken );

    // <SDFIELD>
public:
    static sal_uInt16 GetNumType( const OUString& rStr, sal_uInt16 eDfltType );
private:
    void NewField();
    void EndField();
    void InsertFieldText();

    // <SPACER>
    void InsertSpacer();

    // Einfuegen von Grafiken, Plugins und Applets

    // Image-Maps suchen und mit Grafik-Nodes verbinden
    ImageMap *FindImageMap( const OUString& rURL ) const;
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
                                 bool bDontAppend=false );
    void SetAnchorAndAdjustment( const SfxItemSet &rItemSet,
                                 const SvxCSS1PropertyInfo &rPropInfo,
                                 SfxItemSet &rFrmItemSet );

    static void SetFrameFormatAttrs( SfxItemSet &rItemSet, SvxCSS1PropertyInfo &rPropInfo,
                         sal_uInt16 nFlags, SfxItemSet &rFrmItemSet );

    // Frames anlegen und Auto-gebundene Rahmen registrieren
    void RegisterFlyFrm( SwFrameFormat *pFlyFrm );

    // Die Groesse des Fly-Frames an die Vorgaben und Gegebenheiten anpassen
    // (nicht fuer Grafiken, deshalb htmlplug.cxx)
    static void SetFixSize( const Size& rPixSize, const Size& rTwipDfltSize,
                     bool bPrcWidth, bool bPrcHeight,
                     SfxItemSet &rItemSet, SvxCSS1PropertyInfo &rPropInfo,
                     SfxItemSet& rFlyItemSet );
    static void SetVarSize( SfxItemSet &rItemSet, SvxCSS1PropertyInfo &rPropInfo,
                     SfxItemSet& rFlyItemSet, SwTwips nDfltWidth=MINLAY,
                     sal_uInt8 nDltPrcWidth=0 );
    static void SetSpace( const Size& rPixSpace, SfxItemSet &rItemSet,
                   SvxCSS1PropertyInfo &rPropInfo, SfxItemSet& rFlyItemSet );

    sal_uInt16 IncGrfsThatResizeTable();

    void GetDefaultScriptType( ScriptType& rType,
                               OUString& rTypeStr ) const;

    // die eigentlichen Einfuege-Methoden fuer <IMG>, <EMBED> und <APPLET>
    // und <PARAM>
    void InsertImage();     // htmlgrin.cxx
    void InsertEmbed();     // htmlplug.cxx

#if HAVE_FEATURE_JAVA
    void NewObject();   // htmlplug.cxx
#endif
    void EndObject();       // CommandLine mit Applet verkn. (htmlplug.cxx)
#if HAVE_FEATURE_JAVA
    void InsertApplet();    // htmlplug.cxx
#endif
    void EndApplet();       // CommandLine mit Applet verkn. (htmlplug.cxx)
    void InsertParam();     // htmlplug.cxx

    void InsertFloatingFrame();
    void EndFloatingFrame() { m_bInFloatingFrame = false; }

    // <BODY>-Tag auswerten: Hintergrund-Grafiken und -Farben setzen (htmlgrin.cxx)
    void InsertBodyOptions();

    // Einfuegen von Links und Bookmarks (htmlgrin.cxx)

    // <A>-Tag auswerten: einen Link bzw. eine Bookmark einfuegen
    void NewAnchor();
    void EndAnchor();

    // eine Bookmark einfuegen
    void InsertBookmark( const OUString& rName );

    void InsertCommentText( const sal_Char *pTag = 0 );
    void InsertComment( const OUString& rName, const sal_Char *pTag = 0 );

    // sind im aktuellen Absatz Bookmarks vorhanden?
    bool HasCurrentParaBookmarks( bool bIgnoreStack=false ) const;

    // Einfuegen von Script/Basic-Elementen

    // das zueletzt gelsene Basic-Modul parsen (htmlbas.cxx)
    void NewScript();
    void EndScript();

    void AddScriptSource();

    // ein Event in die SFX-Konfiguation eintragen (htmlbas.cxx)
    void InsertBasicDocEvent( const OUString& aEventName, const OUString& rName,
                              ScriptType eScrType, const OUString& rScrType );

    // Inserting styles

    // <STYLE>
    void NewStyle();
    void EndStyle();

    static inline bool HasStyleOptions( const OUString &rStyle, const OUString &rId,
                                 const OUString &rClass, const OUString *pLang=0,
                                 const OUString *pDir=0 );
    bool ParseStyleOptions( const OUString &rStyle, const OUString &rId,
                            const OUString &rClass, SfxItemSet &rItemSet,
                            SvxCSS1PropertyInfo &rPropInfo,
                            const OUString *pLang=0, const OUString *pDir=0 );

    // Inserting Controls and Forms (htmlform.cxx)

    // Insert draw object into document
    void InsertDrawObject( SdrObject* pNewDrawObj, const Size& rSpace,
                           sal_Int16 eVertOri,
                           sal_Int16 eHoriOri,
                           SfxItemSet& rCSS1ItemSet,
                           SvxCSS1PropertyInfo& rCSS1PropInfo,
                           bool bHidden=false );
     css::uno::Reference< css::drawing::XShape >  InsertControl(
                        const css::uno::Reference< css::form::XFormComponent > & rFormComp,
                        const css::uno::Reference< css::beans::XPropertySet > & rFCompPropSet,
                        const Size& rSize,
                        sal_Int16 eVertOri,
                        sal_Int16 eHoriOri,
                        SfxItemSet& rCSS1ItemSet,
                        SvxCSS1PropertyInfo& rCSS1PropInfo,
                        const SvxMacroTableDtor& rMacroTable,
                        const std::vector<OUString>& rUnoMacroTable,
                        const std::vector<OUString>& rUnoMacroParamTable,
                        bool bSetPropSet = true,
                        bool bHidden = false );
    void SetControlSize( const css::uno::Reference< css::drawing::XShape > & rShape, const Size& rTextSz,
                         bool bMinWidth, bool bMinHeight );

public:
    static void ResizeDrawObject( SdrObject* pObj, SwTwips nWidth );
private:
    static void RegisterDrawObjectToTable( HTMLTable *pCurTable, SdrObject* pObj,
                                    sal_uInt8 nWidth );

    void NewForm( bool bAppend=true );
    void EndForm( bool bAppend=true );

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
    HTMLTableCnts *InsertTableContents( bool bHead );

private:
    // Create a section for the temporary storage of the table caption
    SwStartNode *InsertTempTableCaptionSection();

    void BuildTableCell( HTMLTable *pTable, bool bReadOptions, bool bHead );
    void BuildTableRow( HTMLTable *pTable, bool bReadOptions,
                        SvxAdjust eGrpAdjust, sal_Int16 eVertOri );
    void BuildTableSection( HTMLTable *pTable, bool bReadOptions, bool bHead );
    void BuildTableColGroup( HTMLTable *pTable, bool bReadOptions );
    void BuildTableCaption( HTMLTable *pTable );
    HTMLTable *BuildTable( SvxAdjust eCellAdjust,
                           bool bIsParentHead = false,
                           bool bHasParentSection=true,
                           bool bHasToFlow = false );

    // misc ...

    void ParseMoreMetaOptions();

    bool FileDownload( const OUString& rURL, OUString& rStr );
    void InsertLink();

    void InsertIDOption();
    void InsertLineBreak();
    void InsertHorzRule();

    void FillEndNoteInfo( const OUString& rContent );
    void FillFootNoteInfo( const OUString& rContent );
    void InsertFootEndNote( const OUString& rName, bool bEndNote, bool bFixed );
    void FinishFootEndNote();
    void InsertFootEndNoteText();
    SwNodeIndex *GetFootEndNoteSection( const OUString& rName );
    void DeleteFootEndNoteImpl();

    sal_Int32 StripTrailingLF();

    // Remove empty paragraph at the PaM position
    void StripTrailingPara();

    // Are there fly frames in the current paragraph?
    bool HasCurrentParaFlys( bool bNoSurroundOnly = false,
                             bool bSurroundOnly = false ) const;

public:         // used in tables

    // Create brush item (with new) or 0
    SvxBrushItem* CreateBrushItem( const Color *pColor,
                                   const OUString &rImageURL,
                                   const OUString &rStyle,
                                   const OUString &rId,
                                   const OUString &rClass );

protected:
    // Executed for each token recognized by CallParser
    virtual void NextToken( int nToken ) override;
    virtual ~SwHTMLParser();

    // If the document is removed, remove the parser as well
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew ) override;

    virtual void AddMetaUserDefined( OUString const & i_rMetaName ) override;

public:

    SwHTMLParser( SwDoc* pD, SwPaM & rCrsr, SvStream& rIn,
                    const OUString& rFileName,
                    const OUString& rBaseURL,
                    bool bReadNewDoc = true,
                    SfxMedium* pMed = 0, bool bReadUTF8 = false,
                    bool bIgnoreHTMLComments = false );

    virtual SvParserState CallParser() override;

    static sal_uInt16 ToTwips( sal_uInt16 nPixel );

    // for reading asynchronously from SvStream
    virtual void Continue( int nToken ) override;

    virtual bool ParseMetaOptions( const css::uno::Reference<css::document::XDocumentProperties>&,
            SvKeyValueIterator* ) override;
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
    nSttContent = rPos.nContent.GetIndex();
    nEndPara = nSttPara;
    nEndContent = nSttContent;
}

inline void _HTMLAttrContext::SetMargins( sal_uInt16 nLeft, sal_uInt16 nRight,
                                          short nIndent )
{
    nLeftMargin = nLeft;
    nRightMargin = nRight;
    nFirstLineIndent = nIndent;
    bLRSpaceChanged = true;
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
    bULSpaceChanged = true;
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

inline bool SwHTMLParser::HasStyleOptions( const OUString &rStyle,
                                            const OUString &rId,
                                            const OUString &rClass,
                                            const OUString *pLang,
                                               const OUString *pDir )
{
    return !rStyle.isEmpty() || !rId.isEmpty() || !rClass.isEmpty() ||
           (pLang && !pLang->isEmpty()) || (pDir && !pDir->isEmpty());
}

inline void SwHTMLParser::PushContext( _HTMLAttrContext *pCntxt )
{
    m_aContexts.push_back( pCntxt );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
