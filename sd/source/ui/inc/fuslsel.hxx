/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fuslsel.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 16:27:21 $
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

#ifndef SD_FU_SLIDE_SELECTION_HXX
#define SD_FU_SLIDE_SELECTION_HXX

#ifndef SD_FU_SLIDE_HXX
#include "fuslid.hxx"
#endif
#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif

class SdDrawDocument;
class Sound;

namespace sd {

class SlideView;
class SlideViewShell;
class Window;

struct FSS_IsShowingEffectInfo
{
    BOOL bIsShowingEffect;          // TRUE while we show a fade effect one the slide view
    BOOL bDisposed;                 // TRUE if the FuSlideSelection was deleted during fade effect
};


class FuSlideSelection
    : public FuSlide
{
public:
    TYPEINFO();

    static FunctionReference Create( SlideViewShell* pViewSh, ::sd::Window* pWin, SlideView* pView, SdDrawDocument* pDoc, SfxRequest& rReq );
    virtual void DoExecute( SfxRequest& rReq );

    // Mouse- & Key-Events
    virtual BOOL                KeyInput(const KeyEvent& rKEvt);
    virtual BOOL                MouseMove(const MouseEvent& rMEvt);
    virtual BOOL                MouseButtonUp(const MouseEvent& rMEvt);
    virtual BOOL                MouseButtonDown(const MouseEvent& rMEvt);
    virtual void                Paint(const Rectangle& rRect, ::sd::Window* pWin);

    virtual void                Activate();        // Function aktivieren
    virtual void                Deactivate();          // Function deaktivieren

    virtual void                ScrollStart();
    virtual void                ScrollEnd();

    BOOL                        IsShowingEffect() const { return pIsShowingEffectInfo && pIsShowingEffectInfo->bIsShowingEffect; }

    /** is called when the currenct function should be aborted. <p>
        This is used when a function gets a KEY_ESCAPE but can also
        be called directly.

        @returns true if a active function was aborted
    */
    virtual bool cancel();

protected:
    FuSlideSelection (
        SlideViewShell* pViewSh,
        ::sd::Window* pWin,
        SlideView* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);
    virtual ~FuSlideSelection (void);

private:
    BOOL                        bSubstShown;
    BOOL                        bPageHit;
    List                        aSubstList;       // Liste mit Ertsatzdarstellungen
    Point                       aDragPos;             // hier wird die Seite angefasst
    BOOL                        bDragSelection;
    Point                       aDragSelRectAnchor;  // fester Punkt des Selektionsrechtecks
    Rectangle                   aDragSelRect;
    Point                       aPosOfInsertMarker;
    FSS_IsShowingEffectInfo*    pIsShowingEffectInfo;

    void                        DrawInsertMarker(BOOL bShow);
    Point                       CalcPosOfInsertMarker(const Point& rPoint);

    USHORT                      GetTargetPage(const Point& rPoint) const;

    void                        CreateSubst();
    void                        DeleteSubst();
    void                        DrawSubst() const;
    void                        ChangeSubstPos(const Point& rVector);
    void                        DrawDragSelectionRect() const;
    void                        ShowEffect(USHORT nPageNo);

                                DECL_LINK( DragSlideHdl, Timer* );
};

} // end of namespace sd

#endif

