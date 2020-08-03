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
#pragma once

#include "WTabPage.hxx"
#include "WCopyTable.hxx"

#include <comphelper/stl_types.hxx>

namespace dbaui
{
    class OFieldDescription;

    // Wizard Page: OWizColumnSelect

    class OWizColumnSelect : public OWizardPage
    {
        std::unique_ptr<weld::TreeView> m_xOrgColumnNames; // left side
        std::unique_ptr<weld::Button> m_xColumn_RH;
        std::unique_ptr<weld::Button> m_xColumns_RH;
        std::unique_ptr<weld::Button> m_xColumn_LH;
        std::unique_ptr<weld::Button> m_xColumns_LH;
        std::unique_ptr<weld::TreeView> m_xNewColumnNames; // right side

        DECL_LINK( ButtonClickHdl, weld::Button&, void );
        DECL_LINK( ListDoubleClickHdl, weld::TreeView&, bool );

        static void clearListBox(weld::TreeView& _rListBox);
        static void fillColumns(weld::TreeView const * pRight,
                                std::vector< OUString> &_rRightColumns);

        void createNewColumn(   weld::TreeView* _pListbox,
                                OFieldDescription const * _pSrcField,
                                std::vector< OUString>& _rRightColumns,
                                const OUString&  _sColumnName,
                                const OUString&  _sExtraChars,
                                sal_Int32               _nMaxNameLen,
                                const ::comphelper::UStringMixEqual& _aCase);

        void moveColumn(        weld::TreeView* _pRight,
                                weld::TreeView const * _pLeft,
                                std::vector< OUString>& _rRightColumns,
                                const OUString&  _sColumnName,
                                const OUString&  _sExtraChars,
                                sal_Int32               _nMaxNameLen,
                                const ::comphelper::UStringMixEqual& _aCase);

        void enableButtons();

        sal_Int32 adjustColumnPosition(weld::TreeView const * _pLeft,
                                    const OUString&  _sColumnName,
                                    ODatabaseExport::TColumnVector::size_type nCurrentPos,
                                    const ::comphelper::UStringMixEqual& _aCase);

    public:
        virtual void            Reset ( ) override;
        virtual void            Activate() override;
        virtual bool            LeavePage() override;
        virtual OUString        GetTitle() const override ;

        OWizColumnSelect(weld::Container* pParent, OCopyTableWizard* pWizard);
        virtual ~OWizColumnSelect() override;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
