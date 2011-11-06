/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
#include "AutoRetrievingBase.hxx"

namespace connectivity
{
    ::rtl::OUString OAutoRetrievingBase::getTransformedGeneratedStatement(const ::rtl::OUString& _sInsertStatement) const
    {
        ::rtl::OUString sStmt = _sInsertStatement;
        OSL_ENSURE( m_bAutoRetrievingEnabled,"Illegal call here. isAutoRetrievingEnabled is false!");
        sStmt = sStmt.toAsciiUpperCase();
        ::rtl::OUString sStatement;
        if ( sStmt.compareToAscii("INSERT",6) == 0 )
        {
            sStatement = m_sGeneratedValueStatement;
            static const ::rtl::OUString sColumn(RTL_CONSTASCII_USTRINGPARAM("$column"));
            static const ::rtl::OUString sTable(RTL_CONSTASCII_USTRINGPARAM("$table"));
            sal_Int32 nIndex = 0;
            nIndex = sStatement.indexOf(sColumn,nIndex);
            if ( -1 != nIndex )
            { // we need a column
            }
            nIndex = 0;
            nIndex = sStatement.indexOf(sTable,nIndex);
            if ( -1 != nIndex )
            { // we need a table name
                sal_Int32 nIntoIndex = sStmt.indexOf(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("INTO ")));
                sStmt = sStmt.copy(nIntoIndex+5);
                do
                {
                    if ( sStmt.indexOf(' ') == 0 )
                        sStmt = sStmt.copy(1);
                }
                while (sStmt.indexOf(' ') == 0 );

                nIntoIndex = 0;
                ::rtl::OUString sTableName = sStmt.getToken(0,' ',nIntoIndex);
                sStatement = sStatement.replaceAt(nIndex,sTable.getLength(),sTableName);
            }
        }
        return sStatement;
    }
}

