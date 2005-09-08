/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BCatalog.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 06:41:40 $
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
#ifndef _CONNECTIVITY_ADABAS_CATALOG_HXX_
#define _CONNECTIVITY_ADABAS_CATALOG_HXX_

#ifndef _CONNECTIVITY_SDBCX_CATALOG_HXX_
#include "connectivity/sdbcx/VCatalog.hxx"
#endif
#ifndef _CONNECTIVITY_OFUNCTIONDEFS_HXX_
#include "odbc/OFunctiondefs.hxx"
#endif
#ifndef CONNECTIVITY_STDTYPEDEFS_HXX
#include "connectivity/StdTypeDefs.hxx"
#endif

namespace connectivity
{
    namespace adabas
    {
        // please don't name the class the same name as in an other namespaces
        // some compilers have problems with this task as I noticed on windows
        class OAdabasConnection;
        class OAdabasCatalog : public connectivity::sdbcx::OCatalog
        {
            OAdabasConnection*  m_pConnection;      // used to get the metadata
            SQLHANDLE           m_aConnectionHdl;   // used for odbc specific stuff

            void fillVector(const ::rtl::OUString& _sQuery,TStringVector& _rVector);

        protected:
            /** builds the name which should be used to access the object later on in the collection.
                Will only be called in fillNames.
                @param  _xRow
                    The cuurent row from a call of XDatabaseMetaData::getTables.
            */
            virtual ::rtl::OUString buildName(  const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >& _xRow);
        public:
            // implementation of the pure virtual methods
            virtual void refreshTables();
            virtual void refreshViews() ;
            virtual void refreshGroups();
            virtual void refreshUsers() ;

        public:
            OAdabasCatalog(SQLHANDLE _aConnectionHdl,OAdabasConnection* _pCon);

            OAdabasConnection*      getConnection()     const { return m_pConnection; }
            sdbcx::OCollection*     getPrivateTables()  const { return m_pTables;}
            sdbcx::OCollection*     getPrivateViews()   const { return m_pViews; }

            static const ::rtl::OUString& getDot();
            // correct the the column properties of float/real/double values
            // all & parameters are IN and OUT
            static void correctColumnProperties(sal_Int32 _nPrec, sal_Int32& _rnType,::rtl::OUString& _rsTypeName);
        };
    }
}
#endif // _CONNECTIVITY_ADABAS_CATALOG_HXX_

