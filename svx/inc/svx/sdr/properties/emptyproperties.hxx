/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SDR_PROPERTIES_EMPTYPROPERTIES_HXX
#define _SDR_PROPERTIES_EMPTYPROPERTIES_HXX

#include <svx/sdr/properties/properties.hxx>
#include "svx/svxdllapi.h"

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        class SVX_DLLPUBLIC EmptyProperties : public BaseProperties
        {
        protected:
            // the to be used ItemSet
            SfxItemSet*                                     mpEmptyItemSet;

            // create a new itemset
            virtual SfxItemSet& CreateObjectSpecificItemSet(SfxItemPool& rPool);

            // test changeability for a single item
            virtual sal_Bool AllowItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem = 0) const;

            // Do the ItemChange, may do special handling
            virtual void ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem = 0);

            // Called after ItemChange() is done for all items.
            virtual void PostItemChange(const sal_uInt16 nWhich);

            // react on ItemSet changes
            virtual void ItemSetChanged(const SfxItemSet& rSet);

        public:
            // basic constructor
            EmptyProperties(SdrObject& rObj);

            // constructor for copying, but using new object
            EmptyProperties(const EmptyProperties& rProps, SdrObject& rObj);

            // destructor
            virtual ~EmptyProperties();

            // Clone() operator, normally just calls the local copy constructor
            virtual BaseProperties& Clone(SdrObject& rObj) const;

            // get itemset
            virtual const SfxItemSet& GetObjectItemSet() const;

            // set single item
            virtual void SetObjectItem(const SfxPoolItem& rItem);

            // set single item direct, do not do any notifies or things like that
            virtual void SetObjectItemDirect(const SfxPoolItem& rItem);

            // clear single item
            virtual void ClearObjectItem(const sal_uInt16 nWhich = 0);

            // clear single item direct, do not do any notifies or things like that.
            // Also supports complete deleteion of items when default parameter 0 is used.
            virtual void ClearObjectItemDirect(const sal_uInt16 nWhich = 0);

            // set complete item set
            virtual void SetObjectItemSet(const SfxItemSet& rSet);

            // set a new StyleSheet and broadcast
            virtual void SetStyleSheet(SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr);

            // get the installed StyleSheet
            virtual SfxStyleSheet* GetStyleSheet() const;
        };
    } // end of namespace properties
} // end of namespace sdr

#endif //_SDR_PROPERTIES_EMPTYPROPERTIES_HXX

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
