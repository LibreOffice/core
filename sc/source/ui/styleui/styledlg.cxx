/*************************************************************************
 *
 *  $RCSfile: styledlg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-11-27 08:51:30 $
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
#include <svx/align.hxx>
#include <svx/backgrnd.hxx>
#include <svx/border.hxx>
#include <svx/chardlg.hxx>
#include <svx/numfmt.hxx>
#include <svx/page.hxx>
#include <sfx2/objsh.hxx>
#include <svtools/style.hxx>

#include "styledlg.hxx"
#include "tabpages.hxx"     // Zellvorlagen
#include "tphf.hxx"         // Seitenvorlage: Kopf-/Fusszeilen
#include "tptable.hxx"      // Seitenvorlage: Tabelle
#include "scresid.hxx"
#include "sc.hrc"
#include "styledlg.hrc"

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
    switch ( nRscId )
    {
        case RID_SCDLG_STYLES_PAR:  // Zellformatvorlagen
            {
                AddTabPage( TP_NUMBER, &SvxNumberFormatTabPage::Create, &SvxNumberFormatTabPage::GetRanges );
                AddTabPage( TP_FONT, &SvxCharNamePage::Create,          &SvxCharNamePage::GetRanges );
                AddTabPage( TP_FONTEFF, &SvxCharEffectsPage::Create,            &SvxCharEffectsPage::GetRanges );
                AddTabPage( TP_ALIGNMENT, &SvxAlignmentTabPage::Create, &SvxAlignmentTabPage::GetRanges );
                AddTabPage( TP_BORDER, &SvxBorderTabPage::Create,       &SvxBorderTabPage::GetRanges );
                AddTabPage( TP_BACKGROUND, &SvxBackgroundTabPage::Create,   &SvxBackgroundTabPage::GetRanges );
                AddTabPage( TP_PROTECTION, &ScTabPageProtection::Create,    &ScTabPageProtection::GetRanges );
            }
            break;

        case RID_SCDLG_STYLES_PAGE: // Seitenvorlagen
            {
                AddTabPage( TP_PAGE_STD, &SvxPageDescPage::Create,  &SvxPageDescPage::GetRanges );
                AddTabPage( TP_BORDER, &SvxBorderTabPage::Create,   &SvxBorderTabPage::GetRanges );
                AddTabPage( TP_BACKGROUND, &SvxBackgroundTabPage::Create,   &SvxBackgroundTabPage::GetRanges );
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

        switch ( nPageId )
        {
            case TP_NUMBER:
                {
                    const SfxPoolItem* pInfoItem
                        = pDocSh->GetItem( SID_ATTR_NUMBERFORMAT_INFO );

                    DBG_ASSERT( pInfoItem, "NumberInfoItem nicht gefunden!" );

                    ((SvxNumberFormatTabPage&)rTabPage).
                        SetNumberFormatList(
                            (const SvxNumberInfoItem&)*pInfoItem ) ;
                }
                break;

            case TP_FONT:
                {
                    const SfxPoolItem* pInfoItem
                        = pDocSh->GetItem( SID_ATTR_CHAR_FONTLIST );

                    DBG_ASSERT( pInfoItem, "FontListItem nicht gefunden!" );

                    ((SvxCharNamePage&)rTabPage).
                        SetFontList(
                            (const SvxFontListItem&)*pInfoItem );
                }
                break;

            default:
            break;
        }
    }
    else if ( nDlgRsc == RID_SCDLG_STYLES_PAGE )
    {
        switch ( nPageId )
        {
            case TP_PAGE_STD:
                ((SvxPageDescPage&)rTabPage).SetMode( SVX_PAGE_MODE_CENTER );
                break;

            case TP_PAGE_HEADER:
            case TP_PAGE_FOOTER:
                ((ScHFPage&)rTabPage).SetStyleDlg( this );
                ((ScHFPage&)rTabPage).SetPageStyle( GetStyleSheet().GetName() );
                ((ScHFPage&)rTabPage).DisableDeleteQueryBox();
                break;
            case TP_BACKGROUND:
                    if( nDlgRsc == RID_SCDLG_STYLES_PAGE)
                        ((SvxBackgroundTabPage&)rTabPage).ShowSelector();
                break;

            default:
                break;
        }
    }
}


// -----------------------------------------------------------------------

const SfxItemSet* __EXPORT ScStyleDlg::GetRefreshedSet()
{
    delete GetInputSetImpl();

    return new SfxItemSet( GetStyleSheet().GetItemSet() );
}



