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

SvxHlinkDlgWrapper::SvxHlinkDlgWrapper( vcl::Window* _pParent, sal_uInt16 nId,
                                                SfxBindings* pBindings,
                                                SfxChildWinInfo* pInfo ) :
    SfxChildWindow( _pParent, nId ),

    mpDlg( nullptr )

{
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "Dialog creation failed!");
    mpDlg = pFact->CreateSvxHpLinkDlg(_pParent, pBindings);
    DBG_ASSERT(mpDlg, "Dialog creation failed!");
    SetWindow( mpDlg->GetWindow() );
    SetVisible_Impl(false);

    vcl::Window* pTopWindow = nullptr;
    if ( pInfo->aSize.Width() != 0 && pInfo->aSize.Height() != 0 &&
            (nullptr != (pTopWindow = SfxGetpApp()->GetTopWindow())))
    {
        Size aParentSize( pTopWindow->GetSizePixel() );
        Size aDlgSize ( GetSizePixel () );

        if( aParentSize.Width() < pInfo->aPos.X() )
            pInfo->aPos.setX( aParentSize.Width()-aDlgSize.Width() < long(0.1*aParentSize.Width()) ?
                              long(0.1*aParentSize.Width()) : aParentSize.Width()-aDlgSize.Width() );
        if( aParentSize.Height() < pInfo->aPos. Y() )
            pInfo->aPos.setY( aParentSize.Height()-aDlgSize.Height() < long(0.1*aParentSize.Height()) ?
                              long(0.1*aParentSize.Height()) : aParentSize.Height()-aDlgSize.Height() );

        GetWindow()->SetPosPixel( pInfo->aPos );
    }

    SetHideNotDelete( true );
}

SfxChildWinInfo SvxHlinkDlgWrapper::GetInfo() const
{
    return SfxChildWindow::GetInfo();
}

bool SvxHlinkDlgWrapper::QueryClose()
{
    return !mpDlg || mpDlg->QueryClose();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
