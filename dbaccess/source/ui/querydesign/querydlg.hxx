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

#include <vcl/weld.hxx>

#include <QEnumTypes.hxx>

#include <RelControliFace.hxx>
#include <JoinTableView.hxx>


namespace dbaui
{
    class OTableListBoxControl;
    class OQueryTableView;
    class DlgQryJoin final : public weld::GenericDialogController
                           , public IRelationControlInterface
    {
        EJoinType                           eJoinType;
        TTableConnectionData::value_type    m_pConnData; // contains left and right table
        TTableConnectionData::value_type    m_pOrigConnData;
        css::uno::Reference< css::sdbc::XConnection > m_xConnection;

        std::unique_ptr<weld::Label> m_xML_HelpText;
        std::unique_ptr<weld::Button> m_xPB_OK;
        std::unique_ptr<weld::ComboBox> m_xLB_JoinType;
        std::unique_ptr<weld::CheckButton> m_xCBNatural;
        std::unique_ptr<OTableListBoxControl> m_xTableControl;

        DECL_LINK(OKClickHdl, weld::Button&, void);
        DECL_LINK(LBChangeHdl, weld::ComboBox&, void);
        DECL_LINK(NaturalToggleHdl, weld::ToggleButton&, void);

        /** setJoinType enables and set the new join type
            @param  _eNewJoinType   the new jointype
        */
        void setJoinType(EJoinType _eNewJoinType);
    public:
        DlgQryJoin( const OQueryTableView * pParent,
                    const TTableConnectionData::value_type& pData,
                    const OJoinTableView::OTableWindowMap*    _pTableMap,
                    const css::uno::Reference< css::sdbc::XConnection >& _xConnection,
                    bool _bAllowTableSelect);
        virtual ~DlgQryJoin() override;
        EJoinType GetJoinType() const { return eJoinType; };

        /** setValid set the valid inside, can be used for OK buttons
            @param  _bValid true when the using control allows an update
        */
        virtual void setValid(bool _bValid) override;

        /** notifyConnectionChange is callback which is called when the table selection has changed and a new connection exists
            @param  _pConnectionData    the connection which exists between the new tables
        */
        virtual void notifyConnectionChange() override;
    };
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
