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



#ifndef SC_SORTDLG_HXX
#define SC_SORTDLG_HXX

#include <sfx2/tabdlg.hxx>

#if TEST_LAYOUT
#define LAYOUT_SFX_TABDIALOG_BROKEN 0
#else /* !TEST_LAYOUT */
#ifndef LAYOUT_SFX_TABDIALOG_BROKEN
#define LAYOUT_SFX_TABDIALOG_BROKEN 1
#endif /* !LAYOUT_SFX_TABDIALOG_BROKEN */
#endif /* !TEST_LAYOUT */

#if !LAYOUT_SFX_TABDIALOG_BROKEN
#include <sfx2/layout.hxx>
#include <layout/layout-pre.hxx>
#endif

class ScSortDlg : public SfxTabDialog
{
public:
                ScSortDlg( Window*           pParent,
                           const SfxItemSet* pArgSet );
                ~ScSortDlg();

    void    SetHeaders( BOOL bHeaders );
    void    SetByRows ( BOOL bByRows );
    BOOL    GetHeaders() const;
    BOOL    GetByRows () const;


private:
    BOOL    bIsHeaders;
    BOOL    bIsByRows;
};

inline void ScSortDlg::SetHeaders( BOOL bHeaders )  { bIsHeaders = bHeaders; }
inline void ScSortDlg::SetByRows ( BOOL bByRows  )  { bIsByRows = bByRows; }
inline BOOL ScSortDlg::GetHeaders() const           { return bIsHeaders; }
inline BOOL ScSortDlg::GetByRows () const           { return bIsByRows; }

#if !LAYOUT_SFX_TABDIALOG_BROKEN
#include <layout/layout-post.hxx>
#endif

#endif // SC_SORTDLG_HXX
