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
#include <svx/sdr/properties/circleproperties.hxx>
#include <svl/itemset.hxx>
#include <svl/style.hxx>
#include <svx/svddef.hxx>
#include <editeng/eeitem.hxx>
#include <svx/svdocirc.hxx>
#include <svx/sxcikitm.hxx>
#include <svx/sxciaitm.hxx>
#include <svx/sxciaitm.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        // create a new itemset
        SfxItemSet& CircleProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            return *(new SfxItemSet(rPool,

                // range from SdrAttrObj
                SDRATTR_START, SDRATTR_SHADOW_LAST,
                SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
                SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,

                // range from SdrCircObj
                SDRATTR_CIRC_FIRST, SDRATTR_CIRC_LAST,

                // range from SdrTextObj
                EE_ITEMS_START, EE_ITEMS_END,

                // end
                0, 0));
        }

        CircleProperties::CircleProperties(SdrObject& rObj)
        :   RectangleProperties(rObj)
        {
        }

        CircleProperties::CircleProperties(const CircleProperties& rProps, SdrObject& rObj)
        :   RectangleProperties(rProps, rObj)
        {
        }

        CircleProperties::~CircleProperties()
        {
        }

        BaseProperties& CircleProperties::Clone(SdrObject& rObj) const
        {
            return *(new CircleProperties(*this, rObj));
        }

        void CircleProperties::ItemSetChanged(const SfxItemSet& rSet)
        {
            SdrCircObj& rObj = (SdrCircObj&)GetSdrObject();

            // call parent
            RectangleProperties::ItemSetChanged(rSet);

            // local changes
            rObj.ImpSetAttrToCircInfo();
        }

        void CircleProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr)
        {
            SdrCircObj& rObj = (SdrCircObj&)GetSdrObject();

            // local changes
            rObj.SetXPolyDirty();

            // call parent
            RectangleProperties::SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);

            // local changes
            rObj.ImpSetAttrToCircInfo();
        }

        void CircleProperties::ForceDefaultAttributes()
        {
            SdrCircObj& rObj = (SdrCircObj&)GetSdrObject();
            SdrCircKind eKindA = SDRCIRC_FULL;
            SdrObjKind eKind = rObj.GetCircleKind();

            if(eKind == OBJ_SECT)
            {
                eKindA = SDRCIRC_SECT;
            }
            else if(eKind == OBJ_CARC)
            {
                eKindA = SDRCIRC_ARC;
            }
            else if(eKind == OBJ_CCUT)
            {
                eKindA = SDRCIRC_CUT;
            }

            if(eKindA != SDRCIRC_FULL)
            {
                // force ItemSet
                GetObjectItemSet();

                mpItemSet->Put(SdrCircKindItem(eKindA));

                if(rObj.GetStartWink())
                {
                    mpItemSet->Put(SdrCircStartAngleItem(rObj.GetStartWink()));
                }

                if(rObj.GetEndWink() != 36000)
                {
                    mpItemSet->Put(SdrCircEndAngleItem(rObj.GetEndWink()));
                }
            }

            // call parent after SetObjectItem(SdrCircKindItem())
            // because ForceDefaultAttr() will call
            // ImpSetAttrToCircInfo() which needs a correct
            // SdrCircKindItem
            RectangleProperties::ForceDefaultAttributes();
        }
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
