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

#ifndef INCLUDED_SD_SOURCE_UI_INC_FUCONSTR_HXX
#define INCLUDED_SD_SOURCE_UI_INC_FUCONSTR_HXX

#include "fudraw.hxx"

class SdrObject;
class SfxItemSet;

namespace sd {

class FuConstruct : public FuDraw
{
public:

    // Mouse Events
    virtual bool MouseMove(const MouseEvent& rMEvt) override;
    virtual bool MouseButtonUp(const MouseEvent& rMEvt) override;
    virtual bool MouseButtonDown(const MouseEvent& rMEvt) override;

    virtual void Activate() override;
    virtual void Deactivate() override;

    virtual void SelectionHasChanged() override { bSelectionChanged = true; }

    // SJ: setting stylesheet, the use of a filled or unfilled style
    // is determined by the member nSlotId :
    void SetStyleSheet(SfxItemSet& rAttr, SdrObject* pObj);

    // SJ: setting stylesheet, the use of a filled or unfilled style
    // is determined by the parameters bUseFillStyle and bUseNoFillStyle :
    void SetStyleSheet( SfxItemSet& rAttr, SdrObject* pObj,
            const bool bUseFillStyle, const bool bUseNoFillStyle );

protected:
    FuConstruct (ViewShell* pViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);
private:
    bool bSelectionChanged;
};

} // end of namespace sd

#endif      // _SD_FUCONSTR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
