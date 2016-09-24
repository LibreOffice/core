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


#include "HTerminateListener.hxx"
#include "hsqldb/HDriver.hxx"


namespace connectivity
{

    using namespace hsqldb;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::lang;

// XEventListener
void SAL_CALL OConnectionController::disposing( const EventObject& /*Source*/ )
throw( RuntimeException, std::exception )
{
}

// XTerminateListener
void SAL_CALL OConnectionController::queryTermination( const EventObject& /*aEvent*/ )
throw( TerminationVetoException, RuntimeException, std::exception )
{
    m_pDriver->flushConnections();
}

void SAL_CALL OConnectionController::notifyTermination( const EventObject& /*aEvent*/ )
throw( RuntimeException, std::exception )
{
    m_pDriver->shutdownConnections();
}


}   // namespace connectivity


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
