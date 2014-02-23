/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <vcl/settings.hxx>
#include <unotools/viewoptions.hxx>
#include "svx/hyperdlg.hxx"
#include <svx/svxdlg.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfxsids.hrc>


//#                                                                      #
//# Childwindow-Wrapper-Class                                            #
//#                                                                      #


SFX_IMPL_CHILDWINDOW_WITHID(SvxHlinkDlgWrapper, SID_HYPERLINK_DIALOG)



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
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "Dialogdiet fail!");
    mpDlg = pFact->CreateSvxHpLinkDlg( _pParent, pBindings, SID_HYPERLINK_DIALOG );
    DBG_ASSERT(mpDlg, "Dialogdiet fail!");
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
