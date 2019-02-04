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

#include <sdr/properties/customshapeproperties.hxx>
#include <svl/itemset.hxx>
#include <svl/style.hxx>
#include <svx/svdoashp.hxx>
#include <editeng/eeitem.hxx>
#include <svx/sdtagitm.hxx>
#include <svl/whiter.hxx>
#include <svl/hint.hxx>


namespace sdr
{
    namespace properties
    {
        void CustomShapeProperties::UpdateTextFrameStatus(bool bInvalidateRenderGeometry)
        {
            SdrObjCustomShape& rObj = static_cast< SdrObjCustomShape& >(GetSdrObject());
            const bool bOld(rObj.bTextFrame);

            // change TextFrame flag when bResizeShapeToFitText changes (which is mapped
            // on the item SDRATTR_TEXT_AUTOGROWHEIGHT for custom shapes, argh)
            rObj.bTextFrame = GetObjectItemSet().Get(SDRATTR_TEXT_AUTOGROWHEIGHT).GetValue();

            // check if it did change
            if(rObj.bTextFrame != bOld)
            {
                // on change also invalidate render geometry
                bInvalidateRenderGeometry = true;

                // #115391# Potential recursion, since it calls SetObjectItemSet again, but rObj.bTextFrame
                // will not change again. Thus it will be only one level and terminate safely
                rObj.AdaptTextMinSize();
            }

            if(bInvalidateRenderGeometry)
            {
                // if asked for or bResizeShapeToFitText changed, make sure that
                // the render geometry is reconstructed using changed parameters
                rObj.InvalidateRenderGeometry();
            }
        }

        std::unique_ptr<SfxItemSet> CustomShapeProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            return std::make_unique<SfxItemSet>(
                rPool,
                svl::Items<
                    // Ranges from SdrAttrObj:
                    SDRATTR_START, SDRATTR_SHADOW_LAST,
                    SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
                    SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,
                    // Graphic attributes, 3D properties, CustomShape
                    // properties:
                    SDRATTR_GRAF_FIRST, SDRATTR_CUSTOMSHAPE_LAST,
                    // Range from SdrTextObj:
                    EE_ITEMS_START, EE_ITEMS_END>{});
        }

        bool CustomShapeProperties::AllowItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem ) const
        {
            bool bAllowItemChange = true;
            if ( !pNewItem )
            {
                if ( ( nWhich >= SDRATTR_CUSTOMSHAPE_FIRST ) && ( nWhich <= SDRATTR_CUSTOMSHAPE_LAST ) )
                    bAllowItemChange = false;
            }
            if ( bAllowItemChange )
                bAllowItemChange = TextProperties::AllowItemChange( nWhich, pNewItem );
            return bAllowItemChange;
        }

        void CustomShapeProperties::ClearObjectItem(const sal_uInt16 nWhich)
        {
            if ( !nWhich )
            {
                SfxWhichIter aIter( *mpItemSet );
                sal_uInt16 nWhich2 = aIter.FirstWhich();
                while( nWhich2 )
                {
                    TextProperties::ClearObjectItemDirect( nWhich2 );
                    nWhich2 = aIter.NextWhich();
                }
                SfxItemSet aSet(GetSdrObject().GetObjectItemPool());
                ItemSetChanged(aSet);
            }
            else
                TextProperties::ClearObjectItem( nWhich );
        }

        void CustomShapeProperties::ClearObjectItemDirect(const sal_uInt16 nWhich)
        {
            if ( !nWhich )
            {
                SfxWhichIter aIter( *mpItemSet );
                sal_uInt16 nWhich2 = aIter.FirstWhich();
                while( nWhich2 )
                {
                    TextProperties::ClearObjectItemDirect( nWhich2 );
                    nWhich2 = aIter.NextWhich();
                }
            }
            else
                TextProperties::ClearObjectItemDirect( nWhich );
        }

        void CustomShapeProperties::ItemSetChanged(const SfxItemSet& rSet)
        {
            // call parent
            TextProperties::ItemSetChanged(rSet);

            // update bTextFrame and RenderGeometry
            UpdateTextFrameStatus(true);
        }

        void CustomShapeProperties::PostItemChange(const sal_uInt16 nWhich)
        {
            switch(nWhich)
            {
                case SDRATTR_TEXT_AUTOGROWHEIGHT:
                {
                    // #115391#  update bTextFrame and RenderGeometry using AdaptTextMinSize()
                    UpdateTextFrameStatus(false);
                    break;
                }
                default:
                {
                    break;
                }
            }

            // call parent
            TextProperties::PostItemChange(nWhich);
        }

        void CustomShapeProperties::ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem)
        {
            // call parent
            TextProperties::ItemChange( nWhich, pNewItem );

            // update bTextFrame and RenderGeometry
            UpdateTextFrameStatus(true);
        }

        void CustomShapeProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr)
        {
            // call parent (always first thing to do, may create the SfxItemSet)
            TextProperties::SetStyleSheet( pNewStyleSheet, bDontRemoveHardAttr );

            // update bTextFrame and RenderGeometry
            UpdateTextFrameStatus(true);
        }

        void CustomShapeProperties::ForceDefaultAttributes()
        {
            // update bTextFrame and RenderGeometry
            UpdateTextFrameStatus(true);

            // SJ: Following is no good if creating customshapes, leading to objects that are white after loading via xml
            // This means: Do *not* call parent here is by purpose...
        }

        CustomShapeProperties::CustomShapeProperties(SdrObject& rObj)
        :   TextProperties(rObj)
        {
        }

        CustomShapeProperties::CustomShapeProperties(const CustomShapeProperties& rProps, SdrObject& rObj)
        :   TextProperties(rProps, rObj)
        {
        }

        CustomShapeProperties::~CustomShapeProperties()
        {
        }

        std::unique_ptr<BaseProperties> CustomShapeProperties::Clone(SdrObject& rObj) const
        {
            return std::unique_ptr<BaseProperties>(new CustomShapeProperties(*this, rObj));
        }

        void CustomShapeProperties::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
        {
            TextProperties::Notify( rBC, rHint );

            bool bRemoveRenderGeometry = false;
            const SfxStyleSheetHint* pStyleHint = dynamic_cast<const SfxStyleSheetHint*>(&rHint);

            if ( pStyleHint && pStyleHint->GetStyleSheet() == GetStyleSheet() )
            {
                switch( pStyleHint->GetId() )
                {
                    case SfxHintId::StyleSheetModified :
                    case SfxHintId::StyleSheetChanged  :
                        bRemoveRenderGeometry = true;
                    break;
                    default: break;
                };
            }
            else if ( rHint.GetId() == SfxHintId::DataChanged )
            {
                bRemoveRenderGeometry = true;
            }

                // update bTextFrame and RenderGeometry
            UpdateTextFrameStatus(bRemoveRenderGeometry);
        }
    } // end of namespace properties
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
