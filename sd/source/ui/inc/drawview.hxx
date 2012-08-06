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

#ifndef SD_DRAW_VIEW_HXX
#define SD_DRAW_VIEW_HXX

#include "View.hxx"


namespace sd {

class DrawDocShell;
class DrawViewShell;

/*************************************************************************
|*
|* Ableitung von ::sd::View; enthaelt auch einen Zeiger auf das Dokument
|*
\************************************************************************/

class DrawView : public ::sd::View
{
public:
    TYPEINFO();

    DrawView (
        DrawDocShell* pDocSh,
        OutputDevice* pOutDev,
        DrawViewShell* pShell);
    virtual ~DrawView (void);

    virtual void MarkListHasChanged();
    void CompleteRedraw(OutputDevice* pOutDev, const Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector = 0L);

    virtual sal_Bool SetAttributes(const SfxItemSet& rSet, sal_Bool bReplaceAll = sal_False);

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);

    void    BlockPageOrderChangedHint(sal_Bool bBlock);

    sal_Bool    SetStyleSheet(SfxStyleSheet* pStyleSheet, sal_Bool bDontRemoveHardAttr = sal_False);
    virtual sal_Bool IsObjMarkable(SdrObject* pObj, SdrPageView* pPV) const;

    virtual void MakeVisible(const Rectangle& rRect, ::Window& rWin);
    virtual void HideSdrPage(); // SdrPageView* pPV);

    void    PresPaint(const Region& rRegion);

    virtual void DeleteMarked(); // from SdrView
protected:
    virtual void ModelHasChanged();

private:
    DrawDocShell*   mpDocShell;
    DrawViewShell*  mpDrawViewShell;
    VirtualDevice*  mpVDev;

    sal_uInt16          mnPOCHSmph; // zum blockieren des PageOrderChangedHint
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
