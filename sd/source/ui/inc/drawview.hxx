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

#include "View.hxx"

#include <sddllapi.h>

namespace sd {

class DrawDocShell;
class DrawViewShell;

/**
 * Derivative of ::sd::View; contains also a pointer to the document
 */
class SAL_DLLPUBLIC_RTTI DrawView : public ::sd::View
{
public:

    DrawView (
        DrawDocShell* pDocSh,
        OutputDevice* pOutDev,
        DrawViewShell* pShell);
    virtual ~DrawView() override;

    virtual void MarkListHasChanged() override;
    void CompleteRedraw(OutputDevice* pOutDev, const vcl::Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector = nullptr) override;

    virtual bool SetAttributes(const SfxItemSet& rSet, bool bReplaceAll = false, bool bSlide = false, bool bMaster = false) override;
    void SetMasterAttributes(SdrObject* pObject, const SdPage& rPage, SfxItemSet rSet, SfxStyleSheetBasePool* pStShPool, bool& bOk, bool bMaster, bool bSlide);

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

    void    BlockPageOrderChangedHint(bool bBlock);

    bool    SetStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr = false) override;

    virtual void MakeVisible(const ::tools::Rectangle& rRect, vcl::Window& rWin) override;
    virtual void HideSdrPage() override; // SdrPageView* pPV);

    virtual void DeleteMarked() override; // from SdrView
protected:
    virtual void ModelHasChanged() override;

private:
    DrawViewShell*  mpDrawViewShell;

    sal_uInt16          mnPOCHSmph; ///< for blocking PageOrderChangedHint
};

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
