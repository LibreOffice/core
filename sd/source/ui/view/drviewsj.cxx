/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

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
#include <svx/scene3d.hxx>

using namespace com::sun::star;

namespace sd {

/*************************************************************************
|*
|* Status (Enabled/Disabled) von Menue-SfxSlots setzen
|*
\************************************************************************/

void DrawViewShell::GetMenuStateSel( SfxItemSet &rSet )
{
    // Status der Menueintraege, bzw. Buttons
    const SdrObject* pSingleObject = mpDrawView->getSelectedIfSingle();

    if(pSingleObject)
    {
        // Einfachselektion
        if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_BEZIER_EDIT ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_UNGROUP ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_ENTER_GROUP ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_NAME_GROUP ) ||
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
             SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_SAVEGRAPHIC ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_TEXTATTR_DLG ) )
        {
            const sal_uInt32 nInv = pSingleObject->GetObjInventor();
            const sal_uInt16 nId = pSingleObject->GetObjIdentifier();
            SdrObjTransformInfoRec aInfoRec;
            pSingleObject->TakeObjInfo( aInfoRec );

            // #91929#; don't show original size entry if not possible
            const SdrOle2Obj* pOleObj = dynamic_cast< const SdrOle2Obj* >(pSingleObject);

            if ( pOleObj )
            {
                if (pOleObj->GetObjRef().is() &&
                    ((pOleObj->GetObjRef()->getStatus( pOleObj->GetAspect() ) & embed::EmbedMisc::MS_EMBED_RECOMPOSEONRESIZE) ) )
                {
                    rSet.DisableItem(SID_ORIGINAL_SIZE);
                }
            }

            const SdrGrafObj* pSdrGrafObj = dynamic_cast< const SdrGrafObj* >(pSingleObject);

            if ( !pSdrGrafObj )
            {
                rSet.DisableItem(SID_SAVEGRAPHIC);
            }

            // Wenn es sich um kein Gruppenobjekt oder 3D-Objekt handelt
            // wird "Gruppe betreten" disabled
            if( ! pSingleObject->getChildrenOfSdrObject() )
            {
                rSet.DisableItem( SID_ENTER_GROUP );
            }

            // Wenn es sich um kein Gruppenobjekt handelt
            // wird "Gruppierung aufheben" disabled
            const SdrObjGroup* pSdrObjGroup = dynamic_cast< const SdrObjGroup* >(pSingleObject);

            if (!(pSdrObjGroup && nInv == SdrInventor))
            {
                rSet.DisableItem(SID_UNGROUP);
            }

//          if (!pSdrObjGroup && !pSdrGrafObj && !pOleObj)
//          {
//              rSet.DisableItem( SID_NAME_GROUP );
//          }

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

            const SdrPathObj* pSdrPathObj = dynamic_cast< const SdrPathObj* >(pSingleObject);

            if(pSdrPathObj)
            {
                if(!pSdrPathObj->isClosed())
                {
                      rSet.DisableItem( SID_ATTR_FILL_STYLE );
                }
            }

            if( (!pSdrPathObj && !aInfoRec.mbCanConvToPath) || pSdrObjGroup ) // Solange es JOE fehlerhaft behandelt!
            {
                // JOE: Ein Gruppenobjekt kann eben u.U. in ein PathObj gewandelt werden
                rSet.DisableItem( SID_LINEEND_POLYGON );
            }

            if((pSdrPathObj && pSdrPathObj->isBezier()) || !aInfoRec.mbCanConvToPath)
            {
                rSet.DisableItem( SID_CHANGEBEZIER );
            }

            if( nInv == SdrInventor &&
                ((pSdrPathObj && !pSdrPathObj->isBezier()) || !aInfoRec.mbCanConvToPoly ) &&
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
                bool bDisable = true;
                SfxItemSet aAttrSet( GetDoc()->GetItemPool() );
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
                            bDisable = false;
                    }
                }
                if( bDisable )
                    rSet.DisableItem( SID_CONNECTION_NEW_ROUTING );
            }

            if ( nInv == E3dInventor                          ||
                 (!mpDrawView->IsConvertToPathObjPossible(false) &&
                  !mpDrawView->IsShearAllowed()                  &&
                  !mpDrawView->IsDistortAllowed()) )
            {
                rSet.DisableItem( SID_OBJECT_SHEAR );
            }

            if(dynamic_cast< const E3dCompoundObject* >(pSingleObject))
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
            if ( !mpDrawView->IsDismantlePossible(false) )
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
        if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_MODIFY_FIELD ) )
        {
            OutlinerView* pOLV = mpDrawView->GetTextEditOutlinerView();

            if( pOLV )
            {
                const SvxFieldItem* pFldItem = pOLV->GetFieldAtSelection();

                if( !( pFldItem && (
                    dynamic_cast< const SvxDateField* >(pFldItem->GetField()) ||
                    dynamic_cast< const SvxAuthorField* >(pFldItem->GetField()) ||
                    dynamic_cast< const SvxExtFileField* >(pFldItem->GetField()) ||
                    dynamic_cast< const SvxExtTimeField* >(pFldItem->GetField()) ) ) )
                {
                    rSet.DisableItem( SID_MODIFY_FIELD );
                }
            }
            else
                rSet.DisableItem( SID_MODIFY_FIELD );
        }

        rSet.DisableItem( SID_GROUP );
        rSet.DisableItem( SID_COMBINE );
        rSet.DisableItem(SID_DISTRIBUTE_DLG);
        rSet.DisableItem(SID_POLY_MERGE);
        rSet.DisableItem(SID_POLY_SUBSTRACT);
        rSet.DisableItem(SID_POLY_INTERSECT);
        rSet.DisableItem( SID_CONNECT );
    }
    else
    {
        if(mpDrawView->areSdrObjectsSelected())
        {
            const SdrObjectVector aSelection(mpDrawView->getSelectedSdrObjectVectorFromSdrMarkView());

            // Mehrfachselektion
        // distribure dialog for 3+n objects
            if(aSelection.size() <= 2)
            {
            rSet.DisableItem(SID_DISTRIBUTE_DLG);
            }

        rSet.DisableItem( SID_LINEEND_POLYGON );
        rSet.DisableItem( SID_ENTER_GROUP );
        rSet.DisableItem( SID_NAME_GROUP );
        rSet.DisableItem( SID_OBJECT_TITLE_DESCRIPTION );
        rSet.DisableItem( SID_MODIFY_FIELD );

            bool bText = false;
            bool bLine = false;
            bool bGroup = false;
            bool bGraf = false;
            bool bDrawObj = false;
            bool b3dObj = false;
            bool bTitOutText = false;
            bool bTable = false;
            bool bMeasureObj = false;
            bool bEdgeObj = false; // Connector
            bool bE3dCompoundObject = false;

            for( sal_uLong i = 0; i < aSelection.size() && !bText && i < 50; i++ )
            {
                SdrObject* pObj = aSelection[i];
                const sal_uInt32 nInv = pObj->GetObjInventor();
                const sal_uInt16 nId = pObj->GetObjIdentifier();

                if (nInv == SdrInventor)
                {
                    switch (nId)
                    {
                        case OBJ_POLY:
                        {
                            SdrPathObj* pSdrPathObj = dynamic_cast< SdrPathObj* >(pObj);

                            if(pSdrPathObj)
                            {
                                bDrawObj = true;
                                bLine = pSdrPathObj->isLine();
                            }
                            else
                            {
                                OSL_ENSURE(false, "OOps, SdrObjKind and dynamic_cast do not fit (!)");
                            }
                            break;
                        }
                        case OBJ_TEXT: bText = true; break;
                        case OBJ_EDGE: bEdgeObj = true; break;
                        case OBJ_MEASURE: bMeasureObj = true; break;
                        case OBJ_RECT:
                        case OBJ_CIRC: bDrawObj = true; break;
                        case OBJ_GRUP: bGroup = true; break;
                        case OBJ_GRAF: bGraf = true; break;
                        case OBJ_TITLETEXT:
                        case OBJ_OUTLINETEXT: bTitOutText = true; break;
                        case OBJ_TABLE: bTable = true; break;
                    }
                }
                else if (nInv == E3dInventor)
                {
                    if(dynamic_cast< E3dScene* >(pObj))
                        b3dObj = true;
                    else if(dynamic_cast< E3dCompoundObject* >(pObj))
                        bE3dCompoundObject = true;
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
                (!mpDrawView->IsConvertToPathObjPossible(false) &&
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

            if ( !mpDrawView->IsDismantlePossible(false) )
            {
                rSet.DisableItem( SID_DISMANTLE );
            }
            if ( !mpDrawView->IsDismantlePossible(true) &&
             !mpDrawView->IsImportMtfPossible()     &&
             !mpDrawView->IsBreak3DObjPossible() )
            {
                rSet.DisableItem( SID_BREAK );
            }
            if ( !mpDrawView->IsCombinePossible(false) )
            {
                rSet.DisableItem( SID_COMBINE );
                rSet.DisableItem(SID_POLY_MERGE);
                rSet.DisableItem(SID_POLY_SUBSTRACT);
                rSet.DisableItem(SID_POLY_INTERSECT);
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
        else
        {
            // kein Objekt selektiert
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
}


} // end of namespace sd
