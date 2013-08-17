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
#ifndef DBAUI_RTABLECONNECTION_HXX
#define DBAUI_RTABLECONNECTION_HXX

#include "TableConnection.hxx"
#include "RTableConnectionData.hxx"

namespace dbaui
{
    class ORelationTableView;
    class ORelationTableConnection : public OTableConnection
    {
    public:
        ORelationTableConnection( ORelationTableView* pContainer, const TTableConnectionData::value_type& pTabConnData );
        ORelationTableConnection( const ORelationTableConnection& rConn );
            // wichtiger Kommentar zum CopyConstructor siehe OTableConnection(const OTableConnection&)
        virtual ~ORelationTableConnection();

        ORelationTableConnection& operator=( const ORelationTableConnection& rConn );

        virtual void    Draw( const Rectangle& rRect );
        using OTableConnection::Draw;
    };
}
#endif // DBAUI_RTABLECONNECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
