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



#include "cmdid.h"
#include "uiitems.hxx"
#include <tools/list.hxx>
#include <vcl/window.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/stritem.hxx>
#include <rsc/rscsfx.hxx>


#include "errhdl.hxx"
#include "view.hxx"
#include "wrtsh.hxx"
#include "basesh.hxx"


void SwView::ExecColl(SfxRequest &rReq)
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem = 0;
    sal_uInt16 nWhich = rReq.GetSlot();
    switch( nWhich )
    {
        case FN_SET_PAGE:
        {
            DBG_ASSERT(sal_False, "Not implemented");
        }
        break;
        case FN_SET_PAGE_STYLE:
        {
            if( pArgs )
            {
                if (pArgs &&
                    SFX_ITEM_SET == pArgs->GetItemState( nWhich , sal_True, &pItem ))
                {
                    if( ((SfxStringItem*)pItem)->GetValue() !=
                                            GetWrtShell().GetCurPageStyle(sal_False) )
                    {
                        SfxStringItem aName(SID_STYLE_APPLY,
                                   ((SfxStringItem*)pItem)->GetValue());
                        SfxUInt16Item aFamItem( SID_STYLE_FAMILY,
                                            SFX_STYLE_FAMILY_PAGE);
                        SwPtrItem aShell(FN_PARAM_WRTSHELL, GetWrtShellPtr());
                        SfxRequest aReq(SID_STYLE_APPLY, 0, GetPool());
                        aReq.AppendItem(aName);
                        aReq.AppendItem(aFamItem);
                        aReq.AppendItem(aShell);
                        GetCurShell()->ExecuteSlot(aReq);
                    }
                }
            }
            else
            {
                SfxRequest aReq(FN_FORMAT_PAGE_DLG, 0, GetPool());
                GetCurShell()->ExecuteSlot(aReq);
            }
        }
        break;
        default:
            ASSERT(sal_False, falscher CommandProcessor fuer Dispatch);
            return;
    }
}



