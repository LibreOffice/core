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

#include <TableWindow.hxx>
#include "QTableWindowData.hxx"
#include <TableFieldDescription.hxx>

namespace dbaui
{
    class OQueryTableWindow : public OTableWindow
    {
        sal_Int32           m_nAliasNum;
        OUString            m_strInitialAlias;
    public:
        OQueryTableWindow( vcl::Window* pParent, const TTableWindowData::value_type& pTabWinData );

        OUString const & GetAliasName() const
        {
            return static_cast<OQueryTableWindowData*>(GetData().get())->GetAliasName();
        }
        void            SetAliasName(const OUString& strNewAlias)
        {
            static_cast<OQueryTableWindowData*>(GetData().get())->SetAliasName(strNewAlias);
        }

        // late Constructor, the base class CREATES Listbox on first call
        virtual bool         Init() override;

        bool                 ExistsField(const OUString& strFieldName, OTableFieldDescRef const & rInfo);
        bool                 ExistsAVisitedConn() const;

        virtual OUString     GetName() const override { return GetWinName(); }

    protected:

        virtual void    OnEntryDoubleClicked(weld::TreeIter& rEntry) override;
            // is called from DoubleClickHdl of the ListBox
        /** delete the user data with the equal type as created within createUserData
            @param  _pUserData
                The user data store in the listbox entries. Created with a call to createUserData.
                _pUserData may be <NULL/>.
        */
        virtual void deleteUserData(void*& _pUserData) override;

        /** creates user information that will be append at the ListBoxentry
            @param  _xColumn
                The corresponding column, can be <NULL/>.
            @param  _bPrimaryKey
                <TRUE/> when the column belongs to the primary key
            @return
                the user data which will be append at the listbox entry, may be <NULL/>
        */
        virtual void* createUserData(const css::uno::Reference<
                                    css::beans::XPropertySet>& _xColumn,
                                    bool _bPrimaryKey) override;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
