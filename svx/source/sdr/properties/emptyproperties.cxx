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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svx/sdr/properties/emptyproperties.hxx>
#include <tools/debug.hxx>
#include <svl/itemset.hxx>
#include <svx/svddef.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpool.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        // create a new itemset
        SfxItemSet& EmptyProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            // Basic implementation; Basic object has NO attributes
            DBG_ASSERT(sal_False, "EmptyProperties::CreateObjectSpecificItemSet() should never be called");
            return *(new SfxItemSet(rPool));
        }

        EmptyProperties::EmptyProperties(SdrObject& rObj)
        :   BaseProperties(rObj),
            mpEmptyItemSet(0L)
        {
        }

        EmptyProperties::EmptyProperties(const EmptyProperties& rProps, SdrObject& rObj)
        :   BaseProperties(rProps, rObj),
            mpEmptyItemSet(0L)
        {
            // #115593#
            // do not gererate an assert, else derivations like PageProperties will generate an assert
            // using the Clone() operator path.
        }

        EmptyProperties::~EmptyProperties()
        {
            if(mpEmptyItemSet)
            {
                delete mpEmptyItemSet;
                mpEmptyItemSet = 0L;
            }
        }

        BaseProperties& EmptyProperties::Clone(SdrObject& rObj) const
        {
            return *(new EmptyProperties(*this, rObj));
        }

        const SfxItemSet& EmptyProperties::GetObjectItemSet() const
        {
            if(!mpEmptyItemSet)
            {
                ((EmptyProperties*)this)->mpEmptyItemSet = &(((EmptyProperties*)this)->CreateObjectSpecificItemSet(*GetSdrObject().GetObjectItemPool()));
            }

            DBG_ASSERT(mpEmptyItemSet, "Could not create an SfxItemSet(!)");
            DBG_ASSERT(sal_False, "EmptyProperties::GetObjectItemSet() should never be called (!)");

            return *mpEmptyItemSet;
        }

        void EmptyProperties::SetObjectItem(const SfxPoolItem& /*rItem*/)
        {
            DBG_ASSERT(sal_False, "EmptyProperties::SetObjectItem() should never be called (!)");
        }

        void EmptyProperties::SetObjectItemDirect(const SfxPoolItem& /*rItem*/)
        {
            DBG_ASSERT(sal_False, "EmptyProperties::SetObjectItemDirect() should never be called (!)");
        }

        void EmptyProperties::ClearObjectItem(const sal_uInt16 /*nWhich*/)
        {
            DBG_ASSERT(sal_False, "EmptyProperties::ClearObjectItem() should never be called (!)");
        }

        void EmptyProperties::ClearObjectItemDirect(const sal_uInt16 /*nWhich*/)
        {
            DBG_ASSERT(sal_False, "EmptyProperties::ClearObjectItemDirect() should never be called (!)");
        }

        void EmptyProperties::SetObjectItemSet(const SfxItemSet& /*rSet*/)
        {
            DBG_ASSERT(sal_False, "EmptyProperties::SetObjectItemSet() should never be called (!)");
        }

        void EmptyProperties::ItemSetChanged(const SfxItemSet& /*rSet*/)
        {
            DBG_ASSERT(sal_False, "EmptyProperties::ItemSetChanged() should never be called (!)");
        }

        sal_Bool EmptyProperties::AllowItemChange(const sal_uInt16 /*nWhich*/, const SfxPoolItem* /*pNewItem*/) const
        {
            DBG_ASSERT(sal_False, "EmptyProperties::AllowItemChange() should never be called (!)");
            return sal_True;
        }

        void EmptyProperties::ItemChange(const sal_uInt16 /*nWhich*/, const SfxPoolItem* /*pNewItem*/)
        {
            DBG_ASSERT(sal_False, "EmptyProperties::ItemChange() should never be called (!)");
        }

        void EmptyProperties::PostItemChange(const sal_uInt16 /*nWhich*/)
        {
            DBG_ASSERT(sal_False, "EmptyProperties::PostItemChange() should never be called (!)");
        }

        void EmptyProperties::SetStyleSheet(SfxStyleSheet* /*pNewStyleSheet*/, sal_Bool /*bDontRemoveHardAttr*/)
        {
            DBG_ASSERT(sal_False, "EmptyProperties::SetStyleSheet() should never be called (!)");
        }

        SfxStyleSheet* EmptyProperties::GetStyleSheet() const
        {
            DBG_ASSERT(sal_False, "EmptyProperties::GetStyleSheet() should never be called (!)");
            return 0L;
        }
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
