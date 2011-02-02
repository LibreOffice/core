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
#include "precompiled_dbaccess.hxx"

#include "finteraction.hxx"
#include <tools/debug.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/ucb/InteractiveIOException.hpp>

//........................................................................
namespace dbaui
{
//........................................................................
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::task;
    using namespace ::com::sun::star::ucb;

    //====================================================================
    //= OFilePickerInteractionHandler
    //====================================================================
    DBG_NAME( OFilePickerInteractionHandler )
    //--------------------------------------------------------------------
    OFilePickerInteractionHandler::OFilePickerInteractionHandler( const Reference< XInteractionHandler >& _rxMaster )
        :m_xMaster( _rxMaster )
        ,m_bDoesNotExist(sal_False)
    {
        DBG_CTOR( OFilePickerInteractionHandler, NULL );
        OSL_ENSURE( m_xMaster.is(), "OFilePickerInteractionHandler::OFilePickerInteractionHandler: invalid master handler!" );
    }

    //--------------------------------------------------------------------
    OFilePickerInteractionHandler::~OFilePickerInteractionHandler( )
    {
        DBG_DTOR( OFilePickerInteractionHandler, NULL );
    }

    //--------------------------------------------------------------------
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

//........................................................................
}   // namespace svt
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
