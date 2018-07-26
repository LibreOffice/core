/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WRITERFILTER_SOURCE_RTFTOK_RTFSPRM_HXX
#define INCLUDED_WRITERFILTER_SOURCE_RTFTOK_RTFSPRM_HXX

#include <string>
#include <utility>
#include <vector>
#include <map>

#include <tools/ref.hxx>
#include "rtfvalue.hxx"

namespace writerfilter
{
namespace rtftok
{
using RTFSprmsImplBase = std::vector<std::pair<Id, RTFValue::Pointer_t>>;

/// The payload of RTFSprms which is only copied on write.
class RTFSprmsImpl : public RTFSprmsImplBase, public virtual SvRefBase
{
};

enum class RTFOverwrite
{
    YES, ///< Yes, if an existing key is found, overwrite it.
    NO_APPEND, ///< No, always append the value to the end of the list.
    NO_IGNORE, ///< No, if the key is already in the list, then ignore, otherwise append.
    YES_PREPEND ///< Yes, always prepend the value to the start of the list and remove existing entries.
};

/// A list of RTFSprm with a copy constructor that performs a deep copy.
class RTFSprms : public virtual SvRefBase
{
public:
    using Pointer_t = tools::SvRef<RTFSprms>;
    using Entry_t = std::pair<Id, RTFValue::Pointer_t>;
    using Iterator_t = std::vector<Entry_t>::iterator;
    using ReverseIterator_t = std::vector<Entry_t>::reverse_iterator;
    RTFSprms();
    ~RTFSprms() override;

    RTFSprms(RTFSprms const&) = default;
    RTFSprms(RTFSprms&&) = default;
    RTFSprms& operator=(RTFSprms const&) = default;
    RTFSprms& operator=(RTFSprms&&) = default;

    RTFValue::Pointer_t find(Id nKeyword, bool bFirst = true, bool bForWrite = false);
    /// Does the same as ->push_back(), except that it can overwrite or ignore existing entries.
    void set(Id nKeyword, RTFValue::Pointer_t pValue, RTFOverwrite eOverwrite = RTFOverwrite::YES);
    bool erase(Id nKeyword);
    void eraseLast(Id nKeyword);
    /// Removes elements which are already in the reference set.
    /// Also insert default values to override attributes of style
    /// (yes, really; that's what Word does).
    RTFSprms cloneAndDeduplicate(RTFSprms& rReference) const;
    /// Inserts default values to override attributes of pAbstract.
    void duplicateList(const RTFValue::Pointer_t& pAbstract);
    /// Removes duplicated values based on in-list properties.
    void deduplicateList(const std::map<int, int>& rInvalidListLevelFirstIndents);
    std::size_t size() const { return m_pSprms->size(); }
    bool empty() const { return m_pSprms->empty(); }
    Entry_t& back() { return m_pSprms->back(); }
    Iterator_t begin() { return m_pSprms->begin(); }
    Iterator_t end() { return m_pSprms->end(); }
    void clear();
    bool equals(RTFValue& rOther);

private:
    void ensureCopyBeforeWrite();
    tools::SvRef<RTFSprmsImpl> m_pSprms;
};

/// RTF keyword with a parameter
class RTFSprm : public Sprm
{
public:
    RTFSprm(Id nKeyword, RTFValue::Pointer_t& pValue);
    sal_uInt32 getId() const override;
    Value::Pointer_t getValue() override;
    writerfilter::Reference<Properties>::Pointer_t getProps() override;
#ifdef DEBUG_WRITERFILTER
    std::string getName() const override;
    std::string toString() const override;
#endif
private:
    Id m_nKeyword;
    RTFValue::Pointer_t& m_pValue;
};
} // namespace rtftok
} // namespace writerfilter

#endif // INCLUDED_WRITERFILTER_SOURCE_RTFTOK_RTFSPRM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
