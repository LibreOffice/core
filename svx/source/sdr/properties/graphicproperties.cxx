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
#include <svx/sdr/properties/graphicproperties.hxx>
#include <svl/itemset.hxx>
#include <svl/style.hxx>
#include <svx/svddef.hxx>
#include <editeng/eeitem.hxx>
#include <svx/svdograf.hxx>
#include <svx/sdgcpitm.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        // create a new itemset
        SfxItemSet& GraphicProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            return *(new SfxItemSet(rPool,

                // range from SdrAttrObj
                SDRATTR_START, SDRATTR_SHADOW_LAST,
                SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
                SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,

                // range from SdrGrafObj
                SDRATTR_GRAF_FIRST, SDRATTR_GRAF_LAST,

                // range from SdrTextObj
                EE_ITEMS_START, EE_ITEMS_END,

                // end
                0, 0));
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

        BaseProperties& GraphicProperties::Clone(SdrObject& rObj) const
        {
            return *(new GraphicProperties(*this, rObj));
        }

        void GraphicProperties::ItemSetChanged(const SfxItemSet& rSet)
        {
            SdrGrafObj& rObj = (SdrGrafObj&)GetSdrObject();

            // local changes
            rObj.SetXPolyDirty();

            // #i29367# Update GraphicAttr, too. This was formerly
            // triggered by SdrGrafObj::Notify, which is no longer
            // called nowadays. BTW: strictly speaking, the whole
            // ImpSetAttrToGrafInfo/ImpSetGrafInfoToAttr stuff could
            // be dumped, when SdrGrafObj::aGrafInfo is removed and
            // always created on the fly for repaint.
            rObj.ImpSetAttrToGrafInfo();

            // call parent
            RectangleProperties::ItemSetChanged(rSet);
        }

        void GraphicProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr)
        {
            SdrGrafObj& rObj = (SdrGrafObj&)GetSdrObject();

            // local changes
            rObj.SetXPolyDirty();

            // call parent
            RectangleProperties::SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);

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
            mpItemSet->Put( SdrGrafInvertItem( sal_False ) );
            mpItemSet->Put( SdrGrafModeItem( GRAPHICDRAWMODE_STANDARD ) );
            mpItemSet->Put( SdrGrafCropItem( 0, 0, 0, 0 ) );

            // #i25616#
            mpItemSet->Put( XFillStyleItem(XFILL_NONE) );
            mpItemSet->Put( XLineStyleItem(XLINE_NONE) );
        }
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
