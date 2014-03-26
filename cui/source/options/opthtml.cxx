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

#include <svtools/langtab.hxx>
#include <svtools/htmlcfg.hxx>
#include "opthtml.hxx"
#include <cuires.hrc>
#include "helpid.hrc"
#include <dialmgr.hxx>
#include <sal/macros.h>

// modus conversion to the positions in the listbox
const sal_uInt16 aPosToExportArr[] =
{
    HTML_CFG_MSIE,
    HTML_CFG_NS40,
    HTML_CFG_WRITER
};

const sal_uInt16 aExportToPosArr[] =
{
    1,  //HTML 3.2 (removed, map to Netscape Navigator 4.0)
    0,  //MS Internet Explorer 4.0
    2,  //StarWriter
    1   //Netscape Navigator 4.0
};


OfaHtmlTabPage::OfaHtmlTabPage(Window* pParent, const SfxItemSet& rSet) :
SfxTabPage( pParent, "OptHtmlPage" , "cui/ui/opthtmlpage.ui", rSet )
{
    get(aSize1NF,"size1");
    get(aSize2NF,"size2");
    get(aSize3NF,"size3");
    get(aSize4NF,"size4");
    get(aSize5NF,"size5");
    get(aSize6NF,"size6");
    get(aSize7NF,"size7");
    get(aNumbersEnglishUSCB,"numbersenglishus");
    get(aUnknownTagCB,"unknowntag");
    get(aIgnoreFontNamesCB,"ignorefontnames");
    get(aExportLB,"export");
    get(aStarBasicCB,"starbasic");
    get(aStarBasicWarningCB,"starbasicwarning");
    get(aPrintExtensionCB,"printextension");
    get(aSaveGrfLocalCB,"savegrflocal");
    get(aCharSetLB,"charset");
    aCharSetLB->SetStyle(aCharSetLB->GetStyle() | WB_SORT);

    // replace placeholder with UI string from language list
    OUString aText( aNumbersEnglishUSCB->GetText());
    OUString aPlaceholder("%ENGLISHUSLOCALE");
    sal_Int32 nPos;
    if ((nPos = aText.indexOf( aPlaceholder)) != -1)
    {
        SvtLanguageTable aLangTab;
        const OUString& rStr = aLangTab.GetString( LANGUAGE_ENGLISH_US);
        if (!rStr.isEmpty())
        {
            aText = aText.replaceAt( nPos, aPlaceholder.getLength(), rStr);
            aNumbersEnglishUSCB->SetText( aText);
        }
    }

    aExportLB->SetSelectHdl(LINK(this, OfaHtmlTabPage, ExportHdl_Impl));
    aStarBasicCB->SetClickHdl(LINK(this, OfaHtmlTabPage, CheckBoxHdl_Impl));

    // initialize the characterset listbox
    aCharSetLB->FillWithMimeAndSelectBest();
}

OfaHtmlTabPage::~OfaHtmlTabPage()
{
}

SfxTabPage* OfaHtmlTabPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet )
{
    return new OfaHtmlTabPage(pParent, rAttrSet);
}

bool OfaHtmlTabPage::FillItemSet( SfxItemSet& )
{
    SvxHtmlOptions& rHtmlOpt = SvxHtmlOptions::Get();
    if(aSize1NF->GetSavedValue() != aSize1NF->GetText())
        rHtmlOpt.SetFontSize(0, (sal_uInt16)aSize1NF->GetValue());
    if(aSize2NF->GetSavedValue() != aSize2NF->GetText())
        rHtmlOpt.SetFontSize(1, (sal_uInt16)aSize2NF->GetValue());
    if(aSize3NF->GetSavedValue() != aSize3NF->GetText())
        rHtmlOpt.SetFontSize(2, (sal_uInt16)aSize3NF->GetValue());
    if(aSize4NF->GetSavedValue() != aSize4NF->GetText())
        rHtmlOpt.SetFontSize(3, (sal_uInt16)aSize4NF->GetValue());
    if(aSize5NF->GetSavedValue() != aSize5NF->GetText())
        rHtmlOpt.SetFontSize(4, (sal_uInt16)aSize5NF->GetValue());
    if(aSize6NF->GetSavedValue() != aSize6NF->GetText())
        rHtmlOpt.SetFontSize(5, (sal_uInt16)aSize6NF->GetValue());
    if(aSize7NF->GetSavedValue() != aSize7NF->GetText())
        rHtmlOpt.SetFontSize(6, (sal_uInt16)aSize7NF->GetValue());

    if(TriState(aNumbersEnglishUSCB->IsChecked()) != aNumbersEnglishUSCB->GetSavedValue())
        rHtmlOpt.SetNumbersEnglishUS(aNumbersEnglishUSCB->IsChecked());

    if(TriState(aUnknownTagCB->IsChecked()) != aUnknownTagCB->GetSavedValue())
        rHtmlOpt.SetImportUnknown(aUnknownTagCB->IsChecked());

    if(TriState(aIgnoreFontNamesCB->IsChecked()) != aIgnoreFontNamesCB->GetSavedValue())
        rHtmlOpt.SetIgnoreFontFamily(aIgnoreFontNamesCB->IsChecked());

    if(aExportLB->GetSelectEntryPos() != aExportLB->GetSavedValue())
        rHtmlOpt.SetExportMode(aPosToExportArr[aExportLB->GetSelectEntryPos()]);

    if(TriState(aStarBasicCB->IsChecked()) != aStarBasicCB->GetSavedValue())
        rHtmlOpt.SetStarBasic(aStarBasicCB->IsChecked());

    if(TriState(aStarBasicWarningCB->IsChecked()) != aStarBasicWarningCB->GetSavedValue())
        rHtmlOpt.SetStarBasicWarning(aStarBasicWarningCB->IsChecked());

    if(TriState(aSaveGrfLocalCB->IsChecked()) != aSaveGrfLocalCB->GetSavedValue())
        rHtmlOpt.SetSaveGraphicsLocal(aSaveGrfLocalCB->IsChecked());

    if(TriState(aPrintExtensionCB->IsChecked()) != aPrintExtensionCB->GetSavedValue())
        rHtmlOpt.SetPrintLayoutExtension(aPrintExtensionCB->IsChecked());

    if( aCharSetLB->GetSelectTextEncoding() != rHtmlOpt.GetTextEncoding() )
        rHtmlOpt.SetTextEncoding( aCharSetLB->GetSelectTextEncoding() );

    return false;
}

void OfaHtmlTabPage::Reset( const SfxItemSet& )
{
    SvxHtmlOptions& rHtmlOpt = SvxHtmlOptions::Get();
    aSize1NF->SetValue(rHtmlOpt.GetFontSize(0));
    aSize2NF->SetValue(rHtmlOpt.GetFontSize(1));
    aSize3NF->SetValue(rHtmlOpt.GetFontSize(2));
    aSize4NF->SetValue(rHtmlOpt.GetFontSize(3));
    aSize5NF->SetValue(rHtmlOpt.GetFontSize(4));
    aSize6NF->SetValue(rHtmlOpt.GetFontSize(5));
    aSize7NF->SetValue(rHtmlOpt.GetFontSize(6));
    aNumbersEnglishUSCB->Check(rHtmlOpt.IsNumbersEnglishUS());
    aUnknownTagCB->Check(rHtmlOpt.IsImportUnknown());
    aIgnoreFontNamesCB->Check(rHtmlOpt.IsIgnoreFontFamily());
    sal_uInt16 nExport = rHtmlOpt.GetExportMode();
    if( nExport >= SAL_N_ELEMENTS( aExportToPosArr ) )
        nExport = 3;    // default for bad config entry is NS 4.0
    sal_uInt16 nPosArr = aExportToPosArr[ nExport ];
    aExportLB->SelectEntryPos( nPosArr );
    aExportLB->SaveValue();

    ExportHdl_Impl(aExportLB);

    aStarBasicCB->Check(rHtmlOpt.IsStarBasic());
    aStarBasicWarningCB->Check(rHtmlOpt.IsStarBasicWarning());
    aStarBasicWarningCB->Enable(!aStarBasicCB->IsChecked());
    aSaveGrfLocalCB->Check(rHtmlOpt.IsSaveGraphicsLocal());
    aPrintExtensionCB->Check(rHtmlOpt.IsPrintLayoutExtension());

    aPrintExtensionCB->SaveValue();
    aStarBasicCB->SaveValue();
    aStarBasicWarningCB->SaveValue();
    aSaveGrfLocalCB->SaveValue();
    aSize1NF->SaveValue();
    aSize2NF->SaveValue();
    aSize3NF->SaveValue();
    aSize4NF->SaveValue();
    aSize5NF->SaveValue();
    aSize6NF->SaveValue();
    aSize7NF->SaveValue();
    aNumbersEnglishUSCB->SaveValue();
    aUnknownTagCB->SaveValue();
    aIgnoreFontNamesCB->SaveValue();

    if( !rHtmlOpt.IsDefaultTextEncoding() &&
        aCharSetLB->GetSelectTextEncoding() != rHtmlOpt.GetTextEncoding() )
        aCharSetLB->SelectTextEncoding( rHtmlOpt.GetTextEncoding() );
}

IMPL_LINK(OfaHtmlTabPage, ExportHdl_Impl, ListBox*, pBox)
{
    sal_uInt16 nExport = aPosToExportArr[ pBox->GetSelectEntryPos() ];
    switch( nExport )
    {
        case HTML_CFG_MSIE:
        case HTML_CFG_NS40  :
        case HTML_CFG_WRITER :
            aPrintExtensionCB->Enable(true);
        break;
        default: aPrintExtensionCB->Enable(false);
    }

    return 0;
}

IMPL_LINK(OfaHtmlTabPage, CheckBoxHdl_Impl, CheckBox*, pBox)
{
    aStarBasicWarningCB->Enable(!pBox->IsChecked());
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
