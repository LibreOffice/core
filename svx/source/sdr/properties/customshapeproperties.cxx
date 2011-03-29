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
#include <svx/sdr/properties/customshapeproperties.hxx>
#include <svl/itemset.hxx>
#include <svl/style.hxx>
#include <svx/svdoashp.hxx>
#include <editeng/eeitem.hxx>
#include <svx/sdtagitm.hxx>
#include <svl/whiter.hxx>
#include <svl/itemset.hxx>
#include <svl/smplhint.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        void CustomShapeProperties::UpdateTextFrameStatus()
        {
            SdrTextObj& rObj = (SdrTextObj&)GetSdrObject();
            SdrTextAutoGrowHeightItem& rAutoGrowHeightItem =
                (SdrTextAutoGrowHeightItem&)rObj.GetMergedItem( SDRATTR_TEXT_AUTOGROWHEIGHT );
            rObj.bTextFrame = rAutoGrowHeightItem.GetValue() != 0;

            if ( rObj.bTextFrame )
                rObj.NbcAdjustTextFrameWidthAndHeight();
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
        sal_Bool CustomShapeProperties::AllowItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem ) const
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
            SdrObjCustomShape& rObj = (SdrObjCustomShape&)GetSdrObject();

            if( SFX_ITEM_SET == rSet.GetItemState( SDRATTR_TEXT_AUTOGROWHEIGHT ) )
            {
                rObj.bTextFrame = ((SdrTextAutoGrowHeightItem&)rSet.Get( SDRATTR_TEXT_AUTOGROWHEIGHT )).GetValue() != 0;
            }

            // call parent
            TextProperties::ItemSetChanged(rSet);

            // local changes, removing cached objects
            rObj.InvalidateRenderGeometry();
        }
        void CustomShapeProperties::ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem)
        {
            SdrObjCustomShape& rObj = (SdrObjCustomShape&)GetSdrObject();
            //OutlinerParaObject* pParaObj = rObj.GetOutlinerParaObject();

            if( pNewItem && ( SDRATTR_TEXT_AUTOGROWHEIGHT == nWhich ) )
            {
                rObj.bTextFrame = ((SdrTextAutoGrowHeightItem*)pNewItem)->GetValue() != 0;
            }
            // call parent
            TextProperties::ItemChange( nWhich, pNewItem );

            rObj.InvalidateRenderGeometry();
        }
        void CustomShapeProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr)
        {
            TextProperties::SetStyleSheet( pNewStyleSheet, bDontRemoveHardAttr );
            UpdateTextFrameStatus();
        }
        void CustomShapeProperties::ForceDefaultAttributes()
        {
            UpdateTextFrameStatus();

/* SJ: Following is no good if creating customshapes, leading to objects that are white after loading via xml

            SdrTextObj& rObj = (SdrTextObj&)GetSdrObject();
            sal_Bool bTextFrame(rObj.IsTextFrame());

            // force ItemSet
            GetObjectItemSet();

            if(bTextFrame)
            {
                mpItemSet->Put(XLineStyleItem(XLINE_NONE));
                mpItemSet->Put(XFillColorItem(String(), Color(COL_WHITE)));
                mpItemSet->Put(XFillStyleItem(XFILL_NONE));
            }
            else
            {
                mpItemSet->Put(SvxAdjustItem(SVX_ADJUST_CENTER));
                mpItemSet->Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_CENTER));
                mpItemSet->Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_CENTER));
            }
*/
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
            if ( bRemoveRenderGeometry )
            {
                UpdateTextFrameStatus();

                // local changes, removing cached objects
                SdrObjCustomShape& rObj = (SdrObjCustomShape&)GetSdrObject();
                rObj.InvalidateRenderGeometry();
            }

        }
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
