/*************************************************************************
 *
 *  $RCSfile: X11_service.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obr $ $Date: 2001-02-14 10:40:53 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <X11_clipboard.hxx>

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif

#ifndef _UNO_DISPATCHER_H_
#include <uno/dispatcher.h> // declaration of generic uno interface
#endif
#ifndef _UNO_MAPPING_HXX_
#include <uno/mapping.hxx> // mapping stuff
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif

using namespace rtl;
using namespace cppu;
using namespace com::sun::star::lang;
using namespace x11;

namespace x11 {

Sequence< OUString > SAL_CALL X11Clipboard_getSupportedServiceNames()
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii("com.sun.star.datatransfer.clipboard.SystemClipboard");
    return aRet;
}

// ------------------------------------------------------------------------

Reference< XInterface > SAL_CALL X11Clipboard_createInstance(
    const Reference< XMultiServiceFactory > & xMultiServiceFactory)
{
    return Reference < XInterface >( ( OWeakObject * ) new X11Clipboard());
}

// ------------------------------------------------------------------------

Sequence< OUString > SAL_CALL Xdnd_getSupportedServiceNames()
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii("com.sun.star.datatransfer.dnd.X11DragSource");
    return aRet;
}

// ------------------------------------------------------------------------

Reference< XInterface > SAL_CALL Xdnd_createInstance(
    const Reference< XMultiServiceFactory > & xMultiServiceFactory)
{
    return Reference < XInterface >( ( OWeakObject * )& SelectionManager::get() );
}

// ------------------------------------------------------------------------

Sequence< OUString > SAL_CALL Xdnd_dropTarget_getSupportedServiceNames()
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii("com.sun.star.datatransfer.dnd.X11DropTarget");
    return aRet;
}

// ------------------------------------------------------------------------

Reference< XInterface > SAL_CALL Xdnd_dropTarget_createInstance(
    const Reference< XMultiServiceFactory > & xMultiServiceFactory)
{
    return Reference < XInterface >( ( OWeakObject * ) new DropTarget() );
}

}

static const OUString& getClipboardImplementationName()
{
    static OUString aImpl = OUString::createFromAscii(X11_CLIPBOARD_IMPLEMENTATION_NAME);
    return aImpl;
}

static const OUString& getClipboardServiceName()
{
    static OUString aImpl = OUString::createFromAscii("com.sun.star.datatransfer.clipboard.SystemClipboard" );
    return aImpl;
}

static const OUString& getXdndImplementationName()
{
    static OUString aImpl = OUString::createFromAscii(XDND_IMPLEMENTATION_NAME );
    return aImpl;
}

static const OUString& getXdndServiceName()
{
    static OUString aImpl = OUString::createFromAscii("com.sun.star.datatransfer.dnd.X11DragSource" );
    return aImpl;
}

static const OUString& getXdndDropTargetImplementationName()
{
    static OUString aImpl = OUString::createFromAscii(XDND_DROPTARGET_IMPLEMENTATION_NAME);
    return aImpl;
}

static const OUString& getXdndDropTargetServiceName()
{
    static OUString aImpl = OUString::createFromAscii("com.sun.star.datatransfer.dnd.X11DropTarget" );
    return aImpl;
}

extern "C" {
    void SAL_CALL component_getImplementationEnvironment(
        const sal_Char** ppEnvTypeName,
        uno_Environment** ppEnv )
    {
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }

    sal_Bool SAL_CALL component_writeInfo( void* pServiceManager, void* pXUnoKey )
    {
        if( pXUnoKey )
        {
            try
            {
                Reference< ::com::sun::star::registry::XRegistryKey > xKey( reinterpret_cast< ::com::sun::star::registry::XRegistryKey* >( pXUnoKey ) );

                OUString aImplName = OUString::createFromAscii( "/" );
                aImplName += getClipboardImplementationName();
                aImplName += ::rtl::OUString::createFromAscii( "/UNO/SERVICES/" );
                aImplName += getClipboardServiceName();
                xKey->createKey( aImplName );

                aImplName = OUString::createFromAscii( "/" );
                aImplName += getXdndImplementationName();
                aImplName += ::rtl::OUString::createFromAscii( "/UNO/SERVICES/" );
                aImplName += getXdndServiceName();
                xKey->createKey( aImplName );

                aImplName = OUString::createFromAscii( "/" );
                aImplName += getXdndDropTargetImplementationName();
                aImplName += ::rtl::OUString::createFromAscii( "/UNO/SERVICES/" );
                aImplName += getXdndDropTargetServiceName();
                xKey->createKey( aImplName );

                return sal_True;
            }
            catch( ::com::sun::star::registry::InvalidRegistryException& )
            {
            }
        }
        return sal_False;
    }

    void* SAL_CALL component_getFactory(
        const sal_Char* pImplementationName,
        void* pXUnoSMgr,
        void* pXUnoKey
        )
    {
        void* pRet = 0;

        ::rtl::OUString aImplName( ::rtl::OUString::createFromAscii( pImplementationName ) );

        if( pXUnoSMgr )
        {
            Reference< ::com::sun::star::lang::XMultiServiceFactory > xMgr(
                reinterpret_cast< ::com::sun::star::lang::XMultiServiceFactory* >( pXUnoSMgr )
                );
            Reference< ::com::sun::star::lang::XSingleServiceFactory > xFactory;
            if( aImplName.equals( getClipboardImplementationName() ) )
            {
                xFactory = ::cppu::createSingleFactory(
                    xMgr, aImplName, X11Clipboard_createInstance,
                    X11Clipboard_getSupportedServiceNames() );
            }
            else if( aImplName.equals( getXdndImplementationName() ) )
            {
                xFactory = ::cppu::createSingleFactory(
                    xMgr, aImplName, Xdnd_createInstance,
                    Xdnd_getSupportedServiceNames() );
            }
            else if( aImplName.equals( getXdndDropTargetImplementationName() ) )
            {
                xFactory = ::cppu::createSingleFactory(
                    xMgr, aImplName, Xdnd_dropTarget_createInstance,
                    Xdnd_dropTarget_getSupportedServiceNames() );
            }
            if( xFactory.is() )
            {
                xFactory->acquire();
                pRet = xFactory.get();
            }
        }
        return pRet;
    }

} /* extern "C" */
