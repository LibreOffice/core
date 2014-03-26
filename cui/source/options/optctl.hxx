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
#ifndef INCLUDED_CUI_SOURCE_OPTIONS_OPTCTL_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_OPTCTL_HXX

#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <sfx2/tabdlg.hxx>

// class SvxCTLOptionsPage -----------------------------------------------------

class SvxCTLOptionsPage : public SfxTabPage
{
private:

    CheckBox*            m_pSequenceCheckingCB;
    CheckBox*            m_pRestrictedCB;
    CheckBox*            m_pTypeReplaceCB;

    RadioButton*         m_pMovementLogicalRB;
    RadioButton*         m_pMovementVisualRB;

    ListBox*             m_pNumeralsLB;

    DECL_LINK( SequenceCheckingCB_Hdl, void* );

    SvxCTLOptionsPage( Window* pParent, const SfxItemSet& rSet );

public:

    virtual ~SvxCTLOptionsPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );
    virtual bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

#endif // INCLUDED_CUI_SOURCE_OPTIONS_OPTCTL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
