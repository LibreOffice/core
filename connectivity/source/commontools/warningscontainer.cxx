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

#include "connectivity/warningscontainer.hxx"
#include "connectivity/dbexception.hxx"

#include <osl/diagnose.h>

//........................................................................
namespace dbtools
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdb;

    //====================================================================
    //= WarningsContainer
    //====================================================================
    //--------------------------------------------------------------------
    static void lcl_concatWarnings( Any& _rChainLeft, const Any& _rChainRight )
    {
        if ( !_rChainLeft.hasValue() )
            _rChainLeft = _rChainRight;
        else
        {
            // to travel the chain by reference (and not by value), we need the getValue ...
            // looks like a hack, but the meaning of getValue is documented, and it's the only chance for reference-traveling ....

            OSL_ENSURE( SQLExceptionInfo( _rChainLeft ).isValid(),
                "lcl_concatWarnings: invalid warnings chain (this will crash)!" );

            const SQLException* pChainTravel = static_cast< const SQLException* >( _rChainLeft.getValue() );
            SQLExceptionIteratorHelper aReferenceIterHelper( *pChainTravel );
            while ( aReferenceIterHelper.hasMoreElements() )
                pChainTravel = aReferenceIterHelper.next();

            // reached the end of the chain, and pChainTravel points to the last element
            const_cast< SQLException* >( pChainTravel )->NextException = _rChainRight;
        }
    }

    //--------------------------------------------------------------------
    WarningsContainer::~WarningsContainer()
    {
    }

    //--------------------------------------------------------------------
    void WarningsContainer::appendWarning(const SQLException& _rWarning)
    {
        lcl_concatWarnings( m_aOwnWarnings, makeAny( _rWarning ) );
    }

    //--------------------------------------------------------------------
    void WarningsContainer::appendWarning( const SQLContext& _rContext )
    {
        lcl_concatWarnings( m_aOwnWarnings, makeAny( _rContext ));
    }

    //--------------------------------------------------------------------
    void WarningsContainer::appendWarning(const SQLWarning& _rWarning)
    {
        lcl_concatWarnings( m_aOwnWarnings, makeAny( _rWarning ) );
    }

    //--------------------------------------------------------------------
    Any SAL_CALL WarningsContainer::getWarnings(  ) const
    {
        Any aAllWarnings;
        if ( m_xExternalWarnings.is() )
            aAllWarnings = m_xExternalWarnings->getWarnings();

        if ( m_aOwnWarnings.hasValue() )
            lcl_concatWarnings( aAllWarnings, m_aOwnWarnings );

        return aAllWarnings;
    }

    //--------------------------------------------------------------------
    void SAL_CALL WarningsContainer::clearWarnings(  )
    {
        if ( m_xExternalWarnings.is() )
            m_xExternalWarnings->clearWarnings();
        m_aOwnWarnings.clear();
    }

    //--------------------------------------------------------------------
    void WarningsContainer::appendWarning( const ::rtl::OUString& _rWarning, const sal_Char* _pAsciiSQLState, const Reference< XInterface >& _rxContext )
    {
        appendWarning( SQLWarning( _rWarning, _rxContext, ::rtl::OUString::createFromAscii( _pAsciiSQLState ), 0, Any() ) );
    }

//........................................................................
}   // namespace dbtools
//........................................................................
