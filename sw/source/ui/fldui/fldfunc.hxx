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
#ifndef _SWFLDFUNC_HXX
#define _SWFLDFUNC_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/group.hxx>
#include <vcl/edit.hxx>

#include "condedit.hxx"
#include "fldpage.hxx"
#include <actctrl.hxx>

class SwFldFuncPage : public SwFldPage
{
    FixedText       aTypeFT;
    ListBox         aTypeLB;
    ListBox         aSelectionLB;
    FixedText       aFormatFT;
    ListBox         aFormatLB;
    FixedText       aNameFT;
    ConditionEdit   aNameED;
    FixedText       aValueFT;
    Edit            aValueED;
    FixedText       aCond1FT;
    ConditionEdit   aCond1ED;
    FixedText       aCond2FT;
    ConditionEdit   aCond2ED;
    PushButton      aMacroBT;

    //controls of "Input list"
    FixedText       aListItemFT;
    ReturnActionEdit aListItemED;
    PushButton      aListAddPB;
    FixedText       aListItemsFT;
    ListBox         aListItemsLB;
    PushButton      aListRemovePB;
    PushButton      aListUpPB;
    PushButton      aListDownPB;
    FixedText       aListNameFT;
    Edit            aListNameED;

    String          sOldValueFT;
    String          sOldNameFT;

    sal_uLong           nOldFormat;
    bool            bDropDownLBChanged;

    DECL_LINK(TypeHdl, void *);
    DECL_LINK(SelectHdl, void * = 0);
    DECL_LINK(InsertMacroHdl, void *);
    DECL_LINK(ModifyHdl, void * = 0);
    DECL_LINK( ListModifyHdl, Control*);
    DECL_LINK( ListEnableHdl, void*);

    // select Macro
    DECL_LINK( MacroHdl, Button * );

    void                UpdateSubType();
    String              TurnMacroString(const String &rMacro);

protected:
    virtual sal_uInt16      GetGroup();

public:
                        SwFldFuncPage(Window* pParent, const SfxItemSet& rSet);

                        ~SwFldFuncPage();

    static SfxTabPage*  Create(Window* pParent, const SfxItemSet& rAttrSet);

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    virtual void        FillUserData();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
