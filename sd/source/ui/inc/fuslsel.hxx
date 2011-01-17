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

#ifndef SD_FU_SLIDE_SELECTION_HXX
#define SD_FU_SLIDE_SELECTION_HXX

#include "fuslid.hxx"
#include <tools/list.hxx>

class SdDrawDocument;
class Sound;

namespace sd {

class SlideView;
class SlideViewShell;
class Window;

struct FSS_IsShowingEffectInfo
{
    sal_Bool bIsShowingEffect;          // sal_True while we show a fade effect one the slide view
    sal_Bool bDisposed;                 // sal_True if the FuSlideSelection was deleted during fade effect
};


class FuSlideSelection
    : public FuSlide
{
public:
    TYPEINFO();

    static FunctionReference Create( SlideViewShell* pViewSh, ::sd::Window* pWin, SlideView* pView, SdDrawDocument* pDoc, SfxRequest& rReq );
    virtual void DoExecute( SfxRequest& rReq );

    // Mouse- & Key-Events
    virtual sal_Bool                KeyInput(const KeyEvent& rKEvt);
    virtual sal_Bool                MouseMove(const MouseEvent& rMEvt);
    virtual sal_Bool                MouseButtonUp(const MouseEvent& rMEvt);
    virtual sal_Bool                MouseButtonDown(const MouseEvent& rMEvt);
    virtual void                Paint(const Rectangle& rRect, ::sd::Window* pWin);

    virtual void                Activate();        // Function aktivieren
    virtual void                Deactivate();          // Function deaktivieren

    virtual void                ScrollStart();
    virtual void                ScrollEnd();

    sal_Bool                        IsShowingEffect() const { return pIsShowingEffectInfo && pIsShowingEffectInfo->bIsShowingEffect; }

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
    sal_Bool                        bSubstShown;
    sal_Bool                        bPageHit;
    List                        aSubstList;       // Liste mit Ertsatzdarstellungen
    Point                       aDragPos;             // hier wird die Seite angefasst
    sal_Bool                        bDragSelection;
    Point                       aDragSelRectAnchor;  // fester Punkt des Selektionsrechtecks
    Rectangle                   aDragSelRect;
    Point                       aPosOfInsertMarker;
    FSS_IsShowingEffectInfo*    pIsShowingEffectInfo;

    void                        DrawInsertMarker(sal_Bool bShow);
    Point                       CalcPosOfInsertMarker(const Point& rPoint);

    sal_uInt16                      GetTargetPage(const Point& rPoint) const;

    void                        CreateSubst();
    void                        DeleteSubst();
    void                        DrawSubst() const;
    void                        ChangeSubstPos(const Point& rVector);
    void                        DrawDragSelectionRect() const;
    void                        ShowEffect(sal_uInt16 nPageNo);

                                DECL_LINK( DragSlideHdl, Timer* );
};

} // end of namespace sd

#endif

