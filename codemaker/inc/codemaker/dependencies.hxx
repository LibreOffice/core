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

#ifndef INCLUDED_CODEMAKER_DEPENDENCIES_HXX
#define INCLUDED_CODEMAKER_DEPENDENCIES_HXX

#include "sal/config.h"

#include <map>

#include "rtl/ref.hxx"
#include "rtl/string.hxx"
#include "rtl/ustring.hxx"

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

    typedef std::map< OString, Kind > Map;

    /**
       Constructs the dependencies for a given type.

       <p>If the given type is not successfully available at the given type
       manager, <code>isValid()</code> will return <code>false</code>.</p>

       @param manager a type manager, to obtain information about the given type

       @param type the UNO type registry name of an enum type, plain struct
       type, polymorphic struct type template, exception type, interface type,
       typedef, module, constant group, service, or singleton
     */
    Dependencies(
        rtl::Reference< TypeManager > const & manager,
        OString const & type);

    ~Dependencies();

    /**
       Add a special dependency (which is not obvious from the type's data
       available at the type manager).

       @param type a UNO type registry name
     */
    void add(OString const & type) { insert(type, false); }

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

    void insert(OUString const & type, bool base);

    void insert(OString const & type, bool base);

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
