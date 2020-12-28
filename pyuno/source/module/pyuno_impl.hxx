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
#ifndef INCLUDED_PYUNO_SOURCE_MODULE_PYUNO_IMPL_HXX
#define INCLUDED_PYUNO_SOURCE_MODULE_PYUNO_IMPL_HXX

#if defined(_MSC_VER)
// Workaround for some horrible hypot() mess
#include <math.h>
#endif

#include <Python.h>

//Python 3.0 and newer don't have these flags
#ifndef Py_TPFLAGS_HAVE_ITER
#  define Py_TPFLAGS_HAVE_ITER 0
#endif
#ifndef Py_TPFLAGS_HAVE_RICHCOMPARE
#  define Py_TPFLAGS_HAVE_RICHCOMPARE 0
#endif
#ifndef Py_TPFLAGS_HAVE_SEQUENCE_IN
#  define Py_TPFLAGS_HAVE_SEQUENCE_IN 0
#endif

#include <pyuno.hxx>

#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/script/XInvocation.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weakref.hxx>

#include <osl/module.hxx>

namespace com::sun::star::beans { class XIntrospection; }
namespace com::sun::star::container { class XEnumeration; }
namespace com::sun::star::container { class XHierarchicalNameAccess; }
namespace com::sun::star::lang { class XSingleServiceFactory; }
namespace com::sun::star::reflection { class XIdlReflection; }
namespace com::sun::star::script { class XInvocation2; }
namespace com::sun::star::script { class XInvocationAdapterFactory2; }
namespace com::sun::star::script { class XTypeConverter; }

namespace pyuno
{


// Logging API - implementation can be found in pyuno_util

struct RuntimeCargo;
namespace LogLevel
{
// when you add a loglevel, extend the log function !
const sal_Int32 NONE = 0;
const sal_Int32 CALL = 1;
const sal_Int32 ARGS = 2;
}

bool isLog( RuntimeCargo const *cargo, sal_Int32 loglevel );
void log( RuntimeCargo *cargo, sal_Int32 level, std::u16string_view logString );
void log( RuntimeCargo *cargo, sal_Int32 level, const char *str );
void logCall( RuntimeCargo *cargo, const char *intro,
              void * ptr, std::u16string_view aFunctionName,
              const css::uno::Sequence< css::uno::Any > & args );
void logReply( RuntimeCargo *cargo, const char *intro,
              void * ptr, std::u16string_view aFunctionName,
              const css::uno::Any &returnValue,
              const css::uno::Sequence< css::uno::Any > & args );
void logException( RuntimeCargo *cargo, const char *intro,
                   void * ptr, std::u16string_view aFunctionName,
                   const void * data, const css::uno::Type & type );
const sal_Int32 VAL2STR_MODE_DEEP = 0;
const sal_Int32 VAL2STR_MODE_SHALLOW = 1;
OUString val2str( const void * pVal, typelib_TypeDescriptionReference * pTypeRef, sal_Int32 mode = VAL2STR_MODE_DEEP );


typedef std::unordered_map
<
    PyRef,
    css::uno::WeakReference< css::script::XInvocation >,
    PyRef::Hash
> PyRef2Adapter;


typedef std::unordered_map
<
    OUString,
    PyRef
> ExceptionClassMap;

typedef std::unordered_map
<
    OUString,
    css::uno::Sequence< sal_Int16 >
> MethodOutIndexMap;

typedef std::unordered_set< PyRef , PyRef::Hash > ClassSet;

int PyUNO_initType();
int PyUNOStruct_initType();

PyRef PyUNO_new (
    const css::uno::Any & targetInterface,
    const css::uno::Reference<css::lang::XSingleServiceFactory> & ssf );

PyRef PyUNOStruct_new (
    const css::uno::Any &targetInterface,
    const  css::uno::Reference<css::lang::XSingleServiceFactory> &ssf );

struct PyUNOInternals
{
    css::uno::Reference <css::script::XInvocation2> xInvocation;
    css::uno::Any wrappedObject;
};

typedef struct
{
    PyObject_HEAD
    PyUNOInternals* members;
} PyUNO;

PyObject* PyUNO_iterator_new (
    const css::uno::Reference<css::container::XEnumeration>& xEnumeration);

struct PyUNO_iterator_Internals
{
    css::uno::Reference <css::container::XEnumeration> xEnumeration;
};

typedef struct
{
        PyObject_HEAD
        PyUNO_iterator_Internals* members;
} PyUNO_iterator;

PyObject* PyUNO_list_iterator_new (
    const css::uno::Reference<css::container::XIndexAccess> &xIndexAccess);

struct PyUNO_list_iterator_Internals
{
    css::uno::Reference <css::container::XIndexAccess> xIndexAccess;
    int index;
};

typedef struct
{
        PyObject_HEAD
        PyUNO_list_iterator_Internals* members;
} PyUNO_list_iterator;

PyRef ustring2PyUnicode( const OUString &source );
PyRef ustring2PyString( std::u16string_view source );
OUString pyString2ustring( PyObject *str );

/// @throws css::reflection::InvocationTargetException
/// @throws css::uno::RuntimeException
void raiseInvocationTargetExceptionWhenNeeded( const Runtime &runtime );

PyRef PyUNO_callable_new (
    const css::uno::Reference<css::script::XInvocation2> &xInv,
    const OUString &methodName,
    ConversionMode mode = REJECT_UNO_ANY );

PyObject* PyUNO_Type_new (const char *typeName , css::uno::TypeClass t , const Runtime &r );
PyObject* PyUNO_Enum_new( const char *enumBase, const char *enumValue, const Runtime &r );
PyObject* PyUNO_char_new (sal_Unicode c , const Runtime &r);
PyObject *PyUNO_ByteSequence_new( const css::uno::Sequence< sal_Int8 > &, const Runtime &r );

PyRef getTypeClass( const Runtime &);
PyRef getEnumClass( const Runtime &);
PyRef getCharClass( const Runtime &);
PyRef getByteSequenceClass( const Runtime & );
PyRef getPyUnoClass();
PyRef getPyUnoStructClass();
PyRef getClass( const OUString & name , const Runtime & runtime );
PyRef getAnyClass( const Runtime &);
PyObject *PyUNO_invoke( PyObject *object, const char *name , PyObject *args );

/// @throws css::uno::RuntimeException
css::uno::Any PyEnum2Enum( PyObject *obj );
/// @throws css::uno::RuntimeException
sal_Unicode PyChar2Unicode( PyObject *o );
/// @throws css::uno::RuntimeException
css::uno::Type PyType2Type( PyObject * o );

void raisePyExceptionWithAny( const css::uno::Any &a );
const char *typeClassToString( css::uno::TypeClass t );

/// @throws css::uno::RuntimeException
PyRef getObjectFromUnoModule( const Runtime &runtime, const char * object );

bool isInterfaceClass( const Runtime &, PyObject *obj );
bool isInstanceOfStructOrException( PyObject *obj);

struct RuntimeCargo
{
    css::uno::Reference< css::lang::XSingleServiceFactory > xInvocation;
    css::uno::Reference< css::script::XTypeConverter> xTypeConverter;
    css::uno::Reference< css::uno::XComponentContext > xContext;
    css::uno::Reference< css::reflection::XIdlReflection > xCoreReflection;
    css::uno::Reference< css::container::XHierarchicalNameAccess > xTdMgr;
    css::uno::Reference< css::script::XInvocationAdapterFactory2 > xAdapterFactory;
    css::uno::Reference< css::beans::XIntrospection > xIntrospection;
    PyRef dictUnoModule;
    osl::Module testModule;
    bool valid;
    ExceptionClassMap exceptionMap;
    ClassSet interfaceSet;
    PyRef2Adapter mappedObjects;
    FILE *logFile;
    sal_Int32 logLevel;

    PyRef const & getUnoModule();
};

struct stRuntimeImpl
{
    PyObject_HEAD
    struct RuntimeCargo *cargo;
public:
    static void del( PyObject *self );

    /// @throws css::uno::RuntimeException
    static PyRef create(
        const css::uno::Reference< css::uno::XComponentContext > & xContext );
};


class Adapter : public cppu::WeakImplHelper<
    css::script::XInvocation, css::lang::XUnoTunnel >
{
    PyRef mWrappedObject;
    PyInterpreterState *mInterpreter;  // interpreters don't seem to be refcounted !
    css::uno::Sequence< css::uno::Type > mTypes;
    MethodOutIndexMap m_methodOutIndexMap;

private:
    css::uno::Sequence< sal_Int16 > getOutIndexes( const OUString & functionName );

public:
public:
    Adapter( const PyRef &obj,
             const css::uno::Sequence< css::uno::Type > & types );

    static css::uno::Sequence< sal_Int8 > getUnoTunnelId();
    const PyRef& getWrappedObject() const { return mWrappedObject; }
    const css::uno::Sequence< css::uno::Type >& getWrappedTypes() const { return mTypes; }
    virtual ~Adapter() override;

    // XInvocation
    virtual css::uno::Reference< css::beans::XIntrospectionAccess >
           SAL_CALL getIntrospection(  ) override;
    virtual css::uno::Any SAL_CALL invoke(
        const OUString& aFunctionName,
        const css::uno::Sequence< css::uno::Any >& aParams,
        css::uno::Sequence< sal_Int16 >& aOutParamIndex,
        css::uno::Sequence< css::uno::Any >& aOutParam ) override;

    virtual void SAL_CALL setValue(
        const OUString& aPropertyName,
        const css::uno::Any& aValue ) override;

    virtual css::uno::Any SAL_CALL getValue( const OUString& aPropertyName ) override;
    virtual sal_Bool SAL_CALL hasMethod( const OUString& aName ) override;
    virtual sal_Bool SAL_CALL hasProperty( const OUString& aName ) override;

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething(
        const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;
};


/** releases a refcount on the interpreter object and on another given python object.

   The function can be called from any thread regardless of whether the global
   interpreter lock is held.

 */
void decreaseRefCount( PyInterpreterState *interpreter, PyObject *object );

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
