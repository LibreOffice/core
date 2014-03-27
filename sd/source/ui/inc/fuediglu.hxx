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

#ifndef INCLUDED_SD_SOURCE_UI_INC_FUEDIGLU_HXX
#define INCLUDED_SD_SOURCE_UI_INC_FUEDIGLU_HXX

#include "fudraw.hxx"

namespace sd {

class FuEditGluePoints
    : public FuDraw
{
public:
    TYPEINFO_OVERRIDE();

    static rtl::Reference<FuPoor> Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq, bool bPermanent );
    virtual void DoExecute( SfxRequest& rReq ) SAL_OVERRIDE;

    // Mouse- & Key-Events
    virtual sal_Bool KeyInput(const KeyEvent& rKEvt) SAL_OVERRIDE;
    virtual sal_Bool MouseMove(const MouseEvent& rMEvt) SAL_OVERRIDE;
    virtual sal_Bool MouseButtonUp(const MouseEvent& rMEvt) SAL_OVERRIDE;
    virtual sal_Bool MouseButtonDown(const MouseEvent& rMEvt) SAL_OVERRIDE;
    virtual sal_Bool Command(const CommandEvent& rCEvt) SAL_OVERRIDE;
    virtual void ReceiveRequest(SfxRequest& rReq) SAL_OVERRIDE;

    virtual void Activate() SAL_OVERRIDE;
    virtual void Deactivate() SAL_OVERRIDE;

    //Add Shift+UP/DOWN/LEFT/RIGHT key to move the position of insert point,
    //and SHIFT+ENTER key to decide the postion and draw the new insert point
    virtual void ForcePointer(const MouseEvent* pMEvt = NULL) SAL_OVERRIDE;
private:
    sal_Bool   bBeginInsertPoint;
    Point    oldPoint;
protected:
    FuEditGluePoints (
        ViewShell* pViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);
    virtual ~FuEditGluePoints (void);
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
