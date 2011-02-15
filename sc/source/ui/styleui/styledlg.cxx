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
#include "precompiled_sc.hxx"

#undef SC_DLLIMPLEMENTATION



//------------------------------------------------------------------

#include "scitems.hxx"
//CHINA001 #include <svx/align.hxx>
//CHINA001 #include <svx/backgrnd.hxx>
//CHINA001 #include <svx/border.hxx>
//CHINA001 #include <svx/chardlg.hxx>
//CHINA001 #include <svx/numfmt.hxx>
#include <svx/numinf.hxx> //CHINA001
//CHINA001 #include <svx/page.hxx>
//CHINA001 #include <svx/paragrph.hxx>
#include <sfx2/objsh.hxx>
#include <svl/style.hxx>
#include <svl/cjkoptions.hxx>

#include "styledlg.hxx"
#include "tabpages.hxx"     // Zellvorlagen
#include "tphf.hxx"         // Seitenvorlage: Kopf-/Fusszeilen
#include "tptable.hxx"      // Seitenvorlage: Tabelle
#include "scresid.hxx"
#include "sc.hrc"
#include "styledlg.hrc"
#include <svx/svxdlg.hxx> //CHINA001
#include <svx/svxids.hrc> //CHINA001
#include <svx/dialogs.hrc> //CHINA001
#include <svl/intitem.hxx> //CHINA001
#include <editeng/flstitem.hxx> //CHINA001
#include <svl/aeitem.hxx> //CHINA001
#include <svx/flagsdef.hxx> //CHINA001
//==================================================================

ScStyleDlg::ScStyleDlg( Window*             pParent,
                        SfxStyleSheetBase&  rStyleBase,
                        sal_uInt16              nRscId )

    :   SfxStyleDialog  ( pParent,
                          ScResId( nRscId ),
                          rStyleBase,
                          sal_False ),
        nDlgRsc         ( nRscId )
{
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create(); //CHINA001
    DBG_ASSERT(pFact, "Dialogdiet fail!");//CHINA001
    switch ( nRscId )
    {
        case RID_SCDLG_STYLES_PAR:  // Zellformatvorlagen
            {
                SvtCJKOptions aCJKOptions;
                DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_NUMBERFORMAT ), "GetTabPageCreatorFunc fail!");//CHINA001
                DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_NUMBERFORMAT ), "GetTabPageRangesFunc fail!");//CHINA001
                AddTabPage( TP_NUMBER, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_NUMBERFORMAT ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_NUMBERFORMAT ) );  //CHINA001 AddTabPage( TP_NUMBER, &SvxNumberFormatTabPage::Create, &SvxNumberFormatTabPage::GetRanges );
                DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), "GetTabPageCreatorFunc fail!");//CHINA001
                DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_NAME ), "GetTabPageRangesFunc fail!");//CHINA001
                AddTabPage( TP_FONT, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_NAME ) ); //CHINA001 AddTabPage( TP_FONT, &SvxCharNamePage::Create,           &SvxCharNamePage::GetRanges );
                DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), "GetTabPageCreatorFunc fail!");//CHINA001
                DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_EFFECTS ), "GetTabPageRangesFunc fail!");//CHINA001
                AddTabPage( TP_FONTEFF, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_EFFECTS ) ); //CHINA001 AddTabPage( TP_FONTEFF, &SvxCharEffectsPage::Create,            &SvxCharEffectsPage::GetRanges );
                DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_ALIGNMENT ), "GetTabPageCreatorFunc fail!");//CHINA001
                DBG_ASSERT( pFact->GetTabPageRangesFunc( RID_SVXPAGE_ALIGNMENT ), "GetTabPageRangesFunc fail!");//CHINA001
                AddTabPage( TP_ALIGNMENT, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_ALIGNMENT ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_ALIGNMENT ) ); //CHINA001 AddTabPage( TP_ALIGNMENT, &SvxAlignmentTabPage::Create, &SvxAlignmentTabPage::GetRanges );
                if ( aCJKOptions.IsAsianTypographyEnabled() )
                {
                //CHINA001  AddTabPage( TP_ASIAN, &SvxAsianTabPage::Create,     &SvxAsianTabPage::GetRanges );

                    DBG_ASSERT(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN), "GetTabPageCreatorFunc fail!");//CHINA001
                    DBG_ASSERT(pFact->GetTabPageRangesFunc(RID_SVXPAGE_PARA_ASIAN), "GetTabPageRangesFunc fail!");//CHINA001
                    AddTabPage( TP_ASIAN,   pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN),       pFact->GetTabPageRangesFunc(RID_SVXPAGE_PARA_ASIAN) );
                }
                else
                    RemoveTabPage( TP_ASIAN );
                DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), "GetTabPageCreatorFunc fail!");//CHINA001
                DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ), "GetTabPageRangesFunc fail!");//CHINA001
                AddTabPage( TP_BORDER, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ) ); //CHINA001 AddTabPage( TP_BORDER, &SvxBorderTabPage::Create,        &SvxBorderTabPage::GetRanges );
                DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageCreatorFunc fail!");//CHINA001
                DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageRangesFunc fail!");//CHINA001
                AddTabPage( TP_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ) ); //CHINA001 AddTabPage( TP_BACKGROUND, &SvxBackgroundTabPage::Create,    &SvxBackgroundTabPage::GetRanges );
                AddTabPage( TP_PROTECTION, &ScTabPageProtection::Create,    &ScTabPageProtection::GetRanges );
            }
            break;

        case RID_SCDLG_STYLES_PAGE: // Seitenvorlagen
            {
                DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_PAGE ), "GetTabPageCreatorFunc fail!");//CHINA001
                DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_PAGE ), "GetTabPageRangesFunc fail!");//CHINA001
                AddTabPage( TP_PAGE_STD, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_PAGE ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_PAGE ) ); //CHINA001 AddTabPage( TP_PAGE_STD, &SvxPageDescPage::Create, &SvxPageDescPage::GetRanges );
                DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), "GetTabPageCreatorFunc fail!");//CHINA001
                DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ), "GetTabPageRangesFunc fail!");//CHINA001
                AddTabPage( TP_BORDER, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ) ); //CHINA001 AddTabPage( TP_BORDER, &SvxBorderTabPage::Create,    &SvxBorderTabPage::GetRanges );
                DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageCreatorFunc fail!");//CHINA001
                DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageRangesFunc fail!");//CHINA001
                AddTabPage( TP_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ) ); //CHINA001 AddTabPage( TP_BACKGROUND, &SvxBackgroundTabPage::Create,    &SvxBackgroundTabPage::GetRanges );
                AddTabPage( TP_PAGE_HEADER, &ScHeaderPage::Create,      &ScHeaderPage::GetRanges );
                AddTabPage( TP_PAGE_FOOTER, &ScFooterPage::Create,      &ScFooterPage::GetRanges );
                AddTabPage( TP_TABLE, &ScTablePage::Create,     &ScTablePage::GetRanges );
            }
            break;

        default:
            DBG_ERROR( "Family not supported" );
    }

    //--------------------------------------------------------------------
    FreeResource();
}

// -----------------------------------------------------------------------

__EXPORT ScStyleDlg::~ScStyleDlg()
{
}

// -----------------------------------------------------------------------

void __EXPORT ScStyleDlg::PageCreated( sal_uInt16 nPageId, SfxTabPage& rTabPage )
{
    if ( nDlgRsc == RID_SCDLG_STYLES_PAR )
    {
        SfxObjectShell* pDocSh = SfxObjectShell::Current();
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool())); //CHINA001
        switch ( nPageId )
        {
            case TP_NUMBER:
                {
                    const SfxPoolItem* pInfoItem
                        = pDocSh->GetItem( SID_ATTR_NUMBERFORMAT_INFO );

                    DBG_ASSERT( pInfoItem, "NumberInfoItem nicht gefunden!" );

                    //CHINA001 ((SvxNumberFormatTabPage&)rTabPage).
                        //CHINA001 SetNumberFormatList(
                            //CHINA001 (const SvxNumberInfoItem&)*pInfoItem ) ;
                    aSet.Put (SvxNumberInfoItem( (const SvxNumberInfoItem&)*pInfoItem ) );
                    rTabPage.PageCreated(aSet);
                }
                break;

            case TP_FONT:
                {
                    const SfxPoolItem* pInfoItem
                        = pDocSh->GetItem( SID_ATTR_CHAR_FONTLIST );

                    DBG_ASSERT( pInfoItem, "FontListItem nicht gefunden!" );

                    //CHINA001 ((SvxCharNamePage&)rTabPage).
                        //CHINA001 SetFontList(
                            //CHINA001 (const SvxFontListItem&)*pInfoItem );
                    aSet.Put (SvxFontListItem(((const SvxFontListItem&)*pInfoItem).GetFontList(), SID_ATTR_CHAR_FONTLIST));
                    rTabPage.PageCreated(aSet);
                }
                break;

            default:
            break;
        }
    }
    else if ( nDlgRsc == RID_SCDLG_STYLES_PAGE )
    {
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));//CHINA001
        switch ( nPageId )
        {
            case TP_PAGE_STD:
                //CHINA001 ((SvxPageDescPage&)rTabPage).SetMode( SVX_PAGE_MODE_CENTER );
                aSet.Put (SfxAllEnumItem((const sal_uInt16)SID_ENUM_PAGE_MODE, SVX_PAGE_MODE_CENTER)); //CHINA001
                rTabPage.PageCreated(aSet); //CHINA001
                break;

            case TP_PAGE_HEADER:
            case TP_PAGE_FOOTER:
                ((ScHFPage&)rTabPage).SetStyleDlg( this );
                ((ScHFPage&)rTabPage).SetPageStyle( GetStyleSheet().GetName() );
                ((ScHFPage&)rTabPage).DisableDeleteQueryBox();
                break;
            case TP_BACKGROUND:
                    if( nDlgRsc == RID_SCDLG_STYLES_PAGE)
                        //CHINA001 ((SvxBackgroundTabPage&)rTabPage).ShowSelector();
                    {   //add CHINA001
                        aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, SVX_SHOW_SELECTOR));
                        rTabPage.PageCreated(aSet);
                    }
                break;

            default:
                break;
        }
    }
}


// -----------------------------------------------------------------------

const SfxItemSet* __EXPORT ScStyleDlg::GetRefreshedSet()
{
    SfxItemSet* pItemSet = GetInputSetImpl();
    pItemSet->ClearItem();
    pItemSet->SetParent( GetStyleSheet().GetItemSet().GetParent() );
    return pItemSet;
}



