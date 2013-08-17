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
#ifndef DBAUI_QUERYTABLECONNECTION_HXX
#define DBAUI_QUERYTABLECONNECTION_HXX

#include "TableConnection.hxx"
#include "QTableConnectionData.hxx"
#include "QEnumTypes.hxx"

namespace dbaui
{
    class OQueryTableView;
    class OQueryTableConnection : public OTableConnection
    {
        sal_Bool m_bVisited;    // is true if the conn was already visited through the join algorithm
    public:
        OQueryTableConnection(OQueryTableView* pContainer, const TTableConnectionData::value_type& pTabConnData);
        OQueryTableConnection(const OQueryTableConnection& rConn);
        virtual ~OQueryTableConnection();

        OQueryTableConnection& operator=(const OQueryTableConnection& rConn);
        sal_Bool operator==(const OQueryTableConnection& rCompare);

        inline OUString  GetAliasName(EConnectionSide nWhich) const { return static_cast<OQueryTableConnectionData*>(GetData().get())->GetAliasName(nWhich); }

        inline sal_Bool IsVisited() const               { return m_bVisited; }
        inline void     SetVisited(sal_Bool bVisited)   { m_bVisited = bVisited; }

    };
}
#endif // DBAUI_QUERYTABLECONNECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
