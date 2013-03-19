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

#include <svx/svdundo.hxx>
#include <sfx2/app.hxx>
#include <sfx2/request.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/viewsh.hxx>
#include "svx/unoapi.hxx"
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XEnhancedCustomShapeDefaulter.hpp>
#include <svx/dialmgr.hxx>
#include <svx/svdoashp.hxx>
#include <svx/dialogs.hrc>
#include <svx/svdview.hxx>
#include <svx/sdasitm.hxx>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <sfx2/bindings.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/kernitem.hxx>
#include <svx/sdrpaintwindow.hxx>

#include <svx/svxids.hrc>
#include <svx/fontworkbar.hxx>
#include "svx/fontworkgallery.hxx"

using ::rtl::OUString;

using namespace ::svx;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;

void SetAlignmentState( SdrView* pSdrView, SfxItemSet& rSet )
{
    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    sal_uIntPtr nCount = rMarkList.GetMarkCount(), i;

    sal_Int32   nAlignment = -1;
    for( i = 0; i < nCount; i++ )
    {
        SdrObject* pObj = rMarkList.GetMark( i )->GetMarkedSdrObj();
        if( pObj->ISA(SdrObjCustomShape) )
        {
            sal_Int32 nOldAlignment = nAlignment;
            SdrTextHorzAdjustItem&      rTextHorzAdjustItem    = (SdrTextHorzAdjustItem&)pObj->GetMergedItem( SDRATTR_TEXT_HORZADJUST );
            SdrTextFitToSizeTypeItem&   rTextFitToSizeTypeItem = (SdrTextFitToSizeTypeItem&)pObj->GetMergedItem( SDRATTR_TEXT_FITTOSIZE );
            switch ( rTextHorzAdjustItem.GetValue() )
            {
                case SDRTEXTHORZADJUST_LEFT   : nAlignment = 0; break;
                case SDRTEXTHORZADJUST_CENTER : nAlignment = 1; break;
                case SDRTEXTHORZADJUST_RIGHT  : nAlignment = 2; break;
                case SDRTEXTHORZADJUST_BLOCK  :
                {
                    if ( rTextFitToSizeTypeItem.GetValue() == SDRTEXTFIT_NONE )
                        nAlignment = 3;
                    else if ( rTextFitToSizeTypeItem.GetValue() == SDRTEXTFIT_ALLLINES )
                        nAlignment = 4;
                }
            }
            if ( ( nOldAlignment != -1 ) && ( nOldAlignment != nAlignment ) )
            {
                nAlignment = -1;
                break;
            }
        }
    }
    rSet.Put( SfxInt32Item( SID_FONTWORK_ALIGNMENT, nAlignment ) );
}

void SetCharacterSpacingState( SdrView* pSdrView, SfxItemSet& rSet )
{
    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    sal_uIntPtr nCount = rMarkList.GetMarkCount(), i;

    sal_Int32   nCharacterSpacing = -1;
    for( i = 0; i < nCount; i++ )
    {
        SdrObject* pObj = rMarkList.GetMark( i )->GetMarkedSdrObj();
        if( pObj->ISA(SdrObjCustomShape) )
        {
            sal_Int32 nOldCharacterSpacing = nCharacterSpacing;
            SvxCharScaleWidthItem& rCharScaleWidthItem = (SvxCharScaleWidthItem&)pObj->GetMergedItem( EE_CHAR_FONTWIDTH );
            nCharacterSpacing = rCharScaleWidthItem.GetValue();
            if ( ( nOldCharacterSpacing != -1 ) && ( nOldCharacterSpacing != nCharacterSpacing ) )
            {
                nCharacterSpacing = -1;
                break;
            }
        }
    }
    rSet.Put( SfxInt32Item( SID_FONTWORK_CHARACTER_SPACING, nCharacterSpacing ) );
}


void SetKernCharacterPairsState( SdrView* pSdrView, SfxItemSet& rSet )
{
    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    sal_uIntPtr nCount = rMarkList.GetMarkCount(), i;

    sal_Bool    bChecked = sal_False;
    for( i = 0; i < nCount; i++ )
    {
        SdrObject* pObj = rMarkList.GetMark( i )->GetMarkedSdrObj();
        if( pObj->ISA(SdrObjCustomShape) )
        {
            SvxKerningItem& rKerningItem = (SvxKerningItem&)pObj->GetMergedItem( EE_CHAR_KERNING );
            if ( rKerningItem.GetValue() )
                bChecked = sal_True;
        }
    }
    rSet.Put( SfxBoolItem( SID_FONTWORK_KERN_CHARACTER_PAIRS, bChecked ) );
}

void SetFontWorkShapeTypeState( SdrView* pSdrView, SfxItemSet& rSet )
{
    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    sal_uIntPtr nCount = rMarkList.GetMarkCount(), i;

    rtl::OUString aFontWorkShapeType;

    for( i = 0; i < nCount; i++ )
    {
        SdrObject* pObj = rMarkList.GetMark( i )->GetMarkedSdrObj();
        if( pObj->ISA( SdrObjCustomShape ) )
        {
            const rtl::OUString sType( "Type" );
            SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
            Any* pAny = aGeometryItem.GetPropertyValueByName( sType );
            if( pAny )
            {
                rtl::OUString aType;
                if ( *pAny >>= aType )
                {
                    if ( !aFontWorkShapeType.isEmpty() )
                    {
                        if ( !aFontWorkShapeType.equals( aType ) )  // different FontWorkShapeTypes selected ?
                        {
                            aFontWorkShapeType = rtl::OUString();
                            break;
                        }
                    }
                    aFontWorkShapeType = aType;
                }
            }
        }
    }
    rSet.Put( SfxStringItem( SID_FONTWORK_SHAPE_TYPE, aFontWorkShapeType ) );
}

/*************************************************************************
|*
|* Standardinterface deklarieren (Die Slotmap darf nicht leer sein, also
|* tragen wir etwas ein, was hier (hoffentlich) nie vorkommt).
|*
\************************************************************************/

SFX_SLOTMAP(FontworkBar)
{
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

SFX_IMPL_INTERFACE(FontworkBar, SfxShell, SVX_RES(RID_SVX_FONTWORK_BAR))
{
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_OBJECT, SVX_RES(RID_SVX_FONTWORK_BAR) );
}

TYPEINIT1( FontworkBar, SfxShell );


/*************************************************************************
|*
|* Standard-Konstruktor
|*
\************************************************************************/

FontworkBar::FontworkBar(SfxViewShell* pViewShell )
: SfxShell(pViewShell)
{
    DBG_ASSERT( pViewShell, "svx::FontworkBar::FontworkBar(), I need a viewshell!" );
    if( pViewShell )
        SetPool(&pViewShell->GetPool());

    SetHelpId( SVX_INTERFACE_FONTWORK_BAR );
    SetName( String( SVX_RES( RID_SVX_FONTWORK_BAR) ));
}


/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FontworkBar::~FontworkBar()
{
    SetRepeatTarget(NULL);
}

static Window* ImpGetViewWin(SdrView* pView)
{
    if( pView )
    {
        const sal_uInt32 nAnz(pView->PaintWindowCount());
        for(sal_uInt32 nNum(0L); nNum < nAnz; nNum++)
        {
            OutputDevice* pOut = &(pView->GetPaintWindow(nNum)->GetOutputDevice());

            if(OUTDEV_WINDOW == pOut->GetOutDevType())
            {
                return (Window*)pOut;
            }
        }
    }

    return 0L;
}

namespace svx {
bool checkForSelectedFontWork( SdrView* pSdrView, sal_uInt32& nCheckStatus )
{
    if ( nCheckStatus & 2 )
        return ( nCheckStatus & 1 ) != 0;

    static const rtl::OUString  sTextPath( "TextPath" );

    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    sal_uIntPtr nCount = rMarkList.GetMarkCount(), i;
    sal_Bool bFound = sal_False;
    for(i=0;(i<nCount) && !bFound ; i++)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
        if( pObj->ISA(SdrObjCustomShape) )
        {
            SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
            Any* pAny = aGeometryItem.GetPropertyValueByName( sTextPath, sTextPath );
            if( pAny )
                *pAny >>= bFound;
        }
    }
    if ( bFound )
        nCheckStatus |= 1;
    nCheckStatus |= 2;
    return bFound;
}
}

static void impl_execute( SdrView*, SfxRequest& rReq, SdrCustomShapeGeometryItem& rGeometryItem, SdrObject* pObj )
{
    static const rtl::OUString  sTextPath( "TextPath" );
    static const rtl::OUString  sSameLetterHeights( "SameLetterHeights" );

    sal_uInt16 nSID = rReq.GetSlot();
    switch( nSID )
    {
        case SID_FONTWORK_SAME_LETTER_HEIGHTS:
        {
            com::sun::star::uno::Any* pAny = rGeometryItem.GetPropertyValueByName( sTextPath, sSameLetterHeights );
            if( pAny )
            {
                sal_Bool bOn;
                (*pAny) >>= bOn;
                bOn = !bOn;
                (*pAny) <<= bOn;
            }
        }
        break;

        case SID_FONTWORK_ALIGNMENT:
        {
            if( rReq.GetArgs() && rReq.GetArgs()->GetItemState( SID_FONTWORK_ALIGNMENT ) == SFX_ITEM_SET )
            {
                sal_Int32 nValue = ((const SfxInt32Item*)rReq.GetArgs()->GetItem(SID_FONTWORK_ALIGNMENT))->GetValue();
                if ( ( nValue >= 0 ) && ( nValue < 5 ) )
                {
                    SdrFitToSizeType eFTS = SDRTEXTFIT_NONE;
                    SdrTextHorzAdjust eHorzAdjust;
                    switch ( nValue )
                    {
                        case 4 : eFTS = SDRTEXTFIT_ALLLINES; // passthrough
                        case 3 : eHorzAdjust = SDRTEXTHORZADJUST_BLOCK; break;
                        default:
                        case 0 : eHorzAdjust = SDRTEXTHORZADJUST_LEFT; break;
                        case 1 : eHorzAdjust = SDRTEXTHORZADJUST_CENTER; break;
                        case 2 : eHorzAdjust = SDRTEXTHORZADJUST_RIGHT; break;
                    }
                    pObj->SetMergedItem( SdrTextHorzAdjustItem( eHorzAdjust ) );
                    pObj->SetMergedItem( SdrTextFitToSizeTypeItem( eFTS ) );
                    pObj->BroadcastObjectChange();
                }
            }
        }
        break;

        case SID_FONTWORK_CHARACTER_SPACING:
        {
            if( rReq.GetArgs() && ( rReq.GetArgs()->GetItemState( SID_FONTWORK_CHARACTER_SPACING ) == SFX_ITEM_SET ) )
            {
                sal_Int32 nCharSpacing = ((const SfxInt32Item*)rReq.GetArgs()->GetItem(SID_FONTWORK_CHARACTER_SPACING))->GetValue();
                pObj->SetMergedItem( SvxCharScaleWidthItem( (sal_uInt16)nCharSpacing, EE_CHAR_FONTWIDTH ) );
                pObj->BroadcastObjectChange();
            }
        }
        break;

        case SID_FONTWORK_KERN_CHARACTER_PAIRS:
        {
            if( rReq.GetArgs() && ( rReq.GetArgs()->GetItemState( SID_FONTWORK_KERN_CHARACTER_PAIRS ) == SFX_ITEM_SET ) )
            {
                // sal_Bool bKernCharacterPairs = ((const SfxBoolItem*)rReq.GetArgs()->GetItem(SID_FONTWORK_KERN_CHARACTER_PAIRS))->GetValue();
//TODO:             pObj->SetMergedItem( SvxCharScaleWidthItem( (sal_uInt16)nCharSpacing, EE_CHAR_FONTWIDTH ) );
                pObj->BroadcastObjectChange();
            }
        }
        break;
    }
}

#include "svx/gallery.hxx"
#include <svx/fmmodel.hxx>
#include <svx/fmpage.hxx>
#include <svl/itempool.hxx>

void GetGeometryForCustomShape( SdrCustomShapeGeometryItem& rGeometryItem, const rtl::OUString rCustomShape )
{
    const rtl::OUString sType( "Type" );

    com::sun::star::beans::PropertyValue aPropVal;
    aPropVal.Name = sType;
    aPropVal.Value <<= rCustomShape;
    rGeometryItem.SetPropertyValue( aPropVal );

    const rtl::OUString sAdjustmentValues( "AdjustmentValues" );
    const rtl::OUString sCoordinateOrigin( "CoordinateOrigin" );
    const rtl::OUString sCoordinateSize( "CoordinateSize" );
    const rtl::OUString sEquations( "Equations" );
    const rtl::OUString sHandles( "Handles" );
    const rtl::OUString sPath( "Path" );
    rGeometryItem.ClearPropertyValue( sAdjustmentValues );
    rGeometryItem.ClearPropertyValue( sCoordinateOrigin );
    rGeometryItem.ClearPropertyValue( sCoordinateSize );
    rGeometryItem.ClearPropertyValue( sEquations );
    rGeometryItem.ClearPropertyValue( sHandles );
    rGeometryItem.ClearPropertyValue( sPath );

    /* SJ: CustomShapes that are available in the gallery are having the highest
       priority, so we will take a look there before taking the internal default */

    if ( GalleryExplorer::GetSdrObjCount( GALLERY_THEME_POWERPOINT ) )
    {
        std::vector< rtl::OUString > aObjList;
        if ( GalleryExplorer::FillObjListTitle( GALLERY_THEME_POWERPOINT, aObjList ) )
        {
            sal_uInt16 i;
            for ( i = 0; i < aObjList.size(); i++ )
            {
                if ( aObjList[ i ].equalsIgnoreAsciiCase( rCustomShape ) )
                {
                    FmFormModel aFormModel;
                    SfxItemPool& rPool = aFormModel.GetItemPool();
                    rPool.FreezeIdRanges();
                    if ( GalleryExplorer::GetSdrObj( GALLERY_THEME_POWERPOINT, i, &aFormModel ) )
                    {
                        const SdrObject* pSourceObj = aFormModel.GetPage( 0 )->GetObj( 0 );
                        if( pSourceObj )
                        {
                            PropertyValue aPropVal_;
                            SdrCustomShapeGeometryItem& rSourceGeometry = (SdrCustomShapeGeometryItem&)pSourceObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );
                            com::sun::star::uno::Any* pAny = rSourceGeometry.GetPropertyValueByName( sType );
                            if ( pAny )
                            {
                                aPropVal_.Name = sType;
                                aPropVal_.Value = *pAny;
                                rGeometryItem.SetPropertyValue( aPropVal_ );
                            }
                            pAny = rSourceGeometry.GetPropertyValueByName( sAdjustmentValues );
                            if ( pAny )
                            {
                                aPropVal_.Name = sAdjustmentValues;
                                aPropVal_.Value = *pAny;
                                rGeometryItem.SetPropertyValue( aPropVal_ );
                            }
                            pAny = rSourceGeometry.GetPropertyValueByName( sCoordinateOrigin );
                            if ( pAny )
                            {
                                aPropVal_.Name = sCoordinateOrigin;
                                aPropVal_.Value = *pAny;
                                rGeometryItem.SetPropertyValue( aPropVal_ );
                            }
                            pAny = rSourceGeometry.GetPropertyValueByName( sCoordinateSize );
                            if ( pAny )
                            {
                                aPropVal_.Name = sCoordinateSize;
                                aPropVal_.Value = *pAny;
                                rGeometryItem.SetPropertyValue( aPropVal_ );
                            }
                            pAny = rSourceGeometry.GetPropertyValueByName( sEquations );
                            if ( pAny )
                            {
                                aPropVal_.Name = sEquations;
                                aPropVal_.Value = *pAny;
                                rGeometryItem.SetPropertyValue( aPropVal_ );
                            }
                            pAny = rSourceGeometry.GetPropertyValueByName( sHandles );
                            if ( pAny )
                            {
                                aPropVal_.Name = sHandles;
                                aPropVal_.Value = *pAny;
                                rGeometryItem.SetPropertyValue( aPropVal_ );
                            }
                            pAny = rSourceGeometry.GetPropertyValueByName( sPath );
                            if ( pAny )
                            {
                                aPropVal_.Name = sPath;
                                aPropVal_.Value = *pAny;
                                rGeometryItem.SetPropertyValue( aPropVal_ );
                            }
                        }
                    }
                }
            }
        }
    }
}


void FontworkBar::execute( SdrView* pSdrView, SfxRequest& rReq, SfxBindings& rBindings )
{
    sal_uInt16 nStrResId = 0;

    sal_uInt16 nSID = rReq.GetSlot();
    switch( nSID )
    {
        case SID_FONTWORK_GALLERY_FLOATER:
        {
            FontWorkGalleryDialog aDlg( pSdrView, ImpGetViewWin(pSdrView), nSID );
            aDlg.Execute();
        }
        break;

        case SID_FONTWORK_SHAPE_TYPE:
        {
            rtl::OUString aCustomShape;
            const SfxItemSet* pArgs = rReq.GetArgs();
            if ( pArgs )
            {
                const SfxStringItem& rItm = (const SfxStringItem&)pArgs->Get( rReq.GetSlot() );
                aCustomShape = rItm.GetValue();
            }
            if ( !aCustomShape.isEmpty() )
            {
                const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
                sal_uInt32 nCount = rMarkList.GetMarkCount(), i;
                for( i = 0; i < nCount; i++ )
                {
                    SdrObject* pObj = rMarkList.GetMark( i )->GetMarkedSdrObj();
                    if( pObj->ISA(SdrObjCustomShape) )
                    {
                        const bool bUndo = pSdrView->IsUndoEnabled();

                        if( bUndo )
                        {
                            String aStr( SVX_RES( RID_SVXSTR_UNDO_APPLY_FONTWORK_SHAPE ) );
                            pSdrView->BegUndo( aStr );
                            pSdrView->AddUndo( pSdrView->GetModel()->GetSdrUndoFactory().CreateUndoAttrObject( *pObj ) );
                        }
                        SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
                        GetGeometryForCustomShape( aGeometryItem, aCustomShape );
                        pObj->SetMergedItem( aGeometryItem );

                        Reference< drawing::XShape > aXShape = GetXShapeForSdrObject( (SdrObjCustomShape*)pObj );
                        if ( aXShape.is() )
                        {
                            Reference< drawing::XEnhancedCustomShapeDefaulter > xDefaulter( aXShape, UNO_QUERY );
                            if( xDefaulter.is() )
                                xDefaulter->createCustomShapeDefaults( aCustomShape );
                        }

                        pObj->BroadcastObjectChange();
                        if( bUndo )
                            pSdrView->EndUndo();
                        pSdrView->AdjustMarkHdl(); //HMH sal_True );
                        rBindings.Invalidate( SID_FONTWORK_SHAPE_TYPE );
                    }
                }
            }
        }
        break;

        case SID_FONTWORK_CHARACTER_SPACING_DIALOG :
        {
            if( rReq.GetArgs() && ( rReq.GetArgs()->GetItemState( SID_FONTWORK_CHARACTER_SPACING ) == SFX_ITEM_SET ) )
            {
                sal_Int32 nCharSpacing = ((const SfxInt32Item*)rReq.GetArgs()->GetItem(SID_FONTWORK_CHARACTER_SPACING))->GetValue();
                FontworkCharacterSpacingDialog aDlg( 0L, nCharSpacing );
                sal_uInt16 nRet = aDlg.Execute();
                if( nRet != 0 )
                {
                    SfxInt32Item aItem( SID_FONTWORK_CHARACTER_SPACING, aDlg.getScale() );
                    SfxPoolItem* aItems[] = { &aItem, 0 };
                    rBindings.Execute( SID_FONTWORK_CHARACTER_SPACING, (const SfxPoolItem**)aItems );
                }
            }
        }
        break;

        case SID_FONTWORK_SHAPE:
        case SID_FONTWORK_ALIGNMENT:
        {
            if ( !nStrResId )
                nStrResId = RID_SVXSTR_UNDO_APPLY_FONTWORK_ALIGNMENT;
        }   // PASSTROUGH
        case SID_FONTWORK_CHARACTER_SPACING:
        {
            if ( !nStrResId )
                nStrResId = RID_SVXSTR_UNDO_APPLY_FONTWORK_CHARACTER_SPACING;
        }   // PASSTROUGH
        case SID_FONTWORK_KERN_CHARACTER_PAIRS:
        {
            if ( !nStrResId )
                nStrResId = RID_SVXSTR_UNDO_APPLY_FONTWORK_CHARACTER_SPACING;
        }   // PASSTROUGH
        case SID_FONTWORK_SAME_LETTER_HEIGHTS:
        {
            if ( !nStrResId )
                nStrResId = RID_SVXSTR_UNDO_APPLY_FONTWORK_SAME_LETTER_HEIGHT;

            const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
            sal_uIntPtr nCount = rMarkList.GetMarkCount(), i;
            for( i = 0; i < nCount; i++ )
            {
                SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
                if( pObj->ISA(SdrObjCustomShape) )
                {
                    const bool bUndo = pSdrView->IsUndoEnabled();
                    if( bUndo )
                    {
                        String aStr( SVX_RES( nStrResId ) );
                        pSdrView->BegUndo( aStr );
                        pSdrView->AddUndo( pSdrView->GetModel()->GetSdrUndoFactory().CreateUndoAttrObject( *pObj ) );
                    }
                    SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
                    impl_execute( pSdrView, rReq, aGeometryItem, pObj );
                    pObj->SetMergedItem( aGeometryItem );
                    pObj->BroadcastObjectChange();
                    if( bUndo )
                        pSdrView->EndUndo();
                }
            }
        }
        break;
    };
}

void FontworkBar::getState( SdrView* pSdrView, SfxItemSet& rSet )
{
    sal_uInt32 nCheckStatus = 0;

    if ( rSet.GetItemState( SID_FONTWORK_ALIGNMENT_FLOATER ) != SFX_ITEM_UNKNOWN )
    {
        if ( !checkForSelectedFontWork( pSdrView, nCheckStatus ) )
            rSet.DisableItem( SID_FONTWORK_ALIGNMENT_FLOATER );
    }
    if ( rSet.GetItemState( SID_FONTWORK_ALIGNMENT ) != SFX_ITEM_UNKNOWN )
    {
        if ( !checkForSelectedFontWork( pSdrView, nCheckStatus ) )
            rSet.DisableItem( SID_FONTWORK_ALIGNMENT );
        else
            SetAlignmentState( pSdrView, rSet );
    }
    if ( rSet.GetItemState( SID_FONTWORK_CHARACTER_SPACING_FLOATER ) != SFX_ITEM_UNKNOWN )
    {
        if ( !checkForSelectedFontWork( pSdrView, nCheckStatus ) )
            rSet.DisableItem( SID_FONTWORK_CHARACTER_SPACING_FLOATER );
    }
    if ( rSet.GetItemState( SID_FONTWORK_CHARACTER_SPACING ) != SFX_ITEM_UNKNOWN )
    {
        if ( !checkForSelectedFontWork( pSdrView, nCheckStatus ) )
            rSet.DisableItem( SID_FONTWORK_CHARACTER_SPACING );
        else
            SetCharacterSpacingState( pSdrView, rSet );
    }
    if ( rSet.GetItemState( SID_FONTWORK_KERN_CHARACTER_PAIRS ) != SFX_ITEM_UNKNOWN )
    {
        if ( !checkForSelectedFontWork( pSdrView, nCheckStatus ) )
            rSet.DisableItem( SID_FONTWORK_KERN_CHARACTER_PAIRS );
        else
            SetKernCharacterPairsState( pSdrView, rSet );
    }
    if ( rSet.GetItemState( SID_FONTWORK_SAME_LETTER_HEIGHTS ) != SFX_ITEM_UNKNOWN )
    {
        if ( !checkForSelectedFontWork( pSdrView, nCheckStatus ) )
            rSet.DisableItem( SID_FONTWORK_SAME_LETTER_HEIGHTS );
    }
    if ( rSet.GetItemState( SID_FONTWORK_SHAPE_TYPE ) != SFX_ITEM_UNKNOWN )
    {
        if ( !checkForSelectedFontWork( pSdrView, nCheckStatus  ) )
            rSet.DisableItem( SID_FONTWORK_SHAPE_TYPE );
        else
            SetFontWorkShapeTypeState( pSdrView, rSet );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
