/*************************************************************************
 *
 *  $RCSfile: tphf.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:04 $
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

#define _TPHF_CXX
#include "scitems.hxx"
#include <sfx2/basedlgs.hxx>
#include <svtools/style.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>

#include "tphf.hxx"
#include "sc.hrc"
#include "globstr.hrc"
#include "tabvwsh.hxx"
#include "viewdata.hxx"
#include "document.hxx"
#include "tphfedit.hxx"
#include "hfedtdlg.hxx"
#include "styledlg.hxx"
#include "scresid.hxx"

#undef _TPHF_CXX



//==================================================================
// class ScHFPage
//==================================================================

ScHFPage::ScHFPage( Window* pParent, USHORT nResId,
                    const SfxItemSet& rSet, USHORT nSetId )

    :   SvxHFPage   ( pParent, nResId, rSet, nSetId ),
        aBtnEdit    ( this, ScResId( RID_SCBTN_HFEDIT ) ),
        aDataSet    ( *rSet.GetPool(),
                       ATTR_PAGE_HEADERLEFT, ATTR_PAGE_FOOTERRIGHT,
                       ATTR_PAGE, ATTR_PAGE, 0 ),
        nPageUsage  ( (USHORT)SVX_PAGE_ALL ),
        pStyleDlg   ( NULL )
{
    SetExchangeSupport();

    SfxViewShell*   pSh = SfxViewShell::Current();
    ScTabViewShell* pViewSh = PTR_CAST(ScTabViewShell,pSh);
    Point           aPos( aBackgroundBtn.GetPosPixel() );

    aPos.Y() -= 8;
    aBackgroundBtn.SetPosPixel( aPos );

    aPos.Y() += aBackgroundBtn.GetSizePixel().Height();
    aPos.Y() += 4;
    aBtnEdit.SetPosPixel( aPos );
    aBtnEdit.Show();

    aDataSet.Put( rSet );

    if ( pViewSh )
    {
        ScViewData* pViewData = pViewSh->GetViewData();
        ScDocument* pDoc      = pViewData->GetDocument();

        aStrPageStyle = pDoc->GetPageStyle( pViewData->GetTabNo() );
    }

    aBtnEdit.SetClickHdl    ( LINK( this, ScHFPage, BtnHdl ) );
    aTurnOnBox.SetClickHdl  ( LINK( this, ScHFPage, TurnOnHdl ) );

    if ( nId == SID_ATTR_PAGE_HEADERSET )
        aBtnEdit.SetHelpId( HID_SC_HEADER_EDIT );
    else
        aBtnEdit.SetHelpId( HID_SC_FOOTER_EDIT );
}

//------------------------------------------------------------------

__EXPORT ScHFPage::~ScHFPage()
{
}

//------------------------------------------------------------------

void __EXPORT ScHFPage::Reset( const SfxItemSet& rSet )
{
    SvxHFPage::Reset( rSet );
    TurnOnHdl( 0 );
}

//------------------------------------------------------------------

BOOL __EXPORT ScHFPage::FillItemSet( SfxItemSet& rOutSet )
{
    BOOL bResult = SvxHFPage::FillItemSet( rOutSet );

    if ( nId == SID_ATTR_PAGE_HEADERSET )
    {
        rOutSet.Put( aDataSet.Get( ATTR_PAGE_HEADERLEFT ) );
        rOutSet.Put( aDataSet.Get( ATTR_PAGE_HEADERRIGHT ) );
    }
    else
    {
        rOutSet.Put( aDataSet.Get( ATTR_PAGE_FOOTERLEFT ) );
        rOutSet.Put( aDataSet.Get( ATTR_PAGE_FOOTERRIGHT ) );
    }

    return bResult;
}

//------------------------------------------------------------------

void __EXPORT ScHFPage::ActivatePage( const SfxItemSet& rSet )
{
    USHORT              nPageWhich = GetWhich( SID_ATTR_PAGE );
    const SvxPageItem&  rPageItem  = (const SvxPageItem&)
                                     rSet.Get(nPageWhich);

    nPageUsage = rPageItem.GetPageUsage();

    if ( pStyleDlg )
        aStrPageStyle = pStyleDlg->GetStyleSheet().GetName();

    aDataSet.Put( rSet.Get(ATTR_PAGE) );

    SvxHFPage::ActivatePage( rSet );
}

//------------------------------------------------------------------

int __EXPORT ScHFPage::DeactivatePage( SfxItemSet* pSet )
{
    if ( LEAVE_PAGE == SvxHFPage::DeactivatePage( pSet ) )
        if ( pSet )
            FillItemSet( *pSet );

    return LEAVE_PAGE;
}

//------------------------------------------------------------------
// Handler:
//------------------------------------------------------------------

IMPL_LINK( ScHFPage, TurnOnHdl, CheckBox*, EMPTYARG )
{
    SvxHFPage::TurnOnHdl( &aTurnOnBox );

    if ( aTurnOnBox.IsChecked() )
        aBtnEdit.Enable();
    else
        aBtnEdit.Disable();

    return NULL;
}


//------------------------------------------------------------------

IMPL_LINK( ScHFPage, BtnHdl, PushButton*, EMPTYARG )
{
    //  Wenn der Bearbeiten-Dialog direkt aus dem Click-Handler des Buttons
    //  aufgerufen wird, funktioniert im Bearbeiten-Dialog unter OS/2 das
    //  GrabFocus nicht (Bug #41805#).
    //  Mit dem neuen StarView sollte dieser Workaround wieder raus koennen!

    Application::PostUserEvent( LINK( this, ScHFPage, HFEditHdl ) );
    return 0;
}

IMPL_LINK( ScHFPage, HFEditHdl, void*, EMPTYARG )
{
    SfxViewShell*   pViewSh = SfxViewShell::Current();

    if ( !pViewSh )
    {
        DBG_ERROR( "Current ViewShell not found." );
        return NULL;
    }

    if (   aCntSharedBox.IsEnabled()
        && !aCntSharedBox.IsChecked() )
    {
        USHORT nResId = ( nId == SID_ATTR_PAGE_HEADERSET )
                            ? RID_SCDLG_HFED_HEADER
                            : RID_SCDLG_HFED_FOOTER;

        ScHFEditDlg* pDlg
             = new ScHFEditDlg( pViewSh->GetViewFrame(), this,
                                aDataSet, aStrPageStyle, nResId );

        if ( pDlg->Execute() == RET_OK )
        {
            aDataSet.Put( *pDlg->GetOutputItemSet() );
        }

        delete pDlg;
    }
    else
    {
        String              aText;
        SfxSingleTabDialog* pDlg
            = new SfxSingleTabDialog( pViewSh->GetViewFrame(), this,
                                      aDataSet, 42, FALSE );
        BOOL bRightPage =   aCntSharedBox.IsChecked()
                         || ( SVX_PAGE_LEFT != SvxPageUsage(nPageUsage) );

        if ( nId == SID_ATTR_PAGE_HEADERSET )
        {
            aText = ScGlobal::GetRscString( STR_PAGEHEADER );
            if ( bRightPage )
                pDlg->SetTabPage( ScRightHeaderEditPage::Create( pDlg, aDataSet ) );
            else
                pDlg->SetTabPage( ScLeftHeaderEditPage::Create( pDlg, aDataSet ) );
        }
        else
        {
            aText = ScGlobal::GetRscString( STR_PAGEFOOTER );
            if ( bRightPage )
                pDlg->SetTabPage( ScRightFooterEditPage::Create( pDlg, aDataSet ) );
            else
                pDlg->SetTabPage( ScLeftFooterEditPage::Create( pDlg, aDataSet ) );
        }

        SvxNumType eNumType = ((const SvxPageItem&)aDataSet.Get(ATTR_PAGE)).GetNumType();
        ((ScHFEditPage*)pDlg->GetTabPage())->SetNumType(eNumType);

        aText.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " (" ));
        aText += ScGlobal::GetRscString( STR_PAGESTYLE );
        aText.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ": " ));
        aText += aStrPageStyle;
        aText += ')';

        pDlg->SetText( aText );

        if ( pDlg->Execute() == RET_OK )
        {
            aDataSet.Put( *pDlg->GetOutputItemSet() );
        }

        delete pDlg;
    }

    return NULL;
}

//==================================================================
// class ScHeaderPage
//==================================================================

ScHeaderPage::ScHeaderPage( Window* pParent, const SfxItemSet& rSet )
    : ScHFPage( pParent, RID_SVXPAGE_HEADER, rSet, SID_ATTR_PAGE_HEADERSET )
{
}

//------------------------------------------------------------------

SfxTabPage* __EXPORT ScHeaderPage::Create( Window* pParent, const SfxItemSet& rCoreSet )
{
    return ( new ScHeaderPage( pParent, rCoreSet ) );
}

//------------------------------------------------------------------

USHORT* __EXPORT ScHeaderPage::GetRanges()
{
    return SvxHeaderPage::GetRanges();
}

//==================================================================
// class ScFooterPage
//==================================================================

ScFooterPage::ScFooterPage( Window* pParent, const SfxItemSet& rSet )
    : ScHFPage( pParent, RID_SVXPAGE_FOOTER, rSet, SID_ATTR_PAGE_FOOTERSET )
{
}

//------------------------------------------------------------------

SfxTabPage* __EXPORT ScFooterPage::Create( Window* pParent, const SfxItemSet& rCoreSet )
{
    return ( new ScFooterPage( pParent, rCoreSet ) );
}

//------------------------------------------------------------------

USHORT* __EXPORT ScFooterPage::GetRanges()
{
    return SvxHeaderPage::GetRanges();
}




