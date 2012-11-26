/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    bool bIsShowingEffect;          // true while we show a fade effect one the slide view
    bool bDisposed;                 // true if the FuSlideSelection was deleted during fade effect
};


class FuSlideSelection
    : public FuSlide
{
public:
    static FunctionReference Create( SlideViewShell* pViewSh, ::sd::Window* pWin, SlideView* pView, SdDrawDocument* pDoc, SfxRequest& rReq );
    virtual void DoExecute( SfxRequest& rReq );

    // Mouse- & Key-Events
    virtual bool KeyInput(const KeyEvent& rKEvt);
    virtual bool MouseMove(const MouseEvent& rMEvt);
    virtual bool MouseButtonUp(const MouseEvent& rMEvt);
    virtual bool MouseButtonDown(const MouseEvent& rMEvt);
    virtual void                Paint(const Rectangle& rRect, ::sd::Window* pWin);

    virtual void                Activate();        // Function aktivieren
    virtual void                Deactivate();          // Function deaktivieren

    virtual void                ScrollStart();
    virtual void                ScrollEnd();

    bool                        IsShowingEffect() const { return pIsShowingEffectInfo && pIsShowingEffectInfo->bIsShowingEffect; }

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
    bool                        bSubstShown;
    bool                        bPageHit;
    List                        aSubstList;       // Liste mit Ertsatzdarstellungen
    Point                       aDragPos;             // hier wird die Seite angefasst
    bool                        bDragSelection;
    Point                       aDragSelRectAnchor;  // fester Punkt des Selektionsrechtecks
    Rectangle                   aDragSelRect;
    Point                       aPosOfInsertMarker;
    FSS_IsShowingEffectInfo*    pIsShowingEffectInfo;

    void                        DrawInsertMarker(bool bShow);
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

