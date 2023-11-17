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

#include <DrawViewShell.hxx>
#include <com/sun/star/embed/EmbedMisc.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>
#include <sfx2/objsh.hxx>
#include <svx/svxids.hrc>
#include <svx/sdmetitm.hxx>
#include <editeng/flditem.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/sdtfsitm.hxx>
#include <svx/svdopath.hxx>
#include <svx/obj3d.hxx>
#include <svx/scene3d.hxx>

#include <app.hrc>

#include <anminfo.hxx>
#include <drawdoc.hxx>
#include <drawview.hxx>

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
    const size_t nMarkCount = rMarkList.GetMarkCount();

    if ( nMarkCount == 1 )
    {
        if( SfxItemState::DEFAULT == rSet.GetItemState( SID_BEZIER_EDIT ) ||
            SfxItemState::DEFAULT == rSet.GetItemState( SID_UNGROUP ) ||
            SfxItemState::DEFAULT == rSet.GetItemState( SID_ENTER_GROUP ) ||
            SfxItemState::DEFAULT == rSet.GetItemState( SID_NAME_GROUP ) ||

            // #i68101#
            SfxItemState::DEFAULT == rSet.GetItemState( SID_OBJECT_TITLE_DESCRIPTION ) ||

            SfxItemState::DEFAULT == rSet.GetItemState( SID_ATTR_FILL_STYLE ) ||
            SfxItemState::DEFAULT == rSet.GetItemState( SID_ATTR_FILL_USE_SLIDE_BACKGROUND ) ||
            SfxItemState::DEFAULT == rSet.GetItemState( SID_ATTR_FILL_TRANSPARENCE ) ||
            SfxItemState::DEFAULT == rSet.GetItemState( SID_ATTR_FILL_FLOATTRANSPARENCE ) ||
            SfxItemState::DEFAULT == rSet.GetItemState( SID_CHANGEBEZIER ) ||
            SfxItemState::DEFAULT == rSet.GetItemState( SID_CHANGEPOLYGON ) ||
            SfxItemState::DEFAULT == rSet.GetItemState( SID_LINEEND_POLYGON ) ||
            SfxItemState::DEFAULT == rSet.GetItemState( SID_MEASURE_DLG ) ||
            SfxItemState::DEFAULT == rSet.GetItemState( SID_CONNECTION_DLG ) ||
            SfxItemState::DEFAULT == rSet.GetItemState( SID_CONNECTION_NEW_ROUTING ) ||
            SfxItemState::DEFAULT == rSet.GetItemState( SID_OBJECT_SHEAR ) ||
            SfxItemState::DEFAULT == rSet.GetItemState( SID_OBJECT_ALIGN_LEFT ) ||
            SfxItemState::DEFAULT == rSet.GetItemState( SID_OBJECT_ALIGN_CENTER ) ||
            SfxItemState::DEFAULT == rSet.GetItemState( SID_OBJECT_ALIGN_RIGHT ) ||
            SfxItemState::DEFAULT == rSet.GetItemState( SID_OBJECT_ALIGN_UP ) ||
            SfxItemState::DEFAULT == rSet.GetItemState( SID_OBJECT_ALIGN_MIDDLE ) ||
            SfxItemState::DEFAULT == rSet.GetItemState( SID_OBJECT_ALIGN_DOWN ) ||
            SfxItemState::DEFAULT == rSet.GetItemState( SID_FRAME_TO_TOP ) ||
            SfxItemState::DEFAULT == rSet.GetItemState( SID_MOREFRONT ) ||
            SfxItemState::DEFAULT == rSet.GetItemState( SID_FRAME_UP ) ||
            SfxItemState::DEFAULT == rSet.GetItemState( SID_MOREBACK ) ||
            SfxItemState::DEFAULT == rSet.GetItemState( SID_FRAME_DOWN ) ||
            SfxItemState::DEFAULT == rSet.GetItemState( SID_FRAME_TO_BOTTOM ) ||
            SfxItemState::DEFAULT == rSet.GetItemState( SID_BEFORE_OBJ ) ||
            SfxItemState::DEFAULT == rSet.GetItemState( SID_BEHIND_OBJ ) ||
            SfxItemState::DEFAULT == rSet.GetItemState( SID_REVERSE_ORDER ) ||
            SfxItemState::DEFAULT == rSet.GetItemState( SID_ORIGINAL_SIZE ) ||
            SfxItemState::DEFAULT == rSet.GetItemState( SID_SAVE_GRAPHIC ) ||
            SfxItemState::DEFAULT == rSet.GetItemState( SID_COMPRESS_GRAPHIC ) ||
            SfxItemState::DEFAULT == rSet.GetItemState( SID_TEXTATTR_DLG ) ||
            SfxItemState::DEFAULT == rSet.GetItemState( SID_EXECUTE_ANIMATION_EFFECT ))
        {
            const SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
            const SdrGrafObj* pSdrGrafObj = dynamic_cast< const SdrGrafObj* >(pObj);
            const SdrOle2Obj* pSdrOle2Obj = dynamic_cast< const SdrOle2Obj* >(pObj);
            const SdAnimationInfo* pAnimationInfo
                = SdDrawDocument::GetAnimationInfo(rMarkList.GetMark(0)->GetMarkedSdrObj());
            SdrInventor nInv = pObj->GetObjInventor();
            SdrObjKind  nId  = pObj->GetObjIdentifier();
            SdrObjTransformInfoRec aInfoRec;
            pObj->TakeObjInfo( aInfoRec );

            // don't show original size entry if not possible
            if(pSdrOle2Obj)
            {
                if (pSdrOle2Obj->GetObjRef().is() &&
                    (pSdrOle2Obj->GetObjRef()->getStatus( pSdrOle2Obj->GetAspect() ) & embed::EmbedMisc::MS_EMBED_RECOMPOSEONRESIZE) )
                    rSet.DisableItem(SID_ORIGINAL_SIZE);
            }

            if(!pSdrGrafObj)
            {
                rSet.DisableItem(SID_SAVE_GRAPHIC);
                rSet.DisableItem(SID_COMPRESS_GRAPHIC);
            }

            if (!pAnimationInfo
                || pAnimationInfo->meClickAction == presentation::ClickAction::ClickAction_NONE
                // Sound does not work in edit mode
                || pAnimationInfo->meClickAction == presentation::ClickAction::ClickAction_SOUND
                // No point in exiting the presentation in edit mode
                || pAnimationInfo->meClickAction
                       == presentation::ClickAction::ClickAction_STOPPRESENTATION)
            {
                rSet.DisableItem(SID_EXECUTE_ANIMATION_EFFECT);
            }

            /* If it is not a group object or 3D object, we disable "enter
               group". */
            const auto* pSdrObjGroup = dynamic_cast<const SdrObjGroup*>(pObj);

            if( !( ( pSdrObjGroup != nullptr && nInv == SdrInventor::Default ) ||
                   DynCastE3dScene(pObj) ) )
            {
                rSet.DisableItem( SID_ENTER_GROUP );
            }

            // Don't allow enter Diagrams
            if(nullptr != pSdrObjGroup && pSdrObjGroup->isDiagram())
            {
                rSet.DisableItem( SID_ENTER_GROUP );
            }

            // If it is not a group object, we disable "ungroup"
            if(pSdrObjGroup == nullptr || nInv != SdrInventor::Default)
            {
                rSet.DisableItem(SID_UNGROUP);
            }

            // Support advanced DiagramHelper
            if(!pSdrObjGroup || !pSdrObjGroup->isDiagram())
            {
                rSet.DisableItem( SID_REGENERATE_DIAGRAM );
                rSet.DisableItem( SID_EDIT_DIAGRAM );
            }

            if( nInv == SdrInventor::Default &&
               (nId == SdrObjKind::Line ||
                nId == SdrObjKind::PolyLine ||
                nId == SdrObjKind::PathLine ||
                nId == SdrObjKind::FreehandLine ))
            {
                //rSet.DisableItem( SID_ATTRIBUTES_AREA ); // remove again!
                rSet.DisableItem( SID_ATTR_FILL_STYLE );
                rSet.DisableItem( SID_ATTR_FILL_USE_SLIDE_BACKGROUND );
                rSet.DisableItem( SID_ATTR_FILL_TRANSPARENCE );
                rSet.DisableItem( SID_ATTR_FILL_FLOATTRANSPARENCE );
            }
            if( (dynamic_cast< const SdrPathObj *>( pObj ) ==  nullptr&& !aInfoRec.bCanConvToPath) || dynamic_cast< const SdrObjGroup *>( pObj ) !=  nullptr ) // As long as JOE handles it incorrectly!
            { // JOE: a group object may can be converted into a PathObj
                rSet.DisableItem( SID_LINEEND_POLYGON );
            }
            if(nInv == SdrInventor::Default &&
               (nId == SdrObjKind::PathFill || nId == SdrObjKind::PathLine || !aInfoRec.bCanConvToPath))
                rSet.DisableItem( SID_CHANGEBEZIER );

            if( nInv == SdrInventor::Default &&
                ( nId == SdrObjKind::Polygon || nId == SdrObjKind::PolyLine || !aInfoRec.bCanConvToPoly ) &&
                !GetView()->IsVectorizeAllowed() )
            {
                rSet.DisableItem( SID_CHANGEPOLYGON );
            }

            if(nInv == SdrInventor::Default && nId == SdrObjKind::Table )
            {
                rSet.DisableItem( SID_TEXTATTR_DLG );
            }

            if( nInv != SdrInventor::Default || nId != SdrObjKind::Measure )
                rSet.DisableItem( SID_MEASURE_DLG );

            if( nInv != SdrInventor::Default || nId != SdrObjKind::Edge )
                rSet.DisableItem( SID_CONNECTION_DLG );
            else
            {
                bool bDisable = true;
                SfxItemSet aAttrSet( GetDoc()->GetPool() );
                GetView()->GetAttributes( aAttrSet );

                if( aAttrSet.GetItemState( SDRATTR_EDGELINE1DELTA ) >= SfxItemState::DEFAULT &&
                    aAttrSet.GetItemState( SDRATTR_EDGELINE2DELTA ) >= SfxItemState::DEFAULT &&
                    aAttrSet.GetItemState( SDRATTR_EDGELINE3DELTA ) >= SfxItemState::DEFAULT )
                {
                    ::tools::Long nVal1 = aAttrSet.Get( SDRATTR_EDGELINE1DELTA ).GetValue();
                    ::tools::Long nVal2 = aAttrSet.Get( SDRATTR_EDGELINE2DELTA ).GetValue();
                    ::tools::Long nVal3 = aAttrSet.Get( SDRATTR_EDGELINE3DELTA ).GetValue();
                    {
                        if( nVal1 != 0 || nVal2 != 0 || nVal3 != 0 )
                            bDisable = false;
                    }
                }
                if( bDisable )
                    rSet.DisableItem( SID_CONNECTION_NEW_ROUTING );
            }

            if ( nInv == SdrInventor::E3d                          ||
                 (!mpDrawView->IsConvertToPathObjPossible() &&
                  !mpDrawView->IsShearAllowed()                  &&
                  !mpDrawView->IsDistortAllowed()) )
            {
                rSet.DisableItem( SID_OBJECT_SHEAR );
            }

            if(dynamic_cast< const E3dCompoundObject *>( pObj ) !=  nullptr)
            {
                rSet.DisableItem( SID_OBJECT_ALIGN );
                rSet.DisableItem( SID_OBJECT_ALIGN_LEFT );
                rSet.DisableItem( SID_OBJECT_ALIGN_CENTER );
                rSet.DisableItem( SID_OBJECT_ALIGN_RIGHT );
                rSet.DisableItem( SID_OBJECT_ALIGN_UP );
                rSet.DisableItem( SID_OBJECT_ALIGN_MIDDLE );
                rSet.DisableItem( SID_OBJECT_ALIGN_DOWN );
                rSet.DisableItem( SID_FRAME_TO_TOP );
                rSet.DisableItem( SID_MOREFRONT );
                rSet.DisableItem( SID_FRAME_UP );
                rSet.DisableItem( SID_MOREBACK );
                rSet.DisableItem( SID_FRAME_DOWN );
                rSet.DisableItem( SID_FRAME_TO_BOTTOM );
                rSet.DisableItem( SID_BEFORE_OBJ );
                rSet.DisableItem( SID_BEHIND_OBJ );
                rSet.DisableItem( SID_REVERSE_ORDER );
                rSet.DisableItem( SID_POSITION );
            }
        }

        if( SfxItemState::DEFAULT == rSet.GetItemState( SID_DISMANTLE ) ||
            SfxItemState::DEFAULT == rSet.GetItemState( SID_BREAK ) )
        {
            if ( !mpDrawView->IsDismantlePossible() )
            {
                rSet.DisableItem( SID_DISMANTLE );
            }

            if ( !mpDrawView->IsDismantlePossible(true) &&
                 !mpDrawView->IsImportMtfPossible()     &&
                 !mpDrawView->IsBreak3DObjPossible() )
            {
                rSet.DisableItem( SID_BREAK );
            }
        }
        if( SfxItemState::DEFAULT == rSet.GetItemState( SID_MODIFY_FIELD ) )
        {
            OutlinerView* pOLV = mpDrawView->GetTextEditOutlinerView();

            if( pOLV )
            {
                const SvxFieldItem* pFldItem = pOLV->GetFieldAtSelection();

                if( !( pFldItem && (nullptr != dynamic_cast< const SvxDateField *>( pFldItem->GetField() ) ||
                                 nullptr != dynamic_cast< const SvxAuthorField *>( pFldItem->GetField() ) ||
                                 nullptr != dynamic_cast< const SvxExtFileField *>( pFldItem->GetField() ) ||
                                 nullptr != dynamic_cast< const SvxExtTimeField *>( pFldItem->GetField() ) ) ) )
                {
                    rSet.DisableItem( SID_MODIFY_FIELD );
                }
            }
            else
                rSet.DisableItem( SID_MODIFY_FIELD );
        }
        if( SfxItemState::DEFAULT == rSet.GetItemState( SID_OUTLINE_TEXT_AUTOFIT ) )
        {
            const SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
            const SdrTextFitToSizeTypeItem* pItem = pObj->GetMergedItemSet().GetItem<SdrTextFitToSizeTypeItem>(SDRATTR_TEXT_FITTOSIZE);
            const bool bSet = pItem && pItem->GetValue() != drawing::TextFitToSizeType_NONE;
            rSet.Put(SfxBoolItem(SID_OUTLINE_TEXT_AUTOFIT, bSet));
        }

        rSet.DisableItem(SID_GROUP);
        rSet.DisableItem(SID_TEXT_COMBINE);
        rSet.DisableItem(SID_COMBINE);
        rSet.DisableItem(SID_DISTRIBUTE_HLEFT);
        rSet.DisableItem(SID_DISTRIBUTE_HCENTER);
        rSet.DisableItem(SID_DISTRIBUTE_HDISTANCE);
        rSet.DisableItem(SID_DISTRIBUTE_HRIGHT);
        rSet.DisableItem(SID_DISTRIBUTE_VTOP);
        rSet.DisableItem(SID_DISTRIBUTE_VCENTER);
        rSet.DisableItem(SID_DISTRIBUTE_VDISTANCE);
        rSet.DisableItem(SID_DISTRIBUTE_VBOTTOM);
        rSet.DisableItem(SID_POLY_MERGE);
        rSet.DisableItem(SID_POLY_SUBSTRACT);
        rSet.DisableItem(SID_POLY_INTERSECT);
        rSet.DisableItem(SID_EQUALIZEWIDTH);
        rSet.DisableItem(SID_EQUALIZEHEIGHT);
        rSet.DisableItem(SID_CONNECT);
    }
    // multi-selection
    else if( nMarkCount > 1 )
    {
        // distribute dialog for 3+n objects
        if(nMarkCount <= 2)
        {
            rSet.DisableItem(SID_DISTRIBUTE_HLEFT);
            rSet.DisableItem(SID_DISTRIBUTE_HCENTER);
            rSet.DisableItem(SID_DISTRIBUTE_HDISTANCE);
            rSet.DisableItem(SID_DISTRIBUTE_HRIGHT);
            rSet.DisableItem(SID_DISTRIBUTE_VTOP);
            rSet.DisableItem(SID_DISTRIBUTE_VCENTER);
            rSet.DisableItem(SID_DISTRIBUTE_VDISTANCE);
            rSet.DisableItem(SID_DISTRIBUTE_VBOTTOM);
        }

        rSet.DisableItem( SID_LINEEND_POLYGON );
        rSet.DisableItem( SID_ENTER_GROUP );
        // Now names for objects have to be unique
        rSet.DisableItem( SID_NAME_GROUP );
        // #i68101#
        rSet.DisableItem( SID_OBJECT_TITLE_DESCRIPTION );
        rSet.DisableItem( SID_MODIFY_FIELD );

        {
            bool bText = false;
            bool bLine = false;
            bool bGroup = false;
            bool bDrawObj = false;
            bool b3dObj = false;
            bool bTable = false;
            bool bMeasureObj = false;
            bool bEdgeObj = false; // Connector
            bool bE3dCompoundObject = false;

            for( size_t i = 0; i < nMarkCount && !bText && i < 50; ++i )
            {
                SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
                SdrInventor nInv = pObj->GetObjInventor();
                SdrObjKind  nId  = pObj->GetObjIdentifier();

                if (nInv == SdrInventor::Default)
                {
                    switch (nId)
                    {
                        case SdrObjKind::Text: bText = true; break;

                        case SdrObjKind::Line: bLine = true; break;

                        case SdrObjKind::Edge: bEdgeObj = true; break;

                        case SdrObjKind::Measure: bMeasureObj = true; break;

                        case SdrObjKind::Rectangle:
                        case SdrObjKind::CircleOrEllipse:
                        case SdrObjKind::FreehandLine:
                        case SdrObjKind::FreehandFill:
                        case SdrObjKind::PathFill:
                        case SdrObjKind::PathLine:
                        case SdrObjKind::CircleSection:
                        case SdrObjKind::CircleArc:
                        case SdrObjKind::CircleCut: bDrawObj = true; break;

                        case SdrObjKind::Group: bGroup = true; break;

                        case SdrObjKind::Graphic: break;

                        case SdrObjKind::Table: bTable = true; break;
                        default: ;
                    }
                }
                else if (nInv == SdrInventor::E3d)
                {
                    if(DynCastE3dScene(pObj))
                        b3dObj = true;
                    else if(dynamic_cast< const E3dCompoundObject* >(pObj) !=  nullptr)
                        bE3dCompoundObject = true;
                }
            }
            if( bLine && !bText && !bDrawObj &&!b3dObj)
            {
                rSet.DisableItem( SID_ATTR_FILL_STYLE );
                rSet.DisableItem( SID_ATTR_FILL_USE_SLIDE_BACKGROUND );
                rSet.DisableItem( SID_ATTR_FILL_TRANSPARENCE );
                rSet.DisableItem( SID_ATTR_FILL_FLOATTRANSPARENCE );
            }
            if( !bEdgeObj )
                rSet.DisableItem( SID_CONNECTION_DLG );

            if (b3dObj)
            {
                rSet.DisableItem( SID_COMBINE );
                rSet.DisableItem(SID_POLY_MERGE);
                rSet.DisableItem(SID_POLY_SUBSTRACT);
                rSet.DisableItem(SID_POLY_INTERSECT);
                rSet.DisableItem(SID_EQUALIZEWIDTH);
                rSet.DisableItem(SID_EQUALIZEHEIGHT);
            }

            if (b3dObj                                        ||
                 (!mpDrawView->IsConvertToPathObjPossible() &&
                  !mpDrawView->IsShearAllowed()             &&
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

            if(bE3dCompoundObject)
            {
                rSet.DisableItem( SID_OBJECT_ALIGN );
                rSet.DisableItem( SID_OBJECT_ALIGN_LEFT );
                rSet.DisableItem( SID_OBJECT_ALIGN_CENTER );
                rSet.DisableItem( SID_OBJECT_ALIGN_RIGHT );
                rSet.DisableItem( SID_OBJECT_ALIGN_UP );
                rSet.DisableItem( SID_OBJECT_ALIGN_MIDDLE );
                rSet.DisableItem( SID_OBJECT_ALIGN_DOWN );
                rSet.DisableItem( SID_FRAME_TO_TOP );
                rSet.DisableItem( SID_MOREFRONT );
                rSet.DisableItem( SID_FRAME_UP );
                rSet.DisableItem( SID_MOREBACK );
                rSet.DisableItem( SID_FRAME_DOWN );
                rSet.DisableItem( SID_FRAME_TO_BOTTOM );
                rSet.DisableItem( SID_BEFORE_OBJ );
                rSet.DisableItem( SID_BEHIND_OBJ );
                rSet.DisableItem( SID_REVERSE_ORDER );
                rSet.DisableItem( SID_POSITION );
            }
        }

        if ( !mpDrawView->IsDismantlePossible() )
        {
            rSet.DisableItem( SID_DISMANTLE );
        }
        if ( !mpDrawView->IsDismantlePossible(true) &&
             !mpDrawView->IsImportMtfPossible()     &&
             !mpDrawView->IsBreak3DObjPossible() )
        {
            rSet.DisableItem( SID_BREAK );
        }
        if ( !mpDrawView->IsCombinePossible() )
        {
            rSet.DisableItem(SID_COMBINE);
            rSet.DisableItem(SID_POLY_MERGE);
            rSet.DisableItem(SID_POLY_SUBSTRACT);
            rSet.DisableItem(SID_POLY_INTERSECT);
            rSet.DisableItem(SID_EQUALIZEWIDTH);
            rSet.DisableItem(SID_EQUALIZEHEIGHT);
        }
        if ( !mpDrawView->IsCombinePossible(true) )
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
        rSet.DisableItem(SID_SETLAYER);
        rSet.DisableItem( SID_ENTER_GROUP );
        rSet.DisableItem( SID_CUT );
        rSet.DisableItem( SID_COPY );
        rSet.DisableItem( SID_DELETE );
        rSet.DisableItem( SID_ATTR_TRANSFORM );

        rSet.DisableItem( SID_OBJECT_ALIGN );
        rSet.DisableItem( SID_OBJECT_ALIGN_LEFT );
        rSet.DisableItem( SID_OBJECT_ALIGN_CENTER );
        rSet.DisableItem( SID_OBJECT_ALIGN_RIGHT );
        rSet.DisableItem( SID_OBJECT_ALIGN_UP );
        rSet.DisableItem( SID_OBJECT_ALIGN_MIDDLE );
        rSet.DisableItem( SID_OBJECT_ALIGN_DOWN );

        rSet.DisableItem( SID_FRAME_TO_TOP );
        rSet.DisableItem( SID_MOREFRONT );
        rSet.DisableItem( SID_FRAME_UP );
        rSet.DisableItem( SID_MOREBACK );
        rSet.DisableItem( SID_FRAME_DOWN );
        rSet.DisableItem( SID_FRAME_TO_BOTTOM );
        rSet.DisableItem( SID_BEFORE_OBJ );
        rSet.DisableItem( SID_BEHIND_OBJ );
        rSet.DisableItem( SID_POSITION );

        rSet.DisableItem( SID_SIZE_OPTIMAL );
        rSet.DisableItem( SID_LINEEND_POLYGON );
        rSet.DisableItem( SID_COPYOBJECTS );
        rSet.DisableItem( SID_HORIZONTAL );
        rSet.DisableItem( SID_VERTICAL );
        rSet.DisableItem( SID_FLIP_HORIZONTAL );
        rSet.DisableItem( SID_FLIP_VERTICAL );
        rSet.DisableItem( SID_GROUP );
        rSet.DisableItem( SID_UNGROUP );
        rSet.DisableItem( SID_NAME_GROUP );

        // #i68101#
        rSet.DisableItem( SID_OBJECT_TITLE_DESCRIPTION );

        rSet.DisableItem( SID_DISMANTLE );
        rSet.DisableItem( SID_BREAK );
        rSet.DisableItem( SID_TEXT_COMBINE );
        rSet.DisableItem( SID_COMBINE );
        rSet.DisableItem(SID_DISTRIBUTE_DLG);
        rSet.DisableItem(SID_DISTRIBUTE_HLEFT);
        rSet.DisableItem(SID_DISTRIBUTE_HCENTER);
        rSet.DisableItem(SID_DISTRIBUTE_HDISTANCE);
        rSet.DisableItem(SID_DISTRIBUTE_HRIGHT);
        rSet.DisableItem(SID_DISTRIBUTE_VTOP);
        rSet.DisableItem(SID_DISTRIBUTE_VCENTER);
        rSet.DisableItem(SID_DISTRIBUTE_VDISTANCE);
        rSet.DisableItem(SID_DISTRIBUTE_VBOTTOM);
        rSet.DisableItem(SID_POLY_MERGE);
        rSet.DisableItem(SID_POLY_SUBSTRACT);
        rSet.DisableItem(SID_POLY_INTERSECT);
        rSet.DisableItem(SID_EQUALIZEWIDTH);
        rSet.DisableItem(SID_EQUALIZEHEIGHT);
        rSet.DisableItem( SID_CONNECT );
        rSet.DisableItem( SID_ANIMATION_EFFECTS );
        rSet.DisableItem( SID_EXECUTE_ANIMATION_EFFECT );
        rSet.DisableItem( SID_MODIFY_FIELD );
        rSet.DisableItem (SID_OBJECT_SHEAR);
    }

    if (GetObjectShell()->isContentExtractionLocked())
    {
        rSet.DisableItem(SID_COPY);
        rSet.DisableItem(SID_CUT);
    }
    if(GetObjectShell()->isExportLocked())
    {
        rSet.DisableItem(SID_SAVE_GRAPHIC);
        rSet.DisableItem(SID_EXTERNAL_EDIT);
    }
    if (GetDoc()->getImagePreferredDPI() <= 0)
    {
        rSet.DisableItem(SID_GRAPHIC_SIZE_CHECK);
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
