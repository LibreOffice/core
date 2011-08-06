/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// === includes (declaration) ============================================
#include "svx/databaseregistrationui.hxx"

// === includes (project) ================================================
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>

// === includes (global) =================================================
#include <sfx2/app.hxx>
#include <svl/itemset.hxx>
#include <vcl/msgbox.hxx>


namespace svx
{
    sal_uInt16 administrateDatabaseRegistration( Window* _parentWindow )
    {
        sal_uInt16 nResult = RET_CANCEL;

        SfxItemSet aRegistrationItems( SFX_APP()->GetPool(), SID_SB_DB_REGISTER, SID_SB_DB_REGISTER, 0 );

        SvxAbstractDialogFactory* pDialogFactory = SvxAbstractDialogFactory::Create();
        ::std::auto_ptr< SfxAbstractDialog > pDialog;
        if ( pDialogFactory )
            pDialog.reset( pDialogFactory->CreateSfxDialog( _parentWindow, aRegistrationItems, NULL, RID_SFXPAGE_DBREGISTER ) );
        if ( pDialog.get() )
            nResult = pDialog->Execute();

        return nResult;
    }

}   // namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
