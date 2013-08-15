/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <pyuno/pyuno.hxx>

#include <osl/process.h>
#include <osl/file.hxx>
#include <osl/thread.h>

#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/bootstrap.hxx>

#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/factory.hxx>

// apparently PATH_MAX is not standard and not defined by MSVC
#ifndef PATH_MAX
#ifdef _MAX_PATH
#define PATH_MAX _MAX_PATH
#else
#ifdef MAX_PATH
#define PATH_MAX MAX_PATH
#else
#error no PATH_MAX
#endif
#endif
#endif


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
        PyImport_ImportModule( "pythonloader" ),
        SAL_NO_ACQUIRE );
    raiseRuntimeExceptionWhenNeeded();
    if( !module.is() )
    {
        throw RuntimeException(
            OUString( "pythonloader: Couldn't load pythonloader module" ),
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
    return OUString( "org.openoffice.comp.pyuno.Loader" );
}

Sequence< OUString > getSupportedServiceNames()
{
    OUString serviceName( "com.sun.star.loader.Python" );
    return Sequence< OUString > ( &serviceName, 1 );
}

static void setPythonHome ( const OUString & pythonHome )
{
    OUString systemPythonHome;
    osl_getSystemPathFromFileURL( pythonHome.pData, &(systemPythonHome.pData) );
    OString o = OUStringToOString( systemPythonHome, osl_getThreadTextEncoding() );
#if PY_MAJOR_VERSION >= 3
    // static because Py_SetPythonHome just copies the "wide" pointer
    static wchar_t wide[PATH_MAX + 1];
    size_t len = mbstowcs(wide, o.pData->buffer, PATH_MAX + 1);
    if(len == (size_t)-1)
    {
        PyErr_SetString(PyExc_SystemError, "invalid multibyte sequence in python home path");
        return;
    }
    if(len == PATH_MAX + 1)
    {
        PyErr_SetString(PyExc_SystemError, "python home path is too long");
        return;
    }
    Py_SetPythonHome(wide);
#else
    rtl_string_acquire(o.pData); // increase reference count
    Py_SetPythonHome(o.pData->buffer);
#endif
}

static void prependPythonPath( const OUString & pythonPathBootstrap )
{
    OUStringBuffer bufPYTHONPATH( 256 );
    sal_Int32 nIndex = 0;
    while( 1 )
    {
        sal_Int32 nNew = pythonPathBootstrap.indexOf( ' ', nIndex );
        OUString fileUrl;
        if( nNew == -1 )
        {
            fileUrl = pythonPathBootstrap.copy(nIndex);
        }
        else
        {
            fileUrl = pythonPathBootstrap.copy(nIndex, nNew - nIndex);
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
        bufPYTHONPATH.append( OUString(oldEnv, strlen(oldEnv), osl_getThreadTextEncoding()) );

    OUString envVar("PYTHONPATH");
    OUString envValue(bufPYTHONPATH.makeStringAndClear());
    osl_setEnvironment(envVar.pData, envValue.pData);
}

Reference< XInterface > CreateInstance( const Reference< XComponentContext > & ctx )
{
    Reference< XInterface > ret;

    if( ! Py_IsInitialized() )
    {
        OUString pythonPath;
        OUString pythonHome;
        OUString path( "$BRAND_BASE_DIR/program/" SAL_CONFIGFILE("pythonloader.uno" ));
        rtl::Bootstrap::expandMacros(path); //TODO: detect failure
        rtl::Bootstrap bootstrap(path);

        // look for pythonhome
        bootstrap.getFrom( OUString( "PYUNO_LOADER_PYTHONHOME"), pythonHome );
        bootstrap.getFrom( OUString( "PYUNO_LOADER_PYTHONPATH" ) , pythonPath );

        // pythonhome+pythonpath must be set before Py_Initialize(), otherwise there appear warning on the console
        // sadly, there is no api for setting the pythonpath, we have to use the environment variable
        if( !pythonHome.isEmpty() )
            setPythonHome( pythonHome );

        if( !pythonPath.isEmpty() )
            prependPythonPath( pythonPath );

#ifdef WNT
    //extend PATH under windows to include the branddir/program so ssl libs will be found
    //for use by terminal mailmerge dependency _ssl.pyd
    OUString sEnvName("PATH");
    OUString sPath;
    osl_getEnvironment(sEnvName.pData, &sPath.pData);
    OUString sBrandLocation("$BRAND_BASE_DIR/program");
    rtl::Bootstrap::expandMacros(sBrandLocation);
    osl::FileBase::getSystemPathFromFileURL(sBrandLocation, sBrandLocation);
    sPath = OUStringBuffer(sPath).
        append(static_cast<sal_Unicode>(SAL_PATHSEPARATOR)).
        append(sBrandLocation).makeStringAndClear();
    osl_setEnvironment(sEnvName.pData, sPath.pData);
#endif

#if PY_MAJOR_VERSION >= 3
        PyImport_AppendInittab( (char*)"pyuno", PyInit_pyuno );
#else
        PyImport_AppendInittab( (char*)"pyuno", initpyuno );
#endif
        // initialize python
        Py_Initialize();
        PyEval_InitThreads();

        PyThreadState *tstate = PyThreadState_Get();
        PyEval_ReleaseThread( tstate );
        // This tstate is never used again, so delete it here.
        // This prevents an assertion in PyThreadState_Swap on the
        // PyThreadAttach below.
        PyThreadState_Delete(tstate);
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


static const struct cppu::ImplementationEntry g_entries[] =
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

SAL_DLLPUBLIC_EXPORT void * SAL_CALL pythonloader_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return cppu::component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , g_entries );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
