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
#include <tools/debug.hxx>

#include <svl/brdcst.hxx>

#include <editeng/editdata.hxx>
#include <i18npool/lang.h>
#include <tools/color.hxx>
#include <vcl/graph.hxx>
#include <tools/link.hxx>
#include <rsc/rscsfx.hxx>
#include "editeng/editengdllapi.h"

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
class OLUndoRemove;
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
class SfxUndoManager;
class EditEngine;
class SvKeyValueIterator;
class SvxForbiddenCharactersTable;
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
    friend class OLUndoDepth;
    friend class OutlinerUndoCheckPara;
    friend class OutlinerUndoChangeParaFlags;

    Paragraph& operator=(const Paragraph& rPara );

    USHORT              nFlags;
    XubString           aBulText;
    Size                aBulSize;
    BOOL                bVisible;

    BOOL                IsVisible() const { return bVisible; }
    void                SetText( const XubString& rText ) { aBulText = rText; aBulSize.Width() = -1; }
    void                Invalidate() { aBulSize.Width() = -1; }
    void                SetDepth( sal_Int16 nNewDepth ) { nDepth = nNewDepth; aBulSize.Width() = -1; }
    const XubString&    GetText() const { return aBulText; }

                        Paragraph( sal_Int16 nDepth );
                        Paragraph( const Paragraph& );
                        Paragraph( const ParagraphData& );
                        ~Paragraph();

    sal_Int16           GetDepth() const { return nDepth; }

    sal_Int16           GetNumberingStartValue() const { return mnNumberingStartValue; }
    void                SetNumberingStartValue( sal_Int16 nNumberingStartValue );

    sal_Bool            IsParaIsNumberingRestart() const { return mbParaIsNumberingRestart; }
    void                SetParaIsNumberingRestart( sal_Bool bParaIsNumberingRestart );

    void                SetFlag( USHORT nFlag ) { nFlags |= nFlag; }
    void                RemoveFlag( USHORT nFlag ) { nFlags &= ~nFlag; }
    bool                HasFlag( USHORT nFlag ) const { return (nFlags & nFlag) != 0; }
};

struct ParaRange
{
    USHORT  nStartPara;
    USHORT  nEndPara;

            ParaRange( USHORT nS, USHORT nE ) { nStartPara = nS, nEndPara = nE; }

    void    Adjust();
    USHORT  Len() const { return 1 + ( ( nEndPara > nStartPara ) ? (nEndPara-nStartPara) : (nStartPara-nEndPara) ); }
};

inline void ParaRange::Adjust()
{
    if ( nStartPara > nEndPara )
    {
        USHORT nTmp = nStartPara;
        nStartPara = nEndPara;
        nEndPara = nTmp;
    }
}

#define OL_ROOTPARENT (Paragraph*)NULL

class EDITENG_DLLPUBLIC OutlinerView
{
    friend class Outliner;

private:

    Outliner*   pOwner;
    EditView*   pEditView;

    // Drag & Drop
    BOOL        bBeginDragAtMove_OLDMEMBER;
    BOOL        bInDragMode;
    Point       aDDStartPosRef;
    Point       aDDStartPosPix;
    ULONG       nDDStartPara;
    ULONG       nDDStartParaVisChildCount;
    ULONG       nDDCurPara;
    USHORT      nDDStartDepth;
    USHORT      nDDCurDepth;
    USHORT      nDDMaxDepth;
    BOOL        bDDChangingDepth;
    BOOL        bDDCursorVisible;
    long*       pHorTabArrDoc;
    long        nDDScrollLRBorderWidthWin;  // Left Right
    long        nDDScrollTBBorderWidthWin;  // Top Bottom
    long        nDDScrollLROffs;
    long        nDDScrollTDOffs;

    void*       pDummy;
    ULONG       nDummy;

    enum MouseTarget {
        MouseText = 0,
        MouseBullet = 1,
        MouseHypertext = 2,  // Outside OutputArea
        MouseOutside = 3,    // Outside OutputArea
        MouseDontKnow = 4
    };
    MouseTarget OLD_ePrevMouseTarget;

#ifdef _OUTLINER_CXX

    EDITENG_DLLPRIVATE void         ImplExpandOrCollaps( USHORT nStartPara, USHORT nEndPara, BOOL bExpand );

    EDITENG_DLLPRIVATE ULONG       ImpCheckMousePos( const Point& rPosPixel, MouseTarget& reTarget);
    EDITENG_DLLPRIVATE void        ImpToggleExpand( Paragraph* pParentPara );
    EDITENG_DLLPRIVATE ParaRange    ImpGetSelectedParagraphs( BOOL bIncludeHiddenChilds );
    EDITENG_DLLPRIVATE void        ImpHideDDCursor();
    EDITENG_DLLPRIVATE void        ImpShowDDCursor();
    EDITENG_DLLPRIVATE void        ImpPaintDDCursor();

    EDITENG_DLLPRIVATE void        ImpDragScroll( const Point& rPosPix );
    EDITENG_DLLPRIVATE void        ImpScrollLeft();
    EDITENG_DLLPRIVATE void        ImpScrollRight();
    EDITENG_DLLPRIVATE void        ImpScrollUp();
    EDITENG_DLLPRIVATE void        ImpScrollDown();

    EDITENG_DLLPRIVATE ULONG       ImpGetInsertionPara( const Point& rPosPixel );
    EDITENG_DLLPRIVATE Point       ImpGetDocPos( const Point& rPosPixel );
    EDITENG_DLLPRIVATE Pointer     ImpGetMousePointer( MouseTarget eTarget );
    EDITENG_DLLPRIVATE USHORT      ImpInitPaste( ULONG& rStart );
    EDITENG_DLLPRIVATE void        ImpPasted( ULONG nStart, ULONG nPrevParaCount, USHORT nSize);
    EDITENG_DLLPRIVATE USHORT      ImpCalcSelectedPages( BOOL bIncludeFirstSelected );
    EDITENG_DLLPRIVATE BOOL        ImpIsIndentingPages();

#endif

public:
                OutlinerView( Outliner* pOut, Window* pWindow );
    virtual     ~OutlinerView();

    EditView&   GetEditView() const { return *pEditView; }

    void        Scroll( long nHorzScroll, long nVertScroll );

    void        Paint( const Rectangle& rRect );
    BOOL        PostKeyEvent( const KeyEvent& rKEvt, Window* pFrameWin = NULL );
    BOOL        MouseButtonDown( const MouseEvent& );
    BOOL        MouseButtonUp( const MouseEvent& );
    BOOL        MouseMove( const MouseEvent& );

    void        ShowCursor( BOOL bGotoCursor = TRUE );
    void        HideCursor();

    void        SetOutliner( Outliner* pOutliner );
    Outliner*   GetOutliner() const { return pOwner; }

    void        SetWindow( Window* pWindow );
    Window*     GetWindow() const;

    void        SetReadOnly( BOOL bReadOnly );
    BOOL        IsReadOnly() const;

    void        SetOutputArea( const Rectangle& rRect );
    Rectangle   GetOutputArea() const;

    Rectangle   GetVisArea() const;

    void        CreateSelectionList (std::vector<Paragraph*> &aSelList) ;

    // Retruns the number of selected paragraphs
    ULONG       Select( Paragraph* pParagraph,
                    BOOL bSelect=TRUE,
                    BOOL bWChilds=TRUE);

    String      GetSelected() const;
    void        SelectRange( ULONG nFirst, USHORT nCount );
    void        SetAttribs( const SfxItemSet& );
    void        Indent( short nDiff );
    void        AdjustDepth( short nDX );   // Later replace with Indent!

    BOOL        AdjustHeight( long nDY );
    void        AdjustDepth( Paragraph* pPara, short nDX,
                    BOOL bWithChilds = FALSE );
    void        AdjustHeight( Paragraph* pPara, long nDY,
                    BOOL bWithChilds=FALSE );

    ULONG       Read( SvStream& rInput, const String& rBaseURL, EETextFormat eFormat, BOOL bSelect = FALSE, SvKeyValueIterator* pHTTPHeaderAttrs = NULL );
    ULONG       Write( SvStream& rOutput, EETextFormat eFormat );

    void        InsertText( const String& rNew, BOOL bSelect = FALSE );
    void        InsertText( const OutlinerParaObject& rParaObj );
    void        Expand();
    void        Collapse();
    void        Expand( Paragraph* );
    void        Collapse( Paragraph* );
    void        ExpandAll();
    void        CollapseAll();

    void        SetBackgroundColor( const Color& rColor );
    Color       GetBackgroundColor();

    SfxItemSet  GetAttribs();

    void        Cut();
    void        Copy();
    void        Paste();
    void        PasteSpecial();
    void        EnablePaste( BOOL bEnable );

    void        Undo();
    void        Redo();

    void            SetStyleSheet( SfxStyleSheet* );
    SfxStyleSheet*  GetStyleSheet() const;

    void        SetControlWord( ULONG nWord );
    ULONG       GetControlWord() const;

    void            SetAnchorMode( EVAnchorMode eMode );
    EVAnchorMode    GetAnchorMode() const;

    Pointer     GetPointer( const Point& rPosPixel );
    void        Command( const CommandEvent& rCEvt );
    void        RemoveCharAttribs( ULONG nPara, USHORT nWhich = 0 );

    void        CompleteAutoCorrect();

    EESpellState    StartSpeller( BOOL bMultipleDoc = FALSE );
    EESpellState    StartThesaurus();
    USHORT          StartSearchAndReplace( const SvxSearchItem& rSearchItem );

    // for text conversion
    void            StartTextConversion( LanguageType nSrcLang, LanguageType nDestLang, const Font *pDestFont, INT32 nOptions, BOOL bIsInteractive, BOOL bMultipleDoc );

    void            TransliterateText( sal_Int32 nTransliterationMode );

    ESelection  GetSelection();

    USHORT      GetSelectedScriptType() const;

    void        SetVisArea( const Rectangle& rRec );
    void        SetSelection( const ESelection& );

    void        RemoveAttribs( BOOL bRemoveParaAttribs = FALSE, USHORT nWhich = 0, BOOL bKeepLanguages = FALSE );
    void        RemoveAttribsKeepLanguages( BOOL bRemoveParaAttribs );
    BOOL        HasSelection() const;

    void                InsertField( const SvxFieldItem& rFld );
    const SvxFieldItem* GetFieldUnderMousePointer() const;
    const SvxFieldItem* GetFieldUnderMousePointer( USHORT& nPara, xub_StrLen& nPos ) const;
    const SvxFieldItem* GetFieldAtSelection() const;

    /** enables numbering for the selected paragraphs if the numbering of the first paragraph is off
        or disables numbering for the selected paragraphs if the numbering of the first paragraph is on
    */
    void        ToggleBullets();

    /** enables numbering for the selected paragraphs that are not enabled and ignore all selected
        paragraphs that already have numbering enabled.
    */
    void        EnableBullets();

    BOOL        IsCursorAtWrongSpelledWord( BOOL bMarkIfWrong = FALSE );
    BOOL        IsWrongSpelledWordAtPos( const Point& rPosPixel, BOOL bMarkIfWrong = FALSE );
    void        SpellIgnoreWord();
    void        ExecuteSpellPopup( const Point& rPosPixel, Link* pCallBack = 0 );

    void        SetInvalidateMore( USHORT nPixel );
    USHORT      GetInvalidateMore() const;

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

    BYTE                mnBiDiLevel;

    bool                mbFilled;
    long                mnWidthToFill;

    // bitfield
    unsigned            mbEndOfLine : 1;
    unsigned            mbEndOfParagraph : 1;
    unsigned            mbEndOfBullet : 1;

    BYTE GetBiDiLevel() const { return mnBiDiLevel; }
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
        BYTE nBiDiLevel,
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
    USHORT mnPara;
    const Point& mrStartPos;
    long mnBaseLineY;
    const Point& mrOrigin;
    short mnOrientation;
    OutputDevice* mpOutDev;

    PaintFirstLineInfo( USHORT nPara, const Point& rStartPos, long nBaseLineY, const Point& rOrigin, short nOrientation, OutputDevice* pOutDev )
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

    USHORT              nPara;
    xub_StrLen          nPos;
    BOOL                bSimpleClick;

                        EditFieldInfo();
                        EditFieldInfo( const EditFieldInfo& );

    SdrPage*            mpSdrPage;

public:
                    EditFieldInfo( Outliner* pOutl, const SvxFieldItem& rFItem, USHORT nPa, xub_StrLen nPo )
                        : rFldItem( rFItem )
                    {
                        pOutliner = pOutl;
                        nPara = nPa; nPos = nPo;
                        pTxtColor = 0; pFldColor = 0; bSimpleClick = FALSE;
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

    USHORT          GetPara() const { return nPara; }
    xub_StrLen          GetPos() const { return nPos; }

    BOOL            IsSimpleClick() const { return bSimpleClick; }
    void            SetSimpleClick( BOOL bSimple ) { bSimpleClick = bSimple; }

    const String&       GetRepresentation() const                { return aRepresentation; }
    String&             GetRepresentation()                      { return aRepresentation; }
    void                SetRepresentation( const String& rStr )  { aRepresentation = rStr; }

    void            SetSdrPage( SdrPage* pPage ) { mpSdrPage = pPage; }
    SdrPage*        GetSdrPage() const { return mpSdrPage; }
};

struct EBulletInfo
{
    BOOL        bVisible;
    USHORT      nType;          // see SvxNumberType
    String      aText;
    SvxFont     aFont;
    Graphic     aGraphic;
    USHORT      nParagraph;
    Rectangle   aBounds;

    EBulletInfo() : bVisible( FALSE ), nType( 0 ), nParagraph( EE_PARA_NOT_FOUND ) {}
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
    ULONG               mnFirstSelPage;
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
    USHORT              mnDepthChangeHdlPrevFlags;
    sal_Int16           nMaxDepth;
    const sal_Int16     nMinDepth;
    USHORT              nFirstPage;

    USHORT              nOutlinerMode;

    BOOL                bIsExpanding; // Only valid in Expand/Collaps-Hdl, reset
    BOOL                bFirstParaIsEmpty;
    BOOL                bBlockInsCallback;
    BOOL                bStrippingPortions;
    BOOL                bPasting;

    ULONG               nDummy;

#ifdef _OUTLINER_CXX

    DECL_LINK(              ParaVisibleStateChangedHdl, Paragraph* );
    DECL_LINK(              BeginMovingParagraphsHdl, MoveParagraphsInfo* );
    DECL_LINK(              EndMovingParagraphsHdl, MoveParagraphsInfo* );
    DECL_LINK(              BeginPasteOrDropHdl, PasteOrDropInfos* );
    DECL_LINK(              EndPasteOrDropHdl, PasteOrDropInfos* );
    DECL_LINK(              EditEngineNotifyHdl, EENotify* );
    void                    ImplCheckParagraphs( USHORT nStart, USHORT nEnd );
    BOOL                    ImplHasBullet( USHORT nPara ) const;
    Size                    ImplGetBulletSize( USHORT nPara );
    sal_uInt16              ImplGetNumbering( USHORT nPara, const SvxNumberFormat* pParaFmt );
    void                    ImplCalcBulletText( USHORT nPara, BOOL bRecalcLevel, BOOL bRecalcChilds );
    String                  ImplGetBulletText( USHORT nPara );
    void                    ImplCheckNumBulletItem( USHORT nPara );
    void                    ImplInitDepth( USHORT nPara, sal_Int16 nDepth, BOOL bCreateUndo, BOOL bUndoAction = FALSE );
    void                    ImplSetLevelDependendStyleSheet( USHORT nPara, SfxStyleSheet* pLevelStyle = NULL );

    void                    ImplBlockInsertionCallbacks( BOOL b );

    void                ImplCheckStyleSheet( USHORT nPara, BOOL bReplaceExistingStyle );
    void                ImpRecalcBulletIndent( ULONG nPara );

    const SvxBulletItem& ImpGetBullet( ULONG nPara, USHORT& );
    void        ImpFilterIndents( ULONG nFirstPara, ULONG nLastPara );
    bool        ImpConvertEdtToOut( sal_uInt32 nPara, EditView* pView = 0 );

    void        ImpTextPasted( ULONG nStartPara, USHORT nCount );
    long        ImpCalcMaxBulletWidth( USHORT nPara, const SvxBulletItem& rBullet );
    Font        ImpCalcBulletFont( USHORT nPara ) const;
    Rectangle   ImpCalcBulletArea( USHORT nPara, BOOL bAdjust, BOOL bReturnPaperPos );
    long        ImpGetTextIndent( ULONG nPara );
    BOOL        ImpCanIndentSelectedPages( OutlinerView* pCurView );
    BOOL        ImpCanDeleteSelectedPages( OutlinerView* pCurView );
    BOOL        ImpCanDeleteSelectedPages( OutlinerView* pCurView, USHORT nFirstPage, USHORT nPages );

    USHORT      ImplGetOutlinerMode() const { return nOutlinerMode & OUTLINERMODE_USERMASK; }
    void        ImplCheckDepth( sal_Int16& rnDepth ) const;
#endif

protected:
    void            ParagraphInserted( USHORT nParagraph );
    void            ParagraphDeleted( USHORT nParagraph );
    void            ParaAttribsChanged( USHORT nParagraph );

    virtual void    StyleSheetChanged( SfxStyleSheet* pStyle );

    void        InvalidateBullet( Paragraph* pPara, ULONG nPara );
    void        PaintBullet( USHORT nPara, const Point& rStartPos,
                    const Point& rOrigin, short nOrientation,
                    OutputDevice* pOutDev );

    // used by OutlinerEditEng. Allows Outliner objects to provide
    // bullet access to the EditEngine.
    virtual const SvxNumberFormat*  GetNumberFormat( USHORT nPara ) const;

public:

                    Outliner( SfxItemPool* pPool, USHORT nOutlinerMode );
    virtual         ~Outliner();

    void            Init( USHORT nOutlinerMode );
    USHORT          GetMode() const { return nOutlinerMode; }

    void            SetVertical( BOOL bVertical );
    BOOL            IsVertical() const;

    void            SetFixedCellHeight( BOOL bUseFixedCellHeight );
    BOOL            IsFixedCellHeight() const;

    void                        SetDefaultHorizontalTextDirection( EEHorizontalTextDirection eHTextDir );
    EEHorizontalTextDirection   GetDefaultHorizontalTextDirection() const;

    USHORT          GetScriptType( const ESelection& rSelection ) const;
    LanguageType    GetLanguage( USHORT nPara, USHORT nPos ) const;

    void            SetAsianCompressionMode( USHORT nCompressionMode );
    USHORT          GetAsianCompressionMode() const;

    void            SetKernAsianPunctuation( BOOL bEnabled );
    BOOL            IsKernAsianPunctuation() const;

    void            SetAddExtLeading( BOOL b );
    BOOL            IsAddExtLeading() const;

    size_t          InsertView( OutlinerView* pView, size_t nIndex = size_t(-1) );
    OutlinerView*   RemoveView( OutlinerView* pView );
    OutlinerView*   RemoveView( size_t nIndex );
    OutlinerView*   GetView( size_t nIndex ) const;
    size_t          GetViewCount() const;

    Paragraph*      Insert( const String& rText, ULONG nAbsPos = LIST_APPEND, sal_Int16 nDepth = 0 );
    void            SetText( const OutlinerParaObject& );
    void            AddText( const OutlinerParaObject& );
    void            SetText( const String& rText, Paragraph* pParagraph );
    String          GetText( Paragraph* pPara, ULONG nParaCount=1 ) const;

    OutlinerParaObject* CreateParaObject( USHORT nStartPara = 0, USHORT nParaCount = 0xFFFF ) const;

    const SfxItemSet& GetEmptyItemSet() const;

    void            SetRefMapMode( const MapMode& );
    MapMode         GetRefMapMode() const;

    void            SetBackgroundColor( const Color& rColor );
    Color           GetBackgroundColor() const;

    sal_Int16       GetMinDepth() const { return -1; }

    void            SetMaxDepth( sal_Int16 nDepth, BOOL bCheckParas = FALSE );
    sal_Int16          GetMaxDepth() const { return nMaxDepth; }

    void            SetUpdateMode( BOOL bUpdate );
    BOOL            GetUpdateMode() const;

    void            Clear();

    void            RemoveAttribs( const ESelection& rSelection, sal_Bool bRemoveParaAttribs, sal_uInt16 nWhich );

    ULONG           GetParagraphCount() const;
    Paragraph*      GetParagraph( ULONG nAbsPos ) const;

    BOOL            HasParent( Paragraph* pParagraph ) const;
    BOOL            HasChilds( Paragraph* pParagraph ) const;
    ULONG           GetChildCount( Paragraph* pParent ) const;
    BOOL            IsExpanded( Paragraph* pPara ) const;
    Paragraph*      GetParent( Paragraph* pParagraph ) const;
    ULONG           GetAbsPos( Paragraph* pPara );

    sal_Int16       GetDepth( ULONG nPara ) const;
    void            SetDepth( Paragraph* pParagraph, sal_Int16 nNewDepth );

    void            SetVisible( Paragraph* pPara, BOOL bVisible );
    BOOL            IsVisible( Paragraph* pPara ) const { return pPara->IsVisible(); }

    void            EnableUndo( BOOL bEnable );
    BOOL            IsUndoEnabled() const;
    void            UndoActionStart( USHORT nId );
    void            UndoActionEnd( USHORT nId );
    void            InsertUndo( EditUndo* pUndo );
    BOOL            IsInUndo();

    void            ClearModifyFlag();
    BOOL            IsModified() const;

    Paragraph*      GetHdlParagraph() const { return pHdlParagraph; }
    BOOL            IsExpanding() const { return bIsExpanding; }

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
    USHORT          GetPrevFlags() const { return mnDepthChangeHdlPrevFlags; }

    virtual long    RemovingPagesHdl( OutlinerView* );
    void            SetRemovingPagesHdl(const Link& rLink){aRemovingPagesHdl=rLink;}
    Link            GetRemovingPagesHdl() const { return aRemovingPagesHdl; }
    virtual long    IndentingPagesHdl( OutlinerView* );
    void            SetIndentingPagesHdl(const Link& rLink){aIndentingPagesHdl=rLink;}
    Link            GetIndentingPagesHdl() const { return aIndentingPagesHdl; }
    // valid only in the two upper handlers
    USHORT          GetSelPageCount() const { return nDepthChangedHdlPrevDepth; }

    // valid only in the two upper handlers
    ULONG           GetFirstSelPage() const { return mnFirstSelPage; }

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
    Link            GetNotifyHdl() const;

    void            SetStatusEventHdl( const Link& rLink );
    Link            GetStatusEventHdl() const;

    void            Draw( OutputDevice* pOutDev, const Rectangle& rOutRect );
    void            Draw( OutputDevice* pOutDev, const Rectangle& rOutRect, const Point& rStartDocPos );
    void            Draw( OutputDevice* pOutDev, const Point& rStartPos, short nOrientation = 0 );

    const Size&     GetPaperSize() const;
    void            SetPaperSize( const Size& rSize );

    void            SetFirstPageNumber( USHORT n )  { nFirstPage = n; }
    USHORT          GetFirstPageNumber() const      { return nFirstPage; }

    void                SetPolygon( const basegfx::B2DPolyPolygon& rPolyPolygon );
    void                SetPolygon( const basegfx::B2DPolyPolygon& rPolyPolygon, const basegfx::B2DPolyPolygon* pLinePolyPolygon);
    void                ClearPolygon();
    const PolyPolygon*  GetPolygon();

    const Size&     GetMinAutoPaperSize() const;
    void            SetMinAutoPaperSize( const Size& rSz );

    const Size&     GetMaxAutoPaperSize() const;
    void            SetMaxAutoPaperSize( const Size& rSz );

    void            SetDefTab( USHORT nTab );
    USHORT          GetDefTab() const;

    BOOL            IsFlatMode() const;
    void            SetFlatMode( BOOL bFlat );

    void            EnableAutoColor( BOOL b );
    BOOL            IsAutoColorEnabled() const;

    void            ForceAutoColor( BOOL b );
    BOOL            IsForceAutoColor() const;

    EBulletInfo     GetBulletInfo( USHORT nPara );

    void        SetWordDelimiters( const String& rDelimiters );
    String      GetWordDelimiters() const;
    String      GetWord( USHORT nPara, xub_StrLen nIndex );

    void            StripPortions();

    virtual void DrawingText(
        const Point& rStartPos, const String& rText, USHORT nTextStart, USHORT nTextLen,
        const sal_Int32* pDXArray, const SvxFont& rFont, USHORT nPara, xub_StrLen nIndex, BYTE nRightToLeft,
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
        const SvxFont& rFont, USHORT nPara, xub_StrLen nIndex, BYTE nRightToLeft,
        bool bEndOfLine,
        bool bEndOfParagraph,
        const Color& rOverlineColor,
        const Color& rTextLineColor);

    Size            CalcTextSize();

    Point           GetDocPos( Paragraph* pPara );

    void                SetStyleSheetPool( SfxStyleSheetPool* pSPool );
    SfxStyleSheetPool*  GetStyleSheetPool();

    BOOL            IsInSelectionMode() const;

    void            SetStyleSheet( ULONG nPara, SfxStyleSheet* pStyle );
    SfxStyleSheet*  GetStyleSheet( ULONG nPara );

    void            SetParaAttribs( USHORT nPara, const SfxItemSet& );
    SfxItemSet      GetParaAttribs( USHORT nPara );

    void            Remove( Paragraph* pPara, ULONG nParaCount );
    BOOL            Expand( Paragraph* );
    BOOL            Collapse( Paragraph* );

    void            SetParaFlag( Paragraph* pPara,  sal_uInt16 nFlag );
    void            RemoveParaFlag( Paragraph* pPara, sal_uInt16 nFlag );
    bool            HasParaFlag( const Paragraph* pPara, sal_uInt16 nFlag ) const;

    // Returns an array containing the widths of the Bullet Indentations
    // Last value must be -1. Is deleted by the outliner.
    Link            GetWidthArrReqHdl() const{ return aWidthArrReqHdl; }
    void            SetWidthArrReqHdl(const Link& rLink){aWidthArrReqHdl=rLink; }

    void            SetControlWord( ULONG nWord );
    ULONG           GetControlWord() const;

    Link            GetBeginMovingHdl() const { return aBeginMovingHdl; }
    void            SetBeginMovingHdl(const Link& rLink) {aBeginMovingHdl=rLink;}
    Link            GetEndMovingHdl() const {return aEndMovingHdl;}
    void            SetEndMovingHdl( const Link& rLink){aEndMovingHdl=rLink;}

    ULONG           GetLineCount( ULONG nParagraph ) const;
    USHORT          GetLineLen( ULONG nParagraph, USHORT nLine ) const;
    ULONG           GetLineHeight( ULONG nParagraph, ULONG nLine = 0 );

    // nFormat must be a value from the enum EETextFormat (due to CLOOKS)
    ULONG           Read( SvStream& rInput, const String& rBaseURL, USHORT, SvKeyValueIterator* pHTTPHeaderAttrs = NULL );

    SfxUndoManager& GetUndoManager();

    void            QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel );
    void            QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel );
    void            QuickInsertLineBreak( const ESelection& rSel );

    // Only for EditEngine mode
    void            QuickInsertText( const String& rText, const ESelection& rSel );
    void            QuickDelete( const ESelection& rSel );
    void            QuickRemoveCharAttribs( USHORT nPara, USHORT nWhich = 0 );
    void            QuickFormatDoc( BOOL bFull = FALSE );

    BOOL            UpdateFields();
    void            RemoveFields( BOOL bKeepFieldText, TypeId aType = NULL );

    virtual void    FieldClicked( const SvxFieldItem& rField, USHORT nPara, xub_StrLen nPos );
    virtual void    FieldSelected( const SvxFieldItem& rField, USHORT nPara, xub_StrLen nPos );
    virtual String  CalcFieldValue( const SvxFieldItem& rField, USHORT nPara, xub_StrLen nPos, Color*& rTxtColor, Color*& rFldColor );

    void            SetSpeller( ::com::sun::star::uno::Reference<
                            ::com::sun::star::linguistic2::XSpellChecker1 > &xSpeller );
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellChecker1 >
                    GetSpeller();
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenator >
                    GetHyphenator() const;
    void            SetHyphenator( ::com::sun::star::uno::Reference<
                        ::com::sun::star::linguistic2::XHyphenator >& xHyph );

    void            SetForbiddenCharsTable( rtl::Reference<SvxForbiddenCharactersTable> xForbiddenChars );
    rtl::Reference<SvxForbiddenCharactersTable> GetForbiddenCharsTable() const;

    // Depricated
    void            SetDefaultLanguage( LanguageType eLang );
    LanguageType    GetDefaultLanguage() const;

    BOOL            HasOnlineSpellErrors() const;
    void            CompleteOnlineSpelling();

    EESpellState    HasSpellErrors();
    BOOL            HasText( const SvxSearchItem& rSearchItem );
    virtual BOOL    SpellNextDocument();

    // for text conversion
    sal_Bool        HasConvertibleTextPortion( LanguageType nLang );
    virtual BOOL    ConvertNextDocument();

    void            SetEditTextObjectPool( SfxItemPool* pPool );
    SfxItemPool*    GetEditTextObjectPool() const;

    void            SetRefDevice( OutputDevice* pRefDev );
    OutputDevice*   GetRefDevice() const;

    USHORT          GetFirstLineOffset( ULONG nParagraph );

    ULONG           GetTextHeight() const;
    ULONG           GetTextHeight( ULONG nParagraph ) const;
    Point           GetDocPosTopLeft( ULONG nParagraph );
    Point           GetDocPos( const Point& rPaperPos ) const;
    BOOL            IsTextPos( const Point& rPaperPos, USHORT nBorder = 0 );
    BOOL            IsTextPos( const Point& rPaperPos, USHORT nBorder, BOOL* pbBuuletPos );

    void            SetGlobalCharStretching( USHORT nX = 100, USHORT nY = 100 );
    void            GetGlobalCharStretching( USHORT& rX, USHORT& rY );
    void            DoStretchChars( USHORT nX, USHORT nY );
    void            EraseVirtualDevice();

    void            SetBigTextObjectStart( USHORT nStartAtPortionCount );
    USHORT          GetBigTextObjectStart() const;
    BOOL            ShouldCreateBigTextObject() const;

    const EditEngine& GetEditEngine() const { return *((EditEngine*)pEditEngine); }

    // this is needed for StarOffice Api
    void SetLevelDependendStyleSheet( USHORT nPara );

    USHORT  GetOutlinerMode() const { return nOutlinerMode & OUTLINERMODE_USERMASK; }

    void            StartSpelling(EditView& rEditView, sal_Bool bMultipleDoc);
    // spell and return a sentence
    bool            SpellSentence(EditView& rEditView, ::svx::SpellPortions& rToFill, bool bIsGrammarChecking );
    // put spell position to start of current sentence
    void            PutSpellingToSentenceStart( EditView& rEditView );
    // applies a changed sentence
    void            ApplyChangedSentence(EditView& rEditView, const ::svx::SpellPortions& rNewPortions, bool bRecheck );
    void            EndSpelling();

    /** sets a link that is called at the beginning of a drag operation at an edit view */
    void            SetBeginDropHdl( const Link& rLink );
    Link            GetBeginDropHdl() const;

    /** sets a link that is called at the end of a drag operation at an edit view */
    void            SetEndDropHdl( const Link& rLink );
    Link            GetEndDropHdl() const;

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
