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

#include "AutoRetrievingBase.hxx"

namespace connectivity
{
    OUString OAutoRetrievingBase::getTransformedGeneratedStatement(const OUString& _sInsertStatement) const
    {
        OSL_ENSURE( m_bAutoRetrievingEnabled,"Illegal call here. isAutoRetrievingEnabled is false!");
        OUString sStmt = _sInsertStatement.toAsciiUpperCase();
        OUString sStatement;
        if ( sStmt.startsWith("INSERT") )
        {
            sStatement = m_sGeneratedValueStatement;
            static const OUString sColumn("$column");
            static const OUString sTable("$table");
            sal_Int32 nIndex = 0;
            nIndex = sStatement.indexOf(sColumn,nIndex);
            if ( -1 != nIndex )
            { // we need a column
            }
            nIndex = 0;
            nIndex = sStatement.indexOf(sTable,nIndex);
            if ( -1 != nIndex )
            { // we need a table name
                sal_Int32 nIntoIndex = sStmt.indexOf("INTO ");
                sStmt = sStmt.copy(nIntoIndex+5);
                do
                {
                    if ( sStmt.indexOf(' ') == 0 )
                        sStmt = sStmt.copy(1);
                }
                while (sStmt.indexOf(' ') == 0 );

                nIntoIndex = 0;
                OUString sTableName = sStmt.getToken(0,' ',nIntoIndex);
                sStatement = sStatement.replaceAt(nIndex,sTable.getLength(),sTableName);
            }
        }
        return sStatement;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
