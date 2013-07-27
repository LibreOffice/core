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



#include "pyuno_impl.hxx"

#include <osl/module.hxx>
#include <osl/thread.h>
#include <osl/file.hxx>

#include <typelib/typedescription.hxx>

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
sal_Int32 fillStructWithInitializer(
    const Reference< XInvocation2 > &inv,
    typelib_CompoundTypeDescription *pCompType,
    PyObject *initializer,
    const Runtime &runtime) throw ( RuntimeException )
{
    sal_Int32 nIndex = 0;
    if( pCompType->pBaseTypeDescription )
        nIndex = fillStructWithInitializer(
            inv, pCompType->pBaseTypeDescription, initializer, runtime );

    sal_Int32 nTupleSize =  PyTuple_Size(initializer);
    int i;
    for( i = 0 ; i < pCompType->nMembers ; i ++ )
    {
        if( i + nIndex >= nTupleSize )
        {
            OUStringBuffer buf;
            buf.appendAscii( "pyuno._createUnoStructHelper: too few elements in the initializer tuple,");
            buf.appendAscii( "expected at least " ).append( nIndex + pCompType->nMembers );
            buf.appendAscii( ", got " ).append( nTupleSize );
            throw RuntimeException(buf.makeStringAndClear(), Reference< XInterface > ());
        }
        PyObject *element = PyTuple_GetItem( initializer, i + nIndex );
        Any a = runtime.pyObject2Any( element, ACCEPT_UNO_ANY );
        inv->setValue( pCompType->ppMemberNames[i], a );
    }
    return i+nIndex;
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
#if PY_MAJOR_VERSION >= 3
    if( ! PyUnicode_Check(obj) )
#else
    if( !PyBytes_Check( obj ) && ! PyUnicode_Check(obj))
#endif
    {
        OStringBuffer buf;
        buf.append( funcName ).append( ": expecting one string argument" );
        PyErr_SetString( PyExc_TypeError, buf.getStr());
        return NULL;
    }
    return obj;
}

static PyObject *createUnoStructHelper(PyObject *, PyObject* args )
{
    Any IdlStruct;
    PyRef ret;

    try
    {
        Runtime runtime;
        if( PyTuple_Size( args ) == 2 )
        {
            PyObject *structName = PyTuple_GetItem( args,0 );
            PyObject *initializer = PyTuple_GetItem( args ,1 );

            if( PYSTR_CHECK( structName ) )
            {
                if( PyTuple_Check( initializer ) )
                {
                    OUString typeName( pyString2ustring( structName ) );
                    RuntimeCargo *c = runtime.getImpl()->cargo;
                    Reference<XIdlClass> idl_class ( c->xCoreReflection->forName (typeName),UNO_QUERY);
                    if (idl_class.is ())
                    {
                        idl_class->createObject (IdlStruct);
                        PyUNO *me = (PyUNO*)PyUNO_new_UNCHECKED( IdlStruct, c->xInvocation );
                        PyRef returnCandidate( (PyObject*)me, SAL_NO_ACQUIRE );
                        if( PyTuple_Size( initializer ) > 0 )
                        {
                            TypeDescription desc( typeName );
                            OSL_ASSERT( desc.is() ); // could already instantiate an XInvocation2 !

                            typelib_CompoundTypeDescription *pCompType =
                                ( typelib_CompoundTypeDescription * ) desc.get();
                            sal_Int32 n = fillStructWithInitializer(
                                me->members->xInvocation, pCompType, initializer, runtime );
                            if( n != PyTuple_Size(initializer) )
                            {
                                OUStringBuffer buf;
                                buf.appendAscii( "pyuno._createUnoStructHelper: wrong number of ");
                                buf.appendAscii( "elements in the initializer list, expected " );
                                buf.append( n );
                                buf.appendAscii( ", got " );
                                buf.append( (sal_Int32) PyTuple_Size(initializer) );
                                throw RuntimeException(
                                    buf.makeStringAndClear(), Reference< XInterface > ());
                            }
                        }
                        ret = returnCandidate;
                    }
                    else
                    {
                        OStringBuffer buf;
                        buf.append( "UNO struct " );
                        buf.append( OUStringToOString( typeName, RTL_TEXTENCODING_ASCII_US ) );
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
            PyErr_SetString (PyExc_AttributeError, "1 Arguments: Structure Name");
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
        PyRef ret = getClass( pyString2ustring(obj), runtime );
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
#if PY_MAJOR_VERSION >= 3
        return PyLong_FromLong( isInterfaceClass( r, obj ) );
#else
        return PyInt_FromLong( isInterfaceClass( r, obj ) );

#endif
    }
#if PY_MAJOR_VERSION >= 3
    return PyLong_FromLong( 0 );
#else
    return PyInt_FromLong( 0 );
#endif
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

static PyObject * invoke ( PyObject *, PyObject * args )
{
    PyObject *ret = 0;
    if( PyTuple_Check( args ) && PyTuple_Size( args ) == 3 )
    {
        PyObject *object = PyTuple_GetItem( args, 0 );

        if( PYSTR_CHECK( PyTuple_GetItem( args, 1 ) ) )
        {
#if PY_VERSION_HEX >= 0x03030000
            const char *name = PyUnicode_AsUTF8( PyTuple_GetItem( args, 1 ) );
#elif PY_MAJOR_VERSION >= 3
            PyRef pUtf8(PyUnicode_AsUTF8String( PyTuple_GetItem( args, 1 ) ), SAL_NO_ACQUIRE);
            const char *name = PyBytes_AsString( pUtf8.get() );
#else
            const char *name = PyBytes_AsString( PyTuple_GetItem( args, 1 ) );
#endif
            if( PyTuple_Check( PyTuple_GetItem( args , 2 )))
            {
                ret = PyUNO_invoke( object, name , PyTuple_GetItem( args, 2 ) );
            }
            else
            {
                OStringBuffer buf;
                buf.append( "uno.invoke expects a tuple as 3rd argument, got " );
#if PY_MAJOR_VERSION >= 3
                buf.append( OUStringToOString( pyString2ustring( PyTuple_GetItem( args, 2 ) ), RTL_TEXTENCODING_ASCII_US) );
#else
                buf.append( PyBytes_AsString( PyObject_Str( PyTuple_GetItem( args, 2) ) ) );
#endif
                PyErr_SetString( PyExc_RuntimeError, buf.makeStringAndClear() );
            }
        }
        else
        {
            OStringBuffer buf;
            buf.append( "uno.invoke expected a string as 2nd argument, got " );
#if PY_MAJOR_VERSION >= 3
            buf.append( OUStringToOString( pyString2ustring( PyTuple_GetItem( args, 1 ) ), RTL_TEXTENCODING_ASCII_US ) );
#else
            buf.append( PyBytes_AsString( PyObject_Str( PyTuple_GetItem( args, 1) ) ) );
#endif
            PyErr_SetString( PyExc_RuntimeError, buf.makeStringAndClear() );
        }
    }
    else
    {
        OStringBuffer buf;
        buf.append( "uno.invoke expects object, name, (arg1, arg2, ... )\n" );
        PyErr_SetString( PyExc_RuntimeError, buf.makeStringAndClear() );
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
#if PY_MAJOR_VERSION >= 3
                buf.append( OUStringToOString( pyString2ustring( PyTuple_GetItem( args, 0 ) ), RTL_TEXTENCODING_ASCII_US ) );
#else
                buf.append( PyBytes_AsString( PyObject_Str( PyTuple_GetItem( args, 0) ) ) );
#endif
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
    {const_cast< char * >("getComponentContext"), getComponentContext, METH_NOARGS, NULL},
    {const_cast< char * >("_createUnoStructHelper"), createUnoStructHelper, METH_VARARGS, NULL},
    {const_cast< char * >("getTypeByName"), getTypeByName, METH_VARARGS, NULL},
    {const_cast< char * >("getConstantByName"), getConstantByName, METH_VARARGS, NULL},
    {const_cast< char * >("getClass"), getClass, METH_VARARGS, NULL},
    {const_cast< char * >("checkEnum"), checkEnum, METH_VARARGS, NULL},
    {const_cast< char * >("checkType"), checkType, METH_VARARGS, NULL},
    {const_cast< char * >("generateUuid"), generateUuid, METH_NOARGS, NULL},
    {const_cast< char * >("systemPathToFileUrl"), systemPathToFileUrl, METH_VARARGS, NULL},
    {const_cast< char * >("fileUrlToSystemPath"), fileUrlToSystemPath, METH_VARARGS, NULL},
    {const_cast< char * >("absolutize"), absolutize, METH_VARARGS, NULL},
    {const_cast< char * >("isInterface"), isInterface, METH_VARARGS, NULL},
    {const_cast< char * >("invoke"), invoke, METH_VARARGS, NULL},
    {const_cast< char * >("setCurrentContext"), setCurrentContext, METH_VARARGS, NULL},
    {const_cast< char * >("getCurrentContext"), getCurrentContext, METH_NOARGS, NULL},
    {NULL, NULL, 0, NULL}
};

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef PyUNOModule =
{
    PyModuleDef_HEAD_INIT,
    const_cast< char * >("pyuno"),
    NULL,
    -1,
    PyUNOModule_methods
};
#endif
}

#if PY_MAJOR_VERSION >= 3
extern "C" PyMODINIT_FUNC PyInit_pyuno(void)
{
    PyObject *m;

    PyEval_InitThreads();

    m = PyModule_Create(&PyUNOModule);
    if (m == NULL)
        return NULL;

    if (PyType_Ready((PyTypeObject *)getPyUnoClass().get()))
        return NULL;
    return m;
}
#else
extern "C" PY_DLLEXPORT void initpyuno()
{
    // noop when called already, otherwise needed to allow multiple threads
    // This has to be reworked for Python 3.
    PyEval_InitThreads();
    Py_InitModule (const_cast< char * >("pyuno"), PyUNOModule_methods);
}
#endif
