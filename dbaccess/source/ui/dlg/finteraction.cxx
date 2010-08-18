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

#ifndef DBAUI_FILEPICKER_INTERACTION_HXX
#include "finteraction.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVEIOEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveIOException.hpp>
#endif

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
        DBG_ASSERT( m_xMaster.is(), "OFilePickerInteractionHandler::OFilePickerInteractionHandler: invalid master handler!" );
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

