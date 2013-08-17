/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef DBAUI_WIZ_COLUMNSELECT_HXX
#define DBAUI_WIZ_COLUMNSELECT_HXX

#include "WTabPage.hxx"
#include "WCopyTable.hxx"

#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <comphelper/stl_types.hxx>

namespace dbaui
{
    class OFieldDescription;

    // Wizard Page: OWizColumnSelect

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
                                ::std::vector< OUString> &_rRightColumns);

        void createNewColumn(   ListBox* _pListbox,
                                OFieldDescription* _pSrcField,
                                ::std::vector< OUString>& _rRightColumns,
                                const OUString&  _sColumnName,
                                const OUString&  _sExtraChars,
                                sal_Int32               _nMaxNameLen,
                                const ::comphelper::TStringMixEqualFunctor& _aCase);

        void moveColumn(        ListBox* _pRight,
                                ListBox* _pLeft,
                                ::std::vector< OUString>& _rRightColumns,
                                const OUString&  _sColumnName,
                                const OUString&  _sExtraChars,
                                sal_Int32               _nMaxNameLen,
                                const ::comphelper::TStringMixEqualFunctor& _aCase);

        void enableButtons();

        sal_uInt16 adjustColumnPosition(ListBox* _pLeft,
                                    const OUString&  _sColumnName,
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
