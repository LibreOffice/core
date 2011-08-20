/* -*- Mode: C++; eval:(c-set-style "bsd"); tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

#include <com/sun/star/reflection/XIdlReflection.hpp>
#include <com/sun/star/reflection/XIdlClass.hpp>
#include <com/sun/star/registry/InvalidRegistryException.hpp>

using osl::Module;

using rtl::OString;
using rtl::OUString;
using rtl::OUStringHash;
using rtl::OUStringToOString;
using rtl::OUStringBuffer;
using rtl::OStringBuffer;

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
    typedef std::unordered_map <const OUString, bool, OUStringHash> initialised_t;
    // Keyword arguments used
    PyObject *used;
    // Which structure members are initialised
    std::unordered_map <const OUString, bool, OUStringHash> initialised;
    // How many positional arguments are consumed
    // This is always the so-many first ones
    sal_Int32 nPosConsumed;
    // The total number of members set, either by a keyword argument or a positional argument
    unsigned int nMembersSet;

public:
    fillStructState()
        : used (PyDict_New())
        , initialised ()
        , nPosConsumed (0)
        , nMembersSet (0)
    {
        if ( ! used )
            throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("pyuno._createUnoStructHelper failed to create new dictionary")), Reference< XInterface > ());
    }
    ~fillStructState()
    {
        Py_DECREF(used);
    }
    void setUsed(PyObject *key)
    {
        PyDict_SetItem(used, key, Py_True);
    }
    bool isUsed(PyObject *key) const
    {
        return Py_True == PyDict_GetItem(used, key);
    }
    void setInitialised(OUString key, int pos = -1)
    {
        if (initialised[key])
        {
            OUStringBuffer buf;
            buf.appendAscii( "pyuno._createUnoStructHelper: member '");
            buf.append(key);
            buf.appendAscii( "'");
            if ( pos >= 0 )
            {
                buf.appendAscii( " at position ");
                buf.append(pos);
            }
            buf.appendAscii( " initialised multiple times.");
            throw RuntimeException(buf.makeStringAndClear(), Reference< XInterface > ());
        }
        initialised[key] = true;
        ++nMembersSet;
        if ( pos >= 0 )
            ++nPosConsumed;
    }
    bool isInitialised(OUString key)
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
    int getCntMembersSet() const
    {
        return nMembersSet;
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
            PyObject *pyMemberName = PyString_FromString(::rtl::OUStringToOString( OUMemberName, RTL_TEXTENCODING_UTF8 ).getStr());
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
    for ( int i = 0; i < nMembers ; ++i)
    {
        const OUString memberName (pCompType->ppMemberNames[i]);
        if ( ! state.isInitialised( memberName ) )
        {
            OUStringBuffer buf;
            buf.appendAscii( "pyuno._createUnoStructHelper: member '");
            buf.append(memberName);
            buf.appendAscii( "' of struct type '");
            buf.append(pCompType->aBase.pTypeName);
            buf.appendAscii( "' not given a value.");
            throw RuntimeException(buf.makeStringAndClear(), Reference< XInterface > ());
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
                OUString name ( RTL_CONSTASCII_USTRINGPARAM( "PYUNOLIBDIR" ) );
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

static PyObject* getComponentContext (PyObject*, PyObject*)
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
            if( !path.getLength() )
            {
                PyErr_SetString(
                    PyExc_RuntimeError, "osl_getUrlFromAddress fails, that's why I cannot find ini "
                    "file for bootstrapping python uno bridge\n" );
                return NULL;
            }

            OUStringBuffer iniFileName;
            iniFileName.append( path );
            iniFileName.appendAscii( "/" );
            iniFileName.appendAscii( SAL_CONFIGFILE( "pyuno" ) );
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
    catch (com::sun::star::registry::InvalidRegistryException &e)
    {
        // can't use raisePyExceptionWithAny() here, because the function
        // does any conversions, which will not work with a
        // wrongly bootstrapped pyuno!
        raisePySystemException( "InvalidRegistryException", e.Message );
    }
    catch( com::sun::star::lang::IllegalArgumentException & e)
    {
        raisePySystemException( "IllegalArgumentException", e.Message );
    }
    catch( com::sun::star::script::CannotConvertException & e)
    {
        raisePySystemException( "CannotConvertException", e.Message );
    }
    catch (com::sun::star::uno::RuntimeException & e)
    {
        raisePySystemException( "RuntimeException", e.Message );
    }
    catch (com::sun::star::uno::Exception & e)
    {
        raisePySystemException( "uno::Exception", e.Message );
    }
    return ret.getAcquired();
}

PyObject * extractOneStringArg( PyObject *args, char const *funcName )
{
    if( !PyTuple_Check( args ) || PyTuple_Size( args) != 1 )
    {
        OStringBuffer buf;
        buf.append( funcName ).append( ": expecting one string argument" );
        PyErr_SetString( PyExc_RuntimeError, buf.getStr() );
        return NULL;
    }
    PyObject *obj = PyTuple_GetItem( args, 0 );
    if(!PyString_Check(obj) && !PyUnicode_Check(obj))
    {
        OStringBuffer buf;
        buf.append( funcName ).append( ": expecting one string argument" );
        PyErr_SetString( PyExc_TypeError, buf.getStr());
        return NULL;
    }
    return obj;
}

static PyObject *createUnoStructHelper(PyObject *, PyObject* args, PyObject* keywordArgs)
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

            // Perhaps in Python 3, only PyUnicode_Check returns true and
            // in Python 2, only PyString_Check returns true.
            if(PyString_Check(structName) || PyUnicode_Check(structName))
            {
                if( PyTuple_Check( initializer ) && PyDict_Check ( keywordArgs ) )
                {
                    OUString typeName( OUString::createFromAscii(PyString_AsString(structName)));
                    RuntimeCargo *c = runtime.getImpl()->cargo;
                    Reference<XIdlClass> idl_class ( c->xCoreReflection->forName (typeName),UNO_QUERY);
                    if (idl_class.is ())
                    {
                        idl_class->createObject (IdlStruct);
                        PyUNO *me = (PyUNO*)PyUNO_new_UNCHECKED( IdlStruct, c->xInvocation );
                        PyRef returnCandidate( (PyObject*)me, SAL_NO_ACQUIRE );
                        TypeDescription desc( typeName );
                        OSL_ASSERT( desc.is() ); // could already instantiate an XInvocation2 !

                        typelib_CompoundTypeDescription *pCompType =
                            ( typelib_CompoundTypeDescription * ) desc.get();
                        fillStructState state;
                        if ( PyTuple_Size( initializer ) > 0 || PyDict_Size( keywordArgs ) > 0 )
                            fillStruct( me->members->xInvocation, pCompType, initializer, keywordArgs, state, runtime );
                        if( state.getCntConsumed() != PyTuple_Size(initializer) )
                        {
                            OUStringBuffer buf;
                            buf.appendAscii( "pyuno._createUnoStructHelper: too many ");
                            buf.appendAscii( "elements in the initializer list, expected " );
                            buf.append( state.getCntConsumed() );
                            buf.appendAscii( ", got " );
                            buf.append( (sal_Int32) PyTuple_Size(initializer) );
                            throw RuntimeException( buf.makeStringAndClear(), Reference< XInterface > ());
                        }
                        ret = PyRef( PyTuple_Pack(2, returnCandidate.get(), state.getUsed()), SAL_NO_ACQUIRE);
                    }
                    else
                    {
                        OStringBuffer buf;
                        buf.append( "UNO struct " );
                        buf.append( PyString_AsString(structName) );
                        buf.append( " is unkown" );
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
    catch( com::sun::star::uno::RuntimeException & e )
    {
        raisePyExceptionWithAny( makeAny( e ) );
    }
    catch( com::sun::star::script::CannotConvertException & e )
    {
        raisePyExceptionWithAny( makeAny( e ) );
    }
    catch( com::sun::star::uno::Exception & e )
    {
        raisePyExceptionWithAny( makeAny( e ) );
    }
    return ret.getAcquired();
}

static PyObject *getTypeByName( PyObject *, PyObject *args )
{
    PyObject * ret = NULL;

    try
    {
        char *name;

        if (PyArg_ParseTuple (args, const_cast< char * >("s"), &name))
        {
            OUString typeName ( OUString::createFromAscii( name ) );
            TypeDescription typeDesc( typeName );
            if( typeDesc.is() )
            {
                Runtime runtime;
                ret = PyUNO_Type_new(
                    name, (com::sun::star::uno::TypeClass)typeDesc.get()->eTypeClass, runtime );
            }
            else
            {
                OStringBuffer buf;
                buf.append( "Type " ).append(name).append( " is unknown" );
                PyErr_SetString( PyExc_RuntimeError, buf.getStr() );
            }
        }
    }
    catch ( RuntimeException & e )
    {
        raisePyExceptionWithAny( makeAny( e ) );
    }
    return ret;
}

static PyObject *getConstantByName( PyObject *, PyObject *args )
{
    PyObject *ret = 0;
    try
    {
        char *name;

        if (PyArg_ParseTuple (args, const_cast< char * >("s"), &name))
        {
            OUString typeName ( OUString::createFromAscii( name ) );
            Runtime runtime;
            Any a = runtime.getImpl()->cargo->xTdMgr->getByHierarchicalName(typeName);
            if( a.getValueType().getTypeClass() ==
                com::sun::star::uno::TypeClass_INTERFACE )
            {
                // a idl constant cannot be an instance of an uno-object, thus
                // this cannot be a constant
                OUStringBuffer buf;
                buf.appendAscii( "pyuno.getConstantByName: " ).append( typeName );
                buf.appendAscii( "is not a constant" );
                throw RuntimeException(buf.makeStringAndClear(), Reference< XInterface > () );
            }
            PyRef constant = runtime.any2PyObject( a );
            ret = constant.getAcquired();
        }
    }
    catch( NoSuchElementException & e )
    {
        // to the python programmer, this is a runtime exception,
        // do not support tweakings with the type system
        RuntimeException runExc( e.Message, Reference< XInterface > () );
        raisePyExceptionWithAny( makeAny( runExc ) );
    }
    catch( com::sun::star::script::CannotConvertException & e)
    {
        raisePyExceptionWithAny( makeAny( e ) );
    }
    catch( com::sun::star::lang::IllegalArgumentException & e)
    {
        raisePyExceptionWithAny( makeAny( e ) );
    }
    catch( RuntimeException & e )
    {
        raisePyExceptionWithAny( makeAny(e) );
    }
    return ret;
}

static PyObject *checkType( PyObject *, PyObject *args )
{
    if( !PyTuple_Check( args ) || PyTuple_Size( args) != 1 )
    {
        OStringBuffer buf;
        buf.append( "pyuno.checkType : expecting one uno.Type argument" );
        PyErr_SetString( PyExc_RuntimeError, buf.getStr() );
        return NULL;
    }
    PyObject *obj = PyTuple_GetItem( args, 0 );

    try
    {
        PyType2Type( obj );
    }
    catch( RuntimeException & e)
    {
        raisePyExceptionWithAny( makeAny( e ) );
        return NULL;
    }
    Py_INCREF( Py_None );
    return Py_None;
}

static PyObject *checkEnum( PyObject *, PyObject *args )
{
    if( !PyTuple_Check( args ) || PyTuple_Size( args) != 1 )
    {
        OStringBuffer buf;
        buf.append( "pyuno.checkType : expecting one uno.Type argument" );
        PyErr_SetString( PyExc_RuntimeError, buf.getStr() );
        return NULL;
    }
    PyObject *obj = PyTuple_GetItem( args, 0 );

    try
    {
        PyEnum2Enum( obj );
    }
    catch( RuntimeException & e)
    {
        raisePyExceptionWithAny( makeAny( e) );
        return NULL;
    }
    Py_INCREF( Py_None );
    return Py_None;
}

static PyObject *getClass( PyObject *, PyObject *args )
{
    PyObject *obj = extractOneStringArg( args, "pyuno.getClass");
    if( ! obj )
        return NULL;

    try
    {
        Runtime runtime;
        PyRef ret = getClass(
            OUString( PyString_AsString( obj), strlen(PyString_AsString(obj)),RTL_TEXTENCODING_ASCII_US),
            runtime );
        Py_XINCREF( ret.get() );
        return ret.get();
    }
    catch( RuntimeException & e)
    {
        // NOOPT !!!
        // gcc 3.2.3 crashes here in the regcomp test scenario
        // only since migration to python 2.3.4 ???? strange thing
        // optimization switched off for this module !
        raisePyExceptionWithAny( makeAny(e) );
    }
    return NULL;
}

static PyObject *isInterface( PyObject *, PyObject *args )
{

    if( PyTuple_Check( args ) && PyTuple_Size( args ) == 1 )
    {
        PyObject *obj = PyTuple_GetItem( args, 0 );
        Runtime r;
        return PyLong_FromLong( isInterfaceClass( r, obj ) );
    }
    return PyLong_FromLong( 0 );
}

static PyObject * generateUuid( PyObject *, PyObject * )
{
    Sequence< sal_Int8 > seq( 16 );
    rtl_createUuid( (sal_uInt8*)seq.getArray() , 0 , sal_False );
    PyRef ret;
    try
    {
        Runtime runtime;
        ret = runtime.any2PyObject( makeAny( seq ) );
    }
    catch( RuntimeException & e )
    {
        raisePyExceptionWithAny( makeAny(e) );
    }
    return ret.getAcquired();
}

static PyObject *systemPathToFileUrl( PyObject *, PyObject * args )
{
    PyObject *obj = extractOneStringArg( args, "pyuno.systemPathToFileUrl" );
    if( ! obj )
        return NULL;

    OUString sysPath = pyString2ustring( obj );
    OUString url;
    osl::FileBase::RC e = osl::FileBase::getFileURLFromSystemPath( sysPath, url );

    if( e != osl::FileBase::E_None )
    {
        OUStringBuffer buf;
        buf.appendAscii( "Couldn't convert " );
        buf.append( sysPath );
        buf.appendAscii( " to a file url for reason (" );
        buf.append( (sal_Int32) e );
        buf.appendAscii( ")" );
        raisePyExceptionWithAny(
            makeAny( RuntimeException( buf.makeStringAndClear(), Reference< XInterface > () )));
        return NULL;
    }
    return ustring2PyUnicode( url ).getAcquired();
}

static PyObject * fileUrlToSystemPath( PyObject *, PyObject * args )
{
    PyObject *obj = extractOneStringArg( args, "pyuno.fileUrlToSystemPath" );
    if( ! obj )
        return NULL;

    OUString url = pyString2ustring( obj );
    OUString sysPath;
    osl::FileBase::RC e = osl::FileBase::getSystemPathFromFileURL( url, sysPath );

    if( e != osl::FileBase::E_None )
    {
        OUStringBuffer buf;
        buf.appendAscii( "Couldn't convert file url " );
        buf.append( sysPath );
        buf.appendAscii( " to a system path for reason (" );
        buf.append( (sal_Int32) e );
        buf.appendAscii( ")" );
        raisePyExceptionWithAny(
            makeAny( RuntimeException( buf.makeStringAndClear(), Reference< XInterface > () )));
        return NULL;
    }
    return ustring2PyUnicode( sysPath ).getAcquired();
}

static PyObject * absolutize( PyObject *, PyObject * args )
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
            buf.appendAscii( "Couldn't absolutize " );
            buf.append( ouRel );
            buf.appendAscii( " using root " );
            buf.append( ouPath );
            buf.appendAscii( " for reason (" );
            buf.append( (sal_Int32) e );
            buf.appendAscii( ")" );

            PyErr_SetString(
                PyExc_OSError,
                OUStringToOString(buf.makeStringAndClear(),osl_getThreadTextEncoding()));
            return 0;
        }
        return ustring2PyUnicode( ret ).getAcquired();
    }
    return 0;
}

static PyObject * invoke(PyObject *, PyObject *args)
{
    PyObject *ret = 0;
    if(PyTuple_Check(args) && PyTuple_Size(args) == 3)
    {
        PyObject *object = PyTuple_GetItem(args, 0);
        PyObject *item1 = PyTuple_GetItem(args, 1);
        if(PyString_Check(item1) || PyUnicode_Check(item1))
        {
            const char *name = PyString_AsString(item1);
            PyObject *item2 = PyTuple_GetItem(args, 2);
            if(PyTuple_Check(item2))
            {
                ret = PyUNO_invoke(object, name, item2);
            }
            else
            {
                OStringBuffer buf;
                buf.append("uno.invoke expects a tuple as 3rd argument, got ");
                buf.append(PyString_AsString(PyObject_Str(item2)));
                PyErr_SetString(PyExc_RuntimeError, buf.makeStringAndClear());
            }
        }
        else
        {
            OStringBuffer buf;
            buf.append("uno.invoke expected a string as 2nd argument, got ");
            buf.append(PyString_AsString(PyObject_Str(item1)));
            PyErr_SetString(PyExc_RuntimeError, buf.makeStringAndClear());
        }
    }
    else
    {
        OStringBuffer buf;
        buf.append("uno.invoke expects object, name, (arg1, arg2, ... )\n");
        PyErr_SetString(PyExc_RuntimeError, buf.makeStringAndClear());
    }
    return ret;
}

static PyObject *getCurrentContext( PyObject *, PyObject * )
{
    PyRef ret;
    try
    {
        Runtime runtime;
        ret = runtime.any2PyObject(
            makeAny( com::sun::star::uno::getCurrentContext() ) );
    }
    catch( com::sun::star::uno::Exception & e )
    {
        raisePyExceptionWithAny( makeAny( e ) );
    }
    return ret.getAcquired();
}

static PyObject *setCurrentContext( PyObject *, PyObject * args )
{
    PyRef ret;
    try
    {
        if( PyTuple_Check( args ) && PyTuple_Size( args ) == 1 )
        {

            Runtime runtime;
            Any a = runtime.pyObject2Any( PyTuple_GetItem( args, 0 ) );

            Reference< com::sun::star::uno::XCurrentContext > context;

            if( (a.hasValue() && (a >>= context)) || ! a.hasValue() )
            {
                ret = com::sun::star::uno::setCurrentContext( context ) ? Py_True : Py_False;
            }
            else
            {
                OStringBuffer buf;
                buf.append( "uno.setCurrentContext expects an XComponentContext implementation, got " );
                buf.append( PyString_AsString( PyObject_Str( PyTuple_GetItem( args, 0) ) ) );
                PyErr_SetString( PyExc_RuntimeError, buf.makeStringAndClear() );
            }
        }
        else
        {
            OStringBuffer buf;
            buf.append( "uno.setCurrentContext expects exactly one argument (the current Context)\n" );
            PyErr_SetString( PyExc_RuntimeError, buf.makeStringAndClear() );
        }
    }
    catch( com::sun::star::uno::Exception & e )
    {
        raisePyExceptionWithAny( makeAny( e ) );
    }
    return ret.getAcquired();
}

}

struct PyMethodDef PyUNOModule_methods [] =
{
    {const_cast< char * >("getComponentContext"), getComponentContext, METH_VARARGS, NULL},
    {const_cast< char * >("_createUnoStructHelper"), reinterpret_cast<PyCFunction>(createUnoStructHelper), METH_VARARGS | METH_KEYWORDS, NULL},
    {const_cast< char * >("getTypeByName"), getTypeByName, METH_VARARGS, NULL},
    {const_cast< char * >("getConstantByName"), getConstantByName, METH_VARARGS, NULL},
    {const_cast< char * >("getClass"), getClass, METH_VARARGS, NULL},
    {const_cast< char * >("checkEnum"), checkEnum, METH_VARARGS, NULL},
    {const_cast< char * >("checkType"), checkType, METH_VARARGS, NULL},
    {const_cast< char * >("generateUuid"), generateUuid, METH_VARARGS, NULL},
    {const_cast< char * >("systemPathToFileUrl"), systemPathToFileUrl, METH_VARARGS, NULL},
    {const_cast< char * >("fileUrlToSystemPath"), fileUrlToSystemPath, METH_VARARGS, NULL},
    {const_cast< char * >("absolutize"), absolutize, METH_VARARGS | METH_KEYWORDS, NULL},
    {const_cast< char * >("isInterface"), isInterface, METH_VARARGS, NULL},
    {const_cast< char * >("invoke"), invoke, METH_VARARGS | METH_KEYWORDS, NULL},
    {const_cast< char * >("setCurrentContext"), setCurrentContext, METH_VARARGS, NULL},
    {const_cast< char * >("getCurrentContext"), getCurrentContext, METH_VARARGS, NULL},
    {NULL, NULL, 0, NULL}
};

}

extern "C" PY_DLLEXPORT
#if PY_MAJOR_VERSION >= 3
PyObject* PyInit_pyuno()
{
    // noop when called already, otherwise needed to allow multiple threads
    PyEval_InitThreads();
    static struct PyModuleDef moduledef =
    {
        PyModuleDef_HEAD_INIT,
        "pyuno",             // module name
        0,                   // module documentation
        -1,                  // module keeps state in global variables,
        PyUNOModule_methods, // modules methods
        0,                   // m_reload (must be 0)
        0,                   // m_traverse
        0,                   // m_clear
        0,                   // m_free
    };
    return PyModule_Create(&moduledef);
}
#else
void initpyuno()
{
    PyEval_InitThreads();
    Py_InitModule (const_cast< char * >("pyuno"), PyUNOModule_methods);
}
#endif /* PY_MAJOR_VERSION >= 3 */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
