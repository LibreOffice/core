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

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
