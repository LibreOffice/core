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

#ifndef SD_FU_CONSTRUCT_HXX
#define SD_FU_CONSTRUCT_HXX

#include "fudraw.hxx"

class KeyEvent;
class SdrObject;
class SfxItemSet;

namespace sd {

class FuConstruct
    : public FuDraw
{
public:
    TYPEINFO();

    virtual void DoExecute( SfxRequest& rReq );

    // Mouse- & Key-Events
    virtual sal_Bool KeyInput(const KeyEvent& rKEvt);
    virtual sal_Bool MouseMove(const MouseEvent& rMEvt);
    virtual sal_Bool MouseButtonUp(const MouseEvent& rMEvt);
    virtual sal_Bool MouseButtonDown(const MouseEvent& rMEvt);

    virtual void Activate();
    virtual void Deactivate();

    virtual void SelectionHasChanged() { bSelectionChanged = sal_True; }

    // SJ: setting stylesheet, the use of a filled or unfilled style
    // is determined by the member nSlotId :
    void SetStyleSheet(SfxItemSet& rAttr, SdrObject* pObj);

    // SJ: setting stylesheet, the use of a filled or unfilled style
    // is determinded by the parameters bUseFillStyle and bUseNoFillStyle :
    void SetStyleSheet( SfxItemSet& rAttr, SdrObject* pObj,
            const sal_Bool bUseFillStyle, const sal_Bool bUseNoFillStyle );

protected:
    FuConstruct (ViewShell* pViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);

    bool bSelectionChanged;
};

} // end of namespace sd

#endif      // _SD_FUCONSTR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
