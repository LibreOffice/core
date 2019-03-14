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

#ifndef INCLUDED_ITEM_BASE_IADMINISTRATOR_HXX
#define INCLUDED_ITEM_BASE_IADMINISTRATOR_HXX

#include <vector>
#include <set>
#include <unordered_set>
#include <cassert>
#include <item/base/IBase.hxx>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    class ITEM_DLLPUBLIC IAdministrator
    {
    protected:
        // instance of global default value
        IBase::SharedPtr m_aDefault;

        void SetAdministrated(const IBase& rIBase) const;

    public:
        // constructor/destructor
        IAdministrator(const IBase* pDefault);
        virtual ~IAdministrator();

        // noncopyable
        IAdministrator(const IAdministrator&) = delete;
        IAdministrator& operator=(const IAdministrator&) = delete;

        // needed IAdministrator calls from IBase implementations
        virtual void HintExpired(const IBase* pIBase);
        virtual IBase::SharedPtr Create(const IBase* pIBase) = 0;

        // interface for global default value support
        // on Administrator level
        const IBase::SharedPtr& GetDefault() const;
        bool IsDefault(const IBase* pIBase) const;
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    // uses from IBase
    // - bool IBase::operator<(const IBase& rCand) const
    class ITEM_DLLPUBLIC IAdministrator_sort : public IAdministrator
    {
    private:
        struct less_for_set
        {
            bool operator()(const IBase* pItem1, const IBase* pItem2) const
            {
                // forward to IBase::operator<
                return pItem1->operator<(*pItem2);
            }
        };

        // std::set with all instanciated Items of this type, sorted by
        // operator< (see less_for_set above)
        std::set<const IBase*, less_for_set> m_aEntries;

    public:
        IAdministrator_sort(const IBase* pDefault);
        virtual IBase::SharedPtr Create(const IBase* pIBase) override;
        virtual void HintExpired(const IBase* pIBase) override;
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    // uses from IBase
    // - virtual bool IBase::operator==(const IBase& rCandidate) const
    // - size_t IBase::GetHash() const
    class ITEM_DLLPUBLIC IAdministrator_hash : public IAdministrator
    {
    private:
        struct compare_for_unordered_set
        {
            bool operator()(const IBase* pItem1, const IBase* pItem2) const
            {
                // forward to IBase::operator==
                return pItem1->operator==(*pItem2);
            }
        };

        struct hash_for_unordered_set
        {
            size_t operator()(const IBase* pItem) const
            {
                // forward to IBase::GetHash
                return pItem->GetHash();
            }
        };

        // std::unordered_set with all instanciated Items of this type,
        // using hash and operator== (see hash_for_unordered_set, compare_for_unordered_set above)
        std::unordered_set<const IBase*, hash_for_unordered_set, compare_for_unordered_set> m_aEntries;

    public:
        IAdministrator_hash(const IBase* pDefault);
        virtual IBase::SharedPtr Create(const IBase* pIBase) override;
        virtual void HintExpired(const IBase* pIBase) override;
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    // uses from IBase
    // - virtual bool IBase::operator==(const IBase& rCandidate) const
    class ITEM_DLLPUBLIC IAdministrator_unsortedList : public IAdministrator
    {
    private:
        // this unsorted list is used when only operator== is available. Thus
        // this is the slowest of the available IAdministrators, ony use when
        // not avoidable or in a phase of change (SfxItem's have operator== already).
        // It uses a std::vector to hjold all instances, plus a list of free slots
        // (just indices) for reuse to avoid re-organizing the vector
        std::vector<const IBase*> m_aEntries;
        std::vector<size_t> m_aFreeSlots;

        std::vector<const IBase*>::iterator find(const IBase* pIBase);
        void insert(const IBase* pIBase);
        void erase(std::vector<const IBase*>::iterator& rIter);

    public:
        IAdministrator_unsortedList(const IBase* pDefault);
        virtual IBase::SharedPtr Create(const IBase* pIBase) override;
        virtual void HintExpired(const IBase* pIBase) override;
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_ITEM_BASE_IADMINISTRATOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
