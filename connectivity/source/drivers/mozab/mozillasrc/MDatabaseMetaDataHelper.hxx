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



#ifndef _CONNECTIVITY_MAB_DATABASEMETADATAHELPER_HXX_
#define _CONNECTIVITY_MAB_DATABASEMETADATAHELPER_HXX_

#include "MErrorResource.hxx"
#include <comphelper/proparrhlp.hxx>
#include <comphelper/propertycontainer.hxx>
#include "FDatabaseMetaDataResultSet.hxx"

#ifndef _CONNECTIVITY_MAB_CONNECTION_HXX_
#include <MConnection.hxx>
#endif
#include <com/sun/star/uno/Sequence.hxx>
#ifndef _COM_SUN_STAR_MOZILLA_MOZILLPRODUCTTYPE_HPP_
#include <com/sun/star/mozilla/MozillaProductType.hpp>
#endif

namespace connectivity
{
    namespace mozab
    {
        class MDatabaseMetaDataHelper
        {
        private:
            sal_Bool                                        m_bProfileExists ;
            ::std::vector< ::rtl::OUString >                m_aTableNames;
            ::std::vector< ::rtl::OUString >                m_aTableTypes;
            ::com::sun::star::mozilla::MozillaProductType   m_ProductType;
            ::rtl::OUString                                 m_ProfileName;
            ErrorDescriptor                                 m_aError;

        public:
            MDatabaseMetaDataHelper();
            ~MDatabaseMetaDataHelper();

            //
            sal_Bool getTableStrings( OConnection*                        _pCon,
                                      ::std::vector< ::rtl::OUString >&   _rStrings,
                                      ::std::vector< ::rtl::OUString >&   _rTypes);

            sal_Bool getTables( OConnection* _pCon,
                                const ::rtl::OUString& tableNamePattern,
                                const ::com::sun::star::uno::Sequence< ::rtl::OUString >& types,
                                ODatabaseMetaDataResultSet::ORows& _rRows);
            sal_Bool   testLDAPConnection( OConnection* _pCon );
            sal_Bool   NewAddressBook( OConnection* _pCon,const ::rtl::OUString & aTableName);

            inline const ErrorDescriptor& getError() const { return m_aError; }
        };
    }

}
#endif // _CONNECTIVITY_MAB_DATABASEMETADATAHELPER_HXX_

