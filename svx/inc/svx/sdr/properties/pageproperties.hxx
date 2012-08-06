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

#ifndef _SDR_PROPERTIES_PAGEPROPERTIES_HXX
#define _SDR_PROPERTIES_PAGEPROPERTIES_HXX

#include <svx/sdr/properties/emptyproperties.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        class PageProperties : public EmptyProperties
        {
        protected:
            // create a new object specific itemset with object specific ranges.
            virtual SfxItemSet& CreateObjectSpecificItemSet(SfxItemPool& pPool);

            // Do the ItemChange, may do special handling
            virtual void ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem = 0);

            // Called after ItemChange() is done for all items.
            virtual void PostItemChange(const sal_uInt16 nWhich);

        public:
            // basic constructor
            explicit PageProperties(SdrObject& rObj);

            // constructor for copying, but using new object
            PageProperties(const PageProperties& rProps, SdrObject& rObj);

            // destructor
            virtual ~PageProperties();

            // Clone() operator, normally just calls the local copy constructor
            virtual BaseProperties& Clone(SdrObject& rObj) const;

            // get itemset. Overloaded here to allow creating the empty itemset
            // without asserting
            virtual const SfxItemSet& GetObjectItemSet() const;

            // get the installed StyleSheet
            virtual SfxStyleSheet* GetStyleSheet() const;

            // clear single item
            virtual void ClearObjectItem(const sal_uInt16 nWhich = 0);
        };
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_PROPERTIES_PAGEPROPERTIES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
