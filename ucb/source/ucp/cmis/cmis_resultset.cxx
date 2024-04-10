/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "cmis_datasupplier.hxx"
#include "cmis_resultset.hxx"

using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;

namespace cmis
{
    DynamicResultSet::DynamicResultSet(
        const Reference< XComponentContext >& rxContext,
        ChildrenProvider* pChildrenProvider,
        const OpenCommandArgument2& rCommand,
        const Reference< XCommandEnvironment >& rxEnv ) :
            ResultSetImplHelper( rxContext, rCommand ),
            m_pChildrenProvider( pChildrenProvider ),
            m_xEnv( rxEnv )
    {
    }

    void DynamicResultSet::initStatic()
    {
        m_xResultSet1 = new ::ucbhelper::ResultSet(
            m_xContext, m_aCommand.Properties,
            new DataSupplier( m_pChildrenProvider, m_aCommand.Mode ), m_xEnv );
    }

    void DynamicResultSet::initDynamic()
    {
        initStatic();
        m_xResultSet2 = m_xResultSet1;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
