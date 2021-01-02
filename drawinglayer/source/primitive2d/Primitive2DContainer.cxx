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

#include <sal/config.h>

#include <drawinglayer/primitive2d/Primitive2DContainer.hxx>
#include <drawinglayer/primitive2d/Tools.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>

using namespace css;

namespace drawinglayer::primitive2d
{
Primitive2DContainer Primitive2DContainer::maybeInvert(bool bInvert) const
{
    const sal_uInt32 nSize(size());
    Primitive2DContainer aRetval;

    aRetval.resize(nSize);

    for (sal_uInt32 a(0); a < nSize; a++)
    {
        aRetval[bInvert ? nSize - 1 - a : a] = (*this)[a];
    }

    // all entries taken over to Uno References as owners. To avoid
    // errors with users of this mechanism to delete pointers to BasePrimitive2D
    // itself, clear given vector
    const_cast<Primitive2DContainer&>(*this).clear();

    return aRetval;
}

// get B2DRange from a given Primitive2DSequence
basegfx::B2DRange Primitive2DContainer::getB2DRange(VisitingParameters const& rParameters) const
{
    basegfx::B2DRange aRetval;

    if (!empty())
    {
        const sal_Int32 nCount(size());

        for (sal_Int32 a(0); a < nCount; a++)
        {
            aRetval.expand(getB2DRangeFromPrimitive2DReference((*this)[a], rParameters));
        }
    }

    return aRetval;
}

bool Primitive2DContainer::operator==(const Primitive2DContainer& rB) const
{
    const bool bAHasElements(!empty());

    if (bAHasElements != !rB.empty())
    {
        return false;
    }

    if (!bAHasElements)
    {
        return true;
    }

    const size_t nCount(size());

    if (nCount != rB.size())
    {
        return false;
    }

    for (size_t a(0); a < nCount; a++)
    {
        if (!arePrimitive2DReferencesEqual((*this)[a], rB[a]))
        {
            return false;
        }
    }

    return true;
}

Primitive2DContainer::~Primitive2DContainer() {}

void Primitive2DContainer::append(const Primitive2DReference& rSource) { push_back(rSource); }

void Primitive2DContainer::append(const Primitive2DContainer& rSource)
{
    insert(end(), rSource.begin(), rSource.end());
}

void Primitive2DContainer::append(Primitive2DContainer&& rSource)
{
    this->insert(this->end(), std::make_move_iterator(rSource.begin()),
                 std::make_move_iterator(rSource.end()));
}

void Primitive2DContainer::append(const Primitive2DSequence& rSource)
{
    this->insert(this->end(), rSource.begin(), rSource.end());
}

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
