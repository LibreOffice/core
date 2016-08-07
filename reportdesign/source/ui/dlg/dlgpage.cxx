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


#include "precompiled_rptui.hxx"


#include <svx/dialogs.hrc>
#include <svx/tabarea.hxx>
#include <svx/flagsdef.hxx>
#include <svx/svxdlg.hxx>
#include <editeng/svxenum.hxx>
#include "dlgpage.hxx"
#include "ModuleHelper.hxx"
#include "RptResId.hrc"
#include <svl/intitem.hxx> //add CHINA001
#include <svl/cjkoptions.hxx>
#include <svl/aeitem.hxx>

namespace rptui
{
/*************************************************************************
|*
|* Konstruktor des Tab-Dialogs: Fuegt die Seiten zum Dialog hinzu
|*
\************************************************************************/

ORptPageDialog::ORptPageDialog( Window* pParent, const SfxItemSet* pAttr,sal_uInt16 _nPageId) :
SfxTabDialog ( pParent, ModuleRes( _nPageId ), pAttr ),
        rOutAttrs           ( *pAttr )
{
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "Dialogdiet fail!");
    switch( _nPageId )
    {
        case RID_PAGEDIALOG_BACKGROUND:
            AddTabPage( RID_SVXPAGE_BACKGROUND,String(ModuleRes(1)));
            break;
        case RID_PAGEDIALOG_PAGE:
            //AddTabPage( RID_SVXPAGE_PAGE,String(ModuleRes(1)));
   //         AddTabPage( RID_SVXPAGE_BACKGROUND,String(ModuleRes(1)));
            AddTabPage(RID_SVXPAGE_PAGE, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_PAGE ), 0 );
            AddTabPage(RID_SVXPAGE_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), 0 );
            break;
        case RID_PAGEDIALOG_CHAR:
            AddTabPage(RID_PAGE_CHAR, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), 0 );
            AddTabPage(RID_PAGE_EFFECTS, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), 0 );
            AddTabPage(RID_PAGE_POSITION, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_POSITION ), 0 );
            AddTabPage(RID_PAGE_TWOLN, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_TWOLINES ), 0 );
            AddTabPage(RID_PAGE_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), 0 );
            AddTabPage(RID_PAGE_ALIGNMENT, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_ALIGNMENT ), 0 );

            break;
        case RID_PAGEDIALOG_LINE:
            AddTabPage( RID_SVXPAGE_LINE,pFact->GetTabPageCreatorFunc( RID_SVXPAGE_LINE ), 0 );
            //AddTabPage( RID_SVXPAGE_LINE_DEF,pFact->GetTabPageCreatorFunc( RID_SVXPAGE_LINE_DEF ), 0 );
            //AddTabPage( RID_SVXPAGE_LINEEND_DEF,pFact->GetTabPageCreatorFunc( RID_SVXPAGE_LINEEND_DEF ), 0 );
            break;
        default:
            OSL_ENSURE(0,"Unknown page id");
    }

    SvtCJKOptions aCJKOptions;
    if ( !aCJKOptions.IsDoubleLinesEnabled() )
        RemoveTabPage(RID_PAGE_TWOLN);

    FreeResource();
}
// =============================================================================
} // namespace rptui
// =============================================================================
