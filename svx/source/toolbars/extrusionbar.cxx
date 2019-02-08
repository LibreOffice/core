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


#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterType.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>
#include <com/sun/star/drawing/Position3D.hpp>
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <svx/svxids.hrc>
#include <svx/svdundo.hxx>
#include <sfx2/app.hxx>
#include <sfx2/request.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/bindings.hxx>
#include <svx/xsflclit.hxx>
#include <svx/dialmgr.hxx>
#include <svx/svdoashp.hxx>
#include <svx/strings.hrc>
#include <svx/svdview.hxx>
#include <editeng/colritem.hxx>
#include <svx/chrtitem.hxx>

#include <svx/extrusionbar.hxx>
#include <extrusiondepthdialog.hxx>

using namespace ::svx;
using namespace ::cppu;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::uno;

// Declare the default interface. (The slotmap must not be empty, so
// we enter something which never occurs here (hopefully).)
static SfxSlot aExtrusionBarSlots_Impl[] =
{
    { 0, SfxGroupId::NONE, SfxSlotMode::NONE, 0, 0, nullptr, nullptr, nullptr, nullptr, nullptr, 0, SfxDisableFlags::NONE, nullptr }
};

SFX_IMPL_INTERFACE(ExtrusionBar, SfxShell)

void ExtrusionBar::InitInterface_Impl()
{
    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT, SfxVisibilityFlags::Invisible, ToolbarId::Svx_Extrusion_Bar);
}


ExtrusionBar::ExtrusionBar(SfxViewShell* pViewShell )
: SfxShell(pViewShell)
{
    DBG_ASSERT( pViewShell, "svx::ExtrusionBar::ExtrusionBar(), I need a viewshell!" );
    if( pViewShell )
        SetPool(&pViewShell->GetPool());

    SetName(SvxResId(RID_SVX_EXTRUSION_BAR));
}

ExtrusionBar::~ExtrusionBar()
{
    SetRepeatTarget(nullptr);
}

static void getLightingDirectionDefaults( const Direction3D **pLighting1Defaults, const Direction3D **pLighting2Defaults )
{

    static const Direction3D aLighting1Defaults[9] =
    {
        Direction3D( -50000, -50000, 10000 ),
        Direction3D( 0, -50000, 10000 ),
        Direction3D( 50000, -50000, 10000 ),
        Direction3D( -50000, 0, 10000 ),
        Direction3D( 0, 0, 10000 ),
        Direction3D( 50000, 0, 10000 ),
        Direction3D( -50000, 50000, 10000 ),
        Direction3D( 0, 50000, 10000 ),
        Direction3D( 50000, 50000, 10000 )
    };

    static const Direction3D aLighting2Defaults[9] =
    {
        Direction3D( 50000,0, 10000 ),
        Direction3D( 0, 50000, 10000 ),
        Direction3D( -50000, 0, 10000 ),
        Direction3D( 50000, 0, 10000 ),
        Direction3D( 0, 0, 10000 ),
        Direction3D( -50000, 0, 10000 ),
        Direction3D( 50000, 0, 10000 ),
        Direction3D( 0, -50000, 10000 ),
        Direction3D( -50000, 0, 10000 )
    };

    *pLighting1Defaults = aLighting1Defaults;
    *pLighting2Defaults = aLighting2Defaults;
};

static void impl_execute( SfxRequest const & rReq, SdrCustomShapeGeometryItem& rGeometryItem, SdrObject* pObj )
{
    static const char sExtrusion[] = "Extrusion";
    static const char sRotateAngle[] = "RotateAngle";

    sal_uInt16 nSID = rReq.GetSlot();
    switch( nSID )
    {
    case SID_EXTRUSION_TOGGLE:
    {
        css::uno::Any* pAny = rGeometryItem.GetPropertyValueByName( sExtrusion, sExtrusion );

        if( pAny )
        {
            bool bOn(false);
            (*pAny) >>= bOn;
            bOn = !bOn;
            (*pAny) <<= bOn;
        }
        else
        {
            css::beans::PropertyValue aPropValue;
            aPropValue.Name = sExtrusion;
            aPropValue.Value <<= true;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );
        }
    }
    break;

    case SID_EXTRUSION_TILT_DOWN:
    case SID_EXTRUSION_TILT_UP:
    case SID_EXTRUSION_TILT_LEFT:
    case SID_EXTRUSION_TILT_RIGHT:
    {
        bool bHorizontal = ( nSID == SID_EXTRUSION_TILT_DOWN ) || ( nSID == SID_EXTRUSION_TILT_UP );
        sal_Int32 nDiff = ( nSID == SID_EXTRUSION_TILT_LEFT ) || ( nSID == SID_EXTRUSION_TILT_UP ) ? 5 : -5;
        EnhancedCustomShapeParameterPair aRotateAnglePropPair;
        double fX = 0.0;
        double fY = 0.0;
        aRotateAnglePropPair.First.Value <<= fX;
        aRotateAnglePropPair.First.Type = EnhancedCustomShapeParameterType::NORMAL;
        aRotateAnglePropPair.Second.Value <<= fY;
        aRotateAnglePropPair.Second.Type = EnhancedCustomShapeParameterType::NORMAL;
        css::uno::Any* pAny = rGeometryItem.GetPropertyValueByName( sExtrusion, sRotateAngle );
        if( pAny && ( *pAny >>= aRotateAnglePropPair ) )
        {
            aRotateAnglePropPair.First.Value >>= fX;
            aRotateAnglePropPair.Second.Value >>= fY;
        }
        if ( bHorizontal )
            fX += nDiff;
        else
            fY += nDiff;
        aRotateAnglePropPair.First.Value <<= fX;
        aRotateAnglePropPair.Second.Value <<= fY;
        css::beans::PropertyValue aPropValue;
        aPropValue.Name = sRotateAngle;
        aPropValue.Value <<= aRotateAnglePropPair;
        rGeometryItem.SetPropertyValue( sExtrusion, aPropValue );
    }
    break;

    case SID_EXTRUSION_DIRECTION:
    {
        if( rReq.GetArgs() && rReq.GetArgs()->GetItemState( SID_EXTRUSION_DIRECTION ) == SfxItemState::SET )
        {
            sal_Int32 nSkew = rReq.GetArgs()->GetItem<SfxInt32Item>(SID_EXTRUSION_DIRECTION)->GetValue();

            Position3D  aViewPoint( 3472, -3472, 25000 );
            double      fOriginX = 0.50;
            double      fOriginY = -0.50;
            double      fSkewAngle = nSkew;
            double      fSkew = 50.0;

            switch( nSkew )
            {
            case 135:
                aViewPoint.PositionY = 3472;
                fOriginY = 0.50;
                break;
            case 90:
                aViewPoint.PositionX = 0;
                aViewPoint.PositionY = 3472;
                fOriginX = 0;
                fOriginY = -0.50;
                break;
            case 45:
                aViewPoint.PositionX = -3472;
                aViewPoint.PositionY = 3472;
                fOriginX = -0.50;
                fOriginY = 0.50;
                break;
            case 180:
                aViewPoint.PositionY = 0;
                fOriginY = 0;
                break;
            case 0:
                aViewPoint.PositionX = 0;
                aViewPoint.PositionY = 0;
                fOriginX = 0;
                fOriginY = 0;
                fSkew = 0.0;
                break;
            case -360:
                aViewPoint.PositionX = -3472;
                aViewPoint.PositionY = 0;
                fOriginX = -0.50;
                fOriginY = 0;
                break;
            case -90:
                aViewPoint.PositionX = 0;
                fOriginX = 0;
                break;
            case -45:
                aViewPoint.PositionX = -3472;
                fOriginX = -0.50;
                break;
            }

            css::beans::PropertyValue aPropValue;

            aPropValue.Name = "ViewPoint";
            aPropValue.Value <<= aViewPoint;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );


            EnhancedCustomShapeParameterPair aOriginPropPair;
            aOriginPropPair.First.Value <<= fOriginX;
            aOriginPropPair.First.Type = EnhancedCustomShapeParameterType::NORMAL;
            aOriginPropPair.Second.Value <<= fOriginY;
            aOriginPropPair.Second.Type = EnhancedCustomShapeParameterType::NORMAL;
            aPropValue.Name = "Origin";
            aPropValue.Value <<= aOriginPropPair;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );

            EnhancedCustomShapeParameterPair aSkewPropPair;
            aSkewPropPair.First.Value <<= fSkew;
            aSkewPropPair.First.Type = EnhancedCustomShapeParameterType::NORMAL;
            aSkewPropPair.Second.Value <<= fSkewAngle;
            aSkewPropPair.Second.Type = EnhancedCustomShapeParameterType::NORMAL;
            aPropValue.Name = "Skew";
            aPropValue.Value <<= aSkewPropPair;
            rGeometryItem.SetPropertyValue( sExtrusion, aPropValue );
        }
    }
    break;
    case SID_EXTRUSION_PROJECTION:
    {
        if( rReq.GetArgs() && rReq.GetArgs()->GetItemState( SID_EXTRUSION_PROJECTION ) == SfxItemState::SET )
        {
            sal_Int32 nProjection = rReq.GetArgs()->GetItem<SfxInt32Item>(SID_EXTRUSION_PROJECTION)->GetValue();
            ProjectionMode eProjectionMode = nProjection == 1 ? ProjectionMode_PARALLEL : ProjectionMode_PERSPECTIVE;
            css::beans::PropertyValue aPropValue;
            aPropValue.Name = "ProjectionMode";
            aPropValue.Value <<= eProjectionMode;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );
        }
    }
    break;
    case SID_EXTRUSION_DEPTH:
    {
        if( rReq.GetArgs() && rReq.GetArgs()->GetItemState( SID_EXTRUSION_DEPTH ) == SfxItemState::SET)
        {
            double fDepth = rReq.GetArgs()->GetItem<SvxDoubleItem>(SID_EXTRUSION_DEPTH)->GetValue();
            EnhancedCustomShapeParameterPair aDepthPropPair;
            aDepthPropPair.First.Value <<= fDepth;
            aDepthPropPair.First.Type = EnhancedCustomShapeParameterType::NORMAL;
            aDepthPropPair.Second.Value <<= 0.0; // fraction
            aDepthPropPair.Second.Type = EnhancedCustomShapeParameterType::NORMAL;

            css::beans::PropertyValue aPropValue;
            aPropValue.Name = "Depth";
            aPropValue.Value <<= aDepthPropPair;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );
        }
    }
    break;
    case SID_EXTRUSION_3D_COLOR:
    {
        if( rReq.GetArgs() && rReq.GetArgs()->GetItemState( SID_EXTRUSION_3D_COLOR ) == SfxItemState::SET)
        {
            Color aColor( static_cast<const SvxColorItem&>(rReq.GetArgs()->Get(SID_EXTRUSION_3D_COLOR)).GetValue() );

            const bool bAuto = aColor == COL_AUTO;

            css::beans::PropertyValue aPropValue;
            aPropValue.Name = "Color";
            aPropValue.Value <<= !bAuto;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );

            if( bAuto )
            {
                pObj->ClearMergedItem( XATTR_SECONDARYFILLCOLOR );
            }
            else
            {
                pObj->SetMergedItem( XSecondaryFillColorItem( "", aColor ) );
            }
            pObj->BroadcastObjectChange();
        }
    }
    break;
    case SID_EXTRUSION_SURFACE:
    {
        if( rReq.GetArgs() && rReq.GetArgs()->GetItemState( SID_EXTRUSION_SURFACE ) == SfxItemState::SET)
        {
            sal_Int32 nSurface = rReq.GetArgs()->GetItem<SfxInt32Item>(SID_EXTRUSION_SURFACE)->GetValue();

            ShadeMode eShadeMode( ShadeMode_FLAT );
            bool bMetal = false;
            double fSpecularity = 0;
            double fDiffusion = 0;

            switch( nSurface )
            {
            case 0: // wireframe
                eShadeMode = ShadeMode_DRAFT;
                break;
            case 1: // matte
                break;
            case 2: // plastic
                fSpecularity = 122.0;
                break;
            case 3: // metal
                bMetal = true;
                fSpecularity = 122.0;
                fDiffusion = 122.0;
                break;
            }

            css::beans::PropertyValue aPropValue;
            aPropValue.Name = "ShadeMode";
            aPropValue.Value <<= eShadeMode;
            rGeometryItem.SetPropertyValue( sExtrusion, aPropValue );

            aPropValue.Name = "Metal";
            aPropValue.Value <<= bMetal;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );

            aPropValue.Name = "Specularity";
            aPropValue.Value <<= fSpecularity;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );

            aPropValue.Name = "Diffusion";
            aPropValue.Value <<= fDiffusion;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );
        }
    }
    break;
    case SID_EXTRUSION_LIGHTING_INTENSITY:
    {
        if( rReq.GetArgs() && rReq.GetArgs()->GetItemState( SID_EXTRUSION_LIGHTING_INTENSITY ) == SfxItemState::SET)
        {
            sal_Int32 nLevel = rReq.GetArgs()->GetItem<SfxInt32Item>(SID_EXTRUSION_LIGHTING_INTENSITY)->GetValue();

            double fBrightness;
            bool bHarsh2 = false;
            double fLevel1;
            double fLevel2;

            switch( nLevel )
            {
            case 0: // bright
                fBrightness = 34.0;
                bHarsh2 = false;
                fLevel1 = 66.0;
                fLevel2 = 66.0;
                break;
            case 1: // normal
                fBrightness = 15.0;
                bHarsh2 = false;
                fLevel1 = 67.0;
                fLevel2 = 37.0;
                break;
            case 2: // dim
                fBrightness = 6.0;
                bHarsh2 = true;
                fLevel1 = 79.0;
                fLevel2 = 21.0;
                break;
            }

            css::beans::PropertyValue aPropValue;
            aPropValue.Name = "Brightness";
            aPropValue.Value <<= fBrightness;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );

            aPropValue.Name = "LightFace";
            aPropValue.Value <<= true;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );

            aPropValue.Name = "FirstLightHarsh";
            aPropValue.Value <<= true;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );

            aPropValue.Name = "SecondLightHarsh";
            aPropValue.Value <<= bHarsh2;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );

            aPropValue.Name = "FirstLightLevel";
            aPropValue.Value <<= fLevel1;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );

            aPropValue.Name = "SecondLightLevel";
            aPropValue.Value <<= fLevel2;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );
        }
    }
    break;
    case SID_EXTRUSION_LIGHTING_DIRECTION:
    {
        if( rReq.GetArgs() && rReq.GetArgs()->GetItemState( SID_EXTRUSION_LIGHTING_DIRECTION ) == SfxItemState::SET)
        {
            sal_Int32 nDirection = rReq.GetArgs()->GetItem<SfxInt32Item>(SID_EXTRUSION_LIGHTING_DIRECTION)->GetValue();

            if((nDirection >= 0) && (nDirection < 9))
            {
                const Direction3D * pLighting1Defaults;
                const Direction3D * pLighting2Defaults;

                getLightingDirectionDefaults( &pLighting1Defaults, &pLighting2Defaults );

                css::beans::PropertyValue aPropValue;
                aPropValue.Name = "FirstLightDirection";
                aPropValue.Value <<= pLighting1Defaults[nDirection];
                rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );

                aPropValue.Name = "SecondLightDirection";
                aPropValue.Value <<= pLighting2Defaults[nDirection];
                rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );
            }
        }
    }
    break;

    }
}

void ExtrusionBar::execute( SdrView* pSdrView, SfxRequest const & rReq, SfxBindings& rBindings )
{
    sal_uInt16 nSID = rReq.GetSlot();
    const char* pStrResId = nullptr;

    const bool bUndo = pSdrView && pSdrView->IsUndoEnabled();

    switch( nSID )
    {
        case SID_EXTRUSION_TOGGLE:
        {
            if ( !pStrResId )
                pStrResId = RID_SVXSTR_UNDO_APPLY_EXTRUSION_ON_OFF;
            [[fallthrough]];
        }
        case SID_EXTRUSION_TILT_DOWN:
        {
            if ( !pStrResId )
                pStrResId = RID_SVXSTR_UNDO_APPLY_EXTRUSION_ROTATE_DOWN;
            [[fallthrough]];
        }
        case SID_EXTRUSION_TILT_UP:
        {
            if ( !pStrResId )
                pStrResId = RID_SVXSTR_UNDO_APPLY_EXTRUSION_ROTATE_UP;
            [[fallthrough]];
        }
        case SID_EXTRUSION_TILT_LEFT:
        {
            if ( !pStrResId )
                pStrResId = RID_SVXSTR_UNDO_APPLY_EXTRUSION_ROTATE_LEFT;
            [[fallthrough]];
        }
        case SID_EXTRUSION_TILT_RIGHT:
        {
            if ( !pStrResId )
                pStrResId = RID_SVXSTR_UNDO_APPLY_EXTRUSION_ROTATE_RIGHT;
            [[fallthrough]];
        }
        case SID_EXTRUSION_DIRECTION:
        {
            if ( !pStrResId )
                pStrResId = RID_SVXSTR_UNDO_APPLY_EXTRUSION_ORIENTATION;
            [[fallthrough]];
        }
        case SID_EXTRUSION_PROJECTION:
        {
            if ( !pStrResId )
                pStrResId = RID_SVXSTR_UNDO_APPLY_EXTRUSION_PROJECTION;
            [[fallthrough]];
        }
        case SID_EXTRUSION_DEPTH:
        {
            if ( !pStrResId )
                pStrResId = RID_SVXSTR_UNDO_APPLY_EXTRUSION_DEPTH;
            [[fallthrough]];
        }
        case SID_EXTRUSION_3D_COLOR:
        {
            if ( !pStrResId )
                pStrResId = RID_SVXSTR_UNDO_APPLY_EXTRUSION_COLOR;
            [[fallthrough]];
        }
        case SID_EXTRUSION_SURFACE:
        {
            if ( !pStrResId )
                pStrResId = RID_SVXSTR_UNDO_APPLY_EXTRUSION_SURFACE;
            [[fallthrough]];
        }
        case SID_EXTRUSION_LIGHTING_INTENSITY:
        {
            if ( !pStrResId )
                pStrResId = RID_SVXSTR_UNDO_APPLY_EXTRUSION_BRIGHTNESS;
            [[fallthrough]];
        }
        case SID_EXTRUSION_LIGHTING_DIRECTION:
        {
            if ( !pStrResId )
                pStrResId = RID_SVXSTR_UNDO_APPLY_EXTRUSION_LIGHTING;

            if (pSdrView)
            {
                const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
                const size_t nCount = rMarkList.GetMarkCount();

                for(size_t i=0; i<nCount; ++i)
                {
                    SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
                    if( dynamic_cast<const SdrObjCustomShape*>( pObj) !=  nullptr )
                    {
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

                        // simulate a context change:
                        // force SelectionHasChanged() being called
                        // so that extrusion bar will be visible/hidden
                        pSdrView->MarkListHasChanged();
                    }
                }
            }
        }
        break;

        case SID_EXTRUSION_DEPTH_DIALOG:
            if( rReq.GetArgs() &&
                (rReq.GetArgs()->GetItemState( SID_EXTRUSION_DEPTH ) == SfxItemState::SET) &&
                (rReq.GetArgs()->GetItemState( SID_ATTR_METRIC ) == SfxItemState::SET))
            {
                double fDepth = rReq.GetArgs()->GetItem<SvxDoubleItem>(SID_EXTRUSION_DEPTH)->GetValue();
                FieldUnit eUnit = static_cast<FieldUnit>(rReq.GetArgs()->GetItem<SfxUInt16Item>(SID_ATTR_METRIC)->GetValue());

                ExtrusionDepthDialog aDlg(rReq.GetFrameWeld(), fDepth, eUnit);
                sal_uInt16 nRet = aDlg.run();
                if (nRet == RET_OK)
                {
                    fDepth = aDlg.getDepth();

                    SvxDoubleItem aItem( fDepth, SID_EXTRUSION_DEPTH );
                    SfxPoolItem* aItems[] = { &aItem, nullptr };
                    rBindings.Execute( SID_EXTRUSION_DEPTH, const_cast<const SfxPoolItem**>(aItems) );
                }
            }
            break;
    }

    if( nSID == SID_EXTRUSION_TOGGLE )
    {
        static const sal_uInt16 SidArray[] = {
                SID_EXTRUSION_TILT_DOWN,
                SID_EXTRUSION_TILT_UP,
                SID_EXTRUSION_TILT_LEFT,
                SID_EXTRUSION_TILT_RIGHT,
                SID_EXTRUSION_DEPTH_FLOATER,
                SID_EXTRUSION_DIRECTION_FLOATER,
                SID_EXTRUSION_LIGHTING_FLOATER,
                SID_EXTRUSION_SURFACE_FLOATER,
                SID_EXTRUSION_3D_COLOR,
                SID_EXTRUSION_DEPTH,
                SID_EXTRUSION_DIRECTION,
                SID_EXTRUSION_PROJECTION,
                SID_EXTRUSION_LIGHTING_DIRECTION,
                SID_EXTRUSION_LIGHTING_INTENSITY,
                SID_EXTRUSION_SURFACE,
                0 };

        rBindings.Invalidate( SidArray );
    }
}

static void getExtrusionDirectionState( SdrView const * pSdrView, SfxItemSet& rSet )
{
    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    const size_t nCount = rMarkList.GetMarkCount();

    static const char  sExtrusion[] = "Extrusion";

    const css::uno::Any* pAny;

    double fFinalSkewAngle = -1;
    bool bHasCustomShape = false;

    for(size_t i=0; i<nCount; ++i)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
        if( dynamic_cast<const SdrObjCustomShape*>( pObj) !=  nullptr )
        {
            const SdrCustomShapeGeometryItem aGeometryItem( pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );

            // see if this is an extruded customshape
            if( !bHasCustomShape )
            {
                const Any* pAny_ = aGeometryItem.GetPropertyValueByName( sExtrusion, sExtrusion );
                if( pAny_ )
                    *pAny_ >>= bHasCustomShape;

                if( !bHasCustomShape )
                    continue;
            }

            bool        bParallel = true;
            Position3D  aViewPoint( 3472, -3472, 25000 );
            double      fSkewAngle = -135;

            pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, "ProjectionMode" );
            sal_Int16 nProjectionMode = sal_Int16();
            if( pAny && ( *pAny >>= nProjectionMode ) )
                bParallel = static_cast<ProjectionMode>(nProjectionMode) == ProjectionMode_PARALLEL;

            if( bParallel )
            {
                double      fSkew = 50.0;
                EnhancedCustomShapeParameterPair aSkewPropPair;
                pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, "Skew" );
                if( pAny && ( *pAny >>= aSkewPropPair ) )
                {
                    aSkewPropPair.First.Value >>= fSkew;
                    aSkewPropPair.Second.Value >>= fSkewAngle;
                }
                if ( fSkew == 0.0 )
                    fSkewAngle = 0.0;
                else if ( fSkewAngle == 0.0 )
                    fSkewAngle = -360.0;
            }
            else
            {
                double      fOriginX = 0.50;
                double      fOriginY = -0.50;
                pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, "ViewPoint" );
                if( pAny )
                    *pAny >>= aViewPoint;

                EnhancedCustomShapeParameterPair aOriginPropPair;
                pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, "Origin" );
                if( pAny && ( *pAny >>= aOriginPropPair ) )
                {
                    aOriginPropPair.First.Value >>= fOriginX;
                    aOriginPropPair.Second.Value >>= fOriginY;
                }
                fSkewAngle = -1;
                const double e = 0.0001;
                if( aViewPoint.PositionX > e )
                {
                    if( aViewPoint.PositionY > e )
                    {
                        if( (fOriginX > e ) && ( fOriginY > e ) )
                            fSkewAngle = 135.0;
                    }
                    else if( aViewPoint.PositionY < -e )
                    {
                        if( ( fOriginX > e ) && ( fOriginY < -e ) )
                            fSkewAngle = -135.0;
                    }
                    else
                    {
                        if( ( fOriginX > e ) && ( fOriginY > -e ) && ( fOriginY < e ) )
                            fSkewAngle = 180.0;
                    }
                }
                else if( aViewPoint.PositionX < -e )
                {
                    if( aViewPoint.PositionY < -e )
                    {
                        if( ( fOriginX < -e ) && ( fOriginY < -e ) )
                            fSkewAngle = -45.0;
                    }
                    else if( aViewPoint.PositionY > e )
                    {
                        if( ( fOriginX < -e ) && ( fOriginY > e ) )
                            fSkewAngle = 45.0;
                    }
                    else
                    {
                        if( ( fOriginX < e ) && ( fOriginY > -e ) && ( fOriginY < e ) )
                            fSkewAngle = -360.0;
                    }
                }
                else
                {
                    if( aViewPoint.PositionY < -e )
                    {
                        if( ( fOriginX > -e ) && ( fOriginX < e ) && ( fOriginY < -e ) )
                            fSkewAngle = -90.0;
                    }
                    else if( aViewPoint.PositionY > e )
                    {
                        if( ( fOriginX > -e ) && ( fOriginX < e ) && ( fOriginY > e ) )
                            fSkewAngle = 90.0;
                    }
                    else
                    {
                        if( ( fOriginX > -e ) && ( fOriginX < e ) && ( fOriginY > -e ) && ( fOriginY < e ) )
                            fSkewAngle = 0.0;
                    }
                }
            }

            if( rtl::math::approxEqual(fFinalSkewAngle, -1.0) )
            {
                fFinalSkewAngle = fSkewAngle;
            }
            else if( !rtl::math::approxEqual(fSkewAngle, fFinalSkewAngle) )
            {
                fFinalSkewAngle = -1.0;
            }

            if( rtl::math::approxEqual(fFinalSkewAngle, -1.0) )
                break;
        }
    }

    if( bHasCustomShape )
        rSet.Put( SfxInt32Item( SID_EXTRUSION_DIRECTION, static_cast<sal_Int32>(fFinalSkewAngle) ) );
    else
        rSet.DisableItem( SID_EXTRUSION_DIRECTION );
}

static void getExtrusionProjectionState( SdrView const * pSdrView, SfxItemSet& rSet )
{
    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    const size_t nCount = rMarkList.GetMarkCount();

    static const char  sExtrusion[] = "Extrusion";

    const css::uno::Any* pAny;

    sal_Int32 nFinalProjection = -1;
    bool bHasCustomShape = false;

    for(size_t i=0; i<nCount; ++i)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
        if( dynamic_cast<const SdrObjCustomShape*>( pObj) !=  nullptr )
        {
            // see if this is an extruded customshape
            if( !bHasCustomShape )
            {
                const SdrCustomShapeGeometryItem aGeometryItem( pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
                const Any* pAny_ = aGeometryItem.GetPropertyValueByName( sExtrusion, sExtrusion );
                if( pAny_ )
                    *pAny_ >>= bHasCustomShape;

                if( !bHasCustomShape )
                    continue;
            }

            const SdrCustomShapeGeometryItem aGeometryItem( pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );

            bool    bParallel = true;
            pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, "ProjectionMode" );
            ProjectionMode eProjectionMode;
            if( pAny && ( *pAny >>= eProjectionMode ) )
                bParallel = eProjectionMode == ProjectionMode_PARALLEL;

            if( nFinalProjection == -1 )
            {
                nFinalProjection = bParallel ? 1 : 0;
            }
            else if( nFinalProjection != (bParallel ? 1 : 0) )
            {
                nFinalProjection = -1;
                break;
            }
        }
    }

    if( bHasCustomShape )
        rSet.Put( SfxInt32Item( SID_EXTRUSION_PROJECTION, nFinalProjection ) );
    else
        rSet.DisableItem( SID_EXTRUSION_PROJECTION );
}

static void getExtrusionSurfaceState( SdrView const * pSdrView, SfxItemSet& rSet )
{
    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    const size_t nCount = rMarkList.GetMarkCount();

    static const char  sExtrusion[] = "Extrusion";

    const css::uno::Any* pAny;

    sal_Int32 nFinalSurface = -1;
    bool bHasCustomShape = false;

    for(size_t i=0; i<nCount; ++i)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
        if( dynamic_cast<const SdrObjCustomShape*>( pObj) !=  nullptr )
        {
            const SdrCustomShapeGeometryItem aGeometryItem( pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );

            // see if this is an extruded customshape
            if( !bHasCustomShape )
            {
                const Any* pAny_ = aGeometryItem.GetPropertyValueByName( sExtrusion, sExtrusion );
                if( pAny_ )
                    *pAny_ >>= bHasCustomShape;

                if( !bHasCustomShape )
                    continue;
            }

            sal_Int32 nSurface = 0; // wire frame

            ShadeMode eShadeMode( ShadeMode_FLAT );
            pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, "ShadeMode" );
            if( pAny )
                *pAny >>= eShadeMode;

            if( eShadeMode == ShadeMode_FLAT )
            {
                bool bMetal = false;
                pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, "Metal" );
                if( pAny )
                    *pAny >>= bMetal;

                if( bMetal )
                {
                    nSurface = 3; // metal
                }
                else
                {
                    double fSpecularity = 0;
                    pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, "Specularity" );
                    if( pAny )
                        *pAny >>= fSpecularity;

                    const double e = 0.0001;
                    if( (fSpecularity > -e) && (fSpecularity < e) )
                    {
                        nSurface = 1; // matte
                    }
                    else
                    {
                        nSurface = 2; // plastic
                    }
                }
            }

            if( nFinalSurface == -1 )
            {
                nFinalSurface = nSurface;
            }
            else if( nFinalSurface != nSurface )
            {
                nFinalSurface = -1;
                break;
            }
        }
    }

    if( bHasCustomShape )
        rSet.Put( SfxInt32Item( SID_EXTRUSION_SURFACE, nFinalSurface ) );
    else
        rSet.DisableItem( SID_EXTRUSION_SURFACE );
}

static void getExtrusionDepthState( SdrView const * pSdrView, SfxItemSet& rSet )
{
    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    const size_t nCount = rMarkList.GetMarkCount();

    static const char  sExtrusion[] = "Extrusion";

    const css::uno::Any* pAny;

    double fFinalDepth = -1;
    bool bHasCustomShape = false;

    for(size_t i=0; i<nCount; ++i)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
        if( dynamic_cast<const SdrObjCustomShape*>( pObj) !=  nullptr )
        {
            const SdrCustomShapeGeometryItem aGeometryItem( pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );

            // see if this is an extruded customshape
            if( !bHasCustomShape )
            {
                const Any* pAny_ = aGeometryItem.GetPropertyValueByName( sExtrusion, sExtrusion );
                if( pAny_ )
                    *pAny_ >>= bHasCustomShape;

                if( !bHasCustomShape )
                    continue;
            }

            double fDepth = 1270.0;
            pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, "Depth" );
            if( pAny )
            {
                EnhancedCustomShapeParameterPair aDepthPropPair;
                if ( *pAny >>= aDepthPropPair )
                    aDepthPropPair.First.Value >>= fDepth;
            }

            if( fFinalDepth == -1 )
            {
                fFinalDepth = fDepth;
            }
            else if( !rtl::math::approxEqual(fFinalDepth, fDepth) )
            {
                fFinalDepth = -1;
                break;
            }
        }
    }

    if( pSdrView->GetModel() )
    {
        FieldUnit eUnit = pSdrView->GetModel()->GetUIUnit();
        rSet.Put( SfxUInt16Item( SID_ATTR_METRIC, static_cast<sal_uInt16>(eUnit) ) );
    }

    if( bHasCustomShape )
        rSet.Put( SvxDoubleItem( fFinalDepth, SID_EXTRUSION_DEPTH ) );
    else
        rSet.DisableItem( SID_EXTRUSION_DEPTH );
}

static bool compare_direction( const Direction3D& d1, const Direction3D& d2 )
{
    if( ((d1.DirectionX < 0) && (d2.DirectionX < 0)) || ((d1.DirectionX == 0) && (d2.DirectionX == 0)) || ((d1.DirectionX > 0) && (d2.DirectionX > 0)) )
    {
        if( ((d1.DirectionY < 0) && (d2.DirectionY < 0)) || ((d1.DirectionY == 0) && (d2.DirectionY == 0)) || ((d1.DirectionY > 0) && (d2.DirectionY > 0)) )
        {
            if( ((d1.DirectionZ < 0) && (d2.DirectionZ < 0)) || ((d1.DirectionZ == 0) && (d2.DirectionZ == 0)) || ((d1.DirectionZ > 0) && (d2.DirectionZ > 0)) )
            {
                return true;
            }
        }
    }

    return false;
}

static void getExtrusionLightingDirectionState( SdrView const * pSdrView, SfxItemSet& rSet )
{
    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    const size_t nCount = rMarkList.GetMarkCount();

    static const char  sExtrusion[] = "Extrusion";

    const Direction3D * pLighting1Defaults;
    const Direction3D * pLighting2Defaults;

    getLightingDirectionDefaults( &pLighting1Defaults, &pLighting2Defaults );

    const css::uno::Any* pAny;

    int nFinalDirection = -1;
    bool bHasCustomShape = false;

    for(size_t i=0; i<nCount; ++i)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
        if( dynamic_cast<const SdrObjCustomShape*>( pObj) !=  nullptr )
        {
            const SdrCustomShapeGeometryItem aGeometryItem( pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );

            // see if this is an extruded customshape
            if( !bHasCustomShape )
            {
                const Any* pAny_ = aGeometryItem.GetPropertyValueByName( sExtrusion, sExtrusion );
                if( pAny_ )
                    *pAny_ >>= bHasCustomShape;

                if( !bHasCustomShape )
                    continue;
            }

            Direction3D aFirstLightDirection( 50000, 0, 10000 );
            Direction3D aSecondLightDirection( -50000, 0, 10000 );

            pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, "FirstLightDirection" );
            if( pAny )
                *pAny >>= aFirstLightDirection;

            pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, "SecondLightDirection" );
            if( pAny )
                *pAny >>= aSecondLightDirection;

            int nDirection = -1;

            int j;
            for( j = 0; j < 9; j++ )
            {
                if( compare_direction( aFirstLightDirection, pLighting1Defaults[j] ) &&
                    compare_direction( aSecondLightDirection, pLighting2Defaults[j] ))
                {
                    nDirection = j;
                    break;
                }
            }

            if( nFinalDirection == -1 )
            {
                nFinalDirection = nDirection;
            }
            else if( nDirection != nFinalDirection )
            {
                nFinalDirection = -1;
            }

            if( nFinalDirection == -1 )
                break;
        }
    }

    if( bHasCustomShape )
        rSet.Put( SfxInt32Item( SID_EXTRUSION_LIGHTING_DIRECTION, static_cast<sal_Int32>(nFinalDirection) ) );
    else
        rSet.DisableItem( SID_EXTRUSION_LIGHTING_DIRECTION );
}

static void getExtrusionLightingIntensityState( SdrView const * pSdrView, SfxItemSet& rSet )
{
    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    const size_t nCount = rMarkList.GetMarkCount();

    static const char  sExtrusion[] = "Extrusion";

    const css::uno::Any* pAny;

    int nFinalLevel = -1;
    bool bHasCustomShape = false;

    for(size_t i=0; i<nCount; ++i)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
        if( dynamic_cast<const SdrObjCustomShape*>( pObj) !=  nullptr )
        {
            const SdrCustomShapeGeometryItem aGeometryItem( pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );

            // see if this is an extruded customshape
            if( !bHasCustomShape )
            {
                const Any* pAny_ = aGeometryItem.GetPropertyValueByName( sExtrusion, sExtrusion );
                if( pAny_ )
                    *pAny_ >>= bHasCustomShape;

                if( !bHasCustomShape )
                    continue;
            }

            double fBrightness = 22178.0 / 655.36;
            pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, "Brightness" );
            if( pAny )
                *pAny >>= fBrightness;

            int nLevel;
            if( fBrightness >= 30.0 )
            {
                nLevel = 0; // Bright
            }
            else if( fBrightness >= 10.0 )
            {
                nLevel = 1; // Normal;
            }
            else
            {
                nLevel = 2; // Dim
            }

            if( nFinalLevel == -1 )
            {
                nFinalLevel = nLevel;
            }
            else if( nFinalLevel != nLevel )
            {
                nFinalLevel = -1;
                break;
            }
        }
    }

    if( bHasCustomShape )
        rSet.Put( SfxInt32Item( SID_EXTRUSION_LIGHTING_INTENSITY, nFinalLevel ) );
    else
        rSet.DisableItem( SID_EXTRUSION_LIGHTING_INTENSITY );
}

static void getExtrusionColorState( SdrView const * pSdrView, SfxItemSet& rSet )
{
    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    const size_t nCount = rMarkList.GetMarkCount();

    static const char  sExtrusion[] = "Extrusion";

    const css::uno::Any* pAny;

    bool bInit = false;
    bool bAmbigius = false;
    Color aFinalColor;
    bool bHasCustomShape = false;

    for(size_t i=0; i<nCount; ++i)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
        if( dynamic_cast<const SdrObjCustomShape*>( pObj) !=  nullptr )
        {
            const SdrCustomShapeGeometryItem aGeometryItem( pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );

            // see if this is an extruded customshape
            if( !bHasCustomShape )
            {
                const Any* pAny_ = aGeometryItem.GetPropertyValueByName( sExtrusion, sExtrusion );
                if( pAny_ )
                    *pAny_ >>= bHasCustomShape;

                if( !bHasCustomShape )
                    continue;
            }

            Color aColor;

            bool bUseColor = false;
            pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, "Color" );
            if( pAny )
                *pAny >>= bUseColor;

            if( bUseColor )
            {
                const XSecondaryFillColorItem& rItem = pObj->GetMergedItem( XATTR_SECONDARYFILLCOLOR );
                aColor = rItem.GetColorValue();
            }
            else
            {
                aColor = COL_AUTO;
            }

            if( !bInit )
            {
                aFinalColor = aColor;
                bInit = true;
            }
            else if( aFinalColor != aColor )
            {
                bAmbigius = true;
                break;
            }
        }
    }

    if( bAmbigius )
        aFinalColor = COL_AUTO;

    if( bHasCustomShape )
        rSet.Put( SvxColorItem( aFinalColor, SID_EXTRUSION_3D_COLOR ) );
    else
        rSet.DisableItem( SID_EXTRUSION_3D_COLOR );
}

namespace svx {
bool checkForSelectedCustomShapes( SdrView const * pSdrView, bool bOnlyExtruded )
{
    static const char  sExtrusion[] = "Extrusion";

    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    const size_t nCount = rMarkList.GetMarkCount();
    bool bFound = false;

    for(size_t i=0;(i<nCount) && !bFound ; ++i)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
        if( dynamic_cast<const SdrObjCustomShape*>( pObj) !=  nullptr )
        {
            if( bOnlyExtruded )
            {
                const SdrCustomShapeGeometryItem aGeometryItem( pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
                const Any* pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sExtrusion );
                if( pAny )
                    *pAny >>= bFound;
            }
            else
            {
                bFound = true;
            }
        }
    }

    return bFound;
}
}

void ExtrusionBar::getState( SdrView const * pSdrView, SfxItemSet& rSet )
{
    if (rSet.GetItemState(SID_EXTRUSION_DIRECTION) != SfxItemState::UNKNOWN)
    {
        getExtrusionDirectionState( pSdrView, rSet );
    }
    if (rSet.GetItemState(SID_EXTRUSION_PROJECTION) != SfxItemState::UNKNOWN)
    {
        getExtrusionProjectionState( pSdrView, rSet );
    }
    const bool bOnlyExtrudedCustomShapes =
        checkForSelectedCustomShapes( pSdrView, true );
    if (rSet.GetItemState(SID_EXTRUSION_TILT_DOWN) != SfxItemState::UNKNOWN)
    {
        if (! bOnlyExtrudedCustomShapes)
            rSet.DisableItem( SID_EXTRUSION_TILT_DOWN );
    }
    if (rSet.GetItemState(SID_EXTRUSION_TILT_DOWN) != SfxItemState::UNKNOWN)
    {
        if (! bOnlyExtrudedCustomShapes)
            rSet.DisableItem( SID_EXTRUSION_TILT_DOWN );
    }
    if (rSet.GetItemState(SID_EXTRUSION_TILT_UP) != SfxItemState::UNKNOWN)
    {
        if (! bOnlyExtrudedCustomShapes)
            rSet.DisableItem( SID_EXTRUSION_TILT_UP );
    }
    if (rSet.GetItemState(SID_EXTRUSION_TILT_LEFT) != SfxItemState::UNKNOWN)
    {
        if (! bOnlyExtrudedCustomShapes)
            rSet.DisableItem( SID_EXTRUSION_TILT_LEFT );
    }
    if (rSet.GetItemState(SID_EXTRUSION_TILT_RIGHT) != SfxItemState::UNKNOWN)
    {
        if (! bOnlyExtrudedCustomShapes)
            rSet.DisableItem( SID_EXTRUSION_TILT_RIGHT );
    }
    if (rSet.GetItemState(SID_EXTRUSION_3D_COLOR) != SfxItemState::UNKNOWN)
    {
        if (! bOnlyExtrudedCustomShapes)
            rSet.DisableItem( SID_EXTRUSION_3D_COLOR );
    }
    if (rSet.GetItemState(SID_EXTRUSION_DEPTH_FLOATER) != SfxItemState::UNKNOWN)
    {
        if (! bOnlyExtrudedCustomShapes)
            rSet.DisableItem( SID_EXTRUSION_DEPTH_FLOATER );
    }
    if (rSet.GetItemState(SID_EXTRUSION_DIRECTION_FLOATER) != SfxItemState::UNKNOWN)
    {
        if (! bOnlyExtrudedCustomShapes)
            rSet.DisableItem( SID_EXTRUSION_DIRECTION_FLOATER );
    }
    if (rSet.GetItemState(SID_EXTRUSION_LIGHTING_FLOATER) != SfxItemState::UNKNOWN)
    {
        if (! bOnlyExtrudedCustomShapes)
            rSet.DisableItem( SID_EXTRUSION_LIGHTING_FLOATER );
    }
    if (rSet.GetItemState(SID_EXTRUSION_SURFACE_FLOATER) != SfxItemState::UNKNOWN)
    {
        if(! bOnlyExtrudedCustomShapes)
            rSet.DisableItem( SID_EXTRUSION_SURFACE_FLOATER );
    }
    if (rSet.GetItemState(SID_EXTRUSION_TOGGLE) != SfxItemState::UNKNOWN)
    {
        if( !checkForSelectedCustomShapes( pSdrView, false ) )
            rSet.DisableItem( SID_EXTRUSION_TOGGLE );
    }
    if (rSet.GetItemState(SID_EXTRUSION_DEPTH) != SfxItemState::UNKNOWN)
    {
        getExtrusionDepthState( pSdrView, rSet );
    }
    if (rSet.GetItemState(SID_EXTRUSION_SURFACE) != SfxItemState::UNKNOWN)
    {
        getExtrusionSurfaceState( pSdrView, rSet );
    }
    if (rSet.GetItemState(SID_EXTRUSION_LIGHTING_INTENSITY) != SfxItemState::UNKNOWN)
    {
        getExtrusionLightingIntensityState( pSdrView, rSet );
    }

    if (rSet.GetItemState(SID_EXTRUSION_LIGHTING_DIRECTION) != SfxItemState::UNKNOWN)
    {
        getExtrusionLightingDirectionState( pSdrView, rSet );
    }

    if (rSet.GetItemState(SID_EXTRUSION_3D_COLOR) != SfxItemState::UNKNOWN)
    {
        getExtrusionColorState( pSdrView, rSet );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
