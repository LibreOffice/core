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
#pragma once
#if 1

#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/group.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/txencbox.hxx>

class OfaHtmlTabPage : public SfxTabPage
{
    FixedLine       aFontSizeGB;
    FixedText       aSize1FT;
    NumericField    aSize1NF;
    FixedText       aSize2FT;
    NumericField    aSize2NF;
    FixedText       aSize3FT;
    NumericField    aSize3NF;
    FixedText       aSize4FT;
    NumericField    aSize4NF;
    FixedText       aSize5FT;
    NumericField    aSize5NF;
    FixedText       aSize6FT;
    NumericField    aSize6NF;
    FixedText       aSize7FT;
    NumericField    aSize7NF;

    FixedLine       aImportGB;
    CheckBox        aNumbersEnglishUSCB;
    CheckBox        aUnknownTagCB;
    CheckBox        aIgnoreFontNamesCB;

    FixedLine       aExportGB;
    ListBox         aExportLB;
    CheckBox        aStarBasicCB;
    CheckBox        aStarBasicWarningCB;
    CheckBox        aPrintExtensionCB;
    CheckBox        aSaveGrfLocalCB;
    FixedText       aCharSetFT;
    SvxTextEncodingBox aCharSetLB;

    DECL_LINK(ExportHdl_Impl, ListBox*);
    DECL_LINK(CheckBoxHdl_Impl, CheckBox*);

    OfaHtmlTabPage(Window* pParent, const SfxItemSet& rSet);
    virtual ~OfaHtmlTabPage();
public:

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet );

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

};


#endif //



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
