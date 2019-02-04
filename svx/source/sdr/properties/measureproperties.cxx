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

#include <sal/config.h>

#include <sdr/properties/measureproperties.hxx>
#include <svl/itemset.hxx>
#include <svl/style.hxx>
#include <svx/svddef.hxx>
#include <editeng/eeitem.hxx>
#include <svx/svdomeas.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xlnedwit.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>


namespace sdr
{
    namespace properties
    {
        // create a new itemset
        std::unique_ptr<SfxItemSet> MeasureProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            return std::make_unique<SfxItemSet>(
                rPool,
                svl::Items<
                    // Ranges from SdrAttrObj, SdrMeasureObj:
                    SDRATTR_START, SDRATTR_SHADOW_LAST,
                    SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
                    SDRATTR_MEASURE_FIRST, SDRATTR_MEASURE_LAST,
                    SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,
                    // Range from SdrTextObj:
                    EE_ITEMS_START, EE_ITEMS_END>{});
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

        std::unique_ptr<BaseProperties> MeasureProperties::Clone(SdrObject& rObj) const
        {
            return std::unique_ptr<BaseProperties>(new MeasureProperties(*this, rObj));
        }

        void MeasureProperties::ItemSetChanged(const SfxItemSet& rSet)
        {
            SdrMeasureObj& rObj = static_cast<SdrMeasureObj&>(GetSdrObject());

            // call parent
            TextProperties::ItemSetChanged(rSet);

            // local changes
            rObj.SetTextDirty();
        }

        void MeasureProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr)
        {
            // call parent (always first thing to do, may create the SfxItemSet)
            TextProperties::SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);

            // local changes
            // get access to dimension line object
            SdrMeasureObj& rObj = static_cast<SdrMeasureObj&>(GetSdrObject());

            // mark dimension line text as changed (dirty) in the dimension line object
            rObj.SetTextDirty();

            // tdf#98525 ask the dimension line object to redraw the changed text
            rObj.UndirtyText();
        }

        void MeasureProperties::ForceDefaultAttributes()
        {
            // call parent
            TextProperties::ForceDefaultAttributes();

            // force ItemSet
            GetObjectItemSet();

            //#71958# by default, the show units Bool-Item is set as hard
            // attribute to sal_True to avoid confusion when copying SdrMeasureObj's
            // from one application to another
            mpItemSet->Put(SdrYesNoItem(SDRATTR_MEASURESHOWUNIT, true));

            basegfx::B2DPolygon aNewPolygon;
            aNewPolygon.append(basegfx::B2DPoint(100.0, 0.0));
            aNewPolygon.append(basegfx::B2DPoint(200.0, 400.0));
            aNewPolygon.append(basegfx::B2DPoint(0.0, 400.0));
            aNewPolygon.setClosed(true);

            mpItemSet->Put(XLineStartItem(OUString(), basegfx::B2DPolyPolygon(aNewPolygon)));
            mpItemSet->Put(XLineStartWidthItem(200));
            mpItemSet->Put(XLineEndItem(OUString(), basegfx::B2DPolyPolygon(aNewPolygon)));
            mpItemSet->Put(XLineEndWidthItem(200));
            mpItemSet->Put(XLineStyleItem(css::drawing::LineStyle_SOLID));
        }
    } // end of namespace properties
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
