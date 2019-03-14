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
    // Base class for IAdministrator. It's Task is to administer instances
    // of IBase - Items. Target is to always have only one instance
    // of a typed Item in one attributation, e.g. for sal_uInt16 many
    // users may use the instance representing the value '12'.
    // To do so, IAdministrator has to administer a list of that instances
    // as static global typed entity for each type of Item that allows
    // search and removal of typed Items.
    // The instances are by purpose no shared_ptr's of the instances. I have
    // made experiments with using weak_ptr already, but all slow and error
    // prone at office shutdown and removal. Best and fastest for this
    // anyways non-public accessible mechanism are the direct pointer
    // instances - these are handled by the rest of the Item namespace
    // as shared_ptr and thus will trigger the destructor when the last
    // usage is removed.
    // This is a working, fast and nice spot to then remove from this
    // typed list with a single call from the destructor. One small
    // optimization is to not need to check if member of the list by using
    // a boolean flag in Iase (see m_bAdministrated).
    // Note: These lists may need some semaphore/mutex mechanism later.
    // SfxItemSets never needed this, but were less global in the sense
    // to have global lists of re-used existing Items in the SfxItemPool
    // only. These SfxItemPools are pretty App-specific (except
    // EditEngine), but the Apps never were or are task-bound, so it's
    // worth to check first if problems arise at all - we'll see.
    // This single static global list for one Item-type is also the place
    // where to administer the Item's global default. This again is done
    // as shared_ptr to fit nicely to the rest of the mechanism.
    // Note to this default: This is the Item's global, type-specific
    // default, this may be overriden by ISet and ModelSpecificIValues,
    // but only in association with an ISet (see there). The Item-specific
    // global default will always be the same, so only deviating values
    // for ModelSpecificIValues need to be overriden.
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

        // needed IAdministrator calls from IBase implementations.
        // these are the add/remove calls to the list. The Create
        // will check existance/default and either re-use existing
        // instance (and delete given one) or start using given instance
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
    // IAdministrator-implementation using std::set for implementation
    // which guarantees good runtime for accesses (sorted list)
    // requirements from IBase:
    // - bool IBase::operator<(const IBase& rCand) const
    // Caution: This does not exist for current SfxItem implementations
    class ITEM_DLLPUBLIC IAdministrator_set : public IAdministrator
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
        IAdministrator_set(const IBase* pDefault);

        virtual IBase::SharedPtr Create(const IBase* pIBase) override;
        virtual void HintExpired(const IBase* pIBase) override;
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    // IAdministrator-implementation using std::unordered_set for
    // implementation which guarantees good runtime for accesses (hashed)
    // requirements from IBase:
    // - virtual bool IBase::operator==(const IBase& rCandidate) const
    // - size_t IBase::GetUniqueKey() const
    // Caution: GetUniqueKey does not exist for current SfxItem implementations
    class ITEM_DLLPUBLIC IAdministrator_unordered_set : public IAdministrator
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
                // forward to IBase::GetUniqueKey
                return pItem->GetUniqueKey();
            }
        };

        // std::unordered_set with all instanciated Items of this type,
        // using hash and operator== (see hash_for_unordered_set, compare_for_unordered_set above)
        std::unordered_set<const IBase*, hash_for_unordered_set, compare_for_unordered_set> m_aEntries;

    public:
        IAdministrator_unordered_set(const IBase* pDefault);

        virtual IBase::SharedPtr Create(const IBase* pIBase) override;
        virtual void HintExpired(const IBase* pIBase) override;
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    // IAdministrator-implementation using std::vector for implementation.
    // this is slow, but at least uses a 2nd vector to administer the free
    // slots in the 1st list. This is BTW what SfxItemPool does right now,
    // so this is slowest, but guarantees same speed as current implementation.
    // requirements from IBase:
    // - virtual bool IBase::operator==(const IBase& rCandidate) const
    // This does exist for current SfxItem implementations, so this is the
    // simple default for all Items that are hard to transfer/change/update,
    // it allows to continue to use the single operator== for administration.
    class ITEM_DLLPUBLIC IAdministrator_vector : public IAdministrator
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
        IAdministrator_vector(const IBase* pDefault);

        virtual IBase::SharedPtr Create(const IBase* pIBase) override;
        virtual void HintExpired(const IBase* pIBase) override;
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_ITEM_BASE_IADMINISTRATOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
