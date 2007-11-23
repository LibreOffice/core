/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: opthtml.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 16:41:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

// include ---------------------------------------------------------------

#include <svtools/langtab.hxx>

#include "htmlcfg.hxx"
#include "opthtml.hxx"
#include "opthtml.hrc"
#include <svx/dialogs.hrc>
#include "helpid.hrc"
#include <svx/dialmgr.hxx>

// Umwandlung der Modi zu den Positionen in der Listbox
const USHORT aPosToExportArr[] =
{
    HTML_CFG_HTML32,
    HTML_CFG_MSIE_40,
    HTML_CFG_NS40,
    HTML_CFG_WRITER
};

//#define DEPRECATED_ENTRY  0xFFFF

const USHORT aExportToPosArr[] =
{
    0,  //HTML 3.2
    1,  //MS Internet Explorer 4.0
    3,  //StarWriter
    2   //Netscape Navigator 4.0
};
// -----------------------------------------------------------------------

OfaHtmlTabPage::OfaHtmlTabPage(Window* pParent, const SfxItemSet& rSet) :
        SfxTabPage( pParent, SVX_RES( RID_OFAPAGE_HTMLOPT ), rSet ),
    aFontSizeGB     ( this, SVX_RES( GB_FONTSIZE       ) ),
    aSize1FT        ( this, SVX_RES( FT_SIZE1          ) ),
    aSize1NF        ( this, SVX_RES( NF_SIZE1          ) ),
    aSize2FT        ( this, SVX_RES( FT_SIZE2          ) ),
    aSize2NF        ( this, SVX_RES( NF_SIZE2          ) ),
    aSize3FT        ( this, SVX_RES( FT_SIZE3          ) ),
    aSize3NF        ( this, SVX_RES( NF_SIZE3          ) ),
    aSize4FT        ( this, SVX_RES( FT_SIZE4          ) ),
    aSize4NF        ( this, SVX_RES( NF_SIZE4          ) ),
    aSize5FT        ( this, SVX_RES( FT_SIZE5          ) ),
    aSize5NF        ( this, SVX_RES( NF_SIZE5          ) ),
    aSize6FT        ( this, SVX_RES( FT_SIZE6          ) ),
    aSize6NF        ( this, SVX_RES( NF_SIZE6          ) ),
    aSize7FT        ( this, SVX_RES( FT_SIZE7          ) ),
    aSize7NF        ( this, SVX_RES( NF_SIZE7          ) ),
    aImportGB       ( this, SVX_RES( GB_IMPORT     ) ),
    aNumbersEnglishUSCB ( this, SVX_RES( CB_NUMBERS_ENGLISH_US ) ),
    aUnknownTagCB   ( this, SVX_RES( CB_UNKNOWN_TAGS ) ),
    aIgnoreFontNamesCB( this, SVX_RES( CB_IGNORE_FONTNAMES ) ),
    aExportGB       ( this, SVX_RES( GB_EXPORT       ) ),
    aExportLB       ( this, SVX_RES( LB_EXPORT       ) ),
    aStarBasicCB    ( this, SVX_RES( CB_STARBASIC    ) ),
    aStarBasicWarningCB(this, SVX_RES( CB_STARBASIC_WARNING    ) ),
    aPrintExtensionCB( this,SVX_RES(CB_PRINT_EXTENSION )),
    aSaveGrfLocalCB ( this, SVX_RES( CB_LOCAL_GRF    ) ),
    aCharSetFT      ( this, SVX_RES( FT_CHARSET      ) ),
    aCharSetLB      ( this, SVX_RES( LB_CHARSET      ) )

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

BOOL OfaHtmlTabPage::FillItemSet( SfxItemSet& )
{
    SvxHtmlOptions* pHtmlOpt = SvxHtmlOptions::Get();
    if(aSize1NF.GetSavedValue() != aSize1NF.GetText())
        pHtmlOpt->SetFontSize(0, (USHORT)aSize1NF.GetValue());
    if(aSize2NF.GetSavedValue() != aSize2NF.GetText())
        pHtmlOpt->SetFontSize(1, (USHORT)aSize2NF.GetValue());
    if(aSize3NF.GetSavedValue() != aSize3NF.GetText())
        pHtmlOpt->SetFontSize(2, (USHORT)aSize3NF.GetValue());
    if(aSize4NF.GetSavedValue() != aSize4NF.GetText())
        pHtmlOpt->SetFontSize(3, (USHORT)aSize4NF.GetValue());
    if(aSize5NF.GetSavedValue() != aSize5NF.GetText())
        pHtmlOpt->SetFontSize(4, (USHORT)aSize5NF.GetValue());
    if(aSize6NF.GetSavedValue() != aSize6NF.GetText())
        pHtmlOpt->SetFontSize(5, (USHORT)aSize6NF.GetValue());
    if(aSize7NF.GetSavedValue() != aSize7NF.GetText())
        pHtmlOpt->SetFontSize(6, (USHORT)aSize7NF.GetValue());

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

    return FALSE;
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
    USHORT nExport = pHtmlOpt->GetExportMode();
    if( nExport >= ( sizeof( aExportToPosArr ) / sizeof( USHORT ) ) )
        nExport = 4;    // default for bad config entry is NS 4.0
    USHORT nPosArr = aExportToPosArr[ nExport ];
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
    USHORT nExport = aPosToExportArr[ pBox->GetSelectEntryPos() ];
    switch( nExport )
    {
        case HTML_CFG_MSIE_40:
        case HTML_CFG_NS40  :
        case HTML_CFG_WRITER :
            aPrintExtensionCB.Enable(TRUE);
        break;
        default: aPrintExtensionCB.Enable(FALSE);
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


