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
#ifndef CONNECTIVITY_PRIVILEGESRESULTSET_HXX
#define CONNECTIVITY_PRIVILEGESRESULTSET_HXX

#include "FDatabaseMetaDataResultSet.hxx"
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include "connectivity/dbtoolsdllapi.hxx"

namespace connectivity
{
    class OOO_DLLPUBLIC_DBTOOLS OResultSetPrivileges :
        public ODatabaseMetaDataResultSet
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>   m_xTables;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow>         m_xRow;
        bool                                                                m_bResetValues;
    protected:
        virtual const ORowSetValue& getValue(sal_Int32 columnIndex) SAL_OVERRIDE;
    public:
        OResultSetPrivileges(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData>& _rxMeta
            ,const ::com::sun::star::uno::Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern);

        // ::cppu::OComponentHelper
        virtual void SAL_CALL disposing(void) SAL_OVERRIDE;
        // XResultSet
        virtual sal_Bool SAL_CALL next(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    };
}
#endif // CONNECTIVITY_PRIVILEGESRESULTSET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
