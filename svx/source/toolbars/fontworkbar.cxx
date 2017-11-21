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
#include <svx/unoapi.hxx>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XEnhancedCustomShapeDefaulter.hpp>
#include <svx/dialmgr.hxx>
#include <svx/svdoashp.hxx>
#include <svx/strings.hrc>
#include <svx/svdview.hxx>
#include <svx/sdasitm.hxx>
#include <svx/gallery.hxx>
#include <svx/fmmodel.hxx>
#include <svx/fmpage.hxx>
#include <svl/itempool.hxx>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <sfx2/bindings.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/kernitem.hxx>
#include <svx/sdrpaintwindow.hxx>

#include <svx/svxids.hrc>
#include <svx/fontworkbar.hxx>
#include <svx/fontworkgallery.hxx>


using namespace ::svx;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;

void SetAlignmentState( SdrView const * pSdrView, SfxItemSet& rSet )
{
    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    const size_t nCount = rMarkList.GetMarkCount();

    sal_Int32   nAlignment = -1;
    for( size_t i = 0; i < nCount; ++i )
    {
        SdrObject* pObj = rMarkList.GetMark( i )->GetMarkedSdrObj();
        if( dynamic_cast<const SdrObjCustomShape*>( pObj) !=  nullptr )
        {
            sal_Int32 nOldAlignment = nAlignment;
            const SdrTextHorzAdjustItem&      rTextHorzAdjustItem    = pObj->GetMergedItem( SDRATTR_TEXT_HORZADJUST );
            const SdrTextFitToSizeTypeItem&   rTextFitToSizeTypeItem = pObj->GetMergedItem( SDRATTR_TEXT_FITTOSIZE );
            switch ( rTextHorzAdjustItem.GetValue() )
            {
                case SDRTEXTHORZADJUST_LEFT   : nAlignment = 0; break;
                case SDRTEXTHORZADJUST_CENTER : nAlignment = 1; break;
                case SDRTEXTHORZADJUST_RIGHT  : nAlignment = 2; break;
                case SDRTEXTHORZADJUST_BLOCK  :
                {
                    auto const fit(rTextFitToSizeTypeItem.GetValue());
                    if (fit == drawing::TextFitToSizeType_NONE)
                    {
                        nAlignment = 3;
                    }
                    else if (fit == drawing::TextFitToSizeType_ALLLINES ||
                             fit == drawing::TextFitToSizeType_PROPORTIONAL)
                    {
                        nAlignment = 4;
                    }
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

void SetCharacterSpacingState( SdrView const * pSdrView, SfxItemSet& rSet )
{
    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    const size_t nCount = rMarkList.GetMarkCount();

    sal_Int32   nCharacterSpacing = -1;
    for( size_t i = 0; i < nCount; ++i )
    {
        SdrObject* pObj = rMarkList.GetMark( i )->GetMarkedSdrObj();
        if( dynamic_cast<const SdrObjCustomShape*>( pObj) !=  nullptr )
        {
            sal_Int32 nOldCharacterSpacing = nCharacterSpacing;
            const SvxCharScaleWidthItem& rCharScaleWidthItem = pObj->GetMergedItem( EE_CHAR_FONTWIDTH );
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


void SetKernCharacterPairsState( SdrView const * pSdrView, SfxItemSet& rSet )
{
    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    const size_t nCount = rMarkList.GetMarkCount();

    bool    bChecked = false;
    for( size_t i = 0; i < nCount; ++i )
    {
        SdrObject* pObj = rMarkList.GetMark( i )->GetMarkedSdrObj();
        if( dynamic_cast<const SdrObjCustomShape*>( pObj) !=  nullptr )
        {
            const SvxKerningItem& rKerningItem = pObj->GetMergedItem( EE_CHAR_KERNING );
            if ( rKerningItem.GetValue() )
                bChecked = true;
        }
    }
    rSet.Put( SfxBoolItem( SID_FONTWORK_KERN_CHARACTER_PAIRS, bChecked ) );
}

void SetFontWorkShapeTypeState( SdrView const * pSdrView, SfxItemSet& rSet )
{
    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    const size_t nCount = rMarkList.GetMarkCount();

    OUString aFontWorkShapeType;

    for( size_t i = 0; i < nCount; ++i )
    {
        SdrObject* pObj = rMarkList.GetMark( i )->GetMarkedSdrObj();
        if( dynamic_cast<const SdrObjCustomShape*>( pObj) !=  nullptr )
        {
            const SdrCustomShapeGeometryItem aGeometryItem( pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
            const Any* pAny = aGeometryItem.GetPropertyValueByName( "Type" );
            if( pAny )
            {
                OUString aType;
                if ( *pAny >>= aType )
                {
                    if ( !aFontWorkShapeType.isEmpty() )
                    {
                        if ( aFontWorkShapeType != aType )  // different FontWorkShapeTypes selected ?
                        {
                            aFontWorkShapeType.clear();
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

// Declare the default interface. (The slotmap must not be empty, so
// we enter something which never occurs here (hopefully).)
static SfxSlot aFontworkBarSlots_Impl[] =
{
    { 0, SfxGroupId::NONE, SfxSlotMode::NONE, 0, 0, nullptr, nullptr, nullptr, nullptr, nullptr, 0, SfxDisableFlags::NONE, nullptr }
};

SFX_IMPL_INTERFACE(FontworkBar, SfxShell)

void FontworkBar::InitInterface_Impl()
{
    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT, SfxVisibilityFlags::Invisible, ToolbarId::Svx_Fontwork_Bar);
}


FontworkBar::FontworkBar(SfxViewShell* pViewShell )
: SfxShell(pViewShell)
{
    DBG_ASSERT( pViewShell, "svx::FontworkBar::FontworkBar(), I need a viewshell!" );
    if( pViewShell )
        SetPool(&pViewShell->GetPool());

    SetName( SvxResId( RID_SVX_FONTWORK_BAR ));
}

FontworkBar::~FontworkBar()
{
    SetRepeatTarget(nullptr);
}

static vcl::Window* ImpGetViewWin(SdrView const * pView)
{
    if( pView )
    {
        const sal_uInt32 nCount(pView->PaintWindowCount());
        for(sal_uInt32 nNum(0); nNum < nCount; nNum++)
        {
            OutputDevice* pOut = &(pView->GetPaintWindow(nNum)->GetOutputDevice());

            if(OUTDEV_WINDOW == pOut->GetOutDevType())
            {
                return static_cast<vcl::Window*>(pOut);
            }
        }
    }

    return nullptr;
}

namespace svx {
bool checkForSelectedFontWork( SdrView const * pSdrView, sal_uInt32& nCheckStatus )
{
    if ( nCheckStatus & 2 )
        return ( nCheckStatus & 1 ) != 0;

    static const char sTextPath[] = "TextPath";

    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    const size_t nCount = rMarkList.GetMarkCount();
    bool bFound = false;
    for(size_t i=0; (i<nCount) && !bFound ; ++i)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
        if( dynamic_cast<const SdrObjCustomShape*>( pObj) !=  nullptr )
        {
            const SdrCustomShapeGeometryItem aGeometryItem( pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
            const Any* pAny = aGeometryItem.GetPropertyValueByName( sTextPath, sTextPath );
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

static void impl_execute( SfxRequest const & rReq, SdrCustomShapeGeometryItem& rGeometryItem, SdrObject* pObj )
{
    sal_uInt16 nSID = rReq.GetSlot();
    switch( nSID )
    {
        case SID_FONTWORK_SAME_LETTER_HEIGHTS:
        {
            css::uno::Any* pAny = rGeometryItem.GetPropertyValueByName( "TextPath", "SameLetterHeights" );
            if( pAny )
            {
                bool bOn = false;
                (*pAny) >>= bOn;
                bOn = !bOn;
                (*pAny) <<= bOn;
            }
        }
        break;

        case SID_FONTWORK_ALIGNMENT:
        {
            if( rReq.GetArgs() && rReq.GetArgs()->GetItemState( SID_FONTWORK_ALIGNMENT ) == SfxItemState::SET )
            {
                sal_Int32 nValue = rReq.GetArgs()->GetItem<SfxInt32Item>(SID_FONTWORK_ALIGNMENT)->GetValue();
                if ( ( nValue >= 0 ) && ( nValue < 5 ) )
                {
                    drawing::TextFitToSizeType eFTS = drawing::TextFitToSizeType_NONE;
                    SdrTextHorzAdjust eHorzAdjust;
                    switch ( nValue )
                    {
                        case 4 : eFTS = drawing::TextFitToSizeType_ALLLINES; SAL_FALLTHROUGH;
                        case 3 : eHorzAdjust = SDRTEXTHORZADJUST_BLOCK; break;
                        default: eHorzAdjust = SDRTEXTHORZADJUST_LEFT; break;
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
            if( rReq.GetArgs() && ( rReq.GetArgs()->GetItemState( SID_FONTWORK_CHARACTER_SPACING ) == SfxItemState::SET ) )
            {
                sal_Int32 nCharSpacing = rReq.GetArgs()->GetItem<SfxInt32Item>(SID_FONTWORK_CHARACTER_SPACING)->GetValue();
                pObj->SetMergedItem( SvxCharScaleWidthItem( (sal_uInt16)nCharSpacing, EE_CHAR_FONTWIDTH ) );
                pObj->BroadcastObjectChange();
            }
        }
        break;

        case SID_FONTWORK_KERN_CHARACTER_PAIRS:
        {
            if( rReq.GetArgs() && ( rReq.GetArgs()->GetItemState( SID_FONTWORK_KERN_CHARACTER_PAIRS ) == SfxItemState::SET ) )
            {
                // sal_Bool bKernCharacterPairs = ((const SfxBoolItem*)rReq.GetArgs()->GetItem(SID_FONTWORK_KERN_CHARACTER_PAIRS))->GetValue();
//TODO:             pObj->SetMergedItem( SvxCharScaleWidthItem( (sal_uInt16)nCharSpacing, EE_CHAR_FONTWIDTH ) );
                pObj->BroadcastObjectChange();
            }
        }
        break;
    }
}

void GetGeometryForCustomShape( SdrCustomShapeGeometryItem& rGeometryItem, const OUString& rCustomShape )
{
    const OUString sType( "Type" );

    css::beans::PropertyValue aPropVal;
    aPropVal.Name = sType;
    aPropVal.Value <<= rCustomShape;
    rGeometryItem.SetPropertyValue( aPropVal );

    const OUString sAdjustmentValues( "AdjustmentValues" );
    const OUString sCoordinateOrigin( "CoordinateOrigin" );
    const OUString sCoordinateSize( "CoordinateSize" );
    const OUString sEquations( "Equations" );
    const OUString sHandles( "Handles" );
    const OUString sPath( "Path" );
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
        std::vector< OUString > aObjList;
        if ( GalleryExplorer::FillObjListTitle( GALLERY_THEME_POWERPOINT, aObjList ) )
        {
            for ( std::vector<OUString>::size_type i = 0; i < aObjList.size(); i++ )
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
                            const SdrCustomShapeGeometryItem& rSourceGeometry = pSourceObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );
                            const css::uno::Any* pAny = rSourceGeometry.GetPropertyValueByName( sType );
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


void FontworkBar::execute( SdrView* pSdrView, SfxRequest const & rReq, SfxBindings& rBindings )
{
    const char* pStrResId = nullptr;

    sal_uInt16 nSID = rReq.GetSlot();
    switch( nSID )
    {
        case SID_FONTWORK_GALLERY_FLOATER:
        {
            ScopedVclPtrInstance< FontWorkGalleryDialog > aDlg( pSdrView, ImpGetViewWin(pSdrView) );
            aDlg->Execute();
        }
        break;

        case SID_FONTWORK_SHAPE_TYPE:
        {
            OUString aCustomShape;
            const SfxItemSet* pArgs = rReq.GetArgs();
            if ( pArgs )
            {
                const SfxStringItem& rItm = static_cast<const SfxStringItem&>(pArgs->Get( rReq.GetSlot() ));
                aCustomShape = rItm.GetValue();
            }
            if ( !aCustomShape.isEmpty() )
            {
                const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
                const size_t nCount = rMarkList.GetMarkCount();
                for( size_t i = 0; i < nCount; ++i )
                {
                    SdrObject* pObj = rMarkList.GetMark( i )->GetMarkedSdrObj();
                    if( dynamic_cast<const SdrObjCustomShape*>( pObj) !=  nullptr )
                    {
                        const bool bUndo = pSdrView->IsUndoEnabled();

                        if( bUndo )
                        {
                            OUString aStr( SvxResId( RID_SVXSTR_UNDO_APPLY_FONTWORK_SHAPE ) );
                            pSdrView->BegUndo( aStr );
                            pSdrView->AddUndo( pSdrView->GetModel()->GetSdrUndoFactory().CreateUndoAttrObject( *pObj ) );
                        }
                        SdrCustomShapeGeometryItem aGeometryItem( pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
                        GetGeometryForCustomShape( aGeometryItem, aCustomShape );
                        pObj->SetMergedItem( aGeometryItem );

                        Reference< drawing::XShape > aXShape = GetXShapeForSdrObject( static_cast<SdrObjCustomShape*>(pObj) );
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
            if( rReq.GetArgs() && ( rReq.GetArgs()->GetItemState( SID_FONTWORK_CHARACTER_SPACING ) == SfxItemState::SET ) )
            {
                sal_Int32 nCharSpacing = rReq.GetArgs()->GetItem<SfxInt32Item>(SID_FONTWORK_CHARACTER_SPACING)->GetValue();
                ScopedVclPtrInstance< FontworkCharacterSpacingDialog > aDlg( nullptr, nCharSpacing );
                sal_uInt16 nRet = aDlg->Execute();
                if( nRet != 0 )
                {
                    SfxInt32Item aItem( SID_FONTWORK_CHARACTER_SPACING, aDlg->getScale() );
                    SfxPoolItem* aItems[] = { &aItem, nullptr };
                    rBindings.Execute( SID_FONTWORK_CHARACTER_SPACING, const_cast<const SfxPoolItem**>(aItems) );
                }
            }
        }
        break;

        case SID_FONTWORK_SHAPE:
        case SID_FONTWORK_ALIGNMENT:
        {
            if ( !pStrResId )
                pStrResId = RID_SVXSTR_UNDO_APPLY_FONTWORK_ALIGNMENT;
            SAL_FALLTHROUGH;
        }
        case SID_FONTWORK_CHARACTER_SPACING:
        {
            if ( !pStrResId )
                pStrResId = RID_SVXSTR_UNDO_APPLY_FONTWORK_CHARACTER_SPACING;
            SAL_FALLTHROUGH;
        }
        case SID_FONTWORK_KERN_CHARACTER_PAIRS:
        {
            if ( !pStrResId )
                pStrResId = RID_SVXSTR_UNDO_APPLY_FONTWORK_CHARACTER_SPACING;
            SAL_FALLTHROUGH;
        }
        case SID_FONTWORK_SAME_LETTER_HEIGHTS:
        {
            if ( !pStrResId )
                pStrResId = RID_SVXSTR_UNDO_APPLY_FONTWORK_SAME_LETTER_HEIGHT;

            const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
            const size_t nCount = rMarkList.GetMarkCount();
            for( size_t i = 0; i < nCount; ++i )
            {
                SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
                if( dynamic_cast<const SdrObjCustomShape*>( pObj) !=  nullptr )
                {
                    const bool bUndo = pSdrView->IsUndoEnabled();
                    if( bUndo )
                    {
                        OUString aStr( SvxResId( pStrResId ) );
                        pSdrView->BegUndo( aStr );
                        pSdrView->AddUndo( pSdrView->GetModel()->GetSdrUndoFactory().CreateUndoAttrObject( *pObj ) );
                    }
                    SdrCustomShapeGeometryItem aGeometryItem( pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
                    impl_execute( rReq, aGeometryItem, pObj );
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

void FontworkBar::getState( SdrView const * pSdrView, SfxItemSet& rSet )
{
    sal_uInt32 nCheckStatus = 0;

    if ( rSet.GetItemState( SID_FONTWORK_ALIGNMENT_FLOATER ) != SfxItemState::UNKNOWN )
    {
        if ( !checkForSelectedFontWork( pSdrView, nCheckStatus ) )
            rSet.DisableItem( SID_FONTWORK_ALIGNMENT_FLOATER );
    }
    if ( rSet.GetItemState( SID_FONTWORK_ALIGNMENT ) != SfxItemState::UNKNOWN )
    {
        if ( !checkForSelectedFontWork( pSdrView, nCheckStatus ) )
            rSet.DisableItem( SID_FONTWORK_ALIGNMENT );
        else
            SetAlignmentState( pSdrView, rSet );
    }
    if ( rSet.GetItemState( SID_FONTWORK_CHARACTER_SPACING_FLOATER ) != SfxItemState::UNKNOWN )
    {
        if ( !checkForSelectedFontWork( pSdrView, nCheckStatus ) )
            rSet.DisableItem( SID_FONTWORK_CHARACTER_SPACING_FLOATER );
    }
    if ( rSet.GetItemState( SID_FONTWORK_CHARACTER_SPACING ) != SfxItemState::UNKNOWN )
    {
        if ( !checkForSelectedFontWork( pSdrView, nCheckStatus ) )
            rSet.DisableItem( SID_FONTWORK_CHARACTER_SPACING );
        else
            SetCharacterSpacingState( pSdrView, rSet );
    }
    if ( rSet.GetItemState( SID_FONTWORK_KERN_CHARACTER_PAIRS ) != SfxItemState::UNKNOWN )
    {
        if ( !checkForSelectedFontWork( pSdrView, nCheckStatus ) )
            rSet.DisableItem( SID_FONTWORK_KERN_CHARACTER_PAIRS );
        else
            SetKernCharacterPairsState( pSdrView, rSet );
    }
    if ( rSet.GetItemState( SID_FONTWORK_SAME_LETTER_HEIGHTS ) != SfxItemState::UNKNOWN )
    {
        if ( !checkForSelectedFontWork( pSdrView, nCheckStatus ) )
            rSet.DisableItem( SID_FONTWORK_SAME_LETTER_HEIGHTS );
    }
    if ( rSet.GetItemState( SID_FONTWORK_SHAPE_TYPE ) != SfxItemState::UNKNOWN )
    {
        if ( !checkForSelectedFontWork( pSdrView, nCheckStatus  ) )
            rSet.DisableItem( SID_FONTWORK_SHAPE_TYPE );
        else
            SetFontWorkShapeTypeState( pSdrView, rSet );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
