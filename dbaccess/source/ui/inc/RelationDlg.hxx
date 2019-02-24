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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_RELATIONDLG_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_RELATIONDLG_HXX

#include <memory>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include "JoinTableView.hxx"
#include "RelControliFace.hxx"
#include "RelationControl.hxx"

namespace dbaui
{
    class OJoinTableView;
    class ORelationDialog final : public ModalDialog
                            ,public IRelationControlInterface
    {
        std::unique_ptr<OTableListBoxControl> m_xTableControl;

        VclPtr<RadioButton> m_pRB_NoCascUpd;
        VclPtr<RadioButton> m_pRB_CascUpd;
        VclPtr<RadioButton> m_pRB_CascUpdNull;
        VclPtr<RadioButton> m_pRB_CascUpdDefault;
        VclPtr<RadioButton> m_pRB_NoCascDel;
        VclPtr<RadioButton> m_pRB_CascDel;
        VclPtr<RadioButton> m_pRB_CascDelNull;
        VclPtr<RadioButton> m_pRB_CascDelDefault;

        VclPtr<OKButton>    m_pPB_OK;

        TTableConnectionData::value_type                 m_pConnData;
        TTableConnectionData::value_type                 m_pOrigConnData;

        bool                                             m_bTriedOneUpdate;

    public:
        ORelationDialog(OJoinTableView* pParent,
                        const TTableConnectionData::value_type& pConnectionData,
                        bool bAllowTableSelect = false );
        virtual ~ORelationDialog() override;
        virtual void dispose() override;

        virtual short Execute() override;

        /** setValid set the valid inside, can be used for OK buttons
            @param  _bValid true when the using control allows an update
        */
        virtual void setValid(bool _bValid) override;

        /** notifyConnectionChange is callback which is called when the table selection has changed and a new connection exists
            @param  _pConnectionData    the connection which exists between the new tables
        */
        virtual void notifyConnectionChange() override;
    private:
        void Init(const TTableConnectionData::value_type& _pConnectionData);

        DECL_LINK( OKClickHdl, Button*, void );
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_RELATIONDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
