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


#include "hintids.hxx"
#include <sfx2/viewfrm.hxx>
#include <editeng/tstpitem.hxx>
#include <svl/stritem.hxx>
#include <sfx2/request.hxx>

#include "view.hxx"
#include "wrtsh.hxx"
#include "basesh.hxx"
#include "viewopt.hxx"
#include "uitool.hxx"
#include "cmdid.h"
#include <sfx2/tabdlg.hxx>

#include "pagedesc.hxx"


void SwView::ExecDlg(SfxRequest &rReq)
{
    ModalDialog *pDialog = 0;
    //Damit aus dem Basic keine Dialoge fuer Hintergrund-Views aufgerufen werden:
    const SfxPoolItem* pItem = 0;
    const SfxItemSet* pArgs = rReq.GetArgs();

    sal_uInt16 nSlot = rReq.GetSlot();
    if(pArgs)
        pArgs->GetItemState( GetPool().GetWhich(nSlot), sal_False, &pItem );

    switch ( nSlot )
    {
        case FN_CHANGE_PAGENUM:
        {
            if ( pItem )
            {
                sal_uInt16 nValue = ((SfxUInt16Item *)pItem)->GetValue();
                sal_uInt16 nOldValue = m_pWrtShell->GetPageOffset();
                sal_uInt16 nPage, nLogPage;
                m_pWrtShell->GetPageNum( nPage, nLogPage,
                   m_pWrtShell->IsCrsrVisible(), sal_False);

                if(nValue != nOldValue || nValue != nLogPage)
                {
                    if(!nOldValue)
                        m_pWrtShell->SetNewPageOffset( nValue );
                    else
                        m_pWrtShell->SetPageOffset( nValue );
                }
            }
        }
        break;

        default:
            OSL_ENSURE(!this, "wrong dispatcher");
            return;
    }

    if( pDialog )
    {
        pDialog->Execute();
        delete pDialog;
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
