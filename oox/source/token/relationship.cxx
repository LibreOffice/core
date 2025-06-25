/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <oox/token/relationship.hxx>

#include <sal/log.hxx>
#include <frozen/bits/defines.h>
#include <frozen/bits/elsa_std.h>
#include <frozen/unordered_map.h>

namespace oox
{

namespace
{

constexpr auto constRelationshipMap = frozen::make_unordered_map<Relationship, std::u16string_view>
({
#include "relationship.inc"
});

} // end anonymous ns

OUString getRelationship(Relationship eRelationship)
{
    auto iterator = constRelationshipMap.find(eRelationship);
    if (iterator != constRelationshipMap.end())
        return OUString(iterator->second);
    SAL_WARN("oox", "could not find an entry for the relationship: " << static_cast<int>(eRelationship));
    return OUString();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
