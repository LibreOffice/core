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

#ifndef INCLUDED_CUI_SOURCE_OPTIONS_OPTCHART_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_OPTCHART_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <svtools/valueset.hxx>
#include <svx/dlgctrl.hxx>
#include <vcl/button.hxx>
#include <svx/xtable.hxx>

#include "cfgchart.hxx"

class SvxDefaultColorOptPage : public SfxTabPage
{

private:
    VclPtr<ColorLB>                m_pLbChartColors;
    VclPtr<ValueSet>               m_pValSetColorBox;
    VclPtr<PushButton>             m_pPBDefault;
    VclPtr<PushButton>             m_pPBAdd;
    VclPtr<PushButton>             m_pPBRemove;

    SvxChartOptions*        pChartOptions;
    SvxChartColorTableItem* pColorConfig;
    XColorListRef           pColorList;

    DECL_LINK_TYPED( ResetToDefaults, Button *, void );
    DECL_LINK_TYPED( AddChartColor, Button *, void );
    DECL_LINK_TYPED( RemoveChartColor, Button *, void );
    DECL_LINK_TYPED( ListClickedHdl, ListBox&, void );
    DECL_LINK_TYPED(BoxClickedHdl, ValueSet*, void);

    void FillColorBox();
    long GetColorIndex( const Color& rCol );

public:
    SvxDefaultColorOptPage( vcl::Window* pParent, const SfxItemSet& rInAttrs );
    virtual ~SvxDefaultColorOptPage();
    virtual void dispose() override;

    void    Construct();

    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent, const SfxItemSet* rInAttrs );
    virtual bool        FillItemSet( SfxItemSet* rOutAttrs ) override;
    virtual void        Reset( const SfxItemSet* rInAttrs ) override;
};

#endif // INCLUDED_CUI_SOURCE_OPTIONS_OPTCHART_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
