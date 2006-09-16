/*************************************************************************
*
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StorageBridgeFactory.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 12:30:32 $
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
#include "precompiled_scripting.hxx"


#include "StorageBridgeFactory.hxx"

#include "StorageBridge.hxx"

using namespace ::drafts::com::sun::star::script::framework;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

namespace scripting_runtimemgr
{

//*************************************************************************
StorageBridgeFactory::StorageBridgeFactory(
    const Reference< XComponentContext >& xContext ) :
    m_xContext( xContext )
{}



//*************************************************************************
// StorageBridge is a bridge between us an storage, the StorageBridge implements the
// XScriptInfoAccess
// interface, it allows both the "real" storage and a java test storage component to be
// configured.
// @param location contains a url for the document.
// @returns a storage instance for the location "location".
// @see StorageBridge.cxx  for flag "STORAGEPROXY", set to use java "test" component or
// use "real" storage.

Reference< storage::XScriptInfoAccess >
StorageBridgeFactory::getStorageInstance( sal_Int32 sid )
{

    Reference< storage::XScriptInfoAccess > storage = new StorageBridge( m_xContext,
            sid ) ;
    return storage;
}
} //scripting_runtimemgr
