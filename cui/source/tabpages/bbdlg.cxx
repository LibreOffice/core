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
#include <tools/shl.hxx>
#include <svx/dialogs.hrc>
#include <cuires.hrc>
#include "page.hrc"

#include "bbdlg.hxx"
#include "border.hxx"
#include "backgrnd.hxx"
#include <dialmgr.hxx>

// class SvxBorderBackgroundDlg ------------------------------------------

SvxBorderBackgroundDlg::SvxBorderBackgroundDlg( Window *pParent,
                                                const SfxItemSet& rCoreSet,
                                                sal_Bool bEnableSelector ) :

    SfxTabDialog( pParent, CUI_RES( RID_SVXDLG_BBDLG ), &rCoreSet ),
    bEnableBackgroundSelector( bEnableSelector )
{
    FreeResource();
    AddTabPage( RID_SVXPAGE_BORDER, SvxBorderTabPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_BACKGROUND, SvxBackgroundTabPage::Create, 0 );
}

// -----------------------------------------------------------------------

SvxBorderBackgroundDlg::~SvxBorderBackgroundDlg()
{
}

// -----------------------------------------------------------------------

void SvxBorderBackgroundDlg::PageCreated( sal_uInt16 nPageId, SfxTabPage& rTabPage )
{
    // Umschalten zwischen Farbe/Grafik ermoeglichen:

    if ( bEnableBackgroundSelector && (RID_SVXPAGE_BACKGROUND == nPageId) )
        ((SvxBackgroundTabPage&)rTabPage).ShowSelector( );
}


