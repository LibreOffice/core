/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: warnings.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 15:17:32 $
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

#ifndef DBA_CORE_WARNINGS_HXX
#include "warnings.hxx"
#endif

#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

//........................................................................
namespace dbaccess
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdb;
    using namespace ::dbtools;

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
}   // namespace dbaccess
//........................................................................
