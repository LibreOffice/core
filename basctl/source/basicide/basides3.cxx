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


#include "baside2.hxx"
#include <baside3.hxx>
#include <localizationmgr.hxx>
#include <dlgedview.hxx>
#include <xmlscript/xmldlg_imexp.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace basctl
{

using namespace comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;

VclPtr<DialogWindow> Shell::CreateDlgWin( const ScriptDocument& rDocument, const OUString& rLibName, const OUString& rDlgName )
{
    bCreatingWindow = true;

    sal_uInt16 nKey = 0;
    VclPtr<DialogWindow> pWin;
    OUString aLibName( rLibName );
    OUString aDlgName( rDlgName );

    if ( aLibName.isEmpty() )
        aLibName = "Standard" ;

    rDocument.getOrCreateLibrary( E_DIALOGS, aLibName );

    if ( aDlgName.isEmpty() )
        aDlgName = rDocument.createObjectName( E_DIALOGS, aLibName );

    // maybe there's a suspended one?
    pWin = FindDlgWin( rDocument, aLibName, aDlgName, false, true );

    if ( !pWin )
    {
        try
        {
            Reference< io::XInputStreamProvider > xISP;
            if ( rDocument.hasDialog( aLibName, aDlgName ) )
                rDocument.getDialog( aLibName, aDlgName, xISP );
            else
                rDocument.createDialog( aLibName, aDlgName, xISP );

            if ( xISP.is() )
            {
                // create dialog model
                Reference< XComponentContext > xContext = comphelper::getProcessComponentContext();
                Reference< container::XNameContainer > xDialogModel( xContext->getServiceManager()->createInstanceWithContext
                    ( "com.sun.star.awt.UnoControlDialogModel", xContext ), UNO_QUERY );
                Reference< XInputStream > xInput( xISP->createInputStream() );
                ::xmlscript::importDialogModel( xInput, xDialogModel, xContext, rDocument.isDocument() ? rDocument.getDocument() : Reference< frame::XModel >() );
                LocalizationMgr::setStringResourceAtDialog( rDocument, rLibName, aDlgName, xDialogModel );

                // new dialog window
                if (!pDialogLayout)
                    pDialogLayout.reset(VclPtr<DialogWindowLayout>::Create(&GetViewFrame()->GetWindow(), *aObjectCatalog));
                pWin = VclPtr<DialogWindow>::Create(pDialogLayout.get(), rDocument, aLibName, aDlgName, xDialogModel);
                nKey = InsertWindowInTable( pWin );
            }
        }
        catch (const uno::Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("basctl.basicide");
        }
    }
    else
    {
        pWin->SetStatus( pWin->GetStatus() & ~BASWIN_SUSPENDED );
        nKey = GetWindowId( pWin );
        DBG_ASSERT( nKey, "CreateDlgWin: No Key - Window not found!" );
    }

    if( pWin )
    {
        pWin->GrabScrollBars( aHScrollBar.get(), aVScrollBar.get() );
        pTabBar->InsertPage( nKey, aDlgName );
        pTabBar->Sort();
        if ( !pCurWin )
            SetCurWindow( pWin, false, false );
    }

    bCreatingWindow = false;
    return pWin;
}

VclPtr<DialogWindow> Shell::FindDlgWin (
    ScriptDocument const& rDocument,
    OUString const& rLibName, OUString const& rName,
    bool bCreateIfNotExist, bool bFindSuspended
)
{
    if (VclPtr<BaseWindow> pWin = FindWindow(rDocument, rLibName, rName, TYPE_DIALOG, bFindSuspended))
        return static_cast<DialogWindow*>(pWin.get());
    return bCreateIfNotExist ? CreateDlgWin(rDocument, rLibName, rName) : nullptr;
}

sal_uInt16 Shell::GetWindowId(const BaseWindow* pWin) const
{
    for (auto const& window : aWindowTable)
        if ( window.second == pWin )
            return window.first;
    return 0;
}

SdrView* Shell::GetCurDlgView() const
{
    if (DialogWindow* pDCurWin = dynamic_cast<DialogWindow*>(pCurWin.get()))
        return &pDCurWin->GetView();
    else
        return nullptr;
}

// only if dialogue window above:
void Shell::ExecuteDialog( SfxRequest& rReq )
{
    if (pCurWin && (dynamic_cast<DialogWindow*>(pCurWin.get()) || rReq.GetSlot() == SID_IMPORT_DIALOG))
        pCurWin->ExecuteCommand(rReq);
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
