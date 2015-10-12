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

#ifndef INCLUDED_SD_SOURCE_UI_INC_DRAWVIEW_HXX
#define INCLUDED_SD_SOURCE_UI_INC_DRAWVIEW_HXX

#include "View.hxx"

namespace sd {

class DrawDocShell;
class DrawViewShell;

/**
 * Derivative of ::sd::View; contains also a pointer to the document
 */
class DrawView : public ::sd::View
{
public:
    TYPEINFO_OVERRIDE();

    DrawView (
        DrawDocShell* pDocSh,
        OutputDevice* pOutDev,
        DrawViewShell* pShell);
    virtual ~DrawView();

    virtual void MarkListHasChanged() override;
    void CompleteRedraw(OutputDevice* pOutDev, const vcl::Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector = 0L) override;

    virtual bool SetAttributes(const SfxItemSet& rSet, bool bReplaceAll = false) override;

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

    void    BlockPageOrderChangedHint(bool bBlock);

    bool    SetStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr = false) override;
    virtual bool IsObjMarkable(SdrObject* pObj, SdrPageView* pPV) const override;

    virtual void MakeVisible(const Rectangle& rRect, vcl::Window& rWin) override;
    virtual void HideSdrPage() override; // SdrPageView* pPV);

    void    PresPaint(const vcl::Region& rRegion);

    virtual void DeleteMarked() override; // from SdrView
protected:
    virtual void ModelHasChanged() override;

private:
    DrawDocShell*   mpDocShell;
    DrawViewShell*  mpDrawViewShell;
    VclPtr<VirtualDevice> mpVDev;

    sal_uInt16          mnPOCHSmph; ///< for blocking PageOrderChangedHint
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
