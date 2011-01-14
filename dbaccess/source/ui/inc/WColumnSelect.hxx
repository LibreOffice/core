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



