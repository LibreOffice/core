/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "net_bridge.hxx"

#include <bridges/net_uno/net_context.hxx>

namespace net_uno
{
void Bridge::call_net_func(IntPtr pNetI, const typelib_TypeDescription* pMethodTD,
                           typelib_TypeDescriptionReference* pReturnTDRef, int nParams,
                           typelib_MethodParameter* pParams, void** pArgs, void* pRet,
                           uno_Any** pExc)
{
    Value* pArgsRetExc = static_cast<Value*>(alloca((nParams + 2) * sizeof(Value)));

    // Convert in and inout arguments
    for (int i = 0; i < nParams; ++i)
    {
        const typelib_MethodParameter& param = pParams[i];
        if (param.bIn)
        {
            map_uno_to_net_value(pArgs[i], &pArgsRetExc[i], param.pTypeRef, false);
        }
    }

    OUString sMethodName = OUString::unacquired(&pMethodTD->pTypeName);
    Context* pCtx = static_cast<Context*>(m_net_env->pContext);
    bool error = !pCtx->dispatchCall(pNetI, sMethodName.getStr(), pArgsRetExc,
                                     &pArgsRetExc[nParams], &pArgsRetExc[nParams + 1]);

    // Convert out and inout arguments
    for (int i = 0; i < nParams; ++i)
    {
        const typelib_MethodParameter& param = pParams[i];
        try
        {
            if (param.bOut)
                map_net_value_to_uno(pArgs[i], &pArgsRetExc[i], param.pTypeRef,
                                     param.bIn && param.bOut, true);
        }
        catch (...)
        {
            // Clean up uno out args
            for (int n = 0; n < i; ++n)
            {
                const typelib_MethodParameter& param2 = pParams[n];
                if (param2.bOut)
                {
                    uno_type_destructData(pArgs[n], param2.pTypeRef, nullptr);
                }
            }
            throw;
        }
    }

    if (error)
    {
        map_net_value_to_uno(*pExc, &pArgsRetExc[nParams + 1],
                             cppu::UnoType<css::uno::Any>::get().getTypeLibType(), false, true);
    }
    else
    {
        if (pReturnTDRef && pReturnTDRef->eTypeClass != typelib_TypeClass_VOID)
        {
            map_net_value_to_uno(pRet, &pArgsRetExc[nParams], pReturnTDRef, false, true);
        }

        *pExc = nullptr;
    }
}

bool Bridge::call_uno_func(uno_Interface* pUnoI, const typelib_TypeDescription* pMethodTD,
                           typelib_TypeDescriptionReference* pReturnTDRef, int nParams,
                           typelib_MethodParameter* pParams, Value* pArgs, Value* pRet, Value* pExc)
{
    union largest {
        sal_Int64 n;
        double d;
        void* p;
        uno_Any a;
    };

    // Calculate size of memory required for return value
    sal_Int32 nReturnSize = sizeof(largest);
    if (pReturnTDRef)
    {
        if (pReturnTDRef->eTypeClass == typelib_TypeClass_VOID)
        {
            nReturnSize = 0;
        }
        else if (pReturnTDRef->eTypeClass == typelib_TypeClass_STRUCT
                 || pReturnTDRef->eTypeClass == typelib_TypeClass_EXCEPTION)
        {
            TypeDescHolder returnTD(pReturnTDRef);
            if (o3tl::make_unsigned(returnTD.get()->nSize) > sizeof(largest))
                nReturnSize = returnTD.get()->nSize;
        }
    }

    // Prepare a memory block to contain all the converted arguments and return value
    //
    // The memory block contains pointers to small arguments stored in the same block.
    // If an argument is larger then `largest` union, such as a struct, then the pointer
    // points to an extra block of memory.
    //
    // The argument pointers are followed by the return value. If the return value is
    // larger than the `largest` union, this is a pointer to an extra block containing
    // the return value instead.
    //
    // For example: 2 arguments and return value
    // | Pointer 1    (void*)
    // | Pointer 2    (void*)
    // | Return Value (void*)
    // | Argument 1   (largest*)
    // | Argument 2   (largest*)

    // Complete memory block
    char* mem = static_cast<char*>(
        alloca(nParams * sizeof(void*) + nReturnSize + nParams * sizeof(largest)));
    // Array of argument pointers; at the start of the memory block
    void** uno_args = reinterpret_cast<void**>(mem);
    // Pointer to return value memory; after all argument pointers
    void* uno_ret = nReturnSize == 0 ? nullptr : mem + nParams * sizeof(void*);
    // Space for actual arguments; after return value
    largest* uno_args_mem = reinterpret_cast<largest*>(mem + nParams * sizeof(void*) + nReturnSize);

    for (sal_Int32 i = 0; i < nParams; ++i)
    {
        const typelib_MethodParameter& param = pParams[i];
        typelib_TypeDescriptionReference* type = param.pTypeRef;

        uno_args[i] = &uno_args_mem[i];
        if (type->eTypeClass == typelib_TypeClass_STRUCT
            || type->eTypeClass == typelib_TypeClass_EXCEPTION)
        {
            TypeDescHolder td(type);
            if (o3tl::make_unsigned(td.get()->nSize) > sizeof(largest))
                uno_args[i] = alloca(td.get()->nSize);
        }

        if (param.bIn)
        {
            try
            {
                // in, in/out params
                map_net_value_to_uno(uno_args[i], &pArgs[i], type, false, true);
            }
            catch (...)
            {
                // cleanup uno in args
                for (sal_Int32 n = 0; n < i; ++n)
                {
                    const typelib_MethodParameter& param2 = pParams[n];
                    if (param2.bIn)
                    {
                        uno_type_destructData(uno_args[n], param2.pTypeRef, nullptr);
                    }
                }
                throw;
            }
        }
    }

    uno_Any uno_exc_holder;
    uno_Any* uno_exc = &uno_exc_holder;

    // Propagate function call to binary uno
    (*pUnoI->pDispatcher)(pUnoI, pMethodTD, uno_ret, uno_args, &uno_exc);

    if (!uno_exc)
    {
        // Convert out arguments and destruct previously converted uno args
        for (sal_Int32 i = 0; i < nParams; ++i)
        {
            const typelib_MethodParameter& param = pParams[i];
            typelib_TypeDescriptionReference* type = param.pTypeRef;

            if (param.bOut)
            {
                try
                {
                    map_uno_to_net_value(uno_args[i], &pArgs[i], param.pTypeRef, false);
                }
                catch (...)
                {
                    // Cleanup rest of uno args
                    for (sal_Int32 n = i; n < nParams; ++n)
                    {
                        uno_type_destructData(uno_args[n], pParams[n].pTypeRef, nullptr);
                    }

                    // Cleanup uno return value
                    uno_type_destructData(uno_ret, pReturnTDRef, nullptr);

                    throw;
                }
            }

            // Cleanup args
            if (type->eTypeClass > typelib_TypeClass_DOUBLE
                && type->eTypeClass != typelib_TypeClass_ENUM)
            {
                uno_type_destructData(uno_args[i], type, nullptr);
            }
        }

        if (pReturnTDRef && pReturnTDRef->eTypeClass != typelib_TypeClass_VOID)
        {
            // Convert uno return value
            try
            {
                map_uno_to_net_value(uno_ret, pRet, pReturnTDRef, false);
                uno_type_destructData(uno_ret, pReturnTDRef, nullptr);
            }
            catch (...)
            {
                uno_type_destructData(uno_ret, pReturnTDRef, nullptr);
                throw;
            }
        }

        return true;
    }
    else // An exception occurred
    {
        // Destruct uno in arguments
        for (sal_Int32 i = 0; i < nParams; ++i)
        {
            const typelib_MethodParameter& param = pParams[i];
            if (param.bIn)
            {
                uno_type_destructData(uno_args[i], param.pTypeRef, nullptr);
            }
        }

        map_uno_to_net_value(uno_exc, pExc, cppu::UnoType<css::uno::Any>::get().getTypeLibType(),
                             false);

        return false;
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
