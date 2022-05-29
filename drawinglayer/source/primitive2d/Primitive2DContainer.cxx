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
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>

using namespace css;

namespace drawinglayer::primitive2d
{
std::unique_ptr<Primitive2DContainer::Block> Primitive2DContainer::makeBlock(sal_Int32 nElements)
{
    size_t nBytes = sizeof(Block) + sizeof(Primitive2DReference) * (nElements - 1);
    return std::unique_ptr<Primitive2DContainer::Block>(static_cast<Block*>(malloc(nBytes)));
}

Primitive2DContainer::Primitive2DContainer(
    const css::uno::Sequence<css::uno::Reference<css::graphic::XPrimitive2D>>& rSource)
{
    for (const auto& rPrimitive : rSource)
        append(static_cast<const UnoPrimitive2D*>(rPrimitive.get())->getBasePrimitive2D());
}

Primitive2DContainer::Primitive2DContainer(
    const std::deque<css::uno::Reference<css::graphic::XPrimitive2D>>& rSource)
{
    for (const auto& rPrimitive : rSource)
        append(static_cast<const UnoPrimitive2D*>(rPrimitive.get())->getBasePrimitive2D());
}

Primitive2DContainer::Primitive2DContainer(const Primitive2DContainer& other) { operator=(other); }

css::uno::Sequence<css::uno::Reference<css::graphic::XPrimitive2D>>
Primitive2DContainer::toSequence() const
{
    css::uno::Sequence<css::uno::Reference<css::graphic::XPrimitive2D>> aVal(size());
    auto p = aVal.getArray();
    for (const auto& rPrimitive : *this)
    {
        *p = new UnoPrimitive2D(rPrimitive);
        ++p;
    }
    return aVal;
}

Primitive2DContainer Primitive2DContainer::maybeInvert(bool bInvert)
{
    if (bInvert)
    {
        // invert each block, and reverse the order of the blocks
        Primitive2DContainer rv;
        rv.mnSize = mnSize;
        while (mxFirst)
        {
            std::reverse(mxFirst->maElements, mxFirst->maElements + mxFirst->mnOccupied);
            std::unique_ptr<Block> xSecond = std::move(mxFirst->mxNext);
            mxFirst->mxNext = std::move(rv.mxFirst);
            rv.mxFirst = std::move(mxFirst);
            mxFirst = std::move(xSecond);
        }
        mpLast = nullptr;
        mnSize = 0;
        return rv;
    }
    else
    {
        return std::move(*this);
    }
}

// get B2DRange from a given Primitive2DSequence
basegfx::B2DRange
Primitive2DContainer::getB2DRange(const geometry::ViewInformation2D& aViewInformation) const
{
    basegfx::B2DRange aRetval;

    for (auto const& r : *this)
    {
        aRetval.expand(getB2DRangeFromPrimitive2DReference(r, aViewInformation));
    }

    return aRetval;
}

bool Primitive2DContainer::operator==(const Primitive2DContainer& other) const
{
    if (mnSize != other.mnSize)
        return false;
    auto it1 = begin();
    auto it2 = other.begin();
    while (it1 != end())
    {
        if (!arePrimitive2DReferencesEqual(*it1, *it2))
            return false;
        ++it1;
        ++it2;
    }
    return true;
}

Primitive2DContainer::~Primitive2DContainer() {}

void Primitive2DContainer::append(const Primitive2DReference& rSource)
{
    static constexpr sal_Int32 DEFAULT_NO_ELEMENTS = 16; // this is a guess
    if (!mpLast)
    {
        assert(!mxFirst);
        mxFirst = makeBlock(DEFAULT_NO_ELEMENTS);
        mpLast = mxFirst.get();
        mpLast->mnAllocated = DEFAULT_NO_ELEMENTS - 1;
    }
    else if (mpLast->mnOccupied >= mpLast->mnAllocated)
    {
        assert(!mpLast->mxNext);
        mpLast->mxNext = makeBlock(DEFAULT_NO_ELEMENTS);
        mpLast = mpLast->mxNext.get();
        mpLast->mnAllocated = DEFAULT_NO_ELEMENTS - 1;
    }
    mpLast->maElements[mpLast->mnOccupied] = rSource;
    ++mpLast->mnOccupied;
    ++mnSize;
}

void Primitive2DContainer::append(const Primitive2DContainer& other)
{
    if (mpLast && (mpLast->mnAllocated - mpLast->mnOccupied) < other.mnSize)
    {
        // If the other container has so few elements that we can fit them in our current
        // block, rather copy them over, just to prevent excessive memory usage.
        for (auto& r : other)
            append(r);
        return;
    }

    // copy block by block
    const Block* pBlock = other.mxFirst.get();
    while (pBlock)
    {
        std::unique_ptr<Block> xNewBlock = makeBlock(pBlock->mnOccupied);
        xNewBlock->mnOccupied = pBlock->mnOccupied;
        mpLast->mxNext->mnAllocated = pBlock->mnOccupied;
        std::copy(pBlock->maElements, pBlock->maElements + pBlock->mnOccupied,
                  xNewBlock->maElements);
        if (!mxFirst)
        {
            mxFirst = std::move(xNewBlock);
            mpLast = mxFirst.get();
        }
        else
        {
            mpLast->mxNext = std::move(xNewBlock);
            mpLast = mpLast->mxNext.get();
        }
        pBlock = pBlock->mxNext.get();
    }
    mnSize += other.mnSize;
}

void Primitive2DContainer::append(Primitive2DContainer&& other)
{
    if (!mpLast)
    {
        // If we don't have anything of our own, just take over the data blocks from other.
        std::swap(mxFirst, other.mxFirst);
        std::swap(mpLast, other.mpLast);
        std::swap(mnSize, other.mnSize);
    }
    else if (mpLast->mnAllocated - mpLast->mnOccupied < other.mnSize)
    {
        // If the other container has so few elements that we can fit them in our current
        // block, rather copy them over, just to prevent excessive memory usage.
        for (auto& r : other)
            append(std::move(r));
    }
    else
    {
        // link the data blocks from other to the end of our own list
        mpLast->mxNext = std::move(other.mxFirst);
        mpLast = mpLast->mxNext.get();
        mnSize += other.mnSize;
        other.mpLast = nullptr;
        other.mnSize = 0;
    }
}

void Primitive2DContainer::reserve(sal_Int32 nSize)
{
    if (mxFirst)
        return;
    mxFirst.reset(
        static_cast<Block*>(malloc(sizeof(Block) + sizeof(Primitive2DReference) * (nSize - 1))));
    mpLast = mxFirst.get();
    mpLast->mnAllocated = nSize;
}

void Primitive2DContainer::clear()
{
    mxFirst.reset();
    mpLast = nullptr;
    mnSize = 0;
}

Primitive2DContainer::const_iterator& Primitive2DContainer::const_iterator::operator++()
{
    assert(mpBlock);
    ++mnIndex;
    if (mnIndex >= mpBlock->mnOccupied)
    {
        mpBlock = mpBlock->mxNext.get();
        mnIndex = 0;
    }
    return *this;
}

Primitive2DContainer::iterator& Primitive2DContainer::iterator::operator++()
{
    assert(mpBlock);
    ++mnIndex;
    if (mnIndex >= mpBlock->mnOccupied)
    {
        mpBlock = mpBlock->mxNext.get();
        mnIndex = 0;
    }
    return *this;
}

Primitive2DContainer& Primitive2DContainer::operator=(const Primitive2DContainer& other)
{
    clear();
    Block* pBlock = other.mxFirst.get();
    while (pBlock)
    {
        std::unique_ptr<Block> xNewBlock = makeBlock(pBlock->mnOccupied);
        xNewBlock->mnOccupied = pBlock->mnOccupied;
        mpLast->mxNext->mnAllocated = pBlock->mnOccupied;
        std::copy(pBlock->maElements, pBlock->maElements + pBlock->mnOccupied,
                  xNewBlock->maElements);
        if (!mxFirst)
        {
            mxFirst = std::move(xNewBlock);
            mpLast = mxFirst.get();
        }
        else
        {
            mpLast->mxNext = std::move(xNewBlock);
            mpLast = mpLast->mxNext.get();
        }
        pBlock = pBlock->mxNext.get();
    }
    mnSize = other.mnSize;
    return *this;
}

void Primitive2DContainer::insert_front(const Primitive2DReference& r)
{
    if (empty())
    {
        append(r);
        return;
    }
    if (mxFirst && mxFirst->mnOccupied < mxFirst->mnAllocated)
    {
        std::move(mxFirst->maElements, mxFirst->maElements + mxFirst->mnOccupied,
                  mxFirst->maElements + 1);
        mxFirst->maElements[0] = r;
        ++mnSize;
        return;
    }
    // No space in first block - just allocate a new small block and put it in front. This is a very rare
    // operation, so no need to make it efficient.
    std::unique_ptr<Block> xNewBlock = makeBlock(1);
    xNewBlock->maElements[0] = r;
    xNewBlock->mnOccupied = 1;
    xNewBlock->mxNext = std::move(mxFirst);
    mxFirst = std::move(xNewBlock);
    ++mnSize;
}

const Primitive2DReference& Primitive2DContainer::getChild(sal_Int32 idx) const
{
    assert(idx >= 0);
    Block* pBlock = mxFirst.get();
    while (pBlock && idx > pBlock->mnOccupied)
    {
        idx -= pBlock->mnOccupied;
        pBlock = pBlock->mxNext.get();
    }
    return pBlock->maElements[idx];
}

UnoPrimitive2D::~UnoPrimitive2D() {}

css::uno::Sequence<::css::uno::Reference<::css::graphic::XPrimitive2D>>
    SAL_CALL UnoPrimitive2D::getDecomposition(
        const css::uno::Sequence<css::beans::PropertyValue>& rViewParameters)
{
    std::unique_lock aGuard(m_aMutex);
    return mxPrimitive->getDecomposition(rViewParameters).toSequence();
}

css::geometry::RealRectangle2D SAL_CALL
UnoPrimitive2D::getRange(const css::uno::Sequence<css::beans::PropertyValue>& rViewParameters)
{
    std::unique_lock aGuard(m_aMutex);
    return mxPrimitive->getRange(rViewParameters);
}

sal_Int64 SAL_CALL UnoPrimitive2D::estimateUsage()
{
    std::unique_lock aGuard(m_aMutex);
    return mxPrimitive->estimateUsage();
}

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
