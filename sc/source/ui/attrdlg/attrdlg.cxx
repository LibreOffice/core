/*************************************************************************
 *
 *  $RCSfile: attrdlg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-11-27 08:46:06 $
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

#ifndef PCH
#include "scitems.hxx"

#include <sfx2/objsh.hxx>
#include <svx/numfmt.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/chardlg.hxx>
#include <svx/align.hxx>
#include <svx/border.hxx>
#include <svx/backgrnd.hxx>
/*
#include <sbx.hxx>
#include <sfxdoc.hxx>

#include <dialogs.hrc>
#include <numfmt.hxx>
#include <chardlg.hxx>
#include <border.hxx>
#include <backgrnd.hxx>
//#include <dialogs.hxx>
*/
#endif


//#include <align.hxx>      // SvxAlignmentTabPage

#include "tabpages.hxx"
#include "attrdlg.hxx"
#include "scresid.hxx"
#include "attrdlg.hrc"

#pragma hdrstop

//==================================================================

ScAttrDlg::ScAttrDlg( SfxViewFrame*     pFrame,
                      Window*           pParent,
                      const SfxItemSet* pCellAttrs )

    :   SfxTabDialog( pFrame,
                      pParent,
                      ScResId( RID_SCDLG_ATTR ),
                      pCellAttrs )
{
    AddTabPage( TP_NUMBER,      SvxNumberFormatTabPage::Create, 0 );
    AddTabPage( TP_FONT,        SvxCharNamePage::Create,        0 );
    AddTabPage( TP_FONTEFF,     SvxCharEffectsPage::Create,     0 );
    AddTabPage( TP_ALIGNMENT,   SvxAlignmentTabPage::Create,    0 );
    AddTabPage( TP_BORDER,      SvxBorderTabPage::Create,       0 );
    AddTabPage( TP_BACKGROUND,  SvxBackgroundTabPage::Create,   0 );
    AddTabPage( TP_PROTECTION,  ScTabPageProtection::Create,    0 );
    FreeResource();
}

// -----------------------------------------------------------------------

__EXPORT ScAttrDlg::~ScAttrDlg()
{
}

// -----------------------------------------------------------------------

void __EXPORT ScAttrDlg::PageCreated( USHORT nPageId, SfxTabPage& rTabPage )
{
    SfxObjectShell* pDocSh = SfxObjectShell::Current();

    switch ( nPageId )
    {
        case TP_NUMBER:
        {
            SvxNumberFormatTabPage& rNumPage  = (SvxNumberFormatTabPage&)rTabPage;

            rNumPage.SetOkHdl( LINK( this, ScAttrDlg, OkHandler ) );
        }
        break;

        case TP_FONT:
        {
            const SfxPoolItem* pInfoItem = pDocSh->GetItem( SID_ATTR_CHAR_FONTLIST );

            DBG_ASSERT( pInfoItem, "FontListItem  not found :-(" );

            ((SvxCharNamePage&)rTabPage).
                SetFontList( *((const SvxFontListItem*)pInfoItem) );
        }
        break;

        default:
        break;
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( ScAttrDlg, OkHandler, void*, EMPTYARG )
{
    ((Link&)GetOKButton().GetClickHdl()).Call( NULL );

    return NULL;
}


