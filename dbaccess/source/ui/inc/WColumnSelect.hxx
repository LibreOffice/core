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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_WCOLUMNSELECT_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_WCOLUMNSELECT_HXX

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
        VclPtr<ListBox>      m_pOrgColumnNames; // left side
        VclPtr<PushButton>   m_pColumn_RH;
        VclPtr<PushButton>   m_pColumns_RH;
        VclPtr<PushButton>   m_pColumn_LH;
        VclPtr<PushButton>   m_pColumns_LH;
        VclPtr<ListBox>      m_pNewColumnNames; // right side

        DECL_LINK_TYPED( ButtonClickHdl, Button *, void );
        DECL_LINK_TYPED( ListDoubleClickHdl, ListBox&, void );

        static void clearListBox(ListBox& _rListBox);
        static void fillColumns( ListBox* pRight,
                                ::std::vector< OUString> &_rRightColumns);

        void createNewColumn(   ListBox* _pListbox,
                                OFieldDescription* _pSrcField,
                                ::std::vector< OUString>& _rRightColumns,
                                const OUString&  _sColumnName,
                                const OUString&  _sExtraChars,
                                sal_Int32               _nMaxNameLen,
                                const ::comphelper::UStringMixEqual& _aCase);

        void moveColumn(        ListBox* _pRight,
                                ListBox* _pLeft,
                                ::std::vector< OUString>& _rRightColumns,
                                const OUString&  _sColumnName,
                                const OUString&  _sExtraChars,
                                sal_Int32               _nMaxNameLen,
                                const ::comphelper::UStringMixEqual& _aCase);

        void enableButtons();

        sal_Int32 adjustColumnPosition(ListBox* _pLeft,
                                    const OUString&  _sColumnName,
                                    ODatabaseExport::TColumnVector::size_type nCurrentPos,
                                    const ::comphelper::UStringMixEqual& _aCase);

    public:
        virtual void            Reset ( ) override;
        virtual void            ActivatePage() override;
        virtual bool            LeavePage() override;
        virtual OUString        GetTitle() const override ;

        OWizColumnSelect(vcl::Window* pParent);
        virtual ~OWizColumnSelect();
        virtual void dispose() override;
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_WCOLUMNSELECT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
