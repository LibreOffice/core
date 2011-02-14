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
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------
#include <vcl/settings.hxx>
#include <unotools/viewoptions.hxx>
#include "svx/hyperdlg.hxx"
#include <svx/svxdlg.hxx> //CHINA001
#include <sfx2/app.hxx>
#include <sfx2/sfxsids.hrc>

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

SvxHlinkDlgWrapper::SvxHlinkDlgWrapper( Window* _pParent, sal_uInt16 nId,
                                                SfxBindings* pBindings,
                                                SfxChildWinInfo* pInfo ) :
    SfxChildWindow( _pParent, nId ),

    mpDlg( NULL )

{
    //CHINA001 pWindow = new SvxHpLinkDlg( _pParent, pBindings );
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "Dialogdiet fail!");//CHINA001
    mpDlg = pFact->CreateSvxHpLinkDlg( _pParent, pBindings, SID_HYPERLINK_DIALOG );
    DBG_ASSERT(mpDlg, "Dialogdiet fail!");//CHINA001
    pWindow = mpDlg->GetWindow();
    ((MyStruct*)pImp)->bVisible = sal_False;

    Window* pTopWindow = 0;
    if ( pInfo->aSize.Width() != 0 && pInfo->aSize.Height() != 0 &&
            (0 != (pTopWindow = SFX_APP()->GetTopWindow())))
    {
        Size aParentSize( pTopWindow->GetSizePixel() );
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

    SetHideNotDelete( sal_True );
}

SfxChildWinInfo SvxHlinkDlgWrapper::GetInfo() const
{
    return SfxChildWindow::GetInfo();
}

sal_Bool SvxHlinkDlgWrapper::QueryClose()
{
    return mpDlg ? mpDlg->QueryClose() : sal_True;
}

