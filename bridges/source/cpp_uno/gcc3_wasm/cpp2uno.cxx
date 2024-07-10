/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <typeinfo>

#include <emscripten.h>

#include <com/sun/star/uno/RuntimeException.hpp>
#include <o3tl/string_view.hxx>
#include <o3tl/unreachable.hxx>
#include <rtl/strbuf.hxx>
#include <typelib/typeclass.h>
#include <typelib/typedescription.hxx>

#include <bridge.hxx>
#include <cppinterfaceproxy.hxx>
#include <types.hxx>
#include <vtablefactory.hxx>

#include "abi.hxx"

EM_JS(void*, jsGetExportedSymbol, (char const* name),
      // clang-format off
{
    const val = Module["_" + UTF8ArrayToString(HEAPU8, name)];
    return typeof val === "number" || typeof val === "bigint" ? val : 0;
}
      // clang-format on
);

using bridges::cpp_uno::shared::VtableFactory;

struct VtableFactory::Slot
{
    void const* fn;
};

VtableFactory::Slot* VtableFactory::mapBlockToVtable(void* block)
{
    return static_cast<Slot*>(block) + 2;
}

std::size_t VtableFactory::getBlockSize(sal_Int32 slotCount)
{
    return (slotCount + 2) * sizeof(Slot);
}

namespace
{
// Some dummy type whose RTTI is used in the synthesized proxy vtables to make uses of dynamic_cast
// on such proxy objects not crash:
struct ProxyRtti
{
};
}

VtableFactory::Slot* VtableFactory::initializeBlock(void* block, sal_Int32 slotCount, sal_Int32,
                                                    typelib_InterfaceTypeDescription*)
{
    Slot* slots = mapBlockToVtable(block);
    slots[-2].fn = nullptr;
    slots[-1].fn = &typeid(ProxyRtti);
    return slots + slotCount;
}

namespace
{
class Rtti
{
public:
    std::type_info* getRtti(typelib_TypeDescription const& type);

private:
    typedef std::unordered_map<OUString, std::type_info*> Map;

    osl::Mutex mutex_;
    Map map_;
};

std::type_info* Rtti::getRtti(typelib_TypeDescription const& type)
{
    OUString unoName(type.pTypeName);
    osl::MutexGuard g(mutex_);
    Map::iterator i(map_.find(unoName));
    if (i == map_.end())
    {
        OStringBuffer b("_ZTI");
        auto const ns = unoName.indexOf('.') != 0;
        if (ns)
        {
            b.append('N');
        }
        for (sal_Int32 j = 0; j != -1;)
        {
            OString s(
                OUStringToOString(o3tl::getToken(unoName, 0, '.', j), RTL_TEXTENCODING_ASCII_US));
            b.append(OString::number(s.getLength()) + s);
        }
        if (ns)
        {
            b.append('E');
        }
        OString sym(b.makeStringAndClear());
        std::type_info* rtti = static_cast<std::type_info*>(jsGetExportedSymbol(sym.getStr()));
        if (rtti == nullptr)
        {
            char const* rttiName = strdup(sym.getStr() + std::strlen("_ZTI"));
            if (rttiName == nullptr)
            {
                throw std::bad_alloc();
            }
            assert(type.eTypeClass == typelib_TypeClass_EXCEPTION);
            typelib_CompoundTypeDescription const& ctd
                = reinterpret_cast<typelib_CompoundTypeDescription const&>(type);
            if (ctd.pBaseTypeDescription == nullptr)
            {
                rtti = new __cxxabiv1::__class_type_info(rttiName);
            }
            else
            {
                std::type_info* base = getRtti(ctd.pBaseTypeDescription->aBase);
                auto const sicti = new __cxxabiv1::__si_class_type_info(rttiName);
                sicti->__base_type = static_cast<__cxxabiv1::__class_type_info*>(base);
                rtti = sicti;
            }
        }
        i = map_.insert(Map::value_type(unoName, rtti)).first;
    }
    return i->second;
}

struct theRttiFactory : public rtl::Static<Rtti, theRttiFactory>
{
};

std::type_info* getRtti(typelib_TypeDescription const& type)
{
    return theRttiFactory::get().getRtti(type);
}

extern "C" void* /*_GLIBCXX_CDTOR_CALLABI*/ deleteException(void* exception)
{
    __cxxabiv1::__cxa_exception* header = static_cast<__cxxabiv1::__cxa_exception*>(exception) - 1;
    assert(header->exceptionDestructor == &deleteException);
    OUString unoName(emscriptencxxabi::toUnoName(header->exceptionType->name()));
    typelib_TypeDescription* td = nullptr;
    typelib_typedescription_getByName(&td, unoName.pData);
    assert(td != nullptr);
    uno_destructData(exception, td, &css::uno::cpp_release);
    typelib_typedescription_release(td);
    return exception;
}

void raiseException(uno_Any* any, uno_Mapping* mapping)
{
    typelib_TypeDescription* td = nullptr;
    TYPELIB_DANGER_GET(&td, any->pType);
    if (td == nullptr)
    {
        throw css::uno::RuntimeException("no typedescription for "
                                         + OUString::unacquired(&any->pType->pTypeName));
    }
    void* exc = __cxxabiv1::__cxa_allocate_exception(td->nSize);
    uno_copyAndConvertData(exc, any->pData, td, mapping);
    uno_any_destruct(any, nullptr);
    std::type_info* rtti = getRtti(*td);
    TYPELIB_DANGER_RELEASE(td);
    __cxxabiv1::__cxa_throw(exc, rtti, deleteException);
}

void call(bridges::cpp_uno::shared::CppInterfaceProxy* proxy,
          css::uno::TypeDescription const& description,
          typelib_TypeDescriptionReference* returnType, sal_Int32 count,
          typelib_MethodParameter* parameters, std::vector<sal_uInt64> arguments,
          unsigned /*indirectRect*/)
{
    typelib_TypeDescription* rtd = nullptr;
    if (returnType != nullptr)
    {
        TYPELIB_DANGER_GET(&rtd, returnType);
    }
    void* retin = nullptr;

    void** args = static_cast<void**>(alloca(count * sizeof(void*)));
    void** cppArgs = static_cast<void**>(alloca(count * sizeof(void*)));
    typelib_TypeDescription** argtds
        = static_cast<typelib_TypeDescription**>(alloca(count * sizeof(typelib_TypeDescription*)));
    std::size_t argument_index = 0;
    for (sal_Int32 i = 0; i != count; ++i)
    {
        if (!parameters[i].bOut && bridges::cpp_uno::shared::isSimpleType(parameters[i].pTypeRef))
        {
            assert(false);
        }
        else
        {
            cppArgs[i] = reinterpret_cast<void*>(arguments[argument_index++]);
            typelib_TypeDescription* ptd = nullptr;
            TYPELIB_DANGER_GET(&ptd, parameters[i].pTypeRef);
            if (!parameters[i].bIn)
            {
                args[i] = alloca(ptd->nSize);
                argtds[i] = ptd;
            }
            else if (bridges::cpp_uno::shared::relatesToInterfaceType(ptd))
            {
                args[i] = alloca(ptd->nSize);
                uno_copyAndConvertData(args[i], cppArgs[i], ptd, proxy->getBridge()->getCpp2Uno());
                argtds[i] = ptd;
            }
            else
            {
                args[i] = cppArgs[i];
                argtds[i] = nullptr;
                TYPELIB_DANGER_RELEASE(ptd);
            }
        }
    }
    uno_Any exc;
    uno_Any* pexc = &exc;
    proxy->getUnoI()->pDispatcher(proxy->getUnoI(), description.get(), retin, args, &pexc);
    if (pexc != nullptr)
    {
        for (sal_Int32 i = 0; i != count; ++i)
        {
            if (argtds[i] != nullptr)
            {
                if (parameters[i].bIn)
                {
                    uno_destructData(args[i], argtds[i], nullptr);
                }
                TYPELIB_DANGER_RELEASE(argtds[i]);
            }
        }
        if (rtd != nullptr)
        {
            TYPELIB_DANGER_RELEASE(rtd);
        }
        raiseException(&exc, proxy->getBridge()->getUno2Cpp());
    }
    assert(false);
}

void vtableCall(sal_Int32 functionIndex, sal_Int32 vtableOffset, unsigned thisPtr,
                std::vector<sal_uInt64> const& arguments, unsigned indirectRet)
{
    bridges::cpp_uno::shared::CppInterfaceProxy* proxy
        = bridges::cpp_uno::shared::CppInterfaceProxy::castInterfaceToProxy(
            reinterpret_cast<char*>(thisPtr) - vtableOffset);
    typelib_InterfaceTypeDescription* type = proxy->getTypeDescr();
    assert(functionIndex < type->nMapFunctionIndexToMemberIndex);
    sal_Int32 pos = type->pMapFunctionIndexToMemberIndex[functionIndex];
    css::uno::TypeDescription desc(type->ppAllMembers[pos]);
    switch (desc.get()->eTypeClass)
    {
        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
            if (type->pMapMemberIndexToFunctionIndex[pos] == functionIndex)
            {
                // Getter:
                call(proxy, desc,
                     reinterpret_cast<typelib_InterfaceAttributeTypeDescription*>(desc.get())
                         ->pAttributeTypeRef,
                     0, nullptr, arguments, indirectRet);
            }
            else
            {
                // Setter:
                typelib_MethodParameter param
                    = { nullptr,
                        reinterpret_cast<typelib_InterfaceAttributeTypeDescription*>(desc.get())
                            ->pAttributeTypeRef,
                        true, false };
                call(proxy, desc, nullptr, 1, &param, arguments, indirectRet);
            }
            break;
        case typelib_TypeClass_INTERFACE_METHOD:
            switch (functionIndex)
            {
                case 1:
                    proxy->acquireProxy();
                    break;
                case 2:
                    proxy->releaseProxy();
                    break;
                case 0:
                {
                    typelib_TypeDescription* td = nullptr;
                    TYPELIB_DANGER_GET(
                        &td, (reinterpret_cast<css::uno::Type*>(arguments[0])->getTypeLibType()));
                    if (td != nullptr && td->eTypeClass == typelib_TypeClass_INTERFACE)
                    {
                        css::uno::XInterface* ifc = nullptr;
                        proxy->getBridge()->getCppEnv()->getRegisteredInterface(
                            proxy->getBridge()->getCppEnv(), reinterpret_cast<void**>(&ifc),
                            proxy->getOid().pData,
                            reinterpret_cast<typelib_InterfaceTypeDescription*>(td));
                        if (ifc != nullptr)
                        {
                            uno_any_construct(
                                reinterpret_cast<uno_Any*>(indirectRet), &ifc, td,
                                reinterpret_cast<uno_AcquireFunc>(css::uno::cpp_acquire));
                            ifc->release();
                            TYPELIB_DANGER_RELEASE(td);
                            break;
                        }
                        TYPELIB_DANGER_RELEASE(td);
                    }
                }
                    [[fallthrough]];
                default:
                    call(proxy, desc,
                         reinterpret_cast<typelib_InterfaceMethodTypeDescription*>(desc.get())
                             ->pReturnTypeRef,
                         reinterpret_cast<typelib_InterfaceMethodTypeDescription*>(desc.get())
                             ->nParams,
                         reinterpret_cast<typelib_InterfaceMethodTypeDescription*>(desc.get())
                             ->pParams,
                         arguments, indirectRet);
                    break;
            }
            break;
        default:
            O3TL_UNREACHABLE;
    }
}

extern "C" void vtableSlotFunction_0_0Ii(unsigned indirectRet, unsigned thisPtr, unsigned arg1)
{
    vtableCall(0, 0, thisPtr, { sal_uInt64(arg1) }, indirectRet);
}

extern "C" void vtableSlotFunction_1_0v(unsigned thisPtr)
{
    vtableCall(1, 0, thisPtr, {}, reinterpret_cast<unsigned>(nullptr));
}

extern "C" void vtableSlotFunction_2_0v(unsigned thisPtr)
{
    vtableCall(2, 0, thisPtr, {}, reinterpret_cast<unsigned>(nullptr));
}

extern "C" void vtableSlotFunction_3_0vi(unsigned thisPtr, unsigned arg1)
{
    vtableCall(3, 0, thisPtr, { arg1 }, reinterpret_cast<unsigned>(nullptr));
}

extern "C" void vtableSlotFunction_4_0v(unsigned thisPtr)
{
    vtableCall(4, 0, thisPtr, {}, reinterpret_cast<unsigned>(nullptr));
}

void const* getVtableSlotFunction(std::string_view signature)
{
    if (signature == "0_0Ii")
    {
        return reinterpret_cast<void const*>(vtableSlotFunction_0_0Ii);
    }
    if (signature == "1_0v")
    {
        return reinterpret_cast<void const*>(vtableSlotFunction_1_0v);
    }
    if (signature == "2_0v")
    {
        return reinterpret_cast<void const*>(vtableSlotFunction_2_0v);
    }
    if (signature == "3_0vi")
    {
        return reinterpret_cast<void const*>(vtableSlotFunction_3_0vi);
    }
    if (signature == "4_0v")
    {
        return reinterpret_cast<void const*>(vtableSlotFunction_4_0v);
    }
    throw css::uno::RuntimeException("Wasm bridge cannot fill virtual function slot with signature "
                                     + OUString::fromUtf8(signature));
}
}

unsigned char* VtableFactory::addLocalFunctions(Slot** slots, unsigned char* code,
                                                typelib_InterfaceTypeDescription const* type,
                                                sal_Int32 functionOffset, sal_Int32 functionCount,
                                                sal_Int32 vtableOffset)
{
    *slots -= functionCount;
    auto s = *slots;
    for (sal_Int32 i = 0; i != type->nMembers; ++i)
    {
        switch (type->ppMembers[i]->eTypeClass)
        {
            case typelib_TypeClass_INTERFACE_ATTRIBUTE:
                (s++)->fn = nullptr; //TODO
                ++functionOffset;
                if (!reinterpret_cast<typelib_InterfaceAttributeTypeDescription*>(
                         css::uno::TypeDescription(type->ppMembers[i]).get())
                         ->bReadOnly)
                {
                    (s++)->fn = nullptr; //TODO
                    ++functionOffset;
                }
                break;
            case typelib_TypeClass_INTERFACE_METHOD:
            {
                OStringBuffer sig;
                sig.append(OString::number(functionOffset) + "_" + OString::number(vtableOffset));
                auto const mtd = reinterpret_cast<typelib_InterfaceMethodTypeDescription*>(
                    css::uno::TypeDescription(type->ppMembers[i]).get());
                switch (mtd->pReturnTypeRef->eTypeClass)
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
                        sig.append('I');
                        break;
                    case typelib_TypeClass_STRUCT:
                    {
                        css::uno::TypeDescription rtd(mtd->pReturnTypeRef);
                        switch (abi_wasm::getKind(
                            reinterpret_cast<typelib_CompoundTypeDescription const*>(rtd.get())))
                        {
                            case abi_wasm::StructKind::Empty:
                                break;
                            case abi_wasm::StructKind::I32:
                                sig.append('i');
                                break;
                            case abi_wasm::StructKind::I64:
                                sig.append('j');
                                break;
                            case abi_wasm::StructKind::F32:
                                sig.append('f');
                                break;
                            case abi_wasm::StructKind::F64:
                                sig.append('d');
                                break;
                            case abi_wasm::StructKind::General:
                                sig.append('I');
                                break;
                        }
                        break;
                    }
                    default:
                        O3TL_UNREACHABLE;
                }
                for (sal_Int32 j = 0; j != mtd->nParams; ++j)
                {
                    if (!mtd->pParams[j].bOut
                        && bridges::cpp_uno::shared::isSimpleType(mtd->pParams[j].pTypeRef))
                    {
                        switch (mtd->pParams[j].pTypeRef->eTypeClass)
                        {
                            case typelib_TypeClass_BOOLEAN:
                                sig.append('i');
                                break;
                            case typelib_TypeClass_BYTE:
                                sig.append('i');
                                break;
                            case typelib_TypeClass_SHORT:
                                sig.append('i');
                                break;
                            case typelib_TypeClass_UNSIGNED_SHORT:
                                sig.append('i');
                                break;
                            case typelib_TypeClass_LONG:
                            case typelib_TypeClass_ENUM:
                                sig.append('i');
                                break;
                            case typelib_TypeClass_UNSIGNED_LONG:
                                sig.append('i');
                                break;
                            case typelib_TypeClass_HYPER:
                                sig.append('j');
                                break;
                            case typelib_TypeClass_UNSIGNED_HYPER:
                                sig.append('j');
                                break;
                            case typelib_TypeClass_FLOAT:
                                sig.append('f');
                                break;
                            case typelib_TypeClass_DOUBLE:
                                sig.append('d');
                                break;
                            case typelib_TypeClass_CHAR:
                                sig.append('i');
                                break;
                            default:
                                O3TL_UNREACHABLE;
                        }
                    }
                    else
                    {
                        sig.append('i');
                    }
                }
                (s++)->fn = getVtableSlotFunction(sig);
                ++functionOffset;
                break;
            }
            default:
                O3TL_UNREACHABLE;
        }
    }
    return code;
}

void VtableFactory::flushCode(unsigned char const*, unsigned char const*) {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
