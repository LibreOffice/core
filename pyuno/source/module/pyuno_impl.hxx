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
#ifndef _PYUNO_IMPL_
#define _PYUNO_IMPL_

#if defined(_MSC_VER) && (_MSC_VER >= 1600)
// Workaround for some horrible hypot() mess
#include <math.h>
#endif

#include <Python.h>

//Must define PyVarObject_HEAD_INIT for Python 2.5 or older
#ifndef PyVarObject_HEAD_INIT
#define PyVarObject_HEAD_INIT(type, size)  PyObject_HEAD_INIT(type) size,
#endif

#include <pyuno/pyuno.hxx>

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

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

// In Python 3, the PyString_* functions have been replaced by PyBytes_*
// and PyUnicode_* functions.
#if PY_MAJOR_VERSION >= 3

// compatibility wrappers for Python "str" type (PyUnicode in 3, PyString in 2)
inline PyObject* PyStr_FromString(const char *string)
{
    return PyUnicode_FromString(string);
}

inline char * PyStr_AsString(PyObject *object)
{
    return PyUnicode_AsUTF8(object);
}

inline int PyStr_Check(PyObject *object)
{
    return PyUnicode_Check(object);
}

// compatibility wrappers for Python non-Unicode string/buffer type
// (PyBytes in 3, PyString in 2)
inline int PyStrBytes_Check(PyObject *object)
{
    return PyBytes_Check(object);
}

inline char* PyStrBytes_AsString(PyObject *object)
{
    return PyBytes_AsString(object);
}

inline Py_ssize_t PyStrBytes_Size(PyObject *object)
{
    return PyBytes_Size(object);
}

inline PyObject* PyStrBytes_FromStringAndSize(const char *string, Py_ssize_t len)
{
    return PyBytes_FromStringAndSize(string, len);
}
#else
inline char * PyStr_AsString(PyObject *object)
{
    return PyString_AsString(object);
}

inline PyObject* PyStr_FromString(const char *string)
{
    return PyString_FromString(string);
}

inline int PyStr_Check(PyObject *object)
{
    return PyString_Check(object);
}
inline int PyStrBytes_Check(PyObject *object)
{
    return PyString_Check(object);
}

inline char* PyStrBytes_AsString(PyObject *object)
{
    return PyString_AsString(object);
}

inline Py_ssize_t PyStrBytes_Size(PyObject *object)
{
    return PyString_Size(object);
}

inline PyObject* PyStrBytes_FromStringAndSize(const char *string, Py_ssize_t len)
{
    return PyString_FromStringAndSize(string, len);
}
#endif /* PY_MAJOR_VERSION >= 3 */

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
void log( RuntimeCargo *cargo, sal_Int32 level, const OUString &logString );
void log( RuntimeCargo *cargo, sal_Int32 level, const char *str );
void logCall( RuntimeCargo *cargo, const char *intro,
              void * ptr, const OUString & aFunctionName,
              const com::sun::star::uno::Sequence< com::sun::star::uno::Any > & args );
void logReply( RuntimeCargo *cargo, const char *intro,
              void * ptr, const OUString & aFunctionName,
              const com::sun::star::uno::Any &returnValue,
              const com::sun::star::uno::Sequence< com::sun::star::uno::Any > & args );
void logException( RuntimeCargo *cargo, const char *intro,
                   void * ptr, const OUString &aFunctionName,
                   const void * data, const com::sun::star::uno::Type & type );
static const sal_Int32 VAL2STR_MODE_DEEP = 0;
static const sal_Int32 VAL2STR_MODE_SHALLOW = 1;
OUString val2str( const void * pVal, typelib_TypeDescriptionReference * pTypeRef, sal_Int32 mode = VAL2STR_MODE_DEEP ) SAL_THROW(());
//--------------------------------------------------

typedef ::boost::unordered_map
<
    PyRef,
    com::sun::star::uno::WeakReference< com::sun::star::script::XInvocation >,
    PyRef::Hash,
    std::equal_to< PyRef >
> PyRef2Adapter;


typedef ::boost::unordered_map
<
OUString,
PyRef,
OUStringHash,
std::equal_to<OUString>
> ExceptionClassMap;

typedef ::boost::unordered_map
<
    OUString,
    com::sun::star::uno::Sequence< sal_Int16 >,
    OUStringHash,
    std::equal_to< OUString >
> MethodOutIndexMap;

typedef ::boost::unordered_set< PyRef , PyRef::Hash , std::equal_to<PyRef> > ClassSet;

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

PyRef ustring2PyUnicode( const OUString &source );
PyRef ustring2PyString( const OUString & source );
OUString pyString2ustring( PyObject *str );


PyRef AnyToPyObject (const com::sun::star::uno::Any & a, const Runtime &r )
    throw ( com::sun::star::uno::RuntimeException );

com::sun::star::uno::Any PyObjectToAny (PyObject* o)
    throw ( com::sun::star::uno::RuntimeException );

void raiseInvocationTargetExceptionWhenNeeded( const Runtime &runtime )
    throw ( com::sun::star::reflection::InvocationTargetException );

com::sun::star::uno::TypeClass StringToTypeClass (char* string);

PyRef PyUNO_callable_new (
    const com::sun::star::uno::Reference<com::sun::star::script::XInvocation2> &xInv,
    const OUString &methodName,
    ConversionMode mode = REJECT_UNO_ANY );

PyObject* PyUNO_Type_new (const char *typeName , com::sun::star::uno::TypeClass t , const Runtime &r );
PyObject* PyUNO_Enum_new( const char *enumBase, const char *enumValue, const Runtime &r );
PyObject* PyUNO_char_new (sal_Unicode c , const Runtime &r);
PyObject *PyUNO_ByteSequence_new( const com::sun::star::uno::Sequence< sal_Int8 > &, const Runtime &r );

PyRef getTypeClass( const Runtime &);
PyRef getEnumClass( const Runtime &);
PyRef getBoolClass( const Runtime &);
PyRef getCharClass( const Runtime &);
PyRef getByteSequenceClass( const Runtime & );
PyRef getPyUnoClass();
PyRef getClass( const OUString & name , const Runtime & runtime );
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
    com::sun::star::uno::Sequence< sal_Int16 > getOutIndexes( const OUString & functionName );

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
        const OUString& aFunctionName,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aParams,
        ::com::sun::star::uno::Sequence< sal_Int16 >& aOutParamIndex,
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aOutParam )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::script::CannotConvertException,
               ::com::sun::star::reflection::InvocationTargetException,
               ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setValue(
        const OUString& aPropertyName,
        const ::com::sun::star::uno::Any& aValue )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::script::CannotConvertException,
               ::com::sun::star::reflection::InvocationTargetException,
               ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any SAL_CALL getValue( const OUString& aPropertyName )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasMethod( const OUString& aName )
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasProperty( const OUString& aName )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
