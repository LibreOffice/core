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
#include "pkgdatasupplier.hxx"
#include "pkgresultset.hxx"
#include <comphelper/processfactory.hxx>

using namespace com::sun::star;

using namespace package_ucp;




// DynamicResultSet Implementation.




DynamicResultSet::DynamicResultSet(
              const uno::Reference< uno::XComponentContext >& rxContext,
              const rtl::Reference< Content >& rxContent,
              const ucb::OpenCommandArgument2& rCommand,
              const uno::Reference< ucb::XCommandEnvironment >& rxEnv )
: ResultSetImplHelper(rxContext, rCommand ),
  m_xContent( rxContent ),
  m_xEnv( rxEnv )
{
}



// Non-interface methods.



void DynamicResultSet::initStatic()
{
    m_xResultSet1
        = new ::ucbhelper::ResultSet( m_xContext,
                                      m_aCommand.Properties,
                                      new DataSupplier( m_xContext,
                                                        m_xContent ),
                                      m_xEnv );
}


void DynamicResultSet::initDynamic()
{
    m_xResultSet1
        = new ::ucbhelper::ResultSet( m_xContext,
                                      m_aCommand.Properties,
                                      new DataSupplier( m_xContext,
                                                        m_xContent ),
                                      m_xEnv );
    m_xResultSet2 = m_xResultSet1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
