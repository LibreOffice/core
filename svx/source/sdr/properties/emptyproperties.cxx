/*************************************************************************
 *
 *  $RCSfile: emptyproperties.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: pjunck $ $Date: 2004-11-03 10:51:00 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SDR_PROPERTIES_EMPTYPROPERTIES_HXX
#include <svx/sdr/properties/emptyproperties.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

#ifndef _SVDDEF_HXX
#include <svddef.hxx>
#endif

#ifndef _SVDOBJ_HXX
#include <svdobj.hxx>
#endif

#ifndef _SVDPOOL_HXX
#include <svdpool.hxx>
#endif

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

        void EmptyProperties::SetObjectItem(const SfxPoolItem& rItem)
        {
            DBG_ASSERT(sal_False, "EmptyProperties::SetObjectItem() should never be called (!)");
        }

        void EmptyProperties::SetObjectItemDirect(const SfxPoolItem& rItem)
        {
            DBG_ASSERT(sal_False, "EmptyProperties::SetObjectItemDirect() should never be called (!)");
        }

        void EmptyProperties::ClearObjectItem(const sal_uInt16 nWhich)
        {
            DBG_ASSERT(sal_False, "EmptyProperties::ClearObjectItem() should never be called (!)");
        }

        void EmptyProperties::ClearObjectItemDirect(const sal_uInt16 nWhich)
        {
            DBG_ASSERT(sal_False, "EmptyProperties::ClearObjectItemDirect() should never be called (!)");
        }

        void EmptyProperties::SetObjectItemSet(const SfxItemSet& rSet)
        {
            DBG_ASSERT(sal_False, "EmptyProperties::SetObjectItemSet() should never be called (!)");
        }

        void EmptyProperties::ItemSetChanged(const SfxItemSet& rSet)
        {
            DBG_ASSERT(sal_False, "EmptyProperties::ItemSetChanged() should never be called (!)");
        }

        sal_Bool EmptyProperties::AllowItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem) const
        {
            DBG_ASSERT(sal_False, "EmptyProperties::AllowItemChange() should never be called (!)");
            return sal_True;
        }

        void EmptyProperties::ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem)
        {
            DBG_ASSERT(sal_False, "EmptyProperties::ItemChange() should never be called (!)");
        }

        void EmptyProperties::PostItemChange(const sal_uInt16 nWhich)
        {
            DBG_ASSERT(sal_False, "EmptyProperties::PostItemChange() should never be called (!)");
        }

        void EmptyProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr)
        {
            DBG_ASSERT(sal_False, "EmptyProperties::SetStyleSheet() should never be called (!)");
        }

        SfxStyleSheet* EmptyProperties::GetStyleSheet() const
        {
            DBG_ASSERT(sal_False, "EmptyProperties::GetStyleSheet() should never be called (!)");
            return 0L;
        }

//BFS01     void EmptyProperties::PreProcessSave()
//BFS01     {
//BFS01     }

//BFS01     void EmptyProperties::PostProcessSave()
//BFS01     {
//BFS01     }
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
