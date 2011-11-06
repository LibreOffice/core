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


#ifndef DBAUI_WIZ_NAMEMATCHING_HXX
#define DBAUI_WIZ_NAMEMATCHING_HXX

#ifndef DBAUI_WIZ_TABBPAGE_HXX
#include "WTabPage.hxx"
#endif
#ifndef _DBAUI_MARKTREE_HXX_
#include "marktree.hxx"
#endif
#ifndef DBAUI_DATABASEEXPORT_HXX
#include "DExport.hxx"
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef DBAUI_WIZ_COPYTABLEDIALOG_HXX
#include "WCopyTable.hxx"
#endif

namespace dbaui
{
    // ========================================================
    // columns are at root only no children
    // ========================================================
    class OColumnTreeBox : public OMarkableTreeListBox
    {
        sal_Bool m_bReadOnly;
    protected:
        virtual void InitEntry(SvLBoxEntry* pEntry, const String& rStr, const Image& rImg1, const Image& rImg2, SvLBoxButtonKind eButtonKind);

    public:
        OColumnTreeBox( Window* pParent, const ResId& rResId );

        void FillListBox( const ODatabaseExport::TColumnVector& _rList);
        void SetReadOnly(sal_Bool _bRo=sal_True) { m_bReadOnly = _bRo; }
        virtual sal_Bool Select( SvLBoxEntry* pEntry, sal_Bool bSelect=sal_True );

    private:
        using OMarkableTreeListBox::Select;
    };

    // ========================================================
    // Wizard Page: OWizNameMatching
    // Name matching for data appending
    // ========================================================
    class OWizNameMatching : public OWizardPage
    {
        FixedText           m_FT_TABLE_LEFT;
        FixedText           m_FT_TABLE_RIGHT;
        OColumnTreeBox      m_CTRL_LEFT;    // left side
        OColumnTreeBox      m_CTRL_RIGHT;   // right side
        ImageButton         m_ibColumn_up;
        ImageButton         m_ibColumn_down;
        ImageButton         m_ibColumn_up_right;
        ImageButton         m_ibColumn_down_right;
        PushButton          m_pbAll;
        PushButton          m_pbNone;
        String              m_sSourceText;
        String              m_sDestText;

        sal_Bool            m_bAttrsChanged;

        DECL_LINK( ButtonClickHdl, Button * );
        DECL_LINK( RightButtonClickHdl, Button * );
        DECL_LINK( AllNoneClickHdl, Button * );
        DECL_LINK( TableListClickHdl, void* );
        DECL_LINK( TableListRightSelectHdl, void* );

    public:
        virtual void            Reset ( );
        virtual void            ActivatePage();
        virtual sal_Bool        LeavePage();
        virtual String          GetTitle() const ;

        OWizNameMatching(Window* pParent);
        virtual ~OWizNameMatching();

    };
}
#endif // DBAUI_WIZ_NAMEMATCHING_HXX



