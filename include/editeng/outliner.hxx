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
#ifndef INCLUDED_EDITENG_OUTLINER_HXX
#define INCLUDED_EDITENG_OUTLINER_HXX

#include <editeng/editdata.hxx>
#include <editeng/editstat.hxx>
#include <editeng/numitem.hxx>
#include <i18nlangtag/lang.h>
#include <rtl/ustring.hxx>
#include <svl/SfxBroadcaster.hxx>
#include <svl/languageoptions.hxx>
#include <tools/gen.hxx>
#include <tools/color.hxx>
#include <tools/contnr.hxx>
#include <vcl/graphic.hxx>
#include <vcl/outdev.hxx>
#include <tools/link.hxx>
#include <rsc/rscsfx.hxx>
#include <editeng/editengdllapi.h>
#include <com/sun/star/lang/Locale.hpp>

#include <svtools/grfmgr.hxx>

#include <LibreOfficeKit/LibreOfficeKitTypes.h>
#include <vector>

class OutlinerEditEng;
class Outliner;
class EditView;
class EditUndo;
class ParagraphList;
class OutlinerParaObject;
class SvStream;
class SvxBulletItem;
class SvxFont;
class SvxSearchItem;
class SvxFieldItem;
namespace vcl { class Window; }
class KeyEvent;
class MouseEvent;
class Pointer;
class CommandEvent;
class MapMode;
class OutputDevice;
namespace tools { class PolyPolygon; }
class SfxStyleSheetPool;
class SfxStyleSheet;
class SfxItemPool;
class SfxItemSet;
class SvxNumBulletItem;
class SvxNumberFormat;
class SvxLRSpaceItem;
class EditEngine;
class SvKeyValueIterator;
class SvxForbiddenCharactersTable;
class OverflowingText;
class NonOverflowingText;
class OutlinerSearchable;
class OutlinerViewCallable;

namespace svl
{
    class IUndoManager;
}

#include <com/sun/star/uno/Reference.h>

#include <rtl/ref.hxx>
#include <editeng/svxfont.hxx>
#include <editeng/eedata.hxx>
#include <editeng/paragraphdata.hxx>
#include <o3tl/typed_flags_set.hxx>

class SvxFieldData;


namespace com { namespace sun { namespace star { namespace linguistic2 {
    class XSpellChecker1;
    class XHyphenator;
}}}}
namespace svx{
struct SpellPortion;
typedef std::vector<SpellPortion> SpellPortions;
}

namespace basegfx { class B2DPolyPolygon; }

// internal use only!
enum class ParaFlag
{
    NONE               = 0x0000,
    HOLDDEPTH          = 0x4000,
    SETBULLETTEXT      = 0x8000,
    ISPAGE             = 0x0100,
};
namespace o3tl
{
    template<> struct typed_flags<ParaFlag> : is_typed_flags<ParaFlag, 0xc100> {};
}

// Undo-Action-Ids
#define OLUNDO_DEPTH            EDITUNDO_USER
// #define OLUNDO_HEIGHT           EDITUNDO_USER+1
#define OLUNDO_EXPAND           EDITUNDO_USER+2
#define OLUNDO_COLLAPSE         EDITUNDO_USER+3
// #define OLUNDO_REMOVE           EDITUNDO_USER+4
#define OLUNDO_ATTR             EDITUNDO_USER+5
#define OLUNDO_INSERT           EDITUNDO_USER+6
// #define OLUNDO_MOVEPARAGRAPHS    EDITUNDO_USER+7

class Paragraph : protected ParagraphData
{
private:
    friend class Outliner;
    friend class ParagraphList;
    friend class OutlinerView;
    friend class OutlinerParaObject;
    friend class OutlinerEditEng;
    friend class OutlinerUndoCheckPara;
    friend class OutlinerUndoChangeParaFlags;

    Paragraph& operator=(const Paragraph& rPara ) = delete;

    ParaFlag            nFlags;
    OUString            aBulText;
    Size                aBulSize;
    bool                bVisible;

    bool                IsVisible() const { return bVisible; }
    void                SetText( const OUString& rText ) { aBulText = rText; aBulSize.Width() = -1; }
    void                Invalidate() { aBulSize.Width() = -1; }
    void                SetDepth( sal_Int16 nNewDepth ) { nDepth = nNewDepth; aBulSize.Width() = -1; }
    const OUString& GetText() const { return aBulText; }

                        Paragraph( sal_Int16 nDepth );
                        Paragraph( const Paragraph& ) = delete;
                        Paragraph( const ParagraphData& );
                        ~Paragraph();

    sal_Int16           GetDepth() const { return nDepth; }

    sal_Int16           GetNumberingStartValue() const { return mnNumberingStartValue; }
    void                SetNumberingStartValue( sal_Int16 nNumberingStartValue );

    bool                IsParaIsNumberingRestart() const { return mbParaIsNumberingRestart; }
    void                SetParaIsNumberingRestart( bool bParaIsNumberingRestart );

    void                SetFlag( ParaFlag nFlag ) { nFlags |= nFlag; }
    void                RemoveFlag( ParaFlag nFlag ) { nFlags &= ~nFlag; }
    bool                HasFlag( ParaFlag nFlag ) const { return bool(nFlags & nFlag); }
};

struct ParaRange
{
    sal_Int32  nStartPara;
    sal_Int32  nEndPara;

            ParaRange( sal_Int32 nS, sal_Int32 nE ) { nStartPara = nS, nEndPara = nE; }

    void    Adjust();
};

inline void ParaRange::Adjust()
{
    if ( nStartPara > nEndPara )
    {
        sal_Int32 nTmp = nStartPara;
        nStartPara = nEndPara;
        nEndPara = nTmp;
    }
}

class EDITENG_DLLPUBLIC OutlinerView
{
    friend class Outliner;

private:

    Outliner*   pOwner;
    EditView*   pEditView;

    enum class MouseTarget {
        Text = 0,
        Bullet = 1,
        Hypertext = 2,  // Outside OutputArea
        Outside = 3,    // Outside OutputArea
        DontKnow = 4
    };

    EDITENG_DLLPRIVATE void         ImplExpandOrCollaps( sal_Int32 nStartPara, sal_Int32 nEndPara, bool bExpand );

    EDITENG_DLLPRIVATE sal_Int32    ImpCheckMousePos( const Point& rPosPixel, MouseTarget& reTarget);
    EDITENG_DLLPRIVATE void         ImpToggleExpand( Paragraph* pParentPara );
    EDITENG_DLLPRIVATE ParaRange    ImpGetSelectedParagraphs( bool bIncludeHiddenChildren );

    EDITENG_DLLPRIVATE sal_Int32    ImpInitPaste( sal_Int32& rStart );
    EDITENG_DLLPRIVATE void         ImpPasted( sal_Int32 nStart, sal_Int32 nPrevParaCount, sal_Int32 nSize);
    EDITENG_DLLPRIVATE sal_Int32    ImpCalcSelectedPages( bool bIncludeFirstSelected );

    Link<LinkParamNone*,void> aEndCutPasteLink;

public:
                OutlinerView( Outliner* pOut, vcl::Window* pWindow );
    virtual     ~OutlinerView();

    EditView&   GetEditView() const { return *pEditView; }

    void        Scroll( long nHorzScroll, long nVertScroll );

    void        Paint( const Rectangle& rRect, OutputDevice* pTargetDevice = nullptr );
    bool        PostKeyEvent( const KeyEvent& rKEvt, vcl::Window* pFrameWin = nullptr );
    bool        MouseButtonDown( const MouseEvent& );
    bool        MouseButtonUp( const MouseEvent& );
    void        ReleaseMouse();
    bool        MouseMove( const MouseEvent& );

    void        ShowCursor( bool bGotoCursor = true );
    void        HideCursor();

    Outliner*   GetOutliner() const { return pOwner; }

    void        SetWindow( vcl::Window* pWindow );
    vcl::Window*     GetWindow() const;

    void        SetReadOnly( bool bReadOnly );
    bool        IsReadOnly() const;

    void        SetOutputArea( const Rectangle& rRect );
    Rectangle   GetOutputArea() const;

    Rectangle   GetVisArea() const;

    void        CreateSelectionList (std::vector<Paragraph*> &aSelList) ;

    // Retruns the number of selected paragraphs
    sal_Int32   Select( Paragraph* pParagraph,
                    bool bSelect = true,
                    bool bWChildren = true);

    OUString    GetSelected() const;
    void        SelectRange( sal_Int32 nFirst, sal_Int32 nCount );
    void        SetAttribs( const SfxItemSet& );
    void        Indent( short nDiff );
    void        AdjustDepth( short nDX );   // Later replace with Indent!

    void        AdjustHeight( long nDY );

    sal_uLong   Read( SvStream& rInput, const OUString& rBaseURL, EETextFormat eFormat, bool bSelect = false, SvKeyValueIterator* pHTTPHeaderAttrs = nullptr );

    void        InsertText( const OUString& rNew, bool bSelect = false );
    void        InsertText( const OutlinerParaObject& rParaObj );
    void        Expand();
    void        Collapse();
    void        ExpandAll();
    void        CollapseAll();

    void        SetBackgroundColor( const Color& rColor );
    Color       GetBackgroundColor();

    /// Registers a LOK model callback.
    void registerLibreOfficeKitCallback(OutlinerSearchable* pSearchable);
    /// Registers a LOK view callback.
    void registerLibreOfficeKitViewCallback(OutlinerViewCallable* pCallable);

    SfxItemSet  GetAttribs();

    void        Cut();
    void        Copy();
    void        Paste();
    void        PasteSpecial();

    const SfxStyleSheet*  GetStyleSheet() const;
    SfxStyleSheet*  GetStyleSheet();

    void            SetControlWord( EVControlBits nWord );
    EVControlBits   GetControlWord() const;

    void            SetAnchorMode( EVAnchorMode eMode );
    EVAnchorMode    GetAnchorMode() const;

    Pointer     GetPointer( const Point& rPosPixel );
    void        Command( const CommandEvent& rCEvt );

    void            StartSpeller( bool bMultipleDoc = false );
    EESpellState    StartThesaurus();
    sal_Int32       StartSearchAndReplace( const SvxSearchItem& rSearchItem );

    // for text conversion
    void            StartTextConversion( LanguageType nSrcLang, LanguageType nDestLang, const vcl::Font *pDestFont, sal_Int32 nOptions, bool bIsInteractive, bool bMultipleDoc );

    void            TransliterateText( sal_Int32 nTransliterationMode );

    ESelection      GetSelection();

    SvtScriptType   GetSelectedScriptType() const;

    void        SetVisArea( const Rectangle& rRect );
    void        SetSelection( const ESelection& );
    void GetSelectionRectangles(std::vector<Rectangle>& rLogicRects) const;

    void        RemoveAttribs( bool bRemoveParaAttribs = false, sal_uInt16 nWhich = 0, bool bKeepLanguages = false );
    void        RemoveAttribsKeepLanguages( bool bRemoveParaAttribs );
    bool        HasSelection() const;

    void                InsertField( const SvxFieldItem& rFld );
    const SvxFieldItem* GetFieldUnderMousePointer() const;
    const SvxFieldItem* GetFieldAtSelection() const;

    /** enables bullets for the selected paragraphs if the bullets/numbering of the first paragraph is off
        or disables bullets/numbering for the selected paragraphs if the bullets/numbering of the first paragraph is on
    */
    void ToggleBullets();

    void ToggleBulletsNumbering(
        const bool bToggle,
        const bool bHandleBullets,
        const SvxNumRule* pNumRule = nullptr );

    /** apply bullets/numbering for paragraphs

        @param boolean bHandleBullets
        true: handle bullets
        false: handle numbering

        @param pNewNumRule
        numbering rule which needs to be applied. can be 0.

        @param boolean bAtSelection
        true: apply bullets/numbering at selected paragraphs
        false: apply bullets/numbering at all paragraphs
    */
    void ApplyBulletsNumbering(
        const bool bHandleBullets,
        const SvxNumRule* pNewNumRule,
        const bool bCheckCurrentNumRuleBeforeApplyingNewNumRule,
        const bool bAtSelection = false );

    /** switch off bullets/numbering for paragraphs

        @param boolean bAtSelection
        true: switch off bullets/numbering at selected paragraphs
        false: switch off bullets/numbering at all paragraphs
    */
    void SwitchOffBulletsNumbering(
        const bool bAtSelection = false );

    /** enables numbering for the selected paragraphs that are not enabled and ignore all selected
        paragraphs that already have numbering enabled.
    */
    void        EnableBullets();

    bool        IsCursorAtWrongSpelledWord();
    bool        IsWrongSpelledWordAtPos( const Point& rPosPixel, bool bMarkIfWrong = false );
    void        ExecuteSpellPopup( const Point& rPosPixel, Link<SpellCallbackInfo&,void>* pCallBack = nullptr );

    void        SetInvalidateMore( sal_uInt16 nPixel );
    sal_uInt16  GetInvalidateMore() const;

    OUString    GetSurroundingText() const;
    Selection   GetSurroundingTextSelection() const;

    void        SetEndCutPasteLinkHdl(const Link<LinkParamNone*,void> &rLink) { aEndCutPasteLink = rLink; }
};

/// Interface class to not depend on SdrModel in editeng.
class EDITENG_DLLPUBLIC OutlinerSearchable
{
public:
    virtual ~OutlinerSearchable();

    virtual void libreOfficeKitCallback(int nType, const char* pPayload) const = 0;
};

/// Interface class to not depend on SfxViewShell in editeng, meant to replace OutlinerSearchable at the end.
class SAL_NO_VTABLE SAL_DLLPUBLIC_RTTI OutlinerViewCallable
{
public:
    virtual void libreOfficeKitViewCallback(int nType, const char* pPayload) const = 0;

protected:
    ~OutlinerViewCallable() throw () {}
};

// some thesaurus functionality to avoid code duplication in different projects...
bool EDITENG_DLLPUBLIC  GetStatusValueForThesaurusFromContext( OUString &rStatusVal, LanguageType &rLang, const EditView &rEditView );
void EDITENG_DLLPUBLIC  ReplaceTextWithSynonym( EditView &rEditView, const OUString &rSynonmText );

typedef ::std::vector< OutlinerView* > ViewList;

class EDITENG_DLLPUBLIC DrawPortionInfo
{
public:
    const Point&        mrStartPos;
    const OUString      maText;
    sal_Int32           mnTextStart;
    sal_Int32           mnTextLen;
    sal_Int32           mnPara;
    const SvxFont&      mrFont;
    const long*         mpDXArray;

    const EEngineData::WrongSpellVector*  mpWrongSpellVector;
    const SvxFieldData* mpFieldData;
    const css::lang::Locale* mpLocale;
    const Color         maOverlineColor;
    const Color         maTextLineColor;

    sal_uInt8           mnBiDiLevel;

    bool                mbFilled;
    long                mnWidthToFill;

    // bitfield
    bool                mbEndOfLine : 1;
    bool                mbEndOfParagraph : 1;
    bool                mbEndOfBullet : 1;

    bool IsRTL() const { return mnBiDiLevel % 2 == 1; }

    DrawPortionInfo(
        const Point& rPos,
        const OUString& rTxt,
        sal_Int32 nTxtStart,
        sal_Int32 nTxtLen,
        const SvxFont& rFnt,
        sal_Int32 nPar,
        const long* pDXArr,
        const EEngineData::WrongSpellVector* pWrongSpellVector,
        const SvxFieldData* pFieldData,
        const css::lang::Locale* pLocale,
        const Color& rOverlineColor,
        const Color& rTextLineColor,
        sal_uInt8 nBiDiLevel,
        bool bFilled,
        long nWidthToFill,
        bool bEndOfLine,
        bool bEndOfParagraph,
        bool bEndOfBullet)
    :   mrStartPos(rPos),
        maText(rTxt),
        mnTextStart(nTxtStart),
        mnTextLen(nTxtLen),
        mnPara(nPar),
        mrFont(rFnt),
        mpDXArray(pDXArr),
        mpWrongSpellVector(pWrongSpellVector),
        mpFieldData(pFieldData),
        mpLocale(pLocale),
        maOverlineColor(rOverlineColor),
        maTextLineColor(rTextLineColor),
        mnBiDiLevel(nBiDiLevel),
        mbFilled( bFilled ),
        mnWidthToFill( nWidthToFill ),
        mbEndOfLine(bEndOfLine),
        mbEndOfParagraph(bEndOfParagraph),
        mbEndOfBullet(bEndOfBullet)
    {}
};

class EDITENG_DLLPUBLIC DrawBulletInfo
{
public:
    const GraphicObject maBulletGraphicObject;
    Point               maBulletPosition;
    Size                maBulletSize;

    DrawBulletInfo(
        const GraphicObject& rBulletGraphicObject,
        const Point& rBulletPosition,
        const Size& rBulletSize)
    :   maBulletGraphicObject(rBulletGraphicObject),
        maBulletPosition(rBulletPosition),
        maBulletSize(rBulletSize)
    {}
};

struct EDITENG_DLLPUBLIC PaintFirstLineInfo
{
    sal_Int32 mnPara;
    const Point& mrStartPos;
    VclPtr<OutputDevice> mpOutDev;

    PaintFirstLineInfo( sal_Int32 nPara, const Point& rStartPos, OutputDevice* pOutDev )
        : mnPara( nPara ), mrStartPos( rStartPos ), mpOutDev( pOutDev )
    {}
};

class SdrPage;

class EditFieldInfo
{
private:
    Outliner*           pOutliner;
    const SvxFieldItem& rFldItem;

    Color*              pTxtColor;
    Color*              pFldColor;

    OUString            aRepresentation;

    sal_Int32           nPara;
    sal_Int32           nPos;
    bool                bSimpleClick;

                        EditFieldInfo( const EditFieldInfo& ) = delete;

    SdrPage*            mpSdrPage;

public:
                    EditFieldInfo( Outliner* pOutl, const SvxFieldItem& rFItem, sal_Int32 nPa, sal_Int32 nPo )
                        : rFldItem( rFItem )
                    {
                        pOutliner = pOutl;
                        nPara = nPa; nPos = nPo;
                        pTxtColor = nullptr; pFldColor = nullptr; bSimpleClick = false;
                        mpSdrPage = nullptr;
                    }
                    ~EditFieldInfo()
                    {
                        delete pTxtColor;
                        delete pFldColor;
                    }

    Outliner*       GetOutliner() const { return pOutliner; }

    const SvxFieldItem& GetField() const { return rFldItem; }

    Color*          GetTextColor() const { return pTxtColor; }
    void            SetTextColor( const Color& rColor )
                        { delete pTxtColor; pTxtColor = new Color( rColor ); }

    Color*          GetFieldColor() const { return pFldColor; }
    void            SetFieldColor( const Color& rColor )
                        { delete pFldColor; pFldColor = new Color( rColor ); }
    void            ClearFieldColor()
                        { delete pFldColor; pFldColor = nullptr; }

    sal_Int32       GetPara() const { return nPara; }
    sal_Int32       GetPos() const { return nPos; }

    void            SetSimpleClick( bool bSimple ) { bSimpleClick = bSimple; }

    const OUString&     GetRepresentation() const                { return aRepresentation; }
    OUString&           GetRepresentation()                      { return aRepresentation; }
    void                SetRepresentation( const OUString& rStr ){ aRepresentation = rStr; }

    void            SetSdrPage( SdrPage* pPage ) { mpSdrPage = pPage; }
    SdrPage*        GetSdrPage() const { return mpSdrPage; }
};

struct EBulletInfo
{
    bool        bVisible;
    sal_uInt16  nType;          // see SvxNumberType
    OUString    aText;
    SvxFont     aFont;
    Graphic     aGraphic;
    sal_Int32   nParagraph;
    Rectangle   aBounds;

    EBulletInfo() : bVisible( false ), nType( 0 ), nParagraph( EE_PARA_NOT_FOUND ) {}
};

#define OUTLINERMODE_DONTKNOW       0x0000
#define OUTLINERMODE_TEXTOBJECT     0x0001
#define OUTLINERMODE_TITLEOBJECT    0x0002
#define OUTLINERMODE_OUTLINEOBJECT  0x0003
#define OUTLINERMODE_OUTLINEVIEW    0x0004

#define OUTLINERMODE_USERMASK       0x00FF

#define OUTLINERMODE_SUBTITLE       (0x0100|OUTLINERMODE_TEXTOBJECT)
#define OUTLINERMODE_NOTE           (0x0200|OUTLINERMODE_TEXTOBJECT)

class EDITENG_DLLPUBLIC Outliner : public SfxBroadcaster
{
    friend class OutlinerView;
    friend class OutlinerEditEng;
    friend class OutlinerParaObject;
    friend class OLUndoExpand;
    friend class OutlinerUndoChangeDepth;
    friend class OutlinerUndoCheckPara;
    friend class OutlinerUndoChangeParaFlags;

    friend class TextChainingUtils;

    OutlinerEditEng*    pEditEngine;

    ParagraphList*      pParaList;
    ViewList            aViewList;

    Paragraph*          pHdlParagraph;
    sal_Int32           mnFirstSelPage;
    Link<DrawPortionInfo*,void> aDrawPortionHdl;
    Link<DrawBulletInfo*,void>     aDrawBulletHdl;
    Link<Outliner*,void>           aParaInsertedHdl;
    Link<Outliner*,void>           aParaRemovingHdl;
    Link<Outliner*,void>           aDepthChangedHdl;
    Link<Outliner*,void>           aBeginMovingHdl;
    Link<Outliner*,void>           aEndMovingHdl;
    Link<OutlinerView*,bool>       aIndentingPagesHdl;
    Link<OutlinerView*,bool>       aRemovingPagesHdl;
    Link<EditFieldInfo*,void>      aFieldClickedHdl;
    Link<EditFieldInfo*,void>      aCalcFieldValueHdl;
    Link<PaintFirstLineInfo*,void> maPaintFirstLineHdl;
    Link<PasteOrDropInfos*,void>   maBeginPasteOrDropHdl;
    Link<PasteOrDropInfos*,void>   maEndPasteOrDropHdl;

    sal_Int32           nDepthChangedHdlPrevDepth;
    ParaFlag            mnDepthChangeHdlPrevFlags;
    sal_Int16           nMaxDepth;
    const sal_Int16     nMinDepth;
    sal_Int32           nFirstPage;

    sal_uInt16          nOutlinerMode;

    bool                bIsExpanding; // Only valid in Expand/Collaps-Hdl, reset
    bool                bFirstParaIsEmpty;
    sal_uInt8           nBlockInsCallback;
    bool                bStrippingPortions;
    bool                bPasting;

    DECL_LINK_TYPED(    ParaVisibleStateChangedHdl, Paragraph&, void );
    DECL_LINK_TYPED(    BeginMovingParagraphsHdl, MoveParagraphsInfo&, void );
    DECL_LINK_TYPED(    EndMovingParagraphsHdl, MoveParagraphsInfo&, void );
    DECL_LINK_TYPED(    BeginPasteOrDropHdl, PasteOrDropInfos&, void );
    DECL_LINK_TYPED(    EndPasteOrDropHdl, PasteOrDropInfos&, void );
    DECL_LINK_TYPED(    EditEngineNotifyHdl, EENotify&, void );
    void                ImplCheckParagraphs( sal_Int32 nStart, sal_Int32 nEnd );
    bool                ImplHasNumberFormat( sal_Int32 nPara ) const;
    Size                ImplGetBulletSize( sal_Int32 nPara );
    sal_uInt16          ImplGetNumbering( sal_Int32 nPara, const SvxNumberFormat* pParaFmt );
    void                ImplCalcBulletText( sal_Int32 nPara, bool bRecalcLevel, bool bRecalcChildren );
    OUString            ImplGetBulletText( sal_Int32 nPara );
    void                ImplCheckNumBulletItem( sal_Int32 nPara );
    void                ImplInitDepth( sal_Int32 nPara, sal_Int16 nDepth, bool bCreateUndo );
    void                ImplSetLevelDependendStyleSheet( sal_Int32 nPara );

    void                ImplBlockInsertionCallbacks( bool b );

    void        ImpFilterIndents( sal_Int32 nFirstPara, sal_Int32 nLastPara );
    bool        ImpConvertEdtToOut( sal_Int32 nPara );

    void        ImpTextPasted( sal_Int32 nStartPara, sal_Int32 nCount );
    vcl::Font   ImpCalcBulletFont( sal_Int32 nPara ) const;
    Rectangle   ImpCalcBulletArea( sal_Int32 nPara, bool bAdjust, bool bReturnPaperPos );
    bool        ImpCanIndentSelectedPages( OutlinerView* pCurView );
    bool        ImpCanDeleteSelectedPages( OutlinerView* pCurView );
    bool        ImpCanDeleteSelectedPages( OutlinerView* pCurView, sal_Int32 nFirstPage, sal_Int32 nPages );

    sal_uInt16  ImplGetOutlinerMode() const { return nOutlinerMode & OUTLINERMODE_USERMASK; }
    void        ImplCheckDepth( sal_Int16& rnDepth ) const;

protected:
    void            ParagraphInserted( sal_Int32 nParagraph );
    void            ParagraphDeleted( sal_Int32 nParagraph );
    void            ParaAttribsChanged( sal_Int32 nParagraph );

    void            StyleSheetChanged( SfxStyleSheet* pStyle );

    void            InvalidateBullet(sal_Int32 nPara);
    void            PaintBullet( sal_Int32 nPara, const Point& rStartPos,
                    const Point& rOrigin, short nOrientation,
                    OutputDevice* pOutDev );

    // used by OutlinerEditEng. Allows Outliner objects to provide
    // bullet access to the EditEngine.
    const SvxNumberFormat*  GetNumberFormat( sal_Int32 nPara ) const;

public:

                    Outliner( SfxItemPool* pPool, sal_uInt16 nOutlinerMode );
    virtual         ~Outliner();

    void            Init( sal_uInt16 nOutlinerMode );
    sal_uInt16      GetMode() const { return nOutlinerMode; }

    void            SetVertical( bool bVertical );
    bool            IsVertical() const;

    void            SetFixedCellHeight( bool bUseFixedCellHeight );

    void                        SetDefaultHorizontalTextDirection( EEHorizontalTextDirection eHTextDir );
    EEHorizontalTextDirection   GetDefaultHorizontalTextDirection() const;

    SvtScriptType   GetScriptType( const ESelection& rSelection ) const;
    LanguageType    GetLanguage( sal_Int32 nPara, sal_Int32 nPos ) const;

    void            SetAsianCompressionMode( sal_uInt16 nCompressionMode );

    void            SetKernAsianPunctuation( bool bEnabled );

    void            SetAddExtLeading( bool b );

    size_t          InsertView( OutlinerView* pView, size_t nIndex = size_t(-1) );
    void            RemoveView( OutlinerView* pView );
    OutlinerView*   RemoveView( size_t nIndex );
    OutlinerView*   GetView( size_t nIndex ) const;
    size_t          GetViewCount() const;

    Paragraph*      Insert( const OUString& rText, sal_Int32 nAbsPos = EE_PARA_APPEND, sal_Int16 nDepth = 0 );
    void            SetText( const OutlinerParaObject& );
    void            AddText( const OutlinerParaObject& );
    void            SetText( const OUString& rText, Paragraph* pParagraph );
    OUString        GetText( Paragraph* pPara, sal_Int32 nParaCount=1 ) const;

    void            SetToEmptyText();

    OutlinerParaObject* CreateParaObject( sal_Int32 nStartPara = 0, sal_Int32 nParaCount = EE_PARA_ALL ) const;

    const SfxItemSet& GetEmptyItemSet() const;

    void            SetRefMapMode( const MapMode& );
    MapMode         GetRefMapMode() const;

    void            SetBackgroundColor( const Color& rColor );
    Color           GetBackgroundColor() const;

    void            SetMaxDepth( sal_Int16 nDepth );
    sal_Int16       GetMaxDepth() const { return nMaxDepth; }

    void            SetUpdateMode( bool bUpdate );
    bool            GetUpdateMode() const;

    void            Clear();

    void            RemoveAttribs( const ESelection& rSelection, bool bRemoveParaAttribs, sal_uInt16 nWhich );

    sal_Int32       GetParagraphCount() const;
    Paragraph*      GetParagraph( sal_Int32 nAbsPos ) const;

    bool            HasChildren( Paragraph* pParagraph ) const;
    sal_Int32       GetChildCount( Paragraph* pParent ) const;
    bool            IsExpanded( Paragraph* pPara ) const;
    Paragraph*      GetParent( Paragraph* pParagraph ) const;
    sal_Int32       GetAbsPos( Paragraph* pPara );

    sal_Int16       GetDepth( sal_Int32 nPara ) const;
    void            SetDepth( Paragraph* pParagraph, sal_Int16 nNewDepth );

    void            EnableUndo( bool bEnable );
    bool            IsUndoEnabled() const;
    void            UndoActionStart( sal_uInt16 nId );
    void            UndoActionEnd( sal_uInt16 nId );
    void            InsertUndo( EditUndo* pUndo );
    bool            IsInUndo();

    void            ClearModifyFlag();
    bool            IsModified() const;

    Paragraph*      GetHdlParagraph() const { return pHdlParagraph; }

    void            ParagraphInsertedHdl();
    void            SetParaInsertedHdl(const Link<Outliner*,void>& rLink){aParaInsertedHdl=rLink;}
    Link<Outliner*,void> GetParaInsertedHdl() const { return aParaInsertedHdl; }

    void            ParagraphRemovingHdl();
    void            SetParaRemovingHdl(const Link<Outliner*,void>& rLink){aParaRemovingHdl=rLink;}
    Link<Outliner*,void> GetParaRemovingHdl() const { return aParaRemovingHdl; }

    NonOverflowingText *GetNonOverflowingText() const;
    OverflowingText *GetOverflowingText() const;
    void ClearOverflowingParaNum();
    bool IsPageOverflow();

    OutlinerParaObject *GetEmptyParaObject() const;


    void            DepthChangedHdl();
    void            SetDepthChangedHdl(const Link<Outliner*,void>& rLink){aDepthChangedHdl=rLink;}
    Link<Outliner*,void> GetDepthChangedHdl() const { return aDepthChangedHdl; }
    sal_Int16       GetPrevDepth() const { return static_cast<sal_Int16>(nDepthChangedHdlPrevDepth); }
    ParaFlag        GetPrevFlags() const { return mnDepthChangeHdlPrevFlags; }

    bool            RemovingPagesHdl( OutlinerView* );
    void            SetRemovingPagesHdl(const Link<OutlinerView*,bool>& rLink){aRemovingPagesHdl=rLink;}
    bool            IndentingPagesHdl( OutlinerView* );
    void            SetIndentingPagesHdl(const Link<OutlinerView*,bool>& rLink){aIndentingPagesHdl=rLink;}
    // valid only in the two upper handlers
    sal_Int32       GetSelPageCount() const { return nDepthChangedHdlPrevDepth; }

    void            SetCalcFieldValueHdl(const Link<EditFieldInfo*,void>& rLink ) { aCalcFieldValueHdl= rLink; }
    Link<EditFieldInfo*,void> GetCalcFieldValueHdl() const { return aCalcFieldValueHdl; }

    void            SetDrawPortionHdl(const Link<DrawPortionInfo*,void>& rLink){aDrawPortionHdl=rLink;}

    void            SetDrawBulletHdl(const Link<DrawBulletInfo*,void>& rLink){aDrawBulletHdl=rLink;}

    void            SetPaintFirstLineHdl(const Link<PaintFirstLineInfo*,void>& rLink) { maPaintFirstLineHdl = rLink; }

    void            SetModifyHdl( const Link<LinkParamNone*,void>& rLink );
    Link<LinkParamNone*,void> GetModifyHdl() const;

    void            SetNotifyHdl( const Link<EENotify&,void>& rLink );

    void            SetStatusEventHdl( const Link<EditStatus&, void>& rLink );
    Link<EditStatus&, void> GetStatusEventHdl() const;

    void            Draw( OutputDevice* pOutDev, const Rectangle& rOutRect );
    void            Draw( OutputDevice* pOutDev, const Point& rStartPos );

    const Size&     GetPaperSize() const;
    void            SetPaperSize( const Size& rSize );

    void            SetFirstPageNumber( sal_Int32 n )  { nFirstPage = n; }

    void            SetPolygon( const basegfx::B2DPolyPolygon& rPolyPolygon );
    void            SetPolygon( const basegfx::B2DPolyPolygon& rPolyPolygon, const basegfx::B2DPolyPolygon* pLinePolyPolygon);
    void            ClearPolygon();

    const Size&     GetMinAutoPaperSize() const;
    void            SetMinAutoPaperSize( const Size& rSz );

    const Size&     GetMaxAutoPaperSize() const;
    void            SetMaxAutoPaperSize( const Size& rSz );

    void            SetDefTab( sal_uInt16 nTab );

    bool            IsFlatMode() const;
    void            SetFlatMode( bool bFlat );

    void            EnableAutoColor( bool b );

    void            ForceAutoColor( bool b );
    bool            IsForceAutoColor() const;

    EBulletInfo     GetBulletInfo( sal_Int32 nPara );

    void            SetWordDelimiters( const OUString& rDelimiters );
    OUString        GetWordDelimiters() const;
    OUString        GetWord( sal_Int32 nPara, sal_Int32 nIndex );

    void            StripPortions();

    void DrawingText( const Point& rStartPos, const OUString& rText,
                              sal_Int32 nTextStart, sal_Int32 nTextLen,
                              const long* pDXArray, const SvxFont& rFont,
                              sal_Int32 nPara, sal_uInt8 nRightToLeft,
                              const EEngineData::WrongSpellVector* pWrongSpellVector,
                              const SvxFieldData* pFieldData,
                              bool bEndOfLine,
                              bool bEndOfParagraph,
                              bool bEndOfBullet,
                              const css::lang::Locale* pLocale,
                              const Color& rOverlineColor,
                              const Color& rTextLineColor);

    void DrawingTab( const Point& rStartPos, long nWidth, const OUString& rChar,
                             const SvxFont& rFont, sal_Int32 nPara, sal_uInt8 nRightToLeft,
                             bool bEndOfLine,
                             bool bEndOfParagraph,
                             const Color& rOverlineColor,
                             const Color& rTextLineColor);

    Size            CalcTextSize();
    Size            CalcTextSizeNTP();

    void                SetStyleSheetPool( SfxStyleSheetPool* pSPool );
    SfxStyleSheetPool*  GetStyleSheetPool();

    bool            IsInSelectionMode() const;

    void            SetStyleSheet( sal_Int32 nPara, SfxStyleSheet* pStyle );
    SfxStyleSheet*  GetStyleSheet( sal_Int32 nPara );

    void            SetParaAttribs( sal_Int32 nPara, const SfxItemSet& );
    SfxItemSet      GetParaAttribs( sal_Int32 nPara );

    void            Remove( Paragraph* pPara, sal_Int32 nParaCount );
    bool            Expand( Paragraph* );
    bool            Collapse( Paragraph* );

    void            SetParaFlag( Paragraph* pPara,  ParaFlag nFlag );
    static bool     HasParaFlag( const Paragraph* pPara, ParaFlag nFlag );


    void            SetControlWord( EEControlBits nWord );
    EEControlBits   GetControlWord() const;

    Link<Outliner*,void> GetBeginMovingHdl() const { return aBeginMovingHdl; }
    void            SetBeginMovingHdl(const Link<Outliner*,void>& rLink) {aBeginMovingHdl=rLink;}
    Link<Outliner*,void> GetEndMovingHdl() const {return aEndMovingHdl;}
    void            SetEndMovingHdl( const Link<Outliner*,void>& rLink){aEndMovingHdl=rLink;}

    sal_uLong           GetLineCount( sal_Int32 nParagraph ) const;
    sal_Int32           GetLineLen( sal_Int32 nParagraph, sal_Int32 nLine ) const;
    sal_uLong           GetLineHeight( sal_Int32 nParagraph );

    sal_uLong           Read( SvStream& rInput, const OUString& rBaseURL, sal_uInt16, SvKeyValueIterator* pHTTPHeaderAttrs = nullptr );

    ::svl::IUndoManager& GetUndoManager();
    ::svl::IUndoManager* SetUndoManager(::svl::IUndoManager* pNew);

    void            QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel );
    void            QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel );
    void            QuickInsertLineBreak( const ESelection& rSel );

    // Only for EditEngine mode
    void            QuickInsertText( const OUString& rText, const ESelection& rSel );
    void            QuickDelete( const ESelection& rSel );
    void            RemoveCharAttribs( sal_Int32 nPara, sal_uInt16 nWhich = 0 );
    void            QuickFormatDoc();

    bool            UpdateFields();
    void            RemoveFields( const std::function<bool ( const SvxFieldData* )>& isFieldData = [] (const SvxFieldData* ){return true;} );

    void            FieldClicked( const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos );
    virtual OUString CalcFieldValue( const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos, Color*& rTxtColor, Color*& rFldColor );

    void            SetSpeller( css::uno::Reference< css::linguistic2::XSpellChecker1 > &xSpeller );
    css::uno::Reference< css::linguistic2::XSpellChecker1 >
                    GetSpeller();
    void            SetHyphenator( css::uno::Reference< css::linguistic2::XHyphenator >& xHyph );

    static void     SetForbiddenCharsTable( const rtl::Reference<SvxForbiddenCharactersTable>& xForbiddenChars );

    // Deprecated
    void            SetDefaultLanguage( LanguageType eLang );
    LanguageType    GetDefaultLanguage() const;

    void            CompleteOnlineSpelling();

    EESpellState    HasSpellErrors();
    bool            HasText( const SvxSearchItem& rSearchItem );
    virtual bool    SpellNextDocument();

    // for text conversion
    bool            HasConvertibleTextPortion( LanguageType nLang );
    virtual bool    ConvertNextDocument();

    void            SetEditTextObjectPool( SfxItemPool* pPool );
    SfxItemPool*    GetEditTextObjectPool() const;

    void            SetRefDevice( OutputDevice* pRefDev );
    OutputDevice*   GetRefDevice() const;

    sal_uLong       GetTextHeight() const;
    sal_uLong       GetTextHeight( sal_Int32 nParagraph ) const;
    Point           GetDocPosTopLeft( sal_Int32 nParagraph );
    Point           GetDocPos( const Point& rPaperPos ) const;
    bool            IsTextPos( const Point& rPaperPos, sal_uInt16 nBorder = 0 );
    bool            IsTextPos( const Point& rPaperPos, sal_uInt16 nBorder, bool* pbBulletPos );

    void            SetGlobalCharStretching( sal_uInt16 nX = 100, sal_uInt16 nY = 100 );
    void            GetGlobalCharStretching( sal_uInt16& rX, sal_uInt16& rY ) const;
    void            EraseVirtualDevice();

    bool            ShouldCreateBigTextObject() const;

    const EditEngine& GetEditEngine() const;

    // this is needed for StarOffice Api
    void            SetLevelDependendStyleSheet( sal_Int32 nPara );

    sal_uInt16      GetOutlinerMode() const { return nOutlinerMode & OUTLINERMODE_USERMASK; }

    // spell and return a sentence
    bool            SpellSentence(EditView& rEditView, svx::SpellPortions& rToFill, bool bIsGrammarChecking );
    // put spell position to start of current sentence
    void            PutSpellingToSentenceStart( EditView& rEditView );
    // applies a changed sentence
    void            ApplyChangedSentence(EditView& rEditView, const svx::SpellPortions& rNewPortions, bool bRecheck );

    /** sets a link that is called at the beginning of a drag operation at an edit view */
    void            SetBeginDropHdl( const Link<EditView*,void>& rLink );

    /** sets a link that is called at the end of a drag operation at an edit view */
    void            SetEndDropHdl( const Link<EditView*,void>& rLink );

    /** sets a link that is called before a drop or paste operation. */
    void            SetBeginPasteOrDropHdl( const Link<PasteOrDropInfos*,void>& rLink );

    /** sets a link that is called after a drop or paste operation. */
    void            SetEndPasteOrDropHdl( const Link<PasteOrDropInfos*,void>& rLink );

    sal_Int16 GetNumberingStartValue( sal_Int32 nPara );
    void SetNumberingStartValue( sal_Int32 nPara, sal_Int16 nNumberingStartValue );

    bool IsParaIsNumberingRestart( sal_Int32 nPara );
    void SetParaIsNumberingRestart( sal_Int32 nPara, bool bParaIsNumberingRestart );

    /** determine the bullets/numbering status of the given paragraphs

        @param nParaStart
        index of paragraph at which the check starts

        @param nParaEnd
        index of paragraph at which the check ends

        @returns
        0 : all paragraphs have bullets
        1 : all paragraphs have numbering
        2 : otherwise
    */
    sal_Int32 GetBulletsNumberingStatus(
        const sal_Int32 nParaStart,
        const sal_Int32 nParaEnd ) const;

    // convenient method to determine the bullets/numbering status for all paragraphs
    sal_Int32 GetBulletsNumberingStatus() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
