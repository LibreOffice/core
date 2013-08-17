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

#include "finteraction.hxx"
#include <tools/debug.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/ucb/InteractiveIOException.hpp>

namespace dbaui
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::task;
    using namespace ::com::sun::star::ucb;

    // OFilePickerInteractionHandler
    DBG_NAME( OFilePickerInteractionHandler )
    OFilePickerInteractionHandler::OFilePickerInteractionHandler( const Reference< XInteractionHandler >& _rxMaster )
        :m_xMaster( _rxMaster )
        ,m_bDoesNotExist(sal_False)
    {
        DBG_CTOR( OFilePickerInteractionHandler, NULL );
        OSL_ENSURE( m_xMaster.is(), "OFilePickerInteractionHandler::OFilePickerInteractionHandler: invalid master handler!" );
    }

    OFilePickerInteractionHandler::~OFilePickerInteractionHandler( )
    {
        DBG_DTOR( OFilePickerInteractionHandler, NULL );
    }

    void SAL_CALL OFilePickerInteractionHandler::handle( const Reference< XInteractionRequest >& _rxRequest ) throw (RuntimeException)
    {
        InteractiveIOException aIoException;
        if ( _rxRequest->getRequest() >>= aIoException )
        {
            if ( IOErrorCode_NOT_EXISTING == aIoException.Code )
            {
                m_bDoesNotExist = sal_True;
                return;
            }
        }

        if ( m_xMaster.is() )
            m_xMaster->handle( _rxRequest );
    }

}   // namespace svt

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
