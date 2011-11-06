/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

// === includes (declaration) ============================================
#include "svx/databaseregistrationui.hxx"

// === includes (UNO) ====================================================
// === /includes (UNO) ===================================================

// === includes (project) ================================================
#include <svx/svxdlg.hxx>
#ifndef _SVX_DIALOGS_HRC
#include <svx/dialogs.hrc>
#endif
//#include "connpooloptions.hxx"
// === /includes (project) ===============================================

// === includes (global) =================================================
#include <sfx2/app.hxx>
#include <svl/itemset.hxx>
#include <vcl/msgbox.hxx>
// === /includes (global) ================================================

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
