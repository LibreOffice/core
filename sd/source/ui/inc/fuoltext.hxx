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

#ifndef SD_FU_OUTLINE_TEXT_HXX
#define SD_FU_OUTLINE_TEXT_HXX

#include "fuoutl.hxx"

class SdDrawDocument;
class SfxRequest;

namespace sd {

class View;
class ViewShell;
class Window;

/**
 * text functions in outline mode
 */
class FuOutlineText
    : public FuOutline
{
public:
    TYPEINFO();

    static FunctionReference Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq );

    virtual sal_Bool KeyInput(const KeyEvent& rKEvt);
    virtual sal_Bool MouseMove(const MouseEvent& rMEvt);
    virtual sal_Bool MouseButtonUp(const MouseEvent& rMEvt);
    virtual sal_Bool MouseButtonDown(const MouseEvent& rMEvt);

    virtual void DoCut();
    virtual void DoCopy();
    virtual void DoPaste();

    virtual void Activate();           // Function aktivieren
    virtual void Deactivate();         // Function deaktivieren

    /** Call this method when the text in the outliner (may) has changed.
        It will invalidate some slots of the view frame and update the
        preview in the slide sorter.
    */
    void UpdateForKeyPress (const KeyEvent& rEvent);

protected:
    FuOutlineText (
        ViewShell* pViewShell,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);

};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
