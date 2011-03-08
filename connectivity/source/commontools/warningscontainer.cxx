/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
