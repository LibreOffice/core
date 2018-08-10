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

#ifndef INCLUDED_SD_SOURCE_UI_INC_PRLTEMPL_HXX
#define INCLUDED_SD_SOURCE_UI_INC_PRLTEMPL_HXX

#include <sfx2/tabdlg.hxx>
#include <svx/xtable.hxx>

#include <prlayout.hxx>

class SfxObjectShell;
class SfxStyleSheetBase;
class SfxStyleSheetBasePool;

/**
 * Template-Tab-Dialog
 */
class SdPresLayoutTemplateDlg : public SfxTabDialog
{
private:
    const SfxObjectShell*   mpDocShell;

    XColorListRef         pColorTab;
    XGradientListRef      pGradientList;
    XHatchListRef         pHatchingList;
    XBitmapListRef        pBitmapList;
    XPatternListRef       pPatternList;
    XDashListRef          pDashList;
    XLineEndListRef       pLineEndList;

    sal_uInt16            mnLine;
    sal_uInt16            mnArea;
    sal_uInt16            mnShadow;
    sal_uInt16            mnTransparency;
    sal_uInt16            mnFont;
    sal_uInt16            mnEffects;
    sal_uInt16            mnTextAtt;
    sal_uInt16            mnBackground;

    PresentationObjects ePO;

    virtual void        PageCreated( sal_uInt16 nId, SfxTabPage &rPage ) override;

    // for mapping with the new SvxNumBulletItem
    SfxItemSet aInputSet;
    std::unique_ptr<SfxItemSet> pOutSet;
    const SfxItemSet* pOrgSet;

    sal_uInt16 GetOutlineLevel() const;

    using SfxTabDialog::GetOutputItemSet;

public:
    SdPresLayoutTemplateDlg( SfxObjectShell const * pDocSh, vcl::Window* pParent, bool bBackground, SfxStyleSheetBase& rStyleBase, PresentationObjects ePO, SfxStyleSheetBasePool* pSSPool );
    virtual ~SdPresLayoutTemplateDlg() override;
    virtual void dispose() override;

    const SfxItemSet* GetOutputItemSet() const;
};

#endif // INCLUDED_SD_SOURCE_UI_INC_PRLTEMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
