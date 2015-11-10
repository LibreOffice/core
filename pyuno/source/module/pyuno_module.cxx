/* -*- Mode: C++; eval:(c-set-style "bsd"); tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "pyuno_impl.hxx"

#include <unordered_map>
#include <utility>

#include <osl/module.hxx>
#include <osl/thread.h>
#include <osl/file.hxx>

#include <typelib/typedescription.hxx>

#include <rtl/ustring.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/uuid.h>
#include <rtl/bootstrap.hxx>

#include <uno/current_context.hxx>
#include <cppuhelper/bootstrap.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/reflection/XConstantTypeDescription.hpp>
#include <com/sun/star/reflection/XIdlReflection.hpp>
#include <com/sun/star/reflection/XIdlClass.hpp>
#include <com/sun/star/registry/InvalidRegistryException.hpp>

using osl::Module;


using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Any;
using com::sun::star::uno::makeAny;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::TypeDescription;
using com::sun::star::uno::XComponentContext;
using com::sun::star::container::NoSuchElementException;
using com::sun::star::reflection::XIdlReflection;
using com::sun::star::reflection::XIdlClass;
using com::sun::star::script::XInvocation2;

using namespace pyuno;

namespace {

/**
   @ index of the next to be used member in the initializer list !
 */
// LEM TODO: export member names as keyword arguments in initialiser?
// Python supports very flexible variadic functions. By marking
// variables with one asterisk (e.g. *var) the given variable is
// defined to be a tuple of all the extra arguments. By marking
// variables with two asterisks (e.g. **var) the given variable is a
// dictionary of all extra keyword arguments; the keys are strings,
// which are the names that were used to identify the arguments. If
// they exist, these arguments must be the last one in the list.

class fillStructState
{
    // Keyword arguments used
    PyObject *used;
    // Which structure members are initialised
    std::unordered_map <OUString, bool, OUStringHash> initialised;
    // How many positional arguments are consumed
    // This is always the so-many first ones
    sal_Int32 nPosConsumed;

public:
    fillStructState()
        : used (PyDict_New())
        , initialised ()
        , nPosConsumed (0)
    {
        if ( ! used )
            throw RuntimeException("pyuno._createUnoStructHelper failed to create new dictionary");
    }
    ~fillStructState()
    {
        Py_DECREF(used);
    }
    int setUsed(PyObject *key)
    {
        return PyDict_SetItem(used, key, Py_True);
    }
    void setInitialised(const OUString& key, sal_Int32 pos = -1)
    {
        if (initialised[key])
        {
            OUStringBuffer buf;
            buf.append( "pyuno._createUnoStructHelper: member '");
            buf.append(key);
            buf.append( "'");
            if ( pos >= 0 )
            {
                buf.append( " at position ");
                buf.append(pos);
            }
            buf.append( " initialised multiple times.");
            throw RuntimeException(buf.makeStringAndClear());
        }
        initialised[key] = true;
        if ( pos >= 0 )
            ++nPosConsumed;
    }
    bool isInitialised(const OUString& key)
    {
        return initialised[key];
    }
    PyObject *getUsed() const
    {
        return used;
    }
    sal_Int32 getCntConsumed() const
    {
        return nPosConsumed;
    }
};

static void fillStruct(
    const Reference< XInvocation2 > &inv,
    typelib_CompoundTypeDescription *pCompType,
    PyObject *initializer,
    PyObject *kwinitializer,
    fillStructState &state,
    const Runtime &runtime) throw ( RuntimeException )
{
    if( pCompType->pBaseTypeDescription )
        fillStruct( inv, pCompType->pBaseTypeDescription, initializer, kwinitializer, state, runtime );

    const sal_Int32 nMembers = pCompType->nMembers;
    {
        for( int i = 0 ; i < nMembers ; i ++ )
        {
            const OUString OUMemberName (pCompType->ppMemberNames[i]);
            PyObject *pyMemberName =
                PyStr_FromString(OUStringToOString(OUMemberName,
                        RTL_TEXTENCODING_UTF8).getStr());
            if ( PyObject *element = PyDict_GetItem(kwinitializer, pyMemberName ) )
            {
                state.setInitialised(OUMemberName);
                state.setUsed(pyMemberName);
                Any a = runtime.pyObject2Any( element, ACCEPT_UNO_ANY );
                inv->setValue( OUMemberName, a );
            }
        }
    }
    {
        const int remainingPosInitialisers = PyTuple_Size(initializer) - state.getCntConsumed();
        for( int i = 0 ; i < remainingPosInitialisers && i < nMembers ; i ++ )
        {
            const int tupleIndex = state.getCntConsumed();
            const OUString pMemberName (pCompType->ppMemberNames[i]);
            state.setInitialised(pMemberName, tupleIndex);
            PyObject *element = PyTuple_GetItem( initializer, tupleIndex );
            Any a = runtime.pyObject2Any( element, ACCEPT_UNO_ANY );
            inv->setValue( pMemberName, a );
        }
    }
    if ( PyTuple_Size( initializer ) > 0 )
    {
        // Allow partial initialisation when only keyword arguments are given
        for ( int i = 0; i < nMembers ; ++i)
        {
            const OUString memberName (pCompType->ppMemberNames[i]);
            if ( ! state.isInitialised( memberName ) )
            {
                OUStringBuffer buf;
                buf.append( "pyuno._createUnoStructHelper: member '");
                buf.append(memberName);
                buf.append( "' of struct type '");
                buf.append(pCompType->aBase.pTypeName);
                buf.append( "' not given a value.");
                throw RuntimeException(buf.makeStringAndClear());
            }
        }
    }
}

OUString getLibDir()
{
    static OUString *pLibDir;
    if( !pLibDir )
    {
        osl::MutexGuard guard( osl::Mutex::getGlobalMutex() );
        if( ! pLibDir )
        {
            static OUString libDir;

            // workarounds the $(ORIGIN) until it is available
            if( Module::getUrlFromAddress(
                    reinterpret_cast< oslGenericFunction >(getLibDir), libDir ) )
            {
                libDir = OUString( libDir.getStr(), libDir.lastIndexOf('/' ) );
                OUString name ( "PYUNOLIBDIR" );
                rtl_bootstrap_set( name.pData, libDir.pData );
            }
            pLibDir = &libDir;
        }
    }
    return *pLibDir;
}

void raisePySystemException( const char * exceptionType, const OUString & message )
{
    OStringBuffer buf;
    buf.append( "Error during bootstrapping uno (");
    buf.append( exceptionType );
    buf.append( "):" );
    buf.append( OUStringToOString( message, osl_getThreadTextEncoding() ) );
    PyErr_SetString( PyExc_SystemError, buf.makeStringAndClear().getStr() );
}

extern "C" {

static PyObject* getComponentContext(
    SAL_UNUSED_PARAMETER PyObject*, SAL_UNUSED_PARAMETER PyObject*)
{
    PyRef ret;
    try
    {
        Reference<XComponentContext> ctx;

        // getLibDir() must be called in order to set bootstrap variables correctly !
        OUString path( getLibDir());
        if( Runtime::isInitialized() )
        {
            Runtime runtime;
            ctx = runtime.getImpl()->cargo->xContext;
        }
        else
        {
            OUString iniFile;
            if( path.isEmpty() )
            {
                PyErr_SetString(
                    PyExc_RuntimeError, "osl_getUrlFromAddress fails, that's why I cannot find ini "
                    "file for bootstrapping python uno bridge\n" );
                return nullptr;
            }

            OUStringBuffer iniFileName;
            iniFileName.append( path );
#ifdef MACOSX
            iniFileName.append( "/../" LIBO_ETC_FOLDER );
#endif
            iniFileName.append( "/" );
            iniFileName.append( SAL_CONFIGFILE( "pyuno" ) );
            iniFile = iniFileName.makeStringAndClear();
            osl::DirectoryItem item;
            if( osl::DirectoryItem::get( iniFile, item ) == item.E_None )
            {
                // in case pyuno.ini exists, use this file for bootstrapping
                PyThreadDetach antiguard;
                ctx = cppu::defaultBootstrap_InitialComponentContext (iniFile);
            }
            else
            {
                // defaulting to the standard bootstrapping
                PyThreadDetach antiguard;
                ctx = cppu::defaultBootstrap_InitialComponentContext ();
            }

        }

        if( ! Runtime::isInitialized() )
        {
            Runtime::initialize( ctx );
        }
        Runtime runtime;
        ret = runtime.any2PyObject( makeAny( ctx ) );
    }
    catch (const css::registry::InvalidRegistryException &e)
    {
        // can't use raisePyExceptionWithAny() here, because the function
        // does any conversions, which will not work with a
        // wrongly bootstrapped pyuno!
        raisePySystemException( "InvalidRegistryException", e.Message );
    }
    catch(const css::lang::IllegalArgumentException & e)
    {
        raisePySystemException( "IllegalArgumentException", e.Message );
    }
    catch(const css::script::CannotConvertException & e)
    {
        raisePySystemException( "CannotConvertException", e.Message );
    }
    catch (const css::uno::RuntimeException & e)
    {
        raisePySystemException( "RuntimeException", e.Message );
    }
    catch (const css::uno::Exception & e)
    {
        raisePySystemException( "uno::Exception", e.Message );
    }
    return ret.getAcquired();
}

static PyObject* initTestEnvironment(
    SAL_UNUSED_PARAMETER PyObject*, SAL_UNUSED_PARAMETER PyObject*)
{
    // this tries to bootstrap enough of the soffice from python to run
    // unit tests, which is only possible indirectly because pyuno is URE
    // so load "test" library and invoke a function there to do the work
    try
    {
        PyObject *const ctx(getComponentContext(nullptr, nullptr));
        if (!ctx) { abort(); }
        Runtime const runtime;
        Any const a(runtime.pyObject2Any(ctx));
        Reference<XComponentContext> xContext;
        a >>= xContext;
        if (!xContext.is()) { abort(); }
        using css::lang::XMultiServiceFactory;
        Reference<XMultiServiceFactory> const xMSF(
            xContext->getServiceManager(),
            css::uno::UNO_QUERY_THROW);
        if (!xMSF.is()) { abort(); }
        char *const testlib = getenv("TEST_LIB");
        if (!testlib) { abort(); }
        OString const libname = OString(testlib, strlen(testlib))
#ifdef _WIN32
            .replaceAll(OString('/'), OString('\\'))
#endif
            ;

        osl::Module &mod = runtime.getImpl()->cargo->testModule;
        mod.load(OStringToOUString(libname, osl_getThreadTextEncoding()),
                                SAL_LOADMODULE_LAZY | SAL_LOADMODULE_GLOBAL);
        if (!mod.is()) { abort(); }
        oslGenericFunction const pFunc(
                mod.getFunctionSymbol("test_init"));
        if (!pFunc) { abort(); }
        reinterpret_cast<void (SAL_CALL *)(XMultiServiceFactory*)>(pFunc)(xMSF.get());
    }
    catch (const css::uno::Exception &)
    {
        abort();
    }
    return Py_None;
}

PyObject * extractOneStringArg( PyObject *args, char const *funcName )
{
    if( !PyTuple_Check( args ) || PyTuple_Size( args) != 1 )
    {
        OStringBuffer buf;
        buf.append( funcName ).append( ": expecting one string argument" );
        PyErr_SetString( PyExc_RuntimeError, buf.getStr() );
        return nullptr;
    }
    PyObject *obj = PyTuple_GetItem( args, 0 );
    if (!PyStr_Check(obj) && !PyUnicode_Check(obj))
    {
        OStringBuffer buf;
        buf.append( funcName ).append( ": expecting one string argument" );
        PyErr_SetString( PyExc_TypeError, buf.getStr());
        return nullptr;
    }
    return obj;
}

static PyObject *createUnoStructHelper(
    SAL_UNUSED_PARAMETER PyObject *, PyObject* args, PyObject* keywordArgs)
{
    Any IdlStruct;
    PyRef ret;
    try
    {
        Runtime runtime;
        if( PyTuple_Size( args ) == 2 )
        {
            PyObject *structName = PyTuple_GetItem(args, 0);
            PyObject *initializer = PyTuple_GetItem(args, 1);

            if (PyStr_Check(structName))
            {
                if( PyTuple_Check( initializer ) && PyDict_Check ( keywordArgs ) )
                {
                    OUString typeName( OUString::createFromAscii(PyStr_AsString(structName)));
                    RuntimeCargo *c = runtime.getImpl()->cargo;
                    Reference<XIdlClass> idl_class ( c->xCoreReflection->forName (typeName),UNO_QUERY);
                    if (idl_class.is ())
                    {
                        idl_class->createObject (IdlStruct);
                        PyRef returnCandidate( PyUNOStruct_new( IdlStruct, c->xInvocation ) );
                        PyUNO *me = reinterpret_cast<PyUNO*>( returnCandidate.get() );
                        TypeDescription desc( typeName );
                        OSL_ASSERT( desc.is() ); // could already instantiate an XInvocation2 !

                        typelib_CompoundTypeDescription *pCompType =
                            reinterpret_cast<typelib_CompoundTypeDescription *>(desc.get());
                        fillStructState state;
                        if ( PyTuple_Size( initializer ) > 0 || PyDict_Size( keywordArgs ) > 0 )
                            fillStruct( me->members->xInvocation, pCompType, initializer, keywordArgs, state, runtime );
                        if( state.getCntConsumed() != PyTuple_Size(initializer) )
                        {
                            OUStringBuffer buf;
                            buf.append( "pyuno._createUnoStructHelper: too many ");
                            buf.append( "elements in the initializer list, expected " );
                            buf.append( state.getCntConsumed() );
                            buf.append( ", got " );
                            buf.append( (sal_Int32) PyTuple_Size(initializer) );
                            throw RuntimeException( buf.makeStringAndClear());
                        }
                        ret = PyRef( PyTuple_Pack(2, returnCandidate.get(), state.getUsed()), SAL_NO_ACQUIRE);
                    }
                    else
                    {
                        OStringBuffer buf;
                        buf.append( "UNO struct " );
                        buf.append( PyStr_AsString(structName) );
                        buf.append( " is unknown" );
                        PyErr_SetString (PyExc_RuntimeError, buf.getStr());
                    }
                }
                else
                {
                    PyErr_SetString(
                        PyExc_RuntimeError,
                        "pyuno._createUnoStructHelper: 2nd argument (initializer sequence) is no tuple" );
                }
            }
            else
            {
                PyErr_SetString (PyExc_AttributeError, "createUnoStruct: first argument wasn't a string");
            }
        }
        else
        {
            PyErr_SetString (PyExc_AttributeError, "pyuno._createUnoStructHelper: expects exactly two non-keyword arguments:\n\tStructure Name\n\tinitialiser tuple; may be the empty tuple");
        }
    }
    catch( const css::uno::RuntimeException & e )
    {
        raisePyExceptionWithAny( makeAny( e ) );
    }
    catch( const css::script::CannotConvertException & e )
    {
        raisePyExceptionWithAny( makeAny( e ) );
    }
    catch( const css::uno::Exception & e )
    {
        raisePyExceptionWithAny( makeAny( e ) );
    }
    return ret.getAcquired();
}

static PyObject *getTypeByName(
    SAL_UNUSED_PARAMETER PyObject *, PyObject *args )
{
    PyObject * ret = nullptr;

    try
    {
        char *name;

        if (PyArg_ParseTuple (args, "s", &name))
        {
            OUString typeName ( OUString::createFromAscii( name ) );
            TypeDescription typeDesc( typeName );
            if( typeDesc.is() )
            {
                Runtime runtime;
                ret = PyUNO_Type_new(
                    name, (css::uno::TypeClass)typeDesc.get()->eTypeClass, runtime );
            }
            else
            {
                OStringBuffer buf;
                buf.append( "Type " ).append(name).append( " is unknown" );
                PyErr_SetString( PyExc_RuntimeError, buf.getStr() );
            }
        }
    }
    catch ( const RuntimeException & e )
    {
        raisePyExceptionWithAny( makeAny( e ) );
    }
    return ret;
}

static PyObject *getConstantByName(
    SAL_UNUSED_PARAMETER PyObject *, PyObject *args )
{
    PyObject *ret = nullptr;
    try
    {
        char *name;

        if (PyArg_ParseTuple (args, "s", &name))
        {
            OUString typeName ( OUString::createFromAscii( name ) );
            Runtime runtime;
            css::uno::Reference< css::reflection::XConstantTypeDescription > td;
            if (!(runtime.getImpl()->cargo->xTdMgr->getByHierarchicalName(
                      typeName)
                  >>= td))
            {
                OUStringBuffer buf;
                buf.append( "pyuno.getConstantByName: " ).append( typeName );
                buf.append( "is not a constant" );
                throw RuntimeException(buf.makeStringAndClear() );
            }
            PyRef constant = runtime.any2PyObject( td->getConstantValue() );
            ret = constant.getAcquired();
        }
    }
    catch( const NoSuchElementException & e )
    {
        // to the python programmer, this is a runtime exception,
        // do not support tweakings with the type system
        RuntimeException runExc( e.Message );
        raisePyExceptionWithAny( makeAny( runExc ) );
    }
    catch(const css::script::CannotConvertException & e)
    {
        raisePyExceptionWithAny( makeAny( e ) );
    }
    catch(const css::lang::IllegalArgumentException & e)
    {
        raisePyExceptionWithAny( makeAny( e ) );
    }
    catch( const RuntimeException & e )
    {
        raisePyExceptionWithAny( makeAny(e) );
    }
    return ret;
}

static PyObject *checkType( SAL_UNUSED_PARAMETER PyObject *, PyObject *args )
{
    if( !PyTuple_Check( args ) || PyTuple_Size( args) != 1 )
    {
        OStringBuffer buf;
        buf.append( "pyuno.checkType : expecting one uno.Type argument" );
        PyErr_SetString( PyExc_RuntimeError, buf.getStr() );
        return nullptr;
    }
    PyObject *obj = PyTuple_GetItem( args, 0 );

    try
    {
        PyType2Type( obj );
    }
    catch(const  RuntimeException & e)
    {
        raisePyExceptionWithAny( makeAny( e ) );
        return nullptr;
    }
    Py_INCREF( Py_None );
    return Py_None;
}

static PyObject *checkEnum( SAL_UNUSED_PARAMETER PyObject *, PyObject *args )
{
    if( !PyTuple_Check( args ) || PyTuple_Size( args) != 1 )
    {
        OStringBuffer buf;
        buf.append( "pyuno.checkType : expecting one uno.Type argument" );
        PyErr_SetString( PyExc_RuntimeError, buf.getStr() );
        return nullptr;
    }
    PyObject *obj = PyTuple_GetItem( args, 0 );

    try
    {
        PyEnum2Enum( obj );
    }
    catch(const RuntimeException & e)
    {
        raisePyExceptionWithAny( makeAny( e) );
        return nullptr;
    }
    Py_INCREF( Py_None );
    return Py_None;
}

static PyObject *getClass( SAL_UNUSED_PARAMETER PyObject *, PyObject *args )
{
    PyObject *obj = extractOneStringArg( args, "pyuno.getClass");
    if( ! obj )
        return nullptr;

    try
    {
        Runtime runtime;
        PyRef ret = getClass(pyString2ustring(obj), runtime);
        Py_XINCREF( ret.get() );
        return ret.get();
    }
    catch(const RuntimeException & e)
    {
        raisePyExceptionWithAny( makeAny(e) );
    }
    return nullptr;
}

static PyObject *isInterface( SAL_UNUSED_PARAMETER PyObject *, PyObject *args )
{

    if( PyTuple_Check( args ) && PyTuple_Size( args ) == 1 )
    {
        PyObject *obj = PyTuple_GetItem( args, 0 );
        Runtime r;
        return PyLong_FromLong( isInterfaceClass( r, obj ) );
    }
    return PyLong_FromLong( 0 );
}

static PyObject * generateUuid(
    SAL_UNUSED_PARAMETER PyObject *, SAL_UNUSED_PARAMETER PyObject * )
{
    Sequence< sal_Int8 > seq( 16 );
    rtl_createUuid( reinterpret_cast<sal_uInt8*>(seq.getArray()) , nullptr , sal_False );
    PyRef ret;
    try
    {
        Runtime runtime;
        ret = runtime.any2PyObject( makeAny( seq ) );
    }
    catch( const RuntimeException & e )
    {
        raisePyExceptionWithAny( makeAny(e) );
    }
    return ret.getAcquired();
}

static PyObject *systemPathToFileUrl(
    SAL_UNUSED_PARAMETER PyObject *, PyObject * args )
{
    PyObject *obj = extractOneStringArg( args, "pyuno.systemPathToFileUrl" );
    if( ! obj )
        return nullptr;

    OUString sysPath = pyString2ustring( obj );
    OUString url;
    osl::FileBase::RC e = osl::FileBase::getFileURLFromSystemPath( sysPath, url );

    if( e != osl::FileBase::E_None )
    {
        OUStringBuffer buf;
        buf.append( "Couldn't convert " );
        buf.append( sysPath );
        buf.append( " to a file url for reason (" );
        buf.append( (sal_Int32) e );
        buf.append( ")" );
        raisePyExceptionWithAny(
            makeAny( RuntimeException( buf.makeStringAndClear() )));
        return nullptr;
    }
    return ustring2PyUnicode( url ).getAcquired();
}

static PyObject * fileUrlToSystemPath(
    SAL_UNUSED_PARAMETER PyObject *, PyObject * args )
{
    PyObject *obj = extractOneStringArg( args, "pyuno.fileUrlToSystemPath" );
    if( ! obj )
        return nullptr;

    OUString url = pyString2ustring( obj );
    OUString sysPath;
    osl::FileBase::RC e = osl::FileBase::getSystemPathFromFileURL( url, sysPath );

    if( e != osl::FileBase::E_None )
    {
        OUStringBuffer buf;
        buf.append( "Couldn't convert file url " );
        buf.append( sysPath );
        buf.append( " to a system path for reason (" );
        buf.append( (sal_Int32) e );
        buf.append( ")" );
        raisePyExceptionWithAny(
            makeAny( RuntimeException( buf.makeStringAndClear() )));
        return nullptr;
    }
    return ustring2PyUnicode( sysPath ).getAcquired();
}

static PyObject * absolutize( SAL_UNUSED_PARAMETER PyObject *, PyObject * args )
{
    if( PyTuple_Check( args ) && PyTuple_Size( args ) == 2 )
    {
        OUString ouPath = pyString2ustring( PyTuple_GetItem( args , 0 ) );
        OUString ouRel = pyString2ustring( PyTuple_GetItem( args, 1 ) );
        OUString ret;
        oslFileError e = osl_getAbsoluteFileURL( ouPath.pData, ouRel.pData, &(ret.pData) );
        if( e != osl_File_E_None )
        {
            OUStringBuffer buf;
            buf.append( "Couldn't absolutize " );
            buf.append( ouRel );
            buf.append( " using root " );
            buf.append( ouPath );
            buf.append( " for reason (" );
            buf.append( (sal_Int32) e );
            buf.append( ")" );

            PyErr_SetString(
                PyExc_OSError,
                OUStringToOString(buf.makeStringAndClear(),osl_getThreadTextEncoding()).getStr());
            return nullptr;
        }
        return ustring2PyUnicode( ret ).getAcquired();
    }
    return nullptr;
}

static PyObject * invoke(SAL_UNUSED_PARAMETER PyObject *, PyObject *args)
{
    PyObject *ret = nullptr;
    if(PyTuple_Check(args) && PyTuple_Size(args) == 3)
    {
        PyObject *object = PyTuple_GetItem(args, 0);
        PyObject *item1 = PyTuple_GetItem(args, 1);
        if (PyStr_Check(item1))
        {
            const char *name = PyStr_AsString(item1);
            PyObject *item2 = PyTuple_GetItem(args, 2);
            if(PyTuple_Check(item2))
            {
                ret = PyUNO_invoke(object, name, item2);
            }
            else
            {
                OStringBuffer buf;
                buf.append("uno.invoke expects a tuple as 3rd argument, got ");
                buf.append(PyStr_AsString(PyObject_Str(item2)));
                PyErr_SetString(
                    PyExc_RuntimeError, buf.makeStringAndClear().getStr());
            }
        }
        else
        {
            OStringBuffer buf;
            buf.append("uno.invoke expected a string as 2nd argument, got ");
            buf.append(PyStr_AsString(PyObject_Str(item1)));
            PyErr_SetString(
                PyExc_RuntimeError, buf.makeStringAndClear().getStr());
        }
    }
    else
    {
        OStringBuffer buf;
        buf.append("uno.invoke expects object, name, (arg1, arg2, ... )\n");
        PyErr_SetString(PyExc_RuntimeError, buf.makeStringAndClear().getStr());
    }
    return ret;
}

static PyObject *getCurrentContext(
    SAL_UNUSED_PARAMETER PyObject *, SAL_UNUSED_PARAMETER PyObject * )
{
    PyRef ret;
    try
    {
        Runtime runtime;
        ret = runtime.any2PyObject(
            makeAny( css::uno::getCurrentContext() ) );
    }
    catch( const css::uno::Exception & e )
    {
        raisePyExceptionWithAny( makeAny( e ) );
    }
    return ret.getAcquired();
}

static PyObject *setCurrentContext(
    SAL_UNUSED_PARAMETER PyObject *, SAL_UNUSED_PARAMETER PyObject * args )
{
    PyRef ret;
    try
    {
        if( PyTuple_Check( args ) && PyTuple_Size( args ) == 1 )
        {

            Runtime runtime;
            Any a = runtime.pyObject2Any( PyTuple_GetItem( args, 0 ) );

            Reference< css::uno::XCurrentContext > context;

            if( (a.hasValue() && (a >>= context)) || ! a.hasValue() )
            {
                ret = css::uno::setCurrentContext( context ) ? Py_True : Py_False;
            }
            else
            {
                OStringBuffer buf;
                buf.append( "uno.setCurrentContext expects an XComponentContext implementation, got " );
                buf.append(
                    PyStr_AsString(PyObject_Str(PyTuple_GetItem(args, 0))));
                PyErr_SetString(
                    PyExc_RuntimeError, buf.makeStringAndClear().getStr() );
            }
        }
        else
        {
            OStringBuffer buf;
            buf.append( "uno.setCurrentContext expects exactly one argument (the current Context)\n" );
            PyErr_SetString(
                PyExc_RuntimeError, buf.makeStringAndClear().getStr() );
        }
    }
    catch( const css::uno::Exception & e )
    {
        raisePyExceptionWithAny( makeAny( e ) );
    }
    return ret.getAcquired();
}

}

struct PyMethodDef PyUNOModule_methods [] =
{
    {"private_initTestEnvironment", initTestEnvironment, METH_VARARGS, nullptr},
    {"getComponentContext", getComponentContext, METH_VARARGS, nullptr},
    {"_createUnoStructHelper", reinterpret_cast<PyCFunction>(createUnoStructHelper), METH_VARARGS | METH_KEYWORDS, nullptr},
    {"getTypeByName", getTypeByName, METH_VARARGS, nullptr},
    {"getConstantByName", getConstantByName, METH_VARARGS, nullptr},
    {"getClass", getClass, METH_VARARGS, nullptr},
    {"checkEnum", checkEnum, METH_VARARGS, nullptr},
    {"checkType", checkType, METH_VARARGS, nullptr},
    {"generateUuid", generateUuid, METH_VARARGS, nullptr},
    {"systemPathToFileUrl", systemPathToFileUrl, METH_VARARGS, nullptr},
    {"fileUrlToSystemPath", fileUrlToSystemPath, METH_VARARGS, nullptr},
    {"absolutize", absolutize, METH_VARARGS | METH_KEYWORDS, nullptr},
    {"isInterface", isInterface, METH_VARARGS, nullptr},
    {"invoke", invoke, METH_VARARGS | METH_KEYWORDS, nullptr},
    {"setCurrentContext", setCurrentContext, METH_VARARGS, nullptr},
    {"getCurrentContext", getCurrentContext, METH_VARARGS, nullptr},
    {nullptr, nullptr, 0, nullptr}
};

}

extern "C"
#if PY_MAJOR_VERSION >= 3
PyObject* PyInit_pyuno()
{
    PyUNO_initType();
    PyUNOStruct_initType();
    // noop when called already, otherwise needed to allow multiple threads
    PyEval_InitThreads();
    static struct PyModuleDef moduledef =
    {
        PyModuleDef_HEAD_INIT,
        "pyuno",             // module name
        nullptr,                   // module documentation
        -1,                  // module keeps state in global variables,
        PyUNOModule_methods, // modules methods
        nullptr,                   // m_reload (must be 0)
        nullptr,                   // m_traverse
        nullptr,                   // m_clear
        nullptr,                   // m_free
    };
    return PyModule_Create(&moduledef);
}
#else
void initpyuno()
{
    PyUNO_initType();
    PyUNOStruct_initType();
    PyEval_InitThreads();
    Py_InitModule ("pyuno", PyUNOModule_methods);
}
#endif /* PY_MAJOR_VERSION >= 3 */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
