/*************************************************************************
 *
 *  $RCSfile: measureproperties.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2003-11-24 16:51:02 $
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

#ifndef _SDR_PROPERTIES_MEASUREPROPERTIES_HXX
#include <svx/sdr/properties/measureproperties.hxx>
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

#ifndef _SVDOMEAS_HXX
#include <svdomeas.hxx>
#endif

#ifndef SXMSEITM_HXX
#include <sxmseitm.hxx>
#endif

#ifndef _SXMSUITM_HXX
#include <sxmsuitm.hxx>
#endif

#ifndef _SVX_XLNSTIT_HXX
#include <xlnstit.hxx>
#endif

#ifndef _SVX_XLNSTWIT_HXX
#include <xlnstwit.hxx>
#endif

#ifndef _SVX_XLNEDIT_HXX
#include <xlnedit.hxx>
#endif

#ifndef _SVX_XLNEDWIT_HXX
#include <xlnedwit.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        // create a new itemset
        SfxItemSet& MeasureProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            return *(new SfxItemSet(rPool,

                // range from SdrAttrObj
                SDRATTR_START, SDRATTRSET_SHADOW,
                SDRATTRSET_OUTLINER, SDRATTRSET_MISC,
                SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,

                // range from SdrMeasureObj
                SDRATTR_MEASURE_FIRST, SDRATTRSET_MEASURE,

                // range from SdrTextObj
                EE_ITEMS_START, EE_ITEMS_END,

                // end
                0, 0));
        }

        MeasureProperties::MeasureProperties(SdrObject& rObj)
        :   TextProperties(rObj)
        {
        }

        MeasureProperties::MeasureProperties(const MeasureProperties& rProps, SdrObject& rObj)
        :   TextProperties(rProps, rObj)
        {
        }

        MeasureProperties::~MeasureProperties()
        {
        }

        BaseProperties& MeasureProperties::Clone(SdrObject& rObj) const
        {
            return *(new MeasureProperties(*this, rObj));
        }

        void MeasureProperties::ItemSetChanged(const SfxItemSet& rSet)
        {
            SdrMeasureObj& rObj = (SdrMeasureObj&)GetSdrObject();

            // call parent
            TextProperties::ItemSetChanged(rSet);

            // local changes
            rObj.SetTextDirty();
        }

        void MeasureProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr)
        {
            SdrMeasureObj& rObj = (SdrMeasureObj&)GetSdrObject();

            // local changes
            rObj.SetTextDirty();

            // call parent
            TextProperties::SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);
        }

        void MeasureProperties::PreProcessSave()
        {
            // call parent
            TextProperties::PreProcessSave();

            // prepare SetItems for storage
            const SfxItemSet& rSet = *mpItemSet;
            const SfxItemSet* pParent = mpStyleSheet ? &(mpStyleSheet->GetItemSet()) : 0L;

            SdrMeasureSetItem aMeasAttr(rSet.GetPool());
            aMeasAttr.GetItemSet().Put(rSet);
            aMeasAttr.GetItemSet().SetParent(pParent);
            mpItemSet->Put(aMeasAttr);
        }

        void MeasureProperties::PostProcessSave()
        {
            // call parent
            TextProperties::PostProcessSave();

            // remove SetItems from local itemset
            if(mpItemSet)
            {
                mpItemSet->ClearItem(SDRATTRSET_MEASURE);
            }
        }

        void MeasureProperties::ForceDefaultAttributes()
        {
            // call parent
            TextProperties::ForceDefaultAttributes();

            //#71958# by default, the show units Bool-Item is set as hard
            // attribute to TRUE to aviod confusion when copying SdrMeasureObj's
            // from one application to another
            GetObjectItemSet();
            mpItemSet->Put(SdrMeasureShowUnitItem(TRUE));

            XPolygon aXP(4);            //      []
            aXP[0] = Point(100,0);      // 0,4__[]__2,4
            aXP[1] = Point(200,400);    //    \    /
            aXP[2] = Point(0,400);      //     \  /
            aXP[3] = Point(100,0);      //      \/1,0

            mpItemSet->Put(XLineStartItem(String(), aXP));
            mpItemSet->Put(XLineStartWidthItem(200));
            mpItemSet->Put(XLineEndItem(String(), aXP));
            mpItemSet->Put(XLineEndWidthItem(200));
            mpItemSet->Put(XLineStyleItem(XLINE_SOLID));
        }
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

// eof
