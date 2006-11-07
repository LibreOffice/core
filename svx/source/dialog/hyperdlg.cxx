/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hyperdlg.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: kz $ $Date: 2006-11-07 14:50:17 $
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

// include ---------------------------------------------------------------

#ifndef _SV_SETTINGS_HXX
#include <vcl/settings.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_VIEWOPTIONS_HXX
#include <svtools/viewoptions.hxx>
#endif
#ifndef _CUI_TAB_HYPERLINK_HXX //CHINA001
#include "cuihyperdlg.hxx"  //CHINA001
#endif   //CHINA001
#include "hyperdlg.hxx"
#include "svxdlg.hxx" //CHINA001

#include <sfx2/app.hxx>

#include "hyperdlg.hrc"


//########################################################################
//#                                                                      #
//# Childwindow-Wrapper-Class                                            #
//#                                                                      #
//########################################################################

SFX_IMPL_CHILDWINDOW(SvxHlinkDlgWrapper, SID_HYPERLINK_DIALOG)

// -----------------------------------------------------------------------

struct MyStruct
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xFrame;
    SfxChildWinFactory* pFact;
    sal_Bool                bHideNotDelete;
    sal_Bool                bVisible;
    sal_Bool                bHideAtToggle;
    SfxModule*          pContextModule;
    SfxWorkWindow*      pWorkWin;
};

SvxHlinkDlgWrapper::SvxHlinkDlgWrapper( Window* _pParent, USHORT nId,
                                                SfxBindings* pBindings,
                                                SfxChildWinInfo* pInfo ) :
    SfxChildWindow( _pParent, nId ),

    mpDlg( NULL )

{
    //CHINA001 pWindow = new SvxHpLinkDlg( _pParent, pBindings );
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "Dialogdiet fail!");//CHINA001
    mpDlg = pFact->CreateSvxHpLinkDlg( _pParent, pBindings, ResId(SID_HYPERLINK_DIALOG) );
    DBG_ASSERT(mpDlg, "Dialogdiet fail!");//CHINA001
    pWindow = mpDlg->GetWindow();
    ((MyStruct*)pImp)->bVisible = FALSE;

    if ( pInfo->aSize.Width() != 0 && pInfo->aSize.Height() != 0 )
    {
        Size aParentSize( SFX_APP()->GetTopWindow()->GetSizePixel() );
        Size aDlgSize ( GetSizePixel () );

        if( aParentSize.Width() < pInfo->aPos.X() )
            pInfo->aPos.setX( aParentSize.Width()-aDlgSize.Width() < long(0.1*aParentSize.Width()) ?
                              long(0.1*aParentSize.Width()) : aParentSize.Width()-aDlgSize.Width() );
        if( aParentSize.Height() < pInfo->aPos. Y() )
            pInfo->aPos.setY( aParentSize.Height()-aDlgSize.Height() < long(0.1*aParentSize.Height()) ?
                              long(0.1*aParentSize.Height()) : aParentSize.Height()-aDlgSize.Height() );

        pWindow->SetPosPixel( pInfo->aPos );
    }

    eChildAlignment = SFX_ALIGN_NOALIGNMENT;

    SetHideNotDelete( TRUE );
}

SfxChildWinInfo SvxHlinkDlgWrapper::GetInfo() const
{
    return SfxChildWindow::GetInfo();
}

sal_Bool SvxHlinkDlgWrapper::QueryClose()
{
    return mpDlg ? mpDlg->QueryClose() : sal_True;
}

