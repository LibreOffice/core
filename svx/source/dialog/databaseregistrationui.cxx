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

#include "svx/databaseregistrationui.hxx"

#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>

#include <sfx2/app.hxx>
#include <svl/itemset.hxx>
#include <vcl/msgbox.hxx>
#include <memory>

namespace svx
{
    sal_uInt16 administrateDatabaseRegistration( vcl::Window* _parentWindow )
    {
        sal_uInt16 nResult = RET_CANCEL;

        SfxItemSet aRegistrationItems( SfxGetpApp()->GetPool(), SID_SB_DB_REGISTER, SID_SB_DB_REGISTER, 0 );

        SvxAbstractDialogFactory* pDialogFactory = SvxAbstractDialogFactory::Create();
        std::unique_ptr< SfxAbstractDialog > pDialog;
        if ( pDialogFactory )
            pDialog.reset( pDialogFactory->CreateSfxDialog( _parentWindow, aRegistrationItems, NULL, RID_SFXPAGE_DBREGISTER ) );
        if ( pDialog.get() )
            nResult = pDialog->Execute();

        return nResult;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
