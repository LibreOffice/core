/*************************************************************************
 *
 *  $RCSfile: float3d.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: pb $ $Date: 2000-10-09 11:58:09 $
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
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXMODULE_HXX //autogen
#include <sfx2/module.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SV_COLRDLG_HXX //autogen
#include <svtools/colrdlg.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#ifndef _SFXVIEWSH_HXX //autogen
#include <sfx2/viewsh.hxx>
#endif
#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

#ifndef _SVX_XFLCLIT_HXX
#include <xflclit.hxx>
#endif
#ifndef _SVDMODEL_HXX
#include <svdmodel.hxx>
#endif
#ifndef _E3D_GLOBL3D_HXX
#include <globl3d.hxx>
#endif
#ifndef _E3D_VIEW3D_HXX
#include <view3d.hxx>
#endif
#ifndef _E3D_OBJ3D_HXX
#include <obj3d.hxx>
#endif
#ifndef _E3D_SPHERE3D_HXX
#include <sphere3d.hxx>
#endif
#ifndef _E3D_SCENE3D_HXX
#include <scene3d.hxx>
#endif
#ifndef _CAMERA3D_HXX
#include <camera3d.hxx>
#endif
#ifndef _SVX_FMMODEL_HXX
#include <fmmodel.hxx>
#endif
#ifndef _SVX_FMPAGE_HXX
#include <fmpage.hxx>
#endif
#ifndef _E3D_POLYSC3D_HXX
#include <polysc3d.hxx>
#endif
#ifndef _EEITEM_HXX //autogen
#include <eeitem.hxx>
#endif
#ifndef _SFXSTYLE_HXX
#include <svtools/style.hxx>
#endif


#include <dlgutil.hxx>
#include <dialmgr.hxx>
#include <viewpt3d.hxx> // ProjectionType

#include <svxids.hrc>
#include <dialogs.hrc>

#define ITEMID_COLOR            SID_ATTR_3D_LIGHTCOLOR
#include <colritem.hxx>
#include <e3ditem.hxx>

#include <gallery.hxx>
#define GALLERY_THEME "3D"


#include "float3d.hxx"
#include "float3d.hrc"

SFX_IMPL_DOCKINGWINDOW( Svx3DChildWindow, SID_3D_WIN )

/*************************************************************************
|*  Svx3DWin - FloatingWindow
\************************************************************************/
__EXPORT Svx3DWin::Svx3DWin( SfxBindings* pInBindings,
                SfxChildWindow *pCW, Window* pParent ) :
        SfxDockingWindow    ( pInBindings, pCW, pParent,
                                    SVX_RES( RID_SVXFLOAT_3D ) ),
        aBtnFavorites       ( this, SVX_RES( BTN_FAVORITES ) ),
        aBtnGeo             ( this, SVX_RES( BTN_GEO ) ),
        aBtnRepresentation  ( this, SVX_RES( BTN_REPRESENTATION ) ),
        aBtnLight           ( this, SVX_RES( BTN_LIGHT ) ),
        aBtnTexture         ( this, SVX_RES( BTN_TEXTURE ) ),
        aBtnMaterial        ( this, SVX_RES( BTN_MATERIAL ) ),
        aBtnUpdate          ( this, SVX_RES( BTN_UPDATE ) ),
        aBtnAssign          ( this, SVX_RES( BTN_ASSIGN ) ),

        // Favoriten
        aCtlFavorites       ( this, SVX_RES( CTL_FAVORITES ) ),
        aGrpFavorites       ( this, SVX_RES( GRP_FAVORITES ) ),
        aBtnOnly3D          ( this, SVX_RES( BTN_ONLY_3D ) ),
        aBtnAllAttributes   ( this, SVX_RES( BTN_ALL_ATTRIBUTES ) ),

        // Geometrie
        aFtHorizontal       ( this, SVX_RES( FT_HORIZONTAL ) ),
        aNumHorizontal      ( this, SVX_RES( NUM_HORIZONTAL ) ),
        aFtVertical         ( this, SVX_RES( FT_VERTICAL ) ),
        aNumVertical        ( this, SVX_RES( NUM_VERTICAL ) ),
        aGrpSegments        ( this, SVX_RES( GRP_SEGMENTS ) ),
        aFtPercentDiagonal  ( this, SVX_RES( FT_PERCENT_DIAGONAL ) ),
        aMtrPercentDiagonal ( this, SVX_RES( MTR_PERCENT_DIAGONAL ) ),
        aFtBackscale        ( this, SVX_RES( FT_BACKSCALE ) ),
        aMtrBackscale       ( this, SVX_RES( MTR_BACKSCALE ) ),
        aFtEndAngle         ( this, SVX_RES( FT_END_ANGLE ) ),
        aMtrEndAngle        ( this, SVX_RES( MTR_END_ANGLE ) ),
        aFtDepth            ( this, SVX_RES( FT_DEPTH ) ),
        aMtrDepth           ( this, SVX_RES( MTR_DEPTH ) ),
        aGrpGeometrie       ( this, SVX_RES( GRP_GEOMETRIE ) ),

        aBtnNormalsObj      ( this, SVX_RES( BTN_NORMALS_OBJ ) ),
        aBtnNormalsFlat     ( this, SVX_RES( BTN_NORMALS_FLAT ) ),
        aBtnNormalsSphere   ( this, SVX_RES( BTN_NORMALS_SPHERE ) ),
        aBtnTwoSidedLighting( this, SVX_RES( BTN_TWO_SIDED_LIGHTING ) ),
        aBtnNormalsInvert   ( this, SVX_RES( BTN_NORMALS_INVERT ) ),
        aGrpNormals         ( this, SVX_RES( GRP_NORMALS ) ),

        aBtnDoubleSided     ( this, SVX_RES( BTN_DOUBLE_SIDED ) ),

        // Darstellung
        aFtShademode        ( this, SVX_RES( FT_SHADEMODE ) ),
        aLbShademode        ( this, SVX_RES( LB_SHADEMODE ) ),
        aBtnShadow3d        ( this, SVX_RES( BTN_SHADOW_3D ) ),
        aFtSlant            ( this, SVX_RES( FT_SLANT ) ),
        aMtrSlant           ( this, SVX_RES( MTR_SLANT ) ),
        aGrpShadow          ( this, SVX_RES( GRP_SHADOW ) ),
        aFtDistance         ( this, SVX_RES( FT_DISTANCE ) ),
        aMtrDistance        ( this, SVX_RES( MTR_DISTANCE ) ),
        aFtFocalLeng        ( this, SVX_RES( FT_FOCAL_LENGTH ) ),
        aMtrFocalLength     ( this, SVX_RES( MTR_FOCAL_LENGTH ) ),
        aGrpCamera          ( this, SVX_RES( GRP_CAMERA ) ),
        aGrpRepresentation  ( this, SVX_RES( GRP_REPRESENTATION ) ),

        // Beleuchtung
        aBtnLight1          ( this, SVX_RES( BTN_LIGHT_1 ) ),
        aBtnLight2          ( this, SVX_RES( BTN_LIGHT_2 ) ),
        aBtnLight3          ( this, SVX_RES( BTN_LIGHT_3 ) ),
        aBtnLight4          ( this, SVX_RES( BTN_LIGHT_4 ) ),
        aBtnLight5          ( this, SVX_RES( BTN_LIGHT_5 ) ),
        aBtnLight6          ( this, SVX_RES( BTN_LIGHT_6 ) ),
        aBtnLight7          ( this, SVX_RES( BTN_LIGHT_7 ) ),
        aBtnLight8          ( this, SVX_RES( BTN_LIGHT_8 ) ),
        aLbLight1           ( this, SVX_RES( LB_LIGHT_1 ) ),
        aLbLight2           ( this, SVX_RES( LB_LIGHT_2 ) ),
        aLbLight3           ( this, SVX_RES( LB_LIGHT_3 ) ),
        aLbLight4           ( this, SVX_RES( LB_LIGHT_4 ) ),
        aLbLight5           ( this, SVX_RES( LB_LIGHT_5 ) ),
        aLbLight6           ( this, SVX_RES( LB_LIGHT_6 ) ),
        aLbLight7           ( this, SVX_RES( LB_LIGHT_7 ) ),
        aLbLight8           ( this, SVX_RES( LB_LIGHT_8 ) ),
        aBtnLightColor      ( this, SVX_RES( BTN_LIGHT_COLOR ) ),
        aGrpLightsource     ( this, SVX_RES( GRP_LIGHTSOURCE ) ),
        aLbAmbientlight     ( this, SVX_RES( LB_AMBIENTLIGHT ) ),
        aBtnAmbientColor    ( this, SVX_RES( BTN_AMBIENT_COLOR ) ),
        aGrpAmbientlight    ( this, SVX_RES( GRP_AMBIENTLIGHT ) ),
        aGrpLight           ( this, SVX_RES( GRP_LIGHT ) ),
        aFtLightX           ( this, SVX_RES( FT_LIGHT_X ) ),
        aFtLightY           ( this, SVX_RES( FT_LIGHT_Y ) ),
        aFtLightZ           ( this, SVX_RES( FT_LIGHT_Z ) ),
        aGrpLightInfo       ( this, SVX_RES( GRP_LIGHT_INFO ) ),

        // Texturen
        aFtTexKind          ( this, SVX_RES( FT_TEX_KIND ) ),
        aBtnTexLuminance    ( this, SVX_RES( BTN_TEX_LUMINANCE ) ),
        aBtnTexColor        ( this, SVX_RES( BTN_TEX_COLOR ) ),
        aFtTexMode          ( this, SVX_RES( FT_TEX_MODE ) ),
        aBtnTexReplace      ( this, SVX_RES( BTN_TEX_REPLACE ) ),
        aBtnTexModulate     ( this, SVX_RES( BTN_TEX_MODULATE ) ),
        aBtnTexBlend        ( this, SVX_RES( BTN_TEX_BLEND ) ),
        aFtTexProjectionX   ( this, SVX_RES( FT_TEX_PROJECTION_X ) ),
        aBtnTexParallelX    ( this, SVX_RES( BTN_TEX_PARALLEL_X ) ),
        aBtnTexCircleX      ( this, SVX_RES( BTN_TEX_CIRCLE_X ) ),
        aBtnTexObjectX      ( this, SVX_RES( BTN_TEX_OBJECT_X ) ),
        aFtTexProjectionY   ( this, SVX_RES( FT_TEX_PROJECTION_Y ) ),
        aBtnTexParallelY    ( this, SVX_RES( BTN_TEX_PARALLEL_Y ) ),
        aBtnTexCircleY      ( this, SVX_RES( BTN_TEX_CIRCLE_Y ) ),
        aBtnTexObjectY      ( this, SVX_RES( BTN_TEX_OBJECT_Y ) ),
        aFtTexFilter        ( this, SVX_RES( FT_TEX_FILTER ) ),
        aBtnTexFilter       ( this, SVX_RES( BTN_TEX_FILTER ) ),
        aGrpTexture         ( this, SVX_RES( GRP_TEXTURE ) ),

        // Material
        aFtMatFavorites     ( this, SVX_RES( FT_MAT_FAVORITES ) ),
        aLbMatFavorites     ( this, SVX_RES( LB_MAT_FAVORITES ) ),
        aFtMatColor         ( this, SVX_RES( FT_MAT_COLOR ) ),
        aLbMatColor         ( this, SVX_RES( LB_MAT_COLOR ) ),
        aBtnMatColor        ( this, SVX_RES( BTN_MAT_COLOR ) ),
        aFtMatEmission      ( this, SVX_RES( FT_MAT_EMISSION ) ),
        aLbMatEmission      ( this, SVX_RES( LB_MAT_EMISSION ) ),
        aBtnEmissionColor   ( this, SVX_RES( BTN_EMISSION_COLOR ) ),
        aFtMatSpecular      ( this, SVX_RES( FT_MAT_SPECULAR ) ),
        aLbMatSpecular      ( this, SVX_RES( LB_MAT_SPECULAR ) ),
        aBtnSpecularColor   ( this, SVX_RES( BTN_SPECULAR_COLOR ) ),
        aFtMatSpecularIntensity( this, SVX_RES( FT_MAT_SPECULAR_INTENSITY ) ),
        aMtrMatSpecularIntensity( this, SVX_RES( MTR_MAT_SPECULAR_INTENSITY ) ),
        aGrpMatSpecular     ( this, SVX_RES( GRP_MAT_SPECULAR ) ),
        aGrpMaterial        ( this, SVX_RES( GRP_MATERIAL ) ),

        // Unterer Bereich
        aBtnPerspective     ( this, SVX_RES( BTN_PERSPECTIVE ) ),
        aBtnConvertTo3D     ( this, SVX_RES( BTN_CHANGE_TO_3D ) ),
        aBtnLatheObject     ( this, SVX_RES( BTN_LATHE_OBJ ) ),
        aCtlPreview         ( this, SVX_RES( CTL_PREVIEW ) ),
        aCtlLightPreview    ( this, SVX_RES( CTL_LIGHT_PREVIEW ) ),
        aGrpPreview         ( this, SVX_RES( GRP_PREVIEW ) ),

        aImgLightOn         ( SVX_RES( RID_SVXIMAGE_LIGHT_ON ) ),
        aImgLightOff        ( SVX_RES( RID_SVXIMAGE_LIGHT_OFF ) ),

        pLightGroup         ( NULL ),
        bUpdate             ( FALSE ),
        eViewType           ( VIEWTYPE_FAVORITES ),

        pModel              ( NULL ),
        pFmPage             ( NULL ),
        pVDev               ( NULL ),
        p3DView             ( NULL ),
        pFavorSetList       ( NULL ),
        pMatFavSetList      ( NULL ),

        pBindings           ( pInBindings ),
        pPool               ( NULL ),
        bOnly3DChanged      ( FALSE )
{
    FreeResource();

    // Metrik einstellen
    eFUnit = GetModuleFieldUnit();
    aMtrDepth.SetUnit( eFUnit );
    aMtrDistance.SetUnit( eFUnit );
    aMtrFocalLength.SetUnit( eFUnit );

    pControllerItem = new Svx3DCtrlItem(SID_3D_STATE, this, pBindings);
    pConvertTo3DItem = new SvxConvertTo3DItem(SID_CONVERT_TO_3D, pBindings);
    pConvertTo3DLatheItem = new SvxConvertTo3DItem(SID_CONVERT_TO_3D_LATHE_FAST, pBindings);

    aBtnAssign.SetClickHdl( LINK( this, Svx3DWin, ClickAssignHdl ) );
    aBtnUpdate.SetClickHdl( LINK( this, Svx3DWin, ClickUpdateHdl ) );

    Link aLink( LINK( this, Svx3DWin, ClickViewTypeHdl ) );
    aBtnFavorites.SetClickHdl( aLink );
    aBtnGeo.SetClickHdl( aLink );
    aBtnRepresentation.SetClickHdl( aLink );
    aBtnLight.SetClickHdl( aLink );
    aBtnTexture.SetClickHdl( aLink );
    aBtnMaterial.SetClickHdl( aLink );

    aLink = LINK( this, Svx3DWin, ClickHdl );
    aBtnPerspective.SetClickHdl( aLink );
    aBtnConvertTo3D.SetClickHdl( aLink );
    aBtnLatheObject.SetClickHdl( aLink );

    aBtnOnly3D.SetClickHdl( aLink );
    aBtnAllAttributes.SetClickHdl( aLink );

    // Geometrie
    aBtnNormalsObj.SetClickHdl( aLink );
    aBtnNormalsFlat.SetClickHdl( aLink );
    aBtnNormalsSphere.SetClickHdl( aLink );
    aBtnTwoSidedLighting.SetClickHdl( aLink );
    aBtnNormalsInvert.SetClickHdl( aLink );
    aBtnDoubleSided.SetClickHdl( aLink );

    // Darstellung
    aBtnShadow3d.SetClickHdl( aLink );

    // Beleuchtung
    aBtnLight1.SetClickHdl( aLink );
    aBtnLight2.SetClickHdl( aLink );
    aBtnLight3.SetClickHdl( aLink );
    aBtnLight4.SetClickHdl( aLink );
    aBtnLight5.SetClickHdl( aLink );
    aBtnLight6.SetClickHdl( aLink );
    aBtnLight7.SetClickHdl( aLink );
    aBtnLight8.SetClickHdl( aLink );

    // Texturen
    aBtnTexLuminance.SetClickHdl( aLink );
    aBtnTexColor.SetClickHdl( aLink );
    aBtnTexReplace.SetClickHdl( aLink );
    aBtnTexModulate.SetClickHdl( aLink );
    //aBtnTexBlend.SetClickHdl( aLink );
    aBtnTexParallelX.SetClickHdl( aLink );
    aBtnTexCircleX.SetClickHdl( aLink );
    aBtnTexObjectX.SetClickHdl( aLink );
    aBtnTexParallelY.SetClickHdl( aLink );
    aBtnTexCircleY.SetClickHdl( aLink );
    aBtnTexObjectY.SetClickHdl( aLink );
    aBtnTexFilter.SetClickHdl( aLink );

    // Material
    aLink = LINK( this, Svx3DWin, ClickColorHdl );
    aBtnLightColor.SetClickHdl( aLink );
    aBtnAmbientColor.SetClickHdl( aLink );
    aBtnMatColor.SetClickHdl( aLink );
    aBtnEmissionColor.SetClickHdl( aLink );
    aBtnSpecularColor.SetClickHdl( aLink );


    aLink = LINK( this, Svx3DWin, SelectHdl );
    aLbMatFavorites.SetSelectHdl( aLink );
    aLbMatColor.SetSelectHdl( aLink );
    aLbMatEmission.SetSelectHdl( aLink );
    aLbMatSpecular.SetSelectHdl( aLink );
    aLbLight1.SetSelectHdl( aLink );
    aLbLight2.SetSelectHdl( aLink );
    aLbLight3.SetSelectHdl( aLink );
    aLbLight4.SetSelectHdl( aLink );
    aLbLight5.SetSelectHdl( aLink );
    aLbLight6.SetSelectHdl( aLink );
    aLbLight7.SetSelectHdl( aLink );
    aLbLight8.SetSelectHdl( aLink );
    aLbAmbientlight.SetSelectHdl( aLink );
    aLbShademode.SetSelectHdl( aLink );

    aLink = LINK( this, Svx3DWin, ModifyHdl );
    aMtrMatSpecularIntensity.SetModifyHdl( aLink );
    aNumHorizontal.SetModifyHdl( aLink );
    aNumVertical.SetModifyHdl( aLink );
    aMtrSlant.SetModifyHdl( aLink );

    aLink = LINK( this, Svx3DWin, DoubleClickHdl );
    aCtlFavorites.SetDoubleClickHdl( aLink );

    aLink = LINK( this, Svx3DWin, ClickFavoriteHdl );
    aCtlFavorites.SetSelectHdl( aLink );

    // Preview-Callback
    aLink = LINK( this, Svx3DWin, ChangeLightCallbackHdl );
    aCtlLightPreview.SetUserInteractiveChangeCallback( aLink );
    aLink = LINK( this, Svx3DWin, ChangeSelectionCallbackHdl );
    aCtlLightPreview.SetUserSelectionChangeCallback( aLink );

    aSize = GetOutputSizePixel();
    SetMinOutputSizePixel( aSize );

    Construct();

    // Initiierung der Initialisierung der ColorLBs
    SfxBoolItem aItem( SID_3D_INIT, TRUE );
    SfxViewFrame::Current()->GetBindings().GetDispatcher()->Execute(
        SID_3D_INIT, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L );

    Reset();
}

// -----------------------------------------------------------------------
__EXPORT Svx3DWin::~Svx3DWin()
{
    // ListenSets loeschen
    /*
    if( pFavorSetList ) // sollte nicht NULL sein
    {
        SfxItemSet* pSet;
        for( pSet = (SfxItemSet*) pFavorSetList->First();
             pSet;
             pSet = (SfxItemSet*) pFavorSetList->Next() )
        {
            delete pSet;
        }
        delete pFavorSetList;
    }
    */

    //delete pMatFavSetList;
    delete p3DView;
    delete pVDev;
    delete pModel;

    delete pControllerItem;
    delete pConvertTo3DItem;
    delete pConvertTo3DLatheItem;

    delete pLightGroup;
}

// -----------------------------------------------------------------------
void Svx3DWin::Construct()
{
    // Ueber die Gallery werden die Favoriten eingelesen
    ULONG nFavCount = GalleryExplorer::GetSdrObjCount( GALLERY_THEME_3D );

    // ValueSet Favoriten
    WinBits nWinBits = aCtlFavorites.GetStyle();
    nWinBits |= WB_ITEMBORDER | WB_DOUBLEBORDER; //| WB_NAMEFIELD
    if( nFavCount > 6 )
        nWinBits |= WB_VSCROLL;

    aCtlFavorites.SetStyle( nWinBits );
    aCtlFavorites.SetColCount( 3 );
    aCtlFavorites.SetLineCount( 2 );
    aCtlFavorites.SetExtraSpacing( 1 );

    // Listen erzeugen
//  pFavorSetList = new List();
//  SfxItemSet* pSet;

    // Gallery thema locken
    GalleryExplorer::BeginLocking(GALLERY_THEME_3D);

    for( ULONG nModelPos = 0; nModelPos < nFavCount; nModelPos++ )
    {
        Bitmap  aThumb;
        Size    aNewSize( 60, 60 );

        if( GalleryExplorer::GetSdrObj( GALLERY_THEME_3D, nModelPos,
                                        pModel, &aThumb ) )
        {
            // ValueSet fuellen
            Size aOldSize = Size( aThumb.GetSizePixel() );
            aThumb.Scale( (double) aNewSize.Width() / aOldSize.Width(),
                            (double) aNewSize.Height() / aOldSize.Height() );

            String aStr(SVX_RES(RID_SVXFLOAT3D_FAVORITE));
            aStr += sal_Unicode(' ');
            aStr += String::CreateFromInt32((INT32)nModelPos + 1L);

            aCtlFavorites.InsertItem( (USHORT)nModelPos+1, aThumb, aStr );
        }
    }

    // Gallery thema freigeben
    GalleryExplorer::EndLocking(GALLERY_THEME_3D);

    if( nFavCount == 0 )
    {
        // Keine Favoriten vorhanden
        eViewType = VIEWTYPE_GEO;
    }

    aCtlLightPreview.Hide();
    pLightGroup = new B3dLightGroup();
    *pLightGroup = *aCtlLightPreview.GetPreviewControl().GetLightGroup();
}

// -----------------------------------------------------------------------
void Svx3DWin::Reset()
{
    // Diverse Initialisierungen, default ist AllAttributes
    aBtnAllAttributes.Check();
    aLbMatFavorites.SelectEntryPos( 0 ); // UserDefined

    if( eViewType == VIEWTYPE_FAVORITES )
        ClickViewTypeHdl( &aBtnFavorites );
    else
    {
        ClickViewTypeHdl( &aBtnGeo );
        aBtnFavorites.Enable( FALSE );
    }

    aLbShademode.SelectEntryPos( 0 );
    aMtrMatSpecularIntensity.SetValue( 50 );

    aBtnLight1.Check();
//  ClickHdl( &aBtnLight1 );

    ClickUpdateHdl( NULL );

    // Nichts selektieren, um Fehler beim erstselektieren zu vermeiden
    aCtlLightPreview.GetPreviewControl().SelectLight(Base3DLight0);
}

// -----------------------------------------------------------------------
void Svx3DWin::Update( SfxItemSet& rAttrs )
{
    const SfxPoolItem* pItem;
    BOOL bUpdate = FALSE;

    // evtl. PoolUnit ermitteln
    if( !pPool )
    {
        pPool = rAttrs.GetPool();
        DBG_ASSERT( pPool, "Wo ist der Pool?" );
        ePoolUnit = pPool->GetMetric( SID_ATTR_LINE_WIDTH );
    }
    eFUnit = GetModuleFieldUnit();


// Segmentanzahl aenderbar ? und andere Stati
    SfxItemState eState = rAttrs.GetItemState( SID_ATTR_3D_INTERN, FALSE, &pItem );
    if( SFX_ITEM_SET == eState )
    {
        UINT32 nState = ( ( const SfxUInt32Item* )pItem )->GetValue();
        BOOL bLathe   = (BOOL) ( nState & 1 );
        BOOL bExtrude = (BOOL) ( nState & 2 );
        BOOL bSphere  = (BOOL) ( nState & 4 );
        BOOL bCube    = (BOOL) ( nState & 8 );
        BOOL bNothing = (BOOL) ( nState & 16 ); // Keine Objekte in der Scene

        BOOL bChart = (BOOL) ( nState & 32 ); // Chart

        if( !bChart )
        {
            // Bei Cube-Objekten werden keine Segmente eingestellt
            aFtHorizontal.Enable( !bCube );
            aNumHorizontal.Enable( !bCube );
            aFtVertical.Enable( !bCube );
            aNumVertical.Enable( !bCube );
            aGrpSegments.Enable( !bCube );

            aFtPercentDiagonal.Enable( !bCube && !bSphere );
            aMtrPercentDiagonal.Enable( !bCube && !bSphere );
            aFtBackscale.Enable( !bCube && !bSphere );
            aMtrBackscale.Enable( !bCube && !bSphere );
            aFtDepth.Enable( !bCube && !bSphere );
            aMtrDepth.Enable( !bCube && !bSphere );
            if( bCube )
            {
                aNumHorizontal.SetEmptyFieldValue();
                aNumVertical.SetEmptyFieldValue();
            }
            if( bCube || bSphere )
            {
                aMtrPercentDiagonal.SetEmptyFieldValue();
                aMtrBackscale.SetEmptyFieldValue();
                aMtrDepth.SetEmptyFieldValue();
            }

            // Nur bei Lathe-Objekten gibt es einen Endwinkel
            aFtEndAngle.Enable( !bExtrude && !bCube && !bSphere );
            aMtrEndAngle.Enable( !bExtrude && !bCube && !bSphere );
            if( bExtrude || bCube || bSphere )
                aMtrEndAngle.SetEmptyFieldValue();

            // Sind keine Objekte selektiert
            //aBtnConvertTo3D.Enable( !bNothing );
            //aBtnLatheObject.Enable( !bNothing );
        }
        else
        {
            // Geometrie
            aFtHorizontal.Enable( FALSE );
            aNumHorizontal.Enable( FALSE );
            aNumHorizontal.SetEmptyFieldValue();
            aFtVertical.Enable( FALSE );
            aNumVertical.Enable( FALSE );
            aNumVertical.SetEmptyFieldValue();
            aGrpSegments.Enable( FALSE );
            aFtEndAngle.Enable( FALSE );
            aMtrEndAngle.Enable( FALSE );
            aMtrEndAngle.SetEmptyFieldValue();
            aFtDepth.Enable( FALSE );
            aMtrDepth.Enable( FALSE );
            aMtrDepth.SetEmptyFieldValue();

            // Darstellung
            aBtnShadow3d.Enable( FALSE );
            aFtSlant.Enable( FALSE );
            aMtrSlant.Enable( FALSE );
            aGrpShadow.Enable( FALSE );

            aFtDistance.Enable( FALSE );
            aMtrDistance.Enable( FALSE );
            aMtrDistance.SetEmptyFieldValue();
            aFtFocalLeng.Enable( FALSE );
            aMtrFocalLength.Enable( FALSE );
            aMtrFocalLength.SetEmptyFieldValue();
            aGrpCamera.Enable( FALSE );

            // Unterer Bereich
            aBtnConvertTo3D.Enable( FALSE );
            aBtnLatheObject.Enable( FALSE );
        }
    }
// Bitmapfuellung ? -> Status
    BOOL bBitmap = FALSE;
    if( SFX_ITEM_SET == rAttrs.GetItemState( XATTR_FILLSTYLE, FALSE, &pItem ) )
    {
        XFillStyle eXFS = (XFillStyle) ( ( const XFillStyleItem* )pItem )->GetValue();
        if( eXFS == XFILL_BITMAP
            || eXFS == XFILL_GRADIENT
            || eXFS == XFILL_HATCH)
            bBitmap = TRUE;
    }
    aFtTexKind.Enable( bBitmap );
    aBtnTexLuminance.Enable( bBitmap );
    aBtnTexColor.Enable( bBitmap );
    aFtTexMode.Enable( bBitmap );
    aBtnTexReplace.Enable( bBitmap );
    aBtnTexModulate.Enable( bBitmap );
    aBtnTexBlend.Enable( bBitmap );
    aFtTexProjectionX.Enable( bBitmap );
    aBtnTexParallelX.Enable( bBitmap );
    aBtnTexCircleX.Enable( bBitmap );
    aBtnTexObjectX.Enable( bBitmap );
    aFtTexProjectionY.Enable( bBitmap );
    aBtnTexParallelY.Enable( bBitmap );
    aBtnTexCircleY.Enable( bBitmap );
    aBtnTexObjectY.Enable( bBitmap );
    aFtTexFilter.Enable( bBitmap );
    aBtnTexFilter.Enable( bBitmap );
    aGrpTexture.Enable( bBitmap );


// Geometrie
    // Anzahl Segmente (horizontal)
    if( aNumHorizontal.IsEnabled() )
    {
        if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_HORZ_SEGS, FALSE, &pItem ) )
        {
            UINT32 nValue = ( ( const SfxUInt32Item* )pItem )->GetValue();
            if( nValue != (UINT32) aNumHorizontal.GetValue() )
            {
                aNumHorizontal.SetValue( nValue );
                // evtl. am Ende...
                aCtlLightPreview.GetPreviewControl().SetHorizontalSegments( (UINT16)nValue );
                bUpdate = TRUE;
            }
            else if( aNumHorizontal.IsEmptyFieldValue() )
                aNumHorizontal.SetValue( nValue );
        }
        else
        {
            if( !aNumHorizontal.IsEmptyFieldValue() )
            {
                aNumHorizontal.SetEmptyFieldValue();
                bUpdate = TRUE;
            }
        }
    }

    // Anzahl Segmente (vertikal)
    if( aNumVertical.IsEnabled() )
    {
        if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_VERT_SEGS, FALSE, &pItem ) )
        {
            UINT32 nValue = ( ( const SfxUInt32Item* )pItem )->GetValue();
            if( nValue != (UINT32) aNumVertical.GetValue() )
            {
                aNumVertical.SetValue( nValue );
                // evtl. am Ende...
                aCtlLightPreview.GetPreviewControl().SetVerticalSegments( (UINT16)nValue );
                //aCtlPreview.SetVerticalSegments( (UINT16)nValue );
                bUpdate = TRUE;
            }
            else if( aNumVertical.IsEmptyFieldValue() )
                aNumVertical.SetValue( nValue );
        }
        else
        {
            if( !aNumVertical.IsEmptyFieldValue() )
            {
                aNumVertical.SetEmptyFieldValue();
                bUpdate = TRUE;
            }
        }
    }

    // Tiefe
    if( aMtrDepth.IsEnabled() )
    {
        if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_DEPTH, FALSE, &pItem ) )
        {
            UINT32 nValue = ( ( const SfxUInt32Item* )pItem )->GetValue();
            UINT32 nValue2 = GetCoreValue( aMtrDepth, ePoolUnit );
            if( nValue != nValue2 )
            {
                if( eFUnit != aMtrDepth.GetUnit() )
                    SetFieldUnit( aMtrDepth, eFUnit );

                SetMetricValue( aMtrDepth, nValue, ePoolUnit );
                bUpdate = TRUE;
            }
            else if( aMtrDepth.IsEmptyFieldValue() )
                aMtrDepth.SetValue( aMtrDepth.GetValue() );
        }
        else
        {
            if( !aMtrDepth.IsEmptyFieldValue() )
            {
                aMtrDepth.SetEmptyFieldValue();
                bUpdate = TRUE;
            }
        }
    }

    // Doppelwandig/-seitig
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_DOUBLE_SIDED, FALSE, &pItem ) )
    {
        BOOL bValue = ( ( const SfxBoolItem* )pItem )->GetValue();
        if( bValue != aBtnDoubleSided.IsChecked() )
        {
            aBtnDoubleSided.Check( bValue );
            bUpdate = TRUE;
        }
        else if( aBtnDoubleSided.GetState() == STATE_DONTKNOW )
            aBtnDoubleSided.Check( bValue );
    }
    else
    {
        if( aBtnDoubleSided.GetState() != STATE_DONTKNOW )
        {
            aBtnDoubleSided.SetState( STATE_DONTKNOW );
            bUpdate = TRUE;
        }
    }

    // Kantenrundung
    if( aMtrPercentDiagonal.IsEnabled() )
    {
        if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_PERCENT_DIAGONAL, FALSE, &pItem ) )
        {
            UINT16 nValue = ( ( const SfxUInt16Item* )pItem )->GetValue();
            if( nValue != aMtrPercentDiagonal.GetValue() )
            {
                aMtrPercentDiagonal.SetValue( nValue );
                bUpdate = TRUE;
            }
            else if( aMtrPercentDiagonal.IsEmptyFieldValue() )
                aMtrPercentDiagonal.SetValue( nValue );
        }
        else
        {
            if( !aMtrPercentDiagonal.IsEmptyFieldValue() )
            {
                aMtrPercentDiagonal.SetEmptyFieldValue();
                bUpdate = TRUE;
            }
        }
    }

    // Tiefenskalierung
    if( aMtrBackscale.IsEnabled() )
    {
        if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_BACKSCALE, FALSE, &pItem ) )
        {
            UINT16 nValue = ( ( const SfxUInt16Item* )pItem )->GetValue();
            if( nValue != aMtrBackscale.GetValue() )
            {
                aMtrBackscale.SetValue( nValue );
                bUpdate = TRUE;
            }
            else if( aMtrBackscale.IsEmptyFieldValue() )
                aMtrBackscale.SetValue( nValue );
        }
        else
        {
            if( !aMtrBackscale.IsEmptyFieldValue() )
            {
                aMtrBackscale.SetEmptyFieldValue();
                bUpdate = TRUE;
            }
        }
    }

    // Endwinkel
    if( aMtrEndAngle.IsEnabled() )
    {
        if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_END_ANGLE, FALSE, &pItem ) )
        {
            UINT16 nValue = ( ( const SfxUInt16Item* )pItem )->GetValue();
            if( nValue != aMtrEndAngle.GetValue() )
            {
                aMtrEndAngle.SetValue( nValue );
                bUpdate = TRUE;
            }
        }
        else
        {
            if( !aMtrEndAngle.IsEmptyFieldValue() )
            {
                aMtrEndAngle.SetEmptyFieldValue();
                bUpdate = TRUE;
            }
        }
    }

    // Normalentyp
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_NORMALS_KIND, FALSE, &pItem ) )
    {
        UINT16 nValue = ( ( const SfxUInt16Item* )pItem )->GetValue();

        if( ( !aBtnNormalsObj.IsChecked() && nValue == 0 ) ||
            ( !aBtnNormalsFlat.IsChecked() && nValue == 1 ) ||
            ( !aBtnNormalsSphere.IsChecked() && nValue == 2 ) )
        {
            aBtnNormalsObj.Check( nValue == 0 );
            aBtnNormalsFlat.Check( nValue == 1 );
            aBtnNormalsSphere.Check( nValue == 2 );
            bUpdate = TRUE;
        }
    }
    else
    {
        if( aBtnNormalsObj.IsChecked() ||
            aBtnNormalsFlat.IsChecked() ||
            aBtnNormalsSphere.IsChecked() )
        {
            aBtnNormalsObj.Check( FALSE );
            aBtnNormalsFlat.Check( FALSE );
            aBtnNormalsSphere.Check( FALSE );
            bUpdate = TRUE;
        }
    }

    // Normalen invertieren
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_NORMALS_INVERT, FALSE, &pItem ) )
    {
        BOOL bValue = ( ( const SfxBoolItem* )pItem )->GetValue();
        if( bValue != aBtnNormalsInvert.IsChecked() )
        {
            aBtnNormalsInvert.Check( bValue );
            bUpdate = TRUE;
        }
        else if( aBtnNormalsInvert.GetState() == STATE_DONTKNOW )
            aBtnNormalsInvert.Check( bValue );
    }
    else
    {
        if( aBtnNormalsInvert.GetState() != STATE_DONTKNOW )
        {
            aBtnNormalsInvert.SetState( STATE_DONTKNOW );
            bUpdate = TRUE;
        }
    }

    // 2-seitige Beleuchtung
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_TWO_SIDED_LIGHTING, FALSE, &pItem ) )
    {
        BOOL bValue = ( ( const SfxBoolItem* )pItem )->GetValue();
        if( bValue != aBtnTwoSidedLighting.IsChecked() )
        {
            aBtnTwoSidedLighting.Check( bValue );
            bUpdate = TRUE;
        }
        else if( aBtnTwoSidedLighting.GetState() == STATE_DONTKNOW )
            aBtnTwoSidedLighting.Check( bValue );
    }
    else
    {
        if( aBtnTwoSidedLighting.GetState() != STATE_DONTKNOW )
        {
            aBtnTwoSidedLighting.SetState( STATE_DONTKNOW );
            bUpdate = TRUE;
        }
    }

// Darstellung
    // Shademode
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_SHADE_MODE, FALSE, &pItem ) )
    {
        UINT16 nValue = ( ( const SfxUInt16Item* )pItem )->GetValue();
        if( nValue != aLbShademode.GetSelectEntryPos() )
        {
            aLbShademode.SelectEntryPos( nValue );
            bUpdate = TRUE;
        }
    }
    else
    {
        if( aLbShademode.GetSelectEntryCount() != 0 )
        {
            aLbShademode.SetNoSelection();
            bUpdate = TRUE;
        }
    }

    // 3D-Shatten
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_SHADOW_3D, FALSE, &pItem ) )
    {
        BOOL bValue = ( ( const SfxBoolItem* )pItem )->GetValue();
        if( bValue != aBtnShadow3d.IsChecked() )
        {
            aBtnShadow3d.Check( bValue );
            aFtSlant.Enable( bValue );
            aMtrSlant.Enable( bValue );
            bUpdate = TRUE;
        }
        else if( aBtnShadow3d.GetState() == STATE_DONTKNOW )
            aBtnShadow3d.Check( bValue );
    }
    else
    {
        if( aBtnShadow3d.GetState() != STATE_DONTKNOW )
        {
            aBtnShadow3d.SetState( STATE_DONTKNOW );
            bUpdate = TRUE;
        }
    }

    // Neigung (Schatten)
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_SHADOW_SLANT, FALSE, &pItem ) )
    {
        UINT16 nValue = ( ( const SfxUInt16Item* )pItem )->GetValue();
        if( nValue != aMtrSlant.GetValue() )
        {
            aMtrSlant.SetValue( nValue );
            bUpdate = TRUE;
        }
    }
    else
    {
        if( !aMtrSlant.IsEmptyFieldValue() )
        {
            aMtrSlant.SetEmptyFieldValue();
            bUpdate = TRUE;
        }
    }

    // Distanz
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_DISTANCE, FALSE, &pItem ) )
    {
        UINT32 nValue = ( ( const SfxUInt32Item* )pItem )->GetValue();
        UINT32 nValue2 = GetCoreValue( aMtrDistance, ePoolUnit );
        if( nValue != nValue2 )
        {
            if( eFUnit != aMtrDistance.GetUnit() )
                SetFieldUnit( aMtrDistance, eFUnit );

            SetMetricValue( aMtrDistance, nValue, ePoolUnit );
            bUpdate = TRUE;
        }
    }
    else
    {
        if( !aMtrDepth.IsEmptyFieldValue() )
        {
            aMtrDepth.SetEmptyFieldValue();
            bUpdate = TRUE;
        }
    }

    // Brennweite
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_FOCAL_LENGTH, FALSE, &pItem ) )
    {
        UINT32 nValue = ( ( const SfxUInt32Item* )pItem )->GetValue();
        UINT32 nValue2 = GetCoreValue( aMtrFocalLength, ePoolUnit );
        if( nValue != nValue2 )
        {
            if( eFUnit != aMtrFocalLength.GetUnit() )
                SetFieldUnit( aMtrFocalLength, eFUnit );

            SetMetricValue( aMtrFocalLength, nValue, ePoolUnit );
            bUpdate = TRUE;
        }
    }
    else
    {
        if( !aMtrFocalLength.IsEmptyFieldValue() )
        {
            aMtrFocalLength.SetEmptyFieldValue();
            bUpdate = TRUE;
        }
    }

// Beleuchtung
    Color aColor;
    Vector3D aVector;
    // Licht 1 (Farbe)
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_LIGHTCOLOR_1, FALSE, &pItem ) )
    {
        aColor = ( ( const SvxColorItem* )pItem )->GetValue();
        ColorLB* pLb = &aLbLight1;
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );

            pLightGroup->SetIntensity( aColor, Base3DMaterialDiffuse, Base3DLight0 );
            bUpdate = TRUE;
        }
    }
    else
    {
        if( aLbLight1.GetSelectEntryCount() != 0 )
        {
            aLbLight1.SetNoSelection();
            bUpdate = TRUE;
        }
    }
    // Licht 1 (an/aus)
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_LIGHTON_1, FALSE, &pItem ) )
    {
        BOOL bOn = ( ( const SfxBoolItem* )pItem )->GetValue();
        if( ( bOn && aBtnLight1.GetImage() != aImgLightOn ) ||
            ( !bOn && aBtnLight1.GetImage() != aImgLightOff ) )
        {
            aBtnLight1.SetImage( bOn ? aImgLightOn : aImgLightOff );
            pLightGroup->Enable( bOn, Base3DLight0 );
            bUpdate = TRUE;
        }
        if( aBtnLight1.GetState() == STATE_DONTKNOW )
            aBtnLight1.Check( aBtnLight1.IsChecked() );
    }
    else
    {
        if( aBtnLight1.GetState() != STATE_DONTKNOW )
        {
            aBtnLight1.SetState( STATE_DONTKNOW );
            bUpdate = TRUE;
        }
    }
    // Licht 1 (Richtung)
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_LIGHTDIRECTION_1, FALSE, &pItem ) )
    {
        aVector = ( ( const SvxVector3DItem* )pItem )->GetValue();
        Vector3D aVector2 = pLightGroup->GetDirection( Base3DLight0 );
        if( aVector != aVector2 )
        {
            pLightGroup->SetDirection( aVector, Base3DLight0 );
            bUpdate = TRUE;
        }
    }

    // Licht 2 (Farbe)
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_LIGHTCOLOR_2, FALSE, &pItem ) )
    {
        aColor = ( ( const SvxColorItem* )pItem )->GetValue();
        ColorLB* pLb = &aLbLight2;
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );

            pLightGroup->SetIntensity( aColor, Base3DMaterialDiffuse, Base3DLight1 );
            bUpdate = TRUE;
        }
    }
    else
    {
        if( aLbLight2.GetSelectEntryCount() != 0 )
        {
            aLbLight2.SetNoSelection();
            bUpdate = TRUE;
        }
    }
    // Licht 2 (an/aus)
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_LIGHTON_2, FALSE, &pItem ) )
    {
        BOOL bOn = ( ( const SfxBoolItem* )pItem )->GetValue();
        if( ( bOn && aBtnLight2.GetImage() != aImgLightOn ) ||
            ( !bOn && aBtnLight2.GetImage() != aImgLightOff ) )
        {
            aBtnLight2.SetImage( bOn ? aImgLightOn : aImgLightOff );
            pLightGroup->Enable( bOn, Base3DLight1 );
            bUpdate = TRUE;
        }
        if( aBtnLight2.GetState() == STATE_DONTKNOW )
            aBtnLight2.Check( aBtnLight2.IsChecked() );
    }
    else
    {
        if( aBtnLight2.GetState() != STATE_DONTKNOW )
        {
            aBtnLight2.SetState( STATE_DONTKNOW );
            bUpdate = TRUE;
        }
    }
    // Licht 2 (Richtung)
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_LIGHTDIRECTION_2, FALSE, &pItem ) )
    {
        aVector = ( ( const SvxVector3DItem* )pItem )->GetValue();
        Vector3D aVector2 = pLightGroup->GetDirection( Base3DLight1 );
        if( aVector != aVector2 )
        {
            pLightGroup->SetDirection( aVector, Base3DLight1 );
            bUpdate = TRUE;
        }
    }

    // Licht 3 (Farbe)
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_LIGHTCOLOR_3, FALSE, &pItem ) )
    {
        aColor = ( ( const SvxColorItem* )pItem )->GetValue();
        ColorLB* pLb = &aLbLight3;
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );
            pLightGroup->SetIntensity( aColor, Base3DMaterialDiffuse, Base3DLight2 );

            bUpdate = TRUE;
        }
    }
    else
    {
        if( aLbLight3.GetSelectEntryCount() != 0 )
        {
            aLbLight3.SetNoSelection();
            bUpdate = TRUE;
        }
    }
    // Licht 3 (an/aus)
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_LIGHTON_3, FALSE, &pItem ) )
    {
        BOOL bOn = ( ( const SfxBoolItem* )pItem )->GetValue();
        if( ( bOn && aBtnLight3.GetImage() != aImgLightOn ) ||
            ( !bOn && aBtnLight3.GetImage() != aImgLightOff ) )
        {
            aBtnLight3.SetImage( bOn ? aImgLightOn : aImgLightOff );
            pLightGroup->Enable( bOn, Base3DLight2 );
            bUpdate = TRUE;
        }
        if( aBtnLight3.GetState() == STATE_DONTKNOW )
            aBtnLight3.Check( aBtnLight3.IsChecked() );
    }
    else
    {
        if( aBtnLight3.GetState() != STATE_DONTKNOW )
        {
            aBtnLight3.SetState( STATE_DONTKNOW );
            bUpdate = TRUE;
        }
    }
    // Licht 3 (Richtung)
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_LIGHTDIRECTION_3, FALSE, &pItem ) )
    {
        aVector = ( ( const SvxVector3DItem* )pItem )->GetValue();
        Vector3D aVector2 = pLightGroup->GetDirection( Base3DLight2 );
        if( aVector != aVector2 )
        {
            pLightGroup->SetDirection( aVector, Base3DLight2 );
            bUpdate = TRUE;
        }
    }

    // Licht 4 (Farbe)
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_LIGHTCOLOR_4, FALSE, &pItem ) )
    {
        aColor = ( ( const SvxColorItem* )pItem )->GetValue();
        ColorLB* pLb = &aLbLight4;
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );
            pLightGroup->SetIntensity( aColor, Base3DMaterialDiffuse, Base3DLight3 );

            bUpdate = TRUE;
        }
    }
    else
    {
        if( aLbLight4.GetSelectEntryCount() != 0 )
        {
            aLbLight4.SetNoSelection();
            bUpdate = TRUE;
        }
    }
    // Licht 4 (an/aus)
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_LIGHTON_4, FALSE, &pItem ) )
    {
        BOOL bOn = ( ( const SfxBoolItem* )pItem )->GetValue();
        if( ( bOn && aBtnLight4.GetImage() != aImgLightOn ) ||
            ( !bOn && aBtnLight4.GetImage() != aImgLightOff ) )
        {
            aBtnLight4.SetImage( bOn ? aImgLightOn : aImgLightOff );
            pLightGroup->Enable( bOn, Base3DLight3 );
            bUpdate = TRUE;
        }
        if( aBtnLight4.GetState() == STATE_DONTKNOW )
            aBtnLight4.Check( aBtnLight4.IsChecked() );
    }
    else
    {
        if( aBtnLight4.GetState() != STATE_DONTKNOW )
        {
            aBtnLight4.SetState( STATE_DONTKNOW );
            bUpdate = TRUE;
        }
    }
    // Licht 4 (Richtung)
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_LIGHTDIRECTION_4, FALSE, &pItem ) )
    {
        aVector = ( ( const SvxVector3DItem* )pItem )->GetValue();
        Vector3D aVector2 = pLightGroup->GetDirection( Base3DLight3 );
        if( aVector != aVector2 )
        {
            pLightGroup->SetDirection( aVector, Base3DLight3 );
            bUpdate = TRUE;
        }
    }

    // Licht 5 (Farbe)
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_LIGHTCOLOR_5, FALSE, &pItem ) )
    {
        aColor = ( ( const SvxColorItem* )pItem )->GetValue();
        ColorLB* pLb = &aLbLight5;
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );
            pLightGroup->SetIntensity( aColor, Base3DMaterialDiffuse, Base3DLight4 );

            bUpdate = TRUE;
        }
    }
    else
    {
        if( aLbLight5.GetSelectEntryCount() != 0 )
        {
            aLbLight5.SetNoSelection();
            bUpdate = TRUE;
        }
    }
    // Licht 5 (an/aus)
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_LIGHTON_5, FALSE, &pItem ) )
    {
        BOOL bOn = ( ( const SfxBoolItem* )pItem )->GetValue();
        if( ( bOn && aBtnLight5.GetImage() != aImgLightOn ) ||
            ( !bOn && aBtnLight5.GetImage() != aImgLightOff ) )
        {
            aBtnLight5.SetImage( bOn ? aImgLightOn : aImgLightOff );
            pLightGroup->Enable( bOn, Base3DLight4 );
            bUpdate = TRUE;
        }
        if( aBtnLight5.GetState() == STATE_DONTKNOW )
            aBtnLight5.Check( aBtnLight5.IsChecked() );
    }
    else
    {
        if( aBtnLight5.GetState() != STATE_DONTKNOW )
        {
            aBtnLight5.SetState( STATE_DONTKNOW );
            bUpdate = TRUE;
        }
    }
    // Licht 5 (Richtung)
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_LIGHTDIRECTION_5, FALSE, &pItem ) )
    {
        aVector = ( ( const SvxVector3DItem* )pItem )->GetValue();
        Vector3D aVector2 = pLightGroup->GetDirection( Base3DLight4 );
        if( aVector != aVector2 )
        {
            pLightGroup->SetDirection( aVector, Base3DLight4 );
            bUpdate = TRUE;
        }
    }

    // Licht 6 (Farbe)
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_LIGHTCOLOR_6, FALSE, &pItem ) )
    {
        aColor = ( ( const SvxColorItem* )pItem )->GetValue();
        ColorLB* pLb = &aLbLight6;
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );
            pLightGroup->SetIntensity( aColor, Base3DMaterialDiffuse, Base3DLight5 );

            bUpdate = TRUE;
        }
    }
    else
    {
        if( aLbLight6.GetSelectEntryCount() != 0 )
        {
            aLbLight6.SetNoSelection();
            bUpdate = TRUE;
        }
    }
    // Licht 6 (an/aus)
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_LIGHTON_6, FALSE, &pItem ) )
    {
        BOOL bOn = ( ( const SfxBoolItem* )pItem )->GetValue();
        if( ( bOn && aBtnLight6.GetImage() != aImgLightOn ) ||
            ( !bOn && aBtnLight6.GetImage() != aImgLightOff ) )
        {
            aBtnLight6.SetImage( bOn ? aImgLightOn : aImgLightOff );
            pLightGroup->Enable( bOn, Base3DLight5 );
            bUpdate = TRUE;
        }
        if( aBtnLight6.GetState() == STATE_DONTKNOW )
            aBtnLight6.Check( aBtnLight6.IsChecked() );
    }
    else
    {
        if( aBtnLight6.GetState() != STATE_DONTKNOW )
        {
            aBtnLight6.SetState( STATE_DONTKNOW );
            bUpdate = TRUE;
        }
    }
    // Licht 6 (Richtung)
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_LIGHTDIRECTION_6, FALSE, &pItem ) )
    {
        aVector = ( ( const SvxVector3DItem* )pItem )->GetValue();
        Vector3D aVector2 = pLightGroup->GetDirection( Base3DLight5 );
        if( aVector != aVector2 )
        {
            pLightGroup->SetDirection( aVector, Base3DLight5 );
            bUpdate = TRUE;
        }
    }

    // Licht 7 (Farbe)
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_LIGHTCOLOR_7, FALSE, &pItem ) )
    {
        aColor = ( ( const SvxColorItem* )pItem )->GetValue();
        ColorLB* pLb = &aLbLight7;
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );
            pLightGroup->SetIntensity( aColor, Base3DMaterialDiffuse, Base3DLight6 );

            bUpdate = TRUE;
        }
    }
    else
    {
        if( aLbLight7.GetSelectEntryCount() != 0 )
        {
            aLbLight7.SetNoSelection();
            bUpdate = TRUE;
        }
    }
    // Licht 7 (an/aus)
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_LIGHTON_7, FALSE, &pItem ) )
    {
        BOOL bOn = ( ( const SfxBoolItem* )pItem )->GetValue();
        if( ( bOn && aBtnLight7.GetImage() != aImgLightOn ) ||
            ( !bOn && aBtnLight7.GetImage() != aImgLightOff ) )
        {
            aBtnLight7.SetImage( bOn ? aImgLightOn : aImgLightOff );
            pLightGroup->Enable( bOn, Base3DLight6 );
            bUpdate = TRUE;
        }
        if( aBtnLight7.GetState() == STATE_DONTKNOW )
            aBtnLight7.Check( aBtnLight7.IsChecked() );
    }
    else
    {
        if( aBtnLight7.GetState() != STATE_DONTKNOW )
        {
            aBtnLight7.SetState( STATE_DONTKNOW );
            bUpdate = TRUE;
        }
    }
    // Licht 7 (Richtung)
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_LIGHTDIRECTION_7, FALSE, &pItem ) )
    {
        aVector = ( ( const SvxVector3DItem* )pItem )->GetValue();
        Vector3D aVector2 = pLightGroup->GetDirection( Base3DLight6 );
        if( aVector != aVector2 )
        {
            pLightGroup->SetDirection( aVector, Base3DLight6 );
            bUpdate = TRUE;
        }
    }

    // Licht 8 (Farbe)
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_LIGHTCOLOR_8, FALSE, &pItem ) )
    {
        aColor = ( ( const SvxColorItem* )pItem )->GetValue();
        ColorLB* pLb = &aLbLight8;
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );
            pLightGroup->SetIntensity( aColor, Base3DMaterialDiffuse, Base3DLight7 );

            bUpdate = TRUE;
        }
    }
    else
    {
        if( aLbLight8.GetSelectEntryCount() != 0 )
        {
            aLbLight8.SetNoSelection();
            bUpdate = TRUE;
        }
    }
    // Licht 8 (an/aus)
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_LIGHTON_8, FALSE, &pItem ) )
    {
        BOOL bOn = ( ( const SfxBoolItem* )pItem )->GetValue();
        if( ( bOn && aBtnLight8.GetImage() != aImgLightOn ) ||
            ( !bOn && aBtnLight8.GetImage() != aImgLightOff ) )
        {
            aBtnLight8.SetImage( bOn ? aImgLightOn : aImgLightOff );
            pLightGroup->Enable( bOn, Base3DLight7 );
            bUpdate = TRUE;
        }
        if( aBtnLight8.GetState() == STATE_DONTKNOW )
            aBtnLight8.Check( aBtnLight8.IsChecked() );
    }
    else
    {
        if( aBtnLight8.GetState() != STATE_DONTKNOW )
        {
            aBtnLight8.SetState( STATE_DONTKNOW );
            bUpdate = TRUE;
        }
    }
    // Licht 8 (Richtung)
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_LIGHTDIRECTION_8, FALSE, &pItem ) )
    {
        aVector = ( ( const SvxVector3DItem* )pItem )->GetValue();
        Vector3D aVector2 = pLightGroup->GetDirection( Base3DLight7 );
        if( aVector != aVector2 )
        {
            pLightGroup->SetDirection( aVector, Base3DLight7 );
            bUpdate = TRUE;
        }
    }

    // Umgebungslicht
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_AMBIENTCOLOR, FALSE, &pItem ) )
    {
        aColor = ( ( const SvxColorItem* )pItem )->GetValue();
        ColorLB* pLb = &aLbAmbientlight;
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );
            pLightGroup->SetGlobalAmbientLight( aColor );

            bUpdate = TRUE;
        }
    }
    else
    {
        if( aLbAmbientlight.GetSelectEntryCount() != 0 )
        {
            aLbAmbientlight.SetNoSelection();
            bUpdate = TRUE;
        }
    }


// Texturen
    // Art
    if( bBitmap )
    {
        if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_TEXTURE_KIND, FALSE, &pItem ) )
        {
            UINT16 nValue = ( ( const SfxUInt16Item* )pItem )->GetValue();

            if( ( !aBtnTexLuminance.IsChecked() && nValue == 0 ) ||
                ( !aBtnTexColor.IsChecked() && nValue == 1 ) )
            {
                aBtnTexLuminance.Check( nValue == 0 );
                aBtnTexColor.Check( nValue == 1 );
                bUpdate = TRUE;
            }
        }
        else
        {
            if( aBtnTexLuminance.IsChecked() ||
                aBtnTexColor.IsChecked() )
            {
                aBtnTexLuminance.Check( FALSE );
                aBtnTexColor.Check( FALSE );
                bUpdate = TRUE;
            }
        }

        // Modus
        if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_TEXTURE_MODE, FALSE, &pItem ) )
        {
            UINT16 nValue = ( ( const SfxUInt16Item* )pItem )->GetValue();

            if( ( !aBtnTexReplace.IsChecked() && nValue == 0 ) ||
                ( !aBtnTexModulate.IsChecked() && nValue == 1 ) )
            {
                aBtnTexReplace.Check( nValue == 0 );
                aBtnTexModulate.Check( nValue == 1 );
                //aBtnTexBlend.Check( nValue == 2 );
                bUpdate = TRUE;
            }
        }
        else
        {
            if( aBtnTexReplace.IsChecked() ||
                aBtnTexModulate.IsChecked() )
            {
                aBtnTexReplace.Check( FALSE );
                aBtnTexModulate.Check( FALSE );
                //aBtnTexBlend.Check( FALSE );
                bUpdate = TRUE;
            }
        }

        // Projektion X
        if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_TEXTURE_PROJ_X, FALSE, &pItem ) )
        {
            UINT16 nValue = ( ( const SfxUInt16Item* )pItem )->GetValue();

            if( ( !aBtnTexObjectX.IsChecked() && nValue == 0 ) ||
                ( !aBtnTexParallelX.IsChecked() && nValue == 1 ) ||
                ( !aBtnTexCircleX.IsChecked() && nValue == 2 ) )
            {
                aBtnTexObjectX.Check( nValue == 0 );
                aBtnTexParallelX.Check( nValue == 1 );
                aBtnTexCircleX.Check( nValue == 2 );
                bUpdate = TRUE;
            }
        }
        else
        {
            if( aBtnTexObjectX.IsChecked() ||
                aBtnTexParallelX.IsChecked() ||
                aBtnTexCircleX.IsChecked() )
            {
                aBtnTexObjectX.Check( FALSE );
                aBtnTexParallelX.Check( FALSE );
                aBtnTexCircleX.Check( FALSE );
                bUpdate = TRUE;
            }
        }

        // Projektion Y
        if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_TEXTURE_PROJ_Y, FALSE, &pItem ) )
        {
            UINT16 nValue = ( ( const SfxUInt16Item* )pItem )->GetValue();

            if( ( !aBtnTexObjectY.IsChecked() && nValue == 0 ) ||
                ( !aBtnTexParallelY.IsChecked() && nValue == 1 ) ||
                ( !aBtnTexCircleY.IsChecked() && nValue == 2 ) )
            {
                aBtnTexObjectY.Check( nValue == 0 );
                aBtnTexParallelY.Check( nValue == 1 );
                aBtnTexCircleY.Check( nValue == 2 );
                bUpdate = TRUE;
            }
        }
        else
        {
            if( aBtnTexObjectY.IsChecked() ||
                aBtnTexParallelY.IsChecked() ||
                aBtnTexCircleY.IsChecked() )
            {
                aBtnTexObjectY.Check( FALSE );
                aBtnTexParallelY.Check( FALSE );
                aBtnTexCircleY.Check( FALSE );
                bUpdate = TRUE;
            }
        }

        // Filter
        if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_TEXTURE_FILTER, FALSE, &pItem ) )
        {
            BOOL bValue = ( ( const SfxBoolItem* )pItem )->GetValue();
            if( bValue != aBtnTexFilter.IsChecked() )
            {
                aBtnTexFilter.Check( bValue );
                bUpdate = TRUE;
            }
            if( aBtnTexFilter.GetState() == STATE_DONTKNOW )
                aBtnTexFilter.Check( bValue );
        }
        else
        {
            if( aBtnTexFilter.GetState() != STATE_DONTKNOW )
            {
                aBtnTexFilter.SetState( STATE_DONTKNOW );
                bUpdate = TRUE;
            }
        }
    }


// Material
    // Favoriten
    //aLbMatFavorites.SelectEntryPos( 0 );

    // Objektfarbe
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_MAT_COLOR, FALSE, &pItem ) )
    {
        aColor = ( ( const SvxColorItem* )pItem )->GetValue();
        aCtlLightPreview.GetPreviewControl().SetMaterial( aColor, Base3DMaterialDiffuse );
        ColorLB* pLb = &aLbMatColor;
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );
            bUpdate = TRUE;
        }
    }
    else
    {
        if( aLbMatColor.GetSelectEntryCount() != 0 )
        {
            aLbMatColor.SetNoSelection();
            bUpdate = TRUE;
        }
    }

    // Slebstleuchtfarbe
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_MAT_EMISSION, FALSE, &pItem ) )
    {
        aColor = ( ( const SvxColorItem* )pItem )->GetValue();
        aCtlLightPreview.GetPreviewControl().SetMaterial( aColor, Base3DMaterialEmission );
        ColorLB* pLb = &aLbMatEmission;
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );
            bUpdate = TRUE;
        }
    }
    else
    {
        if( aLbMatEmission.GetSelectEntryCount() != 0 )
        {
            aLbMatEmission.SetNoSelection();
            bUpdate = TRUE;
        }
    }

    // Glanzpunkt
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_MAT_SPECULAR, FALSE, &pItem ) )
    {
        aColor = ( ( const SvxColorItem* )pItem )->GetValue();
        aCtlLightPreview.GetPreviewControl().SetMaterial( aColor, Base3DMaterialSpecular );
        ColorLB* pLb = &aLbMatSpecular;
        if( aColor != pLb->GetSelectEntryColor() )
        {
            LBSelectColor( pLb, aColor );
            bUpdate = TRUE;
        }
    }
    else
    {
        if( aLbMatSpecular.GetSelectEntryCount() != 0 )
        {
            aLbMatSpecular.SetNoSelection();
            bUpdate = TRUE;
        }
    }

    // Glanzpunkt Intensitaet
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_MAT_SPECULAR_INTENSITY, FALSE, &pItem ) )
    {
        UINT16 nValue = ( ( const SfxUInt16Item* )pItem )->GetValue();
        aCtlLightPreview.GetPreviewControl().SetShininess( nValue );
        if( nValue != aMtrMatSpecularIntensity.GetValue() )
        {
            aMtrMatSpecularIntensity.SetValue( nValue );
            bUpdate = TRUE;
        }
    }
    else
    {
        if( !aMtrMatSpecularIntensity.IsEmptyFieldValue() )
        {
            aMtrMatSpecularIntensity.SetEmptyFieldValue();
            bUpdate = TRUE;
        }
    }


// Sonstige
    // Perspektive
    if( SFX_ITEM_SET == rAttrs.GetItemState( SID_ATTR_3D_PERSPECTIVE, FALSE, &pItem ) )
    {
        ProjectionType ePT = (ProjectionType) ( ( const SfxUInt16Item* )pItem )->GetValue();
        if( ( !aBtnPerspective.IsChecked() && ePT == PR_PERSPECTIVE ) ||
            ( aBtnPerspective.IsChecked() && ePT == PR_PARALLEL ) )
        {
            aBtnPerspective.Check( ePT == PR_PERSPECTIVE );
            bUpdate = TRUE;
        }
        if( aBtnPerspective.GetState() == STATE_DONTKNOW )
            aBtnPerspective.Check( ePT == PR_PERSPECTIVE );
    }
    else
    {
        if( aBtnPerspective.GetState() != STATE_DONTKNOW )
        {
            aBtnPerspective.SetState( STATE_DONTKNOW );
            bUpdate = TRUE;
        }
    }

    if( !bUpdate && !bOnly3DChanged )
    {
        // Eventuell sind aber die 2D-Attribute unterschiedlich. Vergleiche
        // diese und entscheide


        bUpdate = TRUE;
    }

    if( bUpdate || bOnly3DChanged )
    {
        // Preview updaten
        aCtlLightPreview.GetPreviewControl().SetLightGroup( pLightGroup );
        aCtlPreview.Set3DAttributes( rAttrs );
    }

    // handle state of converts possible
    aBtnConvertTo3D.Enable(pConvertTo3DItem->GetState());
    aBtnLatheObject.Enable(pConvertTo3DLatheItem->GetState());
}

// -----------------------------------------------------------------------
void Svx3DWin::GetAttr( SfxItemSet& rAttrs )
{
// 2D-Attribute und alle anderen holen
    rAttrs.Put( aCtlPreview.Get3DAttributes() );

// Sonstige, muss vorne stehen da auf allen Seiten
    // Perspektive
    if( aBtnPerspective.GetState() != STATE_DONTKNOW )
    {
        UINT16 nValue;
        if( aBtnPerspective.IsChecked() )
            nValue = PR_PERSPECTIVE;
        else
            nValue = PR_PARALLEL;
        rAttrs.Put( SfxUInt16Item( SID_ATTR_3D_PERSPECTIVE, nValue ) );
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_PERSPECTIVE );

// Geometrie
    // evtl. PoolUnit ermitteln (Falls dies in Update() nicht passiert ist)
    if( !pPool )
    {
        DBG_ERROR( "Kein Pool in GetAttr()! Evtl. inkompatibel zu drviewsi.cxx ?" );
        pPool = rAttrs.GetPool();
        DBG_ASSERT( pPool, "Wo ist der Pool?" );
        ePoolUnit = pPool->GetMetric( SID_ATTR_LINE_WIDTH );

        eFUnit = GetModuleFieldUnit();
    }

    // Anzahl Segmente (horizontal)
    if( !aNumHorizontal.IsEmptyFieldValue() )
    {
        UINT32 nValue = aNumHorizontal.GetValue();
        rAttrs.Put( SfxUInt32Item( SID_ATTR_3D_HORZ_SEGS, nValue ) );
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_HORZ_SEGS );

    // Anzahl Segmente (vertikal)
    if( !aNumVertical.IsEmptyFieldValue() )
    {
        UINT32 nValue = aNumVertical.GetValue();
        rAttrs.Put( SfxUInt32Item( SID_ATTR_3D_VERT_SEGS, nValue ) );
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_VERT_SEGS );

    // Tiefe
    if( !aMtrDepth.IsEmptyFieldValue() )
    {
        UINT32 nValue = GetCoreValue( aMtrDepth, ePoolUnit );
        rAttrs.Put( SfxUInt32Item( SID_ATTR_3D_DEPTH, nValue ) );
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_DEPTH );

    // Doppelseitig
    TriState eState = aBtnDoubleSided.GetState();
    if( eState != STATE_DONTKNOW )
    {
        BOOL bValue = STATE_CHECK == eState;
        rAttrs.Put( SfxBoolItem( SID_ATTR_3D_DOUBLE_SIDED, bValue ) );
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_DOUBLE_SIDED );

    // Kantenrundung
    if( !aMtrPercentDiagonal.IsEmptyFieldValue() )
    {
        UINT16 nValue = (UINT16) aMtrPercentDiagonal.GetValue();
        rAttrs.Put( SfxUInt16Item( SID_ATTR_3D_PERCENT_DIAGONAL, nValue ) );
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_PERCENT_DIAGONAL );

    // Tiefenskalierung
    if( !aMtrBackscale.IsEmptyFieldValue() )
    {
        UINT16 nValue = (UINT16) aMtrBackscale.GetValue();
        rAttrs.Put( SfxUInt16Item( SID_ATTR_3D_BACKSCALE, nValue ) );
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_BACKSCALE );

    // Endwinkel
    if( !aMtrEndAngle.IsEmptyFieldValue() )
    {
        UINT16 nValue = (UINT16) aMtrEndAngle.GetValue();
        rAttrs.Put( SfxUInt16Item( SID_ATTR_3D_END_ANGLE, nValue ) );
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_END_ANGLE );

    // Normalentyp
    UINT16 nValue = 99;
    if( aBtnNormalsObj.IsChecked() )
        nValue = 0;
    else if( aBtnNormalsFlat.IsChecked() )
        nValue = 1;
    else if( aBtnNormalsSphere.IsChecked() )
        nValue = 2;

    if( nValue <= 2 )
        rAttrs.Put( SfxUInt16Item( SID_ATTR_3D_NORMALS_KIND, nValue ) );
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_NORMALS_KIND );

    // Normalen invertieren
    eState = aBtnNormalsInvert.GetState();
    if( eState != STATE_DONTKNOW )
    {
        BOOL bValue = STATE_CHECK == eState;
        rAttrs.Put( SfxBoolItem( SID_ATTR_3D_NORMALS_INVERT, bValue ) );
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_NORMALS_INVERT );

    // 2-seitige Beleuchtung
    eState = aBtnTwoSidedLighting.GetState();
    if( eState != STATE_DONTKNOW )
    {
        BOOL bValue = STATE_CHECK == eState;
        rAttrs.Put( SfxBoolItem( SID_ATTR_3D_TWO_SIDED_LIGHTING, bValue ) );
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_TWO_SIDED_LIGHTING );

// Darstellung
    // Shademode
    if( aLbShademode.GetSelectEntryCount() )
    {
        nValue = aLbShademode.GetSelectEntryPos();
        rAttrs.Put( SfxUInt16Item( SID_ATTR_3D_SHADE_MODE, nValue ) );
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_SHADE_MODE );

    // 3D-Shatten
    eState = aBtnShadow3d.GetState();
    if( eState != STATE_DONTKNOW )
    {
        BOOL bValue = STATE_CHECK == eState;
        rAttrs.Put( SfxBoolItem( SID_ATTR_3D_SHADOW_3D, bValue ) );
        rAttrs.Put( SdrShadowItem( bValue ) );
    }
    else
    {
        rAttrs.InvalidateItem( SID_ATTR_3D_SHADOW_3D );
        rAttrs.InvalidateItem( SDRATTR_SHADOW );
    }

    // Neigung (Schatten)
    if( !aMtrSlant.IsEmptyFieldValue() )
    {
        UINT16 nValue = (UINT16) aMtrSlant.GetValue();
        rAttrs.Put( SfxUInt16Item( SID_ATTR_3D_SHADOW_SLANT, nValue ) );
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_SHADOW_SLANT );

    // Distanz
    if( !aMtrDistance.IsEmptyFieldValue() )
    {
        UINT32 nValue = GetCoreValue( aMtrDistance, ePoolUnit );
        rAttrs.Put( SfxUInt32Item( SID_ATTR_3D_DISTANCE, nValue ) );
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_DISTANCE );

    // Brennweite
    if( !aMtrFocalLength.IsEmptyFieldValue() )
    {
        UINT32 nValue = GetCoreValue( aMtrFocalLength, ePoolUnit );
        rAttrs.Put( SfxUInt32Item( SID_ATTR_3D_FOCAL_LENGTH, nValue ) );
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_FOCAL_LENGTH );

// Beleuchtung
    Image aImg;
    Vector3D aVector;
    Color aColor;
    // Licht 1 Farbe
    if( aLbLight1.GetSelectEntryCount() )
    {
        aColor = aLbLight1.GetSelectEntryColor();
        rAttrs.Put( SvxColorItem( aColor, SID_ATTR_3D_LIGHTCOLOR_1 ) );
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_LIGHTCOLOR_1 );
    // Licht 1 (an/aus)
    eState = aBtnLight1.GetState();
    if( eState != STATE_DONTKNOW )
    {
        aImg = aBtnLight1.GetImage();
        BOOL bValue = FALSE;
        if( aImg == aImgLightOn )
            bValue = TRUE;
        rAttrs.Put( SfxBoolItem( SID_ATTR_3D_LIGHTON_1, bValue ) );

        // Licht 1 (Richtung)
        if( bValue )
        {
            aVector = pLightGroup->GetDirection( Base3DLight0 );
            rAttrs.Put( SvxVector3DItem( SID_ATTR_3D_LIGHTDIRECTION_1, aVector ) );
        }
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_LIGHTON_1 );


    // Licht 2 Farbe
    if( aLbLight2.GetSelectEntryCount() )
    {
        aColor = aLbLight2.GetSelectEntryColor();
        rAttrs.Put( SvxColorItem( aColor, SID_ATTR_3D_LIGHTCOLOR_2 ) );
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_LIGHTCOLOR_2 );
    // Licht 2 (an/aus)
    eState = aBtnLight2.GetState();
    if( eState != STATE_DONTKNOW )
    {
        aImg = aBtnLight2.GetImage();
        BOOL bValue = FALSE;
        if( aImg == aImgLightOn )
            bValue = TRUE;
        rAttrs.Put( SfxBoolItem( SID_ATTR_3D_LIGHTON_2, bValue ) );

        // Licht 2 (Richtung)
        if( bValue )
        {
            aVector = pLightGroup->GetDirection( Base3DLight1 );
            rAttrs.Put( SvxVector3DItem( SID_ATTR_3D_LIGHTDIRECTION_2, aVector ) );
        }
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_LIGHTON_2 );

    // Licht 3 Farbe
    if( aLbLight3.GetSelectEntryCount() )
    {
        aColor = aLbLight3.GetSelectEntryColor();
        rAttrs.Put( SvxColorItem( aColor, SID_ATTR_3D_LIGHTCOLOR_3 ) );
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_LIGHTCOLOR_3 );
    // Licht 3 (an/aus)
    eState = aBtnLight3.GetState();
    if( eState != STATE_DONTKNOW )
    {
        aImg = aBtnLight3.GetImage();
        BOOL bValue = FALSE;
        if( aImg == aImgLightOn )
            bValue = TRUE;
        rAttrs.Put( SfxBoolItem( SID_ATTR_3D_LIGHTON_3, bValue ) );

        // Licht 3 (Richtung)
        if( bValue )
        {
            aVector = pLightGroup->GetDirection( Base3DLight2 );
            rAttrs.Put( SvxVector3DItem( SID_ATTR_3D_LIGHTDIRECTION_3, aVector ) );
        }
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_LIGHTON_3 );

    // Licht 4 Farbe
    if( aLbLight4.GetSelectEntryCount() )
    {
        aColor = aLbLight4.GetSelectEntryColor();
        rAttrs.Put( SvxColorItem( aColor, SID_ATTR_3D_LIGHTCOLOR_4 ) );
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_LIGHTCOLOR_4 );
    // Licht 4 (an/aus)
    eState = aBtnLight4.GetState();
    if( eState != STATE_DONTKNOW )
    {
        aImg = aBtnLight4.GetImage();
        BOOL bValue = FALSE;
        if( aImg == aImgLightOn )
            bValue = TRUE;
        rAttrs.Put( SfxBoolItem( SID_ATTR_3D_LIGHTON_4, bValue ) );

        // Licht 4 (Richtung)
        if( bValue )
        {
            aVector = pLightGroup->GetDirection( Base3DLight3 );
            rAttrs.Put( SvxVector3DItem( SID_ATTR_3D_LIGHTDIRECTION_4, aVector ) );
        }
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_LIGHTON_4 );

    // Licht 5 Farbe
    if( aLbLight5.GetSelectEntryCount() )
    {
        aColor = aLbLight5.GetSelectEntryColor();
        rAttrs.Put( SvxColorItem( aColor, SID_ATTR_3D_LIGHTCOLOR_5 ) );
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_LIGHTCOLOR_5 );
    // Licht 5 (an/aus)
    eState = aBtnLight5.GetState();
    if( eState != STATE_DONTKNOW )
    {
        aImg = aBtnLight5.GetImage();
        BOOL bValue = FALSE;
        if( aImg == aImgLightOn )
            bValue = TRUE;
        rAttrs.Put( SfxBoolItem( SID_ATTR_3D_LIGHTON_5, bValue ) );

        // Licht 5 (Richtung)
        if( bValue )
        {
            aVector = pLightGroup->GetDirection( Base3DLight4 );
            rAttrs.Put( SvxVector3DItem( SID_ATTR_3D_LIGHTDIRECTION_5, aVector ) );
        }
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_LIGHTON_5 );

    // Licht 6 Farbe
    if( aLbLight6.GetSelectEntryCount() )
    {
        aColor = aLbLight6.GetSelectEntryColor();
        rAttrs.Put( SvxColorItem( aColor, SID_ATTR_3D_LIGHTCOLOR_6 ) );
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_LIGHTCOLOR_6 );
    // Licht 6 (an/aus)
    eState = aBtnLight6.GetState();
    if( eState != STATE_DONTKNOW )
    {
        aImg = aBtnLight6.GetImage();
        BOOL bValue = FALSE;
        if( aImg == aImgLightOn )
            bValue = TRUE;
        rAttrs.Put( SfxBoolItem( SID_ATTR_3D_LIGHTON_6, bValue ) );

        // Licht 6 (Richtung)
        if( bValue )
        {
            aVector = pLightGroup->GetDirection( Base3DLight5 );
            rAttrs.Put( SvxVector3DItem( SID_ATTR_3D_LIGHTDIRECTION_6, aVector ) );
        }
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_LIGHTON_6 );

    // Licht 7 Farbe
    if( aLbLight7.GetSelectEntryCount() )
    {
        aColor = aLbLight7.GetSelectEntryColor();
        rAttrs.Put( SvxColorItem( aColor, SID_ATTR_3D_LIGHTCOLOR_7 ) );
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_LIGHTCOLOR_7 );
    // Licht 7 (an/aus)
    eState = aBtnLight7.GetState();
    if( eState != STATE_DONTKNOW )
    {
        aImg = aBtnLight7.GetImage();
        BOOL bValue = FALSE;
        if( aImg == aImgLightOn )
            bValue = TRUE;
        rAttrs.Put( SfxBoolItem( SID_ATTR_3D_LIGHTON_7, bValue ) );

        // Licht 7 (Richtung)
        if( bValue )
        {
            aVector = pLightGroup->GetDirection( Base3DLight6 );
            rAttrs.Put( SvxVector3DItem( SID_ATTR_3D_LIGHTDIRECTION_7, aVector ) );
        }
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_LIGHTON_7 );

    // Licht 8 Farbe
    if( aLbLight8.GetSelectEntryCount() )
    {
        aColor = aLbLight8.GetSelectEntryColor();
        rAttrs.Put( SvxColorItem( aColor, SID_ATTR_3D_LIGHTCOLOR_8 ) );
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_LIGHTCOLOR_8 );
    // Licht 8 (an/aus)
    eState = aBtnLight8.GetState();
    if( eState != STATE_DONTKNOW )
    {
        aImg = aBtnLight8.GetImage();
        BOOL bValue = FALSE;
        if( aImg == aImgLightOn )
            bValue = TRUE;
        rAttrs.Put( SfxBoolItem( SID_ATTR_3D_LIGHTON_8, bValue ) );

        // Licht 8 (Richtung)
        if( bValue )
        {
            aVector = pLightGroup->GetDirection( Base3DLight7 );
            rAttrs.Put( SvxVector3DItem( SID_ATTR_3D_LIGHTDIRECTION_8, aVector ) );
        }
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_LIGHTON_8 );

    // Umgebungslicht
    if( aLbAmbientlight.GetSelectEntryCount() )
    {
        aColor = aLbAmbientlight.GetSelectEntryColor();
        rAttrs.Put( SvxColorItem( aColor, SID_ATTR_3D_AMBIENTCOLOR ) );
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_AMBIENTCOLOR );

// Texturen
    // Art
    nValue = 99;
    if( aBtnTexLuminance.IsChecked() )
        nValue = 0;
    else if( aBtnTexColor.IsChecked() )
        nValue = 1;

    if( nValue <= 1 )
        rAttrs.Put( SfxUInt16Item( SID_ATTR_3D_TEXTURE_KIND, nValue ) );
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_TEXTURE_KIND );


    // Modus
    nValue = 99;
    if( aBtnTexReplace.IsChecked() )
        nValue = 0;
    else if( aBtnTexModulate.IsChecked() )
        nValue = 1;
    //else if( aBtnTexBlend.IsChecked() )
    //  nValue = 2;

    if( nValue <= 2 )
        rAttrs.Put( SfxUInt16Item( SID_ATTR_3D_TEXTURE_MODE, nValue ) );
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_TEXTURE_MODE );

    // Projektion X
    nValue = 99;
    if( aBtnTexObjectX.IsChecked() )
        nValue = 0;
    else if( aBtnTexParallelX.IsChecked() )
        nValue = 1;
    else if( aBtnTexCircleX.IsChecked() )
        nValue = 2;

    if( nValue <= 2 )
        rAttrs.Put( SfxUInt16Item( SID_ATTR_3D_TEXTURE_PROJ_X, nValue ) );
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_TEXTURE_PROJ_X );

    // Projektion Y
    nValue = 99;
    if( aBtnTexObjectY.IsChecked() )
        nValue = 0;
    else if( aBtnTexParallelY.IsChecked() )
        nValue = 1;
    else if( aBtnTexCircleY.IsChecked() )
        nValue = 2;

    if( nValue <= 2 )
        rAttrs.Put( SfxUInt16Item( SID_ATTR_3D_TEXTURE_PROJ_Y, nValue ) );
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_TEXTURE_PROJ_Y );


    // Filter
    eState = aBtnTexFilter.GetState();
    if( eState != STATE_DONTKNOW )
    {
        BOOL bValue = STATE_CHECK == eState;
        rAttrs.Put( SfxBoolItem( SID_ATTR_3D_TEXTURE_FILTER, bValue ) );
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_TEXTURE_FILTER );


// Material
    // Objektfarbe
    if( aLbMatColor.GetSelectEntryCount() )
    {
        aColor = aLbMatColor.GetSelectEntryColor();
        rAttrs.Put( SvxColorItem( aColor, SID_ATTR_3D_MAT_COLOR ) );
    }
    else
    {
        rAttrs.InvalidateItem( SID_ATTR_3D_MAT_COLOR );
        rAttrs.InvalidateItem( XATTR_FILLCOLOR );
    }

    // Slebstleuchtfarbe
    if( aLbMatEmission.GetSelectEntryCount() )
    {
        aColor = aLbMatEmission.GetSelectEntryColor();
        rAttrs.Put( SvxColorItem( aColor, SID_ATTR_3D_MAT_EMISSION ) );
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_MAT_EMISSION );

    // Glanzpunkt
    if( aLbMatSpecular.GetSelectEntryCount() )
    {
        aColor = aLbMatSpecular.GetSelectEntryColor();
        rAttrs.Put( SvxColorItem( aColor, SID_ATTR_3D_MAT_SPECULAR ) );
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_MAT_SPECULAR );

    // Glanzpunkt Intensitaet
    if( !aMtrMatSpecularIntensity.IsEmptyFieldValue() )
    {
        UINT16 nValue = (UINT16) aMtrMatSpecularIntensity.GetValue();
        rAttrs.Put( SfxUInt16Item( SID_ATTR_3D_MAT_SPECULAR_INTENSITY, nValue ) );
    }
    else
        rAttrs.InvalidateItem( SID_ATTR_3D_MAT_SPECULAR_INTENSITY );
}

// -----------------------------------------------------------------------
void __EXPORT Svx3DWin::Resize()
{
    if ( !IsFloatingMode() ||
         !GetFloatingWindow()->IsRollUp() )
    {
        Size aWinSize( GetOutputSizePixel() ); // vorher rSize im Resizing()

        if( aWinSize.Height() >= GetMinOutputSizePixel().Height() &&
            aWinSize.Width() >= GetMinOutputSizePixel().Width() )
        {
            Size aDiffSize;
            aDiffSize.Width() = aWinSize.Width() - aSize.Width();
            aDiffSize.Height() = aWinSize.Height() - aSize.Height();

            Point aXPt;
            Point aYPt;
            aXPt.X() = aDiffSize.Width();
            aYPt.Y() = aDiffSize.Height();

            Size aObjSize;

            // Hide
            aBtnUpdate.Hide();
            aBtnAssign.Hide();

            aBtnConvertTo3D.Hide();
            aBtnLatheObject.Hide();
            aBtnPerspective.Hide();

            aCtlPreview.Hide();
            aCtlLightPreview.Hide();
            aGrpPreview.Hide();

            aGrpFavorites.Hide();
            aGrpGeometrie.Hide();
            aGrpRepresentation.Hide();
            aGrpLight.Hide();
            aGrpTexture.Hide();
            aGrpMaterial.Hide();

            // Verschieben / Resizen
            aBtnUpdate.SetPosPixel( aBtnUpdate.GetPosPixel() + aXPt );
            aBtnAssign.SetPosPixel( aBtnAssign.GetPosPixel() + aXPt );

                // Preview-Controls
            aObjSize = aCtlPreview.GetOutputSizePixel();
            aObjSize.Width() += aDiffSize.Width();
            aObjSize.Height() += aDiffSize.Height();
            aCtlPreview.SetOutputSizePixel( aObjSize );
            aCtlLightPreview.SetOutputSizePixel( aObjSize );

                // Preview-Group
            aObjSize = aGrpPreview.GetOutputSizePixel();
            aObjSize.Width() += aDiffSize.Width();
            aObjSize.Height() += aDiffSize.Height();
            aGrpPreview.SetOutputSizePixel( aObjSize );

                // Groups
            aObjSize = aGrpFavorites.GetOutputSizePixel();
            aObjSize.Width() += aDiffSize.Width();
            aGrpFavorites.SetOutputSizePixel( aObjSize );
            aGrpGeometrie.SetOutputSizePixel( aObjSize );
            aGrpRepresentation.SetOutputSizePixel( aObjSize );
            aGrpLight.SetOutputSizePixel( aObjSize );
            aGrpTexture.SetOutputSizePixel( aObjSize );
            aGrpMaterial.SetOutputSizePixel( aObjSize );

                // Y-Position der unteren Buttons
            aBtnConvertTo3D.SetPosPixel( aBtnConvertTo3D.GetPosPixel() + aYPt );
            aBtnLatheObject.SetPosPixel( aBtnLatheObject.GetPosPixel() + aYPt );
            aBtnPerspective.SetPosPixel( aBtnPerspective.GetPosPixel() + aYPt );

            // Show
            aBtnUpdate.Show();
            aBtnAssign.Show();

            aBtnConvertTo3D.Show();
            aBtnLatheObject.Show();
            aBtnPerspective.Show();

            aGrpPreview.Show();

            if( aBtnFavorites.IsChecked() )
                ClickViewTypeHdl( &aBtnFavorites );
            if( aBtnGeo.IsChecked() )
                ClickViewTypeHdl( &aBtnGeo );
            if( aBtnRepresentation.IsChecked() )
                ClickViewTypeHdl( &aBtnRepresentation );
            if( aBtnLight.IsChecked() )
                ClickViewTypeHdl( &aBtnLight );
            if( aBtnTexture.IsChecked() )
                ClickViewTypeHdl( &aBtnTexture );
            if( aBtnMaterial.IsChecked() )
                ClickViewTypeHdl( &aBtnMaterial );

            aSize = aWinSize;
        }
    }

    SfxDockingWindow::Resize();
}

// -----------------------------------------------------------------------
IMPL_LINK( Svx3DWin, ClickUpdateHdl, void *, EMPTYARG )
{
    bUpdate = !aBtnUpdate.IsChecked();
    aBtnUpdate.Check( bUpdate );

    if( bUpdate )
    {
        SfxBoolItem aItem( SID_3D_STATE, TRUE );
        SfxViewFrame::Current()->GetBindings().GetDispatcher()->Execute(
            SID_3D_STATE, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L );
    }
    else
    {
        // Controls koennen u.U. disabled sein
        /*
        aFtSegments.Enable();
        aNumHorizontal.Enable();
        aNumVertical.Enable();
        */
    }

    return( 0L );
}

// -----------------------------------------------------------------------
IMPL_LINK( Svx3DWin, ClickAssignHdl, void *, EMPTYARG )
{
    SfxBoolItem aItem( SID_3D_ASSIGN, TRUE );
    SfxViewFrame::Current()->GetBindings().GetDispatcher()->Execute(
        SID_3D_ASSIGN, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L );

    return( 0L );
}

// -----------------------------------------------------------------------

IMPL_LINK( Svx3DWin, ClickViewTypeHdl, void *, pBtn )
{

    if( pBtn )
    {
        // Da das permanente Updaten der Preview zu teuer waere
        BOOL bUpdatePreview = aBtnLight.IsChecked();

        aBtnFavorites.Check( &aBtnFavorites == pBtn );
        aBtnGeo.Check( &aBtnGeo == pBtn );
        aBtnRepresentation.Check( &aBtnRepresentation == pBtn );
        aBtnLight.Check( &aBtnLight == pBtn );
        aBtnTexture.Check( &aBtnTexture == pBtn );
        aBtnMaterial.Check( &aBtnMaterial == pBtn );

        if( aBtnFavorites.IsChecked() )
            eViewType = VIEWTYPE_FAVORITES;
        if( aBtnGeo.IsChecked() )
            eViewType = VIEWTYPE_GEO;
        if( aBtnRepresentation.IsChecked() )
            eViewType = VIEWTYPE_REPRESENTATION;
        if( aBtnLight.IsChecked() )
            eViewType = VIEWTYPE_LIGHT;
        if( aBtnTexture.IsChecked() )
            eViewType = VIEWTYPE_TEXTURE;
        if( aBtnMaterial.IsChecked() )
            eViewType = VIEWTYPE_MATERIAL;

        // Favoriten
        if( eViewType == VIEWTYPE_FAVORITES )
        {
            aCtlFavorites.Show();
            aGrpFavorites.Show();
            aBtnOnly3D.Show();
            aBtnAllAttributes.Show();
        }
        else
        {
            aCtlFavorites.Hide();
            aGrpFavorites.Hide();
            aBtnOnly3D.Hide();
            aBtnAllAttributes.Hide();
        }

        // Geometrie
        if( eViewType == VIEWTYPE_GEO )
        {
            aFtHorizontal.Show();
            aNumHorizontal.Show();
            aFtVertical.Show();
            aNumVertical.Show();
            aGrpSegments.Show();
            aFtPercentDiagonal.Show();
            aMtrPercentDiagonal.Show();
            aFtBackscale.Show();
            aMtrBackscale.Show();
            aFtEndAngle.Show();
            aMtrEndAngle.Show();
            aFtDepth.Show();
            aMtrDepth.Show();
            aGrpGeometrie.Show();

            aBtnNormalsObj.Show();
            aBtnNormalsFlat.Show();
            aBtnNormalsSphere.Show();
            aBtnTwoSidedLighting.Show();
            aBtnNormalsInvert.Show();
            aGrpNormals.Show();
            aBtnDoubleSided.Show();
        }
        else
        {
            aFtHorizontal.Hide();
            aNumHorizontal.Hide();
            aFtVertical.Hide();
            aNumVertical.Hide();
            aGrpSegments.Hide();
            aFtPercentDiagonal.Hide();
            aMtrPercentDiagonal.Hide();
            aFtBackscale.Hide();
            aMtrBackscale.Hide();
            aFtEndAngle.Hide();
            aMtrEndAngle.Hide();
            aFtDepth.Hide();
            aMtrDepth.Hide();
            aGrpGeometrie.Hide();

            aBtnNormalsObj.Hide();
            aBtnNormalsFlat.Hide();
            aBtnNormalsSphere.Hide();
            aBtnTwoSidedLighting.Hide();
            aBtnNormalsInvert.Hide();
            aGrpNormals.Hide();
            aBtnDoubleSided.Hide();
        }

        // Darstellung
        if( eViewType == VIEWTYPE_REPRESENTATION )
        {
            aFtShademode.Show();
            aLbShademode.Show();
            aBtnShadow3d.Show();
            aFtSlant.Show();
            aMtrSlant.Show();
            aGrpShadow.Show();
            aFtDistance.Show();
            aMtrDistance.Show();
            aFtFocalLeng.Show();
            aMtrFocalLength.Show();
            aGrpCamera.Show();
            aGrpRepresentation.Show();
        }
        else
        {
            aFtShademode.Hide();
            aLbShademode.Hide();
            aBtnShadow3d.Hide();
            aFtSlant.Hide();
            aMtrSlant.Hide();
            aGrpShadow.Hide();
            aFtDistance.Hide();
            aMtrDistance.Hide();
            aFtFocalLeng.Hide();
            aMtrFocalLength.Hide();
            aGrpCamera.Hide();
            aGrpRepresentation.Hide();
        }

        // Beleuchtung
        if( eViewType == VIEWTYPE_LIGHT )
        {
            aBtnLight1.Show();
            aBtnLight2.Show();
            aBtnLight3.Show();
            aBtnLight4.Show();
            aBtnLight5.Show();
            aBtnLight6.Show();
            aBtnLight7.Show();
            aBtnLight8.Show();
            //aLbLight1.Show();
            aBtnLightColor.Show();
            aGrpLightsource.Show();
            aLbAmbientlight.Show();
            aBtnAmbientColor.Show();
            aGrpAmbientlight.Show();
            aGrpLight.Show();
            //aFtLightX.Show();
            //aFtLightY.Show();
            //aFtLightZ.Show();
            //aGrpLightInfo.Show();

            ColorLB* pLb = GetLbByButton();
            if( pLb )
                pLb->Show();

            aCtlLightPreview.Show();
            aCtlPreview.Hide();
        }
        else
        {
            aBtnLight1.Hide();
            aBtnLight2.Hide();
            aBtnLight3.Hide();
            aBtnLight4.Hide();
            aBtnLight5.Hide();
            aBtnLight6.Hide();
            aBtnLight7.Hide();
            aBtnLight8.Hide();
            aLbLight1.Hide();
            aLbLight2.Hide();
            aLbLight3.Hide();
            aLbLight4.Hide();
            aLbLight5.Hide();
            aLbLight6.Hide();
            aLbLight7.Hide();
            aLbLight8.Hide();
            aBtnLightColor.Hide();
            aGrpLightsource.Hide();
            aLbAmbientlight.Hide();
            aBtnAmbientColor.Hide();
            aGrpAmbientlight.Hide();
            aGrpLight.Hide();
            aFtLightX.Hide();
            aFtLightY.Hide();
            aFtLightZ.Hide();
            aGrpLightInfo.Hide();

            if( !aCtlPreview.IsVisible() )
            {
                aCtlPreview.Show();
                aCtlLightPreview.Hide();
            }
        }

        // Texturen
        if( eViewType == VIEWTYPE_TEXTURE )
        {
            aFtTexKind.Show();
            aBtnTexLuminance.Show();
            aBtnTexColor.Show();
            aFtTexMode.Show();
            aBtnTexReplace.Show();
            aBtnTexModulate.Show();
            //aBtnTexBlend.Show();
            aFtTexProjectionX.Show();
            aBtnTexParallelX.Show();
            aBtnTexCircleX.Show();
            aBtnTexObjectX.Show();
            aFtTexProjectionY.Show();
            aBtnTexParallelY.Show();
            aBtnTexCircleY.Show();
            aBtnTexObjectY.Show();
            aFtTexFilter.Show();
            aBtnTexFilter.Show();
            aGrpTexture.Show();
        }
        else
        {
            aFtTexKind.Hide();
            aBtnTexLuminance.Hide();
            aBtnTexColor.Hide();
            aFtTexMode.Hide();
            aBtnTexReplace.Hide();
            aBtnTexModulate.Hide();
            aBtnTexBlend.Hide();
            aFtTexProjectionX.Hide();
            aBtnTexParallelX.Hide();
            aBtnTexCircleX.Hide();
            aBtnTexObjectX.Hide();
            aFtTexProjectionY.Hide();
            aBtnTexParallelY.Hide();
            aBtnTexCircleY.Hide();
            aBtnTexObjectY.Hide();
            aFtTexFilter.Hide();
            aBtnTexFilter.Hide();
            aGrpTexture.Hide();
        }

        // Material
        if( eViewType == VIEWTYPE_MATERIAL )
        {
            aFtMatFavorites.Show();
            aLbMatFavorites.Show();
            aFtMatColor.Show();
            aLbMatColor.Show();
            aBtnMatColor.Show();
            aFtMatEmission.Show();
            aLbMatEmission.Show();
            aBtnEmissionColor.Show();
            aFtMatSpecular.Show();
            aLbMatSpecular.Show();
            aBtnSpecularColor.Show();
            aFtMatSpecularIntensity.Show();
            aMtrMatSpecularIntensity.Show();
            aGrpMatSpecular.Show();
            aGrpMaterial.Show();
        }
        else
        {
            aFtMatFavorites.Hide();
            aLbMatFavorites.Hide();
            aFtMatColor.Hide();
            aLbMatColor.Hide();
            aBtnMatColor.Hide();
            aFtMatEmission.Hide();
            aLbMatEmission.Hide();
            aBtnEmissionColor.Hide();
            aFtMatSpecular.Hide();
            aLbMatSpecular.Hide();
            aBtnSpecularColor.Hide();
            aFtMatSpecularIntensity.Hide();
            aMtrMatSpecularIntensity.Hide();
            aGrpMatSpecular.Hide();
            aGrpMaterial.Hide();
        }
        if( bUpdatePreview && !aBtnLight.IsChecked() )
            UpdatePreview();

    }
    else
    {
        aBtnFavorites.Check( eViewType == VIEWTYPE_FAVORITES );
        aBtnGeo.Check( eViewType == VIEWTYPE_GEO );
        aBtnRepresentation.Check( eViewType == VIEWTYPE_REPRESENTATION );
        aBtnLight.Check( eViewType == VIEWTYPE_LIGHT );
        aBtnTexture.Check( eViewType == VIEWTYPE_TEXTURE );
        aBtnMaterial.Check( eViewType == VIEWTYPE_MATERIAL );
    }
    return( 0L );
}

// -----------------------------------------------------------------------
IMPL_LINK( Svx3DWin, ClickHdl, PushButton *, pBtn )
{
    BOOL bUpdatePreview = FALSE;

    if( pBtn )
    {
        USHORT nSId = 0;

        if( pBtn == &aBtnConvertTo3D )
        {
            nSId = SID_CONVERT_TO_3D;
        }
        else if( pBtn == &aBtnLatheObject )
        {
            nSId = SID_CONVERT_TO_3D_LATHE_FAST;
        }
        // Favoriten
        else if( pBtn == &aBtnOnly3D ||
                 pBtn == &aBtnAllAttributes )
        {
            bOnly3DChanged = TRUE;
            aBtnOnly3D.Check( pBtn == &aBtnOnly3D );
            aBtnAllAttributes.Check( pBtn == &aBtnAllAttributes );
            bUpdatePreview = TRUE;
        }
        // Geometrie
        else if( pBtn == &aBtnNormalsObj ||
                 pBtn == &aBtnNormalsFlat ||
                 pBtn == &aBtnNormalsSphere )
        {
            aBtnNormalsObj.Check( pBtn == &aBtnNormalsObj );
            aBtnNormalsFlat.Check( pBtn == &aBtnNormalsFlat );
            aBtnNormalsSphere.Check( pBtn == &aBtnNormalsSphere );
            bUpdatePreview = TRUE;
        }
        else if( pBtn == &aBtnLight1 ||
                 pBtn == &aBtnLight2 ||
                 pBtn == &aBtnLight3 ||
                 pBtn == &aBtnLight4 ||
                 pBtn == &aBtnLight5 ||
                 pBtn == &aBtnLight6 ||
                 pBtn == &aBtnLight7 ||
                 pBtn == &aBtnLight8 )
        {
            // Beleuchtung
            ColorLB* pLb = GetLbByButton( pBtn );
            pLb->Show();

            if( pBtn->IsChecked() )
            {
                Image aImg( pBtn->GetImage() );

                if( aImg == aImgLightOn )
                    pBtn->SetImage( aImgLightOff );
                else
                    pBtn->SetImage( aImgLightOn );
            }
            else
            {
                pBtn->Check();

                if( pBtn != &aBtnLight1 && aBtnLight1.IsChecked() )
                {
                    aBtnLight1.Check( FALSE );
                    aLbLight1.Hide();
                }
                if( pBtn != &aBtnLight2 && aBtnLight2.IsChecked() )
                {
                    aBtnLight2.Check( FALSE );
                    aLbLight2.Hide();
                }
                if( pBtn != &aBtnLight3 && aBtnLight3.IsChecked() )
                {
                    aBtnLight3.Check( FALSE );
                    aLbLight3.Hide();
                }
                if( pBtn != &aBtnLight4 && aBtnLight4.IsChecked() )
                {
                    aBtnLight4.Check( FALSE );
                    aLbLight4.Hide();
                }
                if( pBtn != &aBtnLight5 && aBtnLight5.IsChecked() )
                {
                    aBtnLight5.Check( FALSE );
                    aLbLight5.Hide();
                }
                if( pBtn != &aBtnLight6 && aBtnLight6.IsChecked() )
                {
                    aBtnLight6.Check( FALSE );
                    aLbLight6.Hide();
                }
                if( pBtn != &aBtnLight7 && aBtnLight7.IsChecked() )
                {
                    aBtnLight7.Check( FALSE );
                    aLbLight7.Hide();
                }
                if( pBtn != &aBtnLight8 && aBtnLight8.IsChecked() )
                {
                    aBtnLight8.Check( FALSE );
                    aLbLight8.Hide();
                }
            }
            BOOL bEnable = pBtn->GetImage() == aImgLightOn;
            aBtnLightColor.Enable( bEnable );
            pLb->Enable( bEnable );

            ClickLightHdl( pBtn );
            bUpdatePreview = TRUE;
        }
        // Texturen
        else if( pBtn == &aBtnTexLuminance ||
                 pBtn == &aBtnTexColor )
        {
            aBtnTexLuminance.Check( pBtn == &aBtnTexLuminance );
            aBtnTexColor.Check( pBtn == &aBtnTexColor );
            bUpdatePreview = TRUE;
        }
        else if( pBtn == &aBtnTexReplace ||
                 pBtn == &aBtnTexModulate )// ||
                 //pBtn == &aBtnTexBlend )
        {
            aBtnTexReplace.Check( pBtn == &aBtnTexReplace );
            aBtnTexModulate.Check( pBtn == &aBtnTexModulate );
            //aBtnTexBlend.Check( pBtn == &aBtnTexBlend );
            bUpdatePreview = TRUE;
        }
        else if( pBtn == &aBtnTexParallelX ||
                 pBtn == &aBtnTexCircleX ||
                 pBtn == &aBtnTexObjectX )
        {
            aBtnTexParallelX.Check( pBtn == &aBtnTexParallelX );
            aBtnTexCircleX.Check( pBtn == &aBtnTexCircleX );
            aBtnTexObjectX.Check( pBtn == &aBtnTexObjectX );
            bUpdatePreview = TRUE;
        }
        else if( pBtn == &aBtnTexParallelY ||
                 pBtn == &aBtnTexCircleY ||
                 pBtn == &aBtnTexObjectY )
        {
            aBtnTexParallelY.Check( pBtn == &aBtnTexParallelY );
            aBtnTexCircleY.Check( pBtn == &aBtnTexCircleY );
            aBtnTexObjectY.Check( pBtn == &aBtnTexObjectY );
            bUpdatePreview = TRUE;
        }
        else if( pBtn == &aBtnShadow3d  )
        {
            pBtn->Check( !pBtn->IsChecked() );
            aFtSlant.Enable( pBtn->IsChecked() );
            aMtrSlant.Enable( pBtn->IsChecked() );
            bUpdatePreview = TRUE;
        }
        // Sonstige (keine Gruppen)
        else if( pBtn != NULL )
        {
            pBtn->Check( !pBtn->IsChecked() );
            bUpdatePreview = TRUE;
        }

        if( nSId > 0 )
        {
            SfxBoolItem aItem( nSId, TRUE );
            SfxViewFrame::Current()->GetBindings().GetDispatcher()->Execute(
                nSId, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L );
        }
        else if( bUpdatePreview == TRUE )
            UpdatePreview();
    }
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( Svx3DWin, ClickColorHdl, PushButton *, pBtn )
{
    SvColorDialog aColorDlg( this );
    ColorLB* pLb;

    if( pBtn == &aBtnLightColor )
        pLb = GetLbByButton();
    else if( pBtn == &aBtnAmbientColor )
        pLb = &aLbAmbientlight;
    else if( pBtn == &aBtnMatColor )
        pLb = &aLbMatColor;
    else if( pBtn == &aBtnEmissionColor )
        pLb = &aLbMatEmission;
    else if( pBtn == &aBtnSpecularColor )
        pLb = &aLbMatSpecular;

    Color aColor = pLb->GetSelectEntryColor();

    aColorDlg.SetColor( aColor );
    if( aColorDlg.Execute() == RET_OK )
    {
        aColor = aColorDlg.GetColor();
        if( LBSelectColor( pLb, aColor ) )
            SelectHdl( pLb );
    }
    return( 0L );
}

// -----------------------------------------------------------------------
IMPL_LINK( Svx3DWin, SelectHdl, void *, p )
{
    if( p )
    {
        Color aColor;
        BOOL bUpdatePreview = FALSE;

        // Material
        if( p == &aLbMatFavorites )
        {
            Color aColObj( COL_WHITE );
            Color aColEmis( COL_BLACK );
            Color aColSpec( COL_WHITE );
            USHORT nSpecIntens = 20;

            USHORT nPos = aLbMatFavorites.GetSelectEntryPos();
            switch( nPos )
            {
                case 1: // Metall
                {
                    aColObj = Color(230,230,255);
                    aColEmis = Color(10,10,30);
                    aColSpec = Color(200,200,200);
                    nSpecIntens = 20;
                }
                break;

                case 2: // Gold
                {
                    aColObj = Color(230,255,0);
                    aColEmis = Color(51,0,0);
                    aColSpec = Color(255,255,240);
                    nSpecIntens = 20;
                }
                break;

                case 3: // Chrom
                {
                    aColObj = Color(36,117,153);
                    aColEmis = Color(18,30,51);
                    aColSpec = Color(230,230,255);
                    nSpecIntens = 2;
                }
                break;

                case 4: // Plastik
                {
                    aColObj = Color(255,48,57);
                    aColEmis = Color(35,0,0);
                    aColSpec = Color(179,202,204);
                    nSpecIntens = 60;
                }
                break;

                case 5: // Holz
                {
                    aColObj = Color(153,71,1);
                    aColEmis = Color(21,22,0);
                    aColSpec = Color(255,255,153);
                    nSpecIntens = 75;
                }
                break;
            }
            LBSelectColor( &aLbMatColor, aColObj );
            LBSelectColor( &aLbMatEmission, aColEmis );
            LBSelectColor( &aLbMatSpecular, aColSpec );
            aMtrMatSpecularIntensity.SetValue( nSpecIntens );

            bUpdatePreview = TRUE;
        }
        else if( p == &aLbMatColor ||
                 p == &aLbMatEmission ||
                 p == &aLbMatSpecular )
        {
            aColor = ( ( (ColorLB*)p )->GetSelectEntryColor() );

            Base3DMaterialValue eType = Base3DMaterialDiffuse;
            //if( p == &aLbMatColor )
            if( p == &aLbMatEmission )
                eType = Base3DMaterialEmission;
            else if( p == &aLbMatSpecular )
                eType = Base3DMaterialSpecular;

            //aCtlPreview.SetMaterial( aColor, eType );
            aCtlLightPreview.GetPreviewControl().SetMaterial( aColor, eType );

            aLbMatFavorites.SelectEntryPos( 0 );

            bUpdatePreview = TRUE;
        }
        // Beleuchtung
        else if( p == &aLbAmbientlight )
        {
            Color aColor = aLbAmbientlight.GetSelectEntryColor();
            pLightGroup->SetGlobalAmbientLight( aColor );

            aCtlLightPreview.GetPreviewControl().SetLightGroup( pLightGroup );
            //aCtlPreview.SetLightGroup( pLightGroup );
            bUpdatePreview = TRUE;
        }
        else if( p == &aLbLight1 ||
                 p == &aLbLight2 ||
                 p == &aLbLight3 ||
                 p == &aLbLight4 ||
                 p == &aLbLight5 ||
                 p == &aLbLight6 ||
                 p == &aLbLight7 ||
                 p == &aLbLight8 )
        {
            Color aColor = ( (ColorLB*)p )->GetSelectEntryColor();
            USHORT nLightSource = GetLightSource();

            *pLightGroup = *aCtlLightPreview.GetPreviewControl().GetLightGroup();

            pLightGroup->SetIntensity( aColor,
                            Base3DMaterialDiffuse,
                            (Base3DLightNumber) nLightSource );

            aCtlLightPreview.GetPreviewControl().SetLightGroup( pLightGroup );
            //aCtlPreview.SetLightGroup( pLightGroup );
            bUpdatePreview = TRUE;
        }
        else if( p == &aLbShademode )
            bUpdatePreview = TRUE;

        if( bUpdatePreview == TRUE )
            UpdatePreview();
    }
    return( 0L );
}

// -----------------------------------------------------------------------
IMPL_LINK( Svx3DWin, ModifyHdl, void*, pField )
{
    if( pField )
    {
        BOOL bUpdatePreview = FALSE;

        // Material
        if( pField == &aMtrMatSpecularIntensity )
        {
            UINT16 nValue = (UINT16) ( (MetricField*)pField )->GetValue();
            //aCtlPreview.SetShininess( nValue );
            aCtlLightPreview.GetPreviewControl().SetShininess( nValue );
            bUpdatePreview = TRUE;
        }
        else if( pField == &aNumHorizontal )
        {
            UINT16 nValue = (UINT16) ( (NumericField*)pField )->GetValue();
            aCtlLightPreview.GetPreviewControl().SetHorizontalSegments( nValue );
            //aCtlPreview.SetHorizontalSegments( nValue );
            bUpdatePreview = TRUE;
        }
        else if( pField == &aNumVertical )
        {
            UINT16 nValue = (UINT16) ( (NumericField*)pField )->GetValue();
            aCtlLightPreview.GetPreviewControl().SetVerticalSegments( nValue );
            //aCtlPreview.SetVerticalSegments( nValue );
            bUpdatePreview = TRUE;
        }
        else if( pField == &aMtrSlant )
            bUpdatePreview = TRUE;

        if( bUpdatePreview == TRUE )
            UpdatePreview();
    }
    return( 0L );
}

// -----------------------------------------------------------------------

IMPL_LINK( Svx3DWin, ClickLightHdl, PushButton*, pBtn )
{

    if( pBtn )
    {
        USHORT nLightSource = GetLightSource( pBtn );
        ColorLB* pLb = GetLbByButton( pBtn );
        Color aColor( pLb->GetSelectEntryColor() );
        if( nLightSource == 7 &&
            aColor.GetRed()==9&&aColor.GetGreen()==4&&aColor.GetBlue()==67)
        {
            aFtLightX.Show();
            aFtLightY.Show();
            aFtLightZ.Show();
            aGrpLightInfo.Show();
        }
        *pLightGroup = *aCtlLightPreview.GetPreviewControl().GetLightGroup();

        pLightGroup->SetIntensity( aColor,
                        Base3DMaterialDiffuse,
                        (Base3DLightNumber) nLightSource );

        Image aImg( pBtn->GetImage() );
        pLightGroup->Enable( aImg == aImgLightOn,
                        (Base3DLightNumber) nLightSource );

        aCtlLightPreview.GetPreviewControl().SetLightGroup( pLightGroup );
        aCtlLightPreview.GetPreviewControl().SelectLight( (Base3DLightNumber) nLightSource );
        aCtlLightPreview.CheckSelection();

        //aCtlPreview.SetLightGroup( pLightGroup );
    }
    return( 0L );
}


// -----------------------------------------------------------------------
IMPL_LINK( Svx3DWin, DoubleClickHdl, void*, p )
{
    //USHORT nItemId = aCtlFavorites.GetSelectItemId();

    //SfxItemSet* pSet = (SfxItemSet*) pFavorSetList->GetObject( nItemId - 1 );
    //Update( *pSet );

    // und zuweisen
    ClickAssignHdl( NULL );

    return( 0L );
}

// -----------------------------------------------------------------------
IMPL_LINK( Svx3DWin, ClickFavoriteHdl, void*, p )
{
    USHORT nItemId = aCtlFavorites.GetSelectItemId();

    if( nItemId > 0 )
    {
        // neues, leeres Model generieren
        if(pModel)
            delete pModel;
        pModel = new FmFormModel();
        pModel->GetItemPool().FreezeIdRanges();

        if( GalleryExplorer::GetSdrObj( GALLERY_THEME_3D, nItemId-1, pModel ) )
        {
            // VDev
            if( pVDev == NULL )
            {
                pVDev = new VirtualDevice();
                MapMode aMapMode( MAP_100TH_MM );
                pVDev->SetMapMode( aMapMode );
            }
            // 3D View
            E3dView a3DView( pModel, pVDev );
            a3DView.SetMarkHdlHidden(TRUE);

            // PageView
            SdrPageView* pPageView = a3DView.ShowPagePgNum( 0, Point() );
            a3DView.MarkAll();

            // Status der Selektion am Preview setzen, SYNCHRON
            SfxBoolItem aItem( SID_3D_STATE, TRUE );
            SfxViewFrame::Current()->GetBindings().GetDispatcher()->Execute(
                SID_3D_STATE, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L );

            // Attribute des Favoriten holen und setzen
            SfxItemSet* pSet;
            if( aBtnOnly3D.IsChecked())
            {
                // Normale (nicht-3D-) Attribute an der View
                pSet = new SfxItemSet(
                    pModel->GetItemPool(),
                    SDRATTR_SHADOW, SDRATTR_SHADOW,
                    SID_ATTR_3D_START, SID_ATTR_3D_END, 0);
            }
            else
            {
                // Alle Attribute an der View
                pSet = new SfxItemSet( // pModel->GetItemPool());
                    pModel->GetItemPool(),
                    SDRATTR_START,  SDRATTR_END,
                    SID_ATTR_3D_START, SID_ATTR_3D_END, 0);
            }

            // Eingeschraenktes Set produzieren und zuweisen
            pSet->Put( a3DView.Get3DAttributes(), FALSE );

            // #61783# Remove distance and focus from prototypes
            pSet->ClearItem(SID_ATTR_3D_DISTANCE);
            pSet->ClearItem(SID_ATTR_3D_FOCAL_LENGTH);

            aCtlPreview.Set3DAttributes( *pSet );

            BOOL bOldUpdate = bUpdate;
            bUpdate = FALSE;
            Update( *pSet );
            bUpdate = bOldUpdate;

            delete pSet;
        }
    }
    return( 0L );
}

// -----------------------------------------------------------------------

IMPL_LINK( Svx3DWin, ChangeLightCallbackHdl, void*, p )
{
    *pLightGroup = *aCtlLightPreview.GetPreviewControl().GetLightGroup();
    //aCtlPreview.SetLightGroup( pLightGroup );

    // Setzen der Vector Informationen
    Base3DLightNumber eLight = aCtlLightPreview.GetPreviewControl().GetSelectedLight();
    if( eLight != Base3DLightNone && aFtLightX.IsVisible() )
    {
        Vector3D aVector = pLightGroup->GetDirection( eLight );
        aVector.Normalize();

        String aStr;

        aStr = String(SVX_RES(RID_SVXFLOAT3D_FIX_X));
        aStr += sal_Unicode(' ');
        aStr += String::CreateFromInt32((INT32)(aVector.X()*1000.0));
        aFtLightX.SetText( aStr );

        aStr = String(SVX_RES(RID_SVXFLOAT3D_FIX_Y));
        aStr += sal_Unicode(' ');
        aStr += String::CreateFromInt32((INT32)(aVector.Y()*1000.0));
        aFtLightY.SetText( aStr );

        aStr = String(SVX_RES(RID_SVXFLOAT3D_FIX_Z));
        aStr += sal_Unicode(' ');
        aStr += String::CreateFromInt32((INT32)(aVector.Z()*1000.0));
        aFtLightZ.SetText( aStr );
    }
    return( 0L );
}


// -----------------------------------------------------------------------

IMPL_LINK( Svx3DWin, ChangeSelectionCallbackHdl, void*, p )
{
    Base3DLightNumber eLight = aCtlLightPreview.GetPreviewControl().GetSelectedLight();

    PushButton* pBtn = NULL;

    switch( eLight )
    {
        case 0: pBtn = &aBtnLight1; break;
        case 1: pBtn = &aBtnLight2; break;
        case 2: pBtn = &aBtnLight3; break;
        case 3: pBtn = &aBtnLight4; break;
        case 4: pBtn = &aBtnLight5; break;
        case 5: pBtn = &aBtnLight6; break;
        case 6: pBtn = &aBtnLight7; break;
        case 7: pBtn = &aBtnLight8; break;
    }

    if( pBtn )
        ClickHdl( pBtn );
    else
    {
        // Zustand: Keine Lampe selektiert
        if( aBtnLight1.IsChecked() )
        {
            aBtnLight1.Check( FALSE );
            aLbLight1.Enable( FALSE );
        }
        else if( aBtnLight2.IsChecked() )
        {
            aBtnLight2.Check( FALSE );
            aLbLight2.Enable( FALSE );
        }
        else if( aBtnLight3.IsChecked() )
        {
            aBtnLight3.Check( FALSE );
            aLbLight3.Enable( FALSE );
        }
        else if( aBtnLight4.IsChecked() )
        {
            aBtnLight4.Check( FALSE );
            aLbLight4.Enable( FALSE );
        }
        else if( aBtnLight5.IsChecked() )
        {
            aBtnLight5.Check( FALSE );
            aLbLight5.Enable( FALSE );
        }
        else if( aBtnLight6.IsChecked() )
        {
            aBtnLight6.Check( FALSE );
            aLbLight6.Enable( FALSE );
        }
        else if( aBtnLight7.IsChecked() )
        {
            aBtnLight7.Check( FALSE );
            aLbLight7.Enable( FALSE );
        }
        else if( aBtnLight8.IsChecked() )
        {
            aBtnLight8.Check( FALSE );
            aLbLight8.Enable( FALSE );
        }
        aBtnLightColor.Enable( FALSE );
    }

    if( eLight != Base3DLightNone && aFtLightX.IsVisible() )
    {
        Vector3D aVector = pLightGroup->GetDirection( eLight );
        aVector.Normalize();

        String aStr;

        aStr = String(SVX_RES(RID_SVXFLOAT3D_FIX_X));
        aStr += sal_Unicode(' ');
        aStr += String::CreateFromInt32((INT32)(aVector.X()*1000.0));
        aFtLightX.SetText( aStr );

        aStr = String(SVX_RES(RID_SVXFLOAT3D_FIX_Y));
        aStr += sal_Unicode(' ');
        aStr += String::CreateFromInt32((INT32)(aVector.Y()*1000.0));
        aFtLightY.SetText( aStr );

        aStr = String(SVX_RES(RID_SVXFLOAT3D_FIX_Z));
        aStr += sal_Unicode(' ');
        aStr += String::CreateFromInt32((INT32)(aVector.Z()*1000.0));
        aFtLightZ.SetText( aStr );
    }

    return( 0L );
}

// -----------------------------------------------------------------------
// Methode um sicherzustellen, dass die LB auch mit einer Farbe gesetzt ist
// Liefert TRUE zurueck, falls Farbe hinzugefuegt wurde
// -----------------------------------------------------------------------
BOOL Svx3DWin::LBSelectColor( ColorLB* pLb, const Color& rColor )
{
    BOOL bRet = FALSE;

    pLb->SetNoSelection();
    pLb->SelectEntry( rColor );
    if( pLb->GetSelectEntryCount() == 0 )
    {
        String aStr(SVX_RES(RID_SVXFLOAT3D_FIX_R));

        aStr += String::CreateFromInt32((INT32)rColor.GetRed());
        aStr += sal_Unicode(' ');
        aStr += String(SVX_RES(RID_SVXFLOAT3D_FIX_G));
        aStr += String::CreateFromInt32((INT32)rColor.GetGreen());
        aStr += sal_Unicode(' ');
        aStr += String(SVX_RES(RID_SVXFLOAT3D_FIX_B));
        aStr += String::CreateFromInt32((INT32)rColor.GetBlue());

        USHORT nPos = pLb->InsertEntry( rColor, aStr );
        pLb->SelectEntryPos( nPos );
        bRet = TRUE;
    }
    return( bRet );
}

// -----------------------------------------------------------------------
void Svx3DWin::UpdatePreview()
{
    if( pModel == NULL )
        pModel = new FmFormModel();

    if(bOnly3DChanged)
    {
        // slot executen
        SfxBoolItem aItem( SID_3D_STATE, TRUE );
        SfxViewFrame::Current()->GetBindings().GetDispatcher()->Execute(
            SID_3D_STATE, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L );
        // Flag zuruecksetzen
        bOnly3DChanged = FALSE;

        // Favorit nochmals anwenden
        ClickFavoriteHdl(0);
    }

    // ItemSet besorgen
    SfxItemSet aSet( pModel->GetItemPool(),
        SDRATTR_START,  SDRATTR_END,
        SID_ATTR_3D_START, SID_ATTR_3D_END,
        0 );

    // Attribute holen und im Preview setzen
    GetAttr( aSet );
    aCtlPreview.Set3DAttributes( aSet );
    aCtlFavorites.SetNoSelection();
}

// -----------------------------------------------------------------------
void Svx3DWin::InitColorLB( const SdrModel* pDoc )
{
    aLbLight1.Fill( pDoc->GetColorTable() );
    aLbLight2.CopyEntries( aLbLight1 );
    aLbLight3.CopyEntries( aLbLight1 );
    aLbLight4.CopyEntries( aLbLight1 );
    aLbLight5.CopyEntries( aLbLight1 );
    aLbLight6.CopyEntries( aLbLight1 );
    aLbLight7.CopyEntries( aLbLight1 );
    aLbLight8.CopyEntries( aLbLight1 );
    aLbAmbientlight.CopyEntries( aLbLight1 );
    aLbMatColor.CopyEntries( aLbLight1 );
    aLbMatEmission.CopyEntries( aLbLight1 );
    aLbMatSpecular.CopyEntries( aLbLight1 );

    // Erstmal...
    Color aColWhite( COL_WHITE );
    Color aColBlack( COL_BLACK );
    aLbLight1.SelectEntry( aColWhite );
    aLbLight2.SelectEntry( aColWhite );
    aLbLight3.SelectEntry( aColWhite );
    aLbLight4.SelectEntry( aColWhite );
    aLbLight5.SelectEntry( aColWhite );
    aLbLight6.SelectEntry( aColWhite );
    aLbLight7.SelectEntry( aColWhite );
    aLbLight8.SelectEntry( aColWhite );
    aLbAmbientlight.SelectEntry( aColBlack );
    aLbMatColor.SelectEntry( aColWhite );
    aLbMatEmission.SelectEntry( aColBlack );
    aLbMatSpecular.SelectEntry( aColWhite );
}

// -----------------------------------------------------------------------
USHORT Svx3DWin::GetLightSource( const PushButton* pBtn )
{
    USHORT nLight = 8;

    if( pBtn == NULL )
    {
        if( aBtnLight1.IsChecked() )
            nLight = 0;
        else if( aBtnLight2.IsChecked() )
            nLight = 1;
        else if( aBtnLight3.IsChecked() )
            nLight = 2;
        else if( aBtnLight4.IsChecked() )
            nLight = 3;
        else if( aBtnLight5.IsChecked() )
            nLight = 4;
        else if( aBtnLight6.IsChecked() )
            nLight = 5;
        else if( aBtnLight7.IsChecked() )
            nLight = 6;
        else if( aBtnLight8.IsChecked() )
            nLight = 7;
    }
    else
    {
        if( pBtn == &aBtnLight1 )
            nLight = 0;
        else if( pBtn == &aBtnLight2 )
            nLight = 1;
        else if( pBtn == &aBtnLight3 )
            nLight = 2;
        else if( pBtn == &aBtnLight4 )
            nLight = 3;
        else if( pBtn == &aBtnLight5 )
            nLight = 4;
        else if( pBtn == &aBtnLight6 )
            nLight = 5;
        else if( pBtn == &aBtnLight7 )
            nLight = 6;
        else if( pBtn == &aBtnLight8 )
            nLight = 7;
    }
    return( nLight );
};

// -----------------------------------------------------------------------
ColorLB* Svx3DWin::GetLbByButton( const PushButton* pBtn )
{
    ColorLB* pLb = NULL;

    if( pBtn == NULL )
    {
        if( aBtnLight1.IsChecked() )
            pLb = &aLbLight1;
        else if( aBtnLight2.IsChecked() )
            pLb = &aLbLight2;
        else if( aBtnLight3.IsChecked() )
            pLb = &aLbLight3;
        else if( aBtnLight4.IsChecked() )
            pLb = &aLbLight4;
        else if( aBtnLight5.IsChecked() )
            pLb = &aLbLight5;
        else if( aBtnLight6.IsChecked() )
            pLb = &aLbLight6;
        else if( aBtnLight7.IsChecked() )
            pLb = &aLbLight7;
        else if( aBtnLight8.IsChecked() )
            pLb = &aLbLight8;
    }
    else
    {
        if( pBtn == &aBtnLight1 )
            pLb = &aLbLight1;
        else if( pBtn == &aBtnLight2 )
            pLb = &aLbLight2;
        else if( pBtn == &aBtnLight3 )
            pLb = &aLbLight3;
        else if( pBtn == &aBtnLight4 )
            pLb = &aLbLight4;
        else if( pBtn == &aBtnLight5 )
            pLb = &aLbLight5;
        else if( pBtn == &aBtnLight6 )
            pLb = &aLbLight6;
        else if( pBtn == &aBtnLight7 )
            pLb = &aLbLight7;
        else if( pBtn == &aBtnLight8 )
            pLb = &aLbLight8;
    }
    return( pLb );
};

/*************************************************************************
|*
|* Ableitung vom SfxChildWindow als "Behaelter" fuer Effekte
|*
\************************************************************************/
__EXPORT Svx3DChildWindow::Svx3DChildWindow( Window* pParent,
                                                         USHORT nId,
                                                         SfxBindings* pBindings,
                                                         SfxChildWinInfo* pInfo ) :
    SfxChildWindow( pParent, nId )
{
    Svx3DWin* pWin = new Svx3DWin( pBindings, this, pParent );
    pWindow = pWin;

    eChildAlignment = SFX_ALIGN_NOALIGNMENT;

    pWin->Initialize( pInfo );
}

/*************************************************************************
|*
|* ControllerItem fuer 3DStatus
|*
\************************************************************************/
Svx3DCtrlItem::Svx3DCtrlItem( USHORT nId,
                                Svx3DWin* pWin,
                                SfxBindings* pBindings) :
    SfxControllerItem( nId, *pBindings ),
    p3DWin( pWin )
{
}

// -----------------------------------------------------------------------
void __EXPORT Svx3DCtrlItem::StateChanged( USHORT nSId,
                        SfxItemState eState, const SfxPoolItem* pItem )
{
    /*
    if( eState >= SFX_ITEM_AVAILABLE && nSId == SID_3D_STATE )
    {
        const SfxUInt32Item* pStateItem = PTR_CAST( SfxUInt32Item, pItem );
        DBG_ASSERT( pStateItem, "SfxUInt32Item erwartet");
        UINT32 nState = pStateItem->GetValue();
        p3DWin->bCount = (BOOL) (nState & 3D_COUNT);
    }
    */
}

/*************************************************************************
|*
|* ControllerItem fuer Status Slot SID_CONVERT_TO_3D
|*
\************************************************************************/

SvxConvertTo3DItem::SvxConvertTo3DItem(UINT16 nId, SfxBindings* pBindings)
:   SfxControllerItem(nId, *pBindings),
    bState(FALSE)
{
}

void SvxConvertTo3DItem::StateChanged(UINT16 nId, SfxItemState eState, const SfxPoolItem* pState)
{
    BOOL bNewState = (eState != SFX_ITEM_DISABLED);
    if(bNewState != bState)
    {
        bState = bNewState;
        SfxBoolItem aItem( SID_3D_STATE, TRUE );
        SfxViewFrame::Current()->GetBindings().GetDispatcher()->Execute(
            SID_3D_STATE, SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD, &aItem, 0L);
    }
}


