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

#include "DrawViewShell.hxx"
#include <com/sun/star/embed/EmbedMisc.hpp>
#include <svl/aeitem.hxx>
#include <svx/svxids.hrc>
#include <svx/globl3d.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/sxelditm.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <svx/svdopath.hxx>
#include <svx/polysc3d.hxx>
#include <svx/obj3d.hxx>
#include <sfx2/event.hxx>
#include <sfx2/docfile.hxx>
#include <rtl/ustrbuf.hxx>


#include "app.hrc"

#include "Outliner.hxx"
#include "sdpage.hxx"
#include "fupoor.hxx"
#include "fusel.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "drawview.hxx"
#include "optsitem.hxx"

using namespace com::sun::star;

namespace sd {

/**
 * Set state (Enabled/Disabled) of Menu-SfxSlots
 */
void DrawViewShell::GetMenuStateSel( SfxItemSet &rSet )
{
    // Status of menu entries (Buttons,...)

    // Single selection
    const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
    sal_uLong nMarkCount = rMarkList.GetMarkCount();

    if ( nMarkCount == 1 )
    {
        if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_BEZIER_EDIT ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_UNGROUP ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_ENTER_GROUP ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_NAME_GROUP ) ||

            // #i68101#
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_OBJECT_TITLE_DESCRIPTION ) ||

            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_ATTR_FILL_STYLE ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_CHANGEBEZIER ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_CHANGEPOLYGON ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_LINEEND_POLYGON ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( OBJ_TITLETEXT ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( OBJ_OUTLINETEXT ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_MEASURE_DLG ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_CONNECTION_DLG ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_CONNECTION_NEW_ROUTING ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_OBJECT_SHEAR ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_CONVERT_TO_1BIT_THRESHOLD ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_CONVERT_TO_1BIT_MATRIX ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_CONVERT_TO_4BIT_GRAYS ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_CONVERT_TO_4BIT_COLORS ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_CONVERT_TO_8BIT_GRAYS ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_CONVERT_TO_8BIT_COLORS ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_CONVERT_TO_24BIT ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_OBJECT_ALIGN_LEFT ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_OBJECT_ALIGN_CENTER ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_OBJECT_ALIGN_RIGHT ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_OBJECT_ALIGN_UP ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_OBJECT_ALIGN_MIDDLE ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_OBJECT_ALIGN_DOWN ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_FRAME_TO_TOP ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_MOREFRONT ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_MOREBACK ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_FRAME_TO_BOTTOM ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_BEFORE_OBJ ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_BEHIND_OBJ ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_REVERSE_ORDER ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_ORIGINAL_SIZE ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_SAVE_GRAPHIC ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_COMPRESS_GRAPHIC ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_TEXTATTR_DLG ) )
        {
            const SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
            const SdrGrafObj* pSdrGrafObj = dynamic_cast< const SdrGrafObj* >(pObj);
            const SdrOle2Obj* pSdrOle2Obj = dynamic_cast< const SdrOle2Obj* >(pObj);
            sal_uInt32 nInv = pObj->GetObjInventor();
            sal_uInt16 nId = pObj->GetObjIdentifier();
            SdrObjTransformInfoRec aInfoRec;
            pObj->TakeObjInfo( aInfoRec );

            // don't show original size entry if not possible
            if(pSdrOle2Obj)
            {
                if (pSdrOle2Obj->GetObjRef().is() &&
                    ((pSdrOle2Obj->GetObjRef()->getStatus( pSdrOle2Obj->GetAspect() ) & embed::EmbedMisc::MS_EMBED_RECOMPOSEONRESIZE) ) )
                    rSet.DisableItem(SID_ORIGINAL_SIZE);
            }

            if(!pSdrGrafObj)
            {
                rSet.DisableItem(SID_SAVE_GRAPHIC);
                rSet.DisableItem(SID_COMPRESS_GRAPHIC);
            }

            /* If it is not a group object or 3D object, we disable "enter
               group". */
            if( !( ( pObj->ISA( SdrObjGroup ) && nInv == SdrInventor ) ||
                (pObj->ISA (E3dPolyScene) || pObj->ISA (E3dScene) /*|| pObj->ISA (E3dCompoundObject) */) ) )
            {
                rSet.DisableItem( SID_ENTER_GROUP );
            }

            // If it is not a group object, we disable "ungroup"
            if (!(pObj->ISA(SdrObjGroup) && nInv == SdrInventor))
            {
                rSet.DisableItem(SID_UNGROUP);
            }
            if(!pSdrGrafObj ||
                pSdrGrafObj->GetGraphicType() != GRAPHIC_BITMAP ||
                pSdrGrafObj->IsLinkedGraphic() ||
                pSdrGrafObj->isEmbeddedSvg())
            {
                rSet.DisableItem(SID_CONVERT_TO_1BIT_THRESHOLD);
                rSet.DisableItem(SID_CONVERT_TO_1BIT_MATRIX);
                rSet.DisableItem(SID_CONVERT_TO_4BIT_GRAYS);
                rSet.DisableItem(SID_CONVERT_TO_4BIT_COLORS);
                rSet.DisableItem(SID_CONVERT_TO_8BIT_GRAYS);
                rSet.DisableItem(SID_CONVERT_TO_8BIT_COLORS);
                rSet.DisableItem(SID_CONVERT_TO_24BIT);
            }

            if( nInv == SdrInventor &&
               (nId == OBJ_LINE ||
                nId == OBJ_PLIN ||
                nId == OBJ_PATHLINE ||
                nId == OBJ_FREELINE ))
            {
                //rSet.DisableItem( SID_ATTRIBUTES_AREA ); // remove again!
                rSet.DisableItem( SID_ATTR_FILL_STYLE );
            }
            if( (!pObj->ISA( SdrPathObj ) && !aInfoRec.bCanConvToPath) || pObj->ISA( SdrObjGroup ) ) // As long as JOE handles it incorrectly!
            { // JOE: a group object may can be converted into a PathObj
                rSet.DisableItem( SID_LINEEND_POLYGON );
            }
            if(nInv == SdrInventor &&
               (nId == OBJ_PATHFILL || nId == OBJ_PATHLINE || !aInfoRec.bCanConvToPath))
                rSet.DisableItem( SID_CHANGEBEZIER );

            if( nInv == SdrInventor &&
                ( nId == OBJ_POLY || nId == OBJ_PLIN || !aInfoRec.bCanConvToPoly ) &&
                !GetView()->IsVectorizeAllowed() )
            {
                rSet.DisableItem( SID_CHANGEPOLYGON );
            }

            if(nInv == SdrInventor && nId == OBJ_TABLE )
            {
                rSet.DisableItem( SID_TEXTATTR_DLG );
            }

            if( nInv != SdrInventor || nId != OBJ_MEASURE )
                rSet.DisableItem( SID_MEASURE_DLG );

            if( nInv != SdrInventor || nId != OBJ_EDGE )
                rSet.DisableItem( SID_CONNECTION_DLG );
            else
            {
                sal_Bool bDisable = sal_True;
                SfxItemSet aAttrSet( GetDoc()->GetPool() );
                GetView()->GetAttributes( aAttrSet );

                if( aAttrSet.GetItemState( SDRATTR_EDGELINE1DELTA ) >= SFX_ITEM_AVAILABLE &&
                    aAttrSet.GetItemState( SDRATTR_EDGELINE2DELTA ) >= SFX_ITEM_AVAILABLE &&
                    aAttrSet.GetItemState( SDRATTR_EDGELINE3DELTA ) >= SFX_ITEM_AVAILABLE )
                {
                    long nVal1 = ( ( const SdrEdgeLine1DeltaItem& ) aAttrSet.Get( SDRATTR_EDGELINE1DELTA ) ).GetValue();
                    long nVal2 = ( ( const SdrEdgeLine2DeltaItem& ) aAttrSet.Get( SDRATTR_EDGELINE2DELTA ) ).GetValue();
                    long nVal3 = ( ( const SdrEdgeLine3DeltaItem& ) aAttrSet.Get( SDRATTR_EDGELINE3DELTA ) ).GetValue();
                    {
                        if( nVal1 != 0 || nVal2 != 0 || nVal3 != 0 )
                            bDisable = sal_False;
                    }
                }
                if( bDisable )
                    rSet.DisableItem( SID_CONNECTION_NEW_ROUTING );
            }

            if ( nInv == E3dInventor                          ||
                 (!mpDrawView->IsConvertToPathObjPossible(sal_False) &&
                  !mpDrawView->IsShearAllowed()                  &&
                  !mpDrawView->IsDistortAllowed()) )
            {
                rSet.DisableItem( SID_OBJECT_SHEAR );
            }

            if(pObj->ISA(E3dCompoundObject))
            {
                rSet.DisableItem( SID_OBJECT_ALIGN_LEFT );
                rSet.DisableItem( SID_OBJECT_ALIGN_CENTER );
                rSet.DisableItem( SID_OBJECT_ALIGN_RIGHT );
                rSet.DisableItem( SID_OBJECT_ALIGN_UP );
                rSet.DisableItem( SID_OBJECT_ALIGN_MIDDLE );
                rSet.DisableItem( SID_OBJECT_ALIGN_DOWN );
                rSet.DisableItem( SID_FRAME_TO_TOP );
                rSet.DisableItem( SID_MOREFRONT );
                rSet.DisableItem( SID_MOREBACK );
                rSet.DisableItem( SID_FRAME_TO_BOTTOM );
                rSet.DisableItem( SID_BEFORE_OBJ );
                rSet.DisableItem( SID_BEHIND_OBJ );
                rSet.DisableItem( SID_REVERSE_ORDER );
            }
        }

        if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_DISMANTLE ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_BREAK ) )
        {
            if ( !mpDrawView->IsDismantlePossible(sal_False) )
            {
                rSet.DisableItem( SID_DISMANTLE );
            }

            if ( !mpDrawView->IsDismantlePossible(sal_True) &&
                 !mpDrawView->IsImportMtfPossible()     &&
                 !mpDrawView->IsBreak3DObjPossible() )
            {
                rSet.DisableItem( SID_BREAK );
            }
        }
        if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_MODIFY_FIELD ) )
        {
            OutlinerView* pOLV = mpDrawView->GetTextEditOutlinerView();

            if( pOLV )
            {
                const SvxFieldItem* pFldItem = pOLV->GetFieldAtSelection();

                if( !( pFldItem && (pFldItem->GetField()->ISA( SvxDateField ) ||
                                 pFldItem->GetField()->ISA( SvxAuthorField ) ||
                                 pFldItem->GetField()->ISA( SvxExtFileField ) ||
                                 pFldItem->GetField()->ISA( SvxExtTimeField ) ) ) )
                {
                    rSet.DisableItem( SID_MODIFY_FIELD );
                }
            }
            else
                rSet.DisableItem( SID_MODIFY_FIELD );
        }
        if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_OUTLINE_TEXT_AUTOFIT ) )
        {
            const SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
            const bool bSet = ((const SdrTextFitToSizeTypeItem*)pObj->GetMergedItemSet().GetItem(SDRATTR_TEXT_FITTOSIZE))->GetValue() != SDRTEXTFIT_NONE;
            rSet.Put(SfxBoolItem(SID_OUTLINE_TEXT_AUTOFIT, bSet));
        }

        rSet.DisableItem( SID_GROUP );
        rSet.DisableItem( SID_COMBINE );
        rSet.DisableItem(SID_DISTRIBUTE_DLG);
        rSet.DisableItem(SID_POLY_MERGE);
        rSet.DisableItem(SID_POLY_SUBSTRACT);
        rSet.DisableItem(SID_POLY_INTERSECT);
        rSet.DisableItem( SID_CONNECT );
    }
    // multi-selection
    else if( nMarkCount > 1 )
    {
        // distribure dialog for 3+n objects
        if(nMarkCount <= 2)
            rSet.DisableItem(SID_DISTRIBUTE_DLG);

        rSet.DisableItem( SID_LINEEND_POLYGON );
        rSet.DisableItem( SID_ENTER_GROUP );
        // Now names for objects have to be unique
        rSet.DisableItem( SID_NAME_GROUP );
        // #i68101#
        rSet.DisableItem( SID_OBJECT_TITLE_DESCRIPTION );
        rSet.DisableItem( SID_MODIFY_FIELD );

        {
            sal_Bool bText = sal_False;
            sal_Bool bLine = sal_False;
            sal_Bool bGroup = sal_False;
            sal_Bool bGraf = sal_False;
            sal_Bool bDrawObj = sal_False;
            sal_Bool b3dObj = sal_False;
            bool bTable = false;
            sal_Bool bMeasureObj = sal_False;
            sal_Bool bEdgeObj = sal_False; // Connector
            sal_Bool bE3dCompoundObject = sal_False;

            for( sal_uLong i = 0;
                 i < nMarkCount && !bText && i < 50;
                 i++ )
            {
                SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
                sal_uInt32 nInv = pObj->GetObjInventor();
                sal_uInt16 nId = pObj->GetObjIdentifier();

                if (nInv == SdrInventor)
                {
                    switch (nId)
                    {
                        case OBJ_TEXT: bText = sal_True; break;

                        case OBJ_LINE: bLine = sal_True; break;

                        case OBJ_EDGE: bEdgeObj = sal_True; break;

                        case OBJ_MEASURE: bMeasureObj = sal_True; break;

                        case OBJ_RECT:
                        case OBJ_CIRC:
                        case OBJ_FREELINE:
                        case OBJ_FREEFILL:
                        case OBJ_PATHFILL:
                        case OBJ_PATHLINE:
                        case OBJ_SECT:
                        case OBJ_CARC:
                        case OBJ_CCUT: bDrawObj = sal_True; break;

                        case OBJ_GRUP: bGroup = sal_True; break;

                        case OBJ_GRAF: bGraf = sal_True; break;

                        case OBJ_TABLE: bTable = true; break;
                    }
                }
                else if (nInv == E3dInventor)
                {
                    if(pObj->ISA(E3dScene))
                        b3dObj = sal_True;
                    else if(pObj->ISA(E3dCompoundObject))
                        bE3dCompoundObject = sal_True;
                }
            }
            if( bLine && !bText && !bDrawObj &&!b3dObj)
            {
                rSet.DisableItem( SID_ATTR_FILL_STYLE );
            }
            if( !bEdgeObj )
                rSet.DisableItem( SID_CONNECTION_DLG );

            if (b3dObj)
            {
                rSet.DisableItem( SID_COMBINE );
                rSet.DisableItem(SID_POLY_MERGE);
                rSet.DisableItem(SID_POLY_SUBSTRACT);
                rSet.DisableItem(SID_POLY_INTERSECT);
            }

            if (b3dObj                                        ||
                 (!mpDrawView->IsConvertToPathObjPossible(sal_False) &&
                  !mpDrawView->IsShearAllowed()                  &&
                  !mpDrawView->IsDistortAllowed()) )
            {
                rSet.DisableItem( SID_OBJECT_SHEAR );
            }

            if( !bGroup )
            {
                rSet.DisableItem( SID_UNGROUP );
            }
            if( bTable )
                rSet.DisableItem( SID_TEXTATTR_DLG );

            if( !bMeasureObj )
                rSet.DisableItem( SID_MEASURE_DLG );

            if (!bGraf)
            {
                rSet.DisableItem(SID_CONVERT_TO_1BIT_THRESHOLD);
                rSet.DisableItem(SID_CONVERT_TO_1BIT_MATRIX);
                rSet.DisableItem(SID_CONVERT_TO_4BIT_GRAYS);
                rSet.DisableItem(SID_CONVERT_TO_4BIT_COLORS);
                rSet.DisableItem(SID_CONVERT_TO_8BIT_GRAYS);
                rSet.DisableItem(SID_CONVERT_TO_8BIT_COLORS);
                rSet.DisableItem(SID_CONVERT_TO_24BIT);
            }

            if(bE3dCompoundObject)
            {
                rSet.DisableItem( SID_OBJECT_ALIGN_LEFT );
                rSet.DisableItem( SID_OBJECT_ALIGN_CENTER );
                rSet.DisableItem( SID_OBJECT_ALIGN_RIGHT );
                rSet.DisableItem( SID_OBJECT_ALIGN_UP );
                rSet.DisableItem( SID_OBJECT_ALIGN_MIDDLE );
                rSet.DisableItem( SID_OBJECT_ALIGN_DOWN );
                rSet.DisableItem( SID_FRAME_TO_TOP );
                rSet.DisableItem( SID_MOREFRONT );
                rSet.DisableItem( SID_MOREBACK );
                rSet.DisableItem( SID_FRAME_TO_BOTTOM );
                rSet.DisableItem( SID_BEFORE_OBJ );
                rSet.DisableItem( SID_BEHIND_OBJ );
                rSet.DisableItem( SID_REVERSE_ORDER );
            }
        }

        if ( !mpDrawView->IsDismantlePossible(sal_False) )
        {
            rSet.DisableItem( SID_DISMANTLE );
        }
        if ( !mpDrawView->IsDismantlePossible(sal_True) &&
             !mpDrawView->IsImportMtfPossible()     &&
             !mpDrawView->IsBreak3DObjPossible() )
        {
            rSet.DisableItem( SID_BREAK );
        }
        if ( !mpDrawView->IsCombinePossible(sal_False) )
        {
            rSet.DisableItem( SID_COMBINE );
            rSet.DisableItem(SID_POLY_MERGE);
            rSet.DisableItem(SID_POLY_SUBSTRACT);
            rSet.DisableItem(SID_POLY_INTERSECT);
        }
        if ( !mpDrawView->IsCombinePossible(sal_True) )
        {
            rSet.DisableItem( SID_CONNECT );
        }
        if ( !mpDrawView->IsGroupPossible() )
        {
            rSet.DisableItem( SID_GROUP );
        }
        if ( !mpDrawView->IsUnGroupPossible() )
        {
            rSet.DisableItem( SID_UNGROUP );
        }
    }
    // select no object
    else
    {
        rSet.DisableItem( SID_ENTER_GROUP );
        rSet.DisableItem( SID_CUT );
        rSet.DisableItem( SID_COPY );
        rSet.DisableItem( SID_DELETE );
        rSet.DisableItem( SID_ATTR_TRANSFORM );

        rSet.DisableItem( SID_OBJECT_ALIGN_LEFT );
        rSet.DisableItem( SID_OBJECT_ALIGN_CENTER );
        rSet.DisableItem( SID_OBJECT_ALIGN_RIGHT );
        rSet.DisableItem( SID_OBJECT_ALIGN_UP );
        rSet.DisableItem( SID_OBJECT_ALIGN_MIDDLE );
        rSet.DisableItem( SID_OBJECT_ALIGN_DOWN );

        rSet.DisableItem( SID_FRAME_TO_TOP );
        rSet.DisableItem( SID_MOREFRONT );
        rSet.DisableItem( SID_MOREBACK );
        rSet.DisableItem( SID_FRAME_TO_BOTTOM );
        rSet.DisableItem( SID_BEFORE_OBJ );
        rSet.DisableItem( SID_BEHIND_OBJ );
        rSet.DisableItem( SID_CONVERT );

        rSet.DisableItem( SID_SIZE_OPTIMAL );
        rSet.DisableItem( SID_LINEEND_POLYGON );
        rSet.DisableItem( SID_COPYOBJECTS );
        rSet.DisableItem( SID_HORIZONTAL );
        rSet.DisableItem( SID_VERTICAL );
        rSet.DisableItem( SID_GROUP );
        rSet.DisableItem( SID_UNGROUP );
        rSet.DisableItem( SID_NAME_GROUP );

        // #i68101#
        rSet.DisableItem( SID_OBJECT_TITLE_DESCRIPTION );

        rSet.DisableItem( SID_DISMANTLE );
        rSet.DisableItem( SID_BREAK );
        rSet.DisableItem( SID_COMBINE );
        rSet.DisableItem(SID_DISTRIBUTE_DLG);
        rSet.DisableItem(SID_POLY_MERGE);
        rSet.DisableItem(SID_POLY_SUBSTRACT);
        rSet.DisableItem(SID_POLY_INTERSECT);
        rSet.DisableItem( SID_CONNECT );
        rSet.DisableItem( SID_ANIMATION_EFFECTS );
        rSet.DisableItem( SID_MODIFY_FIELD );
        rSet.DisableItem (SID_OBJECT_SHEAR);
    }

}


} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
