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
#ifndef INCLUDED_SC_SOURCE_UI_INC_OPREDLIN_HXX
#define INCLUDED_SC_SOURCE_UI_INC_OPREDLIN_HXX

#include <sfx2/tabdlg.hxx>

#include <vcl/group.hxx>

#include <vcl/button.hxx>

#include <vcl/lstbox.hxx>

#include <vcl/field.hxx>

#include <vcl/fixed.hxx>
#include <svtools/ctrlbox.hxx>
#include <svx/fntctrl.hxx>
#include <svx/strarray.hxx>

class ScRedlineOptionsTabPage : public SfxTabPage
{
    VclPtr<SvxColorListBox>    m_pContentColorLB;
    VclPtr<SvxColorListBox>    m_pRemoveColorLB;
    VclPtr<SvxColorListBox>    m_pInsertColorLB;
    VclPtr<SvxColorListBox>    m_pMoveColorLB;

public:

    ScRedlineOptionsTabPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual ~ScRedlineOptionsTabPage() override;
    virtual void dispose() override;
    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent, const SfxItemSet* rAttrSet );

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
