/*************************************************************************
 *
 *  $RCSfile: styledlg.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:04:27 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

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
#include <svtools/style.hxx>
#include <svtools/cjkoptions.hxx>

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
#ifndef _SFXINTITEM_HXX //CHINA001
#include <svtools/intitem.hxx> //CHINA001
#endif //CHINA001
#ifndef _SVX_FLSTITEM_HXX //CHINA001
#include <svx/flstitem.hxx> //CHINA001
#endif //CHINA001
#ifndef _AEITEM_HXX //CHINA001
#include <svtools/aeitem.hxx> //CHINA001
#endif //CHINA001
#include <svx/flagsdef.hxx> //CHINA001
//==================================================================

ScStyleDlg::ScStyleDlg( Window*             pParent,
                        SfxStyleSheetBase&  rStyleBase,
                        USHORT              nRscId )

    :   SfxStyleDialog  ( pParent,
                          ScResId( nRscId ),
                          rStyleBase,
                          FALSE ),
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

void __EXPORT ScStyleDlg::PageCreated( USHORT nPageId, SfxTabPage& rTabPage )
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
                aSet.Put (SfxAllEnumItem((const USHORT)SID_ENUM_PAGE_MODE, SVX_PAGE_MODE_CENTER)); //CHINA001
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



