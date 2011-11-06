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


#ifndef _CONNECTIVITY_ADABAS_CATALOG_HXX_
#define _CONNECTIVITY_ADABAS_CATALOG_HXX_

#include "connectivity/sdbcx/VCatalog.hxx"
#include "odbc/OFunctiondefs.hxx"
#include "connectivity/StdTypeDefs.hxx"

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

