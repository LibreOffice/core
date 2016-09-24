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

#ifndef INCLUDED_SD_SOURCE_UI_INC_FUOLTEXT_HXX
#define INCLUDED_SD_SOURCE_UI_INC_FUOLTEXT_HXX

#include "fuoutl.hxx"

class SdDrawDocument;
class SfxRequest;

namespace vcl { class Window; }

namespace sd {

class View;
class ViewShell;

/**
 * text functions in outline mode
 */
class FuOutlineText
    : public FuOutline
{
public:

    static rtl::Reference<FuPoor> Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq );

    virtual bool KeyInput(const KeyEvent& rKEvt) override;
    virtual bool MouseMove(const MouseEvent& rMEvt) override;
    virtual bool MouseButtonUp(const MouseEvent& rMEvt) override;
    virtual bool MouseButtonDown(const MouseEvent& rMEvt) override;

    virtual void DoCut() override;
    virtual void DoCopy() override;
    virtual void DoPaste() override;
    virtual void DoPasteUnformatted() override;

    virtual void Activate() override;           // Function aktivieren
    virtual void Deactivate() override;         // Function deaktivieren

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
