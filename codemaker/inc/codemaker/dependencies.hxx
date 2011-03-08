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

#ifndef INCLUDED_CODEMAKER_DEPENDENCIES_HXX
#define INCLUDED_CODEMAKER_DEPENDENCIES_HXX

#include "rtl/string.hxx"

#include <hash_map>

namespace rtl { class OUString; }
class TypeManager;

/// @HTML

namespace codemaker {

/**
   A simple class to track which other types a given type depends on.

   <p>This class is not multi-thread&ndash;safe.</p>
 */
class Dependencies {
public:
    /**
       Flags to distinguish whether or not one type depends on another type
       because the second is a direct base of the first.
     */
    enum Kind { KIND_NO_BASE, KIND_BASE };

    typedef std::hash_map< rtl::OString, Kind, rtl::OStringHash > Map;

    /**
       Constructs the dependencies for a given type.

       <p>If the given type is not successfully available at the given type
       manager, <code>isValid()</code> will return <code>false</code>.</p>

       @param manager a type manager, to obtain information about the given type

       @param type the UNO type registry name of an enum type, plain struct
       type, polymorphic struct type template, exception type, interface type,
       typedef, module, constant group, service, or singleton
     */
    Dependencies(TypeManager const & manager, rtl::OString const & type);

    ~Dependencies();

    /**
       Add a special dependency (which is not obvious from the type's data
       available at the type manager).

       @param type a UNO type registry name
     */
    void add(rtl::OString const & type) { insert(type, false); }

    bool isValid() const { return m_valid; }

    Map const & getMap() const { return m_map; }

    bool hasVoidDependency() const { return m_voidDependency; }

    bool hasBooleanDependency() const { return m_booleanDependency; }

    bool hasByteDependency() const { return m_byteDependency; }

    bool hasShortDependency() const { return m_shortDependency; }

    bool hasUnsignedShortDependency() const
    { return m_unsignedShortDependency; }

    bool hasLongDependency() const { return m_longDependency; }

    bool hasUnsignedLongDependency() const { return m_unsignedLongDependency; }

    bool hasHyperDependency() const { return m_hyperDependency; }

    bool hasUnsignedHyperDependency() const
    { return m_unsignedHyperDependency; }

    bool hasFloatDependency() const { return m_floatDependency; }

    bool hasDoubleDependency() const { return m_doubleDependency; }

    bool hasCharDependency() const { return m_charDependency; }

    bool hasStringDependency() const { return m_stringDependency; }

    bool hasTypeDependency() const { return m_typeDependency; }

    bool hasAnyDependency() const { return m_anyDependency; }

    bool hasSequenceDependency() const { return m_sequenceDependency; }

private:
    Dependencies(Dependencies &); // not implemented
    void operator =(Dependencies); // not implemented

    void insert(rtl::OUString const & type, bool base);

    void insert(rtl::OString const & type, bool base);

    Map m_map;
    bool m_valid;
    bool m_voidDependency;
    bool m_booleanDependency;
    bool m_byteDependency;
    bool m_shortDependency;
    bool m_unsignedShortDependency;
    bool m_longDependency;
    bool m_unsignedLongDependency;
    bool m_hyperDependency;
    bool m_unsignedHyperDependency;
    bool m_floatDependency;
    bool m_doubleDependency;
    bool m_charDependency;
    bool m_stringDependency;
    bool m_typeDependency;
    bool m_anyDependency;
    bool m_sequenceDependency;
};

}

#endif // INCLUDED_CODEMAKER_DEPENDENCIES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
