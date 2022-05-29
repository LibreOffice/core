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

#pragma once

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <drawinglayer/primitive2d/CommonTypes.hxx>
#include <drawinglayer/primitive2d/Primitive2DVisitor.hxx>

#include <basegfx/range/b2drange.hxx>
#include <array>
#include <deque>
#include <memory>

namespace drawinglayer::geometry
{
class ViewInformation2D;
}

namespace drawinglayer::primitive2d
{
/**
  * Container data structure for 2d primitives.
  *
  * It is designed around a particular set of constraints:
  * (1) No heap allocation for zero-sized container, since that is surprisingly common
  * (2) We only care about appending, and forward-iterating the list, we don't delete at all
  * (3) Minimize the number of allocations, since that the the dominant cost
  * (4) Cache friendly for forward-traversal
  * (5) Most of the collections are very small ( < 8 elements), but a few of them are huge
  *
  * So what I came up with a kind of segmented array / linked list of arrays.
  * (1) We have a linked list of blocks,
  *   where the blocks are variable-sized, which means when we know we're going to append
  *   a lot of entries, we can allocated (i.e. reserve) a single large block.
  * (2) But by default we allocate nothing.
  * (3) When we append to a zero-sized container, we only allocate a small block,
  *   which reduces wasted space
  * (4) When we append from a std::move'd container to this container, we can just
  *   splice the list which avoids both heap allocation and memcpy.
  *
  */
class SAL_WARN_UNUSED DRAWINGLAYERCORE_DLLPUBLIC Primitive2DContainer final
    : public Primitive2DDecompositionVisitor
{
    struct Block
    {
        sal_Int32 mnAllocated = 0; /// number of allocated elements in the maElements array
        sal_Int32 mnOccupied = 0; /// number of used elements
        std::unique_ptr<Block> mxNext; /// next Block in linked list
        Primitive2DReference maElements[1];
    };

public:
    // use zero because we allocate a lot of empty containers
    explicit Primitive2DContainer() {}
    Primitive2DContainer(const Primitive2DContainer& other);
    Primitive2DContainer(Primitive2DContainer&& other) noexcept
    {
        std::swap(mxFirst, other.mxFirst);
        std::swap(mpLast, other.mpLast);
    }
    Primitive2DContainer(std::initializer_list<Primitive2DReference>)
    {
        assert(false); // TODO
    }
    Primitive2DContainer(
        const css::uno::Sequence<css::uno::Reference<css::graphic::XPrimitive2D>>&);
    Primitive2DContainer(const std::deque<css::uno::Reference<css::graphic::XPrimitive2D>>&);

    virtual ~Primitive2DContainer() override;

    virtual void visit(const Primitive2DReference& rSource) override { append(rSource); }
    virtual void visit(const Primitive2DContainer& rSource) override { append(rSource); }
    virtual void visit(Primitive2DContainer&& rSource) override { append(std::move(rSource)); }

    Primitive2DContainer& operator=(const Primitive2DContainer& r);
    Primitive2DContainer& operator=(Primitive2DContainer&& other) noexcept
    {
        std::swap(mxFirst, other.mxFirst);
        std::swap(mpLast, other.mpLast);
        return *this;
    }

    bool operator==(const Primitive2DContainer& rB) const;
    bool operator!=(const Primitive2DContainer& rB) const { return !operator==(rB); }

    basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& aViewInformation) const;

    Primitive2DContainer maybeInvert(bool bInvert = false);

    css::uno::Sequence<css::uno::Reference<css::graphic::XPrimitive2D>> toSequence() const;

    struct const_iterator
    {
        typedef const_iterator self_type;
        typedef const Primitive2DReference value_type;
        typedef std::input_iterator_tag iterator_category;
        typedef std::ptrdiff_t difference_type;
        typedef const Primitive2DReference* pointer;
        typedef const Primitive2DReference& reference;

        const_iterator(const Block* pBlock, sal_Int32 nIndex)
            : mpBlock(pBlock)
            , mnIndex(nIndex)
        {
        }
        bool operator!=(const const_iterator& other) const
        {
            return !(mpBlock == other.mpBlock && mnIndex == other.mnIndex);
        }
        const_iterator& operator++();
        const Primitive2DReference& operator*() const { return mpBlock->maElements[mnIndex]; }

    private:
        const Block* mpBlock;
        sal_Int32 mnIndex = 0;
    };
    struct iterator
    {
        typedef iterator self_type;
        typedef Primitive2DReference value_type;
        typedef std::input_iterator_tag iterator_category;
        typedef std::ptrdiff_t difference_type;
        typedef Primitive2DReference* pointer;
        typedef Primitive2DReference& reference;

        iterator(Block* pBlock, sal_Int32 nIndex)
            : mpBlock(pBlock)
            , mnIndex(nIndex)
        {
        }
        bool operator!=(const iterator& other) const
        {
            return !(mpBlock == other.mpBlock && mnIndex == other.mnIndex);
        }
        iterator& operator++();
        Primitive2DReference& operator*() const { return mpBlock->maElements[mnIndex]; }

    private:
        Block* mpBlock;
        sal_Int32 mnIndex = 0;
    };

    const_iterator begin() const { return const_iterator(mxFirst.get(), 0); }
    const_iterator end() const { return const_iterator(mpLast, mpLast ? mpLast->mnOccupied : 0); }
    iterator begin() { return iterator(mxFirst.get(), 0); }
    iterator end() { return iterator(mpLast, mpLast ? mpLast->mnOccupied : 0); }

    const Primitive2DReference& front() const { return mxFirst->maElements[0]; }
    Primitive2DReference& front() { return mxFirst->maElements[0]; }
    const Primitive2DReference& back() const { return mpLast->maElements[mpLast->mnOccupied - 1]; }
    Primitive2DReference& back() { return mpLast->maElements[mpLast->mnOccupied - 1]; }
    bool empty() const { return bool(mxFirst); }
    sal_Int32 size() const { return mnSize; }

    void clear();
    void insert_front(const Primitive2DReference& r);
    void append(const Primitive2DReference&);
    void append(const Primitive2DContainer& rSource);
    void append(Primitive2DContainer&& rSource);
    /** If this is empty, will size the initial block to the given number of elements */
    void reserve(sal_Int32 nElements);

    // no operator[] because we don't do cheap random access
    const Primitive2DReference& getChild(sal_Int32 idx) const;

private:
    static std::unique_ptr<Block> makeBlock(sal_Int32 nElements);

    std::unique_ptr<Block> mxFirst;
    Block* mpLast = nullptr;
    sal_Int32 mnSize = 0;
};

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
