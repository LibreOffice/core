/*************************************************************************
 *
 *  $RCSfile: pyuno_module.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hjs $ $Date: 2003-08-18 15:01:47 $
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
#include <osl/thread.h>
#include <osl/file.hxx>

#include <typelib/typedescription.hxx>

#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/uuid.h>
#include <rtl/bootstrap.hxx>

#include <cppuhelper/bootstrap.hxx>

#include "pyuno_impl.hxx"

#include <com/sun/star/reflection/XIdlReflection.hpp>
#include <com/sun/star/reflection/XIdlClass.hpp>

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

namespace pyuno
{

/**
   @ index of the next to be used member in the initializer list !
 */
static sal_Int32 fillStructWithInitializer(
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
        Any a = runtime.pyObject2Any( element );
        inv->setValue( pCompType->ppMemberNames[i], a );
    }
    return i+nIndex;
}

static PyObject *createUnoStructHelper(PyObject *self, PyObject* args )
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

            if( PyString_Check( structName ) )
            {
                if( PyTuple_Check( initializer ) )
                {
                    OUString typeName( OUString::createFromAscii(PyString_AsString(structName)));
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
static OUString getLibDir()
{
    static OUString *pLibDir;
    if( !pLibDir )
    {
        osl::MutexGuard guard( osl::Mutex::getGlobalMutex() );
        if( ! pLibDir )
        {
            static OUString libDir;

            // workarounds the $(ORIGIN) until it is available
            if( Module::getUrlFromAddress( reinterpret_cast<void*>(getLibDir) , libDir ) )
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

static PyObject* getComponentContext (PyObject* self, PyObject* args)
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

            PyThreadDetach antiguard;
            ctx = cppu::defaultBootstrap_InitialComponentContext (iniFile);
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
        raisePyExceptionWithAny( makeAny(e) );
    }
    catch( com::sun::star::lang::IllegalArgumentException & e)
    {
        raisePyExceptionWithAny( makeAny(e));
    }
    catch( com::sun::star::script::CannotConvertException & e)
    {
        raisePyExceptionWithAny( makeAny(e));
    }
    catch (com::sun::star::uno::RuntimeException & e)
    {
        raisePyExceptionWithAny( makeAny(e) );
    }
    catch (com::sun::star::uno::Exception & e)
    {
        raisePyExceptionWithAny( makeAny(e) );
    }
    return ret.getAcquired();
}

static PyObject *getTypeByName( PyObject *self, PyObject *args )
{
    PyObject * ret = NULL;

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

static PyObject * extractOneStringArg( PyObject *args, char *funcName )
{
    if( !PyTuple_Check( args ) || PyTuple_Size( args) != 1 )
    {
        OStringBuffer buf;
        buf.append( funcName ).append( ": expecting one string argument" );
        PyErr_SetString( PyExc_RuntimeError, buf.getStr() );
        return NULL;
    }
    PyObject *obj = PyTuple_GetItem( args, 0 );
    if( !PyString_Check( obj ) && ! PyUnicode_Check(obj))
    {
        OStringBuffer buf;
        buf.append( funcName ).append( ": expecting one string argument" );
        PyErr_SetString( PyExc_TypeError, buf.getStr());
        return NULL;
    }
    return obj;
}


static PyObject *getConstantByName( PyObject *self, PyObject *args )
{
    PyObject *ret = 0;
    try
    {
        char *name;

        if (PyArg_ParseTuple (args, "s", &name))
        {
            OUString typeName ( OUString::createFromAscii( name ) );
            Runtime runtime;
            Any a = runtime.getImpl()->cargo->xTdMgr->getByHierarchicalName(typeName);
            if( a.getValueType().getTypeClass() ==
                com::sun::star::uno::TypeClass_INTERFACE )
            {
                // a ídl constant cannot be an instance of an uno-object, thus
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

static PyObject *checkType( PyObject *self, PyObject *args )
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
        Runtime runtime;
        PyType2Type( obj , runtime );
    }
    catch( RuntimeException & e)
    {
        raisePyExceptionWithAny( makeAny( e ) );
        return NULL;
    }
    Py_INCREF( Py_None );
    return Py_None;
}

static PyObject *checkEnum( PyObject *self, PyObject *args )
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
        Runtime runtime;
        PyEnum2Enum( obj , runtime );
    }
    catch( RuntimeException & e)
    {
        raisePyExceptionWithAny( makeAny( e) );
        return NULL;
    }
    Py_INCREF( Py_None );
    return Py_None;
}

static PyObject *getClass( PyObject *self, PyObject *args )
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
        raisePyExceptionWithAny( makeAny(e) );
    }
    return NULL;
}

static PyObject *isInterface( PyObject *self, PyObject *args )
{

    if( PyTuple_Check( args ) && PyTuple_Size( args ) == 1 )
    {
        PyObject *obj = PyTuple_GetItem( args, 0 );
        Runtime r;
        return PyInt_FromLong( isInterfaceClass( r, obj ) );
    }
    return PyInt_FromLong( 0 );
}

static PyObject * generateUuid( PyObject *self, PyObject *args )
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

static PyObject *systemPathToFileUrl( PyObject * self, PyObject * args )
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


static PyObject * fileUrlToSystemPath( PyObject *self, PyObject * args )
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


static PyObject * absolutize( PyObject *self, PyObject * args )
{
    Py_UNICODE * path = 0;
    Py_UNICODE * rel = 0;
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
        return PyUnicode_FromUnicode( (const Py_UNICODE * ) ret.pData->buffer, ret.pData->length );
    }
    return 0;
}

static PyObject * invoke ( PyObject *self, PyObject * args )
{
    PyObject *ret = 0;
    if( PyTuple_Check( args ) && PyTuple_Size( args ) == 3 )
    {
        PyObject *object = PyTuple_GetItem( args, 0 );

        if( PyString_Check( PyTuple_GetItem( args, 1 ) ) )
        {
            const char *name = PyString_AsString( PyTuple_GetItem( args, 1 ) );
            if( PyTuple_Check( PyTuple_GetItem( args , 2 )))
            {
                ret = PyUNO_invoke( object, name , PyTuple_GetItem( args, 2 ) );
            }
            else
            {
                OStringBuffer buf;
                buf.append( "uno.invoke expects a tuple as 3rd argument, got " );
                buf.append( PyString_AsString( PyObject_Str( PyTuple_GetItem( args, 2) ) ) );
                PyErr_SetString( PyExc_RuntimeError, buf.makeStringAndClear() );
            }
        }
        else
        {
            OStringBuffer buf;
            buf.append( "uno.invoke expected a string as 2nd argument, got " );
            buf.append( PyString_AsString( PyObject_Str( PyTuple_GetItem( args, 1) ) ) );
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

}

using namespace pyuno;

static struct PyMethodDef PyUNOModule_methods [] =
{
    {"getComponentContext", getComponentContext, 1},
    {"_createUnoStructHelper", createUnoStructHelper, 2},
    {"getTypeByName", getTypeByName, 1},
    {"getConstantByName", getConstantByName,1},
    {"getClass", getClass,1},
    {"checkEnum", checkEnum, 1},
    {"checkType", checkType, 1},
    {"generateUuid", generateUuid,0},
    {"systemPathToFileUrl",systemPathToFileUrl,1},
    {"fileUrlToSystemPath",fileUrlToSystemPath,1},
    {"absolutize",absolutize,2},
    {"isInterface",isInterface,1},
    {"invoke",invoke, 2},
    {NULL, NULL}
};


extern "C" PY_DLLEXPORT void initpyuno()
{
    // noop when called already, otherwise needed to allow multiple threads
    PyEval_InitThreads();
    Py_InitModule ("pyuno", PyUNOModule_methods);
}
