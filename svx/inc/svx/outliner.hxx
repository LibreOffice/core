/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: outliner.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 16:36:17 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _OUTLINER_HXX
#define _OUTLINER_HXX

#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _EDITDATA_HXX //autogen
#include <svx/editdata.hxx>
#endif
#ifndef INCLUDED_I18NPOOL_LANG_H
#include <i18npool/lang.h>
#endif
#ifndef _SV_COLOR_HXX //autogen
#include <tools/color.hxx>
#endif
#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#ifndef _LINK_HXX //autogen
#include <tools/link.hxx>
#endif
#ifndef _RSCSFX_HXX //autogen
#include <rsc/rscsfx.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

#include <tools/rtti.hxx>   // wegen typedef TypeId
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

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#include <vos/ref.hxx>
#include <svx/svxfont.hxx>

namespace com { namespace sun { namespace star { namespace linguistic2 {
    class XSpellChecker1;
    class XHyphenator;
}}}}
namespace svx{
struct SpellPortion;
typedef std::vector<SpellPortion> SpellPortions;
}

namespace basegfx { class B2DPolyPolygon; }

// nur interner Gebrauch!
#define PARAFLAG_DROPTARGET         0x1000
#define PARAFLAG_DROPTARGET_EMPTY   0x2000
#define PARAFLAG_HOLDDEPTH          0x4000
#define PARAFLAG_SETBULLETTEXT      0x8000

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

// MT 07/00: Only for internal use, oder some kind like hPara for the few
// functions where you need it outside ( eg. moving paragraphs... )

class Paragraph
{
private:
    friend class Outliner;
    friend class ParagraphList;
    friend class OutlinerView;
    friend class OutlinerParaObject;
    friend class OutlinerEditEng;
    friend class OLUndoDepth;
    friend class OutlinerUndoCheckPara;

    Paragraph& operator=(const Paragraph& rPara );

    USHORT              nFlags;
    USHORT              nDepth;
    XubString           aBulText;
    Size                aBulSize;
    BOOL                bVisible;

    BOOL                IsVisible() const { return bVisible; }
    void                SetText( const XubString& rText ) { aBulText = rText; aBulSize.Width() = -1; }
    void                Invalidate() { aBulSize.Width() = -1; }
    void                SetDepth( USHORT nNewDepth ) { nDepth = nNewDepth; aBulSize.Width() = -1; }
    const XubString&    GetText() const { return aBulText; }

                        Paragraph( USHORT nDepth );
                        Paragraph( const Paragraph& );
                        ~Paragraph();

    USHORT              GetDepth() const { return nDepth; }
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

class SVX_DLLPUBLIC OutlinerView
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
        MouseHypertext = 2,    //            ausserhalb OutputArea
        MouseOutside = 3,    //            ausserhalb OutputArea
        MouseDontKnow = 4
    };
    MouseTarget OLD_ePrevMouseTarget;

#ifdef _OUTLINER_CXX

    SVX_DLLPRIVATE void         ImplExpandOrCollaps( USHORT nStartPara, USHORT nEndPara, BOOL bExpand );

    SVX_DLLPRIVATE ULONG       ImpCheckMousePos( const Point& rPosPixel, MouseTarget& reTarget);
    SVX_DLLPRIVATE void        ImpToggleExpand( Paragraph* pParentPara );
    SVX_DLLPRIVATE ParaRange    ImpGetSelectedParagraphs( BOOL bIncludeHiddenChilds );
    SVX_DLLPRIVATE void        ImpHideDDCursor();
    SVX_DLLPRIVATE void        ImpShowDDCursor();
    SVX_DLLPRIVATE void        ImpPaintDDCursor();

    SVX_DLLPRIVATE void        ImpDragScroll( const Point& rPosPix );
    SVX_DLLPRIVATE void        ImpScrollLeft();
    SVX_DLLPRIVATE void        ImpScrollRight();
    SVX_DLLPRIVATE void        ImpScrollUp();
    SVX_DLLPRIVATE void        ImpScrollDown();

    SVX_DLLPRIVATE ULONG       ImpGetInsertionPara( const Point& rPosPixel );
    SVX_DLLPRIVATE Point       ImpGetDocPos( const Point& rPosPixel );
    SVX_DLLPRIVATE Pointer     ImpGetMousePointer( MouseTarget eTarget );
    SVX_DLLPRIVATE USHORT      ImpInitPaste( ULONG& rStart );
    SVX_DLLPRIVATE void        ImpPasted( ULONG nStart, ULONG nPrevParaCount, USHORT nSize);
    SVX_DLLPRIVATE USHORT      ImpCalcSelectedPages( BOOL bIncludeFirstSelected );
    SVX_DLLPRIVATE BOOL        ImpIsIndentingPages();

#endif

public:
                OutlinerView( Outliner* pOut, Window* pWindow );
    virtual     ~OutlinerView();

    EditView&   GetEditView() const { return *pEditView; }

    void        Scroll( long nHorzScroll, long nVertScroll );

    void        Paint( const Rectangle& rRect );
    BOOL        PostKeyEvent( const KeyEvent& rKEvt );
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

    List*       CreateSelectionList();

    // gibt Anzahl selektierter Absaetze zurueck
    // MT 07/00: Who needs this?
    ULONG       Select( Paragraph* pParagraph,
                    BOOL bSelect=TRUE,
                    BOOL bWChilds=TRUE);

    String      GetSelected() const;
    void        SelectRange( ULONG nFirst, USHORT nCount );
    void        SetAttribs( const SfxItemSet& );
    void        Indent( short nDiff );
    void        AdjustDepth( short nDX );   // Spaeter gegeb Indent ersetzen!

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

    void        ShowBullets( BOOL bShow, BOOL bAffectLevel0 );

    BOOL        IsCursorAtWrongSpelledWord( BOOL bMarkIfWrong = FALSE );
    BOOL        IsWrongSpelledWordAtPos( const Point& rPosPixel, BOOL bMarkIfWrong = FALSE );
    void        SpellIgnoreWord();
    void        ExecuteSpellPopup( const Point& rPosPixel, Link* pCallBack = 0 );

    void        SetInvalidateMore( USHORT nPixel );
    USHORT      GetInvalidateMore() const;
};

//#if 0 // _SOLAR__PRIVATE
DECLARE_LIST(ViewList,OutlinerView*)
//#else
//typedef List ViewList;
//#endif

class SVX_DLLPUBLIC DrawPortionInfo
{
public:
    const Point&    rStartPos;

    const String&   rText;
    USHORT          nTextStart;
    USHORT          nTextLen;

    USHORT          nPara;
    xub_StrLen      nIndex;

    const SvxFont&  rFont;

    const sal_Int32*     pDXArray;

    // #101498# BiDi level needs to be transported, too.
    BYTE            mnBiDiLevel;

    BYTE GetBiDiLevel() const { return mnBiDiLevel; }
    sal_Bool IsRTL() const;

    DrawPortionInfo( const Point& rPos, const String& rTxt, USHORT nTxtStart, USHORT nTxtLen,
        const SvxFont& rFnt, USHORT nPar, xub_StrLen nIdx, const sal_Int32* pDXArr, BYTE nBiDiLevel)
        :   rStartPos(rPos), rText(rTxt), nPara(nPar), nIndex(nIdx),
            rFont(rFnt), pDXArray(pDXArr), mnBiDiLevel(nBiDiLevel)
        {
            nTextStart = nTxtStart;
            nTextLen = nTxtLen;
        }
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

class SVX_DLLPUBLIC Outliner
{
    friend class OutlinerView;
    friend class OutlinerEditEng;
    friend class OutlinerParaObject;
    friend class OLUndoExpand;
    friend class OutlinerUndoChangeDepth;
    friend class OutlinerUndoCheckPara;

    OutlinerEditEng*    pEditEngine;

    ParagraphList*      pParaList;
    ViewList            aViewList;

    Paragraph*          pHdlParagraph;
    ULONG               mnFirstSelPage;
    Link                aDrawPortionHdl;
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

    USHORT              nDepthChangedHdlPrevDepth;
    USHORT              nMaxDepth;
    USHORT              nMinDepth;
    USHORT              nFirstPage;

    USHORT              nOutlinerMode;

    BOOL                bIsExpanding; // Nur in Expand/Collaps-Hdl gueltig, mal umstellen
    BOOL                bFirstParaIsEmpty;
    BOOL                bBlockInsCallback;
    BOOL                bStrippingPortions;
    BOOL                bPasting;

    SvxNumberFormat*    pOverwriteLevel0Bullet;

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
    const SvxNumberFormat*  ImplGetBullet( USHORT nPara ) const;
    Size                    ImplGetBulletSize( USHORT nPara );
    void                    ImplCalcBulletText( USHORT nPara, BOOL bRecalcLevel, BOOL bRecalcChilds );
    String                  ImplGetBulletText( USHORT nPara );
    void                    ImplCheckNumBulletItem( USHORT nPara );
    void                    ImplInitDepth( USHORT nPara, USHORT nDepth, BOOL bCreateUndo, BOOL bUndoAction = FALSE );
    void                    ImplSetLevelDependendStyleSheet( USHORT nPara, SfxStyleSheet* pLevelStyle = NULL );

    void                    ImplBlockInsertionCallbacks( BOOL b );

    void                ImplCheckStyleSheet( USHORT nPara, BOOL bReplaceExistingStyle );
    void                ImpRecalcBulletIndent( ULONG nPara );

    const SvxBulletItem& ImpGetBullet( ULONG nPara, USHORT& );
    void        ImpFilterIndents( ULONG nFirstPara, ULONG nLastPara );
    void        ImpConvertOutToEdt( Paragraph* pPara, ULONG nPara );
    BOOL        ImpConvertEdtToOut( Paragraph* pPara, ULONG nPara, EditView* pView = 0 );

    void        ImpTextPasted( ULONG nStartPara, USHORT nCount );
    void        ImpDropped( OutlinerView* pView );
    long        ImpCalcMaxBulletWidth( USHORT nPara, const SvxBulletItem& rBullet );
    Font        ImpCalcBulletFont( USHORT nPara ) const;
    Rectangle   ImpCalcBulletArea( USHORT nPara, BOOL bAdjust, BOOL bReturnPaperPos );
    long        ImpGetTextIndent( ULONG nPara );
    BOOL        ImpCanIndentSelectedPages( OutlinerView* pCurView );
    BOOL        ImpCanDeleteSelectedPages( OutlinerView* pCurView );
    BOOL        ImpCanDeleteSelectedPages( OutlinerView* pCurView, USHORT nFirstPage, USHORT nPages );

    USHORT      ImplGetOutlinerMode() const { return nOutlinerMode & OUTLINERMODE_USERMASK; }
    void        ImplCheckDepth( USHORT& rnDepth ) const;
#endif

protected:
    void            ParagraphInserted( USHORT nParagraph );
    void            ParagraphDeleted( USHORT nParagraph );
    void            ParaAttribsChanged( USHORT nParagraph );
    void            ParagraphHeightChanged( USHORT nParagraph );

    virtual void    StyleSheetChanged( SfxStyleSheet* pStyle );

    void        InvalidateBullet( Paragraph* pPara, ULONG nPara );
    void        PaintBullet( USHORT nPara, const Point& rStartPos,
                    const Point& rOrigin, short nOrientation,
                    OutputDevice* pOutDev );

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

    ULONG           InsertView( OutlinerView* pView, ULONG nIndex=LIST_APPEND);
    OutlinerView*   RemoveView( OutlinerView* pView );
    OutlinerView*   RemoveView( ULONG nIndex );
    OutlinerView*   GetView( ULONG nIndex ) const;
    ULONG           GetViewCount() const;

    Paragraph*      Insert( const String& rText, ULONG nAbsPos = LIST_APPEND,
                                USHORT nDepth = 0 );
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

    void            SetMinDepth( USHORT nDepth, BOOL bCheckParas = FALSE );
    USHORT          GetMinDepth() const { return nMinDepth; }

    void            SetMaxDepth( USHORT nDepth, BOOL bCheckParas = FALSE );
    USHORT          GetMaxDepth() const { return nMaxDepth; }

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
//  Paragraph*      GetParagraph( Paragraph* pParent, ULONG nRelPos ) const;
    Paragraph*      GetParent( Paragraph* pParagraph ) const;
//  ULONG           GetRelPos( Paragraph* pParent, Paragraph* pPara ) const;
    ULONG           GetAbsPos( Paragraph* pPara );

    USHORT          GetDepth( ULONG nPara ) const;
    void            SetDepth( Paragraph* pParagraph, USHORT nNewDepth );

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
    USHORT          GetPrevDepth() const { return nDepthChangedHdlPrevDepth; }

    virtual long    RemovingPagesHdl( OutlinerView* );
    void            SetRemovingPagesHdl(const Link& rLink){aRemovingPagesHdl=rLink;}
    Link            GetRemovingPagesHdl() const { return aRemovingPagesHdl; }
    virtual long    IndentingPagesHdl( OutlinerView* );
    void            SetIndentingPagesHdl(const Link& rLink){aIndentingPagesHdl=rLink;}
    Link            GetIndentingPagesHdl() const { return aIndentingPagesHdl; }
    // nur gueltig in den beiden oberen Handlern
    USHORT          GetSelPageCount() const { return nDepthChangedHdlPrevDepth; }
    // nur gueltig in den beiden oberen Handlern
    ULONG           GetFirstSelPage() const { return mnFirstSelPage; }

    void            SetCalcFieldValueHdl(const Link& rLink ) { aCalcFieldValueHdl= rLink; }
    Link            GetCalcFieldValueHdl() const { return aCalcFieldValueHdl; }

    void            SetFieldClickedHdl(const Link& rLink ) { aFieldClickedHdl= rLink; }
    Link            GetFieldClickedHdl() const { return aFieldClickedHdl; }

    void            SetDrawPortionHdl(const Link& rLink){aDrawPortionHdl=rLink;}
    Link            GetDrawPortionHdl() const { return aDrawPortionHdl; }

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

    void            OverwriteLevel0Bullet( const SvxNumberFormat& rNumberFormat );
    EBulletInfo     GetBulletInfo( USHORT nPara );

    void        SetWordDelimiters( const String& rDelimiters );
    String      GetWordDelimiters() const;
    String      GetWord( USHORT nPara, xub_StrLen nIndex );

    void            StripPortions();

    // #101498#
    virtual void    DrawingText( const Point& rStartPos, const String& rText, USHORT nTextStart, USHORT nTextLen,
                        const sal_Int32* pDXArray, const SvxFont& rFont,
                        USHORT nPara, xub_StrLen nIndex, BYTE nRightToLeft);

    Size            CalcTextSize();

    Point           GetDocPos( Paragraph* pPara );

    void                SetStyleSheetPool( SfxStyleSheetPool* pSPool );
    SfxStyleSheetPool*  GetStyleSheetPool();

    BOOL            IsInSelectionMode() const;

    void            SetStyleSheet( ULONG nPara, SfxStyleSheet* pStyle );
    SfxStyleSheet*  GetStyleSheet( ULONG nPara );

    void            SetParaAttribs( ULONG nPara, const SfxItemSet&, bool bApiCall = false );
    SfxItemSet      GetParaAttribs( ULONG nPara );

    void            Remove( Paragraph* pPara, ULONG nParaCount );
    BOOL            Expand( Paragraph* );
    BOOL            Collapse( Paragraph* );

    // gibt ein Array mit den Bulletbreiten der n Einrueckebenen
    // zurueck. Letzter Wert muss -1 sein. Wird vom Outliner geloescht.
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

    // nFormat muss ein Wert aus dem enum EETextFormat sein (wg.CLOOKS)
    ULONG           Read( SvStream& rInput, const String& rBaseURL, USHORT, SvKeyValueIterator* pHTTPHeaderAttrs = NULL );

    SfxUndoManager& GetUndoManager();

    void            QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel );
    void            QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel );
    void            QuickInsertLineBreak( const ESelection& rSel );

    // nur fuer EditEngine-Modus
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

    void            SetForbiddenCharsTable( vos::ORef<SvxForbiddenCharactersTable> xForbiddenChars );
    vos::ORef<SvxForbiddenCharactersTable>  GetForbiddenCharsTable() const;

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

    // #110496#
    /** Enable/disable verbose comments of recorded metafiles

        This method decorates recorded metafiles with XTEXT_EOx
        comments, to transport the logical text structure like
        paragraph, line, sentence, word and cell breaks.

        Specifically, comment actions named XTEXT_EOC contain the
        index of the last glyph of a character, XTEXT_EOW contain the
        index of the last glyph of a word, and XTEXT_EOS the last
        index of a sentence, respectively. The named three comment
        actions appear after a text rendering action
        (e.g. META_TEXT_ACTION), and the index values reference
        character indices in this last text render action (this is
        because text render actions cannot be split up in a
        output-preserving way).

        XTEXT_EOL and XTEXT_EOP appear after the end of a line or a
        paragraph, respectively, and don't carry a glyph index (as
        they cannot appear inbetween a META_TEXT_ACTION).
     */
    void            EnableVerboseTextComments( BOOL bEnable = TRUE );
    BOOL            IsVerboseTextComments() const;

    void            StartSpelling(EditView& rEditView, sal_Bool bMultipleDoc);
    //spell and return a sentence
    bool            SpellSentence(EditView& rEditView, ::svx::SpellPortions& rToFill);
    //applies a changed sentence
    void            ApplyChangedSentence(EditView& rEditView, const ::svx::SpellPortions& rNewPortions);
    void            EndSpelling();

    /** sets a link that is called at the beginning of a drag operation at an edit view */
    void            SetBeginDropHdl( const Link& rLink );
    Link            GetBeginDropHdl() const;

    /** sets a link that is called at the end of a drag operation at an edit view */
    void            SetEndDropHdl( const Link& rLink );
    Link            GetEndDropHdl() const;
};

#endif

