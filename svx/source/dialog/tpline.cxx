/*************************************************************************
 *
 *  $RCSfile: tpline.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: pb $ $Date: 2000-10-31 12:42:41 $
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

// include ---------------------------------------------------------------
#ifndef _SVX_SIZEITEM_HXX //autogen
#define ITEMID_SIZE 0
#include <sizeitem.hxx>
#endif
#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFXMODULE_HXX
#include <sfx2/module.hxx>
#endif
#pragma hdrstop

#define _SVX_TPLINE_CXX

#include "dialogs.hrc"
#include "tabline.hrc"
#include "dlgname.hrc"

#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE
#define ITEMID_DASH_LIST        SID_DASH_LIST
#define ITEMID_LINEEND_LIST     SID_LINEEND_LIST

#include "xattr.hxx"
#include "xpool.hxx"
#include "xtable.hxx"
#include "xoutx.hxx"

#include "drawitem.hxx"
#include "tabline.hxx"
#include "dlgname.hxx"
#include "dialmgr.hxx"
#include "dlgutil.hxx"
#include "svdmodel.hxx"

#include "linectrl.hrc"

//#58425# Symbole auf einer Linie (z.B. StarChart) Includes:


#ifdef DEBUG
#define SVX_TRACE(b,a)                                               \
    {                                                                \
        ByteString _aStr( RTL_CONSTASCII_STRINGPARAM( "SvxTrace" ) );\
        _aStr.Append( ByteString::CreateFromInt32( b ) );            \
        _aStr.Append( "->" );                                        \
        _aStr.Append( ByteString::CreateFromInt32( (sal_Int32) this ) );     \
        _aStr.Append( '@' );                                         \
        _aStr.Append( a );                                           \
        DBG_TRACE( _aStr.GetBuffer() );                      \
    }
#else
#define SVX_TRACE(b,a)
#endif


#ifndef _SVDOBJ_HXX //autogen
#include <svdobj.hxx>
#endif
#ifndef _SVDVIEW_HXX //autogen
#include <svdview.hxx>
#endif
#ifndef _SVDMODEL_HXX //autogen
#include <svdmodel.hxx>
#endif

#ifndef _SVX_NUMVSET_HXX //autogen
#include <numvset.hxx>
#endif
#ifndef _SVX_IMPGRF_HXX //autogen
#include <impgrf.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVX_NUMITEM_HXX //autogen
#include <numitem.hxx>
#endif
#ifndef _SVX_SVXENUM_HXX //autogen
#include <svxenum.hxx>
#endif
#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif
#define ITEMID_BRUSH SID_ATTR_BRUSH
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <brshitem.hxx>
#endif
#ifndef _GALLERY_HXX_ //autogen
#include <gallery.hxx>
#endif
#ifndef _SVX_XBITMAP_HXX //autogen
#include <xbitmap.hxx>
#endif
#define MAX_BMP_WIDTH   16
#define MAX_BMP_HEIGHT  16
// define ----------------------------------------------------------------

#define DLGWIN this->GetParent()->GetParent()

#define BITMAP_WIDTH   32
#define BITMAP_HEIGHT  12
#define XOUT_WIDTH    150

// static ----------------------------------------------------------------

static USHORT pLineRanges[] =
{
    XATTR_LINETRANSPARENCE,
    XATTR_LINETRANSPARENCE,
    SID_ATTR_LINE_STYLE,
    SID_ATTR_LINE_ENDCENTER,
    0
};

/*************************************************************************
|*
|*  Dialog zum Aendern der Linien
|*
\************************************************************************/

SvxLineTabPage::SvxLineTabPage
(
    Window* pParent,
    const SfxItemSet& rInAttrs
) :
    SvxTabPage          ( pParent, SVX_RES( RID_SVXPAGE_LINE ), rInAttrs ),
    pXPool              ( (XOutdevItemPool*) rInAttrs.GetPool() ),
    XOut                ( &aCtlPreview ),
    aXLineAttr          ( pXPool ),
    rXLSet              ( aXLineAttr.GetItemSet() ),

    aXLStyle            ( XLINE_DASH ),
    aXWidth             ( 1 ),
    aXColor             ( String(), COL_LIGHTRED ),
    aXDash              ( String(), XDash( XDASH_RECT, 3, 7, 2, 40, 15 ) ),

    aFtColor            ( this, ResId( FT_COLOR ) ),
    aLbColor            ( this, ResId( LB_COLOR ) ),
    aFtLineStyle        ( this, ResId( FT_LINE_STYLE ) ),
    aLbLineStyle        ( this, ResId( LB_LINE_STYLE ) ),
    aFtLineWidth        ( this, ResId( FT_LINE_WIDTH ) ),
    aMtrLineWidth       ( this, ResId( MTR_FLD_LINE_WIDTH ) ),
    aFtTransparent      ( this, ResId( FT_TRANSPARENT ) ),
    aMtrTransparent     ( this, ResId( MTR_LINE_TRANSPARENT ) ),
    aGrpLine            ( this, ResId( GRP_LINE ) ),
    aFtLineEndsStyle    ( this, ResId( FT_LINE_ENDS_STYLE ) ),
    aFtLineEndsWidth    ( this, ResId( FT_LINE_ENDS_WIDTH ) ),
    aLbStartStyle       ( this, ResId( LB_START_STYLE ) ),
    aMtrStartWidth      ( this, ResId( MTR_FLD_START_WIDTH ) ),
    aTsbCenterStart     ( this, ResId( TSB_CENTER_START ) ),
    aLbEndStyle         ( this, ResId( LB_END_STYLE ) ),
    aMtrEndWidth        ( this, ResId( MTR_FLD_END_WIDTH ) ),
    aTsbCenterEnd       ( this, ResId( TSB_CENTER_END ) ),
    aCbxSynchronize     ( this, ResId( CBX_SYNCHRONIZE ) ),
    aGrpLineEnds        ( this, ResId( GRP_LINE_ENDS ) ),
    aCtlPreview         ( this, ResId( CTL_PREVIEW ), &XOut ),
    aGrpPreview         ( this, ResId( GRP_PREVIEW ) ),
    //#58425# Symbole auf einer Linie (z.B. StarChart) ->
    aSymbolWidthFT      ( this, ResId(FT_SYMBOL_WIDTH)),
    aSymbolWidthMF      ( this, ResId(MF_SYMBOL_WIDTH)),
    aSymbolHeightFT     ( this, ResId(FT_SYMBOL_HEIGHT)),
    aSymbolHeightMF     ( this, ResId(MF_SYMBOL_HEIGHT)),
    aSymbolGB           ( this, ResId(GB_SYMBOL_FORMAT)),
    aSymbolRatioCB      ( this, ResId(CB_SYMBOL_RATIO)),
    aSymbolMB           ( this, ResId(MB_SYMBOL_BITMAP)),
    nSymbolType(SVX_SYMBOLTYPE_UNKNOWN), //unbekannt bzw. unchanged
    bLastWidthModified(FALSE),
    aSymbolLastSize(Size(0,0)),
    bSymbols(FALSE),
    pSymbolList(NULL),
    nNumMenuGalleryItems(0),
    pSymbolAttr(NULL),
    bNewSize(FALSE),
    //#58425# Symbole auf einer Linie (z.B. StarChart) <-

    pPageType           ( NULL ),
    rOutAttrs           ( rInAttrs )

{
    FreeResource();

    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    // Metrik einstellen
    FieldUnit eFUnit = GetModuleFieldUnit( &rInAttrs );

    switch ( eFUnit )
    {
        case FUNIT_M:
        case FUNIT_KM:
            eFUnit = FUNIT_MM;
            // no break -> we now have mm
        case FUNIT_MM:
            aMtrLineWidth.SetSpinSize( 50 );
            aMtrStartWidth.SetSpinSize( 50 );
            aMtrEndWidth.SetSpinSize( 50 );
            break;

            case FUNIT_INCH:
            aMtrLineWidth.SetSpinSize( 2 );
            aMtrStartWidth.SetSpinSize( 2 );
            aMtrEndWidth.SetSpinSize( 2 );
            break;
    }
    SetFieldUnit( aMtrLineWidth, eFUnit );
    SetFieldUnit( aMtrStartWidth, eFUnit );
    SetFieldUnit( aMtrEndWidth, eFUnit );

    // PoolUnit ermitteln
    SfxItemPool* pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool?" );
    ePoolUnit = pPool->GetMetric( SID_ATTR_LINE_WIDTH );

    aLbLineStyle.SetSelectHdl(
        LINK( this, SvxLineTabPage, ClickInvisibleHdl_Impl ) );
    aLbColor.SetSelectHdl(
        LINK( this, SvxLineTabPage, ChangePreviewHdl_Impl ) );
    aMtrLineWidth.SetModifyHdl(
        LINK( this, SvxLineTabPage, ChangePreviewHdl_Impl ) );
    aMtrTransparent.SetModifyHdl(
        LINK( this, SvxLineTabPage, ChangeTransparentHdl_Impl ) );

    Link aStart = LINK( this, SvxLineTabPage, ChangeStartHdl_Impl );
    Link aEnd = LINK( this, SvxLineTabPage, ChangeEndHdl_Impl );
    aLbStartStyle.SetSelectHdl( aStart );
    aLbEndStyle.SetSelectHdl( aEnd );
    aMtrStartWidth.SetModifyHdl( aStart );
    aMtrEndWidth.SetModifyHdl( aEnd );
    aTsbCenterStart.SetClickHdl( aStart );
    aTsbCenterEnd.SetClickHdl( aEnd );

    pColorTab = NULL;
    pDashList = NULL;
    pLineEndList = NULL;


    //#58425# Symbole auf einer Linie (z.B. StarChart) , MB-Handler setzen
    aSymbolMB.SetSelectHdl(LINK(this, SvxLineTabPage, GraphicHdl_Impl));
    aSymbolMB.SetClickHdl(LINK(this, SvxLineTabPage, MenuCreateHdl_Impl));
    aSymbolWidthMF.SetModifyHdl(LINK(this, SvxLineTabPage, SizeHdl_Impl));
    aSymbolHeightMF.SetModifyHdl(LINK(this, SvxLineTabPage, SizeHdl_Impl));
    aSymbolRatioCB.SetClickHdl(LINK(this, SvxLineTabPage, RatioHdl_Impl));

    aSymbolRatioCB.Check(TRUE);
    ShowSymbolControls(FALSE);

    // #63083#
    nActLineWidth = -1;
}
//#58425# Symbole auf einer Linie (z.B. StarChart) , Symbol-Controls aktivieren
void SvxLineTabPage::ShowSymbolControls(BOOL bOn)
{
    bSymbols=bOn;
    aSymbolWidthFT.Show(bOn);
    aSymbolWidthMF.Show(bOn);
    aSymbolHeightFT.Show(bOn);
    aSymbolHeightMF.Show(bOn);
    aSymbolGB.Show(bOn);
    aSymbolRatioCB.Show(bOn);
    aSymbolMB.Show(bOn);
    aCtlPreview.ShowSymbol(bOn);
}
// -----------------------------------------------------------------------
//#58425# Symbole auf einer Linie (z.B. StarChart) , dtor neu!
SvxLineTabPage::~SvxLineTabPage()
{
    delete aSymbolMB.GetPopupMenu()->GetPopupMenu( MN_GALLERY );

    if(pSymbolList)
        delete aSymbolMB.GetPopupMenu()->GetPopupMenu( MN_SYMBOLS );

    String* pStr = (String*)aGrfNames.First();
    while( pStr )
    {
        delete pStr;
        pStr = (String*)aGrfNames.Next();
    }
    SvxBmpItemInfo* pInfo = (SvxBmpItemInfo*)aGrfBrushItems.First();
    while( pInfo )
    {
        delete pInfo->pBrushItem;
        delete pInfo;
        pInfo = (SvxBmpItemInfo*)aGrfBrushItems.Next();
    }
}
void SvxLineTabPage::Construct()
{
    ResMgr* pMgr = DIALOG_MGR();
    // Farbtabelle
    aLbColor.Fill( pColorTab );
    // Linienstile
    aLbLineStyle.InsertEntry( String( ResId( RID_SVXSTR_INVISIBLE, pMgr ) ) );

    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    Bitmap aBitmap ( SVX_RES ( RID_SVXCTRL_LINECTRL ) );
    Color aColorOld ( 0xFF, 0xFF, 0xFF );
    Color aColorNew = rStyles.GetWindowColor();
    aBitmap.Replace ( aColorOld, aColorNew );
    Image aSolidLine ( aBitmap );
    aLbLineStyle.InsertEntry( String( ResId( RID_SVXSTR_SOLID, pMgr ) ), aSolidLine );
    aLbLineStyle.Fill( pDashList );
    // LinienEndenStile
    String sNone( ResId( RID_SVXSTR_NONE, pMgr ) );
    aLbStartStyle.InsertEntry( sNone );
    aLbStartStyle.Fill( pLineEndList );
    aLbEndStyle.InsertEntry( sNone );
    aLbEndStyle.Fill( pLineEndList, FALSE );
}

// -----------------------------------------------------------------------

void SvxLineTabPage::ActivatePage( const SfxItemSet& rSet )
{
    if( *pDlgType == 0 && pDashList ) // Linien-Dialog
    {
        ResMgr* pMgr = DIALOG_MGR();
        int nPos;
        int nCount;

        // Dashliste
        if( ( *pnDashListState & CT_MODIFIED ) ||
            ( *pnDashListState & CT_CHANGED ) )
        {
            if( *pnDashListState & CT_CHANGED )
                pDashList = ( (SvxLineTabDialog*) DLGWIN )->
                                        GetNewDashList();
            *pnDashListState = CT_NONE;

            // Styleliste
            nPos = aLbLineStyle.GetSelectEntryPos();

            aLbLineStyle.Clear();
            aLbLineStyle.InsertEntry(
                String( ResId( RID_SVXSTR_INVISIBLE, pMgr ) ) );
            aLbLineStyle.InsertEntry(
                String( ResId( RID_SVXSTR_SOLID, pMgr ) ) );
            aLbLineStyle.Fill( pDashList );
            nCount = aLbLineStyle.GetEntryCount();

            if ( nCount == 0 )
                ; // Dieser Fall sollte nicht auftreten
            else if( nCount <= nPos )
                aLbLineStyle.SelectEntryPos( 0 );
            else
                aLbLineStyle.SelectEntryPos( nPos );
            // SelectStyleHdl_Impl( this );
        }

        // Ermitteln (evtl. abschneiden) des Namens und in
        // der GroupBox darstellen
        String          aString( ResId( RID_SVXSTR_TABLE, pMgr ) ); aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );
        INetURLObject   aURLDash( pDashList->GetName(), INET_PROT_FILE );

        if ( aURLDash.getBase().Len() > 18 )
        {
            aString += aURLDash.getBase().Copy( 0, 15 );
            aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
        }
        else
            aString += aURLDash.getBase();

        aGrpLine.SetText( aString );

        // LineEndliste
        if( ( *pnLineEndListState & CT_MODIFIED ) ||
            ( *pnLineEndListState & CT_CHANGED ) )
        {
            if( *pnLineEndListState & CT_CHANGED )
                pLineEndList = ( (SvxLineTabDialog*) DLGWIN )->
                                        GetNewLineEndList();
            *pnLineEndListState = CT_NONE;

            nPos = aLbLineStyle.GetSelectEntryPos();
            String sNone( ResId( RID_SVXSTR_NONE, pMgr ) );
            aLbStartStyle.Clear();
            aLbStartStyle.InsertEntry( sNone );

            aLbStartStyle.Fill( pLineEndList );
            nCount = aLbStartStyle.GetEntryCount();
            if( nCount == 0 )
                ; // Dieser Fall sollte nicht auftreten
            else if( nCount <= nPos )
                aLbStartStyle.SelectEntryPos( 0 );
            else
                aLbStartStyle.SelectEntryPos( nPos );

            aLbEndStyle.Clear();
            aLbEndStyle.InsertEntry( sNone );

            aLbEndStyle.Fill( pLineEndList, FALSE );
            nCount = aLbEndStyle.GetEntryCount();

            if( nCount == 0 )
                ; // Dieser Fall sollte nicht auftreten
            else if( nCount <= nPos )
                aLbEndStyle.SelectEntryPos( 0 );
            else
                aLbEndStyle.SelectEntryPos( nPos );
        }
        // Ermitteln (evtl. abschneiden) des Namens und in
        // der GroupBox darstellen
        aString = String( ResId( RID_SVXSTR_TABLE, pMgr ) ); aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );
        INetURLObject   aURLEnd( pLineEndList->GetName(), INET_PROT_FILE );

        if ( aURLEnd.getBase().Len() > 18 )
        {
            aString += aURLEnd.getBase().Copy( 0, 15 );
            aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
        }
        else
            aString += aURLEnd.getBase();

        aGrpLineEnds.SetText( aString );

        // Auswertung, ob von einer anderen TabPage ein anderer Fuelltyp gesetzt wurde
        if( aLbLineStyle.GetSelectEntryPos() != 0 )
        {
            if( *pPageType == 2 ) // 1
            {
                aLbLineStyle.SelectEntryPos( *pPosDashLb + 2 ); // +2 wegen SOLID und INVLISIBLE
                ChangePreviewHdl_Impl( this );
            }
            if( *pPageType == 3 )
            {
                aLbStartStyle.SelectEntryPos( *pPosLineEndLb + 1 );// +1 wegen SOLID
                aLbEndStyle.SelectEntryPos( *pPosLineEndLb + 1 );// +1 wegen SOLID
                ChangePreviewHdl_Impl( this );
            }
        }
        *pPageType = 0;
    }
    // Seite existiert im Ctor noch nicht, deswegen hier!

    else if ( *pDlgType == 1100 || // Chart-Dialog
              *pDlgType == 1101 )
    {
        aFtLineEndsStyle.Hide();
        aFtLineEndsWidth.Hide();
        aLbStartStyle.Hide();
        aMtrStartWidth.Hide();
        aTsbCenterStart.Hide();
        aLbEndStyle.Hide();
        aMtrEndWidth.Hide();
        aTsbCenterEnd.Hide();
        aCbxSynchronize.Hide();
        aGrpLineEnds.Hide();
    }
}

// -----------------------------------------------------------------------

int SvxLineTabPage::DeactivatePage( SfxItemSet* pSet )
{
    if( *pDlgType == 0 ) // Linien-Dialog
    {
        *pPageType = 1; // evtl. fuer Erweiterungen
        *pPosDashLb = aLbLineStyle.GetSelectEntryPos() - 2;// erster Eintrag SOLID !!!
        USHORT nPos = aLbStartStyle.GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
            nPos--;
        *pPosLineEndLb = nPos;
    }

    FillItemSet( *pSet );

    return( LEAVE_PAGE );
}

// -----------------------------------------------------------------------

BOOL SvxLineTabPage::FillItemSet( SfxItemSet& rAttrs )
{
    const SfxPoolItem* pOld = NULL;
    USHORT  nPos;
    BOOL    bModified = FALSE;

    // Um evtl. Modifikationen der Liste vorzubeugen
    // werden Items anderer Seiten nicht gesetzt
    if( *pDlgType != 0 || *pPageType != 2 )
    {
        nPos = aLbLineStyle.GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND &&
            nPos != aLbLineStyle.GetSavedValue() )
        {
            XLineStyleItem* pStyleItem = NULL;

            if( nPos == 0 )
                pStyleItem = new XLineStyleItem( XLINE_NONE );
            else if( nPos == 1 )
                pStyleItem = new XLineStyleItem( XLINE_SOLID );
            else
            {
                pStyleItem = new XLineStyleItem( XLINE_DASH );

                // Zusaetzliche Sicherheit
                if( pDashList->Count() > (long) ( nPos - 2 ) )
                {
                    XLineDashItem aDashItem( aLbLineStyle.GetSelectEntry(),
                                        pDashList->Get( nPos - 2 )->GetDash() );
                    pOld = GetOldItem( rAttrs, XATTR_LINEDASH );
                    if ( !pOld || !( *(const XLineDashItem*)pOld == aDashItem ) )
                    {
                        rAttrs.Put( aDashItem );
                        bModified = TRUE;
                    }
                }
            }
            pOld = GetOldItem( rAttrs, XATTR_LINESTYLE );
            if ( !pOld || !( *(const XLineStyleItem*)pOld == *pStyleItem ) )
            {
                rAttrs.Put( *pStyleItem );
                bModified = TRUE;
            }
            delete pStyleItem;
        }
    }
    // Linienbreite
    // GetSavedValue() liefert String!
    if( aMtrLineWidth.GetText() != aMtrLineWidth.GetSavedValue() )
    {
        XLineWidthItem aItem( GetCoreValue( aMtrLineWidth, ePoolUnit ) );
        pOld = GetOldItem( rAttrs, XATTR_LINEWIDTH );
        if ( !pOld || !( *(const XLineWidthItem*)pOld == aItem ) )
        {
            rAttrs.Put( aItem );
            bModified = TRUE;
        }
    }
    //Breite Linienanfang
    if( aMtrStartWidth.GetText() != aMtrStartWidth.GetSavedValue() )
    {
        XLineStartWidthItem aItem( GetCoreValue( aMtrStartWidth, ePoolUnit ) );
        pOld = GetOldItem( rAttrs, XATTR_LINESTARTWIDTH );
        if ( !pOld || !( *(const XLineStartWidthItem*)pOld == aItem ) )
        {
            rAttrs.Put( aItem );
            bModified = TRUE;
        }
    }
    //Breite Linienende
    if( aMtrEndWidth.GetText() != aMtrEndWidth.GetSavedValue() )
    {
        XLineEndWidthItem aItem( GetCoreValue( aMtrEndWidth, ePoolUnit ) );
        pOld = GetOldItem( rAttrs, XATTR_LINEENDWIDTH );
        if ( !pOld || !( *(const XLineEndWidthItem*)pOld == aItem ) )
        {
            rAttrs.Put( aItem );
            bModified = TRUE;
        }
    }

    // Linienfarbe
    if( aLbColor.GetSelectEntryPos() != aLbColor.GetSavedValue() )
    {
        XLineColorItem aItem( aLbColor.GetSelectEntry(),
                              aLbColor.GetSelectEntryColor() );
        pOld = GetOldItem( rAttrs, XATTR_LINECOLOR );
        if ( !pOld || !( *(const XLineColorItem*)pOld == aItem ) )
        {
            rAttrs.Put( aItem );
            bModified = TRUE;
        }
    }

    if( *pDlgType != 0 || *pPageType != 3 )
    {
        // Linienanfang
        nPos = aLbStartStyle.GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND &&
            nPos != aLbStartStyle.GetSavedValue() )
        {
            XLineStartItem* pItem = NULL;
            if( nPos == 0 )
                pItem = new XLineStartItem();
            else if( pLineEndList->Count() > (long) ( nPos - 1 ) )
                pItem = new XLineStartItem( aLbStartStyle.GetSelectEntry(),
                            pLineEndList->Get( nPos - 1 )->GetLineEnd() );
            pOld = GetOldItem( rAttrs, XATTR_LINESTART );
            if( pItem &&
                ( !pOld || !( *(const XLineEndItem*)pOld == *pItem ) ) )
            {
                rAttrs.Put( *pItem );
                bModified = TRUE;
            }
            delete pItem;
        }
        // Linienende
        nPos = aLbEndStyle.GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND &&
            nPos != aLbEndStyle.GetSavedValue() )
        {
            XLineEndItem* pItem = NULL;
            if( nPos == 0 )
                pItem = new XLineEndItem();
            else if( pLineEndList->Count() > (long) ( nPos - 1 ) )
                pItem = new XLineEndItem( aLbEndStyle.GetSelectEntry(),
                            pLineEndList->Get( nPos - 1 )->GetLineEnd() );
            pOld = GetOldItem( rAttrs, XATTR_LINEEND );
            if( pItem &&
                ( !pOld || !( *(const XLineEndItem*)pOld == *pItem ) ) )
            {
                rAttrs.Put( *pItem );
                bModified = TRUE;
            }
            delete pItem;
        }
    }

    // Linienenden zentriert
    TriState eState = aTsbCenterStart.GetState();
    if( eState != aTsbCenterStart.GetSavedValue() )
    {
        XLineStartCenterItem aItem( eState );
        pOld = GetOldItem( rAttrs, XATTR_LINESTARTCENTER );
        if ( !pOld || !( *(const XLineStartCenterItem*)pOld == aItem ) )
        {
            rAttrs.Put( aItem );
            bModified = TRUE;
        }
    }
    eState = aTsbCenterEnd.GetState();
    if( eState != aTsbCenterEnd.GetSavedValue() )
    {
        XLineEndCenterItem aItem( eState );
        pOld = GetOldItem( rAttrs, XATTR_LINEENDCENTER );
        if ( !pOld || !( *(const XLineEndCenterItem*)pOld == aItem ) )
        {
            rAttrs.Put( aItem );
            bModified = TRUE;
        }
    }

    //Breite Linienanfang
    if( aMtrStartWidth.GetText() != aMtrStartWidth.GetSavedValue() )
    {
        XLineStartWidthItem aItem( GetCoreValue( aMtrStartWidth, ePoolUnit ) );
        pOld = GetOldItem( rAttrs, XATTR_LINESTARTWIDTH );
        if ( !pOld || !( *(const XLineStartWidthItem*)pOld == aItem ) )
        {
            rAttrs.Put( aItem );
            bModified = TRUE;
        }
    }
    //Breite Linienende
    if( aMtrEndWidth.GetText() != aMtrEndWidth.GetSavedValue() )
    {
        XLineEndWidthItem aItem( GetCoreValue( aMtrEndWidth, ePoolUnit ) );
        pOld = GetOldItem( rAttrs, XATTR_LINEENDWIDTH );
        if ( !pOld || !( *(const XLineEndWidthItem*)pOld == aItem ) )
        {
            rAttrs.Put( aItem );
            bModified = TRUE;
        }
    }

    // Transparenz
    UINT16 nVal = (UINT16)aMtrTransparent.GetValue();
    if( nVal != (UINT16)aMtrTransparent.GetSavedValue().ToInt32() )
    {
        XLineTransparenceItem aItem( nVal );
        pOld = GetOldItem( rAttrs, XATTR_LINETRANSPARENCE );
        if ( !pOld || !( *(const XLineTransparenceItem*)pOld == aItem ) )
        {
            rAttrs.Put( aItem );
            bModified = TRUE;
        }
    }
    if(nSymbolType!=SVX_SYMBOLTYPE_UNKNOWN || bNewSize)
    {
        //wurde also per Auswahl gesetzt oder Größe ist anders
        SvxSizeItem  aSItem(rAttrs.GetPool()->GetWhich(SID_ATTR_SYMBOLSIZE),aSymbolSize);
        const SfxPoolItem* pSOld = GetOldItem( rAttrs, rAttrs.GetPool()->GetWhich(SID_ATTR_SYMBOLSIZE) );
        bNewSize  = pSOld ? *(const SvxSizeItem *)pSOld != aSItem : bNewSize ;
        if(bNewSize)
        {
            rAttrs.Put(aSItem);
            bModified=TRUE;
        }

        SfxInt32Item aTItem(rAttrs.GetPool()->GetWhich(SID_ATTR_SYMBOLTYPE),nSymbolType);
        const SfxPoolItem* pTOld = GetOldItem( rAttrs, rAttrs.GetPool()->GetWhich(SID_ATTR_SYMBOLTYPE) );
        BOOL bNewType  = pTOld ? *(const SfxInt32Item*)pTOld != aTItem : TRUE;
        if(bNewType && nSymbolType==SVX_SYMBOLTYPE_UNKNOWN)
            bNewType=FALSE;//kleine Korrektur, Typ wurde garnicht gesetzt -> kein Type-Item erzeugen!
        if(bNewType)
        {
            rAttrs.Put(aTItem);
            bModified=TRUE;
        }

        if(nSymbolType!=SVX_SYMBOLTYPE_NONE)
        {
            SvxBrushItem aBItem(aSymbolGraphic,GPOS_MM,rAttrs.GetPool()->GetWhich(SID_ATTR_BRUSH));
            const SfxPoolItem* pBOld = GetOldItem( rAttrs, rAttrs.GetPool()->GetWhich(SID_ATTR_BRUSH) );
            BOOL bNewBrush = pBOld ? *(const SvxBrushItem*)pBOld != aBItem : TRUE;
            if(bNewBrush)
            {
                rAttrs.Put(aBItem);
                bModified=TRUE;
            }
        }
    }

    return( bModified );
}

// -----------------------------------------------------------------------

BOOL SvxLineTabPage::FillXLSet_Impl()
{
    USHORT nPos;

    if( aLbLineStyle.GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND )
    {
        //rXLSet.Put( XLineDashItem ( rOutAttrs.Get( GetWhich( XATTR_LINEDASH ) ) ) );
        rXLSet.Put( XLineStyleItem( XLINE_NONE ) );
    }
    else if( aLbLineStyle.IsEntryPosSelected( 0 ) )
        rXLSet.Put( XLineStyleItem( XLINE_NONE ) );
    else if( aLbLineStyle.IsEntryPosSelected( 1 ) )
        rXLSet.Put( XLineStyleItem( XLINE_SOLID ) );
    else
    {
        rXLSet.Put( XLineStyleItem( XLINE_DASH ) );

        nPos = aLbLineStyle.GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            rXLSet.Put( XLineDashItem( aLbLineStyle.GetSelectEntry(),
                            pDashList->Get( nPos - 2 )->GetDash() ) );
        }
    }

    nPos = aLbStartStyle.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        if( nPos == 0 )
            rXLSet.Put( XLineStartItem() );
        else
            rXLSet.Put( XLineStartItem( aLbStartStyle.GetSelectEntry(),
                        pLineEndList->Get( nPos - 1 )->GetLineEnd() ) );
    }
    nPos = aLbEndStyle.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        if( nPos == 0 )
            rXLSet.Put( XLineEndItem() );
        else
            rXLSet.Put( XLineEndItem( aLbEndStyle.GetSelectEntry(),
                        pLineEndList->Get( nPos - 1 )->GetLineEnd() ) );
    }

    rXLSet.Put( XLineStartWidthItem( GetCoreValue( aMtrStartWidth, ePoolUnit ) ) );
    rXLSet.Put( XLineEndWidthItem( GetCoreValue( aMtrEndWidth, ePoolUnit ) ) );

    rXLSet.Put( XLineWidthItem( GetCoreValue( aMtrLineWidth, ePoolUnit ) ) );
    rXLSet.Put( XLineColorItem( aLbColor.GetSelectEntry(),
                                    aLbColor.GetSelectEntryColor() ) );

    // Linienenden zentriert
    if( aTsbCenterStart.GetState() == STATE_CHECK )
        rXLSet.Put( XLineStartCenterItem( TRUE ) );
    else if( aTsbCenterStart.GetState() == STATE_NOCHECK )
        rXLSet.Put( XLineStartCenterItem( FALSE ) );

    if( aTsbCenterEnd.GetState() == STATE_CHECK )
        rXLSet.Put( XLineEndCenterItem( TRUE ) );
    else if( aTsbCenterEnd.GetState() == STATE_NOCHECK )
        rXLSet.Put( XLineEndCenterItem( FALSE ) );

    // Transparenz
    UINT16 nVal = (UINT16)aMtrTransparent.GetValue();
    rXLSet.Put( XLineTransparenceItem( nVal ) );

    XOut.SetLineAttr( aXLineAttr.GetItemSet() );

    return( TRUE );
}

// -----------------------------------------------------------------------

void SvxLineTabPage::Reset( const SfxItemSet& rAttrs )
{
    String      aString;
    XLineStyle  eXLS; // XLINE_NONE, XLINE_SOLID, XLINE_DASH

    // Linienstil
/*
    if( bObjSelected &&
        rAttrs.GetItemState( GetWhich( XATTR_LINESTYLE ) ) == SFX_ITEM_DEFAULT )
    {
        aLbLineStyle.Disable();
    }
*/
    const SfxPoolItem *pPoolItem;
    long nSymType=SVX_SYMBOLTYPE_UNKNOWN;
    BOOL bPrevSym=FALSE;
    BOOL bEnable=TRUE;
    BOOL bIgnoreGraphic=FALSE;
    BOOL bIgnoreSize=FALSE;
    if(rAttrs.GetItemState(rAttrs.GetPool()->GetWhich(SID_ATTR_SYMBOLTYPE),TRUE,&pPoolItem) == SFX_ITEM_SET)
    {
        nSymType=((const SfxInt32Item *)pPoolItem)->GetValue();
    }

    if(nSymType == SVX_SYMBOLTYPE_AUTO)
    {
        aSymbolGraphic=aAutoSymbolGraphic;
        aSymbolSize=aSymbolLastSize=aAutoSymbolGraphic.GetPrefSize();
#ifdef DEBUG
        ByteString aStr( "SVX_SYMBOLTYPE_AUTO AutoSymbolSize is " );
        aStr.Append( ByteString::CreateFromInt32( aSymbolSize.Width() ) );
        aStr.Append( ' ' );
        aStr.Append( ByteString::CreateFromInt32( aSymbolSize.Height() ) );
        SVX_TRACE( 213, aStr );
#endif
        bPrevSym=TRUE;
        bEnable=FALSE;
        bIgnoreGraphic=TRUE;
        bIgnoreSize=TRUE;
    }
    else if(nSymType == SVX_SYMBOLTYPE_NONE)
    {
        bEnable=FALSE;
        bIgnoreGraphic=TRUE;
        bIgnoreSize=TRUE;
        SVX_TRACE(213, ByteString( "SVX_SYMBOLTYPE_NONE" ) );
    }
    else if(nSymType >= 0)
    {
#ifdef DEBUG
        ByteString aStr( "SVX_SYMBOLTYPE = " );
        aStr.Append( ByteString::CreateFromInt32( nSymType ) );
        SVX_TRACE(213, aStr );
#endif
        VirtualDevice aVDev;
        aVDev.SetMapMode(MapMode(MAP_100TH_MM));
        SdrModel* pModel = new SdrModel(NULL, NULL, LOADREFCOUNTS);
        pModel->GetItemPool().FreezeIdRanges();
        SdrPage* pPage = new SdrPage( *pModel, FALSE );
        pPage->SetSize(Size(1000,1000));
        pModel->InsertPage( pPage, 0 );
        SdrView* pView = new SdrView( pModel, &aVDev );
        pView->SetMarkHdlHidden( TRUE );
        SdrPageView* pPageView = pView->ShowPage(pPage, Point());
        SdrObject *pObj=NULL;
        long nSymTmp=nSymType;
        if(pSymbolList)
        {
            if(pSymbolList->GetObjCount())
            {
                nSymTmp=nSymTmp%pSymbolList->GetObjCount(); //Liste wird als cyclisch betrachtet!!!
                pObj=pSymbolList->GetObj(nSymTmp);
                if(pObj)
                {
                    pObj=pObj->Clone();
                    pPage->NbcInsertObject(pObj);
                    pView->MarkObj(pObj,pPageView);
                    if(pSymbolAttr)
                    {
//-/                        pObj->NbcSetAttributes(*pSymbolAttr,FALSE);
                        pObj->SetItemSet(*pSymbolAttr);
                    }
                    else
                    {
//-/                        pObj->NbcSetAttributes(rOutAttrs,FALSE);
                        pObj->SetItemSet(rOutAttrs);
                    }
                    GDIMetaFile aMeta(pView->GetAllMarkedMetaFile());

                    aSymbolGraphic=Graphic(aMeta);
                    aSymbolSize=pObj->GetSnapRect().GetSize();
                    aSymbolGraphic.SetPrefSize(aSymbolSize);
                    aSymbolGraphic.SetPrefMapMode(MAP_100TH_MM);
                    bPrevSym=TRUE;
                    bEnable=TRUE;
                    bIgnoreGraphic=TRUE;

                    pView->UnmarkAll();
                    pObj=pPage->RemoveObject(0);
                    if(pObj)
                        delete pObj;
                }
            }
        }
        delete pView;
        delete pModel;
    }
    if(rAttrs.GetItemState(rAttrs.GetPool()->GetWhich(SID_ATTR_BRUSH),TRUE,&pPoolItem) == SFX_ITEM_SET)
    {
        SVX_TRACE(213, ByteString( "SVX_SYMBOLTYPE_BRUSH Item for Brush found" ) );
        const Graphic* pGraphic = ((const SvxBrushItem *)pPoolItem)->GetGraphic();
        if(!bIgnoreGraphic)
        {
            SVX_TRACE(213, ByteString( "SVX_SYMBOLTYPE_BRUSH Item setting Graphic" ) );
            aSymbolGraphic=*pGraphic;
        }
        if(!bIgnoreSize)
        {
            SVX_TRACE(213, ByteString( "SVX_SYMBOLTYPE_BRUSH Item setting Size" ) );
            aSymbolSize=OutputDevice::LogicToLogic( pGraphic->GetPrefSize(),
                                                    pGraphic->GetPrefMapMode(),
                                                    MAP_100TH_MM );
#ifdef DEBUG
            ByteString aStr( "SVX_SYMBOLTYPE_BRUSH Item setting Size: " );
            aStr.Append( ByteString::CreateFromInt32( aSymbolSize.Width() ) );
            aStr.Append( ByteString::CreateFromInt32( aSymbolSize.Height() ) );
            SVX_TRACE( 213, aStr );
#endif
        }
        bPrevSym=TRUE;
    }
    if(rAttrs.GetItemState(rAttrs.GetPool()->GetWhich(SID_ATTR_SYMBOLSIZE),TRUE,&pPoolItem) == SFX_ITEM_SET)
    {
        aSymbolSize = ((const SvxSizeItem *)pPoolItem)->GetSize();
    }


    aSymbolRatioCB.Enable(bEnable);
    aSymbolHeightFT.Enable(bEnable);
    aSymbolWidthFT.Enable(bEnable);
    aSymbolWidthMF.Enable(bEnable);
    aSymbolHeightMF.Enable(bEnable);
    if(bPrevSym)
    {
        SetMetricValue(aSymbolWidthMF,  aSymbolSize.Width(), ePoolUnit);
        SetMetricValue(aSymbolHeightMF, aSymbolSize.Height(),ePoolUnit);
        aCtlPreview.SetSymbol(&aSymbolGraphic,aSymbolSize);
        aSymbolLastSize=aSymbolSize;
    }

    if( rAttrs.GetItemState( XATTR_LINESTYLE ) != SFX_ITEM_DONTCARE )
    {
        eXLS = (XLineStyle) ( ( const XLineStyleItem& ) rAttrs.Get( XATTR_LINESTYLE ) ).GetValue();

        switch( eXLS )
        {
            case XLINE_NONE:
                aLbLineStyle.SelectEntryPos( 0 );
                break;
            case XLINE_SOLID:
                aLbLineStyle.SelectEntryPos( 1 );
                break;

            case XLINE_DASH:
                aLbLineStyle.SetNoSelection();
                aLbLineStyle.SelectEntry( ( ( const XLineDashItem& ) rAttrs.
                                Get( XATTR_LINEDASH ) ).GetName() );
                break;

            default:
                break;
        }
    }
    else
    {
        aLbLineStyle.SetNoSelection();
    }

    // Linienstaerke
/*
    if( bObjSelected &&
        rAttrs.GetItemState( GetWhich( XATTR_LINEWIDTH ) ) == SFX_ITEM_DEFAULT )
    {
        aMtrLineWidth.Disable();
    }
*/
    if( rAttrs.GetItemState( XATTR_LINEWIDTH ) != SFX_ITEM_DONTCARE )
    {
        SetMetricValue( aMtrLineWidth, ( ( const XLineWidthItem& ) rAttrs.
                            Get( XATTR_LINEWIDTH ) ).GetValue(), ePoolUnit );
    }
    else
        aMtrLineWidth.SetText( String() );

    // Linienfarbe
/*
    if( bObjSelected &&
        rAttrs.GetItemState( GetWhich( XATTR_LINECOLOR ) ) == SFX_ITEM_DEFAULT )
    {
        aLbColor.Disable();
    }
*/
    aLbColor.SetNoSelection();

    if ( rAttrs.GetItemState( XATTR_LINECOLOR ) != SFX_ITEM_DONTCARE )
    {
        Color aCol = ( ( const XLineColorItem& ) rAttrs.Get( XATTR_LINECOLOR ) ).GetValue();
        aLbColor.SelectEntry( aCol );
        if( aLbColor.GetSelectEntryCount() == 0 )
        {
            aLbColor.InsertEntry( aCol, String() );
            aLbColor.SelectEntry( aCol );
        }
    }

    // LinienAnfang
    if( bObjSelected &&
        rAttrs.GetItemState( XATTR_LINESTART ) == SFX_ITEM_DEFAULT )
    {
        aLbStartStyle.Disable();
    }
    else if( rAttrs.GetItemState( XATTR_LINESTART ) != SFX_ITEM_DONTCARE )
    {
        String aStr( ( ( const XLineStartItem& ) rAttrs.Get( XATTR_LINESTART ) ).GetName() );
        if( aStr.Len() == 0 )
            aLbStartStyle.SelectEntryPos( 0 );
        else
            aLbStartStyle.SelectEntry( aStr );
    }
    else
    {
        aLbStartStyle.SetNoSelection();
    }

    // LinienEnde
    if( bObjSelected &&
        rAttrs.GetItemState( XATTR_LINEEND ) == SFX_ITEM_DEFAULT )
    {
        aLbEndStyle.Disable();
    }
    else if( rAttrs.GetItemState( XATTR_LINEEND ) != SFX_ITEM_DONTCARE )
    {
        String aStr( ( ( const XLineEndItem& ) rAttrs.Get( XATTR_LINEEND ) ).GetName() );
        if( aStr.Len() == 0 )
            aLbEndStyle.SelectEntryPos( 0 );
        else
            aLbEndStyle.SelectEntry( aStr );
    }
    else
    {
        aLbEndStyle.SetNoSelection();
    }

    // LinienAnfang St"arke
    if( bObjSelected &&
        rAttrs.GetItemState( XATTR_LINESTARTWIDTH ) == SFX_ITEM_DEFAULT )
    {
        aMtrStartWidth.Disable();
    }
    else if( rAttrs.GetItemState( XATTR_LINESTARTWIDTH ) != SFX_ITEM_DONTCARE )
    {
        SetMetricValue( aMtrStartWidth, ( ( const XLineStartWidthItem& ) rAttrs.
                            Get( XATTR_LINESTARTWIDTH ) ).GetValue(), ePoolUnit );
    }
    else
        aMtrStartWidth.SetText( String() );

    // LinienEnde St"arke
    if( bObjSelected &&
        rAttrs.GetItemState( XATTR_LINEENDWIDTH ) == SFX_ITEM_DEFAULT )
    {
        aMtrEndWidth.Disable();
    }
    else if( rAttrs.GetItemState( XATTR_LINEENDWIDTH ) != SFX_ITEM_DONTCARE )
    {
        SetMetricValue( aMtrEndWidth, ( ( const XLineEndWidthItem& ) rAttrs.
                            Get( XATTR_LINEENDWIDTH ) ).GetValue(), ePoolUnit );
    }
    else
        aMtrEndWidth.SetText( String() );

    // Linienenden zentriert (Start)
    if( bObjSelected &&
        rAttrs.GetItemState( XATTR_LINESTARTCENTER ) == SFX_ITEM_DEFAULT )
    {
        aTsbCenterStart.Disable();
    }
    else if( rAttrs.GetItemState( XATTR_LINESTARTCENTER ) != SFX_ITEM_DONTCARE )
    {
        aTsbCenterStart.EnableTriState( FALSE );

        if( ( ( const XLineStartCenterItem& ) rAttrs.Get( XATTR_LINESTARTCENTER ) ).GetValue() )
            aTsbCenterStart.SetState( STATE_CHECK );
        else
            aTsbCenterStart.SetState( STATE_NOCHECK );
    }
    else
    {
        aTsbCenterStart.SetState( STATE_DONTKNOW );
    }

    // Linienenden zentriert (Ende)
    if( bObjSelected &&
        rAttrs.GetItemState( XATTR_LINEENDCENTER ) == SFX_ITEM_DEFAULT )
    {
        aTsbCenterEnd.Disable();
    }
    else if( rAttrs.GetItemState( XATTR_LINEENDCENTER ) != SFX_ITEM_DONTCARE )
    {
        aTsbCenterEnd.EnableTriState( FALSE );

        if( ( ( const XLineEndCenterItem& ) rAttrs.Get( XATTR_LINEENDCENTER ) ).GetValue() )
            aTsbCenterEnd.SetState( STATE_CHECK );
        else
            aTsbCenterEnd.SetState( STATE_NOCHECK );
    }
    else
    {
        aTsbCenterEnd.SetState( STATE_DONTKNOW );
    }

    // Transparenz
    if( rAttrs.GetItemState( XATTR_LINETRANSPARENCE ) != SFX_ITEM_DONTCARE )
    {
        USHORT nTransp = ( ( const XLineTransparenceItem& ) rAttrs.
                                Get( XATTR_LINETRANSPARENCE ) ).GetValue();
        aMtrTransparent.SetValue( nTransp );
        ChangeTransparentHdl_Impl( NULL );
    }
    else
        aMtrTransparent.SetText( String() );

    if( !aLbStartStyle.IsEnabled()  &&
        !aLbEndStyle.IsEnabled()    &&
        !aMtrStartWidth.IsEnabled() &&
        !aMtrEndWidth.IsEnabled()   &&
        !aTsbCenterStart.IsEnabled()&&
        !aTsbCenterEnd.IsEnabled() )
    {
        aCbxSynchronize.Disable();
        aFtLineEndsStyle.Disable();
        aFtLineEndsWidth.Disable();
        aGrpLineEnds.Disable();
    }

    // Synchronisieren
    // Jetzt wird der Wert aus der INI-Datei geholt (#42258#)
    String aStr = GetUserData();
    aCbxSynchronize.Check( (BOOL)aStr.ToInt32() );

    /*
    if( aLbStartStyle.GetSelectEntryPos() == aLbEndStyle.GetSelectEntryPos() &&
        aMtrStartWidth.GetValue() == aMtrEndWidth.GetValue() &&
        aTsbCenterStart.GetState() == aTsbCenterEnd.GetState() )
    {
        aCbxSynchronize.Check();
    }
    */

    // Werte sichern
    aLbLineStyle.SaveValue();
    aMtrLineWidth.SaveValue();
    aLbColor.SaveValue();
    aLbStartStyle.SaveValue();
    aLbEndStyle.SaveValue();
    aMtrStartWidth.SaveValue();
    aMtrEndWidth.SaveValue();
    aTsbCenterStart.SaveValue();
    aTsbCenterEnd.SaveValue();
    aMtrTransparent.SaveValue();


    ClickInvisibleHdl_Impl( this );
    //ClickMeasuringHdl_Impl( this );
    //aCtlPosition.Reset();

    ChangePreviewHdl_Impl( NULL );
}

// -----------------------------------------------------------------------

SfxTabPage* SvxLineTabPage::Create( Window* pWindow,
                const SfxItemSet& rAttrs )
{
    return( new SvxLineTabPage( pWindow, rAttrs ) );
}

//------------------------------------------------------------------------

USHORT* SvxLineTabPage::GetRanges()
{
    return( pLineRanges );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxLineTabPage, ChangePreviewHdl_Impl, void *, pCntrl )
{
    if(pCntrl == &aMtrLineWidth)
    {
        // Linienbreite und Start/EndBreite
        INT32 nNewLineWidth = GetCoreValue( aMtrLineWidth, ePoolUnit );
        if(nActLineWidth == -1)
        {
            // Noch nicht initialisiert, hole den Startwert
            const SfxPoolItem* pOld = GetOldItem( rXLSet, XATTR_LINEWIDTH );
            INT32 nStartLineWidth = 0;
            if(pOld)
                nStartLineWidth = (( const XLineWidthItem *)pOld)->GetValue();
            nActLineWidth = nStartLineWidth;
        }

        if(nActLineWidth != nNewLineWidth)
        {
            // Anpassungen Start/EndWidth #63083#
            INT32 nValAct = GetCoreValue( aMtrStartWidth, ePoolUnit );
            INT32 nValNew = nValAct + (((nNewLineWidth - nActLineWidth) * 15) / 10);
            if(nValNew < 0)
                nValNew = 0;
            SetMetricValue( aMtrStartWidth, nValNew, ePoolUnit );

            nValAct = GetCoreValue( aMtrEndWidth, ePoolUnit );
            nValNew = nValAct + (((nNewLineWidth - nActLineWidth) * 15) / 10);
            if(nValNew < 0)
                nValNew = 0;
            SetMetricValue( aMtrEndWidth, nValNew, ePoolUnit );
        }

        // Aktuellen Wert merken
        nActLineWidth = nNewLineWidth;
    }

    FillXLSet_Impl();
    aCtlPreview.Invalidate();

    // Transparenz entspr. zugaenglich machen
    if( aLbLineStyle.GetSelectEntryPos() == 0 ) // unsichtbar
    {
        aFtTransparent.Disable();
        aMtrTransparent.Disable();
    }
    else
    {
        aFtTransparent.Enable();
        aMtrTransparent.Enable();
    }
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxLineTabPage, ChangeStartHdl_Impl, void *, p )
{
    if( aCbxSynchronize.IsChecked() )
    {
        if( p == &aMtrStartWidth )
            aMtrEndWidth.SetValue( aMtrStartWidth.GetValue() );
        if( p == &aLbStartStyle )
            aLbEndStyle.SelectEntryPos( aLbStartStyle.GetSelectEntryPos() );
        if( p == &aTsbCenterStart )
            aTsbCenterEnd.SetState( aTsbCenterStart.GetState() );
    }

    ChangePreviewHdl_Impl( this );

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxLineTabPage, ChangeEndHdl_Impl, void *, p )
{
    if( aCbxSynchronize.IsChecked() )
    {
        if( p == &aMtrEndWidth )
            aMtrStartWidth.SetValue( aMtrEndWidth.GetValue() );
        if( p == &aLbEndStyle )
            aLbStartStyle.SelectEntryPos( aLbEndStyle.GetSelectEntryPos() );
        if( p == &aTsbCenterEnd )
            aTsbCenterStart.SetState( aTsbCenterEnd.GetState() );
    }

    ChangePreviewHdl_Impl( this );

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxLineTabPage, ClickInvisibleHdl_Impl, void *, EMPTYARG )
{
    if( aLbLineStyle.GetSelectEntryPos() == 0 ) // unsichtbar
    {
        aFtColor.Disable();
        aLbColor.Disable();
        aFtLineWidth.Disable();
        aMtrLineWidth.Disable();

        if( aGrpLineEnds.IsEnabled() )
        {
            aFtLineEndsStyle.Disable();
            aFtLineEndsWidth.Disable();
            aLbStartStyle.Disable();
            aMtrStartWidth.Disable();
            aTsbCenterStart.Disable();
            aLbEndStyle.Disable();
            aMtrEndWidth.Disable();
            aTsbCenterEnd.Disable();
            aCbxSynchronize.Disable();
        }
    }
    else
    {
        aFtColor.Enable();
        aLbColor.Enable();
        aFtLineWidth.Enable();
        aMtrLineWidth.Enable();

        if( aGrpLineEnds.IsEnabled() )
        {
            aFtLineEndsStyle.Enable();
            aFtLineEndsWidth.Enable();
            aLbStartStyle.Enable();
            aMtrStartWidth.Enable();
            aTsbCenterStart.Enable();
            aLbEndStyle.Enable();
            aMtrEndWidth.Enable();
            aTsbCenterEnd.Enable();
            aCbxSynchronize.Enable();
        }
    }
    ChangePreviewHdl_Impl( NULL );

    return( 0L );
}

//------------------------------------------------------------------------
/*

long SvxLineTabPage::ClickMeasuringHdl_Impl( void* )
{
    if( aTsbShowMeasuring.GetState() == STATE_NOCHECK )
    {
        aFtPosition.Disable();
        aCtlPosition.Disable();
    }
    else
    {
        aFtPosition.Enable();
        aCtlPosition.Enable();
    }
    aCtlPosition.Invalidate();

    return( 0L );
}
*/
//------------------------------------------------------------------------

IMPL_LINK( SvxLineTabPage, ChangeTransparentHdl_Impl, void *, EMPTYARG )
{
    UINT16 nVal = (UINT16)aMtrTransparent.GetValue();
    XLineTransparenceItem aItem( nVal );

    rXLSet.Put( XLineTransparenceItem( aItem ) );
    XOut.SetLineAttr( aXLineAttr.GetItemSet() );

    aCtlPreview.Invalidate();

    return( 0L );
}

//------------------------------------------------------------------------

void SvxLineTabPage::PointChanged( Window* pWindow, RECT_POINT eRcPt )
{
    eRP = eRcPt;
}

//------------------------------------------------------------------------

void SvxLineTabPage::FillUserData()
{
    // Abgleich wird in der Ini-Datei festgehalten
    UniString aStrUserData = UniString::CreateFromInt32( (sal_Int32) aCbxSynchronize.IsChecked() );
    SetUserData( aStrUserData );
}


//#58425# Symbole auf einer Linie (z.B. StarChart)
//Handler für Popup-Menue der Symbolauswahl (NumMenueButton)
//der folgende Link stammt urspruenglich aus SvxNumOptionsTabPage
IMPL_LINK( SvxLineTabPage, MenuCreateHdl_Impl, MenuButton *, pButton )
{
    //Popup initialisieren
    if(!pButton->GetPopupMenu()->GetPopupMenu( MN_GALLERY ))
    {
        // Gallery-Eintraege besorgen
        GalleryExplorer::FillObjList(GALLERY_THEME_BULLETS, aGrfNames);

        PopupMenu* pPopup = new PopupMenu;
        String aEmptyStr;
        SfxObjectShell *pDocSh = SfxObjectShell::Current();

        nNumMenuGalleryItems=aGrfNames.Count();
        for(long i = 0; i < nNumMenuGalleryItems; i++)
        {
            const String* pGrfName = (const String*)aGrfNames.GetObject(i);

            SvxBrushItem* pBrushItem = new SvxBrushItem(*pGrfName, aEmptyStr, GPOS_AREA);
            pBrushItem->SetDoneLink(STATIC_LINK(this, SvxLineTabPage, GraphicArrivedHdl_Impl));

            SvxBmpItemInfo* pInfo = new SvxBmpItemInfo();
            pInfo->pBrushItem = pBrushItem;
            pInfo->nItemId = (UINT16)(MN_GALLERY_ENTRY + i);
            aGrfBrushItems.Insert(pInfo, i);
            const Graphic* pGraphic = pBrushItem->GetGraphic(pDocSh);

            if(pGraphic)
            {
                Bitmap aBitmap(pGraphic->GetBitmap());
                Size aSize(aBitmap.GetSizePixel());
                if(aSize.Width()  > MAX_BMP_WIDTH ||
                   aSize.Height() > MAX_BMP_HEIGHT)
                {
                    BOOL bWidth = aSize.Width() > aSize.Height();
                    double nScale = bWidth ?
                                        (double)MAX_BMP_WIDTH / (double)aSize.Width():
                                        (double)MAX_BMP_HEIGHT / (double)aSize.Height();
                    aBitmap.Scale(nScale, nScale);

                }
                Image aImage(aBitmap);
                pPopup->InsertItem(pInfo->nItemId, *pGrfName, aImage );
            }
            else
            {
                Image aImage;
                pPopup->InsertItem(pInfo->nItemId, *pGrfName, aImage );
            }
        }
        aSymbolMB.GetPopupMenu()->SetPopupMenu( MN_GALLERY, pPopup );
        if(!aGrfNames.Count())
            aSymbolMB.GetPopupMenu()->EnableItem(MN_GALLERY, FALSE);
    }

    if(!pButton->GetPopupMenu()->GetPopupMenu( MN_SYMBOLS ) && pSymbolList)
    {
        VirtualDevice aVDev;
        aVDev.SetMapMode(MapMode(MAP_100TH_MM));

        SdrModel* pModel = new SdrModel(NULL, NULL, LOADREFCOUNTS);
        pModel->GetItemPool().FreezeIdRanges();
        // Page
        SdrPage* pPage = new SdrPage( *pModel, FALSE );
        pPage->SetSize(Size(1000,1000));
        pModel->InsertPage( pPage, 0 );
        // 3D View
        SdrView* pView = new SdrView( pModel, &aVDev );
        pView->SetMarkHdlHidden( TRUE );
        SdrPageView* pPageView = pView->ShowPage(pPage, Point());

        PopupMenu* pPopup = new PopupMenu;
        String aEmptyStr;
        SfxObjectShell *pDocSh = SfxObjectShell::Current();
        SdrObject *pObj=NULL;
        long i=0;
        while(pObj=pSymbolList->GetObj(i))
        {
            pObj=pObj->Clone();
            //const String* pGrfName = (const String*)aGrfNames.GetObject(i);
            String *pStr=new String();//String(i));
            aGrfNames.Insert(pStr,LIST_APPEND);
            //Rectangle aRect(pObj->GetLogicRect());
            pPage->NbcInsertObject(pObj);
            pView->MarkObj(pObj,pPageView);
            if(pSymbolAttr)
            {
//-/                pObj->NbcSetAttributes(*pSymbolAttr,FALSE);
                pObj->SetItemSet(*pSymbolAttr);
            }
            else
            {
//-/                pObj->NbcSetAttributes(rOutAttrs,FALSE);
                pObj->SetItemSet(rOutAttrs);
            }

            Bitmap aBitmap(pView->GetAllMarkedBitmap());
            GDIMetaFile aMeta(pView->GetAllMarkedMetaFile());
            pView->UnmarkAll();
            pObj=pPage->RemoveObject(0);
            if(pObj)
                delete pObj;

            SvxBrushItem* pBrushItem = new SvxBrushItem(Graphic(aMeta), GPOS_AREA);
            pBrushItem->SetDoneLink(STATIC_LINK(this, SvxLineTabPage, GraphicArrivedHdl_Impl));

            SvxBmpItemInfo* pInfo = new SvxBmpItemInfo();
            pInfo->pBrushItem = pBrushItem;
            pInfo->nItemId = (UINT16)(MN_GALLERY_ENTRY + i + nNumMenuGalleryItems);
            aGrfBrushItems.Insert(pInfo, nNumMenuGalleryItems + i);

            Size aSize(aBitmap.GetSizePixel());
            if(aSize.Width() > MAX_BMP_WIDTH || aSize.Height() > MAX_BMP_HEIGHT)
            {
                BOOL bWidth = aSize.Width() > aSize.Height();
                double nScale = bWidth ?
                                    (double)MAX_BMP_WIDTH / (double)aSize.Width():
                                    (double)MAX_BMP_HEIGHT / (double)aSize.Height();
                aBitmap.Scale(nScale, nScale);
            }
            Image aImage(aBitmap);
            pPopup->InsertItem(pInfo->nItemId,*pStr,aImage);
            i++;
        }
        aSymbolMB.GetPopupMenu()->SetPopupMenu( MN_SYMBOLS, pPopup );
        if(!aGrfNames.Count())
            aSymbolMB.GetPopupMenu()->EnableItem(MN_SYMBOLS, FALSE);

        delete pView;
        delete pModel;
    }
    return 0;
}
//#58425# Symbole auf einer Linie (z.B. StarChart)
//Handler für Popup-Menue der Symbolauswahl (NumMenueButton)
//der folgende Link stammt urspruenglich aus SvxNumOptionsTabPage:
IMPL_STATIC_LINK(SvxLineTabPage, GraphicArrivedHdl_Impl, SvxBrushItem*, pItem)
{
    PopupMenu* pPopup = pThis->aSymbolMB.GetPopupMenu()->GetPopupMenu( MN_GALLERY );

    SvxBmpItemInfo* pInfo = 0;
    for ( USHORT i = 0; i < pThis->aGrfBrushItems.Count(); i++ )
    {
        SvxBmpItemInfo* pInfo = (SvxBmpItemInfo*)pThis->aGrfBrushItems.GetObject(i);
        if(&pInfo->pBrushItem == &pItem)
        {
            pInfo = pInfo; break;
        }
    }
    if(pInfo)
    {
        Image aImage(pItem->GetGraphic()->GetBitmap());
        pPopup->SetItemImage( pInfo->nItemId, aImage );
    }

    return 0;
}

//#58425# Symbole auf einer Linie (z.B. StarChart)
//Handler für Menuebutton
IMPL_LINK( SvxLineTabPage, GraphicHdl_Impl, MenuButton *, pButton )
{
    USHORT nItemId = pButton->GetCurItemId();
    const Graphic* pGraphic = 0;
    String aGrfName;
    BOOL bDontSetSize=FALSE;
    SvxImportGraphicDialog* pGrfDlg = 0;
    BOOL bEnable=TRUE;

    if(nItemId >= MN_GALLERY_ENTRY)
    {
        if( (nItemId-MN_GALLERY_ENTRY) >= nNumMenuGalleryItems)
        {
            nSymbolType=nItemId-MN_GALLERY_ENTRY-nNumMenuGalleryItems; //Index der Liste
#ifdef DEBUG
            ByteString aStr( "SVX_SYMBOLTYPE = " );
            aStr.Append( ByteString::CreateFromInt32( nSymbolType ) );
            SVX_TRACE(213, aStr);
#endif
        }
        else
        {
            SVX_TRACE(213,ByteString( "SVX_SYMBOLTYPE_BRUSHITEM") );
            nSymbolType=SVX_SYMBOLTYPE_BRUSHITEM;
        }
        SvxBmpItemInfo* pInfo = (SvxBmpItemInfo*)aGrfBrushItems.GetObject(nItemId - MN_GALLERY_ENTRY);
        pGraphic = pInfo->pBrushItem->GetGraphic();
    }
    else switch(nItemId)
    {
        case MN_SYMBOLS_AUTO:
        {
            pGraphic=&aAutoSymbolGraphic;
            aSymbolSize=aAutoSymbolGraphic.GetPrefSize();
#ifdef DEBUG
            ByteString aStr( "AutoSymbolSize is ");
            aStr.Append( ByteString::CreateFromInt32( aSymbolSize.Width() ) );
            aStr.Append( ' ' );
            aStr.Append( ByteString::CreateFromInt32( aSymbolSize.Height() ) );
            SVX_TRACE(213, aStr );
#endif
            bDontSetSize=TRUE;
            bNewSize=FALSE; //frühere Änderungen gelten nicht in diesem Fall!
            nSymbolType=SVX_SYMBOLTYPE_AUTO;
            bEnable=FALSE;
        }
        break;

        case MN_SYMBOLS_NONE:
        {
            SVX_TRACE(213,ByteString( "SVX_SYMBOLTYPE_NONE" ) );
            nSymbolType=SVX_SYMBOLTYPE_NONE;
            pGraphic=NULL;
            bEnable=FALSE;
        }
        break;
        default:
        {
            SVX_TRACE(213,ByteString( "SVX_SYMBOLTYPE_ Brush lesen, wird aus Datei generiert" ) );
            nSymbolType=SVX_SYMBOLTYPE_BRUSHITEM;//wie Gallery, Graphic ist im Item enthalten
            pGrfDlg = new SvxImportGraphicDialog(this,SVX_RESSTR(RID_STR_EDIT_GRAPHIC),ENABLE_STANDARD);
            if ( pGrfDlg->Execute() == RET_OK )
            {
                // ausgewaehlten Filter merken
                aGrfName = pGrfDlg->GetPath();
                pGraphic = pGrfDlg->GetGraphic();
            }
        }
        break;
    }

    if(pGraphic)
    {
        Size aSize = SvxNumberFormat::GetGraphicSizeMM100(pGraphic);
        aSize = OutputDevice::LogicToLogic(aSize, MAP_100TH_MM, (MapUnit)ePoolUnit);
        aSymbolGraphic=*pGraphic;
        if(!bDontSetSize)
        {
            aSymbolSize=aSize;
#ifdef DEBUG
            ByteString aStr( "SymbolSize changed to " );
            aStr.Append( ByteString::CreateFromInt32( aSize.Width() ) );
            aStr.Append( ' ' );
            aStr.Append( ByteString::CreateFromInt32( aSize.Height() ) );
            SVX_TRACE(213, aStr );
#endif
        }
        aCtlPreview.SetSymbol(&aSymbolGraphic,aSymbolSize);
    }
    else
    {
        aSymbolGraphic=Graphic();
        SVX_TRACE(213, ByteString( "setting empty graphic" ) );
        if(!bDontSetSize)
        {
            SVX_TRACE(213, ByteString( "Size set to 0,0" ) );
            aSymbolSize=Size(0,0);
        }
        aCtlPreview.SetSymbol(NULL,aSymbolSize);
        bEnable=FALSE;
    }
    aSymbolLastSize=aSymbolSize;
    SetMetricValue(aSymbolWidthMF,  aSymbolSize.Width(), ePoolUnit);
    SetMetricValue(aSymbolHeightMF, aSymbolSize.Height(), ePoolUnit);
    aSymbolRatioCB.Enable(bEnable);
    aSymbolHeightFT.Enable(bEnable);
    aSymbolWidthFT.Enable(bEnable);
    aSymbolWidthMF.Enable(bEnable);
    aSymbolHeightMF.Enable(bEnable);
    aCtlPreview.Invalidate();



    delete pGrfDlg;
    return 0;
}
IMPL_LINK( SvxLineTabPage, SizeHdl_Impl, MetricField *, pField)
{
    bNewSize=TRUE;
    BOOL bWidth = (BOOL)(pField == &aSymbolWidthMF);
    bLastWidthModified = bWidth;
    BOOL bRatio = aSymbolRatioCB.IsChecked();
    long nWidthVal = aSymbolWidthMF.Denormalize(aSymbolWidthMF.GetValue(FUNIT_100TH_MM));
    long nHeightVal= aSymbolHeightMF.Denormalize(aSymbolHeightMF.GetValue(FUNIT_100TH_MM));
    nWidthVal = OutputDevice::LogicToLogic(nWidthVal,MAP_100TH_MM,(MapUnit)ePoolUnit );
    nHeightVal = OutputDevice::LogicToLogic(nHeightVal,MAP_100TH_MM,(MapUnit)ePoolUnit);
    aSymbolSize=Size(nWidthVal,nHeightVal);
    double  fSizeRatio;

    if(bRatio)
    {
        if (aSymbolLastSize.Height() && aSymbolLastSize.Width())
            fSizeRatio = (double)aSymbolLastSize.Width() / aSymbolLastSize.Height();
        else
            fSizeRatio = (double)1;
    }

    //Size aSymbolSize(aSymbolLastSize);

    if(bWidth)
    {
        long nDelta = nWidthVal - aSymbolLastSize.Width();
        aSymbolSize.Width() = nWidthVal;
        if (bRatio)
        {
            aSymbolSize.Height() = aSymbolLastSize.Height() + (long)((double)nDelta / fSizeRatio);
            aSymbolSize.Height() = OutputDevice::LogicToLogic( aSymbolSize.Height(),(MapUnit)ePoolUnit, MAP_100TH_MM );
            aSymbolHeightMF.SetUserValue(aSymbolHeightMF.Normalize(aSymbolSize.Height()), FUNIT_100TH_MM);
        }
    }
    else
    {
        long nDelta = nHeightVal - aSymbolLastSize.Height();
        aSymbolSize.Height() = nHeightVal;
        if (bRatio)
        {
            aSymbolSize.Width() = aSymbolLastSize.Width() + (long)((double)nDelta * fSizeRatio);
            aSymbolSize.Width() = OutputDevice::LogicToLogic( aSymbolSize.Width(),
                                (MapUnit)ePoolUnit, MAP_100TH_MM );
            aSymbolWidthMF.SetUserValue(aSymbolWidthMF.Normalize(aSymbolSize.Width()), FUNIT_100TH_MM);
        }
    }
    aCtlPreview.ResizeSymbol(aSymbolSize);
    aSymbolLastSize=aSymbolSize;
    return 0;
}
IMPL_LINK( SvxLineTabPage, RatioHdl_Impl, CheckBox *, pBox )
{
    if (pBox->IsChecked())
    {
        if (bLastWidthModified)
            SizeHdl_Impl(&aSymbolWidthMF);
        else
            SizeHdl_Impl(&aSymbolHeightMF);
    }
    return 0;
}


