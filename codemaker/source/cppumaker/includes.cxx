/*************************************************************************
 *
 *  $RCSfile: includes.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 03:12:48 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
    m_includeCppuMacrosHxx(false), m_includeOslDoublecheckedlockingH(false),
    m_includeOslMutexHxx(false), m_includeRtlStrbufHxx(false),
    m_includeRtlStringH(false), m_includeRtlTextencH(false),
    m_includeRtlUstrbufHxx(false), m_includeRtlUstringH(false),
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
    out << "#ifndef _SAL_CONFIG_H_\n#include \"sal/config.h\"\n#endif\n";
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
    static char const * hxxExtensionUpper[2] = { "H", "HXX" };
    if (m_includeAny) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << "#ifndef _COM_SUN_STAR_UNO_ANY_" << hxxExtensionUpper[m_hpp]
            << "_\n#include \"com/sun/star/uno/Any." << hxxExtension[m_hpp]
            << "\"\n#endif\n";
    }
    if (m_includeReference) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << "#ifndef _COM_SUN_STAR_UNO_REFERENCE_"
            << hxxExtensionUpper[m_hpp]
            << "_\n#include \"com/sun/star/uno/Reference."
            << hxxExtension[m_hpp] << "\"\n#endif\n";
    }
    if (m_includeSequence) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << "#ifndef _COM_SUN_STAR_UNO_SEQUENCE_" << hxxExtensionUpper[m_hpp]
            << "_\n#include \"com/sun/star/uno/Sequence." << hxxExtension[m_hpp]
            << "\"\n#endif\n";
    }
    if (m_includeType) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << "#ifndef _COM_SUN_STAR_UNO_TYPE_" << hxxExtensionUpper[m_hpp]
            << "_\n#include \"com/sun/star/uno/Type." << hxxExtension[m_hpp]
            << "\"\n#endif\n";
    }
    if (m_includeCppuMacrosHxx) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << ("#ifndef _CPPU_MACROS_HXX_\n"
                "#include \"cppu/macros.hxx\"\n#endif\n");
    }
    if (m_includeOslDoublecheckedlockingH) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << ("#ifndef INCLUDED_OSL_DOUBLECHECKEDLOCKING_H\n"
                "#include \"osl/doublecheckedlocking.h\"\n#endif\n");
    }
    if (m_includeOslMutexHxx) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << "#ifndef _OSL_MUTEX_HXX_\n#include \"osl/mutex.hxx\"\n#endif\n";
    }
    if (m_includeRtlStrbufHxx) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << ("#ifndef _RTL_STRBUF_HXX_\n"
                "#include \"rtl/strbuf.hxx\"\n#endif\n");
    }
    if (m_includeRtlStringH) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << "#ifndef _RTL_STRING_H_\n#include \"rtl/string.h\"\n#endif\n";
    }
    if (m_includeRtlTextencH) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << "#ifndef _RTL_TEXTENC_H\n#include \"rtl/textenc.h\"\n#endif\n";
    }
    if (m_includeRtlUstrbufHxx) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << ("#ifndef _RTL_USTRBUF_HXX_\n"
                "#include \"rtl/ustrbuf.hxx\"\n#endif\n");
    }
    if (m_includeRtlUstringH) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << "#ifndef _RTL_USTRING_H_\n#include \"rtl/ustring.h\"\n#endif\n";
    }
    if (m_includeRtlUstringHxx) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << ("#ifndef _RTL_USTRING_HXX_\n"
                "#include \"rtl/ustring.hxx\"\n#endif\n");
    }
    if (m_includeSalTypesH) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << "#ifndef _SAL_TYPES_H_\n#include \"sal/types.h\"\n#endif\n";
    }
    if (m_includeTypelibTypeclassH) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << ("#ifndef _TYPELIB_TYPECLASS_H_\n"
                "#include \"typelib/typeclass.h\"\n#endif\n");
    }
    if (m_includeTypelibTypedescriptionH) {
        dumpEmptyLineBeforeFirst(out, &first);
        out << ("#ifndef _TYPELIB_TYPEDESCRIPTION_H_\n"
                "#include \"typelib/typedescription.h\"\n#endif\n");
    }
}

void Includes::dumpInclude(
    FileStream & out, rtl::OString const & registryType, bool hpp,
    rtl::OString const & suffix)
{
    static char const * extension[2] = { "hdl", "hpp" };
    static char const * extensionUpper[2] = { "HDL", "HPP" };
    out << "#ifndef _" << registryType.replace('/', '_').toAsciiUpperCase();
    if (suffix.getLength() > 0) {
        out << "_" << suffix.toAsciiUpperCase();
    }
    out << "_" <<  extensionUpper[hpp] << "_\n#include \""
        << registryType;
    if (suffix.getLength() > 0) {
        out << "/" << suffix;
    }
    out << "." << extension[hpp] << "\"\n#endif\n";
}

bool Includes::isInterfaceType(rtl::OString const & registryType) const {
    return m_manager.getTypeClass(registryType) == RT_TYPE_INTERFACE;
}
