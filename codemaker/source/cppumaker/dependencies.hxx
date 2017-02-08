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

#ifndef INCLUDED_CODEMAKER_SOURCE_CPPUMAKER_DEPENDENCIES_HXX
#define INCLUDED_CODEMAKER_SOURCE_CPPUMAKER_DEPENDENCIES_HXX

#include "sal/config.h"

#include <map>

#include "rtl/ref.hxx"

namespace rtl { class OUString; }
class TypeManager;

/// @HTML

namespace codemaker { namespace cppumaker {

/**
   A simple class to track which other entites a given entity depends on.

   <p>This class is not multi-thread&ndash;safe.</p>
 */
class Dependencies {
public:
    /**
       Flags to distinguish whether one entity depends on another entity because
       the second is a direct base of the first or an exception thrown by the
       first.
     */
    enum Kind { KIND_NORMAL, KIND_BASE, KIND_EXCEPTION };

    typedef std::map< rtl::OUString, Kind > Map;

    /**
       Constructs the dependencies for a given entity.

       @param manager a type manager, to obtain information about the given
       entity; must not be null

       @param name the UNOIDL name of an enum type, plain struct type,
       polymorphic struct type template, exception type, interface type,
       typedef, constant group, single-interface--based service, or
       interface-based singleton entity
     */
    Dependencies(
        rtl::Reference< TypeManager > const & manager,
        rtl::OUString const & name);

    ~Dependencies();

    Dependencies(const Dependencies&) = delete;
    const Dependencies& operator=(const Dependencies&) = delete;

    Map const & getMap() const { return m_map; }

    bool hasBooleanDependency() const { return m_booleanDependency; }

    bool hasByteDependency() const { return m_byteDependency; }

    bool hasShortDependency() const { return m_shortDependency; }

    bool hasUnsignedShortDependency() const
    { return m_unsignedShortDependency; }

    bool hasLongDependency() const { return m_longDependency; }

    bool hasHyperDependency() const { return m_hyperDependency; }

    bool hasUnsignedHyperDependency() const
    { return m_unsignedHyperDependency; }

    bool hasCharDependency() const { return m_charDependency; }

    bool hasStringDependency() const { return m_stringDependency; }

    bool hasTypeDependency() const { return m_typeDependency; }

    bool hasAnyDependency() const { return m_anyDependency; }

    bool hasSequenceDependency() const { return m_sequenceDependency; }

private:
    void insert(rtl::OUString const & name, Kind kind);

    rtl::Reference< TypeManager > m_manager;
    Map m_map;
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

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
