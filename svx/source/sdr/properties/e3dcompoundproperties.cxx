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

#include <svx/sdr/properties/e3dcompoundproperties.hxx>
#include <svl/itemset.hxx>
#include <svx/obj3d.hxx>
#include <svx/scene3d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        E3dCompoundProperties::E3dCompoundProperties(SdrObject& rObj)
        :   E3dProperties(rObj)
        {
        }

        E3dCompoundProperties::E3dCompoundProperties(const E3dCompoundProperties& rProps, SdrObject& rObj)
        :   E3dProperties(rProps, rObj)
        {
        }

        E3dCompoundProperties::~E3dCompoundProperties()
        {
        }

        BaseProperties& E3dCompoundProperties::Clone(SdrObject& rObj) const
        {
            return *(new E3dCompoundProperties(*this, rObj));
        }

        const SfxItemSet& E3dCompoundProperties::GetObjectItemSet() const
        {
            //DBG_ASSERT(sal_False, "E3dCompoundProperties::GetObjectItemSet() maybe the wrong call (!)");
            return E3dProperties::GetObjectItemSet();
        }

        const SfxItemSet& E3dCompoundProperties::GetMergedItemSet() const
        {
            // include Items of scene this object belongs to
            E3dCompoundObject& rObj = (E3dCompoundObject&)GetSdrObject();
            E3dScene* pScene = rObj.GetScene();

            if(pScene)
            {
                // force ItemSet
                GetObjectItemSet();

                // add filtered scene properties (SDRATTR_3DSCENE_) to local itemset
                SfxItemSet aSet(*mpItemSet->GetPool(), SDRATTR_3DSCENE_FIRST, SDRATTR_3DSCENE_LAST);
                aSet.Put(pScene->GetProperties().GetObjectItemSet());
                mpItemSet->Put(aSet);
            }

            // call parent
            return E3dProperties::GetMergedItemSet();
        }

        void E3dCompoundProperties::SetMergedItemSet(const SfxItemSet& rSet, sal_Bool bClearAllItems)
        {
            // Set scene specific items at scene
            E3dCompoundObject& rObj = (E3dCompoundObject&)GetSdrObject();
            E3dScene* pScene = rObj.GetScene();

            if(pScene)
            {
                // force ItemSet
                GetObjectItemSet();

                // Generate filtered scene properties (SDRATTR_3DSCENE_) itemset
                SfxItemSet aSet(*mpItemSet->GetPool(), SDRATTR_3DSCENE_FIRST, SDRATTR_3DSCENE_LAST);
                aSet.Put(rSet);

                if(bClearAllItems)
                {
                    pScene->GetProperties().ClearObjectItem();
                }

                if(aSet.Count())
                {
                    pScene->GetProperties().SetObjectItemSet(aSet);
                }
            }

            // call parent. This will set items on local object, too.
            E3dProperties::SetMergedItemSet(rSet, bClearAllItems);
        }

        void E3dCompoundProperties::PostItemChange(const sal_uInt16 nWhich)
        {
            // call parent
            E3dProperties::PostItemChange(nWhich);

            // handle value change
            E3dCompoundObject& rObj = (E3dCompoundObject&)GetSdrObject();

            switch(nWhich)
            {
                // #i28528#
                // Added extra Item (Bool) for chart2 to be able to show reduced line geometry
                case SDRATTR_3DOBJ_REDUCED_LINE_GEOMETRY:
                {
                    rObj.ActionChanged();
                    break;
                }
                case SDRATTR_3DOBJ_DOUBLE_SIDED:
                {
                    rObj.ActionChanged();
                    break;
                }
                case SDRATTR_3DOBJ_NORMALS_KIND:
                {
                    rObj.ActionChanged();
                    break;
                }
                case SDRATTR_3DOBJ_NORMALS_INVERT:
                {
                    rObj.ActionChanged();
                    break;
                }
                case SDRATTR_3DOBJ_TEXTURE_PROJ_X:
                {
                    rObj.ActionChanged();
                    break;
                }
                case SDRATTR_3DOBJ_TEXTURE_PROJ_Y:
                {
                    rObj.ActionChanged();
                    break;
                }
            }
        }
    } // end of namespace properties
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
