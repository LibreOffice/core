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

#ifndef INCLUDED_ITEM_BASE_ITEMSET_HXX
#define INCLUDED_ITEM_BASE_ITEMSET_HXX

#include <item/base/ModelSpecificItemValues.hxx>
//#include <item/base/ItemControlBlock.hxx>

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
//
// SfxItemSet::PutExtended usages:
// void                        PutExtended( const SfxItemSet&,
//                                          SfxItemState eDontCareAs,
//                                          SfxItemState eDefaultAs );
//
//     rDestSet.PutExtended( rSourceSet, SfxItemState::DONTCARE, SfxItemState::DEFAULT );
//             aDestSub.PutExtended( rSrcSub, SfxItemState::DONTCARE, SfxItemState::DEFAULT );
//             aDestSub.PutExtended( rSrcSub, SfxItemState::DONTCARE, SfxItemState::DEFAULT );
//     pStyles[i].pDest->GetItemSet().PutExtended(
//         pStyles[i].pSource->GetItemSet(), SfxItemState::DONTCARE, SfxItemState::DEFAULT);
// aSetItem.GetItemSet().PutExtended( rCoreSet, SfxItemState::DONTCARE, SfxItemState::SET );
//    pFound[i].pDest->GetItemSet().PutExtended(pFound[i].pSource->GetItemSet(), SfxItemState::DONTCARE, SfxItemState::DEFAULT);
//
// -> all eDontCareAs -> SfxItemState::DONTCARE -> Not needed
// -> only one eDefaultAs different from SfxItemState::DEFAULT
//      -> check ScViewUtil::PutItemScript is this is needed (sc\source\ui\view\viewutil.cxx)
//
// SvxScriptSetItem aSetItem( rPool.GetSlotId(nWhichId), rPool );
// SvxScriptSetItem::SvxScriptSetItem( sal_uInt16 nSlotId, SfxItemPool& rPool )
//     : SfxSetItem( nSlotId, std::make_unique<SfxItemSet>( rPool,
//                         svl::Items<SID_ATTR_CHAR_FONT, SID_ATTR_CHAR_FONT>{} ))
// {
//     sal_uInt16 nLatin, nAsian, nComplex;
//     GetWhichIds( nLatin, nAsian, nComplex );
//     GetItemSet().MergeRange( nLatin, nLatin );
//     GetItemSet().MergeRange( nAsian, nAsian );
//     GetItemSet().MergeRange( nComplex, nComplex );
// }
//
// -> default from pool from source-ItemSet is hard set as Item
// -> since this *is* the default, sould not be needed for new mechanism (?)
//
// case SID_ATTR_CHAR_FONT:
//     rLatin = SID_ATTR_CHAR_FONT;
//     rAsian = SID_ATTR_CHAR_CJK_FONT;
//     rComplex = SID_ATTR_CHAR_CTL_FONT;
//
// -> would be needed if ::SET state is checked for these, e.g. SID_ATTR_CHAR_CJK_FONT
//
// //get the font from itemset
// SfxItemState eState = _pPage->GetItemSet().GetItemState( _nFontWhich );
// if ( eState >= SfxItemState::DEFAULT )
//
// -> SfxItemState::DEFAULT and SfxItemState::SET are handled equal
// but:
//
// nSlot = SID_ATTR_CHAR_CJK_FONT;
// nWhich = GetWhich( nSlot );
// if ( !bChanged && pExampleSet &&
//      pExampleSet->GetItemState( nWhich, false, &pItem ) == SfxItemState::SET &&
//      static_cast<const SvxFontItem*>(pItem)->GetFamilyName() != aFontItem.GetFamilyName() )
//     bChanged = true;
//
// -> here used with SfxItemState::SET, but maybe OK -> needs testing (!)
//
///////////////////////////////////////////////////////////////////////////////


namespace Item
{
    class ITEM_DLLPUBLIC ItemSet : public std::enable_shared_from_this<ItemSet>
    {
    public:
        // SharedPtr typedef to be used handling instances of this type
        typedef std::shared_ptr<ItemSet> SharedPtr;

        // The states that an Item (ItemBase) can have in an ItemSet
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

        // helper class for returning result pairs for calls to ItemSet::StateAndItem
        // that contain the IState and the ItemBase at the same time.
        // ItemBase aItem will be set when IState is either SET or DEFAULT. The
        // DEFAULT will be the evtl. replaced one from ItemSet (see
        // ModelSpecificItemValues)
        template< class TargetType > class StateAndItem
        {
        private:
            const IState        m_aIState;
            const TargetType    m_aItem;

        public:
            StateAndItem(IState aIState, const TargetType& rItem)
            :   m_aIState(aIState),
                m_aItem(rItem)
            {
            }

            IState getIState() const { return m_aIState; }
            const TargetType& getItem() const { return m_aItem; }

            bool isSet() const { return IState::SET == m_aIState; }
            bool isDefault() const { return IState::DEFAULT == m_aIState; }
            bool isDontCare() const { return IState::DONTCARE == m_aIState; }
            bool isDisabled() const { return IState::DISABLED == m_aIState; }
        };

    private:
        // the Parent of this ItemSet
        SharedPtr m_aParent;

        // model-specific ItemValues, e.g. overridden defaults.
        // Maybe set or not set, empty allowed here.
        ModelSpecificItemValues::SharedPtr m_aModelSpecificIValues;

        // the items as content
        std::unordered_map<size_t, const ItemBase*> m_aItems;

        // helpers for reduction of template member implementations,
        // all based on typeid(<type>).hash_code()
        const ItemBase* implGetStateAndItem(size_t hash_code, IState& rIState, bool bSearchParent) const;
        void implInvalidateItem(size_t hash_code, const ItemBase& rItemDefault);
        void implDisableItem(size_t hash_code, const ItemBase& rItemDefault);
        bool implClearItem(size_t hash_code);

    protected:
        // constructor - protected BY DEFAULT - do NOT CHANGE (!)
        // Use ::Create(...) method instead
        ItemSet(const ModelSpecificItemValues::SharedPtr& rModelSpecificIValues);

    public:
        virtual ~ItemSet();

        // noncopyable
        ItemSet(const ItemSet&) = delete;
        ItemSet& operator=(const ItemSet&) = delete;

        // parent
        void setParent(const SharedPtr& rNewParent);
        const SharedPtr& getParent() const;

        // SharedPtr-construtcor
        static SharedPtr create(const ModelSpecificItemValues::SharedPtr& rModelSpecificIValues);

        // read access to ModelSpecificItemValues - maybe nullptr
        const ModelSpecificItemValues::SharedPtr& getModelSpecificIValues() const;

        // set Item(s)
        void setItem(const ItemBase& rItem);
        void setItems(const ItemSet& rSource, bool bDontCareToDefault = true);

        // get methods for a whole selection of Items
        std::vector<std::pair<const ItemBase*, IState>> getAllItemsAndStates() const;
        std::vector<const ItemBase*> getItemsOfState(IState eIState = IState::SET) const;

        // from here are all the type-specific template methods,
        // as reduced as possible due to these being unfolded from
        // the compiler for each class.
        // reduction uses local immp methods wherever possible, based
        // on the fetched TypeID
        template< typename TItem > void invalidateItem()
        {
            implInvalidateItem(typeid(TItem).hash_code(), Item::getDefault<TItem>());
        }

        template< typename TItem > void disableItem()
        {
            implDisableItem(typeid(TItem).hash_code(), Item::getDefault<TItem>());
        }

        template< typename TItem > const TItem& getDefault() const
        {
            // get static available default as instance
            const TItem& rStaticDefault(Item::getDefault<TItem>());

            if(m_aModelSpecificIValues)
            {
                // may use model-specific default, get from helper
                // helper *will* fallback to ItemBase default
                return static_cast<const TItem&>(m_aModelSpecificIValues->getDefault(rStaticDefault));
            }

            return rStaticDefault;
        }

        template< typename TItem > StateAndItem<TItem> getStateAndItem(bool bSearchParent = true) const
        {
            IState aIState(IState::DEFAULT);
            const ItemBase* pItem(implGetStateAndItem(typeid(TItem).hash_code(), aIState, bSearchParent));

            // SfxItemState::DEFAULT
            // SfxItemState::DONTCARE || SfxItemState::DISABLED -> should already be
            //  solved from ImplInvalidateItem/ImplDisableItem, but to have the
            //  fallback here additionally is never wrong
            // in short: no specific ItemBase -> use default
            if(nullptr == pItem)
            {
                return StateAndItem<TItem>(
                    aIState,
                    Item::getDefault<TItem>());
            }

            // SfxItemState::SET
            return StateAndItem<TItem>(
                aIState,
                *static_cast<const TItem*>(pItem));
        }

        template< typename TItem > bool clearItem()
        {
            return implClearItem(typeid(TItem).hash_code());
        }
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_ITEM_BASE_ITEMSET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
