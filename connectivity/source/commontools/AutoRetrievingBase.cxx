/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
