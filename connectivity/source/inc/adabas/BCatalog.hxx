/*************************************************************************
 *
 *  $RCSfile: BCatalog.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _CONNECTIVITY_ADABAS_CATALOG_HXX_
#define _CONNECTIVITY_ADABAS_CATALOG_HXX_

#ifndef _CONNECTIVITY_SDBCX_CATALOG_HXX_
#include "connectivity/sdbcx/VCatalog.hxx"
#endif
#ifndef _CONNECTIVITY_OFUNCTIONDEFS_HXX_
#include "odbc/OFunctiondefs.hxx"
#endif

namespace connectivity
{
    namespace adabas
    {
        class OAdabasConnection;
        class OAdabasCatalog : public connectivity::sdbcx::OCatalog
        {
            OAdabasConnection*  m_pConnection;
            SQLHANDLE           m_aConnectionHdl;
                        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData > m_xMetaData;

        public:
            virtual void refreshTables();
            virtual void refreshViews() ;
            virtual void refreshGroups();
            virtual void refreshUsers() ;

        public:
            OAdabasCatalog(SQLHANDLE _aConnectionHdl,OAdabasConnection* _pCon);

            OAdabasConnection*  getConnection() { return m_pConnection; }
        };
    }
}
#endif // _CONNECTIVITY_ADABAS_CATALOG_HXX_

