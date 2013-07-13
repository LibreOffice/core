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

#ifndef _SVX_OPTCHART_HXX
#define _SVX_OPTCHART_HXX

// header for SfxTabPage
#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
// header for ValueSet
#include <svtools/valueset.hxx>
// header for ColorLB
#include <svx/dlgctrl.hxx>
// header for PushButton
#include <vcl/button.hxx>
// header for XColorList
#include <svx/xtable.hxx>

#include "cfgchart.hxx"

class SvxDefaultColorOptPage : public SfxTabPage
{

private:
    ColorLB*                m_pLbChartColors;
    ValueSet*               m_pValSetColorBox;
    PushButton*             m_pPBDefault;
    PushButton*             m_pPBAdd;
    PushButton*             m_pPBRemove;

    SvxChartOptions*        pChartOptions;
    SvxChartColorTableItem* pColorConfig;
    XColorListRef           pColorList;

    DECL_LINK( ResetToDefaults, void * );
    DECL_LINK( AddChartColor, void * );
    DECL_LINK( RemoveChartColor, PushButton * );
    DECL_LINK( ListClickedHdl, ColorLB * );
    DECL_LINK(BoxClickedHdl, void *);

    void FillColorBox();
    long GetColorIndex( const Color& rCol );

public:
    SvxDefaultColorOptPage( Window* pParent, const SfxItemSet& rInAttrs );
    virtual ~SvxDefaultColorOptPage();

    void    Construct();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rInAttrs );
    virtual sal_Bool        FillItemSet( SfxItemSet& rOutAttrs );
    virtual void        Reset( const SfxItemSet& rInAttrs );
};

#endif  // _SVX_OPTCHART_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
