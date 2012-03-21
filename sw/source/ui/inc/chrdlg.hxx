/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
    SwView&   rView;
    sal_Bool      bIsDrwTxtMode;

public:
    SwCharDlg(Window* pParent, SwView& pVw, const SfxItemSet& rCoreSet,
              const String* pFmtStr = 0, sal_Bool bIsDrwTxtDlg = sal_False);

    ~SwCharDlg();

    virtual void PageCreated( sal_uInt16 nId, SfxTabPage &rPage );
};

class SwCharURLPage : public SfxTabPage
{
    VclVBox m_aBox;
    VclGrid m_aGrid;

    FixedLine           aURLFL;

    FixedText           aURLFT;
    Edit                aURLED;
    FixedText           aTextFT;
    Edit                aTextED;
    FixedText           aNameFT;
    Edit                aNameED;
    FixedText           aTargetFrmFT;
    ComboBox            aTargetFrmLB;
    PushButton          aURLPB;
    PushButton          aEventPB;
    FixedLine           aStyleFL;
    FixedText           aVisitedFT;
    ListBox             aVisitedLB;
    FixedText           aNotVisitedFT;
    ListBox             aNotVisitedLB;

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
