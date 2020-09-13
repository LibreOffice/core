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

namespace dbaui
{
    class OWizColumnSelect;
    class OWizNormalExtend;
    class OCopyTable final : public OWizardPage
    {
        bool               m_bPKeyAllowed;
        bool               m_bUseHeaderAllowed;
        sal_Int16          m_nOldOperation;

        std::unique_ptr<weld::Entry> m_xEdTableName;
        std::unique_ptr<weld::RadioButton> m_xRB_DefData;
        std::unique_ptr<weld::RadioButton> m_xRB_Def;
        std::unique_ptr<weld::RadioButton> m_xRB_View;
        std::unique_ptr<weld::RadioButton> m_xRB_AppendData;
        std::unique_ptr<weld::CheckButton> m_xCB_UseHeaderLine;
        std::unique_ptr<weld::CheckButton> m_xCB_PrimaryColumn;
        std::unique_ptr<weld::Label> m_xFT_KeyName;
        std::unique_ptr<weld::Entry> m_xEdKeyName;

        DECL_LINK( AppendDataClickHdl, weld::Button&, void );
        DECL_LINK( RadioChangeHdl, weld::Button&, void );
        DECL_LINK( KeyClickHdl, weld::Button&, void );

        bool checkAppendData();
        void SetAppendDataRadio();

    public:
        virtual void            Reset() override;
        virtual void            Activate() override;
        virtual bool            LeavePage() override;
        virtual OUString        GetTitle() const override ;

        OCopyTable(weld::Container* pParent, OCopyTableWizard* pWizard);
        virtual ~OCopyTable() override;

        bool IsOptionDefData() const { return m_xRB_DefData->get_active(); }
        bool IsOptionDef() const { return m_xRB_Def->get_active(); }
        bool IsOptionView() const { return m_xRB_View->get_active(); }
        OUString GetKeyName() const { return m_xEdKeyName->get_text(); }

        void setCreateStyleAction();
        void disallowViews()
        {
            m_xRB_View->set_sensitive(false);
        }
        void disallowUseHeaderLine()
        {
            m_bUseHeaderAllowed = false;
            m_xCB_UseHeaderLine->set_sensitive(false);
        }

        void setCreatePrimaryKey( bool _bDoCreate, const OUString& _rSuggestedName );
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
