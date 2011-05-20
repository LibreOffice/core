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

// include ---------------------------------------------------------------

#include <svtools/langtab.hxx>
#include <svtools/htmlcfg.hxx>
#include "opthtml.hxx"
#include "opthtml.hrc"
#include <cuires.hrc>
#include "helpid.hrc"
#include <dialmgr.hxx>
#include <sal/macros.h>

// Umwandlung der Modi zu den Positionen in der Listbox
const sal_uInt16 aPosToExportArr[] =
{
    HTML_CFG_HTML32,
    HTML_CFG_MSIE_40,
    HTML_CFG_NS40,
    HTML_CFG_WRITER
};

//#define DEPRECATED_ENTRY  0xFFFF

const sal_uInt16 aExportToPosArr[] =
{
    0,  //HTML 3.2
    1,  //MS Internet Explorer 4.0
    3,  //StarWriter
    2   //Netscape Navigator 4.0
};
// -----------------------------------------------------------------------

OfaHtmlTabPage::OfaHtmlTabPage(Window* pParent, const SfxItemSet& rSet) :
        SfxTabPage( pParent, CUI_RES( RID_OFAPAGE_HTMLOPT ), rSet ),
    aFontSizeGB     ( this, CUI_RES( GB_FONTSIZE       ) ),
    aSize1FT        ( this, CUI_RES( FT_SIZE1          ) ),
    aSize1NF        ( this, CUI_RES( NF_SIZE1          ) ),
    aSize2FT        ( this, CUI_RES( FT_SIZE2          ) ),
    aSize2NF        ( this, CUI_RES( NF_SIZE2          ) ),
    aSize3FT        ( this, CUI_RES( FT_SIZE3          ) ),
    aSize3NF        ( this, CUI_RES( NF_SIZE3          ) ),
    aSize4FT        ( this, CUI_RES( FT_SIZE4          ) ),
    aSize4NF        ( this, CUI_RES( NF_SIZE4          ) ),
    aSize5FT        ( this, CUI_RES( FT_SIZE5          ) ),
    aSize5NF        ( this, CUI_RES( NF_SIZE5          ) ),
    aSize6FT        ( this, CUI_RES( FT_SIZE6          ) ),
    aSize6NF        ( this, CUI_RES( NF_SIZE6          ) ),
    aSize7FT        ( this, CUI_RES( FT_SIZE7          ) ),
    aSize7NF        ( this, CUI_RES( NF_SIZE7          ) ),
    aImportGB       ( this, CUI_RES( GB_IMPORT     ) ),
    aNumbersEnglishUSCB ( this, CUI_RES( CB_NUMBERS_ENGLISH_US ) ),
    aUnknownTagCB   ( this, CUI_RES( CB_UNKNOWN_TAGS ) ),
    aIgnoreFontNamesCB( this, CUI_RES( CB_IGNORE_FONTNAMES ) ),
    aExportGB       ( this, CUI_RES( GB_EXPORT       ) ),
    aExportLB       ( this, CUI_RES( LB_EXPORT       ) ),
    aStarBasicCB    ( this, CUI_RES( CB_STARBASIC    ) ),
    aStarBasicWarningCB(this, CUI_RES( CB_STARBASIC_WARNING    ) ),
    aPrintExtensionCB( this,CUI_RES(CB_PRINT_EXTENSION )),
    aSaveGrfLocalCB ( this, CUI_RES( CB_LOCAL_GRF    ) ),
    aCharSetFT      ( this, CUI_RES( FT_CHARSET      ) ),
    aCharSetLB      ( this, CUI_RES( LB_CHARSET      ) )

{
    FreeResource();

    // replace placeholder with UI string from language list
    String aText( aNumbersEnglishUSCB.GetText());
    String aPlaceholder( RTL_CONSTASCII_USTRINGPARAM( "%ENGLISHUSLOCALE"));
    xub_StrLen nPos;
    if ((nPos = aText.Search( aPlaceholder)) != STRING_NOTFOUND)
    {
        SvtLanguageTable aLangTab;
        const String& rStr = aLangTab.GetString( LANGUAGE_ENGLISH_US);
        if (rStr.Len())
        {
            aText.Replace( nPos, aPlaceholder.Len(), rStr);
            aNumbersEnglishUSCB.SetText( aText);
        }
    }

    aExportLB.SetSelectHdl(LINK(this, OfaHtmlTabPage, ExportHdl_Impl));
    aStarBasicCB.SetClickHdl(LINK(this, OfaHtmlTabPage, CheckBoxHdl_Impl));

    // initialize the characterset listbox
    aCharSetLB.FillWithMimeAndSelectBest();
}

OfaHtmlTabPage::~OfaHtmlTabPage()
{
}

SfxTabPage* OfaHtmlTabPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet )
{
    return new OfaHtmlTabPage(pParent, rAttrSet);
}

sal_Bool OfaHtmlTabPage::FillItemSet( SfxItemSet& )
{
    SvxHtmlOptions& rHtmlOpt = SvxHtmlOptions::Get();
    if(aSize1NF.GetSavedValue() != aSize1NF.GetText())
        rHtmlOpt.SetFontSize(0, (sal_uInt16)aSize1NF.GetValue());
    if(aSize2NF.GetSavedValue() != aSize2NF.GetText())
        rHtmlOpt.SetFontSize(1, (sal_uInt16)aSize2NF.GetValue());
    if(aSize3NF.GetSavedValue() != aSize3NF.GetText())
        rHtmlOpt.SetFontSize(2, (sal_uInt16)aSize3NF.GetValue());
    if(aSize4NF.GetSavedValue() != aSize4NF.GetText())
        rHtmlOpt.SetFontSize(3, (sal_uInt16)aSize4NF.GetValue());
    if(aSize5NF.GetSavedValue() != aSize5NF.GetText())
        rHtmlOpt.SetFontSize(4, (sal_uInt16)aSize5NF.GetValue());
    if(aSize6NF.GetSavedValue() != aSize6NF.GetText())
        rHtmlOpt.SetFontSize(5, (sal_uInt16)aSize6NF.GetValue());
    if(aSize7NF.GetSavedValue() != aSize7NF.GetText())
        rHtmlOpt.SetFontSize(6, (sal_uInt16)aSize7NF.GetValue());

    if(aNumbersEnglishUSCB.IsChecked() != aNumbersEnglishUSCB.GetSavedValue())
        rHtmlOpt.SetNumbersEnglishUS(aNumbersEnglishUSCB.IsChecked());

    if(aUnknownTagCB.IsChecked() != aUnknownTagCB.GetSavedValue())
        rHtmlOpt.SetImportUnknown(aUnknownTagCB.IsChecked());

    if(aIgnoreFontNamesCB.IsChecked() != aIgnoreFontNamesCB.GetSavedValue())
        rHtmlOpt.SetIgnoreFontFamily(aIgnoreFontNamesCB.IsChecked());

    if(aExportLB.GetSelectEntryPos() != aExportLB.GetSavedValue())
        rHtmlOpt.SetExportMode(aPosToExportArr[aExportLB.GetSelectEntryPos()]);

    if(aStarBasicCB.IsChecked() != aStarBasicCB.GetSavedValue())
        rHtmlOpt.SetStarBasic(aStarBasicCB.IsChecked());

    if(aStarBasicWarningCB.IsChecked() != aStarBasicWarningCB.GetSavedValue())
        rHtmlOpt.SetStarBasicWarning(aStarBasicWarningCB.IsChecked());

    if(aSaveGrfLocalCB.IsChecked() != aSaveGrfLocalCB.GetSavedValue())
        rHtmlOpt.SetSaveGraphicsLocal(aSaveGrfLocalCB.IsChecked());

    if(aPrintExtensionCB.IsChecked() != aPrintExtensionCB.GetSavedValue())
        rHtmlOpt.SetPrintLayoutExtension(aPrintExtensionCB.IsChecked());

    if( aCharSetLB.GetSelectTextEncoding() != rHtmlOpt.GetTextEncoding() )
        rHtmlOpt.SetTextEncoding( aCharSetLB.GetSelectTextEncoding() );

    return sal_False;
}

void OfaHtmlTabPage::Reset( const SfxItemSet& )
{
    SvxHtmlOptions& rHtmlOpt = SvxHtmlOptions::Get();
    aSize1NF.SetValue(rHtmlOpt.GetFontSize(0));
    aSize2NF.SetValue(rHtmlOpt.GetFontSize(1));
    aSize3NF.SetValue(rHtmlOpt.GetFontSize(2));
    aSize4NF.SetValue(rHtmlOpt.GetFontSize(3));
    aSize5NF.SetValue(rHtmlOpt.GetFontSize(4));
    aSize6NF.SetValue(rHtmlOpt.GetFontSize(5));
    aSize7NF.SetValue(rHtmlOpt.GetFontSize(6));
    aNumbersEnglishUSCB.Check(rHtmlOpt.IsNumbersEnglishUS());
    aUnknownTagCB.Check(rHtmlOpt.IsImportUnknown());
    aIgnoreFontNamesCB.Check(rHtmlOpt.IsIgnoreFontFamily());
    sal_uInt16 nExport = rHtmlOpt.GetExportMode();
    if( nExport >= SAL_N_ELEMENTS( aExportToPosArr ) )
        nExport = 4;    // default for bad config entry is NS 4.0
    sal_uInt16 nPosArr = aExportToPosArr[ nExport ];
//  if( nPosArr == DEPRECATED_ENTRY )
//      nPosArr = aExportToPosArr[ 4 ];     // again: NS 4.0 is default
    aExportLB.SelectEntryPos( nPosArr );
    aExportLB.SaveValue();

    ExportHdl_Impl(&aExportLB);

    aStarBasicCB .Check(rHtmlOpt.IsStarBasic());
    aStarBasicWarningCB .Check(rHtmlOpt.IsStarBasicWarning());
    aStarBasicWarningCB.Enable(!aStarBasicCB.IsChecked());
    aSaveGrfLocalCB.Check(rHtmlOpt.IsSaveGraphicsLocal());
    aPrintExtensionCB.Check(rHtmlOpt.IsPrintLayoutExtension());

    aPrintExtensionCB.SaveValue();
    aStarBasicCB .SaveValue();
    aStarBasicWarningCB .SaveValue();
    aSaveGrfLocalCB.SaveValue();
    aSize1NF.SaveValue();
    aSize2NF.SaveValue();
    aSize3NF.SaveValue();
    aSize4NF.SaveValue();
    aSize5NF.SaveValue();
    aSize6NF.SaveValue();
    aSize7NF.SaveValue();
    aNumbersEnglishUSCB.SaveValue();
    aUnknownTagCB.SaveValue();
    aIgnoreFontNamesCB.SaveValue();

    if( !rHtmlOpt.IsDefaultTextEncoding() &&
        aCharSetLB.GetSelectTextEncoding() != rHtmlOpt.GetTextEncoding() )
        aCharSetLB.SelectTextEncoding( rHtmlOpt.GetTextEncoding() );
}

IMPL_LINK(OfaHtmlTabPage, ExportHdl_Impl, ListBox*, pBox)
{
    sal_uInt16 nExport = aPosToExportArr[ pBox->GetSelectEntryPos() ];
    switch( nExport )
    {
        case HTML_CFG_MSIE_40:
        case HTML_CFG_NS40  :
        case HTML_CFG_WRITER :
            aPrintExtensionCB.Enable(sal_True);
        break;
        default: aPrintExtensionCB.Enable(sal_False);
    }

    return 0;
}

IMPL_LINK(OfaHtmlTabPage, CheckBoxHdl_Impl, CheckBox*, pBox)
{
    aStarBasicWarningCB.Enable(!pBox->IsChecked());
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
