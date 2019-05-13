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

#ifndef INCLUDED_ITEM_BASE_MODELSPECIFICITEMVALUES_HXX
#define INCLUDED_ITEM_BASE_MODELSPECIFICITEMVALUES_HXX

#include <memory>
#include <item/itemdllapi.h>
#include <unordered_map>
#include <item/base/ItemBase.hxx>
#include <sal/types.h>
#include <tools/mapunit.hxx>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    // Model-specific ItemValues, e.g. overridden defaults.
    // - override defaults per Set (already in place)
    // - maybe: old SfxItemInfo replacement which is a mapping
    //   between SlotID and WhichID
    // - maybe: ItemNames - currently in ::TakeItemName and others
    // - maybe: hold MapMode info to allow correct GetPresentation
    //   executions
    //
    // Defaults:
    // Ckecked current stuff, there are two sets of items at a pool:
    //  (1) mpStaticDefaults -> size of start_end fully set ptrs, static defaults, not to be changed, may be owned or not, may be cloned or not
    //  (2) maPoolDefaults -> size of start_end inited by nullptr, set/reset by SetPoolDefaultItem/ResetPoolDefaultItem
    // This means that we need also two sets here, one for (1) static defaults
    // which have to be set when different from Item::getDefault and one
    // for just overridden defaults (1). The 2nd one also has to allow resetting
    // single entries.
    // What does SfxItemPool::GetDefaultItem -> first checks maPoolDefaults, then uses mpStaticDefaults
    //
    // MapUnit:
    // in current SfxItemPool there is a 'MapUnit eDefMetric', but also
    // ::GetMetric(sal_uInt16 nWhich) - grep for that. Dependent of the ItemID
    // the eDefMetric may be used (that is the default ignoring nWhich), but there
    // are exceptions - hard to understand.
    // What is and stays clear:
    // - one SfxItemPool incarnation including all its 2ndary pools has one MapUnit
    // - Writer: DrawingLayer runs in MapUnit::MapTwip (which is the default for
    //   SfxItemPool constructor, will be changed using ::SetDefaultMetric, grep).
    //   All Items in Writer run in MapUnit::MapTwip
    // - Draw/Impress/Chart: All Items run in MapUnit::Map100thMM
    // - Calc: here it gets complicated - looks as if between
    //   ATTR_STARTINDEX and ATTR_ENDINDEX MapUnit::MapTwip is used. It is
    //   OTOH known that DrawingLayer in Calc runs in MapUnit::Map100thMM.
    //   Strange is ScMessagePool/ScDocumentPool which use for the given range
    //   MapUnit::MapTwip, but MapUnit::Map100thMM for all else - thus the Items in
    //   ScMessagePool are in Map100thMM while the doc items in Calc use MapTwip ?!?
    // Usually the eDefMetric at SfxItemPool is correct, it may be necessary
    // to define 'Item-Groups' e.g. for ScMessagePool which have an exceptional
    // MapUnit.
    // Alternatively each Item could have an entry for MapUnit, but that would
    // make them expensive - this has to be taken into account in compare, search,
    // grep, sort, ecerywhere. No more simple Items, so not really an alternative - sigh.

    class ITEM_DLLPUBLIC ModelSpecificItemValues : public std::enable_shared_from_this<ModelSpecificItemValues>
    {
    public:
        // SharedPtr typedef to be used handling instances of this type
        typedef std::shared_ptr<ModelSpecificItemValues> SharedPtr;

    private:
        // the static and alternative default items
        std::unordered_map<size_t, const ItemBase*> m_aStaticItems;
        std::unordered_map<size_t, const ItemBase*> m_aAlternativeItems;

        // the MapUnit used
        MapUnit                                     m_aMapUnit;

    protected:
        // constructor - protected BY DEFAULT - do NOT CHANGE (!)
        // Use ::Create(...) method instead
        ModelSpecificItemValues(
            MapUnit aMapUnit);

        void implclearStaticDefaultItem(size_t hash_code);
        void implclearAlternativeDefaultItem(size_t hash_code);

    public:
        virtual ~ModelSpecificItemValues();

        // noncopyable
        ModelSpecificItemValues(const ModelSpecificItemValues&) = delete;
        ModelSpecificItemValues& operator=(const ModelSpecificItemValues&) = delete;

        // create instance
        static SharedPtr create(
            MapUnit aMapUnit = MapUnit::MapTwip
        );

        // MapUnit interface
        MapUnit getMapUnit() const
        {
            return m_aMapUnit;
        }

        // StaticDefault interface
        void setStaticDefaultItem(const ItemBase& rItem);
        template<class T> void clearStaticDefaultItem()
        {
            implclearStaticDefaultItem(typeid(T).hash_code());
        }
        void clearStaticDefaultItems();

        // AlternativeDefault interface
        void setAlternativeDefaultItem(const ItemBase& rItem);
        template<class T> void clearAlternativeDefaultItem()
        {
            implclearAlternativeDefaultItem(typeid(T).hash_code());
        }
        void clearAlternativeDefaultItems();

        // default interface using the state of
        // AlternativeDefault/StaticDefault itzems
        bool isDefault(const ItemBase& rItem) const;
        const ItemBase& getDefault(const ItemBase& rItem) const;
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_ITEM_BASE_MODELSPECIFICITEMVALUES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
