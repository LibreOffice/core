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

#include <boost/intrusive_ptr.hpp>
#include "rtfvalue.hxx"

namespace writerfilter
{
namespace rtftok
{

using RTFSprmsImplBase = std::vector< std::pair<Id, RTFValue::Pointer_t> >;

/// The payload of RTFSprms which is only copied on write.
class RTFSprmsImpl : public RTFSprmsImplBase
{
public:
    sal_Int32 m_nRefCount = 0;
};

inline void intrusive_ptr_add_ref(RTFSprmsImpl* p)
{
    ++(p->m_nRefCount);
}
inline void intrusive_ptr_release(RTFSprmsImpl* p)
{
    if (!--(p->m_nRefCount))
        delete p;
}

enum class RTFOverwrite
{
    YES, ///< Yes, if an existing key is found, overwrite it.
    NO_APPEND, ///< No, always append the value to the end of the list.
    NO_IGNORE ///< No, if the key is already in the list, then ignore, otherwise append.
};

/// A list of RTFSprm with a copy constructor that performs a deep copy.
class RTFSprms
{
public:
    using Pointer_t = std::shared_ptr<RTFSprms>;
    using Entry_t = std::pair<Id, RTFValue::Pointer_t>;
    using Iterator_t = std::vector<Entry_t>::iterator;
    using ReverseIterator_t = std::vector<Entry_t>::reverse_iterator;
    RTFSprms();
    RTFSprms(const RTFSprms& rSprms);
    ~RTFSprms();
    RTFValue::Pointer_t find(Id nKeyword, bool bFirst = true, bool bForWrite = false);
    /// Does the same as ->push_back(), except that it can overwrite or ignore existing entries.
    void set(Id nKeyword, RTFValue::Pointer_t pValue, RTFOverwrite eOverwrite = RTFOverwrite::YES);
    bool erase(Id nKeyword);
    void eraseLast(Id nKeyword);
    /// Removes elements which are already in the reference set.
    /// Also insert default values to override attributes of style
    /// (yes, really; that's what Word does).
    RTFSprms cloneAndDeduplicate(RTFSprms& rReference) const;
    std::size_t size() const
    {
        return m_pSprms->size();
    }
    bool empty() const
    {
        return m_pSprms->empty();
    }
    Entry_t& back()
    {
        return m_pSprms->back();
    }
    Iterator_t begin()
    {
        return m_pSprms->begin();
    }
    Iterator_t end()
    {
        return m_pSprms->end();
    }
    void clear();
    bool equals(RTFValue& rOther);
private:
    void ensureCopyBeforeWrite();
    boost::intrusive_ptr<RTFSprmsImpl> m_pSprms;
};

/// RTF keyword with a parameter
class RTFSprm
    : public Sprm
{
public:
    RTFSprm(Id nKeyword, RTFValue::Pointer_t& pValue);
    virtual ~RTFSprm() = default;
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
