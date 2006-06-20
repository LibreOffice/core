/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pyuno_loader.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:02:53 $
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

Reference< XInterface > CreateInstance( const Reference< XComponentContext > & ctx )
{
    Reference< XInterface > ret;

    if( ! Py_IsInitialized() )
    {
        // in case python path is already set, nothing is done ...
        const OUString pythonPath ( RTL_CONSTASCII_USTRINGPARAM( "PYTHONPATH" ) );

        // otherwise, try to get the PYTHONPATH bootstrap variable
        OUStringBuffer bufPYTHONPATH( 256 );
        OUString path = getLibDir();
        if( path.getLength() )
        {
            path += OUString( RTL_CONSTASCII_USTRINGPARAM( "/" SAL_CONFIGFILE("pythonloader.uno" )));
            rtl::Bootstrap bootstrap(path);

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

            // check for pythonpath
            OUString pythonPathBootstrap;
            bootstrap.getFrom( pythonPath , pythonPathBootstrap );

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
                bufPYTHONPATH.append( systemPath );
                if( nNew == -1 )
                    break;
                bufPYTHONPATH.append( (sal_Unicode) SAL_PATHSEPARATOR );
                nIndex = nNew + 1;
            }
        }

        OUString value;
        osl_getEnvironment( pythonPath.pData,  &value.pData );
        bufPYTHONPATH.append( value );

        rtl::OStringBuffer stringBuffer;
        stringBuffer.append( "PYTHONPATH=" );
        stringBuffer.append(
            rtl::OUStringToOString( bufPYTHONPATH.makeStringAndClear(), osl_getThreadTextEncoding()));

        OString env = stringBuffer.makeStringAndClear();

        // leak this string (putenv does not make a copy)
        rtl_string_acquire( env.pData );
        putenv( env.pData->buffer );


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

