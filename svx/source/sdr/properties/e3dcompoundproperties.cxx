/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: e3dcompoundproperties.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 05:41:22 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef _SDR_PROPERTIES_E3DCOMPOUNDPROPERTIES_HXX
#include <svx/sdr/properties/e3dcompoundproperties.hxx>
#endif

#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

#ifndef _E3D_OBJ3D_HXX
#include <obj3d.hxx>
#endif

#ifndef _E3D_SCENE3D_HXX
#include <scene3d.hxx>
#endif

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
                    rObj.InvalidateGeometry();
                    break;
                }
                case SDRATTR_3DOBJ_DOUBLE_SIDED:
                {
                    rObj.InvalidateGeometry();
                    break;
                }
                case SDRATTR_3DOBJ_NORMALS_KIND:
                {
                    rObj.InvalidateGeometry();
                    break;
                }
                case SDRATTR_3DOBJ_NORMALS_INVERT:
                {
                    rObj.InvalidateGeometry();
                    break;
                }
                case SDRATTR_3DOBJ_TEXTURE_PROJ_X:
                {
                    rObj.InvalidateGeometry();
                    break;
                }
                case SDRATTR_3DOBJ_TEXTURE_PROJ_Y:
                {
                    rObj.InvalidateGeometry();
                    break;
                }
            }
        }
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
