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
#ifndef INCLUDED_SVX_SOURCE_INC_TBXFORM_HXX
#define INCLUDED_SVX_SOURCE_INC_TBXFORM_HXX

#include <sfx2/tbxctrl.hxx>
#include <svtools/recorditemwindow.hxx>

class SvxFmAbsRecWin final : public RecordItemWindow
{
public:
    SvxFmAbsRecWin(vcl::Window* _pParent, SfxToolBoxControl* _pController);

private:
    virtual void PositionFired(sal_Int64 nRecord) override;

    SfxToolBoxControl* m_pController;
};

class SvxFmTbxCtlAbsRec : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxFmTbxCtlAbsRec(sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx);
    virtual ~SvxFmTbxCtlAbsRec() override;

    virtual VclPtr<InterimItemWindow> CreateItemWindow(vcl::Window* pParent) override;

    virtual void StateChanged(sal_uInt16 nSID, SfxItemState eState,
                              const SfxPoolItem* pState) override;
};

class SvxFmTbxCtlRecText : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxFmTbxCtlRecText(sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx);
    virtual ~SvxFmTbxCtlRecText() override;

    virtual VclPtr<InterimItemWindow> CreateItemWindow(vcl::Window* pParent) override;
};

class SvxFmTbxCtlRecFromText : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxFmTbxCtlRecFromText(sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx);
    virtual ~SvxFmTbxCtlRecFromText() override;

    virtual VclPtr<InterimItemWindow> CreateItemWindow(vcl::Window* pParent) override;
};

class LabelItemWindow;

class SvxFmTbxCtlRecTotal : public SfxToolBoxControl
{
    VclPtr<LabelItemWindow> m_xFixedText;

public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxFmTbxCtlRecTotal(sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx);
    virtual ~SvxFmTbxCtlRecTotal() override;

    virtual VclPtr<InterimItemWindow> CreateItemWindow(vcl::Window* pParent) override;
    virtual void StateChanged(sal_uInt16 nSID, SfxItemState eState,
                              const SfxPoolItem* pState) override;
};

class SvxFmTbxNextRec : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxFmTbxNextRec(sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx);
};

class SvxFmTbxPrevRec : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxFmTbxPrevRec(sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
