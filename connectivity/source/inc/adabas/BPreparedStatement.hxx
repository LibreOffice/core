/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BPreparedStatement.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 06:45:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef CONNECTIVITY_ADABAS_PREPAREDSTATEMENT_HXX
#define CONNECTIVITY_ADABAS_PREPAREDSTATEMENT_HXX

#ifndef _CONNECTIVITY_ODBC_OPREPAREDSTATEMENT_HXX_
#include "odbc/OPreparedStatement.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_BCONNECTION_HXX_
#include "adabas/BConnection.hxx"
#endif

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

