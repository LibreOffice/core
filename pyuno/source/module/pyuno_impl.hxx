/*************************************************************************
 *
 *  $RCSfile: pyuno_impl.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jbu $ $Date: 2003-03-23 12:12:57 $
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
#ifndef _PYUNO_IMPL_
#define _PYUNO_IMPL_

#include <hash_map>
#include <hash_set>

#include <pyuno/pyuno.hxx>

#include <com/sun/star/beans/XIntrospection.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/script/XInvocation2.hpp>
#include <com/sun/star/script/XInvocationAdapterFactory2.hpp>

#include <com/sun/star/reflection/XIdlReflection.hpp>

#include <com/sun/star/container/XHierarchicalNameAccess.hpp>

#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/weakref.hxx>

namespace pyuno
{

typedef ::std::hash_map
<
    PyRef,
    com::sun::star::uno::WeakReference< com::sun::star::uno::XInterface >,
    PyRef::Hash,
    std::equal_to< PyRef >
> PyRef2Adapter;


typedef ::std::hash_map
<
rtl::OUString,
PyRef,
rtl::OUStringHash,
std::equal_to<rtl::OUString>
> ExceptionClassMap;


typedef ::std::hash_set< PyRef , PyRef::Hash , std::equal_to<PyRef> > ClassSet;

PyObject* PyUNO_new(
    const com::sun::star::uno::Any & targetInterface,
    const com::sun::star::uno::Reference<com::sun::star::lang::XSingleServiceFactory> & ssf);

PyObject* PyUNO_new_UNCHECKED (
    const com::sun::star::uno::Any & targetInterface,
    const com::sun::star::uno::Reference<com::sun::star::lang::XSingleServiceFactory> & ssf);

typedef struct
{
    com::sun::star::uno::Reference <com::sun::star::script::XInvocation2> xInvocation;
    com::sun::star::uno::Any wrappedObject;
} PyUNOInternals;

typedef struct
{
    PyObject_HEAD
    PyUNOInternals* members;
} PyUNO;

PyRef ustring2PyUnicode( const rtl::OUString &source );
PyRef ustring2PyString( const ::rtl::OUString & source );
rtl::OUString pyString2ustring( PyObject *str );


PyRef AnyToPyObject (const com::sun::star::uno::Any & a, const Runtime &r )
    throw ( com::sun::star::uno::RuntimeException );

com::sun::star::uno::Any PyObjectToAny (PyObject* o)
    throw ( com::sun::star::uno::RuntimeException );

void raiseInvocationTargetExceptionWhenNeeded( const Runtime &runtime )
    throw ( com::sun::star::reflection::InvocationTargetException );

// bool CheckPyObjectTypes (PyObject* o, Sequence<Type> types);
// bool CheckPyObjectType (PyObject* o, Type type); //Only check 1 object.

com::sun::star::uno::TypeClass StringToTypeClass (char* string);

PyRef PyUNO_callable_new (
    const com::sun::star::uno::Reference<com::sun::star::script::XInvocation2> &xInv,
    const rtl::OUString &methodName,
    const com::sun::star::uno::Reference<com::sun::star::lang::XSingleServiceFactory> &ssf,
    const com::sun::star::uno::Reference<com::sun::star::script::XTypeConverter> &tc );

PyObject* PyUNO_Type_new (const char *typeName , com::sun::star::uno::TypeClass t , const Runtime &r );
PyObject* PyUNO_Enum_new( const char *enumBase, const char *enumValue, const Runtime &r );
PyObject* PyUNO_char_new (sal_Unicode c , const Runtime &r);
PyObject *PyUNO_ByteSequence_new( const com::sun::star::uno::Sequence< sal_Int8 > &, const Runtime &r );

PyObject *importToGlobal( PyObject *typeName, PyObject *dict, PyObject *targetName );

PyRef getTypeClass( const Runtime &);
PyRef getEnumClass( const Runtime &);
PyRef getBoolClass( const Runtime &);
PyRef getCharClass( const Runtime &);
PyRef getByteSequenceClass( const Runtime & );
PyRef getPyUnoClass( const Runtime &);
PyRef getClass( const rtl::OUString & name , const Runtime & runtime );

com::sun::star::uno::Any PyEnum2Enum( PyObject *obj, const Runtime & r )
    throw ( com::sun::star::uno::RuntimeException );
sal_Bool PyBool2Bool( PyObject *o, const Runtime & r )
    throw ( com::sun::star::uno::RuntimeException );
sal_Unicode PyChar2Unicode( PyObject *o, const Runtime & r )
    throw ( com::sun::star::uno::RuntimeException );
com::sun::star::uno::Type PyType2Type( PyObject * o, const Runtime & r )
    throw( com::sun::star::uno::RuntimeException );

void raisePyExceptionWithAny( const com::sun::star::uno::Any &a );
const char *typeClassToString( com::sun::star::uno::TypeClass t );

PyRef getObjectFromUnoModule( const Runtime &runtime, const char * object )
    throw ( com::sun::star::uno::RuntimeException );

sal_Bool isInterfaceClass( const Runtime &, PyObject *obj );
sal_Bool isInstanceOfStructOrException( const Runtime & runtime, PyObject *obj);
com::sun::star::uno::Sequence<com::sun::star::uno::Type> implementsInterfaces(
    const Runtime & runtime, PyObject *obj );

struct RuntimeCargo
{
    com::sun::star::uno::Reference< com::sun::star::lang::XSingleServiceFactory > xInvocation;
    com::sun::star::uno::Reference< com::sun::star::script::XTypeConverter> xTypeConverter;
    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > xContext;
    com::sun::star::uno::Reference< com::sun::star::reflection::XIdlReflection > xCoreReflection;
    com::sun::star::uno::Reference< com::sun::star::container::XHierarchicalNameAccess > xTdMgr;
    com::sun::star::uno::Reference< com::sun::star::script::XInvocationAdapterFactory2 > xAdapterFactory;
    com::sun::star::uno::Reference< com::sun::star::beans::XIntrospection > xIntrospection;
    PyRef dictUnoModule;
    bool valid;
    ExceptionClassMap exceptionMap;
    ClassSet interfaceSet;
    PyRef2Adapter mappedObjects;

    PyRef getUnoModule();
};

struct stRuntimeImpl
{
    PyObject_HEAD
    struct RuntimeCargo *cargo;
public:
    static void del( PyObject *self );

    static PyRef create(
        const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > & xContext )
        throw ( com::sun::star::uno::RuntimeException );
};


class Adapter : public cppu::WeakImplHelper2<
    com::sun::star::script::XInvocation, com::sun::star::lang::XUnoTunnel >
{
    PyRef mWrappedObject;
    PyInterpreterState *mInterpreter;  // interpreters don't seem to be refcounted !
    com::sun::star::uno::WeakReference< com::sun::star::uno::XInterface > mWeakUnoAdapter;
    com::sun::star::uno::Reference< com::sun::star::beans::XIntrospectionAccess > mIntrospectionAccess;
public:
    Adapter( const PyRef &obj, const Runtime & );

    // should be called directly after construction of the adapter
    // implementation stores a weak reference to it until it has
    // created a reflection. (Ensure, that there is already a refcount
    // on the Adapter object !)
    void setUnoAdapter( const com::sun::star::uno::Reference< com::sun::star::uno::XInterface > &itr);

    static com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();
    PyRef getWrappedObject() { return mWrappedObject; }
    virtual ~Adapter();

    // XInvocation
    virtual com::sun::star::uno::Reference< ::com::sun::star::beans::XIntrospectionAccess >
           SAL_CALL getIntrospection(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL invoke(
        const ::rtl::OUString& aFunctionName,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aParams,
        ::com::sun::star::uno::Sequence< sal_Int16 >& aOutParamIndex,
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aOutParam )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::script::CannotConvertException,
               ::com::sun::star::reflection::InvocationTargetException,
               ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setValue(
        const ::rtl::OUString& aPropertyName,
        const ::com::sun::star::uno::Any& aValue )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::script::CannotConvertException,
               ::com::sun::star::reflection::InvocationTargetException,
               ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any SAL_CALL getValue( const ::rtl::OUString& aPropertyName )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasMethod( const ::rtl::OUString& aName )
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasProperty( const ::rtl::OUString& aName )
        throw (::com::sun::star::uno::RuntimeException);

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething(
        const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier )
        throw (::com::sun::star::uno::RuntimeException);
};


/** releases a refcount on the interpreter object and on another given python object.

   The function can be called from any thread regardless of whether the global
   interpreter lock is held.

 */
void decreaseRefCount( PyInterpreterState *interpreter, PyObject *object );

#ifdef PYUNO_DEBUG
#define PYUNO_DEBUG_1( msg ) fprintf( stdout, msg )
#define PYUNO_DEBUG_2( msg, arg1 ) fprintf( stdout, msg ,  arg1 )
#define PYUNO_DEBUG_3( msg, arg1 , arg2 ) fprintf( stdout, msg, arg1, arg2 )
#else
#define PYUNO_DEBUG_1(msg) ((void)0)
#define PYUNO_DEBUG_2(msg,arg1) ((void)0)
#define PYUNO_DEBUG_3(msg,arg2,arg3) ((void)0)
#endif

}

#endif
