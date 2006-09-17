/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: finteraction.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 07:09:58 $
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

