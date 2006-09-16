/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: copydlg.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 18:35:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif

#include "copydlg.hxx"

#ifndef _SVX_DLGUTIL_HXX //autogen
#include <svx/dlgutil.hxx>
#endif
#ifndef _SFXMODULE_HXX //autogen
#include <sfx2/module.hxx>
#endif

#ifndef _SVX_XCOLORITEM_HXX //autogen
#include <svx/xcolit.hxx>
#endif
#ifndef _SVX_XFLCLIT_HXX //autogen
#include <svx/xflclit.hxx>
#endif
#ifndef _XDEF_HXX //autogen
#include <svx/xdef.hxx>
#endif
#ifndef SVX_XFILLIT0_HXX //autogen
#include <svx/xfillit0.hxx>
#endif
#ifndef _XENUM_HXX //autogen
#include <svx/xenum.hxx>
#endif

#include <sfx2/app.hxx>



#include "sdattr.hxx"

#include "copydlg.hrc"
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
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
    XColorTable* pColTab,
    ::sd::View* pInView )
    : SfxModalDialog     ( pWindow, SdResId( DLG_COPY ) ),
      aFtCopies           ( this, SdResId( FT_COPIES ) ),
      aNumFldCopies       ( this, SdResId( NUM_FLD_COPIES ) ),
      aFtMoveX            ( this, SdResId( FT_MOVE_X ) ),
      aMtrFldMoveX        ( this, SdResId( MTR_FLD_MOVE_X ) ),
      aFtMoveY            ( this, SdResId( FT_MOVE_Y ) ),
      aMtrFldMoveY        ( this, SdResId( MTR_FLD_MOVE_Y ) ),
      aFtAngle            ( this, SdResId( FT_ANGLE ) ),
      aMtrFldAngle        ( this, SdResId( MTR_FLD_ANGLE ) ),
      aGrpMovement        ( this, SdResId( GRP_MOVEMENT ) ),
      aFtWidth            ( this, SdResId( FT_WIDTH ) ),
      aMtrFldWidth        ( this, SdResId( MTR_FLD_WIDTH ) ),
      aFtHeight           ( this, SdResId( FT_HEIGHT ) ),
      aMtrFldHeight       ( this, SdResId( MTR_FLD_HEIGHT ) ),
      aGrpEnlargement     ( this, SdResId( GRP_ENLARGEMENT ) ),
      aFtStartColor       ( this, SdResId( FT_START_COLOR ) ),
      aLbStartColor       ( this, SdResId( LB_START_COLOR ) ),
      aFtEndColor         ( this, SdResId( FT_END_COLOR ) ),
      aLbEndColor         ( this, SdResId( LB_END_COLOR ) ),
      aGrpColor           ( this, SdResId( GRP_COLOR ) ),
      aBtnOK              ( this, SdResId( BTN_OK ) ),
      aBtnCancel          ( this, SdResId( BTN_CANCEL ) ),
      aBtnHelp            ( this, SdResId( BTN_HELP ) ),
      aBtnSetViewData     ( this, SdResId( BTN_SET_VIEWDATA ) ),
      aBtnSetDefault      ( this, SdResId( BTN_SET_DEFAULT ) ),
      rOutAttrs         ( rInAttrs ),
      pColorTab         ( pColTab ),
      pView             ( pInView ),
      eUIUnit(pInView->GetDoc()->GetUIUnit()),
      aUIScale(pInView->GetDoc()->GetUIScale())
{
    FreeResource();

    // Set up the view data button (image and accessible name).
    aBtnSetViewData.SetModeImage( Image( SdResId( IMG_PIPETTE_H ) ), BMP_COLOR_HIGHCONTRAST );
    aBtnSetViewData.SetAccessibleName (aBtnSetViewData.GetQuickHelpText());

    // Farbtabellen
    DBG_ASSERT( pColorTab, "Keine gueltige ColorTable uebergeben!" );
    aLbStartColor.Fill( pColorTab );
    aLbEndColor.CopyEntries( aLbStartColor );

    aLbStartColor.SetSelectHdl( LINK( this, CopyDlg, SelectColorHdl ) );
    aBtnSetViewData.SetClickHdl( LINK( this, CopyDlg, SetViewData ) );
    aBtnSetDefault.SetClickHdl( LINK( this, CopyDlg, SetDefault ) );


    FieldUnit eFUnit( GetModuleFieldUnit() );

    SetFieldUnit( aMtrFldMoveX, eFUnit, TRUE );
    SetFieldUnit( aMtrFldMoveY, eFUnit, TRUE );
    SetFieldUnit( aMtrFldWidth, eFUnit, TRUE );
    SetFieldUnit( aMtrFldHeight, eFUnit, TRUE );

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

    rStr = UniString::CreateFromInt32( aNumFldCopies.GetValue() );
    rStr.Append( TOKEN );

    rStr += UniString::CreateFromInt32( aMtrFldMoveX.GetValue() );
    rStr.Append( TOKEN );

    rStr += UniString::CreateFromInt32( aMtrFldMoveY.GetValue() );
    rStr.Append( TOKEN );

    rStr += UniString::CreateFromInt32( aMtrFldAngle.GetValue() );
    rStr.Append( TOKEN );

    rStr += UniString::CreateFromInt32( aMtrFldWidth.GetValue() );
    rStr.Append( TOKEN );

    rStr += UniString::CreateFromInt32( aMtrFldHeight.GetValue() );
    rStr.Append( TOKEN );

    rStr += UniString::CreateFromInt32( (long)aLbStartColor.GetSelectEntryColor().GetColor() );
    rStr.Append( TOKEN );

    rStr += UniString::CreateFromInt32( (long)aLbEndColor.GetSelectEntryColor().GetColor() );
}

/*************************************************************************
|*
|* Liest uebergebenen Item-Set oder wertet den INI-String aus
|*
\************************************************************************/

IMPL_LINK( CopyDlg, Reset, void*, p )
{
    const SfxPoolItem* pPoolItem = NULL;
    String aStr( GetExtraData() );

    if( aStr.GetTokenCount( TOKEN ) < 8 )
    {
        if( SFX_ITEM_SET == rOutAttrs.GetItemState( ATTR_COPY_NUMBER, TRUE, &pPoolItem ) )
            aNumFldCopies.SetValue( ( ( const SfxUInt16Item* ) pPoolItem )->GetValue() );
        else
            aNumFldCopies.SetValue( 1L );

        long nMoveX = 500L;
        if( SFX_ITEM_SET == rOutAttrs.GetItemState( ATTR_COPY_MOVE_X, TRUE, &pPoolItem ) )
            nMoveX = ( ( ( const SfxInt32Item* ) pPoolItem )->GetValue() );
        SetMetricValue( aMtrFldMoveX, Fraction(nMoveX) / aUIScale, SFX_MAPUNIT_100TH_MM);

        long nMoveY = 500L;
        if( SFX_ITEM_SET == rOutAttrs.GetItemState( ATTR_COPY_MOVE_Y, TRUE, &pPoolItem ) )
            nMoveY = ( ( ( const SfxInt32Item* ) pPoolItem )->GetValue() );
        SetMetricValue( aMtrFldMoveY, Fraction(nMoveY) / aUIScale, SFX_MAPUNIT_100TH_MM);

        if( SFX_ITEM_SET == rOutAttrs.GetItemState( ATTR_COPY_ANGLE, TRUE, &pPoolItem ) )
            aMtrFldAngle.SetValue( ( ( const SfxInt32Item* ) pPoolItem )->GetValue() );
        else
            aMtrFldAngle.SetValue( 0L );

        long nWidth = 0L;
        if( SFX_ITEM_SET == rOutAttrs.GetItemState( ATTR_COPY_WIDTH, TRUE, &pPoolItem ) )
            nWidth = ( ( ( const SfxInt32Item* ) pPoolItem )->GetValue() );
        SetMetricValue( aMtrFldWidth, Fraction(nWidth) / aUIScale, SFX_MAPUNIT_100TH_MM);

        long nHeight = 0L;
        if( SFX_ITEM_SET == rOutAttrs.GetItemState( ATTR_COPY_HEIGHT, TRUE, &pPoolItem ) )
            nHeight = ( ( ( const SfxInt32Item* ) pPoolItem )->GetValue() );
        SetMetricValue( aMtrFldHeight, Fraction(nHeight) / aUIScale, SFX_MAPUNIT_100TH_MM);

        if( SFX_ITEM_SET == rOutAttrs.GetItemState( ATTR_COPY_START_COLOR, TRUE, &pPoolItem ) )
        {
            Color aColor = ( ( const XColorItem* ) pPoolItem )->GetColorValue();
            aLbStartColor.SelectEntry( aColor );
            aLbEndColor.SelectEntry( aColor );
        }
        else
        {
            aLbStartColor.SetNoSelection();
            aLbEndColor.SetNoSelection();
            aLbEndColor.Disable();
            aFtEndColor.Disable();
        }
    }
    else
    {
        long nTmp;
        nTmp = (long)aStr.GetToken( 0, TOKEN ).ToInt32();
        aNumFldCopies.SetValue( nTmp );

        nTmp = (long)aStr.GetToken( 1, TOKEN ).ToInt32();
        aMtrFldMoveX.SetValue( nTmp );

        nTmp = (long)aStr.GetToken( 2, TOKEN ).ToInt32();
        aMtrFldMoveY.SetValue( nTmp );

        nTmp = (long)aStr.GetToken( 3, TOKEN ).ToInt32();
        aMtrFldAngle.SetValue( nTmp );

        nTmp = (long)aStr.GetToken( 4, TOKEN ).ToInt32();
        aMtrFldWidth.SetValue( nTmp );

        nTmp = (long)aStr.GetToken( 5, TOKEN ).ToInt32();
        aMtrFldHeight.SetValue( nTmp );

        nTmp = (long)aStr.GetToken( 6, TOKEN ).ToInt32();
        aLbStartColor.SelectEntry( Color( nTmp ) );

        nTmp = (long)aStr.GetToken( 7, TOKEN ).ToInt32();
        aLbEndColor.SelectEntry( Color( nTmp ) );
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
    long nMoveX = Fraction( GetCoreValue( aMtrFldMoveX, SFX_MAPUNIT_100TH_MM) ) * aUIScale;
    long nMoveY = Fraction( GetCoreValue( aMtrFldMoveY, SFX_MAPUNIT_100TH_MM) ) * aUIScale;
    long nHeight = Fraction( GetCoreValue( aMtrFldHeight, SFX_MAPUNIT_100TH_MM) ) * aUIScale;
    long nWidth  = Fraction( GetCoreValue( aMtrFldWidth, SFX_MAPUNIT_100TH_MM) ) * aUIScale;

    rOutAttrs.Put( SfxUInt16Item( ATTR_COPY_NUMBER, (UINT16) aNumFldCopies.GetValue() ) );
    rOutAttrs.Put( SfxInt32Item( ATTR_COPY_MOVE_X, nMoveX ) );
    rOutAttrs.Put( SfxInt32Item( ATTR_COPY_MOVE_Y, nMoveY ) );
    rOutAttrs.Put( SfxInt32Item( ATTR_COPY_ANGLE, aMtrFldAngle.GetValue() ) );
    rOutAttrs.Put( SfxInt32Item( ATTR_COPY_WIDTH, nWidth ) );
    rOutAttrs.Put( SfxInt32Item( ATTR_COPY_HEIGHT, nHeight ) );

    if( aLbStartColor.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND )
    {
        XColorItem aXColorItem( ATTR_COPY_START_COLOR, aLbStartColor.GetSelectEntry(),
                                    aLbStartColor.GetSelectEntryColor() );
        rOutAttrs.Put( aXColorItem );
    }
    if( aLbEndColor.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND )
    {
        XColorItem aXColorItem( ATTR_COPY_END_COLOR, aLbEndColor.GetSelectEntry(),
                                    aLbEndColor.GetSelectEntryColor() );
        rOutAttrs.Put( aXColorItem );
    }
}

/*************************************************************************
|*
|* Enabled und selektiert Endfarben-LB
|*
\************************************************************************/

IMPL_LINK( CopyDlg, SelectColorHdl, void *, p )
{
    USHORT nPos = aLbStartColor.GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND &&
        !aLbEndColor.IsEnabled() )
    {
        aLbEndColor.SelectEntryPos( nPos );
        aLbEndColor.Enable();
        aFtEndColor.Enable();
    }
    return 0;
}

/*************************************************************************
|* Setzt Werte der Selektion
\************************************************************************/

IMPL_LINK( CopyDlg, SetViewData, void*, EMPTYARG )
{
    Rectangle aRect = pView->GetAllMarkedRect();

    SetMetricValue( aMtrFldMoveX, Fraction( aRect.GetWidth() ) /
                                    aUIScale, SFX_MAPUNIT_100TH_MM);
    SetMetricValue( aMtrFldMoveY, Fraction( aRect.GetHeight() ) /
                                    aUIScale, SFX_MAPUNIT_100TH_MM);

    // Farb-Attribut setzen
    const SfxPoolItem*  pPoolItem = NULL;
    if( SFX_ITEM_SET == rOutAttrs.GetItemState( ATTR_COPY_START_COLOR, TRUE, &pPoolItem ) )
    {
        Color aColor = ( ( const XColorItem* ) pPoolItem )->GetColorValue();
        aLbStartColor.SelectEntry( aColor );
    }

    return 0;
}

/*************************************************************************
|* Setzt Werte auf Standard
\************************************************************************/

IMPL_LINK( CopyDlg, SetDefault, void*, EMPTYARG )
{
    aNumFldCopies.SetValue( 1L );

    long nValue = 500L;
    SetMetricValue( aMtrFldMoveX, Fraction(nValue) / aUIScale, SFX_MAPUNIT_100TH_MM);
    SetMetricValue( aMtrFldMoveY, Fraction(nValue) / aUIScale, SFX_MAPUNIT_100TH_MM);

    nValue = 0L;
    aMtrFldAngle.SetValue( nValue );
    SetMetricValue( aMtrFldWidth, Fraction(nValue) / aUIScale, SFX_MAPUNIT_100TH_MM);
    SetMetricValue( aMtrFldHeight, Fraction(nValue) / aUIScale, SFX_MAPUNIT_100TH_MM);

    // Farb-Attribut setzen
    const SfxPoolItem*  pPoolItem = NULL;
    if( SFX_ITEM_SET == rOutAttrs.GetItemState( ATTR_COPY_START_COLOR, TRUE, &pPoolItem ) )
    {
        Color aColor = ( ( const XColorItem* ) pPoolItem )->GetColorValue();
        aLbStartColor.SelectEntry( aColor );
        aLbEndColor.SelectEntry( aColor );
    }

    return 0;
}


} // end of namespace sd
