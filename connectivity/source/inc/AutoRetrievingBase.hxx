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
#ifndef _CONNECTIVITY_AUTOKEYRETRIEVINGBASE_HXX_
#define _CONNECTIVITY_AUTOKEYRETRIEVINGBASE_HXX_

#include <rtl/ustring.hxx>
#include "connectivity/dbtoolsdllapi.hxx"

namespace connectivity
{
    class OOO_DLLPUBLIC_DBTOOLS OAutoRetrievingBase
    {
        OUString m_sGeneratedValueStatement; // contains the statement which should be used when query for automatically generated values
        sal_Bool        m_bAutoRetrievingEnabled; // set to when we should allow to query for generated values
    protected:
        OAutoRetrievingBase() : m_bAutoRetrievingEnabled(sal_False) {}
        virtual ~OAutoRetrievingBase(){}

        inline void enableAutoRetrievingEnabled(sal_Bool _bAutoEnable)          { m_bAutoRetrievingEnabled = _bAutoEnable; }
        inline void setAutoRetrievingStatement(const OUString& _sStmt)   { m_sGeneratedValueStatement = _sStmt; }
    public:
        inline sal_Bool                 isAutoRetrievingEnabled()       const { return m_bAutoRetrievingEnabled; }
        inline const OUString&   getAutoRetrievingStatement()    const { return m_sGeneratedValueStatement; }

        /** transform the statement to query for auto generated values
            @param  _sInsertStatement
                The "INSERT" statement, is used to query for column and table names
            @return
                The transformed generated statement.
        */
        OUString getTransformedGeneratedStatement(const OUString& _sInsertStatement) const;
    };
}
#endif // _CONNECTIVITY_AUTOKEYRETRIEVINGBASE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
