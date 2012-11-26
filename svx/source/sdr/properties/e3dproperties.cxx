/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svx/sdr/properties/e3dproperties.hxx>
#include <svl/itemset.hxx>
#include <svx/svddef.hxx>
#include <svx/obj3d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        // create a new itemset
        SfxItemSet& E3dProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            return *(new SfxItemSet(rPool,

                // ranges from SdrAttrObj
                SDRATTR_START, SDRATTR_SHADOW_LAST,
                SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
                SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,

                // ranges from E3dObject, contains object and scene because of GetMergedItemSet()
                SDRATTR_3D_FIRST, SDRATTR_3D_LAST,

                // end
                0, 0));
        }

        E3dProperties::E3dProperties(SdrObject& rObj)
        :   AttributeProperties(rObj)
        {
        }

        E3dProperties::E3dProperties(const E3dProperties& rProps, SdrObject& rObj)
        :   AttributeProperties(rProps, rObj)
        {
        }

        E3dProperties::~E3dProperties()
        {
        }

        BaseProperties& E3dProperties::Clone(SdrObject& rObj) const
        {
            return *(new E3dProperties(*this, rObj));
        }

        void E3dProperties::ItemSetChanged(const SfxItemSet& rSet)
        {
            E3dObject& rObj = (E3dObject&)GetSdrObject();

            // call parent
            AttributeProperties::ItemSetChanged(rSet);

            // local changes
            rObj.StructureChanged();
        }

        void E3dProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr)
        {
            // call parent
            AttributeProperties::SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);

            // propagate call to contained objects
            const SdrObjList* pSub = GetSdrObject().getChildrenOfSdrObject();

            if(pSub)
            {
                const sal_uInt32 nCount(pSub->GetObjCount());

                for(sal_uInt32 a(0L); a < nCount; a++)
                {
                    pSub->GetObj(a)->GetProperties().SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);
                }
            }
        }
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
