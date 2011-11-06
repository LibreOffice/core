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



#ifndef CONNECTIVITY_DBTOOLS_DBTFACTORY_HXX
#define CONNECTIVITY_DBTOOLS_DBTFACTORY_HXX

#include <connectivity/virtualdbtools.hxx>
#include "refbase.hxx"

//........................................................................
namespace connectivity
{
//........................................................................

    //================================================================
    //= ODataAccessToolsFactory
    //================================================================
    class ODataAccessToolsFactory
            :public simple::IDataAccessToolsFactory
            ,public ORefBase
    {
    protected:
        ::rtl::Reference< simple::IDataAccessTypeConversion >   m_xTypeConversionHelper;
        ::rtl::Reference< simple::IDataAccessTools >            m_xToolsHelper;

    public:
        ODataAccessToolsFactory();

        // IDataAccessToolsFactory
        virtual ::rtl::Reference< simple::ISQLParser >  createSQLParser(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxServiceFactory,
            const IParseContext* _pContext
        ) const;

        virtual ::rtl::Reference< simple::IDataAccessCharSet > createCharsetHelper( ) const;

        virtual ::rtl::Reference< simple::IDataAccessTypeConversion > getTypeConversionHelper();

        virtual ::rtl::Reference< simple::IDataAccessTools > getDataAccessTools();

        virtual ::std::auto_ptr< ::dbtools::FormattedColumnValue >  createFormattedColumnValue(
            const ::comphelper::ComponentContext& _rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& _rxRowSet,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxColumn
        );

        // IReference
        virtual oslInterlockedCount SAL_CALL acquire();
        virtual oslInterlockedCount SAL_CALL release();
    };

//........................................................................
}   // namespace connectivity
//........................................................................

#endif // CONNECTIVITY_DBTOOLS_DBTFACTORY_HXX

