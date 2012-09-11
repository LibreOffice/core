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
#ifndef _SWFLDVAR_HXX
#define _SWFLDVAR_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/group.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/edit.hxx>

#include "fldpage.hxx"
#include "condedit.hxx"
#include "numfmtlb.hxx"

class SwFldVarPage;

class SelectionListBox : public ListBox
{
    sal_Bool            bCallAddSelection;

    virtual long        PreNotify( NotifyEvent& rNEvt );

public:
    SelectionListBox( SwFldVarPage* pDialog, const ResId& rResId );

    //  detect selection via Ctrl or Alt and evaluate with SelectHdl
    sal_Bool            IsCallAddSelection() const {return bCallAddSelection;}
    void            ResetCallAddSelection() {bCallAddSelection = sal_False;}
};

class SwFldVarPage : public SwFldPage
{
    friend class SelectionListBox;

    FixedText           aTypeFT;
    ListBox             aTypeLB;
    FixedText           aSelectionFT;
    SelectionListBox    aSelectionLB;
    FixedText           aNameFT;
    Edit                aNameED;
    FixedText           aValueFT;
    ConditionEdit       aValueED;
    FixedText           aFormatFT;
    NumFormatListBox    aNumFormatLB;
    ListBox             aFormatLB;
    FixedText           aChapterHeaderFT;
    FixedText           aChapterLevelFT;
    ListBox             aChapterLevelLB;
    CheckBox            aInvisibleCB;
    FixedText           aSeparatorFT;
    Edit                aSeparatorED;
    ToolBox             aNewDelTBX;

    String              sOldValueFT;
    String              sOldNameFT;

    sal_uLong               nOldFormat;
    sal_Bool                bInit;

    DECL_LINK(TypeHdl, void *);
    DECL_LINK( SubTypeHdl, ListBox* pLB = 0 );
    DECL_LINK(ModifyHdl, void * = 0);
    DECL_LINK( TBClickHdl, ToolBox *pTB = 0);
    DECL_LINK(ChapterHdl, void * = 0);
    DECL_LINK(SeparatorHdl, void * = 0);

    void                UpdateSubType();
    sal_uInt16              FillFormatLB(sal_uInt16 nTypeId);

protected:
    virtual sal_uInt16      GetGroup();

public:
                        SwFldVarPage(Window* pParent, const SfxItemSet& rSet);

                        ~SwFldVarPage();

    static SfxTabPage*  Create(Window* pParent, const SfxItemSet& rAttrSet);

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    virtual void        FillUserData();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
