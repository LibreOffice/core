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



#if !TEST_LAYOUT
// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"
#endif /*! TEST_LAYOUT */

#undef SC_DLLIMPLEMENTATION



#include "tpsort.hxx"
#include "sortdlg.hxx"
#include "scresid.hxx"
#include "sortdlg.hrc"

#if !LAYOUT_SFX_TABDIALOG_BROKEN
#include <layout/layout-pre.hxx>
#endif

ScSortDlg::ScSortDlg( Window*           pParent,
                      const SfxItemSet* pArgSet ) :
        SfxTabDialog( pParent,
                      ScResId( RID_SCDLG_SORT ),
                      pArgSet ),
        bIsHeaders  ( FALSE ),
        bIsByRows   ( FALSE )

{
#if LAYOUT_SFX_TABDIALOG_BROKEN
    AddTabPage( TP_FIELDS,  ScTabPageSortFields::Create,  0 );
    AddTabPage( TP_OPTIONS, ScTabPageSortOptions::Create, 0 );
#else /* !LAYOUT_SFX_TABDIALOG_BROKEN */
    String fields = rtl::OUString::createFromAscii ("fields");
    AddTabPage( TP_FIELDS, fields, ScTabPageSortFields::Create, 0, FALSE, TAB_APPEND);
    String options = rtl::OUString::createFromAscii ("options");
    AddTabPage( TP_OPTIONS, options, ScTabPageSortOptions::Create, 0, FALSE, TAB_APPEND);
#endif /* !LAYOUT_SFX_TABDIALOG_BROKEN */
    FreeResource();
}

__EXPORT ScSortDlg::~ScSortDlg()
{
}

