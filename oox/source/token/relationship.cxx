/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <oox/token/relationship.hxx>

#include <map>

namespace oox {

OUString getRelationship(Relationship eRelationship)
{
    static const std::map<Relationship, OUString> aMap =
    {
#include "relationship.inc"
    };

    auto itr = aMap.find(eRelationship);
    if (itr != aMap.end())
        return itr->second;

    SAL_WARN("oox", "could not find an entry for the relationship: " << static_cast<int>(eRelationship));
    return OUString();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
