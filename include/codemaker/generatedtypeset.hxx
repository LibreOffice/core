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

#ifndef INCLUDED_CODEMAKER_GENERATEDTYPESET_HXX
#define INCLUDED_CODEMAKER_GENERATEDTYPESET_HXX

#include <rtl/string.hxx>

#include <unordered_set>

/// @HTML

namespace codemaker {

/**
   A simple class to track which types have already been processed by a code
   maker.

   <p>This class is not multi-thread&ndash;safe.</p>
 */
class GeneratedTypeSet {
public:
    GeneratedTypeSet() {}

    ~GeneratedTypeSet() {}

    /**
       Add a type to the set of generated types.

       <p>If the type was already present, nothing happens.</p>

       @param type a UNO type registry name
     */
    void add(OString const & type) { m_set.insert(type); }

    /**
       Checks whether a given type has already been generated.

       @param type a UNO type registry name

       @return true iff the given type has already been generated
     */
    bool contains(OString const & type) const
    { return m_set.find(type) != m_set.end(); }

private:
    GeneratedTypeSet(GeneratedTypeSet &) = delete;
    void operator =(const GeneratedTypeSet&) = delete;

    std::unordered_set< OString, OStringHash > m_set;
};

}

#endif // INCLUDED_CODEMAKER_GENERATEDTYPESET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
