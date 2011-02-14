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
#include "precompiled_sw.hxx"



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
                sal_uInt16 nOldValue = pWrtShell->GetPageOffset();
                sal_uInt16 nPage, nLogPage;
                pWrtShell->GetPageNum( nPage, nLogPage,
                   pWrtShell->IsCrsrVisible(), sal_False);

                if(nValue != nOldValue || nValue != nLogPage)
                {
                    if(!nOldValue)
                        pWrtShell->SetNewPageOffset( nValue );
                    else
                        pWrtShell->SetPageOffset( nValue );
                }
            }
        }
        break;

        default:
            ASSERT(!this, falscher Dispatcher);
            return;
    }

    if( pDialog )
    {
        pDialog->Execute();
        delete pDialog;
    }
}



