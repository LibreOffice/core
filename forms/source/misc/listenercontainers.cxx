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
#include "precompiled_forms.hxx"

#include "listenercontainers.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

//........................................................................
namespace frm
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::form;

    //====================================================================
    //= ResetListeners
    //====================================================================
    //---------------------------------------------------------------------
    bool ResetListeners::implTypedNotify( const Reference< XResetListener >& _rxListener,
        const EventObject& _rEvent )   SAL_THROW( ( Exception ) )
    {
        switch ( m_eCurrentNotificationType )
        {
        case eApproval:
            if ( !_rxListener->approveReset( _rEvent ) )
                return false;
            return true;
        case eFinal:
            _rxListener->resetted( _rEvent );
            break;
        default:
            OSL_FAIL( "ResetListeners::implNotify: invalid notification type!" );
        }
        return true;
    }

//........................................................................
} // namespace frm
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
