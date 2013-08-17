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

/**************************************************************************
                                TODO
 **************************************************************************

 - This implementation is not a dynamic result set!!! It only implements
   the necessary interfaces, but never recognizes/notifies changes!!!

 *************************************************************************/

#include "myucp_datasupplier.hxx"
#include "myucp_resultset.hxx"
#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::container;

using namespace dbaccess;

// DynamicResultSet Implementation.
DynamicResultSet::DynamicResultSet(
                      const Reference< XComponentContext >& rxContext,
                      const rtl::Reference< ODocumentContainer >& rxContent,
                      const OpenCommandArgument2& rCommand,
                      const Reference< XCommandEnvironment >& rxEnv )
    :ResultSetImplHelper( rxContext, rCommand )
    ,m_xContent(rxContent)
    ,m_xEnv( rxEnv )
{
}

// Non-interface methods.
void DynamicResultSet::initStatic()
{
    m_xResultSet1
        = new ::ucbhelper::ResultSet( m_xContext,
                                      m_aCommand.Properties,
                                      new DataSupplier( m_xContent,
                                                        m_aCommand.Mode ),
                                      m_xEnv );
}

void DynamicResultSet::initDynamic()
{
    m_xResultSet1
        = new ::ucbhelper::ResultSet( m_xContext,
                                      m_aCommand.Properties,
                                      new DataSupplier( m_xContent,
                                                        m_aCommand.Mode ),
                                      m_xEnv );
    m_xResultSet2 = m_xResultSet1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
