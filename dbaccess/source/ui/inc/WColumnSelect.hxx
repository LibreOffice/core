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


#ifndef DBAUI_WIZ_COLUMNSELECT_HXX
#define DBAUI_WIZ_COLUMNSELECT_HXX

#include "WTabPage.hxx"
#include "WCopyTable.hxx"

#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <comphelper/stl_types.hxx>

namespace dbaui
{
    class OFieldDescription;

    // ========================================================
    // Wizard Page: OWizColumnSelect
    // ========================================================

    class OWizColumnSelect : public OWizardPage
    {

        FixedLine               m_flColumns;
        MultiListBox            m_lbOrgColumnNames; // left side
        ImageButton             m_ibColumn_RH;
        ImageButton             m_ibColumns_RH;
        ImageButton             m_ibColumn_LH;
        ImageButton             m_ibColumns_LH;
        MultiListBox            m_lbNewColumnNames; // right side


        DECL_LINK( ButtonClickHdl, Button * );
        DECL_LINK( ListDoubleClickHdl, MultiListBox * );


        void clearListBox(MultiListBox& _rListBox);
        void fillColumns(       ListBox* pRight,
                                ::std::vector< ::rtl::OUString> &_rRightColumns);

        void createNewColumn(   ListBox* _pListbox,
                                OFieldDescription* _pSrcField,
                                ::std::vector< ::rtl::OUString>& _rRightColumns,
                                const ::rtl::OUString&  _sColumnName,
                                const ::rtl::OUString&  _sExtraChars,
                                sal_Int32               _nMaxNameLen,
                                const ::comphelper::TStringMixEqualFunctor& _aCase);

        void moveColumn(        ListBox* _pRight,
                                ListBox* _pLeft,
                                ::std::vector< ::rtl::OUString>& _rRightColumns,
                                const ::rtl::OUString&  _sColumnName,
                                const ::rtl::OUString&  _sExtraChars,
                                sal_Int32               _nMaxNameLen,
                                const ::comphelper::TStringMixEqualFunctor& _aCase);

        void enableButtons();


        sal_uInt16 adjustColumnPosition(ListBox* _pLeft,
                                    const ::rtl::OUString&  _sColumnName,
                                    ODatabaseExport::TColumnVector::size_type nCurrentPos,
                                    const ::comphelper::TStringMixEqualFunctor& _aCase);

    public:
        virtual void            Reset ( );
        virtual void            ActivatePage();
        virtual sal_Bool        LeavePage();
        virtual String          GetTitle() const ;

        OWizColumnSelect(Window* pParent);
        virtual ~OWizColumnSelect();
    };
}
#endif // DBAUI_WIZ_COLUMNSELECT_HXX



