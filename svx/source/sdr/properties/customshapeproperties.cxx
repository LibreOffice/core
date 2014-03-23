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

#include <svx/sdr/properties/customshapeproperties.hxx>
#include <svl/itemset.hxx>
#include <svl/style.hxx>
#include <svx/svdoashp.hxx>
#include <editeng/eeitem.hxx>
#include <svx/sdtagitm.hxx>
#include <svl/whiter.hxx>
#include <svl/smplhint.hxx>



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
            rObj.bTextFrame = static_cast< const SdrTextAutoGrowHeightItem& >(GetObjectItemSet().Get(SDRATTR_TEXT_AUTOGROWHEIGHT)).GetValue();

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

        SfxItemSet& CustomShapeProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            return *(new SfxItemSet(rPool,

                // ranges from SdrAttrObj
                SDRATTR_START, SDRATTR_SHADOW_LAST,
                SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
                SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,

                // Graphic Attributes
                SDRATTR_GRAF_FIRST, SDRATTR_GRAF_LAST,

                // 3d Properties
                SDRATTR_3D_FIRST, SDRATTR_3D_LAST,

                // CustomShape properties
                SDRATTR_CUSTOMSHAPE_FIRST, SDRATTR_CUSTOMSHAPE_LAST,

                // range from SdrTextObj
                EE_ITEMS_START, EE_ITEMS_END,

                // end
                0, 0));
        }

        bool CustomShapeProperties::AllowItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem ) const
        {
            sal_Bool bAllowItemChange = sal_True;
            if ( !pNewItem )
            {
                if ( ( nWhich >= SDRATTR_CUSTOMSHAPE_FIRST ) && ( nWhich <= SDRATTR_CUSTOMSHAPE_LAST ) )
                    bAllowItemChange = sal_False;
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
                SfxItemSet aSet((SfxItemPool&)(*GetSdrObject().GetObjectItemPool()));
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
            // call parent
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

        BaseProperties& CustomShapeProperties::Clone(SdrObject& rObj) const
        {
            return *(new CustomShapeProperties(*this, rObj));
        }

        void CustomShapeProperties::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
        {
            TextProperties::Notify( rBC, rHint );

            sal_Bool bRemoveRenderGeometry = sal_False;
            const SfxStyleSheetHint *pStyleHint = PTR_CAST( SfxStyleSheetHint, &rHint );
            const SfxSimpleHint *pSimpleHint = PTR_CAST( SfxSimpleHint, &rHint );

            if ( pStyleHint && pStyleHint->GetStyleSheet() == GetStyleSheet() )
            {
                switch( pStyleHint->GetHint() )
                {
                    case SFX_STYLESHEET_MODIFIED :
                    case SFX_STYLESHEET_CHANGED  :
                        bRemoveRenderGeometry = sal_True;
                    break;
                };
            }
            else if ( pSimpleHint && pSimpleHint->GetId() == SFX_HINT_DATACHANGED )
            {
                bRemoveRenderGeometry = sal_True;
            }

                // update bTextFrame and RenderGeometry
            UpdateTextFrameStatus(bRemoveRenderGeometry);
        }
    } // end of namespace properties
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
