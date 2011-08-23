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
#ifndef _SWPVIEW_HXX
#define _SWPVIEW_HXX

#ifndef _SFXVIEWFRM_HXX //autogen
#include <bf_sfx2/viewfrm.hxx>
#endif
#ifndef _VIEWFAC_HXX //autogen
#include <bf_sfx2/viewfac.hxx>
#endif
#ifndef _LINK_HXX //autogen
#include <tools/link.hxx>
#endif
#ifndef _WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif
#ifndef _SFXVIEWSH_HXX //autogen
#include <bf_sfx2/viewsh.hxx>
#endif
#include "shellid.hxx"
#ifndef _SVX_ZOOMITEM_HXX
#include <bf_svx/zoomitem.hxx>
#endif
class ImageButton; 
class Button; 
class SvtAccessibilityOptions; 
namespace binfilter {

class SwViewOption;
class SwDocShell;
class SwScrollbar;
class ViewShell;
class SwPagePreView;
class SwRect;
class DataChangedEvent;
class CommandEvent;

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


public:
    SwPagePreViewWin( Window* pParent, SwPagePreView& rView );
    ~SwPagePreViewWin();

    //Ruft ViewShell::Paint


    ViewShell* GetViewShell() const { return mpViewShell; }


    /** get selected page number of document preview

        OD 13.12.2002 #103492#

        @author OD

        @return selected page number
    */

    /** set selected page number in document preview

        OD 13.12.2002 #103492#

        @author OD

        @param _nSelectedPageNum
        input parameter - physical page number of page that will be the selected one.
    */

    //JP 19.08.98: bei Einspaltigkeit gibt es keine 0. Seite!



    // OD 18.12.2002 #103492# - add <MV_SELPAGE>, <MV_SCROLL>

    // erzeuge den String fuer die StatusLeiste


    /** method to adjust preview to a new zoom factor

        OD 02.12.2002 #103492#
        paint of preview is prepared for a new zoom factor

        @author OD
    */
#ifdef ACCESSIBLE_LAYOUT
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
    String 					sSwViewData,
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
     Window					*pScrollFill;

    USHORT                  mnPageCount;
    BOOL                    bNormalPrint;

    // OD 09.01.2003 #106334#
    // new members to reset design mode at draw view for form shell on switching
    // back from writer page preview to normal view.
     sal_Bool                mbResetFormDesignMode:1;
    sal_Bool                mbFormDesignModeToReset:1;






    /** help method to execute SfxRequest FN_PAGE_UP and FN_PAGE_DOWN

        OD 04.03.2003 #107369#

        @param _bPgUp
        input parameter - boolean that indicates, if FN_PAGE_UP or FN_PAGE_DOWN
        has to be executed.

        @param _pReq
        optional input parameter - pointer to the <SfxRequest> instance, if existing.

        @author OD
    */

protected:

    void         SetZoom(SvxZoomType eSet, USHORT nFactor);

public:
    SFX_DECL_VIEWFACTORY(SwPagePreView);
    SFX_DECL_INTERFACE(SW_PAGEPREVIEW);
    TYPEINFO();

     inline ViewShell*       GetViewShell() const { return aViewWin.GetViewShell(); }








        // Handler
    void            Execute(SfxRequest&){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 	void            Execute(SfxRequest&);
    void            GetState(SfxItemSet&){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 	void            GetState(SfxItemSet&);
    void            StateUndo(SfxItemSet&){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 	void            StateUndo(SfxItemSet&);

    SwDocShell*     GetDocShell();

    //apply Accessiblity options

    // OD 09.01.2003 #106334# - inline method to request values of new members
    // <mbResetFormDesignMode> and <mbFormDesignModeToReset>


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



} //namespace binfilter
#endif
