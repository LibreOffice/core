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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
