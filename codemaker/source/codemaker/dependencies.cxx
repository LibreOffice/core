/*************************************************************************
 *
 *  $RCSfile: dependencies.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-23 14:45:24 $
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

#include "codemaker/dependencies.hxx"

#include "codemaker/typemanager.hxx"
#include "codemaker/unotype.hxx"

#include "osl/diagnose.h"
#include "registry/reader.hxx"
#include "rtl/string.hxx"
#include "rtl/textcvt.h"
#include "rtl/textenc.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include <hash_map>
#include <vector>

using codemaker::Dependencies;

namespace {

struct Bad {};

}

Dependencies::Dependencies(
    TypeManager const & manager, rtl::OString const & type):
    m_voidDependency(false), m_booleanDependency(false),
    m_byteDependency(false), m_shortDependency(false),
    m_unsignedShortDependency(false), m_longDependency(false),
    m_unsignedLongDependency(false), m_hyperDependency(false),
    m_unsignedHyperDependency(false), m_floatDependency(false),
    m_doubleDependency(false), m_charDependency(false),
    m_stringDependency(false), m_typeDependency(false), m_anyDependency(false),
    m_sequenceDependency(false)
{
    typereg::Reader reader(manager.getTypeReader(type));
    m_valid = reader.isValid();
    if (m_valid) {
        // Not everything is checked for consistency, just things that are cheap
        // to test:
        try {
            RTTypeClass tc = reader.getTypeClass();
            if (tc != RT_TYPE_SERVICE) {
                for (sal_Int16 i = 0; i < reader.getSuperTypeCount(); ++i) {
                    insert(reader.getSuperTypeName(i), true);
                }
            }
            if (tc != RT_TYPE_ENUM) {
                {for (sal_Int16 i = 0; i < reader.getFieldCount(); ++i) {
                    if ((reader.getFieldFlags(i) & RT_ACCESS_PARAMETERIZED_TYPE)
                        == 0)
                    {
                        insert(reader.getFieldTypeName(i), false);
                    }
                }}
            }
            for (sal_Int16 i = 0; i < reader.getMethodCount(); ++i) {
                insert(reader.getMethodReturnTypeName(i), false);
                for (sal_Int16 j = 0; j < reader.getMethodParameterCount(i);
                      ++j)
                {
                    if ((reader.getMethodParameterFlags(i, j) & RT_PARAM_REST)
                        != 0)
                    {
                        m_sequenceDependency = true;
                    }
                    insert(reader.getMethodParameterTypeName(i, j), false);
                }
                for (sal_Int16 j = 0; j < reader.getMethodExceptionCount(i);
                      ++j)
                {
                    insert(reader.getMethodExceptionTypeName(i, j), false);
                }
            }
            for (sal_Int16 i = 0; i < reader.getReferenceCount(); ++i) {
                if (reader.getReferenceSort(i) != RT_REF_TYPE_PARAMETER) {
                    insert(reader.getReferenceTypeName(i), false);
                }
            }
        } catch (Bad &) {
            m_map.clear();
            m_valid = false;
            m_voidDependency = false;
            m_booleanDependency = false;
            m_byteDependency = false;
            m_shortDependency = false;
            m_unsignedShortDependency = false;
            m_longDependency = false;
            m_unsignedLongDependency = false;
            m_hyperDependency = false;
            m_unsignedHyperDependency = false;
            m_floatDependency = false;
            m_doubleDependency = false;
            m_charDependency = false;
            m_stringDependency = false;
            m_typeDependency = false;
            m_anyDependency = false;
            m_sequenceDependency = false;
        }
    }
}

Dependencies::~Dependencies()
{}

void Dependencies::insert(rtl::OUString const & type, bool base) {
    rtl::OString t;
    if (!type.convertToString(
            &t, RTL_TEXTENCODING_UTF8,
            (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
             | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR)))
    {
        throw Bad();
    }
    insert(t, base);
}

void Dependencies::insert(rtl::OString const & type, bool base) {
    sal_Int32 rank;
    std::vector< rtl::OString > args;
    rtl::OString t(UnoType::decompose(type, &rank, &args));
    if (rank > 0) {
        m_sequenceDependency = true;
    }
    switch (UnoType::getSort(t)) {
    case UnoType::SORT_VOID:
        if (rank != 0 || !args.empty()) {
            throw Bad();
        }
        m_voidDependency = true;
        break;

    case UnoType::SORT_BOOLEAN:
        if (!args.empty()) {
            throw Bad();
        }
        m_booleanDependency = true;
        break;

    case UnoType::SORT_BYTE:
        if (!args.empty()) {
            throw Bad();
        }
        m_byteDependency = true;
        break;

    case UnoType::SORT_SHORT:
        if (!args.empty()) {
            throw Bad();
        }
        m_shortDependency = true;
        break;

    case UnoType::SORT_UNSIGNED_SHORT:
        if (!args.empty()) {
            throw Bad();
        }
        m_unsignedShortDependency = true;
        break;

    case UnoType::SORT_LONG:
        if (!args.empty()) {
            throw Bad();
        }
        m_longDependency = true;
        break;

    case UnoType::SORT_UNSIGNED_LONG:
        if (!args.empty()) {
            throw Bad();
        }
        m_unsignedLongDependency = true;
        break;

    case UnoType::SORT_HYPER:
        if (!args.empty()) {
            throw Bad();
        }
        m_hyperDependency = true;
        break;

    case UnoType::SORT_UNSIGNED_HYPER:
        if (!args.empty()) {
            throw Bad();
        }
        m_unsignedHyperDependency = true;
        break;

    case UnoType::SORT_FLOAT:
        if (!args.empty()) {
            throw Bad();
        }
        m_floatDependency = true;
        break;

    case UnoType::SORT_DOUBLE:
        if (!args.empty()) {
            throw Bad();
        }
        m_doubleDependency = true;
        break;

    case UnoType::SORT_CHAR:
        if (!args.empty()) {
            throw Bad();
        }
        m_charDependency = true;
        break;

    case UnoType::SORT_STRING:
        if (!args.empty()) {
            throw Bad();
        }
        m_stringDependency = true;
        break;

    case UnoType::SORT_TYPE:
        if (!args.empty()) {
            throw Bad();
        }
        m_typeDependency = true;
        break;

    case UnoType::SORT_ANY:
        if (!args.empty()) {
            throw Bad();
        }
        m_anyDependency = true;
        break;

    case UnoType::SORT_COMPLEX:
        {
            {for (std::vector< rtl::OString >::iterator i(args.begin());
                  i != args.end(); ++i)
            {
                insert(*i, false);
            }}
            Map::iterator i(m_map.find(t));
            if (i == m_map.end()) {
                m_map.insert(
                    Map::value_type(t, base ? KIND_BASE : KIND_NO_BASE));
            } else if (base) {
                i->second = KIND_BASE;
            }
            break;
        }

    default:
        OSL_ASSERT(false);
        break;
    }
}
