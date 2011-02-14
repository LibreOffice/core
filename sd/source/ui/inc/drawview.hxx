/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SD_DRAW_VIEW_HXX
#define SD_DRAW_VIEW_HXX

#include "View.hxx"


namespace sd {

class DrawDocShell;
class DrawViewShell;
class FuSlideShow;
class SlideShow;

#define SDDRAWVIEW_MAGIC  0x456789BA

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
    friend class DrawViewRedirector;

    DrawDocShell*   mpDocShell;
    DrawViewShell*  mpDrawViewShell;
    VirtualDevice*  mpVDev;

    sal_uInt16          mnPOCHSmph; // zum blockieren des PageOrderChangedHint
};

} // end of namespace sd

#endif
