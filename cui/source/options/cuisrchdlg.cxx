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
#include "precompiled_cui.hxx"

// include ---------------------------------------------------------------
#include <vcl/wrkwin.hxx>
#include <vcl/morebtn.hxx>
#include <vcl/msgbox.hxx>
#include <svl/slstitm.hxx>
#include <svl/itemiter.hxx>
#include <svl/style.hxx>
#include <unotools/searchopt.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/module.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/basedlgs.hxx>
#include <svl/cjkoptions.hxx>
#include <com/sun/star/i18n/TransliterationModules.hpp>

#define _CUI_SRCHDLG_CXX
#include "cuisrchdlg.hxx"

#include <cuires.hrc>

#define ITEMID_SETITEM      0

#include <svl/srchitem.hxx>
#include <svx/pageitem.hxx>
#include <dialmgr.hxx>
#include <svx/dlgutil.hxx>
#include <optjsearch.hxx>
#include <editeng/brshitem.hxx>
#include "backgrnd.hxx"


// class SvxJSearchOptionsDialog -----------------------------------------

SvxJSearchOptionsDialog::SvxJSearchOptionsDialog(
            Window *pParent,
            const SfxItemSet& rOptionsSet, sal_Int32 nInitialFlags ) :
    SfxSingleTabDialog  ( pParent, rOptionsSet, RID_SVXPAGE_JSEARCH_OPTIONS ),
    nInitialTlFlags( nInitialFlags )
{
    pPage = (SvxJSearchOptionsPage *)
                    SvxJSearchOptionsPage::Create( this, rOptionsSet );
    SetTabPage( pPage );    //! implicitly calls pPage->Reset(...)!
    pPage->EnableSaveOptions( sal_False );
}


SvxJSearchOptionsDialog::~SvxJSearchOptionsDialog()
{
    // pPage will be implicitly destroyed by the
    // SfxSingleTabDialog destructor
}


void SvxJSearchOptionsDialog::Activate()
{
    pPage->SetTransliterationFlags( nInitialTlFlags );
}


sal_Int32 SvxJSearchOptionsDialog::GetTransliterationFlags() const
{
    return pPage->GetTransliterationFlags();
}
