/*************************************************************************
 *
 *  $RCSfile: attrdlg.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:01:48 $
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

#include "scitems.hxx"

#include <sfx2/objsh.hxx>
#include <sfx2/tabdlg.hxx>
//CHINA001 #include <svx/align.hxx>
//CHINA001 #include <svx/backgrnd.hxx>
//CHINA001 #include <svx/border.hxx>
//CHINA001 #include <svx/chardlg.hxx>
//CHINA001 #include <svx/numfmt.hxx>
//CHINA001 #include <svx/paragrph.hxx>
#include <svtools/cjkoptions.hxx>

#include "tabpages.hxx"
#include "attrdlg.hxx"
#include "scresid.hxx"
#include "attrdlg.hrc"
#include <svx/svxdlg.hxx> //CHINA001
#include <svx/dialogs.hrc> //CHINA001
#include <svx/flagsdef.hxx> //CHINA001
#ifndef _SVX_FLSTITEM_HXX //CHINA001
#include <svx/flstitem.hxx> //CHINA001
#endif //CHINA001
#include <sfx2/app.hxx> //CHINA001
//==================================================================

ScAttrDlg::ScAttrDlg( SfxViewFrame*     pFrame,
                      Window*           pParent,
                      const SfxItemSet* pCellAttrs )

    :   SfxTabDialog( pFrame,
                      pParent,
                      ScResId( RID_SCDLG_ATTR ),
                      pCellAttrs )
{
    SvtCJKOptions aCJKOptions;
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "Dialogdiet fail!");//CHINA001

    DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_NUMBERFORMAT ), "GetTabPageCreatorFunc fail!");//CHINA001
    AddTabPage( TP_NUMBER, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_NUMBERFORMAT ), 0 ); //CHINA001 AddTabPage( TP_NUMBER,     SvxNumberFormatTabPage::Create, 0 );
    DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), "GetTabPageCreatorFunc fail!");//CHINA001
    AddTabPage( TP_FONT, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), 0 ); //CHINA001 AddTabPage( TP_FONT,        SvxCharNamePage::Create,        0 );
    DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), "GetTabPageCreatorFunc fail!");//CHINA001
    AddTabPage( TP_FONTEFF, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), 0 ); //CHINA001 AddTabPage( TP_FONTEFF,       SvxCharEffectsPage::Create,     0 );
    DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_ALIGNMENT ), "GetTabPageCreatorFunc fail!");//CHINA001
    AddTabPage( TP_ALIGNMENT, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_ALIGNMENT ),    0 ); //CHINA001 AddTabPage( TP_ALIGNMENT,   SvxAlignmentTabPage::Create,    0 );

    if ( aCJKOptions.IsAsianTypographyEnabled() )
    {
        //CHINA001 AddTabPage( TP_ASIAN,    SvxAsianTabPage::Create,        0 );
        ::CreateTabPage pCreateTabpage = pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN);
        DBG_ASSERT(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN), "GetTabPageCreatorFunc fail!");//CHINA001
        AddTabPage( TP_ASIAN,   pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN),       0 );
    }
    else
        RemoveTabPage( TP_ASIAN );
    DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), "GetTabPageCreatorFunc fail!");//CHINA001
    AddTabPage( TP_BORDER,      pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ),     0 ); //CHINA001 AddTabPage( TP_BORDER,      SvxBorderTabPage::Create,       0 );
    DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageCreatorFunc fail!");//CHINA001
    AddTabPage( TP_BACKGROUND,  pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), 0 ); //CHINA001 AddTabPage( TP_BACKGROUND,  SvxBackgroundTabPage::Create,   0 );
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
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool())); //CHINA001
    switch ( nPageId )
    {
        case TP_NUMBER:
        {
            //CHINA001 SvxNumberFormatTabPage& rNumPage  = (SvxNumberFormatTabPage&)rTabPage;

            //CHINA001 rNumPage.SetOkHdl( LINK( this, ScAttrDlg, OkHandler ) );
            aSet.Put (SfxLinkItem( SID_LINK_TYPE, LINK( this, ScAttrDlg, OkHandler )));
            rTabPage.PageCreated(aSet);
        }
        break;

        case TP_FONT:
        {
            const SfxPoolItem* pInfoItem = pDocSh->GetItem( SID_ATTR_CHAR_FONTLIST );

            DBG_ASSERT( pInfoItem, "FontListItem  not found :-(" );

            //CHINA001 ((SvxCharNamePage&)rTabPage).
                //CHINA001 SetFontList( *((const SvxFontListItem*)pInfoItem) );
            aSet.Put (SvxFontListItem(((const SvxFontListItem*)pInfoItem)->GetFontList(), SID_ATTR_CHAR_FONTLIST ));
            rTabPage.PageCreated(aSet);
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

    return 0;
}


