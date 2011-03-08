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
#include "precompiled_vcl.hxx"

#include "salinst.h"

#include <X11_clipboard.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <uno/dispatcher.h> // declaration of generic uno interface
#include <uno/mapping.hxx> // mapping stuff
#include <cppuhelper/factory.hxx>
#include <cppuhelper/compbase1.hxx>

using namespace cppu;
using namespace com::sun::star::lang;
using namespace com::sun::star::datatransfer::clipboard;
using namespace com::sun::star::awt;
using namespace x11;

using ::rtl::OUString;

Sequence< OUString > SAL_CALL x11::X11Clipboard_getSupportedServiceNames()
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.datatransfer.clipboard.SystemClipboard"));
    return aRet;
}

Sequence< OUString > SAL_CALL x11::Xdnd_getSupportedServiceNames()
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.datatransfer.dnd.X11DragSource"));
    return aRet;
}

Sequence< OUString > SAL_CALL x11::Xdnd_dropTarget_getSupportedServiceNames()
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.datatransfer.dnd.X11DropTarget"));
    return aRet;
}

// ------------------------------------------------------------------------

Reference< XInterface > X11SalInstance::CreateClipboard( const Sequence< Any >& arguments )
{
    static boost::unordered_map< OUString, ::boost::unordered_map< Atom, Reference< XClipboard > >, ::rtl::OUStringHash > m_aInstances;

    OUString aDisplayName;
    Atom nSelection;

    // extract display name from connection argument. An exception is thrown
    // by SelectionManager.initialize() if no display connection is given.
    if( arguments.getLength() > 0 )
    {
        Reference< XDisplayConnection > xConn;
        arguments.getConstArray()[0] >>= xConn;

        if( xConn.is() )
        {
            Any aIdentifier = xConn->getIdentifier();
            aIdentifier >>= aDisplayName;
        }
    }

    SelectionManager& rManager = SelectionManager::get( aDisplayName );
    rManager.initialize( arguments );

    // check if any other selection than clipboard selection is specified
    if( arguments.getLength() > 1 )
    {
        OUString aSelectionName;

        arguments.getConstArray()[1] >>= aSelectionName;
        nSelection = rManager.getAtom( aSelectionName );
    }
    else
    {
        // default atom is clipboard selection
        nSelection = rManager.getAtom( OUString(RTL_CONSTASCII_USTRINGPARAM("CLIPBOARD")) );
    }

    ::boost::unordered_map< Atom, Reference< XClipboard > >& rMap( m_aInstances[ aDisplayName ] );
    ::boost::unordered_map< Atom, Reference< XClipboard > >::iterator it = rMap.find( nSelection );
    if( it != rMap.end() )
        return it->second;

    X11Clipboard* pClipboard = new X11Clipboard( rManager, nSelection );
    rMap[ nSelection ] = pClipboard;

    return static_cast<OWeakObject*>(pClipboard);
}

// ------------------------------------------------------------------------

Reference< XInterface > X11SalInstance::CreateDragSource()
{
    return Reference < XInterface >( ( OWeakObject * ) new SelectionManagerHolder() );
}

// ------------------------------------------------------------------------

Reference< XInterface > X11SalInstance::CreateDropTarget()
{
    return Reference < XInterface >( ( OWeakObject * ) new DropTarget() );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
