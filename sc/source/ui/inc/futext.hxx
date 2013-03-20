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

#pragma once
#if 1

#include "fuconstr.hxx"

class SdrObject;
class SdrTextObj;
class SdrOutliner;



/** Base class for Text functions */
class FuText : public FuConstruct
{
protected:
    SdrTextObj*     pTextObj;

public:
    FuText(ScTabViewShell* pViewSh, Window* pWin, ScDrawView* pView,
                   SdrModel* pDoc, SfxRequest& rReq);

    virtual ~FuText();

    virtual sal_Bool KeyInput(const KeyEvent& rKEvt);
    virtual sal_Bool MouseMove(const MouseEvent& rMEvt);
    virtual sal_Bool MouseButtonUp(const MouseEvent& rMEvt);
    virtual sal_Bool MouseButtonDown(const MouseEvent& rMEvt);

    virtual void Activate();
    virtual void Deactivate();

    virtual void ForcePointer(const MouseEvent* pMEvt);

    virtual void SelectionHasChanged();

    void    SetInEditMode( SdrObject* pObj = NULL, const Point* pMousePixel = NULL,
                            sal_Bool bCursorToEnd = false, const KeyEvent* pInitialKey = NULL );
    void    StopEditMode(sal_Bool bTextDirection = false);
    void    StopDragMode(SdrObject* pObject);

    // Create default drawing objects via keyboard
    virtual SdrObject* CreateDefaultObject(const sal_uInt16 nID, const Rectangle& rRectangle);

private:
    SdrOutliner* MakeOutliner();
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
