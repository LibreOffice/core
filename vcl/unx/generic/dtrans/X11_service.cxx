/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include "unx/salinst.h"

#include <X11_clipboard.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <uno/dispatcher.h> // declaration of generic uno interface
#include <uno/mapping.hxx> // mapping stuff
#include <cppuhelper/factory.hxx>
#include <cppuhelper/compbase1.hxx>

namespace {

namespace css = com::sun::star;

}

using namespace rtl;
using namespace cppu;
using namespace com::sun::star::lang;
using namespace com::sun::star::datatransfer::clipboard;
using namespace com::sun::star::awt;
using namespace x11;

Sequence< OUString > SAL_CALL x11::X11Clipboard_getSupportedServiceNames()
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii("com.sun.star.datatransfer.clipboard.SystemClipboard");
    return aRet;
}

Sequence< OUString > SAL_CALL x11::Xdnd_getSupportedServiceNames()
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii("com.sun.star.datatransfer.dnd.X11DragSource");
    return aRet;
}

Sequence< OUString > SAL_CALL x11::Xdnd_dropTarget_getSupportedServiceNames()
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii("com.sun.star.datatransfer.dnd.X11DropTarget");
    return aRet;
}

// ------------------------------------------------------------------------

css::uno::Reference< XInterface > X11SalInstance::CreateClipboard( const Sequence< Any >& arguments )
{
    static std::hash_map< OUString, ::std::hash_map< Atom, css::uno::Reference< XClipboard > >, ::rtl::OUStringHash > m_aInstances;

    OUString aDisplayName;
    Atom nSelection;

    // extract display name from connection argument. An exception is thrown
    // by SelectionManager.initialize() if no display connection is given.
    if( arguments.getLength() > 0 )
    {
        css::uno::Reference< XDisplayConnection > xConn;
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
        nSelection = rManager.getAtom( OUString::createFromAscii( "CLIPBOARD" ) );
    }

    ::std::hash_map< Atom, css::uno::Reference< XClipboard > >& rMap( m_aInstances[ aDisplayName ] );
    ::std::hash_map< Atom, css::uno::Reference< XClipboard > >::iterator it = rMap.find( nSelection );
    if( it != rMap.end() )
        return it->second;

    X11Clipboard* pClipboard = new X11Clipboard( rManager, nSelection );
    rMap[ nSelection ] = pClipboard;

    return static_cast<OWeakObject*>(pClipboard);
}

// ------------------------------------------------------------------------

css::uno::Reference< XInterface > X11SalInstance::CreateDragSource()
{
    return css::uno::Reference < XInterface >( ( OWeakObject * ) new SelectionManagerHolder() );
}

// ------------------------------------------------------------------------

css::uno::Reference< XInterface > X11SalInstance::CreateDropTarget()
{
    return css::uno::Reference < XInterface >( ( OWeakObject * ) new DropTarget() );
}


