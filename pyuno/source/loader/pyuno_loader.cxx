/*************************************************************************
 *
 *  $RCSfile: pyuno_loader.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jbu $ $Date: 2003-04-06 17:15:16 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Ralph Thomas
 *
 *   Copyright: 2000 by Sun Microsystems, Inc.
 *
 *   All Rights Reserved.
 *
 *   Contributor(s): Ralph Thomas, Joerg Budischewski
 *
 *
 ************************************************************************/
#include <osl/module.hxx>
#include <osl/process.h>
#include <osl/file.h>
#include <osl/thread.h>

#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/bootstrap.hxx>

#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/factory.hxx>

#include <pyuno/pyuno.hxx>

using rtl::OUString;
using rtl::OUStringBuffer;
using rtl::OString;

using pyuno::PyRef;
using pyuno::Runtime;
using pyuno::PyThreadAttach;

using com::sun::star::registry::XRegistryKey;
using com::sun::star::uno::Reference;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::XComponentContext;
using com::sun::star::uno::RuntimeException;

namespace pyuno_loader
{

static void raiseRuntimeExceptionWhenNeeded() throw ( RuntimeException )
{
    if( PyErr_Occurred() )
    {
        PyRef excType, excValue, excTraceback;
        PyErr_Fetch( (PyObject **)&excType, (PyObject**)&excValue,(PyObject**)&excTraceback);
        if( excType.is() )
        {
            PyRef str( PyObject_Repr( excTraceback.get() ), SAL_NO_ACQUIRE );
            OUStringBuffer buf;
            buf.appendAscii( "python-loader:" );
            PyRef valueRep( PyObject_Repr( excValue.get() ), SAL_NO_ACQUIRE );
            buf.appendAscii( PyString_AsString( valueRep.get())).appendAscii( ", traceback follows\n" );
            buf.appendAscii( PyString_AsString( str.get() ) );
            throw RuntimeException( buf.makeStringAndClear(), Reference< XInterface> () );
        }
    }
}

static PyRef getLoaderModule() throw( RuntimeException )
{
    PyRef module( PyImport_ImportModule( "pythonloader" ), SAL_NO_ACQUIRE );
    raiseRuntimeExceptionWhenNeeded();
    if( !module.is() )
    {
        throw RuntimeException(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "pythonloader: Couldn't load pythonloader module" ) ),
            Reference< XInterface > () );
    }
    return PyRef( PyModule_GetDict( module.get() ));
}

static PyRef getObjectFromLoaderModule( const char * func )
    throw ( RuntimeException )
{
    PyRef object( PyDict_GetItemString(getLoaderModule().get(), (char*)func ) );
    if( !object.is() )
    {
        OUStringBuffer buf;
        buf.appendAscii( "pythonloader: couldn't find core element pythonloader." );
        buf.appendAscii( func );
        throw RuntimeException(buf.makeStringAndClear(),Reference< XInterface >());
    }
    return object;
}

OUString getImplementationName()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.comp.pyuno.Loader" ) );
}

Sequence< OUString > getSupportedServiceNames()
{
    OUString serviceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.loader.Python" ) );
    return Sequence< OUString > ( &serviceName, 1 );
}

static OUString getLibDir()
{
    static OUString *pLibDir;
    if( !pLibDir )
    {
        osl::MutexGuard guard( osl::Mutex::getGlobalMutex() );
        if( ! pLibDir )
        {
            static OUString libDir;

            if( osl::Module::getUrlFromAddress( reinterpret_cast<void*>(getLibDir) , libDir ) )
            {
                libDir = OUString( libDir.getStr(), libDir.lastIndexOf('/' ) );
                OUString name ( RTL_CONSTASCII_USTRINGPARAM( "PYTHONLOADERLIBDIR" ) );
                rtl_bootstrap_set( name.pData, libDir.pData );
            }
            pLibDir = &libDir;
        }
    }
    return *pLibDir;
}

Reference< XInterface > CreateInstance( const Reference< XComponentContext > & ctx )
{
    Reference< XInterface > ret;

    if( ! Py_IsInitialized() )
    {
        // in case python path is already set, nothing is done ...
        const OUString pythonPath ( RTL_CONSTASCII_USTRINGPARAM( "PYTHONPATH" ) );
        OUString value;
        if( osl_Process_E_None != osl_getEnvironment( pythonPath.pData,  &value.pData ) ||
            value.getLength() == 0 )
        {
            // otherwise, try to get the PYTHONPATH bootstrap variable
            OUString path = getLibDir();
            if( path.getLength() )
            {
                path += OUString( RTL_CONSTASCII_USTRINGPARAM( "/" SAL_CONFIGFILE("pythonloader.uno" )));
                rtl::Bootstrap bootstrap(path);

                OUString pythonPathBootstrap;
                bootstrap.getFrom( pythonPath , pythonPathBootstrap );

                OUStringBuffer buf( pythonPathBootstrap.getLength() );
                sal_Int32 nIndex = 0;
                while( 1 )
                {
                    sal_Int32 nNew = pythonPathBootstrap.indexOf( ' ', nIndex );
                    OUString fileUrl;
                    if( nNew == -1 )
                    {
                        fileUrl = OUString( &( pythonPathBootstrap[nIndex] ) );
                    }
                    else
                    {
                        fileUrl = OUString( &(pythonPathBootstrap[nIndex]) , nNew - nIndex );
                    }
                    OUString systemPath;
                    osl_getSystemPathFromFileURL( fileUrl.pData, &(systemPath.pData) );
                    buf.append( (sal_Unicode) SAL_PATHSEPARATOR );
                    buf.append( systemPath );
                    if( nNew == -1 )
                        break;
                    nIndex = nNew + 1;
                }

                rtl::OStringBuffer stringBuffer;
                stringBuffer.append( "PYTHONPATH=" );
                stringBuffer.append(
                    rtl::OUStringToOString( buf.makeStringAndClear(), osl_getThreadTextEncoding()));

                OString env = stringBuffer.makeStringAndClear();

                // leak this string (putenv does not make a copy)
                rtl_string_acquire( env.pData );
                putenv( env.pData->buffer );


                // look for pythonhome
                OUString pythonHome;
                if( bootstrap.getFrom( OUString ( RTL_CONSTASCII_USTRINGPARAM( "PYTHONHOME") ),
                                       pythonHome ) )
                {
                    osl_getFileURLFromSystemPath( pythonHome.pData, &(pythonHome.pData) );
                    rtl::OStringBuffer stringBuffer( pythonHome.getLength() +20);
                    stringBuffer.append( "PYTHONHOME=" );
                    stringBuffer.append(
                        rtl::OUStringToOString( pythonHome, osl_getThreadTextEncoding() ) );

                    OString env2= stringBuffer.makeStringAndClear();
                    rtl_string_acquire(env2.pData );
                    putenv( env2.pData->buffer );

                }
            }
        }

        Py_Initialize();
        PyEval_InitThreads();

        PyThreadState *tstate = PyThreadState_Get();
        PyEval_ReleaseThread( tstate );
    }

    PyThreadAttach attach( PyInterpreterState_Head() );
    {
        if( ! Runtime::isInitialized() )
        {
            Runtime::initialize( ctx );
        }
        Runtime runtime;

        PyRef pyCtx = runtime.any2PyObject(
            com::sun::star::uno::makeAny( ctx ) );

        PyRef clazz = getObjectFromLoaderModule( "Loader" );
        PyRef args ( PyTuple_New( 1 ), SAL_NO_ACQUIRE );
        PyTuple_SetItem( args.get(), 0 , pyCtx.getAcquired() );
        PyRef pyInstance( PyObject_CallObject( clazz.get() , args.get() ), SAL_NO_ACQUIRE );
        runtime.pyObject2Any( pyInstance ) >>= ret;
    }
    return ret;
}

}


static struct cppu::ImplementationEntry g_entries[] =
{
    {
        pyuno_loader::CreateInstance, pyuno_loader::getImplementationName,
        pyuno_loader::getSupportedServiceNames, cppu::createSingleComponentFactory,
        0 , 0
    },
    { 0, 0, 0, 0, 0, 0 }
};

extern "C"
{

//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    return cppu::component_writeInfoHelper( pServiceManager, pRegistryKey, g_entries );
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return cppu::component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , g_entries );
}

}

