/*************************************************************************
 *
 *  $RCSfile: drviewsj.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:44 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _IDETEMP_HXX //autogen
#include <basctl/idetemp.hxx>
#endif
#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif
#ifndef _GLOBL3D_HXX //autogen
#include <svx/globl3d.hxx>
#endif
#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif
#define ITEMID_FIELD    EE_FEATURE_FIELD
#ifndef _FLDITEM_HXX
#include <svx/flditem.hxx>
#endif
#ifndef _SVDOGRP_HXX //autogen
#include <svx/svdogrp.hxx>
#endif
#ifndef _SVDOGRAF_HXX //autogen
#include <svx/svdograf.hxx>
#endif
#ifndef _SXELDITM_HXX //autogen
#include <svx/sxelditm.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SVDOPATH_HXX //autogen
#include <svx/svdopath.hxx>
#endif
#ifndef _E3D_POLYSC3D_HXX //autogen
#include <svx/polysc3d.hxx>
#endif
#ifndef _E3D_OBJ3D_HXX //autogen
#include <svx/obj3d.hxx>
#endif
#ifndef _SFXEVENT_HXX //autogen
#include <sfx2/event.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif

#pragma hdrstop

#include "app.hrc"

#include "sdoutl.hxx"
#include "sdpage.hxx"
#include "fupoor.hxx"
#include "fusel.hxx"
#include "drviewsh.hxx"
#include "drawdoc.hxx"
#include "docshell.hxx"
#include "drawview.hxx"
#include "optsitem.hxx"



/*************************************************************************
|*
|* Status (Enabled/Disabled) von Menue-SfxSlots setzen
|*
\************************************************************************/

void __EXPORT SdDrawViewShell::GetMenuStateSel( SfxItemSet &rSet )
{
    // Status der Menueintraege, bzw. Buttons
    // Einfachselektion

    const SdrMarkList& rMarkList = pDrView->GetMarkList();
    ULONG nMarkCount = rMarkList.GetMarkCount();

    if ( nMarkCount == 1 )
    {
        if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_BEZIER_EDIT ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_UNGROUP ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_ENTER_GROUP ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_NAME_GROUP ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_ATTR_FILL_STYLE ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_CHANGEBEZIER ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_CHANGEPOLYGON ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_LINEEND_POLYGON ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( OBJ_TITLETEXT ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( OBJ_OUTLINETEXT ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_MEASURE_DLG ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_CONNECTION_DLG ) ||
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_CONNECTION_NEW_ROUTING ) ||
//          SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_CONVERT_TO_3D_LATHE ) ||
//          SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_CONVERT_TO_3D_LATHE_FAST ) ||
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
            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_REVERSE_ORDER ) )
        {
            const SdrObject* pObj = rMarkList.GetMark(0)->GetObj();
            UINT32 nInv = pObj->GetObjInventor();
            UINT16 nId = pObj->GetObjIdentifier();
            SdrObjTransformInfoRec aInfoRec;
            pObj->TakeObjInfo( aInfoRec );

            // Wenn es sich um kein Gruppenobjekt oder 3D-Objekt handelt
            // wird "Gruppe betreten" disabled
            if( !( ( pObj->ISA( SdrObjGroup ) && nInv == SdrInventor ) ||
                (pObj->ISA (E3dPolyScene) || pObj->ISA (E3dScene) /*|| pObj->ISA (E3dCompoundObject) */) ) )
            {
                rSet.DisableItem( SID_ENTER_GROUP );
            }

            // Wenn es sich um kein Gruppenobjekt handelt
            // wird "Gruppierung aufheben" disabled
            if (!(pObj->ISA(SdrObjGroup) && nInv == SdrInventor))
            {
                rSet.DisableItem(SID_UNGROUP);
            }

            if (!pObj->ISA(SdrObjGroup) && !pObj->ISA(SdrGrafObj))
            {
                rSet.DisableItem( SID_NAME_GROUP );
            }

            if (!pObj->ISA(SdrGrafObj) ||
                ((SdrGrafObj*) pObj)->GetGraphicType() != GRAPHIC_BITMAP ||
                ((SdrGrafObj*) pObj)->IsLinkedGraphic())
            {
                rSet.DisableItem(SID_CONVERT_TO_1BIT_THRESHOLD);
                rSet.DisableItem(SID_CONVERT_TO_1BIT_MATRIX);
                rSet.DisableItem(SID_CONVERT_TO_4BIT_GRAYS);
                rSet.DisableItem(SID_CONVERT_TO_4BIT_COLORS);
                rSet.DisableItem(SID_CONVERT_TO_8BIT_GRAYS);
                rSet.DisableItem(SID_CONVERT_TO_8BIT_COLORS);
                rSet.DisableItem(SID_CONVERT_TO_24BIT);
            }

            if(nInv == SdrInventor && nId == OBJ_OLE2)
            {
                if (pFuActual && pFuActual->GetSlotID() == SID_BEZIER_EDIT)
                {
                    SfxAllEnumItem aItem(SID_OBJECT_SELECT, SVX_SNAP_SELECT);
                    GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT,
                            SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L);
                }
            }
            if( nInv == SdrInventor &&
               (nId == OBJ_LINE ||
                nId == OBJ_PLIN ||
                nId == OBJ_PATHLINE ||
                nId == OBJ_FREELINE ))
            {
                //rSet.DisableItem( SID_ATTRIBUTES_AREA ); // wieder raus!
                rSet.DisableItem( SID_ATTR_FILL_STYLE );
            }
            if( !pObj->ISA( SdrPathObj ) &&
                !aInfoRec.bCanConvToPath ||
                pObj->ISA( SdrObjGroup ) ) // Solange es JOE fehlerhaft behandelt!
            { // JOE: Ein Gruppenobjekt kann eben u.U. in ein PathObj gewandelt werden
                rSet.DisableItem( SID_LINEEND_POLYGON );
            }
            if(nInv == SdrInventor &&
               (nId == OBJ_PATHFILL || nId == OBJ_PATHLINE || !aInfoRec.bCanConvToPath))
                rSet.DisableItem( SID_CHANGEBEZIER );

            if( nInv == SdrInventor &&
                ( nId == OBJ_POLY || nId == OBJ_PLIN || !aInfoRec.bCanConvToPoly ) &&
                !pView->IsVectorizeAllowed() )
            {
                rSet.DisableItem( SID_CHANGEPOLYGON );
            }

            if(nInv == SdrInventor &&
               (nId == OBJ_TITLETEXT || nId == OBJ_OUTLINETEXT))
                rSet.DisableItem( SID_TEXTATTR_DLG );

            if( nInv != SdrInventor || nId != OBJ_MEASURE )
                rSet.DisableItem( SID_MEASURE_DLG );

            if( nInv != SdrInventor || nId != OBJ_EDGE )
                rSet.DisableItem( SID_CONNECTION_DLG );
            else
            {
                BOOL bDisable = TRUE;
                SfxItemSet aAttrSet( pDoc->GetPool() );
                pView->GetAttributes( aAttrSet );

                if( aAttrSet.GetItemState( SDRATTR_EDGELINE1DELTA ) >= SFX_ITEM_AVAILABLE &&
                    aAttrSet.GetItemState( SDRATTR_EDGELINE2DELTA ) >= SFX_ITEM_AVAILABLE &&
                    aAttrSet.GetItemState( SDRATTR_EDGELINE3DELTA ) >= SFX_ITEM_AVAILABLE )
                {
                    long nVal1 = ( ( const SdrEdgeLine1DeltaItem& ) aAttrSet.Get( SDRATTR_EDGELINE1DELTA ) ).GetValue();
                    long nVal2 = ( ( const SdrEdgeLine2DeltaItem& ) aAttrSet.Get( SDRATTR_EDGELINE2DELTA ) ).GetValue();
                    long nVal3 = ( ( const SdrEdgeLine3DeltaItem& ) aAttrSet.Get( SDRATTR_EDGELINE3DELTA ) ).GetValue();
                    {
                        if( nVal1 != 0 || nVal2 != 0 || nVal3 != 0 )
                            bDisable = FALSE;
                    }
                }
                if( bDisable )
                    rSet.DisableItem( SID_CONNECTION_NEW_ROUTING );
            }

            if ( nInv == E3dInventor                          ||
                 (!pDrView->IsConvertToPathObjPossible(FALSE) &&
                  !pDrView->IsShearAllowed()                  &&
                  !pDrView->IsDistortAllowed()) )
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
            if ( !pDrView->IsDismantlePossible(FALSE) )
            {
                rSet.DisableItem( SID_DISMANTLE );
            }

            if ( !pDrView->IsDismantlePossible(TRUE) &&
                 !pDrView->IsImportMtfPossible()     &&
                 !pDrView->IsBreak3DObjPossible() )
            {
                rSet.DisableItem( SID_BREAK );
            }
        }
        if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_MODIFY_FIELD ) )
        {
            OutlinerView* pOLV = pDrView->GetTextEditOutlinerView();

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

        rSet.DisableItem( SID_GROUP );
        rSet.DisableItem( SID_COMBINE );
        rSet.DisableItem(SID_DISTRIBUTE_DLG);
        rSet.DisableItem(SID_POLY_MERGE);
        rSet.DisableItem(SID_POLY_SUBSTRACT);
        rSet.DisableItem(SID_POLY_INTERSECT);
        rSet.DisableItem( SID_CONNECT );
    }
    // Mehrfachselektion
    else if( nMarkCount > 1 )
    {
        // distribure dialog for 3+n objects
        if(nMarkCount <= 2)
            rSet.DisableItem(SID_DISTRIBUTE_DLG);

//        rSet.ClearItem( SID_BEZIER_EDIT );
//        rSet.DisableItem( SID_BEZIER_EDIT );
        rSet.DisableItem( SID_LINEEND_POLYGON );
        rSet.DisableItem( SID_ENTER_GROUP );
        // Jetzt (28.10.96) muessen Namen fuer Objekte eindeutig sein
        rSet.DisableItem( SID_NAME_GROUP );
        rSet.DisableItem( SID_MODIFY_FIELD );

        if( 1 )
//      if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_ATTR_FILL_STYLE ) ||
//          SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_UNGROUP ) ||
//          SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_MEASURE_DLG ) ||
//          SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_CONNECTION_DLG ) ||
//          SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_COMBINE ) ||
//            SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_OBJECT_SHEAR ) ||
//          SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_TEXTATTR_DLG ) ||
//          SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_CONVERT_TO_3D_LATHE ) ||
//          SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_CONVERT_TO_3D_LATHE_FAST ) ||
//          SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_OBJECT_ALIGN_LEFT ) ||
//          SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_OBJECT_ALIGN_CENTER ) ||
//          SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_OBJECT_ALIGN_RIGHT ) ||
//          SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_OBJECT_ALIGN_UP ) ||
//          SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_OBJECT_ALIGN_MIDDLE ) ||
//          SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_OBJECT_ALIGN_DOWN ) )
        {
            BOOL bText = FALSE;
            BOOL bLine = FALSE;
            BOOL bGroup = FALSE;
            BOOL bGraf = FALSE;
            BOOL bDrawObj = FALSE;
            BOOL b3dObj = FALSE;
            BOOL bTitOutText = FALSE;
            BOOL bMeasureObj = FALSE;
            BOOL bEdgeObj = FALSE; // Connector
            BOOL bE3dCompoundObject = FALSE;

            for( ULONG i = 0;
                 i < nMarkCount && !bText && i < 50;
                 i++ )
            {
                SdrObject* pObj = rMarkList.GetMark(i)->GetObj();
                UINT32 nInv = pObj->GetObjInventor();
                UINT16 nId = pObj->GetObjIdentifier();

                if (nInv == SdrInventor)
                {
                    switch (nId)
                    {
                        case OBJ_TEXT: bText = TRUE; break;

                        case OBJ_LINE: bLine = TRUE; break;

                        case OBJ_EDGE: bEdgeObj = TRUE; break;

                        case OBJ_MEASURE: bMeasureObj = TRUE; break;

                        case OBJ_RECT:
                        case OBJ_CIRC:
                        case OBJ_FREELINE:
                        case OBJ_FREEFILL:
                        case OBJ_PATHFILL:
                        case OBJ_PATHLINE:
                        case OBJ_SECT:
                        case OBJ_CARC:
                        case OBJ_CCUT: bDrawObj = TRUE; break;

                        case OBJ_GRUP: bGroup = TRUE; break;

                        case OBJ_GRAF: bGraf = TRUE; break;

                        case OBJ_TITLETEXT:
                        case OBJ_OUTLINETEXT: bTitOutText = TRUE; break;
                    }
                }
                else if (nInv == E3dInventor)
                {
                    if(pObj->ISA(E3dScene))
                        b3dObj = TRUE;
                    else if(pObj->ISA(E3dCompoundObject))
                        bE3dCompoundObject = TRUE;
                }
            }
            /* Kann wohl raus, da jedes(?) Objekt Text enthalten kann
            if( !bText )
            {
                rSet.DisableItem( SID_CHAR_DLG );
                rSet.DisableItem( SID_PARA_DLG );
                rSet.DisableItem( SID_BULLET );
            }
            */
            if( bLine && !bText && !bDrawObj &&!b3dObj)
            {
                //rSet.DisableItem( SID_ATTRIBUTES_AREA );
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
                 (!pDrView->IsConvertToPathObjPossible(FALSE) &&
                  !pDrView->IsShearAllowed()                  &&
                  !pDrView->IsDistortAllowed()) )
            {
                rSet.DisableItem( SID_OBJECT_SHEAR );
            }

            if( !bGroup )
            {
                rSet.DisableItem( SID_UNGROUP );
            }
            if( bTitOutText )
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

        if ( !pDrView->IsDismantlePossible(FALSE) )
        {
            rSet.DisableItem( SID_DISMANTLE );
        }
        if ( !pDrView->IsDismantlePossible(TRUE) &&
             !pDrView->IsImportMtfPossible()     &&
             !pDrView->IsBreak3DObjPossible() )
        {
            rSet.DisableItem( SID_BREAK );
        }
        if ( !pDrView->IsCombinePossible(FALSE) )
        {
            rSet.DisableItem( SID_COMBINE );
            rSet.DisableItem(SID_POLY_MERGE);
            rSet.DisableItem(SID_POLY_SUBSTRACT);
            rSet.DisableItem(SID_POLY_INTERSECT);
        }
        if ( !pDrView->IsCombinePossible(TRUE) )
        {
            rSet.DisableItem( SID_CONNECT );
        }
        if ( !pDrView->IsGroupPossible() )
        {
            rSet.DisableItem( SID_GROUP );
        }
        if ( !pDrView->IsUnGroupPossible() )
        {
            rSet.DisableItem( SID_UNGROUP );
        }
    }
    // kein Objekt selektiert
    else
    {
//        rSet.ClearItem( SID_BEZIER_EDIT );

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

//      rSet.DisableItem( SID_BEZIER_EDIT );
        rSet.DisableItem( SID_SIZE_OPTIMAL );
        rSet.DisableItem( SID_LINEEND_POLYGON );
        rSet.DisableItem( SID_COPYOBJECTS );
        rSet.DisableItem( SID_HORIZONTAL );
        rSet.DisableItem( SID_VERTICAL );
        rSet.DisableItem( SID_GROUP );
        rSet.DisableItem( SID_UNGROUP );
        rSet.DisableItem( SID_NAME_GROUP );
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

/*************************************************************************
|*
|* Notify
|*
\************************************************************************/

void SdDrawViewShell::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType,
                                 const SfxHint& rHint, const TypeId& rHintType)
{
    BOOL bOK = FALSE;

    // Zuerst die Basisklasse
    SdViewShell::SFX_NOTIFY(rBC, rBCType, rHint, rHintType);

    SfxSimpleHint* pSimple = PTR_CAST(SfxSimpleHint, &rHint);
    ULONG nId = pSimple == NULL ? 0 : pSimple->GetId();

    if (nId == SFX_HINT_MODECHANGED)
    {
        if (pDocSh->IsReadOnly() && pFuActual && !pFuActual->ISA(FuSelection))
        {
            // Aktuelle Funktion beenden und Selektion aktivieren
            SfxRequest aReq(SID_OBJECT_SELECT, 0, pDoc->GetItemPool());
            FuPermanent(aReq);
            bOK = TRUE;
        }

        if (pDocSh->IsReadOnly() != bReadOnly )
        {
            bReadOnly = pDocSh->IsReadOnly();

            SfxBoolItem aItem( SID_FM_DESIGN_MODE, !bReadOnly );
            GetViewFrame()->GetDispatcher()->Execute( SID_FM_DESIGN_MODE,
                      SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L );
        }
    }

    if (!bOK && pDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS &&
        SD_MOD()->GetSdOptions(DOCUMENT_TYPE_IMPRESS)->IsStartWithTemplate() &&
        rBC.ISA(SdDrawDocShell))
    {
        SfxEventHint* pEvent = PTR_CAST(SfxEventHint, &rHint);
        ULONG nId = pEvent == NULL ? 0 : pEvent->GetEventId();

        if (nId == SFX_EVENT_CREATEDOC && pEvent->GetObjShell() == pDocSh
            && pDoc->GetSdPageCount(PK_STANDARD) == 1)
        {
            SfxItemSet* pSet = pDocSh->GetMedium()->GetItemSet();

            if ( pSet &&
                (SFX_ITEM_SET != pSet->GetItemState(SID_SILENT) ||
                !((SfxBoolItem&) (pSet->Get(SID_SILENT))).GetValue()) )
            {
                // SeitenLayout-Dialog oeffnen,
                // sofern es sich um ein Dokument mit nur einer Standard-Seite
                // handelt (Neues Dokument oder Praesentationshintergrund).
                // Praesentationsvolagen enthalten mehrere fertige Seiten und
                // beduerfen daher nicht dieses Dialogs.
                // SID_SILENT: Slot wurde nicht ueber API ausgefuehrt (z.B. BASIC)
                pDoc->GetSdPage(0, PK_STANDARD)->SetAutoLayout(AUTOLAYOUT_TITLE);
                SfxBoolItem aIsChangedItem(SID_MODIFYPAGE, pDoc->IsChanged());
                GetViewFrame()->GetDispatcher()->Execute(SID_MODIFYPAGE,
                       SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aIsChangedItem, 0L);
            }
        }
    }
}



