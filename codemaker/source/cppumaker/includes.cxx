/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_codemaker.hxx"

#include "includes.hxx"

#include "dumputils.hxx"

#include "codemaker/dependencies.hxx"
#include "codemaker/global.hxx"
#include "codemaker/typemanager.hxx"
#include "codemaker/unotype.hxx"

#include "osl/diagnose.h"
#include "rtl/string.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include <vector>

using codemaker::cppumaker::Includes;

Includes::Includes(
    TypeManager const & manager, codemaker::Dependencies const & dependencies,
    bool hpp):
    m_manager(manager), m_map(dependencies.getMap()), m_hpp(hpp),
    m_includeAny(dependencies.hasAnyDependency()), m_includeReference(false),
    m_includeSequence(dependencies.hasSequenceDependency()),
    m_includeType(dependencies.hasTypeDependency()),
    m_includeCppuMacrosHxx(false), m_includeCppuUnotypeHxx(false),
    m_includeOslDoublecheckedlockingH(false), m_includeOslMutexHxx(false),
    m_includeRtlStrbufHxx(false), m_includeRtlStringH(false),
    m_includeRtlTextencH(false), m_includeRtlUstrbufHxx(false),
    m_includeRtlUstringH(false),
    m_includeRtlUstringHxx(dependencies.hasStringDependency()),
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

void Includes::add(rtl::OString const & registryType) {
    sal_Int32 rank;
    std::vector< rtl::OString > args;
    rtl::OString type(
        codemaker::UnoType::decompose(registryType, &rank, &args));
    if (rank > 0) {
        m_includeSequence = true;
    }
    switch (codemaker::UnoType::getSort(type)) {
    case codemaker::UnoType::SORT_VOID:
        OSL_ASSERT(args.empty());
        OSL_ASSERT(false);
        break;

    case codemaker::UnoType::SORT_BOOLEAN:
    case codemaker::UnoType::SORT_BYTE:
    case codemaker::UnoType::SORT_SHORT:
    case codemaker::UnoType::SORT_UNSIGNED_SHORT:
    case codemaker::UnoType::SORT_LONG:
    case codemaker::UnoType::SORT_UNSIGNED_LONG:
    case codemaker::UnoType::SORT_HYPER:
    case codemaker::UnoType::SORT_UNSIGNED_HYPER:
    case codemaker::UnoType::SORT_CHAR:
        OSL_ASSERT(args.empty());
        m_includeSalTypesH = true;
        break;

    case codemaker::UnoType::SORT_FLOAT:
    case codemaker::UnoType::SORT_DOUBLE:
        OSL_ASSERT(args.empty());
        break;

    case codemaker::UnoType::SORT_STRING:
        OSL_ASSERT(args.empty());
        m_includeRtlUstringHxx = true;
        break;

    case codemaker::UnoType::SORT_TYPE:
        OSL_ASSERT(args.empty());
        m_includeType = true;
        break;

    case codemaker::UnoType::SORT_ANY:
        OSL_ASSERT(args.empty());
        m_includeAny = true;
        break;

    case codemaker::UnoType::SORT_COMPLEX:
        m_map.insert(
            codemaker::Dependencies::Map::value_type(
                type, codemaker::Dependencies::KIND_NO_BASE));
        {for (std::vector< rtl::OString >::iterator i(args.begin());
              i != args.end(); ++i)
        {
            add(*i);
        }}
        break;

    default:
        OSL_ASSERT(false);
        break;
    }
}

namespace {

void dumpEmptyLineBeforeFirst(FileStream & out, bool * first) {
    OSL_ASSERT(first != 0);
    if (*first) {
        out << "\n";
        *first = false;
    }
}

}

void Includes::dump(FileStream & out, rtl::OString const * companionHdl) {
    OSL_ASSERT(companionHdl == 0 || m_hpp);
    if (!m_includeReference) {
        for (codemaker::Dependencies::Map::iterator i(m_map.begin());
             i != m_map.end(); ++i)
        {
            if (isInterfaceType(i->first)) {
                m_includeReference = true;
                break;
            }
        }
    }
    out << "#include \"sal/config.h\"\n";
    if (companionHdl) {
        out << "\n";
        dumpInclude(out, *companionHdl, false);
    }
    bool first = true;
    for (codemaker::Dependencies::Map::iterator i(m_map.begin());
         i != m_map.end(); ++i)
    {
        dumpEmptyLineBeforeFirst(out, &first);
        if (m_hpp || i->second == codemaker::Dependencies::KIND_BASE
            || !isInterfaceType(i->first))
        {
            dumpInclude(out, i->first, m_hpp);
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
        out << "#include \"com/sun/star/uno/Any." << hxxExtension[m_hpp] << "\"\n";
    }
    if (m_includeReference) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << "#include \"com/sun/star/uno/Reference." << hxxExtension[m_hpp] << "\"\n";
    }
    if (m_includeSequence) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << "#include \"com/sun/star/uno/Sequence." << hxxExtension[m_hpp] << "\"\n";
    }
    if (m_includeType) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << "#include \"com/sun/star/uno/Type." << hxxExtension[m_hpp] << "\"\n";
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
    FileStream & out, rtl::OString const & registryType, bool hpp,
    rtl::OString const & suffix)
{
    static char const * extension[2] = { "hdl", "hpp" };
    out << "#include \"" << registryType;
    if (suffix.getLength() > 0) {
        out << "/" << suffix;
    }
    out << "." << extension[hpp] << "\"\n";
}

bool Includes::isInterfaceType(rtl::OString const & registryType) const {
    return m_manager.getTypeClass(registryType) == RT_TYPE_INTERFACE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
