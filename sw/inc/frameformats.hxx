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
#include "frmfmt.hxx"
#include "swtblfmt.hxx"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/tag.hpp>
#include <libxml/xmlstring.h>
#include <libxml/xmlwriter.h>

class SwFrameFormat;
class SwTableFormat;

// Like o3tl::find_partialorder_ptrequals
// We don't allow duplicated object entries!
namespace sw
{
template <class value_type> class FrameFormats final : public SwFormatsBase
{
    struct ByPos
    {
    };
    struct ByTypeAndName
    {
    };
    struct FrameFormatsKey
        : boost::multi_index::composite_key<
              value_type,
              boost::multi_index::const_mem_fun<SwFormat, const UIName&, &SwFormat::GetName>,
              boost::multi_index::const_mem_fun<SwFormat, sal_uInt16, &SwFormat::Which>,
              boost::multi_index::identity<value_type> // the actual object pointer
              >
    {
    };
    typedef boost::multi_index_container<
        value_type, boost::multi_index::indexed_by<
                        boost::multi_index::random_access<boost::multi_index::tag<ByPos>>,
                        boost::multi_index::ordered_unique<boost::multi_index::tag<ByTypeAndName>,
                                                           FrameFormatsKey>>>
        FrameFormatsContainer;
    // function updating ByName index via modify
    friend class ::SwFrameFormat;

public:
    // getting from T* to T const* ...
    typedef typename std::add_pointer<
        typename std::add_const<typename std::remove_pointer<value_type>::type>::type>::type
        const_value_type;
    typedef typename FrameFormatsContainer::size_type size_type;
    typedef typename FrameFormatsContainer::template index<ByPos>::type index_type;
    typedef typename index_type::iterator iterator;
    typedef typename index_type::const_iterator const_iterator;
    typedef typename FrameFormatsContainer::template index<ByTypeAndName>::type name_index_type;
    typedef typename name_index_type::iterator name_iterator;
    typedef typename name_index_type::const_iterator const_name_iterator;
    typedef typename std::pair<const_name_iterator, const_name_iterator> range_type;

private:
    FrameFormatsContainer m_vContainer;
    index_type& GetByPos() { return m_vContainer.template get<ByPos>(); }
    name_index_type& GetByTypeAndName() { return m_vContainer.template get<ByTypeAndName>(); }
    const index_type& GetByPos() const { return m_vContainer.template get<ByPos>(); }
    const name_index_type& GetByTypeAndName() const
    {
        return m_vContainer.template get<ByTypeAndName>();
    }

public:
    FrameFormats(){};
    // frees all SwFrameFormat!
    virtual ~FrameFormats() override { DeleteAndDestroyAll(); };

    bool empty() const { return m_vContainer.empty(); }
    size_t size() const { return m_vContainer.size(); }

    // Only fails, if you try to insert the same object twice
    std::pair<const_iterator, bool> push_back(const value_type& x)
    {
        SAL_WARN_IF(x->m_ffList != nullptr, "sw.core", "Inserting already assigned item");
        assert(x->m_ffList == nullptr);
        x->m_ffList = this;
        return GetByPos().push_back(const_cast<value_type>(x));
    };

    // This will try to remove the exact object!
    bool erase(const value_type& x)
    {
        const_iterator const ret = find(x);
        SAL_WARN_IF(x->m_ffList != this, "sw.core", "Removing invalid / unassigned item");
        if (ret != end())
        {
            assert(x == *ret);
            x->m_ffList = nullptr;
            GetByPos().erase(ret);
            return true;
        }
        return false;
    };
    void erase(size_type index) { erase(begin() + index); };
    void erase(const_iterator const& position)
    {
        (*position)->m_ffList = nullptr;
        GetByPos().erase(begin() + (position - begin()));
    }

    // Get the iterator of the exact object (includes pointer!),
    // e.g for position with std::distance.
    // There is also ContainsFormat, if you don't need the position.
    const_iterator find(const value_type& x) const
    {
        auto it = GetByTypeAndName().find(std::make_tuple(x->GetName(), x->Which(), x));
        return m_vContainer.template project<ByPos>(it);
    };

    const_name_iterator findByTypeAndName(sal_uInt16 type, const UIName& name) const
    {
        return GetByTypeAndName().find(std::make_tuple(name, type));
    };
    // search for formats by name
    range_type findRangeByName(const UIName& rName) const
    {
        auto& idx = GetByTypeAndName();
        auto it = idx.lower_bound(std::make_tuple(rName, sal_uInt16(0)));
        auto itEnd = idx.upper_bound(std::make_tuple(rName, SAL_MAX_UINT16));
        return { it, itEnd };
    };
    // So we can actually check for end()
    const_name_iterator typeAndNameEnd() const { return GetByTypeAndName().end(); }

    const value_type& operator[](size_t index) const { return GetByPos().operator[](index); }
    const value_type& front() const { return GetByPos().front(); }
    const value_type& back() const { return GetByPos().back(); }
    const_iterator begin() const { return GetByPos().begin(); }
    const_iterator end() const { return GetByPos().end(); }

    void dumpAsXml(xmlTextWriterPtr pWriter, const char* pName) const
    {
        (void)xmlTextWriterStartElement(pWriter, BAD_CAST(pName));
        for (const auto pFormat : GetByPos())
            pFormat->dumpAsXml(pWriter);
        (void)xmlTextWriterEndElement(pWriter);
    };

    virtual size_t GetFormatCount() const override { return m_vContainer.size(); }
    virtual SwFormat* GetFormat(size_t idx) const override
    {
        return const_cast<value_type&>(operator[](idx));
    };
    virtual void Rename(const SwFrameFormat& rFormat, const UIName& sNewName) override
    {
        assert(dynamic_cast<value_type>(const_cast<SwFrameFormat*>(&rFormat)));
        iterator it = find(static_cast<value_type>(const_cast<SwFrameFormat*>(&rFormat)));
        assert(end() != it);
        const auto sOldName = rFormat.GetName();
        auto fRenamer
            = [sNewName](SwFormat* pFormat) { pFormat->SwFormat::SetFormatName(sNewName, false); };
        auto fRenamerUndo
            = [sOldName](SwFormat* pFormat) { pFormat->SwFormat::SetFormatName(sOldName, false); };
        bool const renamed = GetByPos().modify(it, fRenamer, fRenamerUndo);
        assert(renamed);
        (void)renamed; // unused in NDEBUG
    };

    /// fast check if given format is contained here
    /// @precond pFormat must not have been deleted
    bool ContainsFormat(const value_type& rpFormat) const { return rpFormat->m_ffList == this; };

    /// not so fast check that given format is still alive (i.e. contained here)
    bool IsAlive(const_value_type pFrameFormat) const
    {
        auto pThisNonConst
            = const_cast<typename std::remove_const<sw::FrameFormats<value_type>>::type*>(this);
        return pThisNonConst->find(const_cast<value_type>(pFrameFormat)) != pThisNonConst->end();
    };

    void DeleteAndDestroyAll(bool keepDefault = false)
    {
        if (empty())
            return;
        const int offset = keepDefault ? 1 : 0;
        for (const_iterator it = begin() + offset; it != end(); ++it)
            delete *it;
        if (offset)
            GetByPos().erase(begin() + offset, end());
        else
            m_vContainer.clear();
    };

    bool newDefault(const value_type& x)
    {
        std::pair<iterator, bool> res = GetByPos().push_front(const_cast<value_type&>(x));
        if (!res.second)
            newDefault(res.first);
        return res.second;
    };
    void newDefault(const_iterator const& position)
    {
        if (position == begin())
            return;
        GetByPos().relocate(begin(), position);
    };

    // Override return type to reduce casting
    value_type FindFrameFormatByName(const UIName& rName) const
    {
        auto& idx = GetByTypeAndName();
        auto it = idx.lower_bound(std::make_tuple(rName, sal_uInt16(0)));
        if (it != idx.end() && (*it)->GetName() == rName)
            return *it;
        return nullptr;
    };
};
typedef FrameFormats<::SwTableFormat*> TableFrameFormats;
typedef FrameFormats<sw::SpzFrameFormat*> SpzFrameFormats;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
