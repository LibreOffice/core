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

#ifndef INCLUDED_ITEM_BASE_ISET_HXX
#define INCLUDED_ITEM_BASE_ISET_HXX

#include <item/base/ModelSpecificIValues.hxx>

///////////////////////////////////////////////////////////////////////////////
//
// Temporary migration infos:
//
///////////////////////////////////////////////////////////////////////////////
//
// -> NO usage of &op, so NOT used as flag values anywhere (!)
// enum class SfxItemState {
//     /** Specifies an unknown state. */
//     UNKNOWN  = 0,
//     /** Specifies that the property is currently disabled. */
//     DISABLED = 0x0001,
//     /** Specifies that the property is currently read-only. */
//     READONLY = 0x0002,
//     /** Specifies that the property is currently in a don't care state.
//      * <br/>
//      * This is normally used if a selection provides more than one state
//      * for a property at the same time.
//      */
//     DONTCARE = 0x0010,
//     /** Specifies that the property is currently in a default state. */
//     DEFAULT  = 0x0020,
//     /** The property has been explicitly set to a given value hence we know
//      * we are not taking the default value.
//      * <br/>
//      * For example, you may want to get the font color and it might either
//      * be the default one or one that has been explicitly set.
//     */
//     SET      = 0x0040
// };
//
///////////////////////////////////////////////////////////////////////////////
//
//    SfxItemSet::GetItemState:
// SfxItemState::UNKNOWN -> not in any range
// SfxItemState::DEFAULT -> nullptr in range
// SfxItemState::DONTCARE -> IsInvalidItem -> pItem == INVALID_POOL_ITEM -> reinterpret_cast<SfxPoolItem*>(-1)
// SfxItemState::DISABLED -> IsVoidItem() -> instance of SfxVoidItem, virtual bool IsVoidItem()
// SfxItemState::SET -> in range and ptr != nullptr && ptr != -1 && ptr != SfxVoidItem
//   NOT USED: SfxItemState::READONLY
//
// SfxItemState::READONLY
// 28 hits in code testing it, but never set ?!?!
//
// 16: interpreted as 'SfxItemState::DISABLED'
//  8: interpreted as 'SfxItemState::SET'
//  1: eState <= SfxItemState::READONLY
//  1: else ... -> used as DISABLED/READ_ONLY (with comment 'SfxItemState::DISABLED or SfxItemState::READONLY')
//  1: typecheck UNO API to throw css::uno::RuntimeException
//  1: name extract ItemBrowser
// check with '//UU' if needed -> testbuild OK
//
///////////////////////////////////////////////////////////////////////////////
//
// C:\lo\work01\workdir\UnoApiHeadersTarget\offapi\comprehensive\com\sun\star\frame\status\ItemState.hdl
//      only used: css::frame::status::ItemState::DONT_CARE;
// namespace com { namespace sun { namespace star { namespace frame { namespace status { namespace ItemState {
// static const ::sal_Int16 DEFAULT_VALUE = (sal_Int16)32;
// static const ::sal_Int16 DISABLED = (sal_Int16)1;
// static const ::sal_Int16 DONT_CARE = (sal_Int16)16;
// static const ::sal_Int16 READ_ONLY = (sal_Int16)2;
// static const ::sal_Int16 SET = (sal_Int16)64;
// static const ::sal_Int16 UNKNOWN = (sal_Int16)0;
//
///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    class ITEM_DLLPUBLIC ISet : public std::enable_shared_from_this<ISet>
    {
    public:
        // SharedPtr typedef to be used handling instances of this type
        typedef std::shared_ptr<ISet> SharedPtr;

        // The states that an Item (IBase) can have in an ISet
        enum class IState
        {
            /** Specifies that the property is currently disabled. */
            DISABLED = 0x0001,

            /** Specifies that the property is currently in a don't care state.
             * <br/>
             * This is normally used if a selection provides more than one state
             * for a property at the same time.
             */
            DONTCARE = 0x0010,

            /** Specifies that the property is currently in a default state. */
            DEFAULT = 0x0020,

            /** The property has been explicitly set to a given value hence we know
             * we are not taking the default value.
             * <br/>
             * For example, you may want to get the font color and it might either
             * be the default one or one that has been explicitly set.
            */
            SET = 0x0040
        };

        // helper class for returning result pairs for calls to ISet::StateAndItem
        // that contain the IState and the IBase at the same time.
        // IBase aItem will be set when IState is either SET or DEFAULT. The
        // DEFAULT will be the evtl. replaced one from ISet (see
        // ModelSpecificIValues)
        template< class TargetType > class StateAndItem
        {
        private:
            typedef std::shared_ptr<const TargetType> SharedPtr;

            const IState        m_aIState;
            const SharedPtr     m_aItem;

        public:
            StateAndItem(IState aIState, const SharedPtr& rItem)
            :   m_aIState(aIState),
                m_aItem(rItem)
            {
            }

            IState GetIState() const { return m_aIState; }
            const SharedPtr& GetItem() const { return m_aItem; }
            const TargetType* GetItemInstance() const { return m_aItem ? m_aItem.get() : nullptr; }

            bool HasItem() const { return bool(m_aItem); }
            bool IsSet() const { return IState::SET == m_aIState; }
            bool IsDefault() const { return IState::DEFAULT == m_aIState; }
            bool IsDontCare() const { return IState::DONTCARE == m_aIState; }
            bool IsDisabled() const { return IState::DISABLED == m_aIState; }
        };

    private:
        // helper class for an ImplInvalidateItem - placeholder for InvaidateState
        // SfxItemState::DONTCARE -> IsInvalidItem -> pItem == INVALID_POOL_ITEM -> reinterpret_cast<SfxPoolItem*>(-1)
        class ImplInvalidateItem : public IBase
        {
        public:
            ImplInvalidateItem() = default;
        };

        // helper class for a ImplDisableItem - placeholder for InvaidateState
        // SfxItemState::DISABLED -> IsVoidItem() -> instance of SfxVoidItem, virtual bool IsVoidItem()
        class ImplDisableItem : public IBase
        {
        public:
            ImplDisableItem() = default;
        };

        // the Parent of this ISet
        SharedPtr m_aParent;

        // model-specific ItemValues, e.g. overridden defaults.
        // Maybe set or not set, empty allowed here.
        ModelSpecificIValues::SharedPtr m_aModelSpecificIValues;

        // the items as content
        std::unordered_map<size_t, IBase::SharedPtr> m_aItems;

        // single global static instance for helper class ImplInvalidateItem
        static const IBase::SharedPtr& getInvalidateItem()
        {
            static IBase::SharedPtr aImplInvalidateItem(new ImplInvalidateItem());

            return aImplInvalidateItem;
        }

        // single global static instance for helper class ImplDisableItem
        static const IBase::SharedPtr& getDisableItem()
        {
            static IBase::SharedPtr aImplDisableItem(new ImplDisableItem());

            return aImplDisableItem;
        }

    protected:
        // constructor - protected BY DEFAULT - do NOT CHANGE (!)
        // Use ::Create(...) method instead
        ISet(const ModelSpecificIValues::SharedPtr& rModelSpecificIValues);

    public:
        virtual ~ISet();

        // noncopyable
        ISet(const ISet&) = delete;
        ISet& operator=(const ISet&) = delete;

        // parent
        void SetParent(const SharedPtr& rNewParent);
        const SharedPtr& GetParent() const;

        static SharedPtr Create(const ModelSpecificIValues::SharedPtr& rModelSpecificIValues);

        const ModelSpecificIValues::SharedPtr& GetModelSpecificIValues() const;

        template< typename TItem > void InvalidateItem()
        {
            const size_t hash_code(typeid(TItem).hash_code());

            m_aItems[hash_code] = getInvalidateItem();
        }

        template< typename TItem > void DisableItem()
        {
            const size_t hash_code(typeid(TItem).hash_code());

            m_aItems[hash_code] = getDisableItem();
        }

        template< typename TItem > std::shared_ptr<TItem> GetDefault() const
        {
            // get static available default as instance
            IBase::SharedPtr aRetval(TItem::GetDefault());
            assert(aRetval && "empty IBase::SharedPtr not allowed for default (!)");

            if(m_aModelSpecificIValues)
            {
                // may use model-specific default, get from helper
                // helper *will* fallback to IBase default
                aRetval = m_aModelSpecificIValues->GetDefault(aRetval);
            }

            return std::static_pointer_cast<TItem>(aRetval);
        }

        void SetItem(const IBase::SharedPtr& rItem);

        template< typename TItem > StateAndItem<TItem> GetStateAndItem(bool bSearchParent = true) const
        {
            const size_t hash_code(typeid(TItem).hash_code());
            const auto aRetval(m_aItems.find(hash_code));

            if(aRetval != m_aItems.end()) // && aRetval->second)
            {
                assert(aRetval->second && "empty IBase::SharedPtr set in ISet (!)");

                if(aRetval->second.get() == getInvalidateItem().get())
                {
                    // SfxItemState::DONTCARE
                    return StateAndItem<TItem>(IState::DONTCARE, std::shared_ptr<TItem>());
                }

                if(aRetval->second.get() == getDisableItem().get())
                {
                    // SfxItemState::DISABLED
                    return StateAndItem<TItem>(IState::DISABLED, std::shared_ptr<TItem>());
                }

                // SfxItemState::SET
                return StateAndItem<TItem>(IState::SET, std::static_pointer_cast<TItem>(aRetval->second));
            }

            // not set
            if(bSearchParent && m_aParent)
            {
                // continue searching in parent
                return m_aParent->GetStateAndItem<TItem>(true);
            }
            else
            {
                // SfxItemState::DEFAULT
                return StateAndItem<TItem>(IState::DEFAULT, GetDefault<TItem>());
            }
        }

        template< typename TItem > bool ClearItem()
        {
            const size_t hash_code(typeid(TItem).hash_code());

            return (0 != m_aItems.erase(hash_code));
        }
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_ITEM_BASE_ISET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
