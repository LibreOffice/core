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
#include "precompiled_xmlhelp.hxx"
#include <com/sun/star/ucb/Command.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>

#include "resultsetforroot.hxx"
#include "databases.hxx"

using namespace chelp;
using namespace com::sun::star;
using namespace com::sun::star::ucb;



ResultSetForRoot::ResultSetForRoot( const uno::Reference< lang::XMultiServiceFactory >&  xMSF,
                                    const uno::Reference< XContentProvider >&  xProvider,
                                    sal_Int32 nOpenMode,
                                    const uno::Sequence< beans::Property >& seq,
                                    const uno::Sequence< NumberedSortingInfo >& seqSort,
                                    URLParameter& aURLParameter,
                                    Databases* pDatabases )
    : ResultSetBase( xMSF,xProvider,nOpenMode,seq,seqSort ),
      m_aURLParameter( aURLParameter ),
      m_pDatabases( pDatabases )
{
    m_aPath = m_pDatabases->getModuleList( m_aURLParameter.get_language() );
    m_aItems.resize( m_aPath.size() );
    m_aIdents.resize( m_aPath.size() );

    Command aCommand;
    aCommand.Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "getPropertyValues" ));
    aCommand.Argument <<= m_sProperty;

    for( sal_uInt32 i = 0; i < m_aPath.size(); ++i )
    {
        m_aPath[i] =
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.help://" )) +
            m_aPath[i]                                               +
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "?Language=" ))           +
            m_aURLParameter.get_language()                           +
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "&System=" ))             +
            m_aURLParameter.get_system();

        m_nRow = sal_Int32( i );

        uno::Reference< XContent > content = queryContent();
        if( content.is() )
        {
            uno::Reference< XCommandProcessor > cmd( content,uno::UNO_QUERY );
            cmd->execute( aCommand,0,uno::Reference< XCommandEnvironment >( 0 ) ) >>= m_aItems[i]; //TODO: check return value of operator >>=
        }
        m_nRow = 0xffffffff;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
