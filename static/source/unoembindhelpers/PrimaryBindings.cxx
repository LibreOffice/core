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

#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Type.hxx>
#include <comphelper/processfactory.hxx>
#include <rtl/string.hxx>
#include <rtl/textcvt.h>
#include <rtl/textenc.h>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <sfx2/viewsh.hxx>
#include <static/unoembindhelpers/PrimaryBindings.hxx>
#include <typelib/typedescription.h>

#include <cstdint>
#include <string>
#include <typeinfo>

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
    static sharing_policy get_sharing_policy() { return sharing_policy::INTRUSIVE; }
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
OString toUtf8(OUString const& string)
{
    OString s;
    if (!string.convertToString(&s, RTL_TEXTENCODING_UTF8,
                                RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
                                    | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR))
    {
        throw css::uno::RuntimeException("cannot convert OUString to UTF-8");
    }
    return s;
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
}

EMSCRIPTEN_BINDINGS(PrimaryBindings)
{
    enum_<unoembindhelpers::uno_Reference>("uno_Reference")
        .value("FromAny", unoembindhelpers::uno_Reference::FromAny);
    enum_<unoembindhelpers::uno_Sequence>("uno_Sequence")
        .value("FromSize", unoembindhelpers::uno_Sequence::FromSize);

    emscripten::class_<typelib_TypeDescriptionReference>("uno_Type")
        .smart_ptr<css::uno::Type>("uno_Type$")
        .function("toString", +[](css::uno::Type const& self) {
            auto const name = toUtf8(self.getTypeName());
            return std::string(name.getStr(), name.getLength());
        });

    // Any
    class_<Any>("Any").constructor(+[](const val& rObject, const TypeClass& rUnoType) -> Any {
        switch (rUnoType)
        {
            case TypeClass_VOID:
                break;
            case TypeClass_CHAR:
                return Any{ rObject.as<sal_Int8>() };
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
            case TypeClass_STRING:
                return Any{ OUString(rObject.as<std::u16string>()) };
            case TypeClass_TYPE:
            case TypeClass_ANY:
            case TypeClass_ENUM:
            case TypeClass_STRUCT:
            case TypeClass_EXCEPTION:
            case TypeClass_SEQUENCE:
            case TypeClass_INTERFACE:
            case TypeClass_TYPEDEF:
            case TypeClass_SERVICE:
            case TypeClass_MODULE:
            case TypeClass_INTERFACE_METHOD:
            case TypeClass_INTERFACE_ATTRIBUTE:
            default:
                break;
        }
        return {};
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
