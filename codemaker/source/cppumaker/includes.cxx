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


#include "includes.hxx"

#include "dependencies.hxx"
#include "dumputils.hxx"

#include "codemaker/global.hxx"
#include "codemaker/typemanager.hxx"
#include "codemaker/unotype.hxx"

#include "osl/diagnose.h"
#include "rtl/ref.hxx"
#include "rtl/string.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include <vector>

using codemaker::cppumaker::Includes;

Includes::Includes(
    rtl::Reference< TypeManager > const & manager,
    codemaker::cppumaker::Dependencies const & dependencies, bool hpp):
    m_manager(manager), m_map(dependencies.getMap()), m_hpp(hpp),
    m_includeCassert(false), m_includeException(false),
    m_includeAny(dependencies.hasAnyDependency()), m_includeReference(false),
    m_includeSequence(dependencies.hasSequenceDependency()),
    m_includeType(dependencies.hasTypeDependency()),
    m_includeCppuMacrosHxx(false), m_includeCppuUnotypeHxx(false),
    m_includeOslDoublecheckedlockingH(false), m_includeOslMutexHxx(false),
    m_includeRtlStrbufHxx(false), m_includeRtlStringH(false),
    m_includeRtlTextencH(false), m_includeRtlUstrbufHxx(false),
    m_includeRtlUstringH(false),
    m_includeRtlUstringHxx(dependencies.hasStringDependency()),
    m_includeRtlInstanceHxx(false),
    m_includeSalTypesH(
        dependencies.hasBooleanDependency() || dependencies.hasByteDependency()
        || dependencies.hasShortDependency()
        || dependencies.hasUnsignedShortDependency()
        || dependencies.hasLongDependency()
        || dependencies.hasUnsignedShortDependency()
        || dependencies.hasHyperDependency()
        || dependencies.hasUnsignedHyperDependency()
        || dependencies.hasCharDependency()),
    m_includeTypelibTypeclassH(false),
    m_includeTypelibTypedescriptionH(false)
{}

Includes::~Includes()
{}

void Includes::add(OString const & entityName) {
    sal_Int32 k;
    std::vector< OString > args;
    OUString n(b2u(codemaker::UnoType::decompose(entityName, &k, &args)));
    if (k != 0) {
        m_includeSequence = true;
    }
    switch (m_manager->getSort(n)) {
    case codemaker::UnoType::SORT_BOOLEAN:
    case codemaker::UnoType::SORT_BYTE:
    case codemaker::UnoType::SORT_SHORT:
    case codemaker::UnoType::SORT_UNSIGNED_SHORT:
    case codemaker::UnoType::SORT_LONG:
    case codemaker::UnoType::SORT_UNSIGNED_LONG:
    case codemaker::UnoType::SORT_HYPER:
    case codemaker::UnoType::SORT_UNSIGNED_HYPER:
    case codemaker::UnoType::SORT_CHAR:
        m_includeSalTypesH = true;
        break;
    case codemaker::UnoType::SORT_FLOAT:
    case codemaker::UnoType::SORT_DOUBLE:
        break;
    case codemaker::UnoType::SORT_STRING:
        m_includeRtlUstringHxx = true;
        break;
    case codemaker::UnoType::SORT_TYPE:
        m_includeType = true;
        break;
    case codemaker::UnoType::SORT_ANY:
        m_includeAny = true;
        break;
    case codemaker::UnoType::SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE:
        for (std::vector< OString >::iterator i(args.begin()); i != args.end();
             ++i)
        {
            add(*i);
        }
        // fall through
    case codemaker::UnoType::SORT_SEQUENCE_TYPE:
    case codemaker::UnoType::SORT_ENUM_TYPE:
    case codemaker::UnoType::SORT_PLAIN_STRUCT_TYPE:
    case codemaker::UnoType::SORT_EXCEPTION_TYPE:
    case codemaker::UnoType::SORT_INTERFACE_TYPE:
    case codemaker::UnoType::SORT_TYPEDEF:
        m_map.insert(
            Dependencies::Map::value_type(n, Dependencies::KIND_NO_BASE));
        break;
    default:
        throw CannotDumpException(
            "unexpected type \"" + b2u(entityName)
            + "\" in call to codemaker::cppumaker::Includes::add");
    }
}

namespace {

void dumpEmptyLineBeforeFirst(FileStream & out, bool * first) {
    OSL_ASSERT(first != nullptr);
    if (*first) {
        out << "\n";
        *first = false;
    }
}

}

void Includes::dump(FileStream & out, OUString const * companionHdl) {
    OSL_ASSERT(companionHdl == nullptr || m_hpp);
    if (!m_includeReference) {
        for (Dependencies::Map::iterator i(m_map.begin()); i != m_map.end();
             ++i)
        {
            if (isInterfaceType(u2b(i->first))) {
                m_includeReference = true;
                break;
            }
        }
    }
    out << "#include \"sal/config.h\"\n";
    if (m_includeCassert || m_includeException) {
        out << "\n";
        if (m_includeCassert) {
            out << "#include <cassert>\n";
        }
        if (m_includeException) {
            out << "#include <exception>\n";
        }
    }
    if (companionHdl) {
        out << "\n";
        dumpInclude(out, u2b(*companionHdl), false);
    }
    bool first = true;
    for (Dependencies::Map::iterator i(m_map.begin()); i != m_map.end(); ++i)
    {
        dumpEmptyLineBeforeFirst(out, &first);
        if (m_hpp || i->second == Dependencies::KIND_BASE
            || !isInterfaceType(u2b(i->first)))
        {
            dumpInclude(out, u2b(i->first), m_hpp);
        } else {
            bool ns = dumpNamespaceOpen(out, i->first, false);
            if (ns) {
                out << " ";
            }
            out << "class ";
            dumpTypeIdentifier(out, i->first);
            out << ";";
            if (ns) {
                out << " ";
            }
            dumpNamespaceClose(out, i->first, false);
            out << "\n";
        }
    }
    static char const * hxxExtension[2] = { "h", "hxx" };
    if (m_includeAny) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << "#include \"com/sun/star/uno/Any." << hxxExtension[m_hpp]
            << "\"\n";
    }
    if (m_includeReference) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << "#include \"com/sun/star/uno/Reference." << hxxExtension[m_hpp]
            << "\"\n";
    }
    if (m_includeSequence) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << "#include \"com/sun/star/uno/Sequence." << hxxExtension[m_hpp]
            << "\"\n";
    }
    if (m_includeType) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << "#include \"com/sun/star/uno/Type." << hxxExtension[m_hpp]
            << "\"\n";
    }
    if (m_includeCppuMacrosHxx) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << ("#include \"cppu/macros.hxx\"\n");
    }
    if (m_includeCppuUnotypeHxx) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << ("#include \"cppu/unotype.hxx\"\n");
    }
    if (m_includeOslDoublecheckedlockingH) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << ("#include \"osl/doublecheckedlocking.h\"\n");
    }
    if (m_includeOslMutexHxx) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << "#include \"osl/mutex.hxx\"\n";
    }
    if (m_includeRtlStrbufHxx) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << ("#include \"rtl/strbuf.hxx\"\n");
    }
    if (m_includeRtlStringH) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << "#include \"rtl/string.h\"\n";
    }
    if (m_includeRtlTextencH) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << "#include \"rtl/textenc.h\"\n";
    }
    if (m_includeRtlUstrbufHxx) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << ("#include \"rtl/ustrbuf.hxx\"\n");
    }
    if (m_includeRtlUstringH) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << "#include \"rtl/ustring.h\"\n";
    }
    if (m_includeRtlUstringHxx) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << ("#include \"rtl/ustring.hxx\"\n");
    }
    if (m_includeRtlInstanceHxx) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << "#include \"rtl/instance.hxx\"\n";
    }
    if (m_includeSalTypesH) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << "#include \"sal/types.h\"\n";
    }
    if (m_includeTypelibTypeclassH) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << ("#include \"typelib/typeclass.h\"\n");
    }
    if (m_includeTypelibTypedescriptionH) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << ("#include \"typelib/typedescription.h\"\n");
    }
}

void Includes::dumpInclude(
    FileStream & out, OString const & entityName, bool hpp)
{
    out << "#include \"" << entityName.replace('.', '/') << "."
        << (hpp ? "hpp" : "hdl") << "\"\n";
}

bool Includes::isInterfaceType(OString const & entityName) const {
    return m_manager->getSort(b2u(entityName)) == UnoType::SORT_INTERFACE_TYPE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
