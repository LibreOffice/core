/*************************************************************************
 *
 *  $RCSfile: pview.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 16:14:54 $
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
#ifndef _SWPVIEW_HXX
#define _SWPVIEW_HXX

#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _VIEWFAC_HXX //autogen
#include <sfx2/viewfac.hxx>
#endif
#ifndef _LINK_HXX //autogen
#include <tools/link.hxx>
#endif
#ifndef _WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif
#ifndef _SFXVIEWSH_HXX //autogen
#include <sfx2/viewsh.hxx>
#endif
#include "shellid.hxx"
#ifndef _SVX_ZOOMITEM_HXX
#include <svx/zoomitem.hxx>
#endif

class SwViewOption;
class SwDocShell;
class SwScrollbar;
class ViewShell;
class SwPagePreView;
class ImageButton;
class Button;
class SwRect;
class DataChangedEvent;
class CommandEvent;
class SvtAccessibilityOptions;
// OD 12.12.2002 #103492#
class SwPagePreviewLayout;

// OD 24.03.2003 #108282# - delete member <mnVirtPage> and its accessor.
class SwPagePreViewWin : public Window
{
    ViewShell*          mpViewShell;
    USHORT              mnSttPage;
    BYTE                mnRow, mnCol;
    Size                maPxWinSize;
    Fraction            maScale;
    SwPagePreView&      mrView;
    // OD 02.12.2002 #103492#
    bool                mbCalcScaleForPreviewLayout;
    Rectangle           maPaintedPreviewDocRect;
    // OD 12.12.2002 #103492#
    SwPagePreviewLayout* mpPgPrevwLayout;

    void SetPagePreview( BYTE nRow, BYTE nCol );

public:
    SwPagePreViewWin( Window* pParent, SwPagePreView& rView );
    ~SwPagePreViewWin();

    //Ruft ViewShell::Paint
    virtual void Paint( const Rectangle& rRect );
    virtual void KeyInput( const KeyEvent & );
    virtual void Command( const CommandEvent& rCEvt );
    virtual void MouseButtonDown(const MouseEvent& rMEvt);
    virtual void DataChanged( const DataChangedEvent& );

    void SetViewShell( ViewShell* pShell );

    ViewShell* GetViewShell() const { return mpViewShell; }

    BYTE    GetRow() const      { return mnRow; }
    void    SetRow( BYTE n )    { if( n ) mnRow = n; }

    BYTE    GetCol() const      { return mnCol; }
    void    SetCol( BYTE n )    { if( n ) mnCol = n; }

    USHORT  GetSttPage() const      { return mnSttPage; }
    void    SetSttPage( USHORT n )  { mnSttPage = n; }

    /** get selected page number of document preview

        OD 13.12.2002 #103492#

        @author OD

        @return selected page number
    */
    sal_uInt16 SelectedPage() const;

    /** set selected page number in document preview

        OD 13.12.2002 #103492#

        @author OD

        @param _nSelectedPageNum
        input parameter - physical page number of page that will be the selected one.
    */
    void SetSelectedPage( sal_uInt16 _nSelectedPageNum );

    //JP 19.08.98: bei Einspaltigkeit gibt es keine 0. Seite!
    USHORT  GetDefSttPage() const   { return 1 == mnCol ? 1 : 0; }

    void CalcWish( BYTE nNewRow, BYTE nNewCol );

    const Size& GetWinSize() const  { return maPxWinSize; }
    void SetWinSize( const Size& rNewSize );

    // OD 18.12.2002 #103492# - add <MV_SELPAGE>, <MV_SCROLL>
    enum MoveMode{ MV_CALC, MV_PAGE_UP, MV_PAGE_DOWN, MV_DOC_STT, MV_DOC_END,
                   MV_SELPAGE, MV_SCROLL, MV_NEWWINSIZE };
    int MovePage( int eMoveMode );

    // erzeuge den String fuer die StatusLeiste
    void GetStatusStr( String& rStr, USHORT nPageCount ) const;

    void RepaintCoreRect( const SwRect& rRect );

    /** method to adjust preview to a new zoom factor

        OD 02.12.2002 #103492#
        paint of preview is prepared for a new zoom factor

        @author OD
    */
    void AdjustPreviewToNewZoom( const sal_uInt16 nZoomFactor );

    const Rectangle&           GetPaintedPreviewDocRect() const
                                    { return maPaintedPreviewDocRect;}
    void                       Scroll(long nXMove, long nYMove);

#ifdef ACCESSIBLE_LAYOUT
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible>
                    CreateAccessible();
#endif
};


/*--------------------------------------------------------------------
    Beschreibung:   Sicht auf ein Dokument
 --------------------------------------------------------------------*/

class SwPagePreView: public SfxViewShell
{
    // ViewWindow und Henkel zur Core
    // aktuelle Dispatcher-Shell
    SwPagePreViewWin        aViewWin;
    //viewdata of the previous SwView and the new crsrposition
    String                  sSwViewData,
    //and the new cursor position if the user double click in the PagePreView
                            sNewCrsrPos;
    // to support keyboard the number of the page to go to can be set too
    USHORT                  nNewPage;
    // Sichtbarer Bereich
    String                  sPageStr;
    Size                    aDocSz;
    Rectangle               aVisArea;

    // MDI Bedienelemente
    SwScrollbar             *pHScrollbar;
    SwScrollbar             *pVScrollbar;
    ImageButton             *pPageUpBtn,
                            *pPageDownBtn;
    // Dummy-Window zum FÅllen der rechten unteren Ecke, wenn beide Scrollbars
    // aktiv sind
    Window                  *pScrollFill;

    USHORT                  mnPageCount;
    BOOL                    bNormalPrint;

    // OD 09.01.2003 #106334#
    // new members to reset design mode at draw view for form shell on switching
    // back from writer page preview to normal view.
    sal_Bool                mbResetFormDesignMode:1;
    sal_Bool                mbFormDesignModeToReset:1;

    void            Init(const SwViewOption* = 0);
    Point           AlignToPixel(const Point& rPt) const;

    int             _CreateScrollbar( int bHori );
    DECL_LINK( ScrollHdl, SwScrollbar * );
    DECL_LINK( EndScrollHdl, SwScrollbar * );
    DECL_LINK( BtnPage, Button * );
    int             ChgPage( int eMvMode, int bUpdateScrollbar = TRUE );


    virtual USHORT          Print( SfxProgress &rProgress,
                                   PrintDialog *pPrintDialog = 0 );
    virtual SfxPrinter*     GetPrinter( BOOL bCreate = FALSE );
    virtual USHORT          SetPrinter( SfxPrinter *pNewPrinter, USHORT nDiffFlags = SFX_PRINTER_ALL );
    virtual SfxTabPage*     CreatePrintOptionsPage( Window *pParent,
                                                const SfxItemSet &rOptions );
    virtual PrintDialog*    CreatePrintDialog( Window *pParent );

    void CalcAndSetBorderPixel( SvBorder &rToFill, FASTBOOL bInner );

    /** help method to execute SfxRequest FN_PAGE_UP and FN_PAGE_DOWN

        OD 04.03.2003 #107369#

        @param _bPgUp
        input parameter - boolean that indicates, if FN_PAGE_UP or FN_PAGE_DOWN
        has to be executed.

        @param _pReq
        optional input parameter - pointer to the <SfxRequest> instance, if existing.

        @author OD
    */
    void _ExecPgUpAndPgDown( const bool  _bPgUp,
                             SfxRequest* _pReq = 0 );

protected:
    virtual void    InnerResizePixel( const Point &rOfs, const Size &rSize );
    virtual void    OuterResizePixel( const Point &rOfs, const Size &rSize );
    virtual Size    GetOptimalSizePixel() const;

    void         SetZoom(SvxZoomType eSet, USHORT nFactor);

public:
    SFX_DECL_VIEWFACTORY(SwPagePreView);
    SFX_DECL_INTERFACE(SW_PAGEPREVIEW);
    TYPEINFO();

    inline Window*          GetFrameWindow() const { return &(GetViewFrame())->GetWindow(); }
    inline ViewShell*       GetViewShell() const { return aViewWin.GetViewShell(); }
    inline const Rectangle& GetVisArea() const { return aVisArea; }
    inline void             GrabFocusViewWin() { aViewWin.GrabFocus(); }
    inline void             RepaintCoreRect( const SwRect& rRect )
                                { aViewWin.RepaintCoreRect( rRect ); }

    void            DocSzChgd(const Size& rNewSize);
    const Size&     GetDocSz() const { return aDocSz; }

    virtual void    SetVisArea( const Rectangle&, BOOL bUpdateScrollbar = TRUE);

    inline void     AdjustEditWin();

    void            ScrollViewSzChg();
    void            ScrollDocSzChg();
    void            ShowHScrollbar(sal_Bool bShow);
    sal_Bool        IsHScrollbarVisible()const;

    void            ShowVScrollbar(sal_Bool bShow);
    sal_Bool        IsVScrollbarVisible()const;

    USHORT          GetPageCount() const        { return mnPageCount; }

    BOOL            HandleWheelCommands( const CommandEvent& );

    const String&   GetPrevSwViewData() const       { return sSwViewData; }
    void            SetNewCrsrPos( const String& rStr ) { sNewCrsrPos = rStr; }
    const String&   GetNewCrsrPos() const           { return sNewCrsrPos; }

    USHORT          GetNewPage() const {return nNewPage;}
    void            SetNewPage(USHORT nSet)  {nNewPage = nSet;}

        // Handler
    void            Execute(SfxRequest&);
    void            GetState(SfxItemSet&);
    void            StateUndo(SfxItemSet&);

    SwDocShell*     GetDocShell();

    //apply Accessiblity options
    void ApplyAccessiblityOptions(SvtAccessibilityOptions& rAccessibilityOptions);

    // OD 09.01.2003 #106334# - inline method to request values of new members
    // <mbResetFormDesignMode> and <mbFormDesignModeToReset>
    inline sal_Bool ResetFormDesignMode() const
    {
        return mbResetFormDesignMode;
    }

    inline sal_Bool FormDesignModeToReset() const
    {
        return mbFormDesignModeToReset;
    }

    /** adjust position of vertical scrollbar

        OD 19.02.2003 #107369
        Currently used, if the complete preview layout rows fit into to the given
        window, if a new page is selected and this page is visible.

        @author OD

        @param _nNewThumbPos
        input parameter - new position, which will be assigned to the vertical
        scrollbar.
    */
    void SetVScrollbarThumbPos( const sal_uInt16 _nNewThumbPos );

    SwPagePreView( SfxViewFrame* pFrame, SfxViewShell* );
    ~SwPagePreView();
};

// ----------------- inline Methoden ----------------------


inline void SwPagePreView::AdjustEditWin()
{
    OuterResizePixel( Point(), GetFrameWindow()->GetOutputSizePixel() );
}

#endif
