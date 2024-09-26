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

#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <ios>
#include <ostream>
#include <sstream>

#include <com/sun/star/reflection/XIdlReflection.hpp>
#include <com/sun/star/reflection/XIdlClass.hpp>
#include <com/sun/star/reflection/theCoreReflection.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/uno/TypeClass.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppunit/Message.h>
#include <o3tl/any.hxx>
#include <o3tl/unreachable.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

#include <cppunit/Protector.h>

namespace {

css::uno::Type translateType(css::uno::Reference<css::reflection::XIdlClass> const & type) {
    return css::uno::Type(type->getTypeClass(), type->getName());
}

void printUnoValue(
    std::ostream & out, css::uno::Reference<css::reflection::XIdlReflection> const & reflections,
    css::uno::Type const & type, css::uno::Any const & value)
{
    switch (type.getTypeClass()) {
    case css::uno::TypeClass_VOID:
        out << "void";
        break;
    case css::uno::TypeClass_BOOLEAN:
        out << *o3tl::forceAccess<bool>(value);
        break;
    case css::uno::TypeClass_BYTE:
        out << int(*o3tl::forceAccess<sal_Int8>(value));
        break;
    case css::uno::TypeClass_SHORT:
        out << *o3tl::forceAccess<sal_Int16>(value);
        break;
    case css::uno::TypeClass_UNSIGNED_SHORT:
        out << *o3tl::forceAccess<sal_uInt16>(value);
        break;
    case css::uno::TypeClass_LONG:
        out << *o3tl::forceAccess<sal_Int32>(value);
        break;
    case css::uno::TypeClass_UNSIGNED_LONG:
        out << *o3tl::forceAccess<sal_uInt32>(value);
        break;
    case css::uno::TypeClass_HYPER:
        out << *o3tl::forceAccess<sal_Int64>(value);
        break;
    case css::uno::TypeClass_UNSIGNED_HYPER:
        out << *o3tl::forceAccess<sal_uInt64>(value);
        break;
    case css::uno::TypeClass_FLOAT:
    {
        std::ios_base::fmtflags origfmt = out.flags();
        out << std::uppercase << *o3tl::forceAccess<float>(value);
        out.setf(origfmt);
        break;
    }
    case css::uno::TypeClass_DOUBLE:
    {
        std::ios_base::fmtflags origfmt = out.flags();
        out << std::uppercase << *o3tl::forceAccess<double>(value);
        out.setf(origfmt);
        break;
    }
    case css::uno::TypeClass_CHAR:
    {
        std::ios_base::fmtflags origfmt = out.flags();
        out << "\\u" << std::hex << std::uppercase << std::setw(4) << std::setfill('0')
            << std::uint_least16_t(*o3tl::forceAccess<char16_t>(value));
        out.setf(origfmt);
        break;
    }
    case css::uno::TypeClass_STRING:
        out << '"' << *o3tl::forceAccess<OUString>(value) << '"'; //TODO: encode content
        break;
    case css::uno::TypeClass_TYPE:
        out << o3tl::forceAccess<css::uno::Type>(value)->getTypeName();
        break;
    case css::uno::TypeClass_ANY:
        out << value.getValueTypeName() << ": ";
        printUnoValue(out, reflections, value.getValueType(), value);
        break;
    case css::uno::TypeClass_SEQUENCE:
        {
            css::uno::Reference<css::reflection::XIdlClass> const refl(
                reflections->forName(type.getTypeName()), css::uno::UNO_SET_THROW);
            auto const t = translateType(refl->getComponentType());
            auto const array = refl->getArray();
            auto const n = array->getLen(value);
            out << '[';
            for (sal_Int32 i = 0; i != n; ++i) {
                if (i != 0) {
                    out << ", ";
                }
                printUnoValue(out, reflections, t, array->get(value, i));
            }
            out << ']';
            break;
        }
    case css::uno::TypeClass_ENUM:
        out << value.getValueTypeName() << ' ' << *static_cast<sal_Int32 const *>(value.getValue());
        break;
    case css::uno::TypeClass_STRUCT:
    case css::uno::TypeClass_EXCEPTION:
        {
            out << '{';
            auto first = true;
            for (auto const & f: css::uno::Reference<css::reflection::XIdlClass>(
                     reflections->forName(type.getTypeName()),
                     css::uno::UNO_SET_THROW)->getFields())
            {
                if (first) {
                    first = false;
                } else {
                    out << ", ";
                }
                out << f->getName() << ": ";
                printUnoValue(out, reflections, translateType(f->getType()), f->get(value));
            }
            out << '}';
            break;
        }
    case css::uno::TypeClass_INTERFACE:
        out << '@' << value.getValue();
        break;
    default:
        O3TL_UNREACHABLE;
    }
}

class Prot : public CppUnit::Protector
{
public:
    Prot() {}

    Prot(const Prot&) = delete;
    Prot& operator=(const Prot&) = delete;

    virtual bool protect(
        CppUnit::Functor const & functor,
        CppUnit::ProtectorContext const & context) override;
};

bool Prot::protect(
    CppUnit::Functor const & functor, CppUnit::ProtectorContext const & context)
{
    try {
        return functor();
    } catch (const css::uno::Exception &e) {
        css::uno::Any a(cppu::getCaughtException());
        std::ostringstream s;
        try {
            printUnoValue(
                s,
                css::reflection::theCoreReflection::get(comphelper::getProcessComponentContext()),
                cppu::UnoType<css::uno::Any>::get(), a);
        } catch (css::uno::Exception &) {
            // Best effort to print full details above; if that fails for whatever reason, print as
            // much detailed information as possible, followed by at least the exception type and
            // message:
            s << "... " << a.getValueTypeName() << ": " << e.Message;
        }
        reportError(
            context,
            CppUnit::Message(
                "An uncaught UNO exception",
                s.str()));
    }
    return false;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT CppUnit::Protector *
unoexceptionprotector() {
    return std::getenv("CPPUNIT_PROPAGATE_EXCEPTIONS") == nullptr ? new Prot : nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
