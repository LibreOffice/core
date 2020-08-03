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
#include "JoinTableView.hxx"

namespace dbaui
{
    class OTableListBoxControl;
    class IRelationControlInterface;
    class ORelationControl;

    class OTableListBoxControl final
    {
        std::unique_ptr<weld::ComboBox> m_xLeftTable;
        std::unique_ptr<weld::ComboBox> m_xRightTable;
        std::unique_ptr<weld::Container> m_xTable;
        css::uno::Reference<css::awt::XWindow> m_xTableCtrlParent;
        VclPtr<ORelationControl> m_xRC_Tables;

        const OJoinTableView::OTableWindowMap*  m_pTableMap;
        IRelationControlInterface*              m_pParentDialog;
        OUString                                m_strCurrentLeft;
        OUString                                m_strCurrentRight;
        DECL_LINK( OnTableChanged, weld::ComboBox&, void );
    public:
        OTableListBoxControl(weld::Builder* _pParent,
                             const OJoinTableView::OTableWindowMap* _pTableMap,
                             IRelationControlInterface* _pParentDialog);
        ~OTableListBoxControl();

        /** fillListBoxes fills the list boxes with the table windows
        */
        void fillListBoxes();

        /** fillAndDisable fill the listboxes only with one entry and then disable them
            @param  _pConnectionData
                    contains the data which should be filled into the listboxes
        */
        void fillAndDisable(const TTableConnectionData::value_type& _pConnectionData);

        /** enables the relation control
        *
        * \param _bEnable when sal_True enables it, otherwise disable it.
        */
        void enableRelation(bool _bEnable);

        /** NotifyCellChange notifies the browse control that the connection data has changed
        */
        void NotifyCellChange();

        /** Init is a call through to the control inside this one
            @param  _pConnData
                    the connection data which is used to init the control
        */
        void Init(const TTableConnectionData::value_type& _pConnData);
        void lateUIInit();
        void lateInit();

        void Disable();
        void Invalidate();

        void SaveModified();

        TTableWindowData::value_type const & getReferencingTable()  const;

        /** getContainer returns the container interface
            @return the interface of the container
        */
        IRelationControlInterface* getContainer() const { return m_pParentDialog; }
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
