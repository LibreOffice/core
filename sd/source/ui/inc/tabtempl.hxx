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


#ifndef SD_TABTEMPL_HXX
#define SD_TABTEMPL_HXX

#include <svx/tabarea.hxx>
#include <sfx2/styledlg.hxx>
class SdrModel;
class SfxObjectShell;
class SdrView;


/**
 * Template-Tab-Dialog
 */
class SdTabTemplateDlg : public SfxStyleDialog
{
private:
    const SfxObjectShell&   rDocShell;
    SdrView*                pSdrView;

    XColorListRef         pColorList;
    XGradientListRef      pGradientList;
    XHatchListRef         pHatchingList;
    XBitmapListRef        pBitmapList;
    XDashListRef          pDashList;
    XLineEndListRef       pLineEndList;

    sal_uInt16              nPageType;
    sal_uInt16              nDlgType;
    sal_uInt16              nPos;
    ChangeType          nColorTableState;
    ChangeType          nBitmapListState;
    ChangeType          nGradientListState;
    ChangeType          nHatchingListState;

    sal_uInt16 m_nLineId;
    sal_uInt16 m_nAreaId;
    sal_uInt16 m_nShadowId;
    sal_uInt16 m_nTransparencyId;
    sal_uInt16 m_nFontId;
    sal_uInt16 m_nFontEffectId;
    sal_uInt16 m_nIndentsId;
    sal_uInt16 m_nTextId;
    sal_uInt16 m_nAnimationId;
    sal_uInt16 m_nDimensionId;
    sal_uInt16 m_nConnectorId;
    sal_uInt16 m_nAlignId;
    sal_uInt16 m_nTabId;
    sal_uInt16 m_nAsianTypoId;

    virtual void                PageCreated( sal_uInt16 nId, SfxTabPage &rPage );
    virtual const SfxItemSet*   GetRefreshedSet();

public:
    SdTabTemplateDlg(Window* pParent,
            const SfxObjectShell* pDocShell,
            SfxStyleSheetBase& rStyleBase,
            SdrModel* pModel,
            SdrView* pView);
};


#endif // SD_TABTEMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
