/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SD_SOURCE_UI_INC_FUSEL_HXX
#define INCLUDED_SD_SOURCE_UI_INC_FUSEL_HXX

#include "fudraw.hxx"

#include <com/sun/star/media/XPlayer.hpp>

class SdrHdl;
class SdrObject;

namespace sd {

class FuSelection
    : public FuDraw
{
public:

    static rtl::Reference<FuPoor> Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq );
    virtual void DoExecute( SfxRequest& rReq ) override;

                                       // Mouse- & Key-Events
    virtual bool KeyInput(const KeyEvent& rKEvt) override;
    virtual bool MouseMove(const MouseEvent& rMEvt) override;
    virtual bool MouseButtonUp(const MouseEvent& rMEvt) override;
    virtual bool MouseButtonDown(const MouseEvent& rMEvt) override;

    virtual void Activate() override;
    virtual void Deactivate() override;

    virtual void SelectionHasChanged() override;

    void    SetEditMode(sal_uInt16 nMode);
    sal_uInt16  GetEditMode() { return nEditMode; }

    bool    AnimateObj(SdrObject* pObj, const Point& rPos);

    /** is called when the current function should be aborted. <p>
        This is used when a function gets a KEY_ESCAPE but can also
        be called directly.

        @returns true if a active function was aborted
    */
    virtual bool cancel() override;

    //let mouse cursor move
    virtual void ForcePointer(const MouseEvent* pMEvt = NULL) override;
protected:
    FuSelection (ViewShell* pViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);

    virtual ~FuSelection();

    bool            bTempRotation;
    bool            bSelectionChanged;
    bool            bHideAndAnimate;
    SdrHdl*         pHdl;
    bool            bSuppressChangesOfSelection;
    bool            bMirrorSide0;
    sal_uInt16      nEditMode;
    css::uno::Reference< css::media::XPlayer > mxPlayer;

private:
    /** This pointer stores a canidate for assigning a style in the water
        can mode between mouse button down and mouse button up.
    */
    SdrObject* pWaterCanCandidate;

    /** Find the object under the given test point without selecting it.
        @param rTestPoint
            The coordinates at which to search for a shape.
        @return
            The shape at the test point.  When there is no shape at this
            position then NULL is returned.
    */
    SdrObject* pickObject (const Point& rTestPoint);
    //Add Shift+UP/DOWN/LEFT/RIGHT key to move the position of insert point,
    //and SHIFT+ENTER key to decide the position and draw the new insert point
    bool   bBeginInsertPoint;
    Point    oldPoint;
    //let mouse cursor move
    bool   bMovedToCenterPoint;
};

} // end of namespace sd

#endif      // _SD_FUSEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
