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
#include "pyuno_impl.hxx"

#include <cassert>
#include <vector>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <typelib/typedescription.hxx>
#include <com/sun/star/reflection/XServiceConstructorDescription.hpp>
#include <com/sun/star/reflection/XServiceTypeDescription2.hpp>
#include <com/sun/star/script/CannotConvertException.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace pyuno
{
namespace
{
struct PyUNO_service_constructor_Internals
{
    css::uno::Reference<css::reflection::XServiceTypeDescription2> xService;
    css::uno::Reference<css::reflection::XServiceConstructorDescription> xConstructor;
    // The parameter types are lazily converted to TypeDescriptions when the constructor is first
    // called.
    bool bTypesInitialized = false;
    std::vector<css::uno::TypeDescription> xParamTypes;
    // Whether the last parameter is a rest parameter
    bool bHasRest = false;

    void ensureParamTypes();
};

void PyUNO_service_constructor_Internals::ensureParamTypes()
{
    if (bTypesInitialized)
        return;

    css::uno::Sequence<css::uno::Reference<css::reflection::XParameter>> aParams
        = xConstructor->getParameters();

    for (const auto& xParameter : aParams)
        xParamTypes.emplace_back(xParameter->getType()->getName());

    bHasRest = aParams.getLength() > 0 && aParams[aParams.getLength() - 1]->isRestParameter();

    bTypesInitialized = true;
}

struct PyUNO_service_constructor
{
    PyObject_HEAD;

    PyUNO_service_constructor_Internals members;
};

void PyUNO_service_constructor_dealloc(PyObject* self)
{
    PyUNO_service_constructor* me = reinterpret_cast<PyUNO_service_constructor*>(self);

    // members was allocated with the placement new operator so we need to explicitly call the
    // destructor
    me->members.~PyUNO_service_constructor_Internals();

    Py_TYPE(self)->tp_free(self);
}

PyObject* PyUNO_service_constructor_call(PyObject* self, PyObject* args,
                                         SAL_UNUSED_PARAMETER PyObject*)
{
    PyUNO_service_constructor* me = reinterpret_cast<PyUNO_service_constructor*>(self);

    me->members.ensureParamTypes();

    Py_ssize_t nParams = PyTuple_Size(args);

    // context, fixed params
    sal_Int32 nMinParams = 1 + me->members.xParamTypes.size();
    sal_Int32 nMaxParams;

    if (me->members.bHasRest)
    {
        // The last parameter can be empty
        nMinParams--;
        nMaxParams = SAL_MAX_INT32;
    }
    else
        nMaxParams = nMinParams;

    if (nParams < nMinParams || nParams > nMaxParams)
    {
        OUString sConstructorName = me->members.xConstructor->getName();

        if (sConstructorName.isEmpty() && me->members.xConstructor->isDefaultConstructor())
            sConstructorName = "create";

        OUStringBuffer buf(me->members.xService->getName() + "::" + sConstructorName
                           + " requires ");

        if (me->members.bHasRest)
            buf.append("at least ");

        buf.append(OUString::number(nMinParams) + " argument");

        if (nMinParams > 1)
            buf.append('s');

        PyErr_SetString(
            PyExc_AttributeError,
            OUStringToOString(buf.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US).getStr());

        return nullptr;
    }

    try
    {
        Runtime runtime;
        css::uno::Any contextAny = runtime.pyObject2Any(PyTuple_GetItem(args, 0));
        css::uno::Reference<css::uno::XComponentContext> xContext;

        if (!(contextAny >>= xContext) || !xContext.is())
        {
            PyErr_SetString(PyExc_AttributeError,
                            "First argument to a service constructor must be an XComponentContext");
            return nullptr;
        }

        assert(nParams > 0
               && "nParams >= 1 after bounds check against nMinParams which is always >= 1");
        css::uno::Sequence<css::uno::Any> aParams(nParams - 1);
        css::uno::Any* pParams = aParams.getArray();

        for (sal_Int32 i = 0; i < nParams - 1; ++i)
        {
            css::uno::Any param = runtime.pyObject2Any(PyTuple_GetItem(args, i + 1));

            // Use the type of this parameter or the last one if we’re building the rest parameters
            std::size_t nConstructorParam
                = std::min(std::size_t(i), me->members.xParamTypes.size() - 1);
            typelib_TypeDescription* pDestType = me->members.xParamTypes[nConstructorParam].get();

            // Try to coax the any to the right type. This is needed for example to allow passing
            // None to as an interface reference
            pParams[i]
                = runtime.getImpl()->cargo->xTypeConverter->convertTo(param, pDestType->pWeakRef);
        }

        css::uno::Reference<css::uno::XInterface> xInterface
            = xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                me->members.xService->getName(), aParams, xContext);
        return runtime.any2PyObject(css::uno::Any(std::move(xInterface))).getAcquired();
    }
    catch (const css::reflection::InvocationTargetException& e)
    {
        raisePyExceptionWithAny(e.TargetException);
    }
    catch (const css::script::CannotConvertException& e)
    {
        raisePyExceptionWithAny(css::uno::Any(e));
    }
    catch (const css::lang::IllegalArgumentException& e)
    {
        raisePyExceptionWithAny(css::uno::Any(e));
    }
    catch (const css::uno::RuntimeException& e)
    {
        raisePyExceptionWithAny(css::uno::Any(e));
    }

    return nullptr;
}

PyTypeObject PyUNO_service_constructor_Type = {
    PyVarObject_HEAD_INIT(nullptr, 0) "PyUNO_service_constructor",
    sizeof(PyUNO_service_constructor),
    0,
    PyUNO_service_constructor_dealloc,
#if PY_VERSION_HEX >= 0x03080000
    0, // Py_ssize_t tp_vectorcall_offset
#else
    nullptr, // printfunc tp_print
#endif
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    PyUNO_service_constructor_call,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    0,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    0,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    0,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    0
#if PY_VERSION_HEX >= 0x03040000
    ,
    nullptr
#if PY_VERSION_HEX >= 0x03080000
    ,
    nullptr // vectorcallfunc tp_vectorcall
#if PY_VERSION_HEX < 0x03090000
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
    ,
    nullptr // tp_print
#if defined __clang__
#pragma clang diagnostic pop
#endif
#endif
#if PY_VERSION_HEX >= 0x030C00A1
    ,
    0 // tp_watched
#endif
#if PY_VERSION_HEX >= 0x030D00A4
    ,
    0 // tp_versions_used
#endif
#endif
#endif
};
}

PyRef PyUNO_service_constructor_new(
    const css::uno::Reference<css::reflection::XServiceTypeDescription2>& xService,
    const css::uno::Reference<css::reflection::XServiceConstructorDescription>& xConstructor)
{
    OSL_ENSURE(xService.is(), "xService must be valid");
    OSL_ENSURE(xConstructor.is(), "xConstructor must be valid");

    PyUNO_service_constructor* self
        = PyObject_New(PyUNO_service_constructor, &PyUNO_service_constructor_Type);
    if (self == nullptr)
        return nullptr; // NULL == Error!

    // The memory for members was allocated above but we still need to call the constructor so we’ll
    // use a placement new.
    new (&self->members) PyUNO_service_constructor_Internals;

    self->members.xService = xService;
    self->members.xConstructor = xConstructor;

    return PyRef(reinterpret_cast<PyObject*>(self), SAL_NO_ACQUIRE);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
