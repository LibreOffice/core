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

#include <com/sun/star/ucb/Command.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>

#include "resultsetforroot.hxx"
#include "databases.hxx"

using namespace chelp;
using namespace com::sun::star;
using namespace com::sun::star::ucb;



ResultSetForRoot::ResultSetForRoot( const uno::Reference< uno::XComponentContext >&  rxContext,
                                    const uno::Reference< XContentProvider >&  xProvider,
                                    const uno::Sequence< beans::Property >& seq,
                                    const uno::Sequence< NumberedSortingInfo >& seqSort,
                                    URLParameter& aURLParameter,
                                    Databases* pDatabases )
    : ResultSetBase( rxContext, xProvider,seq,seqSort ),
      m_aURLParameter( aURLParameter ),
      m_pDatabases( pDatabases )
{
    m_aPath = m_pDatabases->getModuleList( m_aURLParameter.get_language() );
    m_aItems.resize( m_aPath.size() );
    m_aIdents.resize( m_aPath.size() );

    Command aCommand;
    aCommand.Name = "getPropertyValues";
    aCommand.Argument <<= m_sProperty;

    for( size_t i = 0; i < m_aPath.size(); ++i )
    {
        m_aPath[i] =
            "vnd.sun.star.help://" +
            m_aPath[i] +
            "?Language=" +
            m_aURLParameter.get_language() +
            "&System=" +
            m_aURLParameter.get_system();

        m_nRow = sal_Int32( i );

        uno::Reference< XContent > content = queryContent();
        if( content.is() )
        {
            uno::Reference< XCommandProcessor > cmd( content,uno::UNO_QUERY );
            cmd->execute( aCommand,0,uno::Reference< XCommandEnvironment >( nullptr ) ) >>= m_aItems[i]; //TODO: check return value of operator >>=
        }
        m_nRow = 0xffffffff;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
