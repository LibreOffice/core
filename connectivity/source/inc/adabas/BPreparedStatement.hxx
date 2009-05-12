/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: BPreparedStatement.hxx,v $
 * $Revision: 1.6 $
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
#ifndef CONNECTIVITY_ADABAS_PREPAREDSTATEMENT_HXX
#define CONNECTIVITY_ADABAS_PREPAREDSTATEMENT_HXX

#include "odbc/OPreparedStatement.hxx"
#include "adabas/BConnection.hxx"

namespace connectivity
{
    namespace adabas
    {
        class OAdabasPreparedStatement :    public  ::connectivity::odbc::OPreparedStatement
        {
            ::vos::ORef<OSQLColumns>    m_aSelectColumns;
        protected:
            virtual odbc::OResultSet* createResulSet();
            virtual void setResultSetConcurrency(sal_Int32 _par0)   ;
            virtual void setResultSetType(sal_Int32 _par0)          ;
            virtual void setUsingBookmarks(sal_Bool _bUseBookmark)  ;
        public:
            OAdabasPreparedStatement( OAdabasConnection* _pConnection,const ::std::vector<OTypeInfo>& _TypeInfo,const ::rtl::OUString& sql);
        };
    }
}

#endif // CONNECTIVITY_ADABAS_PREPAREDSTATEMENT_HXX

