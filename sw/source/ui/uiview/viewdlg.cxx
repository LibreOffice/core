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
            ASSERT(sal_False, "wrong dispatcher");
            return;
    }

    if( pDialog )
    {
        pDialog->Execute();
        delete pDialog;
    }
}



