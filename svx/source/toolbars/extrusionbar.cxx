/*************************************************************************
 *
 *  $RCSfile: extrusionbar.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 14:51:19 $
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

#ifndef _DRAFTS_COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPEEXTRUSIONRENDERMODE_HPP_
#include <drafts/com/sun/star/drawing/EnhancedCustomShapeExtrusionRenderMode.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POSITION3D_HPP_
#include <com/sun/star/drawing/Position3D.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_DIRECTION3D_HPP_
#include <com/sun/star/drawing/Direction3D.hpp>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFXREQUEST_HXX
#include <sfx2/request.hxx>
#endif
#ifndef _SFXOBJFACE_HXX
#include <sfx2/objface.hxx>
#endif
#ifndef _SFXVIEWSH_HXX
#include <sfx2/viewsh.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX
#include <sfx2/bindings.hxx>
#endif
#ifndef _SVX_XSFLCLIT_HXX
#include "xsflclit.hxx"
#endif
#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
#endif
#ifndef _SVDOASHP_HXX
#include "svdoashp.hxx"
#endif
#ifndef _SVX_DIALOGS_HRC
#include "dialogs.hrc"
#endif
#ifndef _SVDVIEW_HXX
#include "svdview.hxx"
#endif
#define ITEMID_COLOR 0
#ifndef _SVX_COLRITEM_HXX
#include "colritem.hxx"
#endif
#define ITEMID_DOUBLE 0
#include "chrtitem.hxx"

#include "extrusionbar.hxx"
#include "extrusioncontrols.hxx"

using namespace ::svx;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::drawing;

/*************************************************************************
|*
|* Standardinterface deklarieren (Die Slotmap darf nicht leer sein, also
|* tragen wir etwas ein, was hier (hoffentlich) nie vorkommt).
|*
\************************************************************************/

#define ShellClass ExtrusionBar

SFX_SLOTMAP(ExtrusionBar)
{
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
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
//  SdDrawDocShell* pDocShell = pSdViewShell->GetDocSh();
//  SetPool(&pDocShell->GetPool());
//  SetUndoManager(pDocShell->GetUndoManager());
//  SetRepeatTarget(pViewShell);
    SetHelpId( SVX_INTERFACE_EXTRUSION_BAR );
    SetName( String( SVX_RES( RID_SVX_EXTRUSION_BAR )));
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

static void impl_execute( SdrView* pSdrView, SfxRequest& rReq, SdrCustomShapeGeometryItem& rGeometryItem, SdrObject* pObj )
{
    static const rtl::OUString  sExtrusion( RTL_CONSTASCII_USTRINGPARAM ( "Extrusion" ) );
    static const rtl::OUString  sParallel( RTL_CONSTASCII_USTRINGPARAM ( "Parallel" ) );
    static const rtl::OUString  sOn( RTL_CONSTASCII_USTRINGPARAM( "On" ) );
    static const rtl::OUString  sAngleX( RTL_CONSTASCII_USTRINGPARAM ( "AngleX" ) );
    static const rtl::OUString  sAngleY( RTL_CONSTASCII_USTRINGPARAM ( "AngleY" ) );
    static const rtl::OUString  sViewPoint( RTL_CONSTASCII_USTRINGPARAM ( "ViewPoint" ) );
    static const rtl::OUString  sOriginX( RTL_CONSTASCII_USTRINGPARAM ( "OriginX" ) );
    static const rtl::OUString  sOriginY( RTL_CONSTASCII_USTRINGPARAM ( "OriginY" ) );
    static const rtl::OUString  sSkew( RTL_CONSTASCII_USTRINGPARAM ( "Skew" ) );
    static const rtl::OUString  sSkewAngle( RTL_CONSTASCII_USTRINGPARAM ( "SkewAngle" ) );
    static const rtl::OUString  sBackwardDepth( RTL_CONSTASCII_USTRINGPARAM ( "BackwardDepth" ) );

    sal_uInt16 nSID = rReq.GetSlot();
    switch( nSID )
    {
    case SID_EXTRUSION_TOOGLE:
    {
        com::sun::star::uno::Any* pAny = rGeometryItem.GetPropertyValueByName( sExtrusion, sOn );

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
            aPropValue.Name = sOn;
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
        sal_Bool bHorizontal = nSID == SID_EXTRUSION_TILT_DOWN || nSID == SID_EXTRUSION_TILT_UP;
        sal_Int32 nDiff = nSID == SID_EXTRUSION_TILT_RIGHT || nSID == SID_EXTRUSION_TILT_UP ? 15 : -15;
        com::sun::star::uno::Any* pAny = rGeometryItem.GetPropertyValueByName( sExtrusion, bHorizontal ? sAngleX : sAngleY );

        if( pAny )
        {
            sal_Int32 nAngle;
            (*pAny) >>= nAngle;
            nAngle += nDiff;
            (*pAny) <<= nAngle;
        }
        else
        {
            com::sun::star::beans::PropertyValue aPropValue;
            aPropValue.Name = bHorizontal ? sAngleX : sAngleY;
            aPropValue.Value <<= nDiff;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );
        }
    }
    break;

    case SID_EXTRUSION_DIRECTION:
    {
        if( rReq.GetArgs() && rReq.GetArgs()->GetItemState( SID_EXTRUSION_DIRECTION ) == SFX_ITEM_SET )
        {
            sal_Int32 nSkew = ((const SfxInt32Item*)rReq.GetArgs()->GetItem(SID_EXTRUSION_DIRECTION))->GetValue();

            sal_Bool    bParallel = sal_True;
            Position3D  aViewPoint( 3472, -3472, 25000 );
            double      fOriginX = 50.0;
            double      fOriginY = -50.0;
            double      fSkewAngle = nSkew;
            double      fSkew = 50.0;

            switch( nSkew )
            {
            case 135:
                aViewPoint.PositionY = 3472;
                fOriginY = 50.0;
                break;
            case 90:
                aViewPoint.PositionX = 0;
                aViewPoint.PositionY = 3472;
                fOriginX = 0;
                fOriginY = -50.0;
                break;
            case 45:
                aViewPoint.PositionX = -3472;
                aViewPoint.PositionY = 3472;
                fOriginX = -50;
                fOriginY = 50;
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
                fOriginX = -50;
                fOriginY = 0;
                break;
            case -90:
                aViewPoint.PositionX = 0;
                fOriginX = 0;
                break;
            case -45:
                aViewPoint.PositionX = -3472;
                fOriginX = -50;
                break;
            }

            com::sun::star::beans::PropertyValue aPropValue;

            aPropValue.Name = sViewPoint;
            aPropValue.Value <<= aViewPoint;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );

            aPropValue.Name = sOriginX;
            aPropValue.Value <<= fOriginX;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );

            aPropValue.Name = sOriginY;
            aPropValue.Value <<= fOriginY;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );

            aPropValue.Name = sSkew;
            aPropValue.Value <<= fSkew;
            rGeometryItem.SetPropertyValue( sExtrusion, aPropValue );

            aPropValue.Name = sSkewAngle;
            aPropValue.Value <<= fSkewAngle;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );
        }
    }
    break;
    case SID_EXTRUSION_PROJECTION:
    {
        if( rReq.GetArgs() && rReq.GetArgs()->GetItemState( SID_EXTRUSION_PROJECTION ) == SFX_ITEM_SET )
        {
            sal_Int32 nProjection = ((const SfxInt32Item*)rReq.GetArgs()->GetItem(SID_EXTRUSION_PROJECTION))->GetValue();

            com::sun::star::beans::PropertyValue aPropValue;
            aPropValue.Name = sParallel;
            aPropValue.Value <<= (nProjection == 1);
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );
        }
    }
    break;
    case SID_EXTRUSION_DEPTH:
    {
        if( rReq.GetArgs() && rReq.GetArgs()->GetItemState( SID_EXTRUSION_DEPTH ) == SFX_ITEM_SET)
        {
            double fDepth = ((const SvxDoubleItem*)rReq.GetArgs()->GetItem(SID_EXTRUSION_DEPTH))->GetValue();

            com::sun::star::beans::PropertyValue aPropValue;
            aPropValue.Name = sBackwardDepth;
            aPropValue.Value <<= fDepth;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );
        }
    }
    break;
    case SID_EXTRUSION_3D_COLOR:
    {
        static const rtl::OUString  sExtrusionColor( RTL_CONSTASCII_USTRINGPARAM ( "Color" ) );

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
                pObj->SetMergedItem( XSecondaryFillColorItem( String(), aColor ) );
            }
        }
    }
    break;
    case SID_EXTRUSION_SURFACE:
    {
        static const rtl::OUString sRenderMode( RTL_CONSTASCII_USTRINGPARAM ( "RenderMode" ) );
        static const rtl::OUString sSpecularity( RTL_CONSTASCII_USTRINGPARAM ( "Specularity" ) );
        static const rtl::OUString sDiffusity( RTL_CONSTASCII_USTRINGPARAM ( "Diffusity" ) );
        static const rtl::OUString sMetal( RTL_CONSTASCII_USTRINGPARAM ( "Metal" ) );

        if( rReq.GetArgs() && rReq.GetArgs()->GetItemState( SID_EXTRUSION_SURFACE ) == SFX_ITEM_SET)
        {
            sal_Int32 nSurface = ((const SfxInt32Item*)rReq.GetArgs()->GetItem(SID_EXTRUSION_SURFACE))->GetValue();

            sal_Int16 nRenderMode = EnhancedCustomShapeExtrusionRenderMode::SOLID;
            sal_Bool bMetal = sal_False;
            double fSpecularity = 0;
            double fDiffusity = 0;

            switch( nSurface )
            {
            case 0: // wireframe
                nRenderMode = EnhancedCustomShapeExtrusionRenderMode::WIREFRAME;
                break;
            case 1: // matte
                break;
            case 2: // plastic
                fSpecularity = 122.0;
                break;
            case 3: // metal
                bMetal = true;
                fSpecularity = 122.0;
                fDiffusity = 122.0;
                break;
            }

            com::sun::star::beans::PropertyValue aPropValue;
            aPropValue.Name = sRenderMode;
            aPropValue.Value <<= nRenderMode;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );

            aPropValue.Name = sMetal;
            aPropValue.Value <<= bMetal;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );

            aPropValue.Name = sSpecularity;
            aPropValue.Value <<= fSpecularity;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );

            aPropValue.Name = sDiffusity;
            aPropValue.Value <<= fDiffusity;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );
        }
    }
    break;
    case SID_EXTRUSION_LIGHTING_INTENSITY:
    {
        static const rtl::OUString sBrightness( RTL_CONSTASCII_USTRINGPARAM ( "Brightness" ) );
        static const rtl::OUString sLightFace( RTL_CONSTASCII_USTRINGPARAM ( "LightFace" ) );
        static const rtl::OUString sLightHarsh1( RTL_CONSTASCII_USTRINGPARAM ( "LightHarsh1" ) );
        static const rtl::OUString sLightHarsh2( RTL_CONSTASCII_USTRINGPARAM ( "LightHarsh2" ) );
        static const rtl::OUString sLightLevel1( RTL_CONSTASCII_USTRINGPARAM ( "LightLevel1" ) );
        static const rtl::OUString sLightLevel2( RTL_CONSTASCII_USTRINGPARAM ( "LightLevel2" ) );

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

            aPropValue.Name = sLightHarsh1;
            aPropValue.Value <<= sal_True;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );

            aPropValue.Name = sLightHarsh2;
            aPropValue.Value <<= bHarsh2;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );

            aPropValue.Name = sLightLevel1;
            aPropValue.Value <<= fLevel1;
            rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );

            aPropValue.Name = sLightLevel2;
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
                const rtl::OUString sLightDirection1( RTL_CONSTASCII_USTRINGPARAM ( "LightDirection1" ) );
                const rtl::OUString sLightDirection2( RTL_CONSTASCII_USTRINGPARAM ( "LightDirection2" ) );

                const Direction3D * pLighting1Defaults;
                const Direction3D * pLighting2Defaults;

                getLightingDirectionDefaults( &pLighting1Defaults, &pLighting2Defaults );

                com::sun::star::beans::PropertyValue aPropValue;
                aPropValue.Name = sLightDirection1;
                aPropValue.Value <<= pLighting1Defaults[nDirection];
                rGeometryItem.SetPropertyValue( sExtrusion,  aPropValue );

                aPropValue.Name = sLightDirection2;
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
    switch( nSID )
    {
        case SID_EXTRUSION_TOOGLE:
        case SID_EXTRUSION_TILT_DOWN:
        case SID_EXTRUSION_TILT_UP:
        case SID_EXTRUSION_TILT_LEFT:
        case SID_EXTRUSION_TILT_RIGHT:
        case SID_EXTRUSION_DIRECTION:
        case SID_EXTRUSION_PROJECTION:
        case SID_EXTRUSION_DEPTH:
        case SID_EXTRUSION_3D_COLOR:
        case SID_EXTRUSION_SURFACE:
        case SID_EXTRUSION_LIGHTING_INTENSITY:
        case SID_EXTRUSION_LIGHTING_DIRECTION:
            {
                const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
                ULONG nCount = rMarkList.GetMarkCount(), i;

                for(i=0; i<nCount; i++)
                {
                    SdrObject* pObj = rMarkList.GetMark(i)->GetObj();
                    if( pObj->ISA(SdrObjCustomShape) )
                    {
                        SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
                        impl_execute( pSdrView, rReq, aGeometryItem, pObj );
                        pObj->SetMergedItem( aGeometryItem );
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
                USHORT nRet = aDlg.Execute();
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
            static USHORT SidArray[] = {
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
    ULONG nCount = rMarkList.GetMarkCount(), i;

    static const rtl::OUString  sExtrusion( RTL_CONSTASCII_USTRINGPARAM ( "Extrusion" ) );
    static const rtl::OUString  sViewPoint( RTL_CONSTASCII_USTRINGPARAM ( "ViewPoint" ) );
    static const rtl::OUString  sOriginX( RTL_CONSTASCII_USTRINGPARAM ( "OriginX" ) );
    static const rtl::OUString  sOriginY( RTL_CONSTASCII_USTRINGPARAM ( "OriginY" ) );
    static const rtl::OUString  sSkew( RTL_CONSTASCII_USTRINGPARAM ( "Skew" ) );
    static const rtl::OUString  sSkewAngle( RTL_CONSTASCII_USTRINGPARAM ( "SkewAngle" ) );
    static const rtl::OUString  sParallel( RTL_CONSTASCII_USTRINGPARAM ( "Parallel" ) );
    static const rtl::OUString  sOn( RTL_CONSTASCII_USTRINGPARAM( "On" ) );

    com::sun::star::uno::Any* pAny;

    double fFinalSkewAngle = -1;
    bool bHasCustomShape = false;

    for(i=0;i<nCount; i++)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetObj();
        if( pObj->ISA(SdrObjCustomShape) )
        {
            SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );

            // see if this is an extruded customshape
            if( !bHasCustomShape )
            {
                Any* pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sOn );
                if( pAny )
                    *pAny >>= bHasCustomShape;

                if( !bHasCustomShape )
                    continue;
            }

            sal_Bool    bParallel = sal_True;
            Position3D  aViewPoint( 3472, -3472, 25000 );
            double      fOriginX = 50.0;
            double      fOriginY = -50.0;
            double      fSkewAngle = -135;
            double      fSkew = 50.0;

            pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sParallel );
            if( pAny )
                *pAny >>= bParallel;

            if( bParallel )
            {
                pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sSkewAngle );
                if( pAny )
                    *pAny >>= fSkewAngle;
                pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sSkew );
                if ( pAny )
                    *pAny >>= fSkew;

                if ( fSkew == 0.0 )
                    fSkewAngle = 0.0;
                else if ( fSkewAngle == 0.0 )
                    fSkewAngle = -360.0;
            }
            else
            {
                pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sViewPoint );
                if( pAny )
                    *pAny >>= aViewPoint;

                pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sOriginX );
                if( pAny )
                    *pAny >>= fOriginX;

                pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sOriginY );
                if( pAny )
                    *pAny >>= fOriginY;

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
    ULONG nCount = rMarkList.GetMarkCount(), i;

    static const rtl::OUString  sExtrusion( RTL_CONSTASCII_USTRINGPARAM ( "Extrusion" ) );
    static const rtl::OUString  sParallel( RTL_CONSTASCII_USTRINGPARAM ( "Parallel" ) );
    static const rtl::OUString  sOn( RTL_CONSTASCII_USTRINGPARAM( "On" ) );

    com::sun::star::uno::Any* pAny;

    sal_Int32 nFinalProjection = -1;
    bool bHasCustomShape = false;

    for(i=0;i<nCount; i++)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetObj();
        if( pObj->ISA(SdrObjCustomShape) )
        {
            // see if this is an extruded customshape
            if( !bHasCustomShape )
            {
                SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
                Any* pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sOn );
                if( pAny )
                    *pAny >>= bHasCustomShape;

                if( !bHasCustomShape )
                    continue;
            }

            SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );

            sal_Bool    bParallel = sal_True;
            pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sParallel );
            if( pAny )
                *pAny >>= bParallel;

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
    ULONG nCount = rMarkList.GetMarkCount(), i;

    static const rtl::OUString  sExtrusion( RTL_CONSTASCII_USTRINGPARAM ( "Extrusion" ) );
    static const rtl::OUString  sRenderMode( RTL_CONSTASCII_USTRINGPARAM ( "RenderMode" ) );
    static const rtl::OUString  sSpecularity( RTL_CONSTASCII_USTRINGPARAM ( "Specularity" ) );
    static const rtl::OUString  sDiffusity( RTL_CONSTASCII_USTRINGPARAM ( "Diffusity" ) );
    static const rtl::OUString  sMetal( RTL_CONSTASCII_USTRINGPARAM ( "Metal" ) );
    static const rtl::OUString  sOn( RTL_CONSTASCII_USTRINGPARAM( "On" ) );

    com::sun::star::uno::Any* pAny;

    sal_Int32 nFinalSurface = -1;
    bool bHasCustomShape = false;

    for(i=0;i<nCount; i++)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetObj();
        if( pObj->ISA(SdrObjCustomShape) )
        {
            SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );

            // see if this is an extruded customshape
            if( !bHasCustomShape )
            {
                Any* pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sOn );
                if( pAny )
                    *pAny >>= bHasCustomShape;

                if( !bHasCustomShape )
                    continue;
            }

            sal_Int32 nSurface = 0; // wire frame

            sal_Int16 nRenderMode = EnhancedCustomShapeExtrusionRenderMode::SOLID;
            pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sRenderMode );
            if( pAny )
                *pAny >>= nRenderMode;

            if( nRenderMode == EnhancedCustomShapeExtrusionRenderMode::SOLID )
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
    ULONG nCount = rMarkList.GetMarkCount(), i;

    static const rtl::OUString  sExtrusion( RTL_CONSTASCII_USTRINGPARAM ( "Extrusion" ) );
    static const rtl::OUString  sBackwardDepth( RTL_CONSTASCII_USTRINGPARAM ( "BackwardDepth" ) );
    static const rtl::OUString  sOn( RTL_CONSTASCII_USTRINGPARAM( "On" ) );

    com::sun::star::uno::Any* pAny;

    double fFinalDepth = -1;
    bool bHasCustomShape = false;

    for(i=0;i<nCount; i++)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetObj();
        if( pObj->ISA(SdrObjCustomShape) )
        {
            SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );

            // see if this is an extruded customshape
            if( !bHasCustomShape )
            {
                Any* pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sOn );
                if( pAny )
                    *pAny >>= bHasCustomShape;

                if( !bHasCustomShape )
                    continue;
            }

            double fBackwardDepth = 1270.0;
            pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sBackwardDepth );
            if( pAny )
                *pAny >>= fBackwardDepth;

            if( fFinalDepth == -1 )
            {
                fFinalDepth = fBackwardDepth;
            }
            else if( fFinalDepth != fBackwardDepth )
            {
                fFinalDepth = -1;
                break;
            }
        }
    }

    if( pSdrView->GetModel() )
    {
        FieldUnit eUnit = pSdrView->GetModel()->GetUIUnit();
        rSet.Put( SfxUInt16Item( SID_ATTR_METRIC, (USHORT)eUnit ) );
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
    ULONG nCount = rMarkList.GetMarkCount(), i;

    static const rtl::OUString  sExtrusion( RTL_CONSTASCII_USTRINGPARAM ( "Extrusion" ) );
    static const rtl::OUString  sLightDirection1( RTL_CONSTASCII_USTRINGPARAM ( "LightDirection1" ) );
    static const rtl::OUString  sLightDirection2( RTL_CONSTASCII_USTRINGPARAM ( "LightDirection2" ) );
    static const rtl::OUString  sOn( RTL_CONSTASCII_USTRINGPARAM( "On" ) );

    const Direction3D * pLighting1Defaults;
    const Direction3D * pLighting2Defaults;

    getLightingDirectionDefaults( &pLighting1Defaults, &pLighting2Defaults );

    com::sun::star::uno::Any* pAny;

    int nFinalDirection = -1;
    bool bHasCustomShape = false;

    for(i=0;i<nCount; i++)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetObj();
        if( pObj->ISA(SdrObjCustomShape) )
        {
            SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );

            // see if this is an extruded customshape
            if( !bHasCustomShape )
            {
                Any* pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sOn );
                if( pAny )
                    *pAny >>= bHasCustomShape;

                if( !bHasCustomShape )
                    continue;
            }

            Direction3D aLightDirection1( 50000, 0, 10000 );
            Direction3D aLightDirection2( -50000, 0, 10000 );

            pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sLightDirection1 );
            if( pAny )
                *pAny >>= aLightDirection1;

            pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sLightDirection2 );
            if( pAny )
                *pAny >>= aLightDirection2;

            int nDirection = -1;

            int i;
            for( i = 0; i < 9; i++ )
            {
                if( compare_direction( aLightDirection1, pLighting1Defaults[i] ) &&
                    compare_direction( aLightDirection2, pLighting2Defaults[i] ))
                {
                    nDirection = i;
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
    ULONG nCount = rMarkList.GetMarkCount(), i;

    static const rtl::OUString  sExtrusion( RTL_CONSTASCII_USTRINGPARAM ( "Extrusion" ) );
    static const rtl::OUString  sBrightness( RTL_CONSTASCII_USTRINGPARAM ( "Brightness" ) );
    static const rtl::OUString  sOn( RTL_CONSTASCII_USTRINGPARAM( "On" ) );

    com::sun::star::uno::Any* pAny;

    int nFinalLevel = -1;
    bool bHasCustomShape = false;

    for(i=0;i<nCount; i++)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetObj();
        if( pObj->ISA(SdrObjCustomShape) )
        {
            SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );

            // see if this is an extruded customshape
            if( !bHasCustomShape )
            {
                Any* pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sOn );
                if( pAny )
                    *pAny >>= bHasCustomShape;

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
    ULONG nCount = rMarkList.GetMarkCount(), i;

    static const rtl::OUString  sExtrusion( RTL_CONSTASCII_USTRINGPARAM ( "Extrusion" ) );
    static const rtl::OUString  sExtrusionColor( RTL_CONSTASCII_USTRINGPARAM ( "Color" ) );
    static const rtl::OUString  sOn( RTL_CONSTASCII_USTRINGPARAM( "On" ) );

    com::sun::star::uno::Any* pAny;

    bool bInit = false;
    bool bAmbigius = false;
    Color aFinalColor;
    bool bHasCustomShape = false;

    for(i=0;i<nCount; i++)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetObj();
        if( pObj->ISA(SdrObjCustomShape) )
        {
            SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );

            // see if this is an extruded customshape
            if( !bHasCustomShape )
            {
                Any* pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sOn );
                if( pAny )
                    *pAny >>= bHasCustomShape;

                if( !bHasCustomShape )
                    continue;
            }

            Color aColor;

            bool bUseColor;
            pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sExtrusionColor );
            if( pAny )
                *pAny >>= bUseColor;

            if( bUseColor )
            {
                const XSecondaryFillColorItem& rItem = *(XSecondaryFillColorItem*)&(pObj->GetMergedItem( XATTR_SECONDARYFILLCOLOR ));
                aColor = rItem.GetValue();
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

bool checkForSelectedCustomShapes( SdrView* pSdrView, bool bOnlyExtruded )
{
    static const rtl::OUString  sExtrusion( RTL_CONSTASCII_USTRINGPARAM ( "Extrusion" ) );
    static const rtl::OUString  sOn( RTL_CONSTASCII_USTRINGPARAM( "On" ) );

    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    ULONG nCount = rMarkList.GetMarkCount(), i;
    bool bFound = false;

    for(i=0;(i<nCount) && !bFound ; i++)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetObj();
        if( pObj->ISA(SdrObjCustomShape) )
        {
            if( bOnlyExtruded )
            {
                SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
                Any* pAny = aGeometryItem.GetPropertyValueByName( sExtrusion, sOn );
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
    if (rSet.GetItemState(SID_EXTRUSION_TILT_DOWN) != SFX_ITEM_UNKNOWN)
    {
        if( !checkForSelectedCustomShapes( pSdrView, true ) )
            rSet.DisableItem( SID_EXTRUSION_TILT_DOWN );
    }
    if (rSet.GetItemState(SID_EXTRUSION_TILT_DOWN) != SFX_ITEM_UNKNOWN)
    {
        if( !checkForSelectedCustomShapes( pSdrView, true ) )
            rSet.DisableItem( SID_EXTRUSION_TILT_DOWN );
    }
    if (rSet.GetItemState(SID_EXTRUSION_TILT_UP) != SFX_ITEM_UNKNOWN)
    {
        if( !checkForSelectedCustomShapes( pSdrView, true ) )
            rSet.DisableItem( SID_EXTRUSION_TILT_UP );
    }
    if (rSet.GetItemState(SID_EXTRUSION_TILT_LEFT) != SFX_ITEM_UNKNOWN)
    {
        if( !checkForSelectedCustomShapes( pSdrView, true ) )
            rSet.DisableItem( SID_EXTRUSION_TILT_LEFT );
    }
    if (rSet.GetItemState(SID_EXTRUSION_TILT_RIGHT) != SFX_ITEM_UNKNOWN)
    {
        if( !checkForSelectedCustomShapes( pSdrView, true ) )
            rSet.DisableItem( SID_EXTRUSION_TILT_RIGHT );
    }
    if (rSet.GetItemState(SID_EXTRUSION_3D_COLOR) != SFX_ITEM_UNKNOWN)
    {
        if( !checkForSelectedCustomShapes( pSdrView, true ) )
            rSet.DisableItem( SID_EXTRUSION_3D_COLOR );
    }
    if (rSet.GetItemState(SID_EXTRUSION_DEPTH_FLOATER) != SFX_ITEM_UNKNOWN)
    {
        if( !checkForSelectedCustomShapes( pSdrView, true ) )
            rSet.DisableItem( SID_EXTRUSION_DEPTH_FLOATER );
    }
    if (rSet.GetItemState(SID_EXTRUSION_DIRECTION_FLOATER) != SFX_ITEM_UNKNOWN)
    {
        if( !checkForSelectedCustomShapes( pSdrView, true ) )
            rSet.DisableItem( SID_EXTRUSION_DIRECTION_FLOATER );
    }
    if (rSet.GetItemState(SID_EXTRUSION_LIGHTING_FLOATER) != SFX_ITEM_UNKNOWN)
    {
        if( !checkForSelectedCustomShapes( pSdrView, true ) )
            rSet.DisableItem( SID_EXTRUSION_LIGHTING_FLOATER );
    }
    if (rSet.GetItemState(SID_EXTRUSION_SURFACE_FLOATER) != SFX_ITEM_UNKNOWN)
    {
        if( !checkForSelectedCustomShapes( pSdrView, true ) )
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
