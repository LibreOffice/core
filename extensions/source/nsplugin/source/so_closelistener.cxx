/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: so_closelistener.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 14:45:00 $
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
#include "precompiled_extensions.hxx"

#include "so_closelistener.hxx"

using namespace ::com::sun::star;

void SAL_CALL PluginDocumentClosePreventer::queryClosing( const lang::EventObject& /*aEvent*/, sal_Bool /*bDeliverOwnership*/ )
        throw ( uno::RuntimeException, util::CloseVetoException )
{
    if ( m_bPreventClose )
        throw util::CloseVetoException();
}

void SAL_CALL PluginDocumentClosePreventer::notifyClosing( const lang::EventObject& /*aEvent*/ ) throw ( uno::RuntimeException )
{}

void SAL_CALL PluginDocumentClosePreventer::disposing( const lang::EventObject& /*aEvent*/ ) throw ( uno::RuntimeException )
{}


