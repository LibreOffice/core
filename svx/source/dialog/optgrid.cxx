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
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------
#include <tools/shl.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <svl/intitem.hxx>

#define _SVX_OPTGRID_CXX

#include <svx/svxids.hrc>
#include <svx/dialmgr.hxx>
#include <svx/optgrid.hxx>
#include <svx/dialogs.hrc>
#include <optgrid.hrc>
#include <svx/dlgutil.hxx>

/* -----------------18.08.98 17:41-------------------
 * local functions
 * --------------------------------------------------*/
void    lcl_GetMinMax(MetricField& rField, long& nFirst, long& nLast, long& nMin, long& nMax)
{
    nFirst  = static_cast<long>(rField.Denormalize( rField.GetFirst( FUNIT_TWIP ) ));
    nLast = static_cast<long>(rField.Denormalize( rField.GetLast( FUNIT_TWIP ) ));
    nMin = static_cast<long>(rField.Denormalize( rField.GetMin( FUNIT_TWIP ) ));
    nMax = static_cast<long>(rField.Denormalize( rField.GetMax( FUNIT_TWIP ) ));
}

void    lcl_SetMinMax(MetricField& rField, long nFirst, long nLast, long nMin, long nMax)
{
    rField.SetFirst( rField.Normalize( nFirst ), FUNIT_TWIP );
    rField.SetLast( rField.Normalize( nLast ), FUNIT_TWIP );
    rField.SetMin( rField.Normalize( nMin ), FUNIT_TWIP );
    rField.SetMax( rField.Normalize( nMax ), FUNIT_TWIP );
}

/*--------------------------------------------------------------------
    Beschreibung: Rastereinstellungen Ctor
 --------------------------------------------------------------------*/

SvxOptionsGrid::SvxOptionsGrid() :
    mnFldDrawX      ( 100 ),
    mnFldDivisionX  ( 0 ),
    mnFldDrawY      ( 100 ),
    mnFldDivisionY  ( 0 ),
    mnFldSnapX      ( 100 ),
    mnFldSnapY      ( 100 ),
    mbUseGridsnap   ( 0 ),
    mbSynchronize   ( 1 ),
    mbGridVisible   ( 0 ),
    mbEqualGrid     ( 1 )
{
}

/*--------------------------------------------------------------------
    Beschreibung: Rastereinstellungen Dtor
 --------------------------------------------------------------------*/

SvxOptionsGrid::~SvxOptionsGrid()
{
}

/*--------------------------------------------------------------------
    Beschreibung: Item fuer Rastereinstellungen
 --------------------------------------------------------------------*/

SvxGridItem::SvxGridItem( const SvxGridItem& rItem )
:   SvxOptionsGrid()
,   SfxPoolItem(rItem)
{
    mbUseGridsnap = rItem.mbUseGridsnap ;
    mbSynchronize = rItem.mbSynchronize ;
    mbGridVisible = rItem.mbGridVisible ;
    mbEqualGrid  = rItem.mbEqualGrid      ;
    mnFldDrawX    = rItem.mnFldDrawX    ;
    mnFldDivisionX= rItem.mnFldDivisionX;
    mnFldDrawY    = rItem.mnFldDrawY    ;
    mnFldDivisionY= rItem.mnFldDivisionY;
    mnFldSnapX    = rItem.mnFldSnapX    ;
    mnFldSnapY    = rItem.mnFldSnapY    ;

};

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SfxPoolItem*  SvxGridItem::Clone( SfxItemPool* ) const
{
    return new SvxGridItem( *this );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

int SvxGridItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unterschiedliche Typen" );

    const SvxGridItem& rItem = (const SvxGridItem&) rAttr;

    return (    mbUseGridsnap == rItem.mbUseGridsnap &&
                mbSynchronize == rItem.mbSynchronize &&
                mbGridVisible == rItem.mbGridVisible &&
                mbEqualGrid  == rItem.mbEqualGrid   &&
                mnFldDrawX    == rItem.mnFldDrawX    &&
                mnFldDivisionX== rItem.mnFldDivisionX&&
                mnFldDrawY    == rItem.mnFldDrawY    &&
                mnFldDivisionY== rItem.mnFldDivisionY&&
                mnFldSnapX    == rItem.mnFldSnapX    &&
                mnFldSnapY    == rItem.mnFldSnapY       );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SfxItemPresentation  SvxGridItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    String&             rText, const IntlWrapper *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return SFX_ITEM_PRESENTATION_NONE;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = String::CreateFromAscii("SvxGridItem");
            return ePres;
        default:
            return SFX_ITEM_PRESENTATION_NONE;
    }
}


/*----------------- OS 23.02.95  -----------------------
 TabPage Rastereinstellungen
-------------------------------------------------------*/

SvxGridTabPage::SvxGridTabPage( Window* pParent, const SfxItemSet& rCoreSet) :

    SfxTabPage( pParent, SVX_RES( RID_SVXPAGE_GRID ), rCoreSet ),

    aCbxUseGridsnap ( this, SVX_RES( CBX_USE_GRIDSNAP ) ),
    aCbxGridVisible ( this, SVX_RES( CBX_GRID_VISIBLE ) ),

    aFlResolution   ( this, SVX_RES( FL_RESOLUTION ) ),
    aFtDrawX        ( this, SVX_RES( FT_DRAW_X ) ),
    aMtrFldDrawX    ( this, SVX_RES( MTR_FLD_DRAW_X ) ),
    aFtDrawY        ( this, SVX_RES( FT_DRAW_Y ) ),
    aMtrFldDrawY    ( this, SVX_RES( MTR_FLD_DRAW_Y ) ),

    aFlDivision     ( this, SVX_RES( FL_DIVISION ) ),
    aFtDivisionX(     this, SVX_RES( FT_DIVISION_X) ),
    aNumFldDivisionX( this, SVX_RES( NUM_FLD_DIVISION_X ) ),
    aDivisionPointX(  this, SVX_RES( FT_HORZ_POINTS) ),

    aFtDivisionY(     this, SVX_RES( FT_DIVISION_Y) ),
    aNumFldDivisionY( this, SVX_RES( NUM_FLD_DIVISION_Y ) ),
    aDivisionPointY(  this, SVX_RES( FT_VERT_POINTS) ),

    aCbxSynchronize ( this, SVX_RES( CBX_SYNCHRONIZE ) ),
    aGrpDrawGrid    ( this, SVX_RES( GRP_DRAWGRID ) ),

    aGrpSnap            ( this, SVX_RES( GRP_SNAP ) ),
    aCbxSnapHelplines   ( this, SVX_RES( CBX_SNAP_HELPLINES ) ),
    aCbxSnapBorder      ( this, SVX_RES( CBX_SNAP_BORDER ) ),
    aCbxSnapFrame       ( this, SVX_RES( CBX_SNAP_FRAME ) ),
    aCbxSnapPoints      ( this, SVX_RES( CBX_SNAP_POINTS ) ),
    aFtSnapArea         ( this, SVX_RES( FT_SNAP_AREA ) ),
    aMtrFldSnapArea     ( this, SVX_RES( MTR_FLD_SNAP_AREA ) ),

    aSeparatorFL        ( this, SVX_RES( FL_SEPARATOR ) ),

    aGrpOrtho           ( this, SVX_RES( GRP_ORTHO ) ),
    aCbxOrtho           ( this, SVX_RES( CBX_ORTHO ) ),
    aCbxBigOrtho        ( this, SVX_RES( CBX_BIGORTHO ) ),
    aCbxRotate          ( this, SVX_RES( CBX_ROTATE ) ),
    aMtrFldAngle        ( this, SVX_RES( MTR_FLD_ANGLE ) ),
    aFtBezAngle         ( this, SVX_RES( FT_BEZ_ANGLE ) ),
    aMtrFldBezAngle     ( this, SVX_RES( MTR_FLD_BEZ_ANGLE ) ),

    mbAttrModified( false )
{
    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    FreeResource();

    aDivisionPointY.SetText(aDivisionPointX.GetText());
    // Metrik einstellen
    FieldUnit eFUnit = GetModuleFieldUnit( rCoreSet );
    long nFirst, nLast, nMin, nMax;

    lcl_GetMinMax(aMtrFldDrawX, nFirst, nLast, nMin, nMax);
    SetFieldUnit( aMtrFldDrawX, eFUnit, sal_True );
    lcl_SetMinMax(aMtrFldDrawX, nFirst, nLast, nMin, nMax);

    lcl_GetMinMax(aMtrFldDrawY, nFirst, nLast, nMin, nMax);
    SetFieldUnit( aMtrFldDrawY, eFUnit, sal_True );
    lcl_SetMinMax(aMtrFldDrawY, nFirst, nLast, nMin, nMax);


    aCbxRotate.SetClickHdl( LINK( this, SvxGridTabPage, ClickRotateHdl_Impl ) );
    Link aLink = LINK( this, SvxGridTabPage, ChangeGridsnapHdl_Impl );
    aCbxUseGridsnap.SetClickHdl( aLink );
    aCbxSynchronize.SetClickHdl( aLink );
    aCbxGridVisible.SetClickHdl( aLink );
    aMtrFldDrawX.SetModifyHdl(
        LINK( this, SvxGridTabPage, ChangeDrawHdl_Impl ) );
    aMtrFldDrawY.SetModifyHdl(
        LINK( this, SvxGridTabPage, ChangeDrawHdl_Impl ) );
    aNumFldDivisionX.SetModifyHdl(
        LINK( this, SvxGridTabPage, ChangeDivisionHdl_Impl ) );
    aNumFldDivisionY.SetModifyHdl(
        LINK( this, SvxGridTabPage, ChangeDivisionHdl_Impl ) );

    ::rtl::OUString sFlResolution( aFlResolution.GetDisplayText() );
    ::rtl::OUString sFtDrawX(aFtDrawX.GetDisplayText());
    ::rtl::OUString sFtDrawY(aFtDrawY.GetDisplayText());
    aMtrFldDrawX.SetAccessibleName( sFtDrawX + sFlResolution   );
    aMtrFldDrawY.SetAccessibleName( sFtDrawY + sFlResolution   );
    ::rtl::OUString sFlDivision( aFlDivision.GetDisplayText() );
    ::rtl::OUString sFtDivisionX(aFtDivisionX.GetDisplayText());
    ::rtl::OUString sFtDivisionY(aFtDivisionY.GetDisplayText());
    aNumFldDivisionX.SetAccessibleName(  sFtDivisionX + sFlDivision );
    aNumFldDivisionY.SetAccessibleName(  sFtDivisionY + sFlDivision );
}

//------------------------------------------------------------------------

SfxTabPage* SvxGridTabPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return ( new SvxGridTabPage( pParent, rAttrSet ) );
}

//------------------------------------------------------------------------

sal_Bool SvxGridTabPage::FillItemSet( SfxItemSet& rCoreSet )
{
    if ( mbAttrModified )
    {
        SvxGridItem aGridItem( SID_ATTR_GRID_OPTIONS );

        aGridItem.mbUseGridsnap = aCbxUseGridsnap.IsChecked();
        aGridItem.mbSynchronize = aCbxSynchronize.IsChecked();
        aGridItem.mbGridVisible = aCbxGridVisible.IsChecked();

        SfxMapUnit eUnit =
            rCoreSet.GetPool()->GetMetric( GetWhich( SID_ATTR_GRID_OPTIONS ) );
        long nX =GetCoreValue(  aMtrFldDrawX, eUnit );
        long nY = GetCoreValue( aMtrFldDrawY, eUnit );

        aGridItem.mnFldDrawX    = (sal_uInt32) nX;
        aGridItem.mnFldDrawY    = (sal_uInt32) nY;
        aGridItem.mnFldDivisionX = static_cast<long>(aNumFldDivisionX.GetValue() - 1);
        aGridItem.mnFldDivisionY = static_cast<long>(aNumFldDivisionY.GetValue() - 1);

        rCoreSet.Put( aGridItem );
    }
    return mbAttrModified;
}

//------------------------------------------------------------------------

void SvxGridTabPage::Reset( const SfxItemSet& rSet )
{
    const SfxPoolItem* pAttr = 0;

    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_GRID_OPTIONS , sal_False,
                                    (const SfxPoolItem**)&pAttr ))
    {
        const SvxGridItem* pGridAttr = (SvxGridItem*)pAttr;
        aCbxUseGridsnap.Check( pGridAttr->mbUseGridsnap == 1 );
        aCbxSynchronize.Check( pGridAttr->mbSynchronize == 1 );
        aCbxGridVisible.Check( pGridAttr->mbGridVisible == 1 );

        SfxMapUnit eUnit =
            rSet.GetPool()->GetMetric( GetWhich( SID_ATTR_GRID_OPTIONS ) );
        SetMetricValue( aMtrFldDrawX , pGridAttr->mnFldDrawX, eUnit );
        SetMetricValue( aMtrFldDrawY , pGridAttr->mnFldDrawY, eUnit );

        aNumFldDivisionX.SetValue( pGridAttr->mnFldDivisionX + 1);
        aNumFldDivisionY.SetValue( pGridAttr->mnFldDivisionY + 1);
    }

    ChangeGridsnapHdl_Impl( &aCbxUseGridsnap );
    mbAttrModified = false;
}

// -----------------------------------------------------------------------

void SvxGridTabPage::ActivatePage( const SfxItemSet& rSet )
{
    const SfxPoolItem* pAttr = NULL;
    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_GRID_OPTIONS , sal_False,
                                    (const SfxPoolItem**)&pAttr ))
    {
        const SvxGridItem* pGridAttr = (SvxGridItem*) pAttr;
        aCbxUseGridsnap.Check( pGridAttr->mbUseGridsnap == 1 );

        ChangeGridsnapHdl_Impl( &aCbxUseGridsnap );
    }

    // Metrik ggfs. aendern (da TabPage im Dialog liegt,
    // wo die Metrik eingestellt werden kann
    //sal_uInt16 nWhich = GetWhich( SID_ATTR_METRIC );
    //if( rSet.GetItemState( GetWhich( SID_ATTR_METRIC ) ) >= SFX_ITEM_AVAILABLE )
    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_METRIC , sal_False,
                                    (const SfxPoolItem**)&pAttr ))
    {
        const SfxUInt16Item* pItem = (SfxUInt16Item*) pAttr;

        FieldUnit eFUnit = (FieldUnit)(long)pItem->GetValue();

        if( eFUnit != aMtrFldDrawX.GetUnit() )
        {
            // Metriken einstellen
            long nFirst, nLast, nMin, nMax;
            long nVal = static_cast<long>(aMtrFldDrawX.Denormalize( aMtrFldDrawX.GetValue( FUNIT_TWIP ) ));

            lcl_GetMinMax(aMtrFldDrawX, nFirst, nLast, nMin, nMax);
            SetFieldUnit( aMtrFldDrawX, eFUnit, sal_True );
            lcl_SetMinMax(aMtrFldDrawX, nFirst, nLast, nMin, nMax);

            aMtrFldDrawX.SetValue( aMtrFldDrawX.Normalize( nVal ), FUNIT_TWIP );

            nVal = static_cast<long>(aMtrFldDrawY.Denormalize( aMtrFldDrawY.GetValue( FUNIT_TWIP ) ));
            lcl_GetMinMax(aMtrFldDrawY, nFirst, nLast, nMin, nMax);
            SetFieldUnit( aMtrFldDrawY, eFUnit, sal_True );
            lcl_SetMinMax(aMtrFldDrawY, nFirst, nLast, nMin, nMax);
            aMtrFldDrawY.SetValue( aMtrFldDrawY.Normalize( nVal ), FUNIT_TWIP );

        }
    }
}

// -----------------------------------------------------------------------
int SvxGridTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( *_pSet );
    return( LEAVE_PAGE );
}
//------------------------------------------------------------------------
IMPL_LINK( SvxGridTabPage, ChangeDrawHdl_Impl, MetricField *, pField )
{
    mbAttrModified = true;
    if( aCbxSynchronize.IsChecked() )
    {
        if(pField == &aMtrFldDrawX)
            aMtrFldDrawY.SetValue( aMtrFldDrawX.GetValue() );
        else
            aMtrFldDrawX.SetValue( aMtrFldDrawY.GetValue() );
    }
    return 0;
}
//------------------------------------------------------------------------

IMPL_LINK( SvxGridTabPage, ClickRotateHdl_Impl, void *, EMPTYARG )
{
    if( aCbxRotate.IsChecked() )
        aMtrFldAngle.Enable();
    else
        aMtrFldAngle.Disable();

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxGridTabPage, ChangeDivisionHdl_Impl, NumericField *, pField )
{
    mbAttrModified = true;
    if( aCbxSynchronize.IsChecked() )
    {
        if(&aNumFldDivisionX == pField)
            aNumFldDivisionY.SetValue( aNumFldDivisionX.GetValue() );
        else
            aNumFldDivisionX.SetValue( aNumFldDivisionY.GetValue() );
    }
    return 0;
}
//------------------------------------------------------------------------

IMPL_LINK( SvxGridTabPage, ChangeGridsnapHdl_Impl, void *, EMPTYARG )
{
    mbAttrModified = true;
    return 0;
}


