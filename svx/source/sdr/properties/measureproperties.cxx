/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: measureproperties.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 13:37:22 $
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

#ifndef _SDR_PROPERTIES_MEASUREPROPERTIES_HXX
#include <svx/sdr/properties/measureproperties.hxx>
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

#ifndef _SVDOMEAS_HXX
#include <svdomeas.hxx>
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

#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
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
                SDRATTR_START, SDRATTR_SHADOW_LAST,
                SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
                SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,

                // range from SdrMeasureObj
                SDRATTR_MEASURE_FIRST, SDRATTR_MEASURE_LAST,

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

        void MeasureProperties::ForceDefaultAttributes()
        {
            // call parent
            TextProperties::ForceDefaultAttributes();

            // force ItemSet
            GetObjectItemSet();

            //#71958# by default, the show units Bool-Item is set as hard
            // attribute to TRUE to aviod confusion when copying SdrMeasureObj's
            // from one application to another
            mpItemSet->Put(SdrMeasureShowUnitItem(TRUE));

            basegfx::B2DPolygon aNewPolygon;
            aNewPolygon.append(basegfx::B2DPoint(100.0, 0.0));
            aNewPolygon.append(basegfx::B2DPoint(200.0, 400.0));
            aNewPolygon.append(basegfx::B2DPoint(0.0, 400.0));
            aNewPolygon.setClosed(true);

            mpItemSet->Put(XLineStartItem(String(), basegfx::B2DPolyPolygon(aNewPolygon)));
            mpItemSet->Put(XLineStartWidthItem(200));
            mpItemSet->Put(XLineEndItem(String(), basegfx::B2DPolyPolygon(aNewPolygon)));
            mpItemSet->Put(XLineEndWidthItem(200));
            mpItemSet->Put(XLineStyleItem(XLINE_SOLID));
        }
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

// eof
