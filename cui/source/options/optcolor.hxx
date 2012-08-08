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
#ifndef SVX_OPTCOLOR_HXX
#define SVX_OPTCOLOR_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>

namespace svtools {class EditableColorConfig;class EditableExtendedColorConfig;}
class ColorConfigCtrl_Impl;
class AbstractSvxNameDialog;
class SvxColorOptionsTabPage : public SfxTabPage
{
    using SfxTabPage::DeactivatePage;

    FixedLine               aColorSchemeFL;
    FixedText               aColorSchemeFT;
    ListBox                 aColorSchemeLB;
    PushButton              aSaveSchemePB;
    PushButton              aDeleteSchemePB;

    FixedLine               aCustomColorsFL;

    sal_Bool                    bFillItemSetCalled;

    svtools::EditableColorConfig* pColorConfig;
    svtools::EditableExtendedColorConfig* pExtColorConfig;
    ColorConfigCtrl_Impl*   pColorConfigCT;

    DECL_LINK(SchemeChangedHdl_Impl, ListBox*);
    DECL_LINK(SaveDeleteHdl_Impl, PushButton*);
    DECL_LINK(CheckNameHdl_Impl, AbstractSvxNameDialog*);
    void UpdateColorConfig();

public:
    SvxColorOptionsTabPage( Window* pParent, const SfxItemSet& rSet );
    ~SvxColorOptionsTabPage(  );

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    virtual int         DeactivatePage( SfxItemSet* pSet );
    virtual void        FillUserData();

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
