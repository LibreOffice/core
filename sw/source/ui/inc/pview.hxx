/*************************************************************************
 *
 *  $RCSfile: pview.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:41 $
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

class SwPagePreViewWin : public Window
{
    ViewShell*          pViewShell;
    USHORT              nSttPage, nVirtPage;
    BYTE                nRow, nCol;
    Size                aWinSize, aPgSize;
    Fraction            aScale;
    SwPagePreView&      rView;

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

    void SetViewShell( ViewShell* pShell ) { pViewShell = pShell; }
    ViewShell* GetViewShell() const { return pViewShell; }

    BYTE    GetRow() const      { return nRow; }
    void    SetRow( BYTE n )    { if( n ) nRow = n; }

    BYTE    GetCol() const      { return nCol; }
    void    SetCol( BYTE n )    { if( n ) nCol = n; }

    USHORT  GetVirtPage() const     { return nVirtPage; }
    USHORT  GetSttPage() const      { return nSttPage; }
    void    SetSttPage( USHORT n )
        { nSttPage = nVirtPage = n; if( !n ) ++nVirtPage; }

    //JP 19.08.98: bei Einspaltigkeit gibt es keine 0. Seite!
    USHORT  GetDefSttPage() const   { return 1 == nCol ? 1 : 0; }

    void CalcWish( BYTE nNewRow, BYTE nNewCol );

    const Size& GetWinSize() const  { return aWinSize; }
    void SetWinSize( const Size& rNewSize );

    enum MoveMode{ MV_CALC, MV_PAGE_UP, MV_PAGE_DOWN, MV_DOC_STT, MV_DOC_END };
    int MovePage( int eMoveMode );

    // erzeuge den String fuer die StatusLeiste
    void GetStatusStr( String& rStr, USHORT nPageCount ) const;
    void GetOptimalSize( Size& rSize ) const;

    void RepaintCoreRect( const SwRect& rRect );
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
    // Sichtbarer Bereich
    String                  sPageStr;
    Size                    aDocSz;
    Rectangle               aVisArea;

    // MDI Bedienelemente
    SwScrollbar             *pHScrollbar;
    SwScrollbar             *pVScrollbar;
    ImageButton             *pPageUpBtn,
                            *pPageDownBtn;
    // Dummy-Window zum Fllen der rechten unteren Ecke, wenn beide Scrollbars
    // aktiv sind
    Window                  *pScrollFill;

    USHORT                  nPageCount;
    BOOL                    bNormalPrint;

    void            Init(const SwViewOption* = 0);
    Point           AlignToPixel(const Point& rPt) const;

    int             _CreateScrollbar( int bHori );
    int             _KillScrollbar( int bHori );
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


protected:
    virtual void    InnerResizePixel( const Point &rOfs, const Size &rSize );
    virtual void    OuterResizePixel( const Point &rOfs, const Size &rSize );
    virtual Size    GetOptimalSizePixel() const;

public:
    SFX_DECL_VIEWFACTORY(SwPagePreView);
    SFX_DECL_INTERFACE(SW_PAGEPREVIEW);
    TYPEINFO();

    inline Window*          GetFrameWindow() const { return &(GetViewFrame())->GetWindow(); }
    inline ViewShell&       GetViewShell() const { return *aViewWin.GetViewShell(); }
    inline const Rectangle& GetVisArea() const { return aVisArea; }
    inline void             GrabFocusViewWin() { aViewWin.GrabFocus(); }
    inline void             RepaintCoreRect( const SwRect& rRect )
                                { aViewWin.RepaintCoreRect( rRect ); }

    void            DocSzChgd(const Size& rNewSize);
    const Size&     GetDocSz() const { return aDocSz; }

    virtual void    SetVisArea( const Rectangle&, BOOL bUpdateScrollbar = TRUE);

    inline int      StatHScrollbar() const;
    inline int      CreateHScrollbar();
    inline int      KillHScrollbar();
    inline int      CreateVScrollbar();
    inline int      KillVScrollbar();
    inline int      StatVScrollbar() const;
    inline void     AdjustEditWin();

    void            VScrollViewSzChg();
    void            VScrollDocSzChg();

    USHORT          GetPageCount() const        { return nPageCount; }

    BOOL            HandleWheelCommands( const CommandEvent& );

    const String&   GetPrevSwViewData() const       { return sSwViewData; }
    void            SetNewCrsrPos( const String& rStr ) { sNewCrsrPos = rStr; }
    const String&   GetNewCrsrPos() const           { return sNewCrsrPos; }
        // Handler
    void            Execute(SfxRequest&);
    void            GetState(SfxItemSet&);
    void            StateUndo(SfxItemSet&);

    SwDocShell*     GetDocShell();

    SwPagePreView( SfxViewFrame* pFrame, SfxViewShell* );
    ~SwPagePreView();
};

// ----------------- inline Methoden ----------------------

inline int SwPagePreView::StatHScrollbar() const
{
    return  0 != pHScrollbar;
}
inline int SwPagePreView::CreateHScrollbar()
{
    return StatHScrollbar() ? 1 : _CreateScrollbar( TRUE );
}
inline int SwPagePreView::KillHScrollbar()
{
    return StatHScrollbar() ? _KillScrollbar( TRUE ) : 1;
}

inline int SwPagePreView::StatVScrollbar() const
{
    return  0 != pVScrollbar;
}
inline int SwPagePreView::CreateVScrollbar()
{
    return StatVScrollbar() ? 1 : _CreateScrollbar( FALSE );
}
inline int SwPagePreView::KillVScrollbar()
{
    return StatVScrollbar() ? _KillScrollbar( FALSE ) : 1;
}

inline void SwPagePreView::AdjustEditWin()
{
    OuterResizePixel( Point(), GetFrameWindow()->GetOutputSizePixel() );
}

#endif
