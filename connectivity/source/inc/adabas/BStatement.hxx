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
#ifndef CONNECTIVITY_ADABAS_STATEMENT_HXX
#define CONNECTIVITY_ADABAS_STATEMENT_HXX

#include "odbc/OStatement.hxx"
#include "adabas/BConnection.hxx"
#include "connectivity/CommonTools.hxx"
#include <rtl/ref.hxx>

namespace connectivity
{
    namespace adabas
    {
        class OAdabasStatement :    public  ::connectivity::odbc::OStatement
        {
            OAdabasConnection*          m_pOwnConnection;
            ::rtl::Reference<OSQLColumns>   m_aSelectColumns;
        protected:
            virtual odbc::OResultSet* createResulSet();
            virtual void setResultSetConcurrency(sal_Int32 _par0);
            virtual void setResultSetType(sal_Int32 _par0)      ;
            virtual void setUsingBookmarks(sal_Bool _bUseBookmark);
        public:
            OAdabasStatement( OAdabasConnection* _pConnection)
                : ::connectivity::odbc::OStatement( _pConnection )
                ,m_pOwnConnection(_pConnection)
            {}

            virtual sal_Bool SAL_CALL execute( const ::rtl::OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        };
    }
}

#endif // CONNECTIVITY_ADABAS_STATEMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
