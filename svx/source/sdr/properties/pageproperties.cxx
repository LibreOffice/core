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

#include <svx/sdr/properties/pageproperties.hxx>
#include <svl/itemset.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpool.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        // create a new itemset
        SfxItemSet& PageProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            // overloaded to legally return a valid ItemSet
            return *(new SfxItemSet(rPool));
        }

        PageProperties::PageProperties(SdrObject& rObj)
        :   EmptyProperties(rObj)
        {
        }

        PageProperties::PageProperties(const PageProperties& rProps, SdrObject& rObj)
        :   EmptyProperties(rProps, rObj)
        {
        }

        PageProperties::~PageProperties()
        {
        }

        BaseProperties& PageProperties::Clone(SdrObject& rObj) const
        {
            return *(new PageProperties(*this, rObj));
        }

        // get itemset. Overloaded here to allow creating the empty itemset
        // without asserting
        const SfxItemSet& PageProperties::GetObjectItemSet() const
        {
            if(!mpEmptyItemSet)
            {
                ((PageProperties*)this)->mpEmptyItemSet = &(((PageProperties*)this)->CreateObjectSpecificItemSet(*GetSdrObject().GetObjectItemPool()));
            }

            DBG_ASSERT(mpEmptyItemSet, "Could not create an SfxItemSet(!)");

            return *mpEmptyItemSet;
        }

        void PageProperties::ItemChange(const sal_uInt16 /*nWhich*/, const SfxPoolItem* /*pNewItem*/)
        {
            // #86481# simply ignore item setting on page objects
        }

        SfxStyleSheet* PageProperties::GetStyleSheet() const
        {
            // overloaded to legally return a 0L pointer here
            return 0L;
        }

        void PageProperties::PostItemChange(const sal_uInt16 nWhich )
        {
            if( (nWhich == XATTR_FILLSTYLE) && (mpEmptyItemSet != NULL) )
                CleanupFillProperties(*mpEmptyItemSet);
        }

        void PageProperties::ClearObjectItem(const sal_uInt16 /*nWhich*/)
        {
            // simply ignore item clearing on page objects
        }
    } // end of namespace properties
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
