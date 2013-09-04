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
#include <svx/svdundo.hxx>
#include <sfx2/app.hxx>
#include <sfx2/request.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/bindings.hxx>
#include <svx/xsflclit.hxx>
#include <svx/dialmgr.hxx>
#include <svx/svdoashp.hxx>
#include <svx/dialogs.hrc>
#include <svx/svdview.hxx>
#include <editeng/colritem.hxx>
#include "svx/chrtitem.hxx"

#include <svx/extrusionbar.hxx>
#include "extrusiondepthdialog.hxx"


using namespace ::svx;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::uno;

/*************************************************************************
|*
|* Standardinterface deklarieren (Die Slotmap darf nicht leer sein, also
|* tragen wir etwas ein, was hier (hoffentlich) nie vorkommt).
|*
\************************************************************************/

SFX_SLOTMAP(ExtrusionBar)
{
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

SFX_IMPL_INTERFACE(ExtrusionBar, SfxShell, SVX_RES(RID_SVX_EXTRUSION_BAR))
{
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_OBJECT, SVX_RES(RID_SVX_EXTRUSION_BAR) );
}

TYPEINIT1( ExtrusionBar, SfxShell );


/*************************************************************************
|*
|* Standard-Konstruktor
|*
\************************************************************************/

ExtrusionBar::ExtrusionBar(SfxViewShell* pViewShell )
: SfxShell(pViewShell)
{
    DBG_ASSERT( pViewShell, "svx::ExtrusionBar::ExtrusionBar(), I need a viewshell!" );
    if( pViewShell )
        SetPool(&pViewShell->GetPool());

    SetHelpId( SVX_INTERFACE_EXTRUSION_BAR );
    SetName(SVX_RESSTR(RID_SVX_EXTRUSION_BAR));
}


/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

ExtrusionBar::~ExtrusionBar()
{
    SetRepeatTarget(NULL);
}

void getLightingDirectionDefaults( const Direction3D **pLighting1Defaults, const Direction3D **pLighting2Defaults )
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

    *pLighting1Defaults = (const Direction3D *)aLighting1Defaults;
    *pLighting2Defaults = (const Direction3D *)aLighting2Defaults;
};

static void impl_execute( SdrView*, SfxRequest& rReq, SdrCustomShapeGeometryItem& rGeometryItem, SdrObject* pObj )
{
    static const OUString  sExtrusion( "Extrusion" );
    static const OUString  sProjectionMode( "ProjectionMode" );
    static const OUString  sRotateAngle( "RotateAngle" );
    static const OUString  sViewPoint( "ViewPoint" );
    static const OUString  sOrigin( "Origin" );
    static const OUString  sSkew( "Skew" );
    static const OUString  sDepth( "Depth" );

    sal_uInt16 nSID = rReq.GetSlot();
    switch( nSID )
    {
    case SID_EXTRUSION_TOOGLE:
    {
        com::sun::star::uno::Any* pAny = rGeometryItem.GetPropertyValueByName( sExtrusion, sExtrusion );

        if( pAny )
        {
            sal_Bool bOn;
            (*pAny) >>= bOn;
            bOn = !bOn;
            (*pAny) <<= bOn;
        }
        else
        {
            com::sun::star::beans::PropertyValue aPropValue;
            aPropValue.Name = sExtrusion;
            aPropValue.Value <<= sal_True;
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
        com::sun::star::uno::Any* pAny = rGeometryItem.GetPropertyValueByName( sExtrusion, sRotateAngle );
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
        com::sun::star::beans::PropertyValue aPropValue;
        aPropValue.Name = sRotateAngle;
        aPropValue.Value <<= aRotateAnglePropPair;
        rGeometryItem.SetPropertyValue( sExtrusion, aPropValue );
    }
    break;

    case SID_EXTRUSION_DIRECTION:
    {
        if( rReq.GetArgs() && rReq.GetArgs()->GetItemState( SID_EXTRUSION_DIRECTION ) == SFX_ITEM_SET )
        {
            sal_Int32 nSkew = ((const SfxInt32Item*)rReq.GetArgs()->GetItem(SID_EXTRUSION_DIRECTION))->GetValue();

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

            com::sun::star::beans::PropertyValue aPropValue;

            aPropValue.Name = sViewPoint;
            aPropValue.Value <<= aViewPoint;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );


            EnhancedCustomShapeParameterPair aOriginPropPair;
            aOriginPropPair.First.Value <<= fOriginX;
            aOriginPropPair.First.Type = EnhancedCustomShapeParameterType::NORMAL;
            aOriginPropPair.Second.Value <<= fOriginY;
            aOriginPropPair.Second.Type = EnhancedCustomShapeParameterType::NORMAL;
            aPropValue.Name = sOrigin;
            aPropValue.Value <<= aOriginPropPair;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );

            EnhancedCustomShapeParameterPair aSkewPropPair;
            aSkewPropPair.First.Value <<= fSkew;
            aSkewPropPair.First.Type = EnhancedCustomShapeParameterType::NORMAL;
            aSkewPropPair.Second.Value <<= fSkewAngle;
            aSkewPropPair.Second.Type = EnhancedCustomShapeParameterType::NORMAL;
            aPropValue.Name = sSkew;
            aPropValue.Value <<= aSkewPropPair;
            rGeometryItem.SetPropertyValue( sExtrusion, aPropValue );
        }
    }
    break;
    case SID_EXTRUSION_PROJECTION:
    {
        if( rReq.GetArgs() && rReq.GetArgs()->GetItemState( SID_EXTRUSION_PROJECTION ) == SFX_ITEM_SET )
        {
            sal_Int32 nProjection = ((const SfxInt32Item*)rReq.GetArgs()->GetItem(SID_EXTRUSION_PROJECTION))->GetValue();
            ProjectionMode eProjectionMode = nProjection == 1 ? ProjectionMode_PARALLEL : ProjectionMode_PERSPECTIVE;
            com::sun::star::beans::PropertyValue aPropValue;
            aPropValue.Name = sProjectionMode;
            aPropValue.Value <<= eProjectionMode;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );
        }
    }
    break;
    case SID_EXTRUSION_DEPTH:
    {
        if( rReq.GetArgs() && rReq.GetArgs()->GetItemState( SID_EXTRUSION_DEPTH ) == SFX_ITEM_SET)
        {
            double fDepth = ((const SvxDoubleItem*)rReq.GetArgs()->GetItem(SID_EXTRUSION_DEPTH))->GetValue();
            double fFraction = 0.0;
            EnhancedCustomShapeParameterPair aDepthPropPair;
            aDepthPropPair.First.Value <<= fDepth;
            aDepthPropPair.First.Type = EnhancedCustomShapeParameterType::NORMAL;
            aDepthPropPair.Second.Value <<= fFraction;
            aDepthPropPair.Second.Type = EnhancedCustomShapeParameterType::NORMAL;

            com::sun::star::beans::PropertyValue aPropValue;
            aPropValue.Name = sDepth;
            aPropValue.Value <<= aDepthPropPair;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );
        }
    }
    break;
    case SID_EXTRUSION_3D_COLOR:
    {
        static const OUString  sExtrusionColor( "Color" );

        if( rReq.GetArgs() && rReq.GetArgs()->GetItemState( SID_EXTRUSION_3D_COLOR ) == SFX_ITEM_SET)
        {
            Color aColor( ((const SvxColorItem&)rReq.GetArgs()->Get(SID_EXTRUSION_3D_COLOR)).GetValue() );

            const bool bAuto = aColor == COL_AUTO;

            com::sun::star::beans::PropertyValue aPropValue;
            aPropValue.Name = sExtrusionColor;
            aPropValue.Value <<= bAuto ? sal_False : sal_True;
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
        static const OUString sShadeMode( "ShadeMode" );
        static const OUString sSpecularity( "Specularity" );
        static const OUString sDiffusion( "Diffusion" );
        static const OUString sMetal( "Metal" );

        if( rReq.GetArgs() && rReq.GetArgs()->GetItemState( SID_EXTRUSION_SURFACE ) == SFX_ITEM_SET)
        {
            sal_Int32 nSurface = ((const SfxInt32Item*)rReq.GetArgs()->GetItem(SID_EXTRUSION_SURFACE))->GetValue();

            ShadeMode eShadeMode( ShadeMode_FLAT );
            sal_Bool bMetal = sal_False;
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

            com::sun::star::beans::PropertyValue aPropValue;
            aPropValue.Name = sShadeMode;
            aPropValue.Value <<= eShadeMode;
            rGeometryItem.SetPropertyValue( sExtrusion, aPropValue );

            aPropValue.Name = sMetal;
            aPropValue.Value <<= bMetal;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );

            aPropValue.Name = sSpecularity;
            aPropValue.Value <<= fSpecularity;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );

            aPropValue.Name = sDiffusion;
            aPropValue.Value <<= fDiffusion;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );
        }
    }
    break;
    case SID_EXTRUSION_LIGHTING_INTENSITY:
    {
        static const OUString sBrightness( "Brightness" );
        static const OUString sLightFace( "LightFace" );
        static const OUString sFirstLightHarsh( "FirstLightHarsh" );
        static const OUString sSecondLightHarsh( "SecondLightHarsh" );
        static const OUString sFirstLightLevel( "FirstLightLevel" );
        static const OUString sSecondLightLevel( "SecondLightLevel" );

        if( rReq.GetArgs() && rReq.GetArgs()->GetItemState( SID_EXTRUSION_LIGHTING_INTENSITY ) == SFX_ITEM_SET)
        {
            sal_Int32 nLevel = ((const SfxInt32Item*)rReq.GetArgs()->GetItem(SID_EXTRUSION_LIGHTING_INTENSITY))->GetValue();

            double fBrightness;
            sal_Bool bHarsh2;
            double fLevel1;
            double fLevel2;

            switch( nLevel )
            {
            case 0: // bright
                fBrightness = 34.0;
                bHarsh2 = sal_False;
                fLevel1 = 66.0;
                fLevel2 = 66.0;
                break;
            case 1: // normal
                fBrightness = 15.0;
                bHarsh2 = sal_False;
                fLevel1 = 67.0;
                fLevel2 = 37.0;
                break;
            case 2: // dim
                fBrightness = 6.0;
                bHarsh2 = sal_True;
                fLevel1 = 79.0;
                fLevel2 = 21.0;
                break;
            }

            com::sun::star::beans::PropertyValue aPropValue;
            aPropValue.Name = sBrightness;
            aPropValue.Value <<= fBrightness;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );

            aPropValue.Name = sLightFace;
            aPropValue.Value <<= sal_True;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );

            aPropValue.Name = sFirstLightHarsh;
            aPropValue.Value <<= sal_True;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );

            aPropValue.Name = sSecondLightHarsh;
            aPropValue.Value <<= bHarsh2;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );

            aPropValue.Name = sFirstLightLevel;
            aPropValue.Value <<= fLevel1;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );

            aPropValue.Name = sSecondLightLevel;
            aPropValue.Value <<= fLevel2;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );
        }
    }
    break;
    case SID_EXTRUSION_LIGHTING_DIRECTION:
    {
        if( rReq.GetArgs() && rReq.GetArgs()->GetItemState( SID_EXTRUSION_LIGHTING_DIRECTION ) == SFX_ITEM_SET)
        {
            sal_Int32 nDirection = ((const SfxInt32Item*)rReq.GetArgs()->GetItem(SID_EXTRUSION_LIGHTING_DIRECTION))->GetValue();

            if((nDirection >= 0) && (nDirection < 9))
            {
                const OUString sFirstLightDirection( "FirstLightDirection" );
                const OUString sSecondLightDirection( "SecondLightDirection" );

                const Direction3D * pLighting1Defaults;
                const Direction3D * pLighting2Defaults;

                getLightingDirectionDefaults( &pLighting1Defaults, &pLighting2Defaults );

                com::sun::star::beans::PropertyValue aPropValue;
                aPropValue.Name = sFirstLightDirection;
                aPropValue.Value <<= pLighting1Defaults[nDirection];
                rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );

                aPropValue.Name = sSecondLightDirection;
                aPropValue.Value <<= pLighting2Defaults[nDirection];
                rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );
            }
        }
    }
    break;

    }
}

void ExtrusionBar::execute( SdrView* pSdrView, SfxRequest& rReq, SfxBindings& rBindings )
{
    sal_uInt16 nSID = rReq.GetSlot();
    sal_uInt16 nStrResId = 0;

    const bool bUndo = pSdrView && pSdrView->IsUndoEnabled();

    switch( nSID )
    {
        case SID_EXTRUSION_TOOGLE:
        {
            if ( !nStrResId )
                nStrResId = RID_SVXSTR_UNDO_APPLY_EXTRUSION_ON_OFF;
        }   // PASSTROUGH
        case SID_EXTRUSION_TILT_DOWN:
        {
            if ( !nStrResId )
                nStrResId = RID_SVXSTR_UNDO_APPLY_EXTRUSION_ROTATE_DOWN;
        }   // PASSTROUGH
        case SID_EXTRUSION_TILT_UP:
        {
            if ( !nStrResId )
                nStrResId = RID_SVXSTR_UNDO_APPLY_EXTRUSION_ROTATE_UP;
        }   // PASSTROUGH
        case SID_EXTRUSION_TILT_LEFT:
        {
            if ( !nStrResId )
                nStrResId = RID_SVXSTR_UNDO_APPLY_EXTRUSION_ROTATE_LEFT;
        }   // PASSTROUGH
        case SID_EXTRUSION_TILT_RIGHT:
        {
            if ( !nStrResId )
                nStrResId = RID_SVXSTR_UNDO_APPLY_EXTRUSION_ROTATE_RIGHT;
        }   // PASSTROUGH
        case SID_EXTRUSION_DIRECTION:
        {
            if ( !nStrResId )
                nStrResId = RID_SVXSTR_UNDO_APPLY_EXTRUSION_ORIENTATION;
        }   // PASSTROUGH
        case SID_EXTRUSION_PROJECTION:
        {
            if ( !nStrResId )
                nStrResId = RID_SVXSTR_UNDO_APPLY_EXTRUSION_PROJECTION;
        }   // PASSTROUGH
        case SID_EXTRUSION_DEPTH:
        {
            if ( !nStrResId )
                nStrResId = RID_SVXSTR_UNDO_APPLY_EXTRUSION_DEPTH;
        }   // PASSTROUGH
        case SID_EXTRUSION_3D_COLOR:
        {
            if ( !nStrResId )
                nStrResId = RID_SVXSTR_UNDO_APPLY_EXTRUSION_COLOR;
        }   // PASSTROUGH
        case SID_EXTRUSION_SURFACE:
        {
            if ( !nStrResId )
                nStrResId = RID_SVXSTR_UNDO_APPLY_EXTRUSION_SURFACE;
        }   // PASSTROUGH
        case SID_EXTRUSION_LIGHTING_INTENSITY:
        {
            if ( !nStrResId )
                nStrResId = RID_SVXSTR_UNDO_APPLY_EXTRUSION_BRIGHTNESS;
        }   // PASSTROUGH
        case SID_EXTRUSION_LIGHTING_DIRECTION:
        {
            if ( !nStrResId )
                nStrResId = RID_SVXSTR_UNDO_APPLY_EXTRUSION_LIGHTING;

            const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
            sal_uIntPtr nCount = rMarkList.GetMarkCount(), i;

            for(i=0; i<nCount; i++)
            {
                SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
                if( pObj->ISA(SdrObjCustomShape) )
                {
                    if( bUndo )
                    {
                        OUString aStr( SVX_RESSTR( nStrResId ) );
                        pSdrView->BegUndo( aStr );
                        pSdrView->AddUndo( pSdrView->GetModel()->GetSdrUndoFactory().CreateUndoAttrObject( *pObj ) );
                    }
                    SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
                    impl_execute( pSdrView, rReq, aGeometryItem, pObj );
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
        break;

        case SID_EXTRUSION_DEPTH_DIALOG:
            if( rReq.GetArgs() &&
                (rReq.GetArgs()->GetItemState( SID_EXTRUSION_DEPTH ) == SFX_ITEM_SET) &&
                (rReq.GetArgs()->GetItemState( SID_ATTR_METRIC ) == SFX_ITEM_SET))
            {
                double fDepth = ((const SvxDoubleItem*)rReq.GetArgs()->GetItem(SID_EXTRUSION_DEPTH))->GetValue();
                FieldUnit eUnit = (FieldUnit)((const SfxUInt16Item*)rReq.GetArgs()->GetItem(SID_ATTR_METRIC))->GetValue();

                ExtrusionDepthDialog aDlg( 0L, fDepth, eUnit );
                sal_uInt16 nRet = aDlg.Execute();
                if( nRet != 0 )
                {
                    fDepth = aDlg.getDepth();

                    SvxDoubleItem aItem( fDepth, SID_EXTRUSION_DEPTH );
                    SfxPoolItem* aItems[] = { &aItem, 0 };
                    rBindings.Execute( SID_EXTRUSION_DEPTH, (const SfxPoolItem**)aItems );
                }
            }
            break;
    }

    if( nSID == SID_EXTRUSION_TOOGLE )
    {
            static sal_uInt16 SidArray[] = {
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

void getExtrusionDirectionState( SdrView* pSdrView, SfxItemSet& rSet )
{
    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    sal_uIntPtr nCount = rMarkList.GetMarkCount(), i;

    static const OUString  sExtrusion( "Extrusion" );
    static const OUString  sViewPoint( "ViewPoint" );
    static const OUString  sOrigin( "Origin" );
    static const OUString  sSkew( "Skew" );
    static const OUString  sProjectionMode( "ProjectionMode" );

    com::sun::star::uno::Any* pAny;

    double fFinalSkewAngle = -1;
    bool bHasCustomShape = false;

    for(i=0;i<nCount; i++)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
        if( pObj->ISA(SdrObjCustomShape) )
        {
            SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );

            // see if this is an extruded customshape
            if( !bHasCustomShape )
            {
                Any* pAny_ = aGeometryItem.GetPropertyValueByName( sExtrusion, sExtrusion );
                if( pAny_ )
                    *pAny_ >>= bHasCustomShape;

                if( !bHasCustomShape )
                    continue;
            }

            bool        bParallel = true;
            Position3D  aViewPoint( 3472, -3472, 25000 );
            double      fSkewAngle = -135;

            pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sProjectionMode );
            sal_Int16 nProjectionMode = sal_Int16();
            if( pAny && ( *pAny >>= nProjectionMode ) )
                bParallel = nProjectionMode == ProjectionMode_PARALLEL;

            if( bParallel )
            {
                double      fSkew = 50.0;
                EnhancedCustomShapeParameterPair aSkewPropPair;
                pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sSkew );
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
                pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sViewPoint );
                if( pAny )
                    *pAny >>= aViewPoint;

                EnhancedCustomShapeParameterPair aOriginPropPair;
                pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sOrigin );
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

            if( fFinalSkewAngle == -1.0 )
            {
                fFinalSkewAngle = fSkewAngle;
            }
            else if( fSkewAngle != fFinalSkewAngle )
            {
                fFinalSkewAngle = -1.0;
            }

            if( fFinalSkewAngle == -1.0 )
                break;
        }
    }

    if( bHasCustomShape )
        rSet.Put( SfxInt32Item( SID_EXTRUSION_DIRECTION, (sal_Int32)fFinalSkewAngle ) );
    else
        rSet.DisableItem( SID_EXTRUSION_DIRECTION );
}

void getExtrusionProjectionState( SdrView* pSdrView, SfxItemSet& rSet )
{
    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    sal_uIntPtr nCount = rMarkList.GetMarkCount(), i;

    static const OUString  sExtrusion( "Extrusion" );
    static const OUString  sProjectionMode( "ProjectionMode" );

    com::sun::star::uno::Any* pAny;

    sal_Int32 nFinalProjection = -1;
    bool bHasCustomShape = false;

    for(i=0;i<nCount; i++)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
        if( pObj->ISA(SdrObjCustomShape) )
        {
            // see if this is an extruded customshape
            if( !bHasCustomShape )
            {
                SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
                Any* pAny_ = aGeometryItem.GetPropertyValueByName( sExtrusion, sExtrusion );
                if( pAny_ )
                    *pAny_ >>= bHasCustomShape;

                if( !bHasCustomShape )
                    continue;
            }

            SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );

            sal_Bool    bParallel = sal_True;
            pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sProjectionMode );
            ProjectionMode eProjectionMode;
            if( pAny && ( *pAny >>= eProjectionMode ) )
                bParallel = eProjectionMode == ProjectionMode_PARALLEL;

            if( nFinalProjection == -1 )
            {
                nFinalProjection = bParallel;
            }
            else if( nFinalProjection != bParallel )
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

void getExtrusionSurfaceState( SdrView* pSdrView, SfxItemSet& rSet )
{
    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    sal_uIntPtr nCount = rMarkList.GetMarkCount(), i;

    static const OUString  sExtrusion( "Extrusion" );
    static const OUString  sShadeMode( "ShadeMode" );
    static const OUString  sSpecularity( "Specularity" );
    static const OUString  sMetal( "Metal" );

    com::sun::star::uno::Any* pAny;

    sal_Int32 nFinalSurface = -1;
    bool bHasCustomShape = false;

    for(i=0;i<nCount; i++)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
        if( pObj->ISA(SdrObjCustomShape) )
        {
            SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );

            // see if this is an extruded customshape
            if( !bHasCustomShape )
            {
                Any* pAny_ = aGeometryItem.GetPropertyValueByName( sExtrusion, sExtrusion );
                if( pAny_ )
                    *pAny_ >>= bHasCustomShape;

                if( !bHasCustomShape )
                    continue;
            }

            sal_Int32 nSurface = 0; // wire frame

            ShadeMode eShadeMode( ShadeMode_FLAT );
            pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sShadeMode );
            if( pAny )
                *pAny >>= eShadeMode;

            if( eShadeMode == ShadeMode_FLAT )
            {
                sal_Bool bMetal = sal_False;
                pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sMetal );
                if( pAny )
                    *pAny >>= bMetal;

                if( bMetal )
                {
                    nSurface = 3; // metal
                }
                else
                {
                    double fSpecularity = 0;
                    pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sSpecularity );
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

void getExtrusionDepthState( SdrView* pSdrView, SfxItemSet& rSet )
{
    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    sal_uIntPtr nCount = rMarkList.GetMarkCount(), i;

    static const OUString  sExtrusion( "Extrusion" );
    static const OUString  sDepth( "Depth" );

    com::sun::star::uno::Any* pAny;

    double fFinalDepth = -1;
    bool bHasCustomShape = false;

    for(i=0;i<nCount; i++)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
        if( pObj->ISA(SdrObjCustomShape) )
        {
            SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );

            // see if this is an extruded customshape
            if( !bHasCustomShape )
            {
                Any* pAny_ = aGeometryItem.GetPropertyValueByName( sExtrusion, sExtrusion );
                if( pAny_ )
                    *pAny_ >>= bHasCustomShape;

                if( !bHasCustomShape )
                    continue;
            }

            double fDepth = 1270.0;
            pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sDepth );
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
            else if( fFinalDepth != fDepth )
            {
                fFinalDepth = -1;
                break;
            }
        }
    }

    if( pSdrView->GetModel() )
    {
        FieldUnit eUnit = pSdrView->GetModel()->GetUIUnit();
        rSet.Put( SfxUInt16Item( SID_ATTR_METRIC, (sal_uInt16)eUnit ) );
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

void getExtrusionLightingDirectionState( SdrView* pSdrView, SfxItemSet& rSet )
{
    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    sal_uIntPtr nCount = rMarkList.GetMarkCount(), i;

    static const OUString  sExtrusion( "Extrusion" );
    static const OUString  sFirstLightDirection( "FirstLightDirection" );
    static const OUString  sSecondLightDirection( "SecondLightDirection" );

    const Direction3D * pLighting1Defaults;
    const Direction3D * pLighting2Defaults;

    getLightingDirectionDefaults( &pLighting1Defaults, &pLighting2Defaults );

    com::sun::star::uno::Any* pAny;

    int nFinalDirection = -1;
    bool bHasCustomShape = false;

    for(i=0;i<nCount; i++)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
        if( pObj->ISA(SdrObjCustomShape) )
        {
            SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );

            // see if this is an extruded customshape
            if( !bHasCustomShape )
            {
                Any* pAny_ = aGeometryItem.GetPropertyValueByName( sExtrusion, sExtrusion );
                if( pAny_ )
                    *pAny_ >>= bHasCustomShape;

                if( !bHasCustomShape )
                    continue;
            }

            Direction3D aFirstLightDirection( 50000, 0, 10000 );
            Direction3D aSecondLightDirection( -50000, 0, 10000 );

            pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sFirstLightDirection );
            if( pAny )
                *pAny >>= aFirstLightDirection;

            pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sSecondLightDirection );
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
        rSet.Put( SfxInt32Item( SID_EXTRUSION_LIGHTING_DIRECTION, (sal_Int32)nFinalDirection ) );
    else
        rSet.DisableItem( SID_EXTRUSION_LIGHTING_DIRECTION );
}

void getExtrusionLightingIntensityState( SdrView* pSdrView, SfxItemSet& rSet )
{
    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    sal_uIntPtr nCount = rMarkList.GetMarkCount(), i;

    static const OUString  sExtrusion( "Extrusion" );
    static const OUString  sBrightness( "Brightness" );

    com::sun::star::uno::Any* pAny;

    int nFinalLevel = -1;
    bool bHasCustomShape = false;

    for(i=0;i<nCount; i++)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
        if( pObj->ISA(SdrObjCustomShape) )
        {
            SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );

            // see if this is an extruded customshape
            if( !bHasCustomShape )
            {
                Any* pAny_ = aGeometryItem.GetPropertyValueByName( sExtrusion, sExtrusion );
                if( pAny_ )
                    *pAny_ >>= bHasCustomShape;

                if( !bHasCustomShape )
                    continue;
            }

            double fBrightness = 22178.0 / 655.36;
            pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sBrightness );
            if( pAny )
                *pAny >>= fBrightness;

            int nLevel;
            if( fBrightness >= 30.0 )
            {
                nLevel = 0; // Bright
            }
            else if( fBrightness >= 10.0 )
            {
                nLevel = 1; // Noraml;
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

void getExtrusionColorState( SdrView* pSdrView, SfxItemSet& rSet )
{
    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    sal_uIntPtr nCount = rMarkList.GetMarkCount(), i;

    static const OUString  sExtrusion( "Extrusion" );
    static const OUString  sExtrusionColor( "Color" );

    com::sun::star::uno::Any* pAny;

    bool bInit = false;
    bool bAmbigius = false;
    Color aFinalColor;
    bool bHasCustomShape = false;

    for(i=0;i<nCount; i++)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
        if( pObj->ISA(SdrObjCustomShape) )
        {
            SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );

            // see if this is an extruded customshape
            if( !bHasCustomShape )
            {
                Any* pAny_ = aGeometryItem.GetPropertyValueByName( sExtrusion, sExtrusion );
                if( pAny_ )
                    *pAny_ >>= bHasCustomShape;

                if( !bHasCustomShape )
                    continue;
            }

            Color aColor;

            bool bUseColor = false;
            pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sExtrusionColor );
            if( pAny )
                *pAny >>= bUseColor;

            if( bUseColor )
            {
                const XSecondaryFillColorItem& rItem = *(XSecondaryFillColorItem*)&(pObj->GetMergedItem( XATTR_SECONDARYFILLCOLOR ));
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
bool checkForSelectedCustomShapes( SdrView* pSdrView, bool bOnlyExtruded )
{
    static const OUString  sExtrusion( "Extrusion" );

    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    sal_uIntPtr nCount = rMarkList.GetMarkCount(), i;
    bool bFound = false;

    for(i=0;(i<nCount) && !bFound ; i++)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
        if( pObj->ISA(SdrObjCustomShape) )
        {
            if( bOnlyExtruded )
            {
                SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
                Any* pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sExtrusion );
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

void ExtrusionBar::getState( SdrView* pSdrView, SfxItemSet& rSet )
{
    if (rSet.GetItemState(SID_EXTRUSION_DIRECTION) != SFX_ITEM_UNKNOWN)
    {
        getExtrusionDirectionState( pSdrView, rSet );
    }
    if (rSet.GetItemState(SID_EXTRUSION_PROJECTION) != SFX_ITEM_UNKNOWN)
    {
        getExtrusionProjectionState( pSdrView, rSet );
    }
    const bool bOnlyExtrudedCustomShapes =
        checkForSelectedCustomShapes( pSdrView, true );
    if (rSet.GetItemState(SID_EXTRUSION_TILT_DOWN) != SFX_ITEM_UNKNOWN)
    {
        if (! bOnlyExtrudedCustomShapes)
            rSet.DisableItem( SID_EXTRUSION_TILT_DOWN );
    }
    if (rSet.GetItemState(SID_EXTRUSION_TILT_DOWN) != SFX_ITEM_UNKNOWN)
    {
        if (! bOnlyExtrudedCustomShapes)
            rSet.DisableItem( SID_EXTRUSION_TILT_DOWN );
    }
    if (rSet.GetItemState(SID_EXTRUSION_TILT_UP) != SFX_ITEM_UNKNOWN)
    {
        if (! bOnlyExtrudedCustomShapes)
            rSet.DisableItem( SID_EXTRUSION_TILT_UP );
    }
    if (rSet.GetItemState(SID_EXTRUSION_TILT_LEFT) != SFX_ITEM_UNKNOWN)
    {
        if (! bOnlyExtrudedCustomShapes)
            rSet.DisableItem( SID_EXTRUSION_TILT_LEFT );
    }
    if (rSet.GetItemState(SID_EXTRUSION_TILT_RIGHT) != SFX_ITEM_UNKNOWN)
    {
        if (! bOnlyExtrudedCustomShapes)
            rSet.DisableItem( SID_EXTRUSION_TILT_RIGHT );
    }
    if (rSet.GetItemState(SID_EXTRUSION_3D_COLOR) != SFX_ITEM_UNKNOWN)
    {
        if (! bOnlyExtrudedCustomShapes)
            rSet.DisableItem( SID_EXTRUSION_3D_COLOR );
    }
    if (rSet.GetItemState(SID_EXTRUSION_DEPTH_FLOATER) != SFX_ITEM_UNKNOWN)
    {
        if (! bOnlyExtrudedCustomShapes)
            rSet.DisableItem( SID_EXTRUSION_DEPTH_FLOATER );
    }
    if (rSet.GetItemState(SID_EXTRUSION_DIRECTION_FLOATER) != SFX_ITEM_UNKNOWN)
    {
        if (! bOnlyExtrudedCustomShapes)
            rSet.DisableItem( SID_EXTRUSION_DIRECTION_FLOATER );
    }
    if (rSet.GetItemState(SID_EXTRUSION_LIGHTING_FLOATER) != SFX_ITEM_UNKNOWN)
    {
        if (! bOnlyExtrudedCustomShapes)
            rSet.DisableItem( SID_EXTRUSION_LIGHTING_FLOATER );
    }
    if (rSet.GetItemState(SID_EXTRUSION_SURFACE_FLOATER) != SFX_ITEM_UNKNOWN)
    {
        if(! bOnlyExtrudedCustomShapes)
            rSet.DisableItem( SID_EXTRUSION_SURFACE_FLOATER );
    }
    if (rSet.GetItemState(SID_EXTRUSION_TOOGLE) != SFX_ITEM_UNKNOWN)
    {
        if( !checkForSelectedCustomShapes( pSdrView, false ) )
            rSet.DisableItem( SID_EXTRUSION_TOOGLE );
    }
    if (rSet.GetItemState(SID_EXTRUSION_DEPTH) != SFX_ITEM_UNKNOWN)
    {
        getExtrusionDepthState( pSdrView, rSet );
    }
    if (rSet.GetItemState(SID_EXTRUSION_SURFACE) != SFX_ITEM_UNKNOWN)
    {
        getExtrusionSurfaceState( pSdrView, rSet );
    }
    if (rSet.GetItemState(SID_EXTRUSION_LIGHTING_INTENSITY) != SFX_ITEM_UNKNOWN)
    {
        getExtrusionLightingIntensityState( pSdrView, rSet );
    }

    if (rSet.GetItemState(SID_EXTRUSION_LIGHTING_DIRECTION) != SFX_ITEM_UNKNOWN)
    {
        getExtrusionLightingDirectionState( pSdrView, rSet );
    }

    if (rSet.GetItemState(SID_EXTRUSION_3D_COLOR) != SFX_ITEM_UNKNOWN)
    {
        getExtrusionColorState( pSdrView, rSet );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
