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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"

#include <connectivity/virtualdbtools.hxx>
#include <connectivity/formattedcolumnvalue.hxx>
#include "dbtfactory.hxx"
#include "parser_s.hxx"
#include "staticdbtools_s.hxx"
#include "charset_s.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

//================================================================
// the entry point for load-on-call usage of the DBTOOLS lib
extern "C" void* SAL_CALL createDataAccessToolsFactory()
{
    ::connectivity::ODataAccessToolsFactory* pFactory = new ::connectivity::ODataAccessToolsFactory;
    pFactory->acquire();
    return pFactory;
}

//........................................................................
namespace connectivity
{
//........................................................................

    //================================================================
    //= ODataAccessToolsFactory
    //================================================================
    //----------------------------------------------------------------
    ODataAccessToolsFactory::ODataAccessToolsFactory()
    {
        ODataAccessStaticTools* pStaticTools = new ODataAccessStaticTools;
        m_xTypeConversionHelper = pStaticTools;
        m_xToolsHelper = pStaticTools;
    }

    //----------------------------------------------------------------
    oslInterlockedCount SAL_CALL ODataAccessToolsFactory::acquire()
    {
        return ORefBase::acquire();
    }

    //----------------------------------------------------------------
    oslInterlockedCount SAL_CALL ODataAccessToolsFactory::release()
    {
        return ORefBase::release();
    }

    //----------------------------------------------------------------
    ::rtl::Reference< simple::IDataAccessTypeConversion > ODataAccessToolsFactory::getTypeConversionHelper()
    {
        return m_xTypeConversionHelper;
    }

    //----------------------------------------------------------------
    ::rtl::Reference< simple::IDataAccessCharSet > ODataAccessToolsFactory::createCharsetHelper( ) const
    {
        return new ODataAccessCharSet;
    }

    //----------------------------------------------------------------
    ::rtl::Reference< simple::IDataAccessTools > ODataAccessToolsFactory::getDataAccessTools()
    {
        return m_xToolsHelper;
    }

    //----------------------------------------------------------------
    ::std::auto_ptr< ::dbtools::FormattedColumnValue > ODataAccessToolsFactory::createFormattedColumnValue( const ::comphelper::ComponentContext& _rContext,
        const Reference< ::com::sun::star::sdbc::XRowSet >& _rxRowSet, const Reference< ::com::sun::star::beans::XPropertySet >& _rxColumn )
    {
        ::std::auto_ptr< ::dbtools::FormattedColumnValue > pValue( new ::dbtools::FormattedColumnValue( _rContext, _rxRowSet, _rxColumn ) );
        return pValue;
    }

    //----------------------------------------------------------------
    ::rtl::Reference< simple::ISQLParser > ODataAccessToolsFactory::createSQLParser(const Reference< XMultiServiceFactory >& _rxServiceFactory,const IParseContext* _pContext) const
    {
        return new OSimpleSQLParser(_rxServiceFactory,_pContext);
    }

//........................................................................
}   // namespace connectivity
//........................................................................


