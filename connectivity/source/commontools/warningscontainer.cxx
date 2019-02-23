/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <connectivity/warningscontainer.hxx>
#include <connectivity/dbexception.hxx>

#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>

#include <o3tl/any.hxx>
#include <osl/diagnose.h>


namespace dbtools
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdb;

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

            const SQLException* pChainTravel = o3tl::doAccess<SQLException>( _rChainLeft );
            SQLExceptionIteratorHelper aReferenceIterHelper( *pChainTravel );
            while ( aReferenceIterHelper.hasMoreElements() )
                pChainTravel = aReferenceIterHelper.next();

            // reached the end of the chain, and pChainTravel points to the last element
            const_cast< SQLException* >( pChainTravel )->NextException = _rChainRight;
        }
    }


    void WarningsContainer::appendWarning(const SQLException& _rWarning)
    {
        lcl_concatWarnings( m_aOwnWarnings, makeAny( _rWarning ) );
    }


    void WarningsContainer::appendWarning( const SQLContext& _rContext )
    {
        lcl_concatWarnings( m_aOwnWarnings, makeAny( _rContext ));
    }


    void WarningsContainer::appendWarning(const SQLWarning& _rWarning)
    {
        lcl_concatWarnings( m_aOwnWarnings, makeAny( _rWarning ) );
    }


    Any WarningsContainer::getWarnings(  ) const
    {
        Any aAllWarnings;
        if ( m_xExternalWarnings.is() )
            aAllWarnings = m_xExternalWarnings->getWarnings();

        if ( m_aOwnWarnings.hasValue() )
            lcl_concatWarnings( aAllWarnings, m_aOwnWarnings );

        return aAllWarnings;
    }


    void WarningsContainer::clearWarnings(  )
    {
        if ( m_xExternalWarnings.is() )
            m_xExternalWarnings->clearWarnings();
        m_aOwnWarnings.clear();
    }


    void WarningsContainer::appendWarning( const OUString& _rWarning, const sal_Char* _pAsciiSQLState, const Reference< XInterface >& _rxContext )
    {
        appendWarning( SQLWarning( _rWarning, _rxContext, OUString::createFromAscii( _pAsciiSQLState ), 0, Any() ) );
    }


}   // namespace dbtools


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
