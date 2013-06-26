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
#ifndef _SWCHARDLG_HXX
#define _SWCHARDLG_HXX
#include <sfx2/tabdlg.hxx>

#include <vcl/group.hxx>

#include <vcl/fixed.hxx>

#include <vcl/edit.hxx>

#include <vcl/combobox.hxx>

#include <vcl/button.hxx>

#include <vcl/lstbox.hxx>

class SwView;
class SvxMacroItem;

class SwCharDlg: public SfxTabDialog
{
    SwView&   m_rView;
    sal_uInt8 m_nDialogMode;

    sal_uInt16 m_nCharStdId;
    sal_uInt16 m_nCharExtId;
    sal_uInt16 m_nCharPosId;
    sal_uInt16 m_nCharTwoId;
    sal_uInt16 m_nCharUrlId;
    sal_uInt16 m_nCharBgdId;
    sal_uInt16 m_nCharBrdId;

public:
    SwCharDlg(Window* pParent, SwView& pVw, const SfxItemSet& rCoreSet,
              sal_uInt8 nDialogMode, const String* pFmtStr = 0);

    ~SwCharDlg();

    virtual void PageCreated( sal_uInt16 nId, SfxTabPage &rPage );
};

class SwCharURLPage : public SfxTabPage
{
    Edit*               m_pURLED;
    FixedText*          m_pTextFT;
    Edit*               m_pTextED;
    Edit*               m_pNameED;
    ComboBox*           m_pTargetFrmLB;
    PushButton*         m_pURLPB;
    PushButton*         m_pEventPB;
    ListBox*            m_pVisitedLB;
    ListBox*            m_pNotVisitedLB;

    VclContainer*       m_pCharStyleContainer;

    SvxMacroItem*       pINetItem;
    sal_Bool                bModified;

    DECL_LINK(InsertFileHdl, void *);
    DECL_LINK(EventHdl, void *);

public:
                        SwCharURLPage( Window* pParent,
                                           const SfxItemSet& rSet );

                        ~SwCharURLPage();
    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
