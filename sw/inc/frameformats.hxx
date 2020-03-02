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

#include "docary.hxx"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>

// Like o3tl::find_partialorder_ptrequals
// We don't allow duplicated object entries!
struct type_name_key
    : boost::multi_index::composite_key<
          SwFrameFormat*, boost::multi_index::const_mem_fun<SwFormat, sal_uInt16, &SwFormat::Which>,
          boost::multi_index::const_mem_fun<SwFormat, const OUString&, &SwFormat::GetName>,
          boost::multi_index::identity<SwFrameFormat*> // the actual object pointer
          >
{
};

typedef boost::multi_index_container<
    SwFrameFormat*,
    boost::multi_index::indexed_by<boost::multi_index::random_access<>,
                                   boost::multi_index::ordered_unique<type_name_key>>>
    SwFrameFormatsBase;

/// Specific frame formats (frames, DrawObjects).
class SW_DLLPUBLIC SwFrameFormats : public SwFormatsBase
{
    // function updating ByName index via modify
    friend void SwFrameFormat::SetName(const OUString&, bool);

    typedef SwFrameFormatsBase::nth_index<0>::type ByPos;
    typedef SwFrameFormatsBase::nth_index<1>::type ByTypeAndName;
    typedef ByPos::iterator iterator;

    SwFrameFormatsBase m_Array;
    ByPos& m_PosIndex;
    ByTypeAndName& m_TypeAndNameIndex;

public:
    typedef ByPos::const_iterator const_iterator;
    typedef ByTypeAndName::const_iterator const_range_iterator;
    typedef SwFrameFormatsBase::size_type size_type;
    typedef SwFrameFormatsBase::value_type value_type;

    SwFrameFormats();
    // frees all SwFrameFormat!
    virtual ~SwFrameFormats() override;

    bool empty() const { return m_Array.empty(); }
    size_t size() const { return m_Array.size(); }

    // Only fails, if you try to insert the same object twice
    std::pair<const_iterator, bool> push_back(const value_type& x);

    // This will try to remove the exact object!
    bool erase(const value_type& x);
    void erase(size_type index);
    void erase(const_iterator const& position);

    // Get the iterator of the exact object (includes pointer!),
    // e.g for position with std::distance.
    // There is also ContainsFormat, if you don't need the position.
    const_iterator find(const value_type& x) const;

    // As this array is non-unique related to type and name,
    // we always get ranges for the "key" values.
    std::pair<const_range_iterator, const_range_iterator> rangeFind(sal_uInt16 type,
                                                                    const OUString& name) const;
    // Convenience function, which just uses type and name!
    // To look for the exact object use find.
    std::pair<const_range_iterator, const_range_iterator> rangeFind(const value_type& x) const;
    // So we can actually check for end()
    const_range_iterator rangeEnd() const { return m_TypeAndNameIndex.end(); }
    const_iterator rangeProject(const_range_iterator const& position)
    {
        return m_Array.project<0>(position);
    }

    const value_type& operator[](size_t index_) const { return m_PosIndex.operator[](index_); }
    const value_type& front() const { return m_PosIndex.front(); }
    const value_type& back() const { return m_PosIndex.back(); }
    const_iterator begin() const { return m_PosIndex.begin(); }
    const_iterator end() const { return m_PosIndex.end(); }

    void dumpAsXml(xmlTextWriterPtr pWriter, const char* pName) const;

    virtual size_t GetFormatCount() const override { return m_Array.size(); }
    virtual SwFormat* GetFormat(size_t idx) const override { return operator[](idx); }

    /// fast check if given format is contained here
    /// @precond pFormat must not have been deleted
    bool ContainsFormat(SwFrameFormat const& rFormat) const;
    /// not so fast check that given format is still alive (i.e. contained here)
    bool IsAlive(SwFrameFormat const*) const;

    void DeleteAndDestroyAll(bool keepDefault = false);

    bool newDefault(const value_type& x);
    void newDefault(const_iterator const& position);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
