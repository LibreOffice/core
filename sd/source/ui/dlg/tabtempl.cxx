/*************************************************************************
 *
 *  $RCSfile: tabtempl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:34 $
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

#include <svx/flstitem.hxx>

#pragma hdrstop

#ifndef _SVDMODEL_HXX
#include <svx/svdmodel.hxx>
#endif

#ifndef _EEITEMID_HXX //autogen
#include <svx/eeitemid.hxx>
#endif

#include <svx/dialogs.hrc>

#ifdef MAC
#ifdef ITEMID_FONTLIST
#undef ITEMID_FONTLIST
#endif
#endif
#define ITEMID_FONTLIST     SID_ATTR_CHAR_FONTLIST
#define ITEMID_ESCAPEMENT   SID_ATTR_CHAR_ESCAPEMENT
#define ITEMID_CASEMAP      SID_ATTR_CHAR_CASEMAP

#include <svx/chardlg.hxx>
#include <svx/paragrph.hxx>
#include <svx/tabstpge.hxx>


#ifndef _SVX_CONNECT_HXX //autogen
#include <svx/connect.hxx>
#endif
#ifndef _SVX_MEASURE_HXX //autogen
#include <svx/measure.hxx>
#endif
#ifndef _SVX_TEXTANIM_HXX //autogen
#include <svx/textanim.hxx>
#endif
#ifndef _SVX_TEXTATTR_HXX //autogen
#include <svx/textattr.hxx>
#endif
#ifndef _SVX_TAB_LINE_HXX //autogen
#include <svx/tabline.hxx>
#endif
#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#ifndef _SVX_TABSTPGE_HXX //autogen
#include <svx/tabstpge.hxx>
#endif
#ifndef _XTABLE_HXX
#include <svx/xtable.hxx>
#endif

#include "docshell.hxx"
#include "tabtempl.hxx"
#include "tabtempl.hrc"
#include "sdresid.hxx"
#include "dlg_char.hxx"
#include "paragr.hxx"

/*************************************************************************
|*
|* Konstruktor des Tab-Dialogs: Fuegt die Seiten zum Dialog hinzu
|*
\************************************************************************/

SdTabTemplateDlg::SdTabTemplateDlg( Window* pParent,
                                const SfxObjectShell* pDocShell,
                                SfxStyleSheetBase& rStyleBase,
                                SdrModel* pModel,
                                SdrView* pView ) :
        SfxStyleDialog      ( pParent, SdResId( TAB_TEMPLATE ), rStyleBase, FALSE ),
        pColorTab           ( pModel->GetColorTable() ),
        pDashList           ( pModel->GetDashList() ),
        pLineEndList        ( pModel->GetLineEndList() ),
        pGradientList       ( pModel->GetGradientList() ),
        pHatchingList       ( pModel->GetHatchList() ),
        pBitmapList         ( pModel->GetBitmapList() ),
        rDocShell           ( *pDocShell ),
        pSdrView            ( pView )
{
    FreeResource();

    // Listbox fuellen und Select-Handler ueberladen

    AddTabPage( RID_SVXPAGE_LINE, SvxLineTabPage::Create,
                                    SvxLineTabPage::GetRanges );
    AddTabPage( RID_SVXPAGE_AREA, SvxAreaTabPage::Create,
                                    SvxAreaTabPage::GetRanges );
    AddTabPage( RID_SVXPAGE_SHADOW, SvxShadowTabPage::Create,
                                    SvxShadowTabPage::GetRanges );
    AddTabPage( RID_SVXPAGE_TRANSPARENCE, SvxTransparenceTabPage::Create,
                                    SvxTransparenceTabPage::GetRanges );
    AddTabPage( RID_SVXPAGE_CHAR_STD, SvxCharStdPage::Create,
                                    SvxCharStdPage::GetRanges );
    AddTabPage( RID_SVXPAGE_STD_PARAGRAPH,
                    SvxStdParagraphTabPage::Create,
                    SvxStdParagraphTabPage::GetRanges );
    AddTabPage( RID_SVXPAGE_TEXTATTR, SvxTextAttrPage::Create,
                    SvxTextAttrPage::GetRanges );
    AddTabPage( RID_SVXPAGE_TEXTANIMATION, SvxTextAnimationPage::Create,
                    SvxTextAnimationPage::GetRanges );
    AddTabPage( RID_SVXPAGE_MEASURE, SvxMeasurePage::Create,
                    SvxMeasurePage::GetRanges );
    AddTabPage( RID_SVXPAGE_CONNECTION, SvxConnectionPage::Create,
                    SvxConnectionPage::GetRanges );

    AddTabPage( RID_SVXPAGE_ALIGN_PARAGRAPH, SvxParaAlignTabPage::Create,
                    SvxParaAlignTabPage::GetRanges );
    AddTabPage( RID_SVXPAGE_TABULATOR, SvxTabulatorTabPage::Create,
                    SvxTabulatorTabPage::GetRanges );


    nDlgType = 1;
    nPageType = 0;
    nPos = 0;

    nColorTableState = CT_NONE;
    nBitmapListState = CT_NONE;
    nGradientListState = CT_NONE;
    nHatchingListState = CT_NONE;
}

// -----------------------------------------------------------------------

SdTabTemplateDlg::~SdTabTemplateDlg()
{
}

// -----------------------------------------------------------------------

void __EXPORT SdTabTemplateDlg::PageCreated( USHORT nId, SfxTabPage &rPage )
{
    switch( nId )
    {
        case RID_SVXPAGE_LINE:
            ( (SvxLineTabPage&) rPage ).SetColorTable( pColorTab );
            ( (SvxLineTabPage&) rPage ).SetDashList( pDashList );
            ( (SvxLineTabPage&) rPage ).SetLineEndList( pLineEndList );
            //( (SvxLineTabPage&) rPage ).SetPageType( &nPageType );
            ( (SvxLineTabPage&) rPage ).SetDlgType( &nDlgType );
            //( (SvxLineTabPage&) rPage ).SetPos( &nPos );
            ( (SvxLineTabPage&) rPage ).Construct();
            // ( (SvxLineTabPage&) rPage ).ActivatePage();
        break;

        case RID_SVXPAGE_AREA:
            ( (SvxAreaTabPage&) rPage ).SetColorTable( pColorTab );
            ( (SvxAreaTabPage&) rPage ).SetGradientList( pGradientList );
            ( (SvxAreaTabPage&) rPage ).SetHatchingList( pHatchingList );
            ( (SvxAreaTabPage&) rPage ).SetBitmapList( pBitmapList );
            ( (SvxAreaTabPage&) rPage ).SetPageType( &nPageType );
            ( (SvxAreaTabPage&) rPage ).SetDlgType( &nDlgType );
            ( (SvxAreaTabPage&) rPage ).SetPos( &nPos );
            ( (SvxAreaTabPage&) rPage ).SetGrdChgd( &nGradientListState );
            ( (SvxAreaTabPage&) rPage ).SetHtchChgd( &nHatchingListState );
            ( (SvxAreaTabPage&) rPage ).SetBmpChgd( &nBitmapListState );
            ( (SvxAreaTabPage&) rPage ).SetColorChgd( &nColorTableState );
            ( (SvxAreaTabPage&) rPage ).Construct();
            // ( (SvxAreaTabPage&) rPage ).ActivatePage();
        break;

        case RID_SVXPAGE_SHADOW:
            ( (SvxShadowTabPage&) rPage ).SetColorTable( pColorTab );
            ( (SvxShadowTabPage&) rPage ).SetPageType( &nPageType );
            ( (SvxShadowTabPage&) rPage ).SetDlgType( &nDlgType );
            ( (SvxShadowTabPage&) rPage ).SetColorChgd( &nColorTableState );
            ( (SvxShadowTabPage&) rPage ).Construct();
        break;

        case RID_SVXPAGE_TRANSPARENCE:
            ( (SvxTransparenceTabPage&) rPage ).SetPageType( &nPageType );
            ( (SvxTransparenceTabPage&) rPage ).SetDlgType( &nDlgType );
            ( (SvxTransparenceTabPage&) rPage ).Construct();
        break;

        case RID_SVXPAGE_CHAR_STD:
        {
            SvxFontListItem aItem(*( (const SvxFontListItem*)
                ( rDocShell.GetItem( SID_ATTR_CHAR_FONTLIST) ) ) );

            ( (SvxCharStdPage&) rPage ).SetFontList( aItem );
        }
        break;

        case RID_SVXPAGE_STD_PARAGRAPH:
        break;

        case RID_SVXPAGE_TEXTATTR:
        {
            ( (SvxTextAttrPage&) rPage ).SetView( pSdrView );
            ( (SvxTextAttrPage&) rPage ).Construct();
        }
        break;

        case RID_SVXPAGE_TEXTANIMATION:
        break;

        case RID_SVXPAGE_MEASURE:
            ( (SvxMeasurePage&) rPage ).SetView( pSdrView );
            ( (SvxMeasurePage&) rPage ).Construct();
        break;

        case RID_SVXPAGE_CONNECTION:
        {
            ( (SvxConnectionPage&) rPage ).SetView( pSdrView );
            ( (SvxConnectionPage&) rPage ).Construct();
        }
        break;

        case RID_SVXPAGE_TABULATOR:
            ( (SvxTabulatorTabPage&) rPage ).
                    DisableControls( TABTYPE_ALL &~TABTYPE_LEFT |
                                     TABFILL_ALL &~TABFILL_NONE );
    }
}

// -----------------------------------------------------------------------

const SfxItemSet* __EXPORT SdTabTemplateDlg::GetRefreshedSet()
{
    delete GetInputSetImpl();

    return new SfxItemSet( GetStyleSheet().GetItemSet() );
}



