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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_QUERYDESIGN_QUERYTABCONNUNDOACTION_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_QUERYDESIGN_QUERYTABCONNUNDOACTION_HXX

#include "QueryDesignUndoAction.hxx"

namespace dbaui
{
    class OQueryTableConnection;
    class OQueryTableView;
    class OQueryTabConnUndoAction : public OQueryDesignUndoAction
    {
    protected:
        VclPtr<OQueryTableConnection>  m_pConnection;
        bool                    m_bOwnerOfConn;
        // am I the only owner of the connection? (changes with every redo and undo)

    public:
        OQueryTabConnUndoAction(OQueryTableView* pOwner, const char* pCommentID);
        virtual ~OQueryTabConnUndoAction() override;

        virtual void Undo() override = 0;
        virtual void Redo() override = 0;

        void SetConnection(OQueryTableConnection* pConn) { m_pConnection = pConn; }
        // now SetOwnership please
        void SetOwnership(bool bTakeIt) { m_bOwnerOfConn = bTakeIt; }
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_QUERYDESIGN_QUERYTABCONNUNDOACTION_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
