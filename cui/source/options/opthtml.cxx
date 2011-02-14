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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cui.hxx"

// include ---------------------------------------------------------------

#include <svtools/langtab.hxx>
#include <svtools/htmlcfg.hxx>
#include "opthtml.hxx"
#include "opthtml.hrc"
#include <cuires.hrc>
#include "helpid.hrc"
#include <dialmgr.hxx>

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

/*-----------------02.09.96 13.47-------------------

--------------------------------------------------*/

OfaHtmlTabPage::~OfaHtmlTabPage()
{
}

/*-----------------02.09.96 13.47-------------------

--------------------------------------------------*/

SfxTabPage* OfaHtmlTabPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet )
{
    return new OfaHtmlTabPage(pParent, rAttrSet);
}

/*-----------------02.09.96 13.47-------------------

--------------------------------------------------*/

sal_Bool OfaHtmlTabPage::FillItemSet( SfxItemSet& )
{
    SvxHtmlOptions* pHtmlOpt = SvxHtmlOptions::Get();
    if(aSize1NF.GetSavedValue() != aSize1NF.GetText())
        pHtmlOpt->SetFontSize(0, (sal_uInt16)aSize1NF.GetValue());
    if(aSize2NF.GetSavedValue() != aSize2NF.GetText())
        pHtmlOpt->SetFontSize(1, (sal_uInt16)aSize2NF.GetValue());
    if(aSize3NF.GetSavedValue() != aSize3NF.GetText())
        pHtmlOpt->SetFontSize(2, (sal_uInt16)aSize3NF.GetValue());
    if(aSize4NF.GetSavedValue() != aSize4NF.GetText())
        pHtmlOpt->SetFontSize(3, (sal_uInt16)aSize4NF.GetValue());
    if(aSize5NF.GetSavedValue() != aSize5NF.GetText())
        pHtmlOpt->SetFontSize(4, (sal_uInt16)aSize5NF.GetValue());
    if(aSize6NF.GetSavedValue() != aSize6NF.GetText())
        pHtmlOpt->SetFontSize(5, (sal_uInt16)aSize6NF.GetValue());
    if(aSize7NF.GetSavedValue() != aSize7NF.GetText())
        pHtmlOpt->SetFontSize(6, (sal_uInt16)aSize7NF.GetValue());

    if(aNumbersEnglishUSCB.IsChecked() != aNumbersEnglishUSCB.GetSavedValue())
        pHtmlOpt->SetNumbersEnglishUS(aNumbersEnglishUSCB.IsChecked());

    if(aUnknownTagCB.IsChecked() != aUnknownTagCB.GetSavedValue())
        pHtmlOpt->SetImportUnknown(aUnknownTagCB.IsChecked());

    if(aIgnoreFontNamesCB.IsChecked() != aIgnoreFontNamesCB.GetSavedValue())
        pHtmlOpt->SetIgnoreFontFamily(aIgnoreFontNamesCB.IsChecked());

    if(aExportLB.GetSelectEntryPos() != aExportLB.GetSavedValue())
        pHtmlOpt->SetExportMode(aPosToExportArr[aExportLB.GetSelectEntryPos()]);

    if(aStarBasicCB.IsChecked() != aStarBasicCB.GetSavedValue())
        pHtmlOpt->SetStarBasic(aStarBasicCB.IsChecked());

    if(aStarBasicWarningCB.IsChecked() != aStarBasicWarningCB.GetSavedValue())
        pHtmlOpt->SetStarBasicWarning(aStarBasicWarningCB.IsChecked());

    if(aSaveGrfLocalCB.IsChecked() != aSaveGrfLocalCB.GetSavedValue())
        pHtmlOpt->SetSaveGraphicsLocal(aSaveGrfLocalCB.IsChecked());

    if(aPrintExtensionCB.IsChecked() != aPrintExtensionCB.GetSavedValue())
        pHtmlOpt->SetPrintLayoutExtension(aPrintExtensionCB.IsChecked());

    if( aCharSetLB.GetSelectTextEncoding() != pHtmlOpt->GetTextEncoding() )
        pHtmlOpt->SetTextEncoding( aCharSetLB.GetSelectTextEncoding() );

    return sal_False;
}

/*-----------------02.09.96 13.47-------------------

--------------------------------------------------*/

void OfaHtmlTabPage::Reset( const SfxItemSet& )
{
    SvxHtmlOptions* pHtmlOpt = SvxHtmlOptions::Get();
    aSize1NF.SetValue(pHtmlOpt->GetFontSize(0));
    aSize2NF.SetValue(pHtmlOpt->GetFontSize(1));
    aSize3NF.SetValue(pHtmlOpt->GetFontSize(2));
    aSize4NF.SetValue(pHtmlOpt->GetFontSize(3));
    aSize5NF.SetValue(pHtmlOpt->GetFontSize(4));
    aSize6NF.SetValue(pHtmlOpt->GetFontSize(5));
    aSize7NF.SetValue(pHtmlOpt->GetFontSize(6));
    aNumbersEnglishUSCB.Check(pHtmlOpt->IsNumbersEnglishUS());
    aUnknownTagCB.Check(pHtmlOpt->IsImportUnknown());
    aIgnoreFontNamesCB.Check(pHtmlOpt->IsIgnoreFontFamily());
    sal_uInt16 nExport = pHtmlOpt->GetExportMode();
    if( nExport >= ( sizeof( aExportToPosArr ) / sizeof( sal_uInt16 ) ) )
        nExport = 4;    // default for bad config entry is NS 4.0
    sal_uInt16 nPosArr = aExportToPosArr[ nExport ];
//  if( nPosArr == DEPRECATED_ENTRY )
//      nPosArr = aExportToPosArr[ 4 ];     // again: NS 4.0 is default
    aExportLB.SelectEntryPos( nPosArr );
    aExportLB.SaveValue();

    ExportHdl_Impl(&aExportLB);

    aStarBasicCB .Check(pHtmlOpt->IsStarBasic());
    aStarBasicWarningCB .Check(pHtmlOpt->IsStarBasicWarning());
    aStarBasicWarningCB.Enable(!aStarBasicCB.IsChecked());
    aSaveGrfLocalCB.Check(pHtmlOpt->IsSaveGraphicsLocal());
    aPrintExtensionCB.Check(pHtmlOpt->IsPrintLayoutExtension());

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

    if( !pHtmlOpt->IsDefaultTextEncoding() &&
        aCharSetLB.GetSelectTextEncoding() != pHtmlOpt->GetTextEncoding() )
        aCharSetLB.SelectTextEncoding( pHtmlOpt->GetTextEncoding() );
}

/*-----------------16.04.98 16:03-------------------

--------------------------------------------------*/
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
/* -----------------05.02.99 09:17-------------------
 *
 * --------------------------------------------------*/
IMPL_LINK(OfaHtmlTabPage, CheckBoxHdl_Impl, CheckBox*, pBox)
{
    aStarBasicWarningCB.Enable(!pBox->IsChecked());
    return 0;
}


