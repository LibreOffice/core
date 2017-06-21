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
#include <svtools/htmltokn.h>
#include <editeng/svxenum.hxx>
#include <rtl/ref.hxx>
#include <fmtornt.hxx>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/form/XFormComponent.hpp>

#include "calbck.hxx"

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

// some things you often need
extern HTMLOptionEnum<SvxAdjust> aHTMLPAlignTable[];
extern HTMLOptionEnum<sal_Int16> aHTMLImgHAlignTable[];
extern HTMLOptionEnum<sal_Int16> aHTMLImgVAlignTable[];

// attribute stack:

class HTMLAttr;
typedef std::deque<HTMLAttr *> HTMLAttrs;

class HTMLAttr
{
    friend class SwHTMLParser;
    friend class CellSaveStruct;

    SwNodeIndex nSttPara, nEndPara;
    sal_Int32 nSttContent, nEndContent;
    bool bInsAtStart : 1;
    bool bLikePara : 1; // set attribute above the whole paragraph
    bool bValid : 1;    // is the attribute valid?

    std::unique_ptr<SfxPoolItem> pItem;
    HTMLAttr *pNext;   // still to close attributes with different values
    HTMLAttr *pPrev;   // already closed but not set attributes
    HTMLAttr **ppHead; // list head

    HTMLAttr( const SwPosition& rPos, const SfxPoolItem& rItem,
               HTMLAttr **pHd=nullptr );

    HTMLAttr( const HTMLAttr &rAttr, const SwNodeIndex &rEndPara,
               sal_Int32 nEndCnt, HTMLAttr **pHd );

public:

    ~HTMLAttr();

    HTMLAttr *Clone( const SwNodeIndex& rEndPara, sal_Int32 nEndCnt ) const;
    void Reset( const SwNodeIndex& rSttPara, sal_Int32 nSttCnt,
                HTMLAttr **pHd );
    inline void SetStart( const SwPosition& rPos );

    sal_uInt32 GetSttParaIdx() const { return nSttPara.GetIndex(); }
    sal_uInt32 GetEndParaIdx() const { return nEndPara.GetIndex(); }

    const SwNodeIndex& GetSttPara() const { return nSttPara; }
    const SwNodeIndex& GetEndPara() const { return nEndPara; }

    sal_Int32 GetSttCnt() const { return nSttContent; }
    sal_Int32 GetEndCnt() const { return nEndContent; }

    bool IsLikePara() const { return bLikePara; }
    void SetLikePara() { bLikePara = true; }

          SfxPoolItem& GetItem()        { return *pItem; }
    const SfxPoolItem& GetItem() const  { return *pItem; }

    HTMLAttr *GetNext() const { return pNext; }
    void InsertNext( HTMLAttr *pNxt ) { pNext = pNxt; }

    HTMLAttr *GetPrev() const { return pPrev; }
    void InsertPrev( HTMLAttr *pPrv );
    void ClearPrev() { pPrev = nullptr; }

    void SetHead( HTMLAttr **ppHd ) { ppHead = ppHd; }

    // During setting attributes from styles it can happen that these
    // shouldn't be set anymore. To delete them would be very expensive, because
    // you don't know all the places where they are linked in. Therefore they're
    // made invalid and deleted at the next call of SetAttr_().
    void Invalidate() { bValid = false; }
};

// Table of attributes: The order here is important: The attributes in the
// beginning of the table will set first in EndAllAttrs.
struct HTMLAttrTable
{
    HTMLAttr
                *pKeep,         // frame attributes
                *pBox,
                *pBrush,
                *pBreak,
                *pPageDesc,

                *pLRSpace,      // paragraph attributes
                *pULSpace,
                *pLineSpacing,
                *pAdjust,
                *pDropCap,
                *pSplit,
                *pWidows,
                *pOrphans,
                *pDirection,

                *pCharFormats,     // text attributes
                *pINetFormat,

                *pBold,         // character attributes
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
                *pKerning,      // (only for SPACER)
                *pCharBrush,    // character background
                *pLanguage,
                *pLanguageCJK,
                *pLanguageCTL,
                *pCharBox
                ;
};

class HTMLAttrContext_SaveDoc;

enum SwHTMLAppendMode {
     AM_NORMAL,         // no paragraph spacing handling
     AM_NOSPACE,        // set spacing hard to 0cm
     AM_SPACE,          // set spacing hard to 0.5cm
     AM_SOFTNOSPACE,    // don't set spacing, but save 0cm
     AM_NONE            // no append
};

class HTMLAttrContext
{
    HTMLAttrs aAttrs;      // the attributes created in the context

    OUString    aClass;          // context class

    HTMLAttrContext_SaveDoc *pSaveDocContext;
    std::unique_ptr<SfxItemSet> pFrameItemSet;

    HtmlTokenId nToken;         // the token of the context

    sal_uInt16  nTextFormatColl;    // a style created in the context or zero

    sal_uInt16  nLeftMargin;        // a changed left border
    sal_uInt16  nRightMargin;       // a changed right border
    sal_uInt16  nFirstLineIndent;   // a changed first line indent

    sal_uInt16  nUpperSpace;
    sal_uInt16  nLowerSpace;

    SwHTMLAppendMode eAppend;

    bool    bLRSpaceChanged : 1;    // left/right border, changed indent?
    bool    bULSpaceChanged : 1;    // top/bottom border changed?
    bool    bDfltTextFormatColl : 1;// nTextFormatColl is only default
    bool    bSpansSection : 1;      // the context opens a SwSection
    bool    bPopStack : 1;          // delete above stack elements
    bool    bFinishPREListingXMP : 1;
    bool    bRestartPRE : 1;
    bool    bRestartXMP : 1;
    bool    bRestartListing : 1;

public:
    void ClearSaveDocContext();

    HTMLAttrContext( HtmlTokenId nTokn, sal_uInt16 nPoolId, const OUString& rClass,
                      bool bDfltColl=false ) :
        aClass( rClass ),
        pSaveDocContext( nullptr ),
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

    explicit HTMLAttrContext( HtmlTokenId nTokn ) :
        pSaveDocContext( nullptr ),
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

    ~HTMLAttrContext() { ClearSaveDocContext(); }

    HtmlTokenId GetToken() const { return nToken; }

    sal_uInt16 GetTextFormatColl() const { return bDfltTextFormatColl ? 0 : nTextFormatColl; }
    sal_uInt16 GetDfltTextFormatColl() const { return bDfltTextFormatColl ? nTextFormatColl : 0; }

    const OUString& GetClass() const { return aClass; }

    inline void SetMargins( sal_uInt16 nLeft, sal_uInt16 nRight, short nIndent );

    bool IsLRSpaceChanged() const { return bLRSpaceChanged; }
    inline void GetMargins( sal_uInt16& nLeft, sal_uInt16& nRight,
                            short &nIndent ) const;

    inline void SetULSpace( sal_uInt16 nUpper, sal_uInt16 nLower );
    bool IsULSpaceChanged() const { return bULSpaceChanged; }
    inline void GetULSpace( sal_uInt16& rUpper, sal_uInt16& rLower ) const;

    bool HasAttrs() const { return !aAttrs.empty(); }
    const HTMLAttrs& GetAttrs() const { return aAttrs; }
    HTMLAttrs& GetAttrs() { return aAttrs; }

    void SetSpansSection( bool bSet ) { bSpansSection = bSet; }
    bool GetSpansSection() const { return bSpansSection; }

    void SetPopStack( bool bSet ) { bPopStack = bSet; }
    bool GetPopStack() const { return bPopStack; }

    bool HasSaveDocContext() const { return pSaveDocContext!=nullptr; }
    HTMLAttrContext_SaveDoc *GetSaveDocContext( bool bCreate=false );

    const SfxItemSet *GetFrameItemSet() const { return pFrameItemSet.get(); }
    SfxItemSet *GetFrameItemSet( SwDoc *pCreateDoc );

    void SetFinishPREListingXMP( bool bSet ) { bFinishPREListingXMP = bSet; }
    bool IsFinishPREListingXMP() const { return bFinishPREListingXMP; }

    void SetRestartPRE( bool bSet ) { bRestartPRE = bSet; }
    bool IsRestartPRE() const { return bRestartPRE; }

    void SetRestartXMP( bool bSet ) { bRestartXMP = bSet; }
    bool IsRestartXMP() const { return bRestartXMP; }

    void SetRestartListing( bool bSet ) { bRestartListing = bSet; }
    bool IsRestartListing() const { return bRestartListing; }

    void SetAppendMode( SwHTMLAppendMode eMode ) { eAppend = eMode; }
    SwHTMLAppendMode GetAppendMode() const { return eAppend; }
};

typedef std::vector<HTMLAttrContext *> HTMLAttrContexts;

class HTMLTable;
class SwCSS1Parser;
class SwHTMLNumRuleInfo;

typedef std::vector<std::unique_ptr<ImageMap>> ImageMaps;

enum class HtmlContextFlags {
    ProtectStack    = 0x0001,
    StripPara       = 0x0002,
    KeepNumrule     = 0x0004,
    HeaderDist      = 0x0008,
    FooterDist      = 0x0010,
    KeepAttrs       = 0x0020,
    MultiColMask    = StripPara | KeepNumrule | KeepAttrs // for headers, footers or footnotes
};
namespace o3tl
{
    template<> struct typed_flags<HtmlContextFlags> : is_typed_flags<HtmlContextFlags, 0x03f> {};
}

enum class HtmlFrameFormatFlags {
    Box                 = 0x0001,
    Background          = 0x0002,
    Padding             = 0x0004,
    Direction           = 0x0008,
};
namespace o3tl
{
    template<> struct typed_flags<HtmlFrameFormatFlags> : is_typed_flags<HtmlFrameFormatFlags, 0x0f> {};
}

class SwHTMLParser : public SfxHTMLParser, public SwClient
{
    friend class SectionSaveStruct;
    friend class CellSaveStruct;
    friend class CaptionSaveStruct;

    OUString      m_aPathToFile;
    OUString      m_sBaseURL;
    OUString      m_aBasicLib;
    OUString      m_aBasicModule;
    OUString      m_aScriptSource;  // content of the current script block
    OUString      m_aScriptType;    // type of read script (StarBasic/VB/JAVA)
    OUString      m_aScriptURL;     // script URL
    OUString      m_aStyleSource;   // content of current style sheet
    OUString      m_aContents;      // text of current marquee, field and so
    OUString      m_sTitle;
    OUString      m_aUnknownToken;  // a started unknown token
    OUString      m_aBulletGrfs[MAXLEVEL];
    OUString      m_sJmpMark;

    std::vector<sal_uInt16>   m_aBaseFontStack; // stack for <BASEFONT>
                                // Bit 0-2: font size (1-7)
    std::vector<sal_uInt16>   m_aFontStack;     // stack for <FONT>, <BIG>, <SMALL>
                                // Bit 0-2: font size (1-7)
                                // Bit 15:  font colour was set

    HTMLAttrs      m_aSetAttrTab;// "closed", not set attributes
    HTMLAttrs      m_aParaAttrs; // temporary paragraph attributes
    HTMLAttrTable  m_aAttrTab;   // "open" attributes
    HTMLAttrContexts m_aContexts;// the current context of attribute/token
    std::vector<SwFrameFormat *> m_aMoveFlyFrames;// Fly-Frames, the anchor is moved
    std::deque<sal_Int32> m_aMoveFlyCnts;// and the Content-Positions

    SwApplet_Impl *m_pAppletImpl; // current applet

    SwCSS1Parser    *m_pCSS1Parser;   // Style-Sheet-Parser
    SwHTMLNumRuleInfo *m_pNumRuleInfo;
    SwPendingStack  *m_pPendStack;

    rtl::Reference<SwDoc> m_xDoc;
    SwPaM           *m_pPam;      // SwPosition should be enough, or ??
    SwViewShell       *m_pActionViewShell;  // SwViewShell, where StartAction was called
    SwNodeIndex     *m_pSttNdIdx;

    HTMLTable       *m_pTable;      // current "outermost" table
    SwHTMLForm_Impl *m_pFormImpl;   // current form
    SdrObject       *m_pMarquee;    // current marquee
    SwField         *m_pField;      // current field
    ImageMap        *m_pImageMap;   // current image map
    ImageMaps       *m_pImageMaps;  ///< all Image-Maps that have been read
    SwHTMLFootEndNote_Impl *m_pFootEndNoteImpl;

    Size    m_aHTMLPageSize;      // page size of HTML template

    sal_uInt32  m_aFontHeights[7];    // font heights 1-7
    ImplSVEvent * m_nEventId;

    sal_uInt16  m_nBaseFontStMin;
    sal_uInt16  m_nFontStMin;
    sal_uInt16  m_nDefListDeep;
    sal_uInt16  m_nFontStHeadStart;   // elements in font stack at <Hn>
    sal_uInt16  m_nSBModuleCnt;       // counter for basic modules
    sal_uInt16  m_nMissingImgMaps;    // How many image maps are still missing?
    size_t m_nParaCnt;
    size_t m_nContextStMin;           // lower limit of PopContext
    size_t m_nContextStAttrMin;       // lower limit of attributes
    sal_uInt16  m_nSelectEntryCnt;    // Number of entries in the actual listbox
    HtmlTokenId m_nOpenParaToken;     // opened paragraph element

    enum JumpToMarks { JUMPTO_NONE, JUMPTO_MARK, JUMPTO_TABLE, JUMPTO_FRAME,
                        JUMPTO_REGION, JUMPTO_GRAPHIC } m_eJumpTo;

#ifdef DBG_UTIL
    sal_uInt16  m_nContinue;        // depth of Continue calls
#endif

    SvxAdjust   m_eParaAdjust;    // adjustment of current paragraph
    HTMLScriptLanguage m_eScriptLang; // current script language

    bool m_bOldIsHTMLMode : 1;    // Was it a HTML document?

    bool m_bDocInitalized : 1;    // document resp. shell was initialize
                                  // flag to prevent double init via recursion
    bool m_bViewCreated : 1;      // the view was already created (asynchronous)
    bool m_bSetModEnabled : 1;

    bool m_bInFloatingFrame : 1;  // We are in a floating frame
    bool m_bInField : 1;
    bool m_bKeepUnknown : 1;      // handle unknown/not supported tokens
    // 8
    bool m_bCallNextToken : 1;    // In tables: call NextToken in any case
    bool m_bIgnoreRawData : 1;    // ignore content of script/style
    bool m_bLBEntrySelected : 1;  // Is the current option selected?
    bool m_bTAIgnoreNewPara : 1;  // ignore next LF in text area?
    bool m_bFixMarqueeWidth : 1;  // Change size of marquee?

    bool m_bUpperSpace : 1;       // top paragraph spacing is needed
    bool m_bNoParSpace : 1;
    // 16

    bool m_bInNoEmbed : 1;        // we are in a NOEMBED area

    bool m_bInTitle : 1;          // we are in title

    bool m_bChkJumpMark : 1;      // maybe jump to predetermined mark
    bool m_bUpdateDocStat : 1;
    bool m_bFixSelectWidth : 1;   // Set new width of select?
    bool m_bFixSelectHeight : 1;  // Set new height of select?
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
    SwViewShell *CallStartAction( SwViewShell *pVSh = nullptr, bool bChkPtr = true );
    SwViewShell *CallEndAction( bool bChkAction = false, bool bChkPtr = true );
    SwViewShell *CheckActionViewShell();

    DECL_LINK( AsyncCallback, void*, void );

    // set attribute on document
    void SetAttr_( bool bChkEnd, bool bBeforeTable, HTMLAttrs *pPostIts );
    void SetAttr( bool bChkEnd = true, bool bBeforeTable = false,
                         HTMLAttrs *pPostIts = nullptr )
    {
        if( !m_aSetAttrTab.empty() || !m_aMoveFlyFrames.empty() )
            SetAttr_( bChkEnd, bBeforeTable, pPostIts );
    }

    HTMLAttr **GetAttrTabEntry( sal_uInt16 nWhich );

    // create a new text node on PaM position
    bool AppendTextNode( SwHTMLAppendMode eMode=AM_NORMAL, bool bUpdateNum=true );
    void AddParSpace();

    // start/end an attribute
    // ppDepAttr indicated an attribute table entry, which attribute has to be
    // set, before the attribute is closed
    void NewAttr( HTMLAttr **ppAttr, const SfxPoolItem& rItem );
    bool EndAttr( HTMLAttr *pAttr, bool bChkEmpty=true );
    void DeleteAttr( HTMLAttr* pAttr );

    void EndContextAttrs( HTMLAttrContext *pContext );
    void SaveAttrTab( HTMLAttrTable& rNewAttrTab );
    void SplitAttrTab( const SwPosition& rNewPos );
    void SplitAttrTab( HTMLAttrTable& rNewAttrTab, bool bMoveEndBack );
    void RestoreAttrTab( HTMLAttrTable& rNewAttrTab );
    void InsertAttr( const SfxPoolItem& rItem, bool bInsAtStart );
    void InsertAttrs( HTMLAttrs& rAttrs );

    bool DoPositioning( SfxItemSet &rItemSet,
                        SvxCSS1PropertyInfo &rPropInfo,
                        HTMLAttrContext *pContext );
    bool CreateContainer( const OUString& rClass, SfxItemSet &rItemSet,
                          SvxCSS1PropertyInfo &rPropInfo,
                          HTMLAttrContext *pContext );
    bool EndSection( bool bLFStripped=false );

    void InsertAttrs( SfxItemSet &rItemSet, SvxCSS1PropertyInfo &rPropInfo,
                      HTMLAttrContext *pContext, bool bCharLvl=false );
    void InsertAttr( HTMLAttr **ppAttr, const SfxPoolItem & rItem,
                     HTMLAttrContext *pCntxt );
    void SplitPREListingXMP( HTMLAttrContext *pCntxt );
    void FixHeaderFooterDistance( bool bHeader, const SwPosition *pOldPos );

    void EndContext( HTMLAttrContext *pContext );
    void ClearContext( HTMLAttrContext *pContext );

    const SwFormatColl *GetCurrFormatColl() const;

    SwTwips GetCurrentBrowseWidth();

    SwHTMLNumRuleInfo& GetNumInfo() { return *m_pNumRuleInfo; }
    // add parameter <bCountedInList>
    void SetNodeNum( sal_uInt8 nLevel );

    // Manage paragraph styles

    // set the style resp. its attributes on the stack
    void SetTextCollAttrs( HTMLAttrContext *pContext = nullptr );

    void InsertParaAttrs( const SfxItemSet& rItemSet );

    // Manage attribute context

    // save current context
    inline void PushContext( HTMLAttrContext *pCntxt );

    // Fetch top/specified context but not outside the context with token
    // nLimit. If bRemove set then remove it.
    HTMLAttrContext *PopContext( HtmlTokenId nToken = HtmlTokenId::NONE );

    bool GetMarginsFromContext( sal_uInt16 &nLeft, sal_uInt16 &nRight, short& nIndent,
                                bool bIgnoreCurrent=false ) const;
    void GetMarginsFromContextWithNumBul( sal_uInt16 &nLeft, sal_uInt16 &nRight,
                                          short& nIndent ) const;
    void GetULSpaceFromContext( sal_uInt16 &rUpper, sal_uInt16 &rLower ) const;

    void MovePageDescAttrs( SwNode *pSrcNd, sal_uLong nDestIdx, bool bFormatBreak );

    // Handling of tags at paragraph level

    // <P> and <H1> to <H6>
    void NewPara();
    void EndPara( bool bReal = false );
    void NewHeading( HtmlTokenId nToken );
    void EndHeading();

    // <ADDRESS>, <BLOCKQUOTE> and <PRE>
    void NewTextFormatColl( HtmlTokenId nToken, sal_uInt16 nPoolId );
    void EndTextFormatColl( HtmlTokenId nToken );

    // <DIV> and <CENTER>
    void NewDivision( HtmlTokenId nToken );
    void EndDivision();

    // insert/close Fly-Frames
    void InsertFlyFrame( const SfxItemSet& rItemSet, HTMLAttrContext *pCntxt,
                         const OUString& rId );

    void SaveDocContext( HTMLAttrContext *pCntxt, HtmlContextFlags nFlags,
                       const SwPosition *pNewPos );
    void RestoreDocContext( HTMLAttrContext *pCntxt );

    // end all opened <DIV> areas
    bool EndSections( bool bLFStripped );

    // <MULTICOL>
    void NewMultiCol( sal_uInt16 columnsFromCss=0 );

    // <MARQUEE>
    void NewMarquee( HTMLTable *pCurTable=nullptr );
    void EndMarquee();
    void InsertMarqueeText();

    // Handling of lists

    // order list <OL> and unordered list <UL> with <LI>
    void NewNumBulList( HtmlTokenId nToken );
    void EndNumBulList( HtmlTokenId nToken = HtmlTokenId::NONE );
    void NewNumBulListItem( HtmlTokenId nToken );
    void EndNumBulListItem( HtmlTokenId nToken, bool bSetColl);

    // definitions lists <DL> with <DD>, <DT>
    void NewDefList();
    void EndDefList();
    void NewDefListItem( HtmlTokenId nToken );
    void EndDefListItem( HtmlTokenId nToken = HtmlTokenId::NONE );

    // Handling of tags on character level

    // handle tags like <B>, <I> and so, which enable/disable a certain
    // attribute or like <SPAN> get attributes from styles
    void NewStdAttr( HtmlTokenId nToken );
    void NewStdAttr( HtmlTokenId nToken,
                     HTMLAttr **ppAttr, const SfxPoolItem & rItem,
                     HTMLAttr **ppAttr2=nullptr, const SfxPoolItem *pItem2=nullptr,
                     HTMLAttr **ppAttr3=nullptr, const SfxPoolItem *pItem3=nullptr );
    void EndTag( HtmlTokenId nToken );

    // handle font attributes
    void NewBasefontAttr();             // for <BASEFONT>
    void EndBasefontAttr();
    void NewFontAttr( HtmlTokenId nToken ); // for <FONT>, <BIG> and <SMALL>
    void EndFontAttr( HtmlTokenId nToken );

    // tags realized via character styles
    void NewCharFormat( HtmlTokenId nToken );

    // <SDFIELD>
public:
    static SvxNumType GetNumType( const OUString& rStr, SvxNumType eDfltType );
private:
    void NewField();
    void EndField();
    void InsertFieldText();

    // <SPACER>
    void InsertSpacer();

    // Inserting graphics, plug-ins and applets

    // search image maps and link with graphic nodes
    ImageMap *FindImageMap( const OUString& rURL ) const;
    void ConnectImageMaps();

    // find anchor of Fly-Frames and set corresponding attributes
    // in Attrset (htmlgrin.cxx)
    void SetAnchorAndAdjustment( sal_Int16 eVertOri,
                                 sal_Int16 eHoriOri,
                                 const SvxCSS1PropertyInfo &rPropInfo,
                                 SfxItemSet& rFrameSet );
    void SetAnchorAndAdjustment( sal_Int16 eVertOri,
                                 sal_Int16 eHoriOri,
                                 SfxItemSet& rFrameSet,
                                 bool bDontAppend=false );
    void SetAnchorAndAdjustment( const SvxCSS1PropertyInfo &rPropInfo,
                                 SfxItemSet &rFrameItemSet );

    static void SetFrameFormatAttrs( SfxItemSet &rItemSet,
                         HtmlFrameFormatFlags nFlags, SfxItemSet &rFrameItemSet );

    // create frames and register auto bound frames
    void RegisterFlyFrame( SwFrameFormat *pFlyFrame );

    // Adjust the size of the Fly-Frames to requirements and conditions
    // (not for graphics, therefore htmlplug.cxx)
    static void SetFixSize( const Size& rPixSize, const Size& rTwipDfltSize,
                     bool bPrcWidth, bool bPrcHeight,
                     SfxItemSet &rItemSet, SvxCSS1PropertyInfo &rPropInfo,
                     SfxItemSet& rFlyItemSet );
    static void SetVarSize( SvxCSS1PropertyInfo &rPropInfo,
                     SfxItemSet& rFlyItemSet, SwTwips nDfltWidth=MINLAY,
                     sal_uInt8 nDltPrcWidth=0 );
    static void SetSpace( const Size& rPixSpace, SfxItemSet &rItemSet,
                   SvxCSS1PropertyInfo &rPropInfo, SfxItemSet& rFlyItemSet );

    sal_uInt16 IncGrfsThatResizeTable();

    void GetDefaultScriptType( ScriptType& rType,
                               OUString& rTypeStr ) const;

    // the actual insert methods for <IMG>, <EMBED>, <APPLET> and <PARAM>
    void InsertImage();     // htmlgrin.cxx
    void InsertEmbed();     // htmlplug.cxx

#if HAVE_FEATURE_JAVA
    void NewObject();   // htmlplug.cxx
#endif
    void EndObject();       // link CommandLine with applet (htmlplug.cxx)
#if HAVE_FEATURE_JAVA
    void InsertApplet();    // htmlplug.cxx
#endif
    void EndApplet();       // link CommandLine with applet (htmlplug.cxx)
    void InsertParam();     // htmlplug.cxx

    void InsertFloatingFrame();

    // parse <BODY>-tag: set background graphic and background colour (htmlgrin.cxx)
    void InsertBodyOptions();

    // Inserting links and bookmarks (htmlgrin.cxx)

    // parse <A>-tag: insert a link resp. bookmark
    void NewAnchor();
    void EndAnchor();

    // insert bookmark
    void InsertBookmark( const OUString& rName );

    void InsertCommentText( const sal_Char *pTag );
    void InsertComment( const OUString& rName, const sal_Char *pTag = nullptr );

    // Has the current paragraph bookmarks?
    bool HasCurrentParaBookmarks( bool bIgnoreStack=false ) const;

    // Inserting script/basic elements

    // parse the last read basic module (htmlbas.cxx)
    void NewScript();
    void EndScript();

    void AddScriptSource();

    // insert event in SFX configuration (htmlbas.cxx)
    void InsertBasicDocEvent( const OUString& aEventName, const OUString& rName,
                              ScriptType eScrType, const OUString& rScrType );

    // Inserting styles

    // <STYLE>
    void NewStyle();
    void EndStyle();

    static inline bool HasStyleOptions( const OUString &rStyle, const OUString &rId,
                                 const OUString &rClass, const OUString *pLang=nullptr,
                                 const OUString *pDir=nullptr );
    bool ParseStyleOptions( const OUString &rStyle, const OUString &rId,
                            const OUString &rClass, SfxItemSet &rItemSet,
                            SvxCSS1PropertyInfo &rPropInfo,
                            const OUString *pLang=nullptr, const OUString *pDir=nullptr );

    // Inserting Controls and Forms (htmlform.cxx)

    // Insert draw object into document
    void InsertDrawObject( SdrObject* pNewDrawObj, const Size& rSpace,
                           sal_Int16 eVertOri,
                           sal_Int16 eHoriOri,
                           SfxItemSet& rCSS1ItemSet,
                           SvxCSS1PropertyInfo& rCSS1PropInfo );
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

    // Insert methods for <INPUT>, <TEXTAREA> and <SELECT>
    void InsertInput();

    void NewTextArea();
    void InsertTextAreaText( HtmlTokenId nToken );
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
    virtual void NextToken( HtmlTokenId nToken ) override;
    virtual ~SwHTMLParser() override;

    // If the document is removed, remove the parser as well
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew ) override;

    virtual void AddMetaUserDefined( OUString const & i_rMetaName ) override;

public:

    SwHTMLParser( SwDoc* pD, SwPaM & rCursor, SvStream& rIn,
                    const OUString& rFileName,
                    const OUString& rBaseURL,
                    bool bReadNewDoc,
                    SfxMedium* pMed = nullptr, bool bReadUTF8 = false,
                    bool bIgnoreHTMLComments = false );

    virtual SvParserState CallParser() override;

    static sal_uInt16 ToTwips( sal_uInt16 nPixel );

    // for reading asynchronously from SvStream
    virtual void Continue( HtmlTokenId nToken ) override;

    virtual bool ParseMetaOptions( const css::uno::Reference<css::document::XDocumentProperties>&,
            SvKeyValueIterator* ) override;
};

struct SwPendingStackData
{
    virtual ~SwPendingStackData() {}
};

struct SwPendingStack
{
    HtmlTokenId nToken;
    SwPendingStackData* pData;
    SwPendingStack* pNext;

    SwPendingStack( HtmlTokenId nTkn, SwPendingStack* pNxt )
        : nToken( nTkn ), pData( nullptr ), pNext( pNxt )
        {}
};

inline void HTMLAttr::SetStart( const SwPosition& rPos )
{
    nSttPara = rPos.nNode;
    nSttContent = rPos.nContent.GetIndex();
    nEndPara = nSttPara;
    nEndContent = nSttContent;
}

inline void HTMLAttrContext::SetMargins( sal_uInt16 nLeft, sal_uInt16 nRight,
                                          short nIndent )
{
    nLeftMargin = nLeft;
    nRightMargin = nRight;
    nFirstLineIndent = nIndent;
    bLRSpaceChanged = true;
}

inline void HTMLAttrContext::GetMargins( sal_uInt16& nLeft,
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

inline void HTMLAttrContext::SetULSpace( sal_uInt16 nUpper, sal_uInt16 nLower )
{
    nUpperSpace = nUpper;
    nLowerSpace = nLower;
    bULSpaceChanged = true;
}

inline void HTMLAttrContext::GetULSpace( sal_uInt16& rUpper,
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

inline void SwHTMLParser::PushContext( HTMLAttrContext *pCntxt )
{
    m_aContexts.push_back( pCntxt );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
