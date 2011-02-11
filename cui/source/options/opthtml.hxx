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
#ifndef _OFA_OPTHTML_HXX
#define _OFA_OPTHTML_HXX

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



