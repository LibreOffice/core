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
#include <helpids.h>
#include <dialmgr.hxx>
#include <sal/macros.h>


OfaHtmlTabPage::OfaHtmlTabPage(vcl::Window* pParent, const SfxItemSet& rSet) :
SfxTabPage( pParent, "OptHtmlPage" , "cui/ui/opthtmlpage.ui", &rSet )
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
        const OUString& rStr = SvtLanguageTable::GetLanguageString( LANGUAGE_ENGLISH_US);
        if (!rStr.isEmpty())
        {
            aText = aText.replaceAt( nPos, aPlaceholder.getLength(), rStr);
            aNumbersEnglishUSCB->SetText( aText);
        }
    }

    aStarBasicCB->SetClickHdl(LINK(this, OfaHtmlTabPage, CheckBoxHdl_Impl));

    // initialize the characterset listbox
    aCharSetLB->FillWithMimeAndSelectBest();
}

OfaHtmlTabPage::~OfaHtmlTabPage()
{
    disposeOnce();
}

void OfaHtmlTabPage::dispose()
{
    aSize1NF.clear();
    aSize2NF.clear();
    aSize3NF.clear();
    aSize4NF.clear();
    aSize5NF.clear();
    aSize6NF.clear();
    aSize7NF.clear();
    aNumbersEnglishUSCB.clear();
    aUnknownTagCB.clear();
    aIgnoreFontNamesCB.clear();
    aStarBasicCB.clear();
    aStarBasicWarningCB.clear();
    aPrintExtensionCB.clear();
    aSaveGrfLocalCB.clear();
    aCharSetLB.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> OfaHtmlTabPage::Create( vcl::Window* pParent,
                                           const SfxItemSet* rAttrSet )
{
    return VclPtr<OfaHtmlTabPage>::Create(pParent, *rAttrSet);
}

bool OfaHtmlTabPage::FillItemSet( SfxItemSet* )
{
    SvxHtmlOptions& rHtmlOpt = SvxHtmlOptions::Get();
    if(aSize1NF->IsValueChangedFromSaved())
        rHtmlOpt.SetFontSize(0, static_cast<sal_uInt16>(aSize1NF->GetValue()));
    if(aSize2NF->IsValueChangedFromSaved())
        rHtmlOpt.SetFontSize(1, static_cast<sal_uInt16>(aSize2NF->GetValue()));
    if(aSize3NF->IsValueChangedFromSaved())
        rHtmlOpt.SetFontSize(2, static_cast<sal_uInt16>(aSize3NF->GetValue()));
    if(aSize4NF->IsValueChangedFromSaved())
        rHtmlOpt.SetFontSize(3, static_cast<sal_uInt16>(aSize4NF->GetValue()));
    if(aSize5NF->IsValueChangedFromSaved())
        rHtmlOpt.SetFontSize(4, static_cast<sal_uInt16>(aSize5NF->GetValue()));
    if(aSize6NF->IsValueChangedFromSaved())
        rHtmlOpt.SetFontSize(5, static_cast<sal_uInt16>(aSize6NF->GetValue()));
    if(aSize7NF->IsValueChangedFromSaved())
        rHtmlOpt.SetFontSize(6, static_cast<sal_uInt16>(aSize7NF->GetValue()));

    if(aNumbersEnglishUSCB->IsValueChangedFromSaved())
        rHtmlOpt.SetNumbersEnglishUS(aNumbersEnglishUSCB->IsChecked());

    if(aUnknownTagCB->IsValueChangedFromSaved())
        rHtmlOpt.SetImportUnknown(aUnknownTagCB->IsChecked());

    if(aIgnoreFontNamesCB->IsValueChangedFromSaved())
        rHtmlOpt.SetIgnoreFontFamily(aIgnoreFontNamesCB->IsChecked());

    if(aStarBasicCB->IsValueChangedFromSaved())
        rHtmlOpt.SetStarBasic(aStarBasicCB->IsChecked());

    if(aStarBasicWarningCB->IsValueChangedFromSaved())
        rHtmlOpt.SetStarBasicWarning(aStarBasicWarningCB->IsChecked());

    if(aSaveGrfLocalCB->IsValueChangedFromSaved())
        rHtmlOpt.SetSaveGraphicsLocal(aSaveGrfLocalCB->IsChecked());

    if(aPrintExtensionCB->IsValueChangedFromSaved())
        rHtmlOpt.SetPrintLayoutExtension(aPrintExtensionCB->IsChecked());

    if( aCharSetLB->GetSelectTextEncoding() != rHtmlOpt.GetTextEncoding() )
        rHtmlOpt.SetTextEncoding( aCharSetLB->GetSelectTextEncoding() );

    return false;
}

void OfaHtmlTabPage::Reset( const SfxItemSet* )
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

IMPL_LINK(OfaHtmlTabPage, CheckBoxHdl_Impl, Button*, pBox, void)
{
    aStarBasicWarningCB->Enable(!static_cast<CheckBox*>(pBox)->IsChecked());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
