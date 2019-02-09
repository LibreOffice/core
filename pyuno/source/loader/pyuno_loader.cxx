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



#include <pyuno/pyuno.hxx>

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
    rtl::OUStringBuffer bufPYTHONPATH( 256 );
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
        bufPYTHONPATH.append( static_cast<sal_Unicode>(SAL_PATHSEPARATOR) );
        if( nNew == -1 )
            break;
        nIndex = nNew + 1;
    }
    const char * oldEnv = getenv( "PYTHONPATH");
    if( oldEnv )
        bufPYTHONPATH.append( rtl::OUString(oldEnv, strlen(oldEnv), osl_getThreadTextEncoding()) );

    rtl::OUString envVar(RTL_CONSTASCII_USTRINGPARAM("PYTHONPATH"));
    rtl::OUString envValue(bufPYTHONPATH.makeStringAndClear());
    osl_setEnvironment(envVar.pData, envValue.pData);
}

Reference< XInterface > CreateInstance( const Reference< XComponentContext > & ctx )
{
    Reference< XInterface > ret;

    if( ! Py_IsInitialized() )
    {
        OUString pythonPath;
        OUString pythonHome;
        OUString path( RTL_CONSTASCII_USTRINGPARAM( "$OOO_BASE_DIR/program/" SAL_CONFIGFILE("pythonloader.uno" )));
        rtl::Bootstrap::expandMacros(path); //TODO: detect failure
        rtl::Bootstrap bootstrap(path);

        // look for pythonhome
        bootstrap.getFrom( OUString( RTL_CONSTASCII_USTRINGPARAM( "PYUNO_LOADER_PYTHONHOME") ), pythonHome );
        bootstrap.getFrom( OUString( RTL_CONSTASCII_USTRINGPARAM( "PYUNO_LOADER_PYTHONPATH" ) ) , pythonPath );

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
SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return cppu::component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , g_entries );
}

}

