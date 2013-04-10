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

#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif

#include "copydlg.hxx"
#include <svx/dlgutil.hxx>
#include <sfx2/module.hxx>
#include <svx/xcolit.hxx>
#include <svx/xflclit.hxx>
#include <svx/xdef.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xenum.hxx>

#include <sfx2/app.hxx>



#include "sdattr.hxx"

#include "copydlg.hrc"
#include "View.hxx"
#include "sdresid.hxx"
#include "drawdoc.hxx"
#include "res_bmp.hrc"

namespace sd {

#define TOKEN (sal_Unicode(';'))

/*************************************************************************
|*
|* Dialog zum Kopieren von Objekten
|*
\************************************************************************/

CopyDlg::CopyDlg(
    ::Window* pWindow,
    const SfxItemSet& rInAttrs,
    XColorList* pColTab,
    ::sd::View* pInView )
    : SfxModalDialog     ( pWindow, SdResId( DLG_COPY ) ),
      maFtCopies           ( this, SdResId( FT_COPIES ) ),
      maNumFldCopies       ( this, SdResId( NUM_FLD_COPIES ) ),
      maBtnSetViewData     ( this, SdResId( BTN_SET_VIEWDATA ) ),
      maGrpMovement        ( this, SdResId( GRP_MOVEMENT ) ),
      maFtMoveX            ( this, SdResId( FT_MOVE_X ) ),
      maMtrFldMoveX        ( this, SdResId( MTR_FLD_MOVE_X ) ),
      maFtMoveY            ( this, SdResId( FT_MOVE_Y ) ),
      maMtrFldMoveY        ( this, SdResId( MTR_FLD_MOVE_Y ) ),
      maFtAngle            ( this, SdResId( FT_ANGLE ) ),
      maMtrFldAngle        ( this, SdResId( MTR_FLD_ANGLE ) ),
      maGrpEnlargement     ( this, SdResId( GRP_ENLARGEMENT ) ),
      maFtWidth            ( this, SdResId( FT_WIDTH ) ),
      maMtrFldWidth        ( this, SdResId( MTR_FLD_WIDTH ) ),
      maFtHeight           ( this, SdResId( FT_HEIGHT ) ),
      maMtrFldHeight       ( this, SdResId( MTR_FLD_HEIGHT ) ),
      maGrpColor           ( this, SdResId( GRP_COLOR ) ),
      maFtStartColor       ( this, SdResId( FT_START_COLOR ) ),
      maLbStartColor       ( this, SdResId( LB_START_COLOR ) ),
      maFtEndColor         ( this, SdResId( FT_END_COLOR ) ),
      maLbEndColor         ( this, SdResId( LB_END_COLOR ) ),
      maBtnOK              ( this, SdResId( BTN_OK ) ),
      maBtnCancel          ( this, SdResId( BTN_CANCEL ) ),
      maBtnHelp            ( this, SdResId( BTN_HELP ) ),
      maBtnSetDefault      ( this, SdResId( BTN_SET_DEFAULT ) ),
      mrOutAttrs            ( rInAttrs ),
      mpColorTab            ( pColTab ),
      maUIScale(pInView->GetDoc()->GetUIScale()),
      mpView                ( pInView )
{
    FreeResource();

    // Set up the view data button (image and accessible name).
    maBtnSetViewData.SetModeImage( Image( SdResId( IMG_PIPETTE_H ) ), BMP_COLOR_HIGHCONTRAST );
    maBtnSetViewData.SetAccessibleName (maBtnSetViewData.GetQuickHelpText());

    // Farbtabellen
    DBG_ASSERT( mpColorTab, "Keine gueltige ColorTable uebergeben!" );
    maLbStartColor.Fill( mpColorTab );
    maLbEndColor.CopyEntries( maLbStartColor );

    maLbStartColor.SetSelectHdl( LINK( this, CopyDlg, SelectColorHdl ) );
    maBtnSetViewData.SetClickHdl( LINK( this, CopyDlg, SetViewData ) );
    maBtnSetDefault.SetClickHdl( LINK( this, CopyDlg, SetDefault ) );


    FieldUnit eFUnit( SfxModule::GetCurrentFieldUnit() );

    SetFieldUnit( maMtrFldMoveX, eFUnit, sal_True );
    SetFieldUnit( maMtrFldMoveY, eFUnit, sal_True );
    SetFieldUnit( maMtrFldWidth, eFUnit, sal_True );
    SetFieldUnit( maMtrFldHeight, eFUnit, sal_True );

    Reset(0L);
}

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

CopyDlg::~CopyDlg()
{
    String& rStr = GetExtraData();

    rStr = UniString::CreateFromInt64( maNumFldCopies.GetValue() );
    rStr.Append( TOKEN );

    rStr += UniString::CreateFromInt64( maMtrFldMoveX.GetValue() );
    rStr.Append( TOKEN );

    rStr += UniString::CreateFromInt64( maMtrFldMoveY.GetValue() );
    rStr.Append( TOKEN );

    rStr += UniString::CreateFromInt64( maMtrFldAngle.GetValue() );
    rStr.Append( TOKEN );

    rStr += UniString::CreateFromInt64( maMtrFldWidth.GetValue() );
    rStr.Append( TOKEN );

    rStr += UniString::CreateFromInt64( maMtrFldHeight.GetValue() );
    rStr.Append( TOKEN );

    rStr += UniString::CreateFromInt32( (long)maLbStartColor.GetSelectEntryColor().GetColor() );
    rStr.Append( TOKEN );

    rStr += UniString::CreateFromInt32( (long)maLbEndColor.GetSelectEntryColor().GetColor() );
}

/*************************************************************************
|*
|* Liest uebergebenen Item-Set oder wertet den INI-String aus
|*
\************************************************************************/

IMPL_LINK( CopyDlg, Reset, void*, EMPTYARG )
{
    const SfxPoolItem* pPoolItem = NULL;
    String aStr( GetExtraData() );

    if( aStr.GetTokenCount( TOKEN ) < 8 )
    {
        if( SFX_ITEM_SET == mrOutAttrs.GetItemState( ATTR_COPY_NUMBER, sal_True, &pPoolItem ) )
            maNumFldCopies.SetValue( ( ( const SfxUInt16Item* ) pPoolItem )->GetValue() );
        else
            maNumFldCopies.SetValue( 1L );

        long nMoveX = 500L;
        if( SFX_ITEM_SET == mrOutAttrs.GetItemState( ATTR_COPY_MOVE_X, sal_True, &pPoolItem ) )
            nMoveX = ( ( ( const SfxInt32Item* ) pPoolItem )->GetValue() );
        SetMetricValue( maMtrFldMoveX, Fraction(nMoveX) / maUIScale, SFX_MAPUNIT_100TH_MM);

        long nMoveY = 500L;
        if( SFX_ITEM_SET == mrOutAttrs.GetItemState( ATTR_COPY_MOVE_Y, sal_True, &pPoolItem ) )
            nMoveY = ( ( ( const SfxInt32Item* ) pPoolItem )->GetValue() );
        SetMetricValue( maMtrFldMoveY, Fraction(nMoveY) / maUIScale, SFX_MAPUNIT_100TH_MM);

        if( SFX_ITEM_SET == mrOutAttrs.GetItemState( ATTR_COPY_ANGLE, sal_True, &pPoolItem ) )
            maMtrFldAngle.SetValue( ( ( const SfxInt32Item* ) pPoolItem )->GetValue() );
        else
            maMtrFldAngle.SetValue( 0L );

        long nWidth = 0L;
        if( SFX_ITEM_SET == mrOutAttrs.GetItemState( ATTR_COPY_WIDTH, sal_True, &pPoolItem ) )
            nWidth = ( ( ( const SfxInt32Item* ) pPoolItem )->GetValue() );
        SetMetricValue( maMtrFldWidth, Fraction(nWidth) / maUIScale, SFX_MAPUNIT_100TH_MM);

        long nHeight = 0L;
        if( SFX_ITEM_SET == mrOutAttrs.GetItemState( ATTR_COPY_HEIGHT, sal_True, &pPoolItem ) )
            nHeight = ( ( ( const SfxInt32Item* ) pPoolItem )->GetValue() );
        SetMetricValue( maMtrFldHeight, Fraction(nHeight) / maUIScale, SFX_MAPUNIT_100TH_MM);

        if( SFX_ITEM_SET == mrOutAttrs.GetItemState( ATTR_COPY_START_COLOR, sal_True, &pPoolItem ) )
        {
            Color aColor = ( ( const XColorItem* ) pPoolItem )->GetColorValue();
            maLbStartColor.SelectEntry( aColor );
            maLbEndColor.SelectEntry( aColor );
        }
        else
        {
            maLbStartColor.SetNoSelection();
            maLbEndColor.SetNoSelection();
            maLbEndColor.Disable();
            maFtEndColor.Disable();
        }
    }
    else
    {
        long nTmp;
        nTmp = (long)aStr.GetToken( 0, TOKEN ).ToInt32();
        maNumFldCopies.SetValue( nTmp );

        nTmp = (long)aStr.GetToken( 1, TOKEN ).ToInt32();
        maMtrFldMoveX.SetValue( nTmp );

        nTmp = (long)aStr.GetToken( 2, TOKEN ).ToInt32();
        maMtrFldMoveY.SetValue( nTmp );

        nTmp = (long)aStr.GetToken( 3, TOKEN ).ToInt32();
        maMtrFldAngle.SetValue( nTmp );

        nTmp = (long)aStr.GetToken( 4, TOKEN ).ToInt32();
        maMtrFldWidth.SetValue( nTmp );

        nTmp = (long)aStr.GetToken( 5, TOKEN ).ToInt32();
        maMtrFldHeight.SetValue( nTmp );

        nTmp = (long)aStr.GetToken( 6, TOKEN ).ToInt32();
        maLbStartColor.SelectEntry( Color( nTmp ) );

        nTmp = (long)aStr.GetToken( 7, TOKEN ).ToInt32();
        maLbEndColor.SelectEntry( Color( nTmp ) );
    }

    return 0;
}

/*************************************************************************
|*
|* Fuellt uebergebenen Item-Set mit Dialogbox-Attributen
|*
\************************************************************************/

void CopyDlg::GetAttr( SfxItemSet& rOutAttrs )
{
    long nMoveX = Fraction( GetCoreValue( maMtrFldMoveX, SFX_MAPUNIT_100TH_MM) ) * maUIScale;
    long nMoveY = Fraction( GetCoreValue( maMtrFldMoveY, SFX_MAPUNIT_100TH_MM) ) * maUIScale;
    long nHeight = Fraction( GetCoreValue( maMtrFldHeight, SFX_MAPUNIT_100TH_MM) ) * maUIScale;
    long nWidth  = Fraction( GetCoreValue( maMtrFldWidth, SFX_MAPUNIT_100TH_MM) ) * maUIScale;

    rOutAttrs.Put( SfxUInt16Item( ATTR_COPY_NUMBER, (sal_uInt16) maNumFldCopies.GetValue() ) );
    rOutAttrs.Put( SfxInt32Item( ATTR_COPY_MOVE_X, nMoveX ) );
    rOutAttrs.Put( SfxInt32Item( ATTR_COPY_MOVE_Y, nMoveY ) );
    rOutAttrs.Put( SfxInt32Item( ATTR_COPY_ANGLE, static_cast<sal_Int32>(maMtrFldAngle.GetValue()) ) );
    rOutAttrs.Put( SfxInt32Item( ATTR_COPY_WIDTH, nWidth ) );
    rOutAttrs.Put( SfxInt32Item( ATTR_COPY_HEIGHT, nHeight ) );

    if( maLbStartColor.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND )
    {
        XColorItem aXColorItem( ATTR_COPY_START_COLOR, maLbStartColor.GetSelectEntry(),
                                    maLbStartColor.GetSelectEntryColor() );
        rOutAttrs.Put( aXColorItem );
    }
    if( maLbEndColor.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND )
    {
        XColorItem aXColorItem( ATTR_COPY_END_COLOR, maLbEndColor.GetSelectEntry(),
                                    maLbEndColor.GetSelectEntryColor() );
        rOutAttrs.Put( aXColorItem );
    }
}

/*************************************************************************
|*
|* Enabled und selektiert Endfarben-LB
|*
\************************************************************************/

IMPL_LINK( CopyDlg, SelectColorHdl, void *, EMPTYARG )
{
    sal_uInt16 nPos = maLbStartColor.GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND &&
        !maLbEndColor.IsEnabled() )
    {
        maLbEndColor.SelectEntryPos( nPos );
        maLbEndColor.Enable();
        maFtEndColor.Enable();
    }
    return 0;
}

/*************************************************************************
|* Setzt Werte der Selektion
\************************************************************************/

IMPL_LINK( CopyDlg, SetViewData, void*, EMPTYARG )
{
    Rectangle aRect = mpView->GetAllMarkedRect();

    SetMetricValue( maMtrFldMoveX, Fraction( aRect.GetWidth() ) /
                                    maUIScale, SFX_MAPUNIT_100TH_MM);
    SetMetricValue( maMtrFldMoveY, Fraction( aRect.GetHeight() ) /
                                    maUIScale, SFX_MAPUNIT_100TH_MM);

    // Farb-Attribut setzen
    const SfxPoolItem*  pPoolItem = NULL;
    if( SFX_ITEM_SET == mrOutAttrs.GetItemState( ATTR_COPY_START_COLOR, sal_True, &pPoolItem ) )
    {
        Color aColor = ( ( const XColorItem* ) pPoolItem )->GetColorValue();
        maLbStartColor.SelectEntry( aColor );
    }

    return 0;
}

/*************************************************************************
|* Setzt Werte auf Standard
\************************************************************************/

IMPL_LINK( CopyDlg, SetDefault, void*, EMPTYARG )
{
    maNumFldCopies.SetValue( 1L );

    long nValue = 500L;
    SetMetricValue( maMtrFldMoveX, Fraction(nValue) / maUIScale, SFX_MAPUNIT_100TH_MM);
    SetMetricValue( maMtrFldMoveY, Fraction(nValue) / maUIScale, SFX_MAPUNIT_100TH_MM);

    nValue = 0L;
    maMtrFldAngle.SetValue( nValue );
    SetMetricValue( maMtrFldWidth, Fraction(nValue) / maUIScale, SFX_MAPUNIT_100TH_MM);
    SetMetricValue( maMtrFldHeight, Fraction(nValue) / maUIScale, SFX_MAPUNIT_100TH_MM);

    // Farb-Attribut setzen
    const SfxPoolItem*  pPoolItem = NULL;
    if( SFX_ITEM_SET == mrOutAttrs.GetItemState( ATTR_COPY_START_COLOR, sal_True, &pPoolItem ) )
    {
        Color aColor = ( ( const XColorItem* ) pPoolItem )->GetColorValue();
        maLbStartColor.SelectEntry( aColor );
        maLbEndColor.SelectEntry( aColor );
    }

    return 0;
}


} // end of namespace sd
