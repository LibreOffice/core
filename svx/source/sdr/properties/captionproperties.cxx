/*************************************************************************
 *
 *  $RCSfile: captionproperties.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: pjunck $ $Date: 2004-11-03 10:47:57 $
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

#ifndef _SDR_PROPERTIES_CAPTIONPROPERTIES_HXX
#include <svx/sdr/properties/captionproperties.hxx>
#endif

#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

#ifndef _SFXSTYLE_HXX
#include <svtools/style.hxx>
#endif

#ifndef _SVDDEF_HXX
#include <svddef.hxx>
#endif

#ifndef _EEITEM_HXX
#include <eeitem.hxx>
#endif

#ifndef _SVDCAPT_HXX
#include <svdocapt.hxx>
#endif

#ifndef _SXCSITM_HXX
#include <sxcsitm.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        // create a new itemset
        SfxItemSet& CaptionProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            return *(new SfxItemSet(rPool,

                // range from SdrAttrObj
                SDRATTR_START, SDRATTR_SHADOW_LAST,
                SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
                SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,

                // range from SdrCaptionObj
                SDRATTR_CAPTION_FIRST, SDRATTR_CAPTION_LAST,

                // range from SdrTextObj
                EE_ITEMS_START, EE_ITEMS_END,

                // end
                0, 0));
        }

        CaptionProperties::CaptionProperties(SdrObject& rObj)
        :   RectangleProperties(rObj)
        {
        }

        CaptionProperties::CaptionProperties(const CaptionProperties& rProps, SdrObject& rObj)
        :   RectangleProperties(rProps, rObj)
        {
        }

        CaptionProperties::~CaptionProperties()
        {
        }

        BaseProperties& CaptionProperties::Clone(SdrObject& rObj) const
        {
            return *(new CaptionProperties(*this, rObj));
        }

        void CaptionProperties::ItemSetChanged(const SfxItemSet& rSet)
        {
            SdrCaptionObj& rObj = (SdrCaptionObj&)GetSdrObject();

            // local changes
            rObj.ImpRecalcTail();

            // call parent
            RectangleProperties::ItemSetChanged(rSet);
        }

        void CaptionProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr)
        {
            SdrCaptionObj& rObj = (SdrCaptionObj&)GetSdrObject();

            // call parent
            RectangleProperties::SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);

            // local changes
            rObj.ImpRecalcTail();
        }

//BFS01     void CaptionProperties::PreProcessSave()
//BFS01     {
//BFS01         // call parent
//BFS01         RectangleProperties::PreProcessSave();
//BFS01
//BFS01         // force ItemSet
//BFS01         GetObjectItemSet();
//BFS01
//BFS01         // prepare SetItems for storage
//BFS01         const SfxItemSet& rSet = *mpItemSet;
//BFS01         const SfxItemSet* pParent = mpStyleSheet ? &(mpStyleSheet->GetItemSet()) : 0L;
//BFS01
//BFS01         SdrCaptionSetItem aCaptAttr(rSet.GetPool());
//BFS01         aCaptAttr.GetItemSet().Put(rSet);
//BFS01         aCaptAttr.GetItemSet().SetParent(pParent);
//BFS01         mpItemSet->Put(aCaptAttr);
//BFS01     }

//BFS01     void CaptionProperties::PostProcessSave()
//BFS01     {
//BFS01         // call parent
//BFS01         RectangleProperties::PostProcessSave();
//BFS01
//BFS01         // remove SetItems from local itemset
//BFS01         if(mpItemSet)
//BFS01         {
//BFS01             mpItemSet->ClearItem(SDRATTRSET_CAPTION);
//BFS01         }
//BFS01     }

        void CaptionProperties::ForceDefaultAttributes()
        {
            // call parent
            RectangleProperties::ForceDefaultAttributes();

            // force ItemSet
            GetObjectItemSet();

            // this was set by TextProperties::ForceDefaultAttributes(),
            // retet to default
            mpItemSet->ClearItem(XATTR_LINESTYLE);
        }
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
