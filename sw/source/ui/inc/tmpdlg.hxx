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
#ifndef _SWTMPDLG_HXX
#define _SWTMPDLG_HXX

#include <sfx2/styledlg.hxx>

class SfxItemSet;
class SwWrtShell;

/*--------------------------------------------------------------------
   Description:    the tab dialog carrier of TabPages
 --------------------------------------------------------------------*/
class SwTemplateDlg: public SfxStyleDialog
{

    sal_uInt16      nType;
    sal_uInt16      nHtmlMode;
    SwWrtShell*     pWrtShell;
    sal_Bool        bNewStyle;

    DECL_LINK( NumOptionsHdl, PushButton* );

    sal_uInt16 m_nIndentsId;
    sal_uInt16 m_nAlignId;
    sal_uInt16 m_nTextFlowId;
    sal_uInt16 m_nAsianTypo;
    sal_uInt16 m_nFontId;
    sal_uInt16 m_nFontEffectId;
    sal_uInt16 m_nPositionId;
    sal_uInt16 m_nAsianLayoutId;
    sal_uInt16 m_nTabId;
    sal_uInt16 m_nOutlineId;
    sal_uInt16 m_nDropCapsId;
    sal_uInt16 m_nBackgroundId;
    sal_uInt16 m_nBorderId;
    sal_uInt16 m_nConditionId;
    sal_uInt16 m_nTypeId;
    sal_uInt16 m_nOptionsId;
    sal_uInt16 m_nWrapId;
    sal_uInt16 m_nColumnId;
    sal_uInt16 m_nMacroId;
    sal_uInt16 m_nHeaderId;
    sal_uInt16 m_nFooterId;
    sal_uInt16 m_nPageId;
    sal_uInt16 m_nFootNoteId;
    sal_uInt16 m_nTextGridId;
    sal_uInt16 m_nSingleId;
    sal_uInt16 m_nBulletId;
    sal_uInt16 m_nNumId;
    sal_uInt16 m_nBmpId;
    sal_uInt16 m_nNumOptId;
    sal_uInt16 m_nNumPosId;

public:
    // @param sPage
    // Identifies name of page to open at by default
    SwTemplateDlg(  Window*             pParent,
                    SfxStyleSheetBase&  rBase,
                    sal_uInt16          nRegion,
                    OString             sPage = OString(),
                    SwWrtShell*         pActShell = 0,
                    bool                bNew = false );

    const SfxItemSet* GetRefreshedSet();

    virtual void PageCreated( sal_uInt16 nId, SfxTabPage &rPage );
    virtual short Ok();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
