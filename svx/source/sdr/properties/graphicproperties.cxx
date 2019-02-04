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

#include <sdr/properties/graphicproperties.hxx>
#include <svl/itemset.hxx>
#include <svl/style.hxx>
#include <svx/svddef.hxx>
#include <editeng/eeitem.hxx>
#include <svx/svdograf.hxx>
#include <svx/sdgcpitm.hxx>
#include <svx/svdmodel.hxx>

namespace sdr
{
    namespace properties
    {
        void GraphicProperties::applyDefaultStyleSheetFromSdrModel()
        {
            SfxStyleSheet* pStyleSheet(GetSdrObject().getSdrModelFromSdrObject().GetDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj());

            if(pStyleSheet)
            {
                // do not delete hard attributes when setting dsefault Style
                SetStyleSheet(pStyleSheet, true);
            }
            else
            {
                SetMergedItem(XFillStyleItem(com::sun::star::drawing::FillStyle_NONE));
                SetMergedItem(XLineStyleItem(com::sun::star::drawing::LineStyle_NONE));
            }
        }

        // create a new itemset
        std::unique_ptr<SfxItemSet> GraphicProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            return std::make_unique<SfxItemSet>(rPool,

                // range from SdrAttrObj
                svl::Items<SDRATTR_START, SDRATTR_SHADOW_LAST,
                SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
                SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,

                // range from SdrGrafObj
                SDRATTR_GRAF_FIRST, SDRATTR_GRAF_LAST,

                // range from SdrTextObj
                EE_ITEMS_START, EE_ITEMS_END>{});
        }

        GraphicProperties::GraphicProperties(SdrObject& rObj)
        :   RectangleProperties(rObj)
        {
        }

        GraphicProperties::GraphicProperties(const GraphicProperties& rProps, SdrObject& rObj)
        :   RectangleProperties(rProps, rObj)
        {
        }

        GraphicProperties::~GraphicProperties()
        {
        }

        std::unique_ptr<BaseProperties> GraphicProperties::Clone(SdrObject& rObj) const
        {
            return std::unique_ptr<BaseProperties>(new GraphicProperties(*this, rObj));
        }

        void GraphicProperties::ItemSetChanged(const SfxItemSet& rSet)
        {
            SdrGrafObj& rObj = static_cast<SdrGrafObj&>(GetSdrObject());

            // local changes
            rObj.SetXPolyDirty();

            // #i29367# Update GraphicAttr, too. This was formerly
            // triggered by SdrGrafObj::Notify, which is no longer
            // called nowadays. BTW: strictly speaking, the whole
            // ImpSetAttrToGrafInfostuff could
            // be dumped, when SdrGrafObj::aGrafInfo is removed and
            // always created on the fly for repaint.
            rObj.ImpSetAttrToGrafInfo();

            // call parent
            RectangleProperties::ItemSetChanged(rSet);
        }

        void GraphicProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr)
        {
            // call parent (always first thing to do, may create the SfxItemSet)
            RectangleProperties::SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);

            // local changes
            SdrGrafObj& rObj = static_cast<SdrGrafObj&>(GetSdrObject());
            rObj.SetXPolyDirty();

            // local changes
            rObj.ImpSetAttrToGrafInfo();
        }

        void GraphicProperties::ForceDefaultAttributes()
        {
            // call parent
            RectangleProperties::ForceDefaultAttributes();

            // force ItemSet
            GetObjectItemSet();

            mpItemSet->Put( SdrGrafLuminanceItem( 0 ) );
            mpItemSet->Put( SdrGrafContrastItem( 0 ) );
            mpItemSet->Put( SdrGrafRedItem( 0 ) );
            mpItemSet->Put( SdrGrafGreenItem( 0 ) );
            mpItemSet->Put( SdrGrafBlueItem( 0 ) );
            mpItemSet->Put( SdrGrafGamma100Item( 100 ) );
            mpItemSet->Put( SdrGrafTransparenceItem( 0 ) );
            mpItemSet->Put( SdrGrafInvertItem( false ) );
            mpItemSet->Put( SdrGrafModeItem( GraphicDrawMode::Standard ) );
            mpItemSet->Put( SdrGrafCropItem( 0, 0, 0, 0 ) );
        }
    } // end of namespace properties
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
