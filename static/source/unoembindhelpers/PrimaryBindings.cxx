/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifdef EMSCRIPTEN
#include <com/sun/star/frame/XModel.hpp>

#include <emscripten.h>
#include <emscripten/bind.h>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Type.hxx>
#include <comphelper/processfactory.hxx>
#include <o3tl/any.hxx>
#include <o3tl/temporary.hxx>
#include <o3tl/unreachable.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <sfx2/viewsh.hxx>
#include <static/unoembindhelpers/PrimaryBindings.hxx>
#include <typelib/typedescription.h>
#include <typelib/typedescription.hxx>
#include <uno/data.h>

#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <utility>

using namespace emscripten;
using namespace css::uno;

template <> struct emscripten::smart_ptr_trait<css::uno::Type>
{
    using PointerType = css::uno::Type;
    using element_type = typelib_TypeDescriptionReference;
    static typelib_TypeDescriptionReference* get(css::uno::Type const& ptr)
    {
        return ptr.getTypeLibType();
    }
    static sharing_policy get_sharing_policy() { return sharing_policy::NONE; }
    static css::uno::Type* share(typelib_TypeDescriptionReference* v)
    {
        return new css::uno::Type(v);
    }
    static css::uno::Type* construct_null() { return new css::uno::Type(); }
};

EM_JS(void, jsRegisterChar, (std::type_info const* raw),
// clang-format off
{
    Module.registerType(raw, {
        name: 'sal_Unicode',
        fromWireType(ptr) {
            let str = String.fromCharCode(Module.HEAPU16[ptr >> 1]);
            return str;
        },
        toWireType(destructors, value) {
            if (typeof value != 'string' || value.length !== 1) {
                Module.throwBindingError(
                    'Cannot pass anything but 1-element string to C++ char16_t');
            }
            let data = Module._malloc(2);
            Module.HEAPU16[data >> 1] = value.charCodeAt(0);
            if (destructors !== null) {
                destructors.push(Module._free, data);
            }
            return data;
        },
        argPackAdvance: 8,
        readValueFromPointer(pointer) {
            return this.fromWireType(Module.HEAPU32[((pointer)>>2)]);
        },
        destructorFunction(ptr) {
            Module._free(ptr);
        },
    });
}
// clang-format on
);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-pp-token"
EM_JS(void, jsRegisterString, (std::type_info const* raw),
// clang-format off
{
    Module.registerType(raw, {
        name: 'rtl::OUString',
        fromWireType(ptr) {
            let data = Module.HEAPU32[ptr >> 2];
            let length = Module.HEAPU32[(data >> 2) + 1];
            let buffer = data + 8;
            let str = '';
            for (let i = 0; i < length; ++i) {
                let c = Module.HEAPU16[(buffer >> 1) + i];
                str += String.fromCharCode(c);
            }
            Module.rtl_uString_release(data);
            Module._free(ptr);
            return str;
        },
        toWireType(destructors, value) {
            if (typeof value != 'string') {
                Module.throwBindingError('Cannot pass non-string to C++ OUString');
            }
            let data = Module._malloc(8 + (value.length + 1) * 2);
            Module.HEAPU32[data >> 2] = 1;
            Module.HEAPU32[(data >> 2) + 1] = value.length;
            let buffer = data + 8;
            for (let i = 0; i < value.length; ++i) {
                Module.HEAPU16[(buffer >> 1) + i] = value.charCodeAt(i);
            }
            Module.HEAPU16[(buffer >> 1) + value.length] = 0;
            let ptr = Module._malloc(4);
            Module.HEAPU32[ptr >> 2] = data;
            if (destructors !== null) {
                destructors.push(Module._free, ptr);
            }
            return ptr;
        },
        argPackAdvance: 8,
        readValueFromPointer(pointer) {
            return this.fromWireType(Module.HEAPU32[((pointer)>>2)]);
        },
        destructorFunction(ptr) {
            Module._free(ptr);
        },
    });
}
// clang-format on
);
#pragma clang diagnostic pop

namespace
{
void copyStruct(typelib_CompoundTypeDescription* desc, void const* source, void* dest)
{
    if (desc->pBaseTypeDescription != nullptr)
    {
        copyStruct(desc->pBaseTypeDescription, source, dest);
    }
    for (sal_Int32 i = 0; i != desc->nMembers; ++i)
    {
        uno_type_copyData(
            static_cast<char*>(dest) + desc->pMemberOffsets[i],
            const_cast<char*>(static_cast<char const*>(source) + desc->pMemberOffsets[i]),
            desc->ppTypeRefs[i], cpp_acquire);
    }
}

template <typename T> void registerInOutParam(char const* name)
{
    class_<unoembindhelpers::UnoInOutParam<T>>(name).constructor().constructor<T>().property(
        "val", &unoembindhelpers::UnoInOutParam<T>::get, &unoembindhelpers::UnoInOutParam<T>::set);
}

Reference<css::frame::XModel> getCurrentModelFromViewSh()
{
    SfxViewShell* pSh = nullptr;
    pSh = SfxViewShell::Current();
    if (!pSh)
    {
        return {};
    }
    return pSh->GetCurrentDocument();
}

struct LessType
{
    bool operator()(css::uno::Type const& type1, css::uno::Type const& type2) const
    {
        return type1.getTypeLibType() < type2.getTypeLibType();
    }
};

std::map<css::uno::Type, std::type_info const*, LessType> unoTypes;

std::type_info const* getTypeId(css::uno::Type const& type)
{
    auto const i = unoTypes.find(type);
    if (i == unoTypes.end())
    {
        throw std::runtime_error("unregistered UNO type");
    }
    return i->second;
}
}

namespace unoembindhelpers::detail
{
void registerUnoType(css::uno::Type const& type, std::type_info const* id) { unoTypes[type] = id; }
}

EMSCRIPTEN_BINDINGS(PrimaryBindings)
{
    enum_<unoembindhelpers::uno_Sequence>("uno_Sequence")
        .value("FromSize", unoembindhelpers::uno_Sequence::FromSize);

    emscripten::class_<typelib_TypeDescriptionReference>("uno_Type")
        .smart_ptr<css::uno::Type>("uno_Type$")
        .class_function("Void", +[]() { return cppu::UnoType<void>::get(); })
        .class_function("Boolean", +[]() { return cppu::UnoType<bool>::get(); })
        .class_function("Byte", +[]() { return cppu::UnoType<sal_Int8>::get(); })
        .class_function("Short", +[]() { return cppu::UnoType<sal_Int16>::get(); })
        .class_function("UnsignedShort", +[]() { return cppu::UnoType<sal_uInt16>::get(); })
        .class_function("Long", +[]() { return cppu::UnoType<sal_Int32>::get(); })
        .class_function("UnsignedLong", +[]() { return cppu::UnoType<sal_uInt32>::get(); })
        .class_function("Hyper", +[]() { return cppu::UnoType<sal_Int64>::get(); })
        .class_function("UnsignedHyper", +[]() { return cppu::UnoType<sal_uInt64>::get(); })
        .class_function("Float", +[]() { return cppu::UnoType<float>::get(); })
        .class_function("Double", +[]() { return cppu::UnoType<double>::get(); })
        .class_function("Char", +[]() { return cppu::UnoType<sal_Unicode>::get(); })
        .class_function("String", +[]() { return cppu::UnoType<OUString>::get(); })
        .class_function("Type", +[]() { return cppu::UnoType<css::uno::Type>::get(); })
        .class_function("Any", +[]() { return cppu::UnoType<css::uno::Any>::get(); })
        .class_function("Sequence",
                        +[](css::uno::Type const& type) {
                            return css::uno::Type(css::uno::TypeClass_SEQUENCE,
                                                  "[]" + type.getTypeName());
                        })
        .class_function("Enum",
                        +[](std::u16string const& name) {
                            return css::uno::Type(css::uno::TypeClass_ENUM, OUString(name));
                        })
        .class_function("Struct",
                        +[](std::u16string const& name) {
                            return css::uno::Type(css::uno::TypeClass_STRUCT, OUString(name));
                        })
        .class_function("Exception",
                        +[](std::u16string const& name) {
                            return css::uno::Type(css::uno::TypeClass_EXCEPTION, OUString(name));
                        })
        .class_function("Interface",
                        +[](std::u16string const& name) {
                            return css::uno::Type(css::uno::TypeClass_INTERFACE, OUString(name));
                        })
        .function("toString", +[](css::uno::Type const& self) {
            auto const name = self.getTypeName();
            return std::u16string(name.getStr(), name.getLength());
        });

    // Any
    class_<Any>("uno_Any")
        .constructor(+[](const css::uno::Type& rUnoType, const val& rObject) -> Any {
            switch (rUnoType.getTypeClass())
            {
                case TypeClass_VOID:
                    return {};
                case TypeClass_BOOLEAN:
                    return Any{ rObject.as<bool>() };
                case TypeClass_BYTE:
                    return Any{ rObject.as<sal_Int8>() };
                case TypeClass_SHORT:
                    return Any{ rObject.as<sal_Int16>() };
                case TypeClass_UNSIGNED_SHORT:
                    return Any{ rObject.as<sal_uInt16>() };
                case TypeClass_LONG:
                    return Any{ rObject.as<sal_Int32>() };
                case TypeClass_UNSIGNED_LONG:
                    return Any{ rObject.as<sal_uInt32>() };
                case TypeClass_HYPER:
                    return Any{ rObject.as<sal_Int64>() };
                case TypeClass_UNSIGNED_HYPER:
                    return Any{ rObject.as<sal_uInt64>() };
                case TypeClass_FLOAT:
                    return Any{ rObject.as<float>() };
                case TypeClass_DOUBLE:
                    return Any{ rObject.as<double>() };
                case TypeClass_CHAR:
                    return Any{ rObject.as<char16_t>() };
                case TypeClass_STRING:
                    return Any{ OUString(rObject.as<std::u16string>()) };
                case TypeClass_TYPE:
                    return css::uno::Any(rObject.as<css::uno::Type>());
                case TypeClass_SEQUENCE:
                case TypeClass_STRUCT:
                case TypeClass_EXCEPTION:
                case TypeClass_INTERFACE:
                {
                    emscripten::internal::EM_DESTRUCTORS destructors = nullptr;
                    emscripten::internal::EM_GENERIC_WIRE_TYPE result
                        = _emval_as(rObject.as_handle(), getTypeId(rUnoType), &destructors);
                    emscripten::internal::DestructorsRunner dr(destructors);
                    return css::uno::Any(
                        emscripten::internal::fromGenericWireType<void const*>(result), rUnoType);
                }
                case TypeClass_ENUM:
                {
                    emscripten::internal::EM_DESTRUCTORS destructors = nullptr;
                    emscripten::internal::EM_GENERIC_WIRE_TYPE result
                        = _emval_as(rObject.as_handle(), getTypeId(rUnoType), &destructors);
                    emscripten::internal::DestructorsRunner dr(destructors);
                    return css::uno::Any(
                        &o3tl::temporary(
                            emscripten::internal::fromGenericWireType<sal_Int32>(result)),
                        rUnoType);
                }
                default:
                    throw std::invalid_argument("bad type class");
            }
        })
        .function("get", +[](css::uno::Any const& self) {
            switch (self.getValueType().getTypeClass())
            {
                case css::uno::TypeClass_VOID:
                    return emscripten::val::undefined();
                case css::uno::TypeClass_BOOLEAN:
                    return emscripten::val(*o3tl::forceAccess<bool>(self));
                case css::uno::TypeClass_BYTE:
                    return emscripten::val(*o3tl::forceAccess<sal_Int8>(self));
                case css::uno::TypeClass_SHORT:
                    return emscripten::val(*o3tl::forceAccess<sal_Int16>(self));
                case css::uno::TypeClass_UNSIGNED_SHORT:
                    return emscripten::val(*o3tl::forceAccess<sal_uInt16>(self));
                case css::uno::TypeClass_LONG:
                    return emscripten::val(*o3tl::forceAccess<sal_Int32>(self));
                case css::uno::TypeClass_UNSIGNED_LONG:
                    return emscripten::val(*o3tl::forceAccess<sal_uInt32>(self));
                case css::uno::TypeClass_HYPER:
                    return emscripten::val(*o3tl::forceAccess<sal_Int64>(self));
                case css::uno::TypeClass_UNSIGNED_HYPER:
                    return emscripten::val(*o3tl::forceAccess<sal_uInt64>(self));
                case css::uno::TypeClass_FLOAT:
                    return emscripten::val(*o3tl::forceAccess<float>(self));
                case css::uno::TypeClass_DOUBLE:
                    return emscripten::val(*o3tl::forceAccess<double>(self));
                case css::uno::TypeClass_CHAR:
                    return emscripten::val(*o3tl::forceAccess<sal_Unicode>(self));
                case css::uno::TypeClass_STRING:
                    return emscripten::val(*o3tl::forceAccess<OUString>(self));
                case css::uno::TypeClass_TYPE:
                    return emscripten::val(*o3tl::forceAccess<css::uno::Type>(self));
                case css::uno::TypeClass_SEQUENCE:
                {
                    auto const seq = *static_cast<uno_Sequence* const*>(self.getValue());
                    auto const copy = std::malloc(sizeof(uno_Sequence*));
                    *static_cast<uno_Sequence**>(copy) = seq;
                    osl_atomic_increment(&seq->nRefCount);
                    emscripten::internal::WireTypePack argv(std::move(copy));
                    return emscripten::val::take_ownership(
                        _emval_take_value(getTypeId(self.getValueType()), argv));
                }
                case css::uno::TypeClass_ENUM:
                {
                    emscripten::internal::WireTypePack argv(
                        std::move(*static_cast<sal_Int32 const*>(self.getValue())));
                    return emscripten::val::take_ownership(
                        _emval_take_value(getTypeId(self.getValueType()), argv));
                }
                case css::uno::TypeClass_STRUCT:
                case css::uno::TypeClass_EXCEPTION:
                {
                    css::uno::TypeDescription desc(self.getValueType().getTypeLibType());
                    assert(desc.is());
                    auto const td = reinterpret_cast<typelib_CompoundTypeDescription*>(desc.get());
                    auto const copy = std::malloc(td->aBase.nSize);
                    copyStruct(td, self.getValue(), copy);
                    emscripten::internal::WireTypePack argv(std::move(copy));
                    return emscripten::val::take_ownership(
                        _emval_take_value(getTypeId(self.getValueType()), argv));
                }
                case css::uno::TypeClass_INTERFACE:
                {
                    auto const ifc = *static_cast<css::uno::XInterface* const*>(self.getValue());
                    auto const copy = std::malloc(sizeof(css::uno::XInterface*));
                    *static_cast<css::uno::XInterface**>(copy) = ifc;
                    ifc->acquire();
                    emscripten::internal::WireTypePack argv(std::move(copy));
                    return emscripten::val::take_ownership(
                        _emval_take_value(getTypeId(self.getValueType()), argv));
                }
                default:
                    O3TL_UNREACHABLE;
            };
        });

    registerInOutParam<bool>("uno_InOutParam_boolean");
    registerInOutParam<sal_Int8>("uno_InOutParam_byte");
    registerInOutParam<sal_Int16>("uno_InOutParam_short");
    registerInOutParam<sal_uInt16>("uno_InOutParam_unsigned_short");
    registerInOutParam<sal_Int32>("uno_InOutParam_long");
    registerInOutParam<sal_uInt32>("uno_InOutParam_unsigned_long");
    registerInOutParam<sal_Int64>("uno_InOutParam_hyper");
    registerInOutParam<sal_uInt64>("uno_InOutParam_unsigned_hyper");
    registerInOutParam<float>("uno_InOutParam_float");
    registerInOutParam<double>("uno_InOutParam_double");
    registerInOutParam<char16_t>("uno_InOutParam_char");

    function("getCurrentModelFromViewSh", &getCurrentModelFromViewSh);
    function("getUnoComponentContext", &comphelper::getProcessComponentContext);
    function("rtl_uString_release",
             +[](std::uintptr_t ptr) { rtl_uString_release(reinterpret_cast<rtl_uString*>(ptr)); });

    jsRegisterChar(&typeid(char16_t));
    jsRegisterString(&typeid(OUString));
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
