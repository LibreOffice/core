/*************************************************************************
 *
 *  $RCSfile: X11_service.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: pl $ $Date: 2001-09-11 11:23:56 $
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
#ifndef _CPPUHELPER_COMPBASE1_HXX_
#include <cppuhelper/compbase1.hxx>
#endif

using namespace rtl;
using namespace cppu;
using namespace com::sun::star::lang;
using namespace com::sun::star::datatransfer::clipboard;
using namespace com::sun::star::awt;
using namespace x11;

namespace x11 {

class X11ClipboardFactory : public ::cppu::WeakComponentImplHelper1 <
    ::com::sun::star::lang::XSingleServiceFactory
>
{
    ::osl::Mutex m_aMutex;
    ::std::hash_map< OUString, ::std::hash_map< Atom, Reference< XClipboard > >, ::rtl::OUStringHash > m_aInstances;
public:
    X11ClipboardFactory();
    virtual ~X11ClipboardFactory();

    /*
     *  XSingleServiceFactory
     */
    virtual Reference< XInterface > createInstance() throw();
    virtual Reference< XInterface > createInstanceWithArguments( const Sequence< Any >& rArgs ) throw();
};

// ------------------------------------------------------------------------

X11ClipboardFactory::X11ClipboardFactory() :
        ::cppu::WeakComponentImplHelper1<
    ::com::sun::star::lang::XSingleServiceFactory
>( m_aMutex )
{
}

// ------------------------------------------------------------------------

X11ClipboardFactory::~X11ClipboardFactory()
{
}

// ------------------------------------------------------------------------

Reference< XInterface > X11ClipboardFactory::createInstance() throw()
{
    return createInstanceWithArguments( Sequence< Any >() );
}

// ------------------------------------------------------------------------

Reference< XInterface > X11ClipboardFactory::createInstanceWithArguments( const Sequence< Any >& arguments ) throw()
{
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
        nSelection = rManager.getAtom( OUString::createFromAscii( "CLIPBOARD" ) );
    }

    ::std::hash_map< Atom, Reference< XClipboard > >& rMap( m_aInstances[ aDisplayName ] );
    ::std::hash_map< Atom, Reference< XClipboard > >::iterator it = rMap.find( nSelection );
    if( it != rMap.end() )
        return it->second;

    X11Clipboard* pClipboard = new X11Clipboard( rManager, nSelection );
    rMap[ nSelection ] = pClipboard;

    return static_cast<OWeakObject*>(pClipboard);
}

// ------------------------------------------------------------------------

Sequence< OUString > SAL_CALL X11Clipboard_getSupportedServiceNames()
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii("com.sun.star.datatransfer.clipboard.SystemClipboard");
    return aRet;
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
    return Reference < XInterface >( ( OWeakObject * ) new SelectionManagerHolder() );
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
                xFactory = new X11ClipboardFactory();
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
