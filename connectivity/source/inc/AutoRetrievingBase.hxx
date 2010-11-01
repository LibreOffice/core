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
#ifndef _CONNECTIVITY_AUTOKEYRETRIEVINGBASE_HXX_
#define _CONNECTIVITY_AUTOKEYRETRIEVINGBASE_HXX_

#include <rtl/ustring.hxx>
#include "connectivity/dbtoolsdllapi.hxx"

namespace connectivity
{
    class OOO_DLLPUBLIC_DBTOOLS OAutoRetrievingBase
    {
        ::rtl::OUString m_sGeneratedValueStatement; // contains the statement which should be used when query for automatically generated values
        sal_Bool        m_bAutoRetrievingEnabled; // set to when we should allow to query for generated values
    protected:
        OAutoRetrievingBase() : m_bAutoRetrievingEnabled(sal_False) {}
        virtual ~OAutoRetrievingBase(){}

        inline void enableAutoRetrievingEnabled(sal_Bool _bAutoEnable)          { m_bAutoRetrievingEnabled = _bAutoEnable; }
        inline void setAutoRetrievingStatement(const ::rtl::OUString& _sStmt)   { m_sGeneratedValueStatement = _sStmt; }
    public:
        inline sal_Bool                 isAutoRetrievingEnabled()       const { return m_bAutoRetrievingEnabled; }
        inline const ::rtl::OUString&   getAutoRetrievingStatement()    const { return m_sGeneratedValueStatement; }

        /** transform the statement to query for auto generated values
            @param  _sInsertStatement
                The "INSERT" statement, is used to query for column and table names
            @return
                The transformed generated statement.
        */
        ::rtl::OUString getTransformedGeneratedStatement(const ::rtl::OUString& _sInsertStatement) const;
    };
}
#endif // _CONNECTIVITY_AUTOKEYRETRIEVINGBASE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
