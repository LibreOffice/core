/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pyuno_loader.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2007-10-11 11:52:16 $
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

#include <pyuno/pyuno.hxx>

#include <osl/module.hxx>
#include <osl/process.h>
#include <osl/file.h>
#include <osl/thread.h>

#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/bootstrap.hxx>

#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/factory.hxx>

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
        Runtime runtime;
        com::sun::star::uno::Any a = runtime.extractUnoException( excType, excValue, excTraceback );
        OUStringBuffer buf;
        buf.appendAscii( "python-loader:" );
        if( a.hasValue() )
            buf.append( ((com::sun::star::uno::Exception *)a.getValue())->Message );
        throw RuntimeException( buf.makeStringAndClear(), Reference< XInterface> () );
    }
}

static PyRef getLoaderModule() throw( RuntimeException )
{
    PyRef module(
        PyImport_ImportModule( const_cast< char * >("pythonloader") ),
        SAL_NO_ACQUIRE );
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

            if( osl::Module::getUrlFromAddress(
                    reinterpret_cast< oslGenericFunction >(getLibDir) , libDir ) )
            {
                libDir = OUString( libDir.getStr(), libDir.lastIndexOf('/' ) );
            }
            pLibDir = &libDir;
        }
    }
    return *pLibDir;
}

static PyRef myOUString2PyString( const OUString &str )
{
    OString o = OUStringToOString( str, osl_getThreadTextEncoding() );
    return PyRef( PyString_FromString( o.getStr() ), SAL_NO_ACQUIRE );
}

static PyRef getSysModuleDict()
{
    PyRef module(
        PyImport_ImportModule( const_cast< char * >("sys") ),
        SAL_NO_ACQUIRE );
    raiseRuntimeExceptionWhenNeeded();
    if( !module.is() )
    {
        throw RuntimeException(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "pythonloader: Couldn't load module sys" ) ),
            Reference< XInterface > () );
    }
    PyRef dict( PyModule_GetDict( module.get() ));
    if( !dict.is() )
    {
        throw RuntimeException(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "pythonloader: Couldn't get dictionary of module sys" ) ),
            Reference< XInterface > () );
    }
    return dict;
}
static void setPythonHome ( const OUString & pythonHome )
{
    OUString systemPythonHome;
    osl_getSystemPathFromFileURL( pythonHome.pData, &(systemPythonHome.pData) );
    OString o = rtl::OUStringToOString( systemPythonHome, osl_getThreadTextEncoding() );
    rtl_string_acquire(o.pData); // leak this string (thats the api!)
    Py_SetPythonHome( o.pData->buffer);
}

static void prependPythonPath( const OUString & pythonPathBootstrap )
{
    rtl::OStringBuffer bufPYTHONPATH( 256 );
    bufPYTHONPATH.append( "PYTHONPATH=");
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
        bufPYTHONPATH.append( rtl::OUStringToOString( systemPath.pData, osl_getThreadTextEncoding() ));
        bufPYTHONPATH.append( SAL_PATHSEPARATOR );
        if( nNew == -1 )
            break;
        nIndex = nNew + 1;
    }
    const char * oldEnv = getenv( "PYTHONPATH");
    if( oldEnv )
        bufPYTHONPATH.append( oldEnv );
    OString result = bufPYTHONPATH.makeStringAndClear();
    rtl_string_acquire( result.pData );

//     printf( "Setting %s\n" , result.pData->buffer );
    putenv( result.pData->buffer );

}

Reference< XInterface > CreateInstance( const Reference< XComponentContext > & ctx )
{
    Reference< XInterface > ret;

    if( ! Py_IsInitialized() )
    {
        OUString pythonPath;
        OUString pythonHome;
        OUString path = getLibDir();
        if( path.getLength() )
        {
            path += OUString( RTL_CONSTASCII_USTRINGPARAM( "/" SAL_CONFIGFILE("pythonloader.uno" )));
            rtl::Bootstrap bootstrap(path);

            // look for pythonhome
            bootstrap.getFrom( OUString( RTL_CONSTASCII_USTRINGPARAM( "PYUNO_LOADER_PYTHONHOME") ), pythonHome );
            bootstrap.getFrom( OUString( RTL_CONSTASCII_USTRINGPARAM( "PYUNO_LOADER_PYTHONPATH" ) ) , pythonPath );
        }

        // pythonhome+pythonpath must be set before Py_Initialize(), otherwise there appear warning on the console
        // sadly, there is no api for setting the pythonpath, we have to use the environment variable
        if( pythonHome.getLength() )
            setPythonHome( pythonHome );

        if( pythonPath.getLength() )
            prependPythonPath( pythonPath );

        // initialize python
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
    const sal_Char ** ppEnvTypeName, uno_Environment ** )
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

