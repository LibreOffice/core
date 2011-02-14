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
#ifndef _SVX_OPTASIAN_HXX
#define _SVX_OPTASIAN_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/group.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/edit.hxx>
#include <svx/langbox.hxx>
struct SvxAsianLayoutPage_Impl;
class SvxAsianLayoutPage : public SfxTabPage
{
    FixedLine    aKerningGB;
    RadioButton aCharKerningRB;
    RadioButton aCharPunctKerningRB;

    FixedLine    aCharDistGB;
    RadioButton aNoCompressionRB;
    RadioButton aPunctCompressionRB;
    RadioButton aPunctKanaCompressionRB;

    FixedLine    aStartEndGB;

    FixedText       aLanguageFT;
    SvxLanguageBox  aLanguageLB;
    CheckBox        aStandardCB;

    FixedText   aStartFT;
    Edit        aStartED;
    FixedText   aEndFT;
    Edit        aEndED;
    FixedText   aHintFT;

    SvxAsianLayoutPage_Impl* pImpl;

    DECL_LINK(LanguageHdl, SvxLanguageBox*);
    DECL_LINK(ChangeStandardHdl, CheckBox*);
    DECL_LINK(ModifyHdl, Edit*);

    SvxAsianLayoutPage( Window* pParent, const SfxItemSet& rSet );
public:

    virtual ~SvxAsianLayoutPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );
    static sal_uInt16*      GetRanges();
    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

#endif

