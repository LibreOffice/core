/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: listenercontainers.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 23:58:05 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_forms.hxx"

#ifndef FORMS_SOURCE_MISC_LISTENERCONTAINERS_HXX
#include "listenercontainers.hxx"
#endif

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
            OSL_ENSURE( sal_False, "ResetListeners::implNotify: invalid notification type!" );
        }
        return true;
    }

//........................................................................
} // namespace frm
//........................................................................

