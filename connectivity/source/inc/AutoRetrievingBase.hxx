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

#include <rtl/ustring.hxx>
#include <connectivity/dbtoolsdllapi.hxx>

namespace connectivity
{
    class OOO_DLLPUBLIC_DBTOOLS OAutoRetrievingBase
    {
        OUString m_sGeneratedValueStatement; // contains the statement which should be used when query for automatically generated values
        bool        m_bAutoRetrievingEnabled; // set to when we should allow to query for generated values
    protected:
        OAutoRetrievingBase() : m_bAutoRetrievingEnabled(false) {}
        virtual ~OAutoRetrievingBase(){}

        void enableAutoRetrievingEnabled(bool _bAutoEnable)          { m_bAutoRetrievingEnabled = _bAutoEnable; }
        void setAutoRetrievingStatement(const OUString& _sStmt)   { m_sGeneratedValueStatement = _sStmt; }
    public:
        bool                 isAutoRetrievingEnabled()       const { return m_bAutoRetrievingEnabled; }

        /** transform the statement to query for auto generated values
            @param  _sInsertStatement
                The "INSERT" statement, is used to query for column and table names
            @return
                The transformed generated statement.
        */
        OUString getTransformedGeneratedStatement(const OUString& _sInsertStatement) const;
    };
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
