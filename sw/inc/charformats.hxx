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
struct char_formats_name_key
    : boost::multi_index::composite_key<
          SwCharFormat*,
          boost::multi_index::const_mem_fun<SwFormat, const UIName&, &SwFormat::GetName>,
          boost::multi_index::identity<SwCharFormat*> // the actual object pointer
          >
{
};

typedef boost::multi_index_container<
    SwCharFormat*,
    boost::multi_index::indexed_by<boost::multi_index::random_access<>,
                                   boost::multi_index::ordered_unique<char_formats_name_key>>>
    SwCharFormatsBase;

class SW_DLLPUBLIC SwCharFormats final : public SwFormatsBase
{
    // function updating ByName index via modify
    friend void SwFormat::SetFormatName(const UIName&, bool);

public:
    typedef SwCharFormatsBase::nth_index<0>::type ByPos;
    typedef SwCharFormatsBase::nth_index<1>::type ByName;
    typedef ByPos::iterator iterator;

private:
    SwCharFormatsBase m_Array;
    ByPos& m_PosIndex;
    ByName& m_NameIndex;

public:
    typedef ByPos::const_iterator const_iterator;
    typedef SwCharFormatsBase::size_type size_type;
    typedef SwCharFormatsBase::value_type value_type;

    SwCharFormats();
    // frees all SwCharFormat!
    virtual ~SwCharFormats() override;

    bool empty() const { return m_Array.empty(); }
    size_t size() const { return m_Array.size(); }

    // Only fails, if you try to insert the same object twice
    void insert(SwCharFormat* x);

    // This will try to remove the exact object!
    void erase(const_iterator const& position);

    // Get the iterator of the exact object (includes pointer!),
    // e.g for position with std::distance.
    // There is also ContainsFormat, if you don't need the position.
    const_iterator find(const SwCharFormat* x) const;
    size_t GetPos(const SwCharFormat* p) const;

    // search for formats by name
    ByName::const_iterator findByName(const UIName& name) const;

    SwCharFormat* operator[](size_t index_) const { return m_PosIndex.operator[](index_); }
    const_iterator begin() const { return m_PosIndex.begin(); }
    const_iterator end() const { return m_PosIndex.end(); }

    void dumpAsXml(xmlTextWriterPtr pWriter) const;

    virtual size_t GetFormatCount() const override { return m_Array.size(); }
    virtual SwCharFormat* GetFormat(size_t idx) const override { return operator[](idx); }

    /// fast check if given format is contained here
    /// @precond pFormat must not have been deleted
    bool ContainsFormat(const SwCharFormat* pFormat) const;

    void DeleteAndDestroyAll(bool keepDefault = false);

    // Override return type to reduce casting
    virtual SwCharFormat* FindFormatByName(const UIName& rName) const override;

    /** Need to call this when the format name changes */
    void SetFormatNameAndReindex(SwCharFormat* v, const UIName& sNewName);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
