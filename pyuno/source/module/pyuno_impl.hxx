/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: pyuno_impl.hxx,v $
 * $Revision: 1.7 $
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
#ifndef _PYUNO_IMPL_
#define _PYUNO_IMPL_

#include <pyuno/pyuno.hxx>

#include <hash_map>
#include <hash_set>

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

//--------------------------------------------------
// Logging API - implementation can be found in pyuno_util
//--------------------------------------------------
struct RuntimeCargo;
namespace LogLevel
{
// when you add a loglevel, extend the log function !
static const sal_Int32 NONE = 0;
static const sal_Int32 CALL = 1;
static const sal_Int32 ARGS = 2;
}

bool isLog( RuntimeCargo *cargo, sal_Int32 loglevel );
void log( RuntimeCargo *cargo, sal_Int32 level, const rtl::OUString &logString );
void log( RuntimeCargo *cargo, sal_Int32 level, const char *str );
void logCall( RuntimeCargo *cargo, const char *intro,
              void * ptr, const rtl::OUString & aFunctionName,
              const com::sun::star::uno::Sequence< com::sun::star::uno::Any > & args );
void logReply( RuntimeCargo *cargo, const char *intro,
              void * ptr, const rtl::OUString & aFunctionName,
              const com::sun::star::uno::Any &returnValue,
              const com::sun::star::uno::Sequence< com::sun::star::uno::Any > & args );
void logException( RuntimeCargo *cargo, const char *intro,
                   void * ptr, const rtl::OUString &aFunctionName,
                   const void * data, const com::sun::star::uno::Type & type );
static const sal_Int32 VAL2STR_MODE_DEEP = 0;
static const sal_Int32 VAL2STR_MODE_SHALLOW = 1;
rtl::OUString val2str( const void * pVal, typelib_TypeDescriptionReference * pTypeRef, sal_Int32 mode = VAL2STR_MODE_DEEP ) SAL_THROW( () );
//--------------------------------------------------

typedef ::std::hash_map
<
    PyRef,
    com::sun::star::uno::WeakReference< com::sun::star::script::XInvocation >,
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

typedef ::std::hash_map
<
    rtl::OUString,
    com::sun::star::uno::Sequence< sal_Int16 >,
    rtl::OUStringHash,
    std::equal_to< rtl::OUString >
> MethodOutIndexMap;

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
    const com::sun::star::uno::Reference<com::sun::star::script::XTypeConverter> &tc,
    ConversionMode mode = REJECT_UNO_ANY );

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
PyRef getAnyClass( const Runtime &);
PyObject *PyUNO_invoke( PyObject *object, const char *name , PyObject *args );

com::sun::star::uno::Any PyEnum2Enum( PyObject *obj )
    throw ( com::sun::star::uno::RuntimeException );
sal_Bool PyBool2Bool( PyObject *o, const Runtime & r )
    throw ( com::sun::star::uno::RuntimeException );
sal_Unicode PyChar2Unicode( PyObject *o )
    throw ( com::sun::star::uno::RuntimeException );
com::sun::star::uno::Type PyType2Type( PyObject * o )
    throw( com::sun::star::uno::RuntimeException );

void raisePyExceptionWithAny( const com::sun::star::uno::Any &a );
const char *typeClassToString( com::sun::star::uno::TypeClass t );

PyRef getObjectFromUnoModule( const Runtime &runtime, const char * object )
    throw ( com::sun::star::uno::RuntimeException );

sal_Bool isInterfaceClass( const Runtime &, PyObject *obj );
bool isInstanceOfStructOrException( PyObject *obj);
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
    FILE *logFile;
    sal_Int32 logLevel;

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
    com::sun::star::uno::Sequence< com::sun::star::uno::Type > mTypes;
    MethodOutIndexMap m_methodOutIndexMap;

private:
    com::sun::star::uno::Sequence< sal_Int16 > getOutIndexes( const rtl::OUString & functionName );

public:
public:
    Adapter( const PyRef &obj,
             const com::sun::star::uno::Sequence< com::sun::star::uno::Type > & types );

    static com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();
    PyRef getWrappedObject() { return mWrappedObject; }
    com::sun::star::uno::Sequence< com::sun::star::uno::Type > getWrappedTypes() { return mTypes; }
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

}

#endif
