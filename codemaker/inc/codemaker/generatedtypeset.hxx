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

#ifndef INCLUDED_CODEMAKER_GENERATEDTYPESET_HXX
#define INCLUDED_CODEMAKER_GENERATEDTYPESET_HXX

#include "rtl/string.hxx"

#include <hash_set>

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
    void add(rtl::OString const & type) { m_set.insert(type); }

    /**
       Checks whether a given type has already been generated.

       @param type a UNO type registry name

       @return true iff the given type has already been generated
     */
    bool contains(rtl::OString const & type) const
    { return m_set.find(type) != m_set.end(); }

private:
    GeneratedTypeSet(GeneratedTypeSet &); // not implemented
    void operator =(GeneratedTypeSet); // not implemented

    std::hash_set< rtl::OString, rtl::OStringHash > m_set;
};

}

#endif // INCLUDED_CODEMAKER_GENERATEDTYPESET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
