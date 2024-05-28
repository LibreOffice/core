/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <exception>
#include <typeinfo>
#include <vector>

#include <alloca.h>

#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/genfunc.hxx>
#include <o3tl/runtimetooustring.hxx>
#include <o3tl/unreachable.hxx>
#include <rtl/strbuf.hxx>
#include <typelib/typeclass.h>
#include <typelib/typedescription.h>
#include <typelib/typedescription.hxx>
#include <uno/any2.h>
#include <uno/data.h>

#include <bridge.hxx>
#include <types.hxx>
#include <unointerfaceproxy.hxx>
#include <vtables.hxx>
#include <wasm/callvirtualfunction.hxx>

#include "abi.hxx"

using namespace ::com::sun::star::uno;

namespace
{
enum class StructKind
{
    Empty,
    I32,
    I64,
    F32,
    F64,
    General
};

StructKind getKind(typelib_CompoundTypeDescription const* type)
{
    if (type->nMembers > 1)
    {
        return StructKind::General;
    }
    auto k = StructKind::Empty;
    if (type->pBaseTypeDescription != nullptr)
    {
        k = getKind(type->pBaseTypeDescription);
    }
    if (type->nMembers == 0)
    {
        return k;
    }
    if (k != StructKind::Empty)
    {
        return StructKind::General;
    }
    switch (type->ppTypeRefs[0]->eTypeClass)
    {
        case typelib_TypeClass_BOOLEAN:
        case typelib_TypeClass_BYTE:
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
        case typelib_TypeClass_CHAR:
        case typelib_TypeClass_ENUM:
            return StructKind::I32;
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
            return StructKind::I64;
        case typelib_TypeClass_FLOAT:
            return StructKind::F32;
        case typelib_TypeClass_DOUBLE:
            return StructKind::F64;
        default:
            return StructKind::General;
    }
}

void call(bridges::cpp_uno::shared::UnoInterfaceProxy* proxy,
          bridges::cpp_uno::shared::VtableSlot slot, typelib_TypeDescriptionReference* returnType,
          sal_Int32 count, typelib_MethodParameter* parameters, void* returnValue, void** arguments,
          uno_Any** exception)
{
    css::uno::TypeDescription rtd(returnType);
    auto const retConv = bridges::cpp_uno::shared::relatesToInterfaceType(rtd.get());
    auto const ret = retConv ? alloca(rtd.get()->nSize) : returnValue;
    OStringBuffer sig;
    std::vector<sal_uInt64> args;
    switch (rtd.get()->eTypeClass)
    {
        case typelib_TypeClass_VOID:
            sig.append('v');
            break;
        case typelib_TypeClass_BOOLEAN:
        case typelib_TypeClass_BYTE:
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
        case typelib_TypeClass_CHAR:
        case typelib_TypeClass_ENUM:
            sig.append('i');
            break;
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
            sig.append('j');
            break;
        case typelib_TypeClass_FLOAT:
            sig.append('f');
            break;
        case typelib_TypeClass_DOUBLE:
            sig.append('d');
            break;
        case typelib_TypeClass_STRING:
        case typelib_TypeClass_TYPE:
        case typelib_TypeClass_ANY:
        case typelib_TypeClass_SEQUENCE:
        case typelib_TypeClass_INTERFACE:
            sig.append("vi");
            args.push_back(reinterpret_cast<sal_uInt32>(ret));
            break;
        case typelib_TypeClass_STRUCT:
        {
            switch (getKind(reinterpret_cast<typelib_CompoundTypeDescription const*>(rtd.get())))
            {
                case StructKind::Empty:
                    break;
                case StructKind::I32:
                    sig.append('i');
                    break;
                case StructKind::I64:
                    sig.append('j');
                    break;
                case StructKind::F32:
                    sig.append('f');
                    break;
                case StructKind::F64:
                    sig.append('d');
                    break;
                case StructKind::General:
                    sig.append("vi");
                    args.push_back(reinterpret_cast<sal_uInt32>(ret));
                    break;
            }
            break;
        }
        default:
            O3TL_UNREACHABLE;
    }
    sig.append('i');
    sal_uInt32 const* const* thisPtr
        = reinterpret_cast<sal_uInt32 const* const*>(proxy->getCppI()) + slot.offset;
    args.push_back(reinterpret_cast<sal_uInt32>(thisPtr));
    std::vector<void*> cppArgs(count);
    std::vector<css::uno::TypeDescription> ptds(count);
    for (sal_Int32 i = 0; i != count; ++i)
    {
        if (!parameters[i].bOut && bridges::cpp_uno::shared::isSimpleType(parameters[i].pTypeRef))
        {
            switch (parameters[i].pTypeRef->eTypeClass)
            {
                case typelib_TypeClass_BOOLEAN:
                    sig.append('i');
                    args.push_back(*reinterpret_cast<sal_Bool const*>(arguments[i]));
                    break;
                case typelib_TypeClass_BYTE:
                    sig.append('i');
                    args.push_back(*reinterpret_cast<sal_Int8 const*>(arguments[i]));
                    break;
                case typelib_TypeClass_SHORT:
                    sig.append('i');
                    args.push_back(*reinterpret_cast<sal_Int16 const*>(arguments[i]));
                    break;
                case typelib_TypeClass_UNSIGNED_SHORT:
                    sig.append('i');
                    args.push_back(*reinterpret_cast<sal_uInt16 const*>(arguments[i]));
                    break;
                case typelib_TypeClass_LONG:
                case typelib_TypeClass_ENUM:
                    sig.append('i');
                    args.push_back(*reinterpret_cast<sal_Int32 const*>(arguments[i]));
                    break;
                case typelib_TypeClass_UNSIGNED_LONG:
                    sig.append('i');
                    args.push_back(*reinterpret_cast<sal_uInt32 const*>(arguments[i]));
                    break;
                case typelib_TypeClass_HYPER:
                    sig.append('j');
                    args.push_back(*reinterpret_cast<sal_Int64 const*>(arguments[i]));
                    break;
                case typelib_TypeClass_UNSIGNED_HYPER:
                    sig.append('j');
                    args.push_back(*reinterpret_cast<sal_uInt64 const*>(arguments[i]));
                    break;
                case typelib_TypeClass_FLOAT:
                    sig.append('f');
                    args.push_back(*reinterpret_cast<sal_uInt32 const*>(arguments[i]));
                    break;
                case typelib_TypeClass_DOUBLE:
                    sig.append('d');
                    args.push_back(*reinterpret_cast<sal_uInt64 const*>(arguments[i]));
                    break;
                case typelib_TypeClass_CHAR:
                    sig.append('i');
                    args.push_back(*reinterpret_cast<sal_Unicode const*>(arguments[i]));
                    break;
                default:
                    O3TL_UNREACHABLE;
            }
        }
        else
        {
            sig.append('i');
            css::uno::TypeDescription ptd(parameters[i].pTypeRef);
            if (!parameters[i].bIn)
            {
                cppArgs[i] = alloca(ptd.get()->nSize);
                uno_constructData(cppArgs[i], ptd.get());
                ptds[i] = ptd;
                args.push_back(reinterpret_cast<sal_uInt32>(cppArgs[i]));
            }
            else if (bridges::cpp_uno::shared::relatesToInterfaceType(ptd.get()))
            {
                cppArgs[i] = alloca(ptd.get()->nSize);
                uno_copyAndConvertData(cppArgs[i], arguments[i], ptd.get(),
                                       proxy->getBridge()->getUno2Cpp());
                ptds[i] = ptd;
                args.push_back(reinterpret_cast<sal_uInt32>(cppArgs[i]));
            }
            else
            {
                args.push_back(reinterpret_cast<sal_uInt32>(arguments[i]));
            }
        }
    }
    try
    {
        try
        {
            callVirtualFunction(sig, (*thisPtr)[slot.index], args.data(), ret);
        }
        catch (css::uno::Exception&)
        {
            throw;
        }
        catch (std::exception& e)
        {
            throw css::uno::RuntimeException("C++ code threw "
                                             + o3tl::runtimeToOUString(typeid(e).name()) + ": "
                                             + o3tl::runtimeToOUString(e.what()));
        }
        catch (...)
        {
            throw css::uno::RuntimeException("C++ code threw unknown exception");
        }
    }
    catch (css::uno::Exception&)
    {
        __cxxabiv1::__cxa_exception* header
            = reinterpret_cast<__cxxabiv1::__cxa_eh_globals*>(__cxxabiv1::__cxa_get_globals())
                  ->caughtExceptions;
        abi_wasm::mapException(header, __cxxabiv1::__cxa_current_exception_type(), *exception,
                               proxy->getBridge()->getCpp2Uno());
        for (sal_Int32 i = 0; i != count; ++i)
        {
            if (cppArgs[i] != nullptr)
            {
                uno_destructData(cppArgs[i], ptds[i].get(),
                                 reinterpret_cast<uno_ReleaseFunc>(css::uno::cpp_release));
            }
        }
        return;
    }
    *exception = nullptr;
    for (sal_Int32 i = 0; i != count; ++i)
    {
        if (cppArgs[i] != nullptr)
        {
            if (parameters[i].bOut)
            {
                if (parameters[i].bIn)
                {
                    uno_destructData(arguments[i], ptds[i].get(), nullptr);
                }
                uno_copyAndConvertData(arguments[i], cppArgs[i], ptds[i].get(),
                                       proxy->getBridge()->getCpp2Uno());
            }
            uno_destructData(cppArgs[i], ptds[i].get(),
                             reinterpret_cast<uno_ReleaseFunc>(css::uno::cpp_release));
        }
    }
    if (retConv)
    {
        uno_copyAndConvertData(returnValue, ret, rtd.get(), proxy->getBridge()->getCpp2Uno());
        uno_destructData(ret, rtd.get(), reinterpret_cast<uno_ReleaseFunc>(css::uno::cpp_release));
    }
}
}

namespace bridges::cpp_uno::shared
{
void unoInterfaceProxyDispatch(uno_Interface* pUnoI, const typelib_TypeDescription* pMemberDescr,
                               void* pReturn, void* pArgs[], uno_Any** ppException)
{
    bridges::cpp_uno::shared::UnoInterfaceProxy* pThis
        = static_cast<bridges::cpp_uno::shared::UnoInterfaceProxy*>(pUnoI);

    switch (pMemberDescr->eTypeClass)
    {
        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
        {
            std::abort();
            break;
        }
        case typelib_TypeClass_INTERFACE_METHOD:
        {
            VtableSlot aVtableSlot(getVtableSlot(
                reinterpret_cast<typelib_InterfaceMethodTypeDescription const*>(pMemberDescr)));

            switch (aVtableSlot.index)
            {
                case 1: // acquire uno interface
                    (*pUnoI->acquire)(pUnoI);
                    *ppException = 0;
                    break;
                case 2: // release uno interface
                    (*pUnoI->release)(pUnoI);
                    *ppException = 0;
                    break;
                case 0: // queryInterface() opt
                {
                    typelib_TypeDescription* pTD = 0;
                    TYPELIB_DANGER_GET(&pTD, reinterpret_cast<Type*>(pArgs[0])->getTypeLibType());
                    if (pTD)
                    {
                        uno_Interface* pInterface = 0;
                        (*pThis->getBridge()->getUnoEnv()->getRegisteredInterface)(
                            pThis->getBridge()->getUnoEnv(), (void**)&pInterface, pThis->oid.pData,
                            (typelib_InterfaceTypeDescription*)pTD);

                        if (pInterface)
                        {
                            ::uno_any_construct(reinterpret_cast<uno_Any*>(pReturn), &pInterface,
                                                pTD, 0);
                            (*pInterface->release)(pInterface);
                            TYPELIB_DANGER_RELEASE(pTD);
                            *ppException = 0;
                            break;
                        }
                        TYPELIB_DANGER_RELEASE(pTD);
                    }
                } // else perform queryInterface()
                    [[fallthrough]];
                default:
                {
                    auto const mtd
                        = reinterpret_cast<typelib_InterfaceMethodTypeDescription const*>(
                            pMemberDescr);
                    call(pThis, aVtableSlot, mtd->pReturnTypeRef, mtd->nParams, mtd->pParams,
                         pReturn, pArgs, ppException);
                }
            }
            break;
        }
        default:
        {
            ::com::sun::star::uno::RuntimeException aExc(
                "illegal member type description!",
                ::com::sun::star::uno::Reference<::com::sun::star::uno::XInterface>());

            Type const& rExcType = cppu::UnoType<decltype(aExc)>::get();
            // binary identical null reference
            ::uno_type_any_construct(*ppException, &aExc, rExcType.getTypeLibType(), 0);
        }
    }
}

} // namespace bridges::cpp_uno::shared

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
