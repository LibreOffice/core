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
#include <com/sun/star/uno/XReference.hpp>

#include <emscripten/bind.h>

#include <sal/log.hxx>
#include <sfx2/viewsh.hxx>

using namespace emscripten;
using namespace css::uno;

namespace
{
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
    class_<BaseReference>("BaseReference");
    enum_<UnoReference_Query>("UnoReference_Query").value("UNO_QUERY", UNO_QUERY);

    class_<OUString>("OUString")
        .constructor(+[](const std::u16string& rString) -> OUString { return OUString(rString); },
                     allow_raw_pointers())
        .function("toString", +[](const OUString& rSelf) -> std::u16string {
            return std::u16string(rSelf.getStr(), rSelf.getLength());
        });

    // Types used for Any construction
    enum_<TypeClass>("UnoType")
        .value("void", TypeClass::TypeClass_VOID)
        .value("char", TypeClass::TypeClass_CHAR)
        .value("bool", TypeClass::TypeClass_BOOLEAN)
        .value("byte", TypeClass::TypeClass_BYTE)
        .value("short", TypeClass::TypeClass_SHORT)
        .value("unsigned_short", TypeClass::TypeClass_UNSIGNED_SHORT)
        .value("long", TypeClass::TypeClass_LONG)
        .value("unsigned_long", TypeClass::TypeClass_UNSIGNED_LONG)
        .value("hyper", TypeClass::TypeClass_HYPER)
        .value("unsigned_hyper", TypeClass::TypeClass_UNSIGNED_HYPER)
        .value("float", TypeClass::TypeClass_FLOAT)
        .value("double", TypeClass::TypeClass_DOUBLE)
        .value("string", TypeClass::TypeClass_STRING);

    // Any
    class_<Any>("Any").constructor(
        +[](const val& rObject, const TypeClass& rUnoType) -> Any {
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
        },
        allow_raw_pointers());

    function("getCurrentModelFromViewSh", &getCurrentModelFromViewSh);
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
