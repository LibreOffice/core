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
#include "DExport.hxx"
#include "WCopyTable.hxx"

namespace dbaui
{
    // Wizard Page: OWizNameMatching
    // Name matching for data appending
    class OWizNameMatching : public OWizardPage
    {
        std::unique_ptr<weld::Label> m_xTABLE_LEFT;
        std::unique_ptr<weld::Label> m_xTABLE_RIGHT;
        std::unique_ptr<weld::TreeView> m_xCTRL_LEFT;    // left side
        std::unique_ptr<weld::TreeView> m_xCTRL_RIGHT;   // right side
        std::unique_ptr<weld::Button> m_xColumn_up;
        std::unique_ptr<weld::Button> m_xColumn_down;
        std::unique_ptr<weld::Button> m_xColumn_up_right;
        std::unique_ptr<weld::Button> m_xColumn_down_right;
        std::unique_ptr<weld::Button> m_xAll;
        std::unique_ptr<weld::Button> m_xNone;
        OUString            m_sSourceText;
        OUString            m_sDestText;

        DECL_LINK( ButtonClickHdl, weld::Button&, void );
        DECL_LINK( RightButtonClickHdl, weld::Button&, void );
        DECL_LINK( AllNoneClickHdl, weld::Button&, void );
        DECL_LINK( TableListClickHdl, weld::TreeView&, void );
        DECL_LINK( TableListRightSelectHdl, weld::TreeView&, void );

        static void FillListBox(weld::TreeView& rTreeView, const ODatabaseExport::TColumnVector& rList, bool bCheckButtons);

    public:
        virtual void            Reset ( ) override;
        virtual void            Activate() override;
        virtual bool            LeavePage() override;
        virtual OUString        GetTitle() const override ;

        OWizNameMatching(weld::Container* pPage, OCopyTableWizard* pWizard);
        virtual ~OWizNameMatching() override;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
