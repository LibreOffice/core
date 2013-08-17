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

#include "QTableConnection.hxx"
#include <osl/diagnose.h>
#include "QueryTableView.hxx"
#include "ConnectionLine.hxx"
using namespace dbaui;
// class OQueryTableConnection
DBG_NAME(OQueryTableConnection)

OQueryTableConnection::OQueryTableConnection(OQueryTableView* pContainer, const TTableConnectionData::value_type& pTabConnData)
    :OTableConnection(pContainer, pTabConnData)
    ,m_bVisited(sal_False)
{
    DBG_CTOR(OQueryTableConnection,NULL);
}

OQueryTableConnection::OQueryTableConnection(const OQueryTableConnection& rConn)
    :OTableConnection( rConn )
{
    DBG_CTOR(OQueryTableConnection,NULL);
    // no own members, so base class functionality is sufficient
}

OQueryTableConnection::~OQueryTableConnection()
{
    DBG_DTOR(OQueryTableConnection,NULL);
}

OQueryTableConnection& OQueryTableConnection::operator=(const OQueryTableConnection& rConn)
{
    if (&rConn == this)
        return *this;

    OTableConnection::operator=(rConn);
    // no own members ...
    return *this;
}

sal_Bool OQueryTableConnection::operator==(const OQueryTableConnection& rCompare)
{
    OSL_ENSURE(GetData() && rCompare.GetData(), "OQueryTableConnection::operator== : one of the two participants has no data!");

    // I don't have to compare all too much (especially not all the members) : merely the windows, which we are connected to, and the indices in the corresponding table have to match.
    OQueryTableConnectionData* pMyData = static_cast<OQueryTableConnectionData*>(GetData().get());
    OQueryTableConnectionData* pCompData = static_cast<OQueryTableConnectionData*>(rCompare.GetData().get());

    // Connections are seen as equal, if source and destination window names and source and destination field Indices match...
    return  (   (   (pMyData->getReferencedTable() == pCompData->getReferencedTable()) &&
                    (pMyData->getReferencingTable() == pCompData->getReferencingTable()) &&
                    (pMyData->GetFieldIndex(JTCS_TO) == pCompData->GetFieldIndex(JTCS_TO)) &&
                    (pMyData->GetFieldIndex(JTCS_FROM) == pCompData->GetFieldIndex(JTCS_FROM))
                )
                ||  // ... or this cross matching is given
                (   (pMyData->getReferencingTable() == pCompData->getReferencedTable()) &&
                    (pMyData->getReferencedTable() == pCompData->getReferencingTable()) &&
                    (pMyData->GetFieldIndex(JTCS_TO) == pCompData->GetFieldIndex(JTCS_FROM)) &&
                    (pMyData->GetFieldIndex(JTCS_FROM) == pCompData->GetFieldIndex(JTCS_TO))
                )
            );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
