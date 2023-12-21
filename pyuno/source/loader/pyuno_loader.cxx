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

#include <config_features.h>
#include <config_folders.h>

#include <pyuno.hxx>

#include <o3tl/any.hxx>
#include <o3tl/char16_t2wchar_t.hxx>

#include <osl/process.h>
#include <osl/file.hxx>
#include <osl/thread.h>

#include <rtl/ustrbuf.hxx>
#include <rtl/bootstrap.hxx>

#include <cppuhelper/factory.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>

#include <comphelper/processfactory.hxx>
#include <officecfg/Office/Common.hxx>

#include <systools/win32/extended_max_path.hxx>

// apparently PATH_MAX is not standard and not defined by MSVC
#ifdef _WIN32
#ifdef PATH_MAX
#undef PATH_MAX
#endif
#define PATH_MAX EXTENDED_MAX_PATH
#endif
#ifndef PATH_MAX
#if defined _MAX_PATH
#define PATH_MAX _MAX_PATH
#elif defined MAX_PATH
#define PATH_MAX MAX_PATH
#else
#error no PATH_MAX
#endif
#endif

using pyuno::PyRef;
using pyuno::NOT_NULL;
using pyuno::Runtime;
using pyuno::PyThreadAttach;

using com::sun::star::uno::Reference;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::XComponentContext;
using com::sun::star::uno::RuntimeException;

namespace pyuno_loader
{

/// @throws RuntimeException
static void raiseRuntimeExceptionWhenNeeded()
{
    if( PyErr_Occurred() )
    {
        PyRef excType, excValue, excTraceback;
        PyErr_Fetch(reinterpret_cast<PyObject **>(&excType), reinterpret_cast<PyObject**>(&excValue), reinterpret_cast<PyObject**>(&excTraceback));
        Runtime runtime;
        css::uno::Any a = runtime.extractUnoException( excType, excValue, excTraceback );
        OUStringBuffer buf( "python-loader:" );
        if( auto e = o3tl::tryAccess<css::uno::Exception>(a) )
            buf.append( e->Message );
        throw RuntimeException( buf.makeStringAndClear() );
    }
}

/// @throws RuntimeException
static PyRef getLoaderModule()
{
    PyRef module(
        PyImport_ImportModule( "pythonloader" ),
        SAL_NO_ACQUIRE );
    raiseRuntimeExceptionWhenNeeded();
    if( !module.is() )
    {
        throw RuntimeException( "pythonloader: Couldn't load pythonloader module" );
    }
    return PyRef( PyModule_GetDict( module.get() ));
}

/// @throws RuntimeException
static PyRef getObjectFromLoaderModule( const char * func )
{
    PyRef object( PyDict_GetItemString(getLoaderModule().get(), func ) );
    if( !object.is() )
    {
        throw RuntimeException( "pythonloader: couldn't find core element pythonloader." +
                OUString::createFromAscii( func ));
    }
    return object;
}

#if PY_VERSION_HEX >= 0x03080000
static void setPythonHome ( const OUString & pythonHome, PyConfig * config )
#else
static void setPythonHome ( const OUString & pythonHome )
#endif
{
    OUString systemPythonHome;
    osl_getSystemPathFromFileURL( pythonHome.pData, &(systemPythonHome.pData) );
    // static because Py_SetPythonHome just copies the "wide" pointer
    static wchar_t wide[PATH_MAX + 1];
#if defined _WIN32
    const size_t len = systemPythonHome.getLength();
    if (len < std::size(wide))
        wcsncpy(wide, o3tl::toW(systemPythonHome.getStr()), len + 1);
#else
    OString o = OUStringToOString(systemPythonHome, osl_getThreadTextEncoding());
    size_t len = mbstowcs(wide, o.pData->buffer, std::size(wide));
    if(len == size_t(-1))
    {
        PyErr_SetString(PyExc_SystemError, "invalid multibyte sequence in python home path");
        return;
    }
#endif
    if (len >= std::size(wide))
    {
        PyErr_SetString(PyExc_SystemError, "python home path is too long");
        return;
    }
#if PY_VERSION_HEX >= 0x03080000
    config->home = wide;
#else
    Py_SetPythonHome(wide);
#endif
}

static void prependPythonPath( std::u16string_view pythonPathBootstrap )
{
    OUStringBuffer bufPYTHONPATH( 256 );
    bool bAppendSep = false;
    sal_Int32 nIndex = 0;
    while( true )
    {
        size_t nNew = pythonPathBootstrap.find( ' ', nIndex );
        std::u16string_view fileUrl;
        if( nNew == std::u16string_view::npos )
        {
            fileUrl = pythonPathBootstrap.substr(nIndex);
        }
        else
        {
            fileUrl = pythonPathBootstrap.substr(nIndex, nNew - nIndex);
        }
        OUString systemPath;
        osl_getSystemPathFromFileURL( OUString(fileUrl).pData, &(systemPath.pData) );
        if (!systemPath.isEmpty())
        {
            if (bAppendSep)
                bufPYTHONPATH.append(static_cast<sal_Unicode>(SAL_PATHSEPARATOR));
            bufPYTHONPATH.append(systemPath);
            bAppendSep = true;
        }
        if( nNew == std::u16string_view::npos )
            break;
        nIndex = nNew + 1;
    }
    const char * oldEnv = getenv( "PYTHONPATH");
    if( oldEnv )
    {
        if (bAppendSep)
            bufPYTHONPATH.append( static_cast<sal_Unicode>(SAL_PATHSEPARATOR) );
        bufPYTHONPATH.append( OUString(oldEnv, strlen(oldEnv), osl_getThreadTextEncoding()) );
    }

    OUString envVar("PYTHONPATH");
    OUString envValue(bufPYTHONPATH.makeStringAndClear());
    osl_setEnvironment(envVar.pData, envValue.pData);
}

namespace {

void pythonInit() {
    if ( Py_IsInitialized()) // may be inited by getComponentContext() already
        return;

#if PY_VERSION_HEX >= 0x03080000
    PyConfig config;
#endif
    OUString pythonPath;
    OUString pythonHome;
    OUString path( "$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("pythonloader.uno" ));
    rtl::Bootstrap::expandMacros(path); //TODO: detect failure
    rtl::Bootstrap bootstrap(path);
#if PY_VERSION_HEX >= 0x03080000
    PyConfig_InitPythonConfig( &config );
#endif

    // look for pythonhome
    bootstrap.getFrom( "PYUNO_LOADER_PYTHONHOME", pythonHome );
    bootstrap.getFrom( "PYUNO_LOADER_PYTHONPATH", pythonPath );

    // pythonhome+pythonpath must be set before Py_Initialize(), otherwise there appear warning on the console
    // sadly, there is no api for setting the pythonpath, we have to use the environment variable
    if( !pythonHome.isEmpty() )
#if PY_VERSION_HEX >= 0x03080000
        setPythonHome( pythonHome, &config );
#else
        setPythonHome( pythonHome );
#endif

    if( !pythonPath.isEmpty() )
        prependPythonPath( pythonPath );

#ifdef _WIN32
    //extend PATH under windows to include the branddir/program so ssl libs will be found
    //for use by terminal mailmerge dependency _ssl.pyd
    OUString sEnvName("PATH");
    OUString sPath;
    osl_getEnvironment(sEnvName.pData, &sPath.pData);
    OUString sBrandLocation("$BRAND_BASE_DIR/program");
    rtl::Bootstrap::expandMacros(sBrandLocation);
    osl::FileBase::getSystemPathFromFileURL(sBrandLocation, sBrandLocation);
    sPath = sPath + OUStringChar(SAL_PATHSEPARATOR) + sBrandLocation;
    osl_setEnvironment(sEnvName.pData, sPath.pData);
#endif

    PyImport_AppendInittab( "pyuno", PyInit_pyuno );

#if HAVE_FEATURE_READONLY_INSTALLSET
    Py_DontWriteBytecodeFlag = 1;
#endif

    // initialize python
    Py_Initialize();
#if PY_VERSION_HEX < 0x03090000
    PyEval_InitThreads();
#endif

    PyThreadState *tstate = PyThreadState_Get();
    PyEval_ReleaseThread( tstate );
#if PY_VERSION_HEX < 0x030B0000
    // This tstate is never used again, so delete it here.
    // This prevents an assertion in PyThreadState_Swap on the
    // PyThreadAttach below.
    PyThreadState_Delete(tstate);
#endif
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
pyuno_Loader_get_implementation(
    css::uno::XComponentContext* ctx , css::uno::Sequence<css::uno::Any> const&)
{
    if (officecfg::Office::Common::Security::Scripting::DisablePythonRuntime::get(ctx))
        return nullptr;

    // tdf#114815 init python only once, via single-instace="true" in pythonloader.component
    pythonInit();

    Reference< XInterface > ret;

    PyThreadAttach attach( PyInterpreterState_Head() );
    {
        // note: this can't race against getComponentContext() because
        // either (in soffice.bin) CreateInstance() must be called before
        // getComponentContext() can be called, or (in python.bin) the other
        // way around
        if( ! Runtime::isInitialized() )
        {
            Runtime::initialize( ctx );
        }
        Runtime runtime;

        PyRef pyCtx = runtime.any2PyObject(
            css::uno::Any( css::uno::Reference(ctx) ) );

        PyRef clazz = getObjectFromLoaderModule( "Loader" );
        PyRef args ( PyTuple_New( 1 ), SAL_NO_ACQUIRE, NOT_NULL );
        PyTuple_SetItem( args.get(), 0 , pyCtx.getAcquired() );
        PyRef pyInstance( PyObject_CallObject( clazz.get() , args.get() ), SAL_NO_ACQUIRE );
        runtime.pyObject2Any( pyInstance ) >>= ret;
    }
    ret->acquire();
    return ret.get();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
