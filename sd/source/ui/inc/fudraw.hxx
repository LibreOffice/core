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

#ifndef INCLUDED_SD_SOURCE_UI_INC_FUDRAW_HXX
#define INCLUDED_SD_SOURCE_UI_INC_FUDRAW_HXX

#include "fupoor.hxx"

struct SdrViewEvent;
class SdrObject;

namespace sd {

/**
 * Base class for all Draw specific functions
 */
class FuDraw
    : public FuPoor
{
public:

    virtual bool KeyInput(const KeyEvent& rKEvt) override;
    virtual bool MouseMove(const MouseEvent& rMEvt) override;
    virtual bool MouseButtonUp(const MouseEvent& rMEvt) override;
    virtual bool MouseButtonDown(const MouseEvent& rMEvt) override;
    virtual bool RequestHelp(const HelpEvent& rHEvt) override;

    virtual void Activate() override;

    virtual void ForcePointer(const MouseEvent* pMEvt = nullptr);

    virtual void DoubleClick(const MouseEvent& rMEvt);

    bool    SetPointer(SdrObject* pObj, const Point& rPos);
    bool    SetHelpText(SdrObject* pObj, const Point& rPos, const SdrViewEvent& rVEvt);

    void    SetPermanent(bool bSet) { bPermanent = bSet; }

    /** is called when the current function should be aborted. <p>
        This is used when a function gets a KEY_ESCAPE but can also
        be called directly.

        @returns true if a active function was aborted
    */
    virtual bool cancel() override;

protected:
    FuDraw (ViewShell* pViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);

    virtual ~FuDraw() override;

    PointerStyle aNewPointer;
    PointerStyle aOldPointer;
    bool    bMBDown;
    bool    bDragHelpLine;
    sal_uInt16  nHelpLine;
    bool    bPermanent;

private:
    void DoModifiers(const MouseEvent& rMEvt, bool bSnapModPressed);

};

} // end of namespace sd

#endif      // _SD_FUDRAW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
