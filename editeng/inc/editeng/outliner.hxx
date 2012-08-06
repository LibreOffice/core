/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _OUTLINER_HXX
#define _OUTLINER_HXX

#include <tools/gen.hxx>
#include <tools/string.hxx>

#include <svl/brdcst.hxx>

#include <editeng/editdata.hxx>
#include <i18npool/lang.h>
#include <tools/color.hxx>
#include <vcl/graph.hxx>
#include <tools/link.hxx>
#include <rsc/rscsfx.hxx>
#include "editeng/editengdllapi.h"
#include <com/sun/star/lang/Locale.hpp>

#include <svtools/grfmgr.hxx>

#include <tools/rtti.hxx>   // due to typedef TypeId
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
class Window;
class KeyEvent;
class MouseEvent;
class Pointer;
class CommandEvent;
class MapMode;
class OutputDevice;
class PolyPolygon;
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

namespace svl
{
    class IUndoManager;
}

#include <com/sun/star/uno/Reference.h>

#include <rtl/ref.hxx>
#include <editeng/svxfont.hxx>
#include <editeng/eedata.hxx>
#include <editeng/paragraphdata.hxx>

class SvxFieldData;
//////////////////////////////////////////////////////////////////////////////

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
#define PARAFLAG_DROPTARGET         0x1000
#define PARAFLAG_DROPTARGET_EMPTY   0x2000
#define PARAFLAG_HOLDDEPTH          0x4000
#define PARAFLAG_SETBULLETTEXT      0x8000
#define PARAFLAG_ISPAGE             0x0100

// Undo-Action-Ids
#define OLUNDO_DEPTH            EDITUNDO_USER
// #define OLUNDO_HEIGHT           EDITUNDO_USER+1
#define OLUNDO_EXPAND           EDITUNDO_USER+2
#define OLUNDO_COLLAPSE         EDITUNDO_USER+3
// #define OLUNDO_REMOVE           EDITUNDO_USER+4
#define OLUNDO_ATTR             EDITUNDO_USER+5
#define OLUNDO_INSERT           EDITUNDO_USER+6
// #define OLUNDO_MOVEPARAGRAPHS    EDITUNDO_USER+7
#define OLUNDO_CHECKPARA        EDITUNDO_USER+8

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

    Paragraph& operator=(const Paragraph& rPara );

    sal_uInt16          nFlags;
    rtl::OUString       aBulText;
    Size                aBulSize;
    sal_Bool            bVisible;

    sal_Bool                IsVisible() const { return bVisible; }
    void                SetText( const rtl::OUString& rText ) { aBulText = rText; aBulSize.Width() = -1; }
    void                Invalidate() { aBulSize.Width() = -1; }
    void                SetDepth( sal_Int16 nNewDepth ) { nDepth = nNewDepth; aBulSize.Width() = -1; }
    const rtl::OUString& GetText() const { return aBulText; }

                        Paragraph( sal_Int16 nDepth );
                        Paragraph( const Paragraph& );
                        Paragraph( const ParagraphData& );
                        ~Paragraph();

    sal_Int16           GetDepth() const { return nDepth; }

    sal_Int16           GetNumberingStartValue() const { return mnNumberingStartValue; }
    void                SetNumberingStartValue( sal_Int16 nNumberingStartValue );

    sal_Bool            IsParaIsNumberingRestart() const { return mbParaIsNumberingRestart; }
    void                SetParaIsNumberingRestart( sal_Bool bParaIsNumberingRestart );

    void                SetFlag( sal_uInt16 nFlag ) { nFlags |= nFlag; }
    void                RemoveFlag( sal_uInt16 nFlag ) { nFlags &= ~nFlag; }
    bool                HasFlag( sal_uInt16 nFlag ) const { return (nFlags & nFlag) != 0; }
};

struct ParaRange
{
    sal_uInt16  nStartPara;
    sal_uInt16  nEndPara;

            ParaRange( sal_uInt16 nS, sal_uInt16 nE ) { nStartPara = nS, nEndPara = nE; }

    void    Adjust();
    sal_uInt16  Len() const { return 1 + ( ( nEndPara > nStartPara ) ? (nEndPara-nStartPara) : (nStartPara-nEndPara) ); }
};

inline void ParaRange::Adjust()
{
    if ( nStartPara > nEndPara )
    {
        sal_uInt16 nTmp = nStartPara;
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

    // Drag & Drop
    sal_Bool        bBeginDragAtMove_OLDMEMBER;
    sal_Bool        bInDragMode;
    Point       aDDStartPosRef;
    Point       aDDStartPosPix;
    sal_uLong       nDDStartPara;
    sal_uLong       nDDStartParaVisChildCount;
    sal_uLong       nDDCurPara;
    sal_uInt16      nDDStartDepth;
    sal_uInt16      nDDCurDepth;
    sal_uInt16      nDDMaxDepth;
    sal_Bool        bDDChangingDepth;
    sal_Bool        bDDCursorVisible;
    long*       pHorTabArrDoc;
    long        nDDScrollLRBorderWidthWin;  // Left Right
    long        nDDScrollTBBorderWidthWin;  // Top Bottom
    long        nDDScrollLROffs;
    long        nDDScrollTDOffs;

    void*       pDummy;
    sal_uLong       nDummy;

    enum MouseTarget {
        MouseText = 0,
        MouseBullet = 1,
        MouseHypertext = 2,  // Outside OutputArea
        MouseOutside = 3,    // Outside OutputArea
        MouseDontKnow = 4
    };
    MouseTarget OLD_ePrevMouseTarget;

#ifdef _OUTLINER_CXX

    EDITENG_DLLPRIVATE void         ImplExpandOrCollaps( sal_uInt16 nStartPara, sal_uInt16 nEndPara, sal_Bool bExpand );

    EDITENG_DLLPRIVATE sal_uLong       ImpCheckMousePos( const Point& rPosPixel, MouseTarget& reTarget);
    EDITENG_DLLPRIVATE void        ImpToggleExpand( Paragraph* pParentPara );
    EDITENG_DLLPRIVATE ParaRange    ImpGetSelectedParagraphs( sal_Bool bIncludeHiddenChildren );

    EDITENG_DLLPRIVATE Pointer     ImpGetMousePointer( MouseTarget eTarget );
    EDITENG_DLLPRIVATE sal_uInt16      ImpInitPaste( sal_uLong& rStart );
    EDITENG_DLLPRIVATE void        ImpPasted( sal_uLong nStart, sal_uLong nPrevParaCount, sal_uInt16 nSize);
    EDITENG_DLLPRIVATE sal_uInt16      ImpCalcSelectedPages( sal_Bool bIncludeFirstSelected );
    EDITENG_DLLPRIVATE sal_Bool        ImpIsIndentingPages();

#endif

public:
                OutlinerView( Outliner* pOut, Window* pWindow );
    virtual     ~OutlinerView();

    EditView&   GetEditView() const { return *pEditView; }

    void        Scroll( long nHorzScroll, long nVertScroll );

    void        Paint( const Rectangle& rRect );
    sal_Bool        PostKeyEvent( const KeyEvent& rKEvt, Window* pFrameWin = NULL );
    sal_Bool        MouseButtonDown( const MouseEvent& );
    sal_Bool        MouseButtonUp( const MouseEvent& );
    sal_Bool        MouseMove( const MouseEvent& );

    void        ShowCursor( sal_Bool bGotoCursor = sal_True );
    void        HideCursor();

    Outliner*   GetOutliner() const { return pOwner; }

    void        SetWindow( Window* pWindow );
    Window*     GetWindow() const;

    void        SetReadOnly( sal_Bool bReadOnly );
    sal_Bool        IsReadOnly() const;

    void        SetOutputArea( const Rectangle& rRect );
    Rectangle   GetOutputArea() const;

    Rectangle   GetVisArea() const;

    void        CreateSelectionList (std::vector<Paragraph*> &aSelList) ;

    // Retruns the number of selected paragraphs
    sal_uLong       Select( Paragraph* pParagraph,
                    sal_Bool bSelect=sal_True,
                    sal_Bool bWChildren=sal_True);

    String      GetSelected() const;
    void        SelectRange( sal_uLong nFirst, sal_uInt16 nCount );
    void        SetAttribs( const SfxItemSet& );
    void        Indent( short nDiff );
    void        AdjustDepth( short nDX );   // Later replace with Indent!

    sal_Bool        AdjustHeight( long nDY );

    sal_uLong       Read( SvStream& rInput, const String& rBaseURL, EETextFormat eFormat, sal_Bool bSelect = sal_False, SvKeyValueIterator* pHTTPHeaderAttrs = NULL );

    void        InsertText( const String& rNew, sal_Bool bSelect = sal_False );
    void        InsertText( const OutlinerParaObject& rParaObj );
    void        Expand();
    void        Collapse();
    void        ExpandAll();
    void        CollapseAll();

    void        SetBackgroundColor( const Color& rColor );
    Color       GetBackgroundColor();

    SfxItemSet  GetAttribs();

    void        Cut();
    void        Copy();
    void        Paste();
    void        PasteSpecial();

    const SfxStyleSheet*  GetStyleSheet() const;
    SfxStyleSheet*  GetStyleSheet();

    void        SetControlWord( sal_uLong nWord );
    sal_uLong       GetControlWord() const;

    void            SetAnchorMode( EVAnchorMode eMode );
    EVAnchorMode    GetAnchorMode() const;

    Pointer     GetPointer( const Point& rPosPixel );
    void        Command( const CommandEvent& rCEvt );

    EESpellState    StartSpeller( sal_Bool bMultipleDoc = sal_False );
    EESpellState    StartThesaurus();
    sal_uInt16          StartSearchAndReplace( const SvxSearchItem& rSearchItem );

    // for text conversion
    void            StartTextConversion( LanguageType nSrcLang, LanguageType nDestLang, const Font *pDestFont, sal_Int32 nOptions, sal_Bool bIsInteractive, sal_Bool bMultipleDoc );

    void            TransliterateText( sal_Int32 nTransliterationMode );

    ESelection  GetSelection();

    sal_uInt16      GetSelectedScriptType() const;

    void        SetVisArea( const Rectangle& rRec );
    void        SetSelection( const ESelection& );

    void        RemoveAttribs( sal_Bool bRemoveParaAttribs = sal_False, sal_uInt16 nWhich = 0, sal_Bool bKeepLanguages = sal_False );
    void        RemoveAttribsKeepLanguages( sal_Bool bRemoveParaAttribs );
    sal_Bool        HasSelection() const;

    void                InsertField( const SvxFieldItem& rFld );
    const SvxFieldItem* GetFieldUnderMousePointer() const;
    const SvxFieldItem* GetFieldAtSelection() const;

    /** enables numbering for the selected paragraphs if the numbering of the first paragraph is off
        or disables numbering for the selected paragraphs if the numbering of the first paragraph is on
    */
    void        ToggleBullets();

    /** enables numbering for the selected paragraphs that are not enabled and ignore all selected
        paragraphs that already have numbering enabled.
    */
    void        EnableBullets();

    sal_Bool        IsCursorAtWrongSpelledWord( sal_Bool bMarkIfWrong = sal_False );
    sal_Bool        IsWrongSpelledWordAtPos( const Point& rPosPixel, sal_Bool bMarkIfWrong = sal_False );
    void        ExecuteSpellPopup( const Point& rPosPixel, Link* pCallBack = 0 );

    void        SetInvalidateMore( sal_uInt16 nPixel );
    sal_uInt16      GetInvalidateMore() const;

    String      GetSurroundingText() const;
     Selection  GetSurroundingTextSelection() const;
};


// some thesaurus functionality to avoid code duplication in different projects...
bool EDITENG_DLLPUBLIC  GetStatusValueForThesaurusFromContext( String &rStatusVal, LanguageType &rLang, const EditView &rEditView );
void EDITENG_DLLPUBLIC  ReplaceTextWithSynonym( EditView &rEditView, const String &rSynonmText );

typedef ::std::vector< OutlinerView* > ViewList;

class EDITENG_DLLPUBLIC DrawPortionInfo
{
public:
    const Point&        mrStartPos;
    const String&       mrText;
    sal_uInt16          mnTextStart;
    sal_uInt16          mnTextLen;
    sal_uInt16          mnPara;
    xub_StrLen          mnIndex;
    const SvxFont&      mrFont;
    const sal_Int32*    mpDXArray;

    const EEngineData::WrongSpellVector*  mpWrongSpellVector;
    const SvxFieldData* mpFieldData;
    const ::com::sun::star::lang::Locale* mpLocale;
    const Color maOverlineColor;
    const Color maTextLineColor;

    sal_uInt8               mnBiDiLevel;

    bool                mbFilled;
    long                mnWidthToFill;

    // bitfield
    unsigned            mbEndOfLine : 1;
    unsigned            mbEndOfParagraph : 1;
    unsigned            mbEndOfBullet : 1;

    sal_uInt8 GetBiDiLevel() const { return mnBiDiLevel; }
    sal_Bool IsRTL() const;

    DrawPortionInfo(
        const Point& rPos,
        const String& rTxt,
        sal_uInt16 nTxtStart,
        sal_uInt16 nTxtLen,
        const SvxFont& rFnt,
        sal_uInt16 nPar,
        xub_StrLen nIdx,
        const sal_Int32* pDXArr,
        const EEngineData::WrongSpellVector* pWrongSpellVector,
        const SvxFieldData* pFieldData,
        const ::com::sun::star::lang::Locale* pLocale,
        const Color& rOverlineColor,
        const Color& rTextLineColor,
        sal_uInt8 nBiDiLevel,
        bool bFilled,
        long nWidthToFill,
        bool bEndOfLine,
        bool bEndOfParagraph,
        bool bEndOfBullet)
    :   mrStartPos(rPos),
        mrText(rTxt),
        mnTextStart(nTxtStart),
        mnTextLen(nTxtLen),
        mnPara(nPar),
        mnIndex(nIdx),
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
    sal_uInt16 mnPara;
    const Point& mrStartPos;
    long mnBaseLineY;
    const Point& mrOrigin;
    short mnOrientation;
    OutputDevice* mpOutDev;

    PaintFirstLineInfo( sal_uInt16 nPara, const Point& rStartPos, long nBaseLineY, const Point& rOrigin, short nOrientation, OutputDevice* pOutDev )
        : mnPara( nPara ), mrStartPos( rStartPos ), mnBaseLineY( nBaseLineY ), mrOrigin( rOrigin ), mnOrientation( nOrientation ), mpOutDev( pOutDev )
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

    String              aRepresentation;

    sal_uInt16              nPara;
    xub_StrLen          nPos;
    sal_Bool                bSimpleClick;

                        EditFieldInfo();
                        EditFieldInfo( const EditFieldInfo& );

    SdrPage*            mpSdrPage;

public:
                    EditFieldInfo( Outliner* pOutl, const SvxFieldItem& rFItem, sal_uInt16 nPa, xub_StrLen nPo )
                        : rFldItem( rFItem )
                    {
                        pOutliner = pOutl;
                        nPara = nPa; nPos = nPo;
                        pTxtColor = 0; pFldColor = 0; bSimpleClick = sal_False;
                        mpSdrPage = 0;
                    }
                    ~EditFieldInfo()
                    {
                        delete pTxtColor;
                        delete pFldColor;
                    }

    Outliner*       GetOutliner() const { return pOutliner; }

    const SvxFieldItem& GetField() const { return rFldItem; }

    Color*          GetTxtColor() const { return pTxtColor; }
    void            SetTxtColor( const Color& rColor )
                        { delete pTxtColor; pTxtColor = new Color( rColor ); }

    Color*          GetFldColor() const { return pFldColor; }
    void            SetFldColor( const Color& rColor )
                        { delete pFldColor; pFldColor = new Color( rColor ); }
    void            ClearFldColor()
                        { delete pFldColor; pFldColor = 0; }

    sal_uInt16          GetPara() const { return nPara; }
    xub_StrLen          GetPos() const { return nPos; }

    sal_Bool            IsSimpleClick() const { return bSimpleClick; }
    void            SetSimpleClick( sal_Bool bSimple ) { bSimpleClick = bSimple; }

    const String&       GetRepresentation() const                { return aRepresentation; }
    String&             GetRepresentation()                      { return aRepresentation; }
    void                SetRepresentation( const String& rStr )  { aRepresentation = rStr; }

    void            SetSdrPage( SdrPage* pPage ) { mpSdrPage = pPage; }
    SdrPage*        GetSdrPage() const { return mpSdrPage; }
};

struct EBulletInfo
{
    sal_Bool        bVisible;
    sal_uInt16      nType;          // see SvxNumberType
    String      aText;
    SvxFont     aFont;
    Graphic     aGraphic;
    sal_uInt16      nParagraph;
    Rectangle   aBounds;

    EBulletInfo() : bVisible( sal_False ), nType( 0 ), nParagraph( EE_PARA_NOT_FOUND ) {}
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

    OutlinerEditEng*    pEditEngine;

    ParagraphList*      pParaList;
    ViewList            aViewList;

    Paragraph*          pHdlParagraph;
    sal_uLong               mnFirstSelPage;
    Link                aDrawPortionHdl;
    Link                aDrawBulletHdl;
    Link                aExpandHdl;
    Link                aParaInsertedHdl;
    Link                aParaRemovingHdl;
    Link                aDepthChangedHdl;
    Link                aWidthArrReqHdl;
    Link                aBeginMovingHdl;
    Link                aEndMovingHdl;
    Link                aIndentingPagesHdl;
    Link                aRemovingPagesHdl;
    Link                aFieldClickedHdl;
    Link                aCalcFieldValueHdl;
    Link                maPaintFirstLineHdl;
    Link                maBeginPasteOrDropHdl;
    Link                maEndPasteOrDropHdl;

    sal_Int16           nDepthChangedHdlPrevDepth;
    sal_uInt16              mnDepthChangeHdlPrevFlags;
    sal_Int16           nMaxDepth;
    const sal_Int16     nMinDepth;
    sal_uInt16              nFirstPage;

    sal_uInt16              nOutlinerMode;

    sal_Bool                bIsExpanding; // Only valid in Expand/Collaps-Hdl, reset
    sal_Bool                bFirstParaIsEmpty;
    sal_Bool                bBlockInsCallback;
    sal_Bool                bStrippingPortions;
    sal_Bool                bPasting;

    sal_uLong               nDummy;

#ifdef _OUTLINER_CXX

    DECL_LINK(              ParaVisibleStateChangedHdl, Paragraph* );
    DECL_LINK(              BeginMovingParagraphsHdl, void* );
    DECL_LINK(              EndMovingParagraphsHdl, MoveParagraphsInfo* );
    DECL_LINK(              BeginPasteOrDropHdl, PasteOrDropInfos* );
    DECL_LINK(              EndPasteOrDropHdl, PasteOrDropInfos* );
    DECL_LINK(              EditEngineNotifyHdl, EENotify* );
    void                    ImplCheckParagraphs( sal_uInt16 nStart, sal_uInt16 nEnd );
    sal_Bool                    ImplHasBullet( sal_uInt16 nPara ) const;
    Size                    ImplGetBulletSize( sal_uInt16 nPara );
    sal_uInt16              ImplGetNumbering( sal_uInt16 nPara, const SvxNumberFormat* pParaFmt );
    void                    ImplCalcBulletText( sal_uInt16 nPara, sal_Bool bRecalcLevel, sal_Bool bRecalcChildren );
    String                  ImplGetBulletText( sal_uInt16 nPara );
    void                    ImplCheckNumBulletItem( sal_uInt16 nPara );
    void                    ImplInitDepth( sal_uInt16 nPara, sal_Int16 nDepth, sal_Bool bCreateUndo, sal_Bool bUndoAction = sal_False );
    void                    ImplSetLevelDependendStyleSheet( sal_uInt16 nPara, SfxStyleSheet* pLevelStyle = NULL );

    void                    ImplBlockInsertionCallbacks( sal_Bool b );

    void                ImplCheckStyleSheet( sal_uInt16 nPara, sal_Bool bReplaceExistingStyle );
    void                ImpRecalcBulletIndent( sal_uLong nPara );

    const SvxBulletItem& ImpGetBullet( sal_uLong nPara, sal_uInt16& );
    void        ImpFilterIndents( sal_uLong nFirstPara, sal_uLong nLastPara );
    bool        ImpConvertEdtToOut( sal_uInt32 nPara, EditView* pView = 0 );

    void        ImpTextPasted( sal_uLong nStartPara, sal_uInt16 nCount );
    long        ImpCalcMaxBulletWidth( sal_uInt16 nPara, const SvxBulletItem& rBullet );
    Font        ImpCalcBulletFont( sal_uInt16 nPara ) const;
    Rectangle   ImpCalcBulletArea( sal_uInt16 nPara, sal_Bool bAdjust, sal_Bool bReturnPaperPos );
    long        ImpGetTextIndent( sal_uLong nPara );
    sal_Bool        ImpCanIndentSelectedPages( OutlinerView* pCurView );
    sal_Bool        ImpCanDeleteSelectedPages( OutlinerView* pCurView );
    sal_Bool        ImpCanDeleteSelectedPages( OutlinerView* pCurView, sal_uInt16 nFirstPage, sal_uInt16 nPages );

    sal_uInt16      ImplGetOutlinerMode() const { return nOutlinerMode & OUTLINERMODE_USERMASK; }
    void        ImplCheckDepth( sal_Int16& rnDepth ) const;
#endif

protected:
    void            ParagraphInserted( sal_uInt16 nParagraph );
    void            ParagraphDeleted( sal_uInt16 nParagraph );
    void            ParaAttribsChanged( sal_uInt16 nParagraph );

    virtual void    StyleSheetChanged( SfxStyleSheet* pStyle );

    void        InvalidateBullet( Paragraph* pPara, sal_uLong nPara );
    void        PaintBullet( sal_uInt16 nPara, const Point& rStartPos,
                    const Point& rOrigin, short nOrientation,
                    OutputDevice* pOutDev );

    // used by OutlinerEditEng. Allows Outliner objects to provide
    // bullet access to the EditEngine.
    virtual const SvxNumberFormat*  GetNumberFormat( sal_uInt16 nPara ) const;

public:

                    Outliner( SfxItemPool* pPool, sal_uInt16 nOutlinerMode );
    virtual         ~Outliner();

    void            Init( sal_uInt16 nOutlinerMode );
    sal_uInt16          GetMode() const { return nOutlinerMode; }

    void            SetVertical( sal_Bool bVertical );
    sal_Bool            IsVertical() const;

    void            SetFixedCellHeight( sal_Bool bUseFixedCellHeight );

    void                        SetDefaultHorizontalTextDirection( EEHorizontalTextDirection eHTextDir );
    EEHorizontalTextDirection   GetDefaultHorizontalTextDirection() const;

    sal_uInt16          GetScriptType( const ESelection& rSelection ) const;
    LanguageType    GetLanguage( sal_uInt16 nPara, sal_uInt16 nPos ) const;

    void            SetAsianCompressionMode( sal_uInt16 nCompressionMode );

    void            SetKernAsianPunctuation( sal_Bool bEnabled );

    void            SetAddExtLeading( sal_Bool b );

    size_t          InsertView( OutlinerView* pView, size_t nIndex = size_t(-1) );
    OutlinerView*   RemoveView( OutlinerView* pView );
    OutlinerView*   RemoveView( size_t nIndex );
    OutlinerView*   GetView( size_t nIndex ) const;
    size_t          GetViewCount() const;

    Paragraph*      Insert( const String& rText, sal_uLong nAbsPos = LIST_APPEND, sal_Int16 nDepth = 0 );
    void            SetText( const OutlinerParaObject& );
    void            AddText( const OutlinerParaObject& );
    void            SetText( const String& rText, Paragraph* pParagraph );
    String          GetText( Paragraph* pPara, sal_uLong nParaCount=1 ) const;

    OutlinerParaObject* CreateParaObject( sal_uInt16 nStartPara = 0, sal_uInt16 nParaCount = 0xFFFF ) const;

    const SfxItemSet& GetEmptyItemSet() const;

    void            SetRefMapMode( const MapMode& );
    MapMode         GetRefMapMode() const;

    void            SetBackgroundColor( const Color& rColor );
    Color           GetBackgroundColor() const;

    sal_Int16       GetMinDepth() const { return -1; }

    void            SetMaxDepth( sal_Int16 nDepth, sal_Bool bCheckParas = sal_False );
    sal_Int16          GetMaxDepth() const { return nMaxDepth; }

    void            SetUpdateMode( sal_Bool bUpdate );
    sal_Bool            GetUpdateMode() const;

    void            Clear();

    void            RemoveAttribs( const ESelection& rSelection, sal_Bool bRemoveParaAttribs, sal_uInt16 nWhich );

    sal_uLong           GetParagraphCount() const;
    Paragraph*      GetParagraph( sal_uLong nAbsPos ) const;

    sal_Bool            HasParent( Paragraph* pParagraph ) const;
    sal_Bool            HasChildren( Paragraph* pParagraph ) const;
    sal_uLong           GetChildCount( Paragraph* pParent ) const;
    sal_Bool            IsExpanded( Paragraph* pPara ) const;
    Paragraph*          GetParent( Paragraph* pParagraph ) const;
    sal_uLong           GetAbsPos( Paragraph* pPara );

    sal_Int16       GetDepth( sal_uLong nPara ) const;
    void            SetDepth( Paragraph* pParagraph, sal_Int16 nNewDepth );

    sal_Bool            IsVisible( Paragraph* pPara ) const { return pPara->IsVisible(); }

    void            EnableUndo( sal_Bool bEnable );
    sal_Bool            IsUndoEnabled() const;
    void            UndoActionStart( sal_uInt16 nId );
    void            UndoActionEnd( sal_uInt16 nId );
    void            InsertUndo( EditUndo* pUndo );
    sal_Bool            IsInUndo();

    void            ClearModifyFlag();
    sal_Bool            IsModified() const;

    Paragraph*      GetHdlParagraph() const { return pHdlParagraph; }
    sal_Bool            IsExpanding() const { return bIsExpanding; }

    virtual void    ExpandHdl();
    void            SetExpandHdl( const Link& rLink ) { aExpandHdl = rLink; }
    Link            GetExpandHdl() const { return aExpandHdl; }

    virtual void    ParagraphInsertedHdl();
    void            SetParaInsertedHdl(const Link& rLink){aParaInsertedHdl=rLink;}
    Link            GetParaInsertedHdl() const { return aParaInsertedHdl; }

    virtual void    ParagraphRemovingHdl();
    void            SetParaRemovingHdl(const Link& rLink){aParaRemovingHdl=rLink;}
    Link            GetParaRemovingHdl() const { return aParaRemovingHdl; }

    virtual void    DepthChangedHdl();
    void            SetDepthChangedHdl(const Link& rLink){aDepthChangedHdl=rLink;}
    Link            GetDepthChangedHdl() const { return aDepthChangedHdl; }
    sal_Int16       GetPrevDepth() const { return nDepthChangedHdlPrevDepth; }
    sal_uInt16          GetPrevFlags() const { return mnDepthChangeHdlPrevFlags; }

    virtual long    RemovingPagesHdl( OutlinerView* );
    void            SetRemovingPagesHdl(const Link& rLink){aRemovingPagesHdl=rLink;}
    Link            GetRemovingPagesHdl() const { return aRemovingPagesHdl; }
    virtual long    IndentingPagesHdl( OutlinerView* );
    void            SetIndentingPagesHdl(const Link& rLink){aIndentingPagesHdl=rLink;}
    Link            GetIndentingPagesHdl() const { return aIndentingPagesHdl; }
    // valid only in the two upper handlers
    sal_uInt16          GetSelPageCount() const { return nDepthChangedHdlPrevDepth; }

    // valid only in the two upper handlers
    sal_uLong           GetFirstSelPage() const { return mnFirstSelPage; }

    void            SetCalcFieldValueHdl(const Link& rLink ) { aCalcFieldValueHdl= rLink; }
    Link            GetCalcFieldValueHdl() const { return aCalcFieldValueHdl; }

    void            SetFieldClickedHdl(const Link& rLink ) { aFieldClickedHdl= rLink; }
    Link            GetFieldClickedHdl() const { return aFieldClickedHdl; }

    void            SetDrawPortionHdl(const Link& rLink){aDrawPortionHdl=rLink;}
    Link            GetDrawPortionHdl() const { return aDrawPortionHdl; }

    void            SetDrawBulletHdl(const Link& rLink){aDrawBulletHdl=rLink;}
    Link            GetDrawBulletHdl() const { return aDrawBulletHdl; }

    void            SetPaintFirstLineHdl(const Link& rLink) { maPaintFirstLineHdl = rLink; }
    Link            GetPaintFirstLineHdl() const { return maPaintFirstLineHdl; }

    void            SetModifyHdl( const Link& rLink );
    Link            GetModifyHdl() const;

    void            SetNotifyHdl( const Link& rLink );

    void            SetStatusEventHdl( const Link& rLink );
    Link            GetStatusEventHdl() const;

    void            Draw( OutputDevice* pOutDev, const Rectangle& rOutRect );
    void            Draw( OutputDevice* pOutDev, const Point& rStartPos, short nOrientation = 0 );

    const Size&     GetPaperSize() const;
    void            SetPaperSize( const Size& rSize );

    void            SetFirstPageNumber( sal_uInt16 n )  { nFirstPage = n; }
    sal_uInt16          GetFirstPageNumber() const      { return nFirstPage; }

    void                SetPolygon( const basegfx::B2DPolyPolygon& rPolyPolygon );
    void                SetPolygon( const basegfx::B2DPolyPolygon& rPolyPolygon, const basegfx::B2DPolyPolygon* pLinePolyPolygon);
    void                ClearPolygon();

    const Size&     GetMinAutoPaperSize() const;
    void            SetMinAutoPaperSize( const Size& rSz );

    const Size&     GetMaxAutoPaperSize() const;
    void            SetMaxAutoPaperSize( const Size& rSz );

    void            SetDefTab( sal_uInt16 nTab );

    sal_Bool            IsFlatMode() const;
    void            SetFlatMode( sal_Bool bFlat );

    void            EnableAutoColor( sal_Bool b );

    void            ForceAutoColor( sal_Bool b );
    sal_Bool            IsForceAutoColor() const;

    EBulletInfo     GetBulletInfo( sal_uInt16 nPara );

    void        SetWordDelimiters( const String& rDelimiters );
    String      GetWordDelimiters() const;
    String      GetWord( sal_uInt16 nPara, xub_StrLen nIndex );

    void            StripPortions();

    virtual void DrawingText(
        const Point& rStartPos, const String& rText, sal_uInt16 nTextStart, sal_uInt16 nTextLen,
        const sal_Int32* pDXArray, const SvxFont& rFont, sal_uInt16 nPara, xub_StrLen nIndex, sal_uInt8 nRightToLeft,
        const EEngineData::WrongSpellVector* pWrongSpellVector,
        const SvxFieldData* pFieldData,
        bool bEndOfLine,
        bool bEndOfParagraph,
        bool bEndOfBullet,
        const ::com::sun::star::lang::Locale* pLocale,
        const Color& rOverlineColor,
        const Color& rTextLineColor);

    virtual void DrawingTab(
        const Point& rStartPos, long nWidth, const String& rChar,
        const SvxFont& rFont, sal_uInt16 nPara, xub_StrLen nIndex, sal_uInt8 nRightToLeft,
        bool bEndOfLine,
        bool bEndOfParagraph,
        const Color& rOverlineColor,
        const Color& rTextLineColor);

    Size            CalcTextSize();
    Size            CalcTextSizeNTP();

    void                SetStyleSheetPool( SfxStyleSheetPool* pSPool );
    SfxStyleSheetPool*  GetStyleSheetPool();

    sal_Bool            IsInSelectionMode() const;

    void            SetStyleSheet( sal_uLong nPara, SfxStyleSheet* pStyle );
    SfxStyleSheet*  GetStyleSheet( sal_uLong nPara );

    void            SetParaAttribs( sal_uInt16 nPara, const SfxItemSet& );
    SfxItemSet      GetParaAttribs( sal_uInt16 nPara );

    void            Remove( Paragraph* pPara, sal_uLong nParaCount );
    sal_Bool            Expand( Paragraph* );
    sal_Bool            Collapse( Paragraph* );

    void            SetParaFlag( Paragraph* pPara,  sal_uInt16 nFlag );
    bool            HasParaFlag( const Paragraph* pPara, sal_uInt16 nFlag ) const;

    // Returns an array containing the widths of the Bullet Indentations
    // Last value must be -1. Is deleted by the outliner.
    Link            GetWidthArrReqHdl() const{ return aWidthArrReqHdl; }
    void            SetWidthArrReqHdl(const Link& rLink){aWidthArrReqHdl=rLink; }

    void            SetControlWord( sal_uLong nWord );
    sal_uLong           GetControlWord() const;

    Link            GetBeginMovingHdl() const { return aBeginMovingHdl; }
    void            SetBeginMovingHdl(const Link& rLink) {aBeginMovingHdl=rLink;}
    Link            GetEndMovingHdl() const {return aEndMovingHdl;}
    void            SetEndMovingHdl( const Link& rLink){aEndMovingHdl=rLink;}

    sal_uLong           GetLineCount( sal_uLong nParagraph ) const;
    sal_uInt16          GetLineLen( sal_uLong nParagraph, sal_uInt16 nLine ) const;
    sal_uLong           GetLineHeight( sal_uLong nParagraph, sal_uLong nLine = 0 );

    // nFormat must be a value from the enum EETextFormat (due to CLOOKS)
    sal_uLong           Read( SvStream& rInput, const String& rBaseURL, sal_uInt16, SvKeyValueIterator* pHTTPHeaderAttrs = NULL );

    ::svl::IUndoManager&
                    GetUndoManager();

    void            QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel );
    void            QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel );
    void            QuickInsertLineBreak( const ESelection& rSel );

    // Only for EditEngine mode
    void            QuickInsertText( const String& rText, const ESelection& rSel );
    void            QuickDelete( const ESelection& rSel );
    void            QuickRemoveCharAttribs( sal_uInt16 nPara, sal_uInt16 nWhich = 0 );
    void            QuickFormatDoc( sal_Bool bFull = sal_False );

    sal_Bool            UpdateFields();
    void            RemoveFields( sal_Bool bKeepFieldText, TypeId aType = NULL );

    virtual void    FieldClicked( const SvxFieldItem& rField, sal_uInt16 nPara, xub_StrLen nPos );
    virtual void    FieldSelected( const SvxFieldItem& rField, sal_uInt16 nPara, xub_StrLen nPos );
    virtual String  CalcFieldValue( const SvxFieldItem& rField, sal_uInt16 nPara, xub_StrLen nPos, Color*& rTxtColor, Color*& rFldColor );

    void            SetSpeller( ::com::sun::star::uno::Reference<
                            ::com::sun::star::linguistic2::XSpellChecker1 > &xSpeller );
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellChecker1 >
                    GetSpeller();
    void            SetHyphenator( ::com::sun::star::uno::Reference<
                        ::com::sun::star::linguistic2::XHyphenator >& xHyph );

    void            SetForbiddenCharsTable( rtl::Reference<SvxForbiddenCharactersTable> xForbiddenChars );

    // Depricated
    void            SetDefaultLanguage( LanguageType eLang );
    LanguageType    GetDefaultLanguage() const;

    void            CompleteOnlineSpelling();

    EESpellState    HasSpellErrors();
    sal_Bool            HasText( const SvxSearchItem& rSearchItem );
    virtual sal_Bool    SpellNextDocument();

    // for text conversion
    sal_Bool        HasConvertibleTextPortion( LanguageType nLang );
    virtual sal_Bool    ConvertNextDocument();

    void            SetEditTextObjectPool( SfxItemPool* pPool );
    SfxItemPool*    GetEditTextObjectPool() const;

    void            SetRefDevice( OutputDevice* pRefDev );
    OutputDevice*   GetRefDevice() const;

    sal_uLong           GetTextHeight() const;
    sal_uLong           GetTextHeight( sal_uLong nParagraph ) const;
    Point           GetDocPosTopLeft( sal_uLong nParagraph );
    Point           GetDocPos( const Point& rPaperPos ) const;
    sal_Bool            IsTextPos( const Point& rPaperPos, sal_uInt16 nBorder = 0 );
    sal_Bool            IsTextPos( const Point& rPaperPos, sal_uInt16 nBorder, sal_Bool* pbBuuletPos );

    void            SetGlobalCharStretching( sal_uInt16 nX = 100, sal_uInt16 nY = 100 );
    void            GetGlobalCharStretching( sal_uInt16& rX, sal_uInt16& rY );
    void            EraseVirtualDevice();

    sal_Bool            ShouldCreateBigTextObject() const;

    const EditEngine& GetEditEngine() const { return *((EditEngine*)pEditEngine); }

    // this is needed for StarOffice Api
    void SetLevelDependendStyleSheet( sal_uInt16 nPara );

    sal_uInt16  GetOutlinerMode() const { return nOutlinerMode & OUTLINERMODE_USERMASK; }

    void            StartSpelling(EditView& rEditView, sal_Bool bMultipleDoc);
    // spell and return a sentence
    bool            SpellSentence(EditView& rEditView, ::svx::SpellPortions& rToFill, bool bIsGrammarChecking );
    // put spell position to start of current sentence
    void            PutSpellingToSentenceStart( EditView& rEditView );
    // applies a changed sentence
    void            ApplyChangedSentence(EditView& rEditView, const ::svx::SpellPortions& rNewPortions, bool bRecheck );

    /** sets a link that is called at the beginning of a drag operation at an edit view */
    void            SetBeginDropHdl( const Link& rLink );

    /** sets a link that is called at the end of a drag operation at an edit view */
    void            SetEndDropHdl( const Link& rLink );

    /** sets a link that is called before a drop or paste operation. */
    void            SetBeginPasteOrDropHdl( const Link& rLink );
    Link            GetBeginPasteOrDropHdl() const { return maBeginPasteOrDropHdl; }

    /** sets a link that is called after a drop or paste operation. */
    void            SetEndPasteOrDropHdl( const Link& rLink );
    Link            GetEndPasteOrDropHdl() const { return maEndPasteOrDropHdl; }

    virtual sal_Int16 GetNumberingStartValue( sal_uInt16 nPara );
    virtual void SetNumberingStartValue( sal_uInt16 nPara, sal_Int16 nNumberingStartValue );

    virtual sal_Bool IsParaIsNumberingRestart( sal_uInt16 nPara );
    virtual void SetParaIsNumberingRestart( sal_uInt16 nPara, sal_Bool bParaIsNumberingRestart );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
