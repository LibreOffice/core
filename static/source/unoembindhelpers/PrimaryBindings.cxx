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

#include <emscripten/bind.h>

#include <comphelper/processfactory.hxx>
#include <sal/log.hxx>
#include <sfx2/viewsh.hxx>
#include <static/unoembindhelpers/PrimaryBindings.hxx>

using namespace emscripten;
using namespace css::uno;

namespace
{
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
    // Reference bits
    enum_<unoembindhelpers::uno_Reference>("uno_Reference")
        .value("FromAny", unoembindhelpers::uno_Reference::FromAny);

    class_<OUString>("OUString")
        .constructor(+[](const std::u16string& rString) -> OUString { return OUString(rString); })
        .function("toString", +[](const OUString& rSelf) -> std::u16string {
            return std::u16string(rSelf.getStr(), rSelf.getLength());
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
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
