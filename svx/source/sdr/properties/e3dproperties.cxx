/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: e3dproperties.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 05:42:07 $
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

#ifndef _SDR_PROPERTIES_E3DPROPERTIES_HXX
#include <svx/sdr/properties/e3dproperties.hxx>
#endif

#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

#ifndef _SVDDEF_HXX
#include <svddef.hxx>
#endif

#ifndef _E3D_OBJ3D_HXX
#include <obj3d.hxx>
#endif

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
            rObj.StructureChanged(&rObj);
        }

        void E3dProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr)
        {
            // call parent
            AttributeProperties::SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);

            // propagate call to contained objects
            const SdrObjList* pSub = ((const E3dObject&)GetSdrObject()).GetSubList();
            const sal_uInt32 nCount(pSub->GetObjCount());

            for(sal_uInt32 a(0L); a < nCount; a++)
            {
                pSub->GetObj(a)->GetProperties().SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);
            }
        }
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
