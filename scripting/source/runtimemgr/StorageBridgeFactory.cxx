/*************************************************************************
*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: StorageBridgeFactory.cxx,v $
 * $Revision: 1.8 $
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
