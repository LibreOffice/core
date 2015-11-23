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

#ifndef INCLUDED_SD_SOURCE_UI_INC_PRNTOPTS_HXX
#define INCLUDED_SD_SOURCE_UI_INC_PRNTOPTS_HXX

#include <vcl/group.hxx>

#include <vcl/button.hxx>
#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>

class SdModule;
class SdPrintOptions : public SfxTabPage
{
 friend class SdModule;

private:
    VclPtr<VclFrame>            m_pFrmContent;
    VclPtr<CheckBox>            m_pCbxDraw;
    VclPtr<CheckBox>            m_pCbxNotes;
    VclPtr<CheckBox>            m_pCbxHandout;
    VclPtr<CheckBox>            m_pCbxOutline;
    VclPtr<RadioButton>         m_pRbtColor;
    VclPtr<RadioButton>         m_pRbtGrayscale;
    VclPtr<RadioButton>         m_pRbtBlackWhite;
    VclPtr<CheckBox>            m_pCbxPagename;
    VclPtr<CheckBox>            m_pCbxDate;
    VclPtr<CheckBox>            m_pCbxTime;
    VclPtr<CheckBox>            m_pCbxHiddenPages;
    VclPtr<RadioButton>         m_pRbtDefault;
    VclPtr<RadioButton>         m_pRbtPagesize;
    VclPtr<RadioButton>         m_pRbtPagetile;
    VclPtr<RadioButton>         m_pRbtBooklet;
    VclPtr<CheckBox>            m_pCbxFront;
    VclPtr<CheckBox>            m_pCbxBack;
    VclPtr<CheckBox>            m_pCbxPaperbin;

    DECL_LINK_TYPED( ClickCheckboxHdl, Button*, void );
    DECL_LINK_TYPED( ClickBookletHdl, Button*, void );

    void updateControls();

    using OutputDevice::SetDrawMode;
public:
            SdPrintOptions( vcl::Window* pParent, const SfxItemSet& rInAttrs);
            virtual ~SdPrintOptions();
    virtual void dispose() override;

    static  VclPtr<SfxTabPage> Create( vcl::Window*, const SfxItemSet* );

    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;

    void         SetDrawMode();
    virtual void PageCreated(const SfxAllItemSet& aSet) override;
};

#endif // INCLUDED_SD_SOURCE_UI_INC_PRNTOPTS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
