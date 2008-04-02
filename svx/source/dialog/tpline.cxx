/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tpline.cxx,v $
 *
 *  $Revision: 1.43 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-02 09:53:40 $
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
#include "precompiled_svx.hxx"

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

// include ---------------------------------------------------------------

#ifndef _SVX_SIZEITEM_HXX //autogen

#include <svx/sizeitem.hxx>
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

#define _SVX_TPLINE_CXX

#include <svx/dialogs.hrc>
#include "tabline.hrc"
#include "dlgname.hrc"





#include "xattr.hxx"
#include <svx/xpool.hxx>
#include <svx/xtable.hxx>
#include <svx/xoutx.hxx>

#include "drawitem.hxx"
#include "cuitabline.hxx"
#include "dlgname.hxx"
#include <svx/dialmgr.hxx>
#include "dlgutil.hxx"
#include <svx/svdmodel.hxx>
#include "svxgrahicitem.hxx"
#include "linectrl.hrc"
#include <sfx2/request.hxx> //add CHINA001
#include "ofaitem.hxx" //add CHINA001
//#58425# Symbole auf einer Linie (z.B. StarChart) Includes:

#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif
#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif
#ifndef _SVDMODEL_HXX //autogen
#include <svx/svdmodel.hxx>
#endif

#ifndef _SVX_NUMVSET_HXX //autogen
#include <numvset.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVX_NUMITEM_HXX //autogen
#include <svx/numitem.hxx>
#endif
#ifndef _SVX_SVXENUM_HXX //autogen
#include <svx/svxenum.hxx>
#endif
#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif

#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _GALLERY_HXX_ //autogen
#include <gallery.hxx>
#endif
#ifndef _SVX_XBITMAP_HXX //autogen
#include <svx/xbitmap.hxx>
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif

#include "opengrf.hxx"

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
    aFtLineStyle        ( this, SVX_RES( FT_LINE_STYLE ) ),
    aLbLineStyle        ( this, SVX_RES( LB_LINE_STYLE ) ),
    aFtColor            ( this, SVX_RES( FT_COLOR ) ),
    aLbColor            ( this, SVX_RES( LB_COLOR ) ),
    aFtLineWidth        ( this, SVX_RES( FT_LINE_WIDTH ) ),
    aMtrLineWidth       ( this, SVX_RES( MTR_FLD_LINE_WIDTH ) ),
    aFtTransparent      ( this, SVX_RES( FT_TRANSPARENT ) ),
    aMtrTransparent     ( this, SVX_RES( MTR_LINE_TRANSPARENT ) ),
    aFlLine             ( this, SVX_RES( FL_LINE ) ),
    aFtLineEndsStyle    ( this, SVX_RES( FT_LINE_ENDS_STYLE ) ),
    aLbStartStyle       ( this, SVX_RES( LB_START_STYLE ) ),
    aFtLineEndsWidth    ( this, SVX_RES( FT_LINE_ENDS_WIDTH ) ),
    aMtrStartWidth      ( this, SVX_RES( MTR_FLD_START_WIDTH ) ),
    aTsbCenterStart     ( this, SVX_RES( TSB_CENTER_START ) ),
    aLbEndStyle         ( this, SVX_RES( LB_END_STYLE ) ),
    aMtrEndWidth        ( this, SVX_RES( MTR_FLD_END_WIDTH ) ),
    aTsbCenterEnd       ( this, SVX_RES( TSB_CENTER_END ) ),
    aCbxSynchronize     ( this, SVX_RES( CBX_SYNCHRONIZE ) ),
    aFlLineEnds         ( this, SVX_RES( FL_LINE_ENDS ) ),
    aCtlPreview         ( this, SVX_RES( CTL_PREVIEW ), &XOut ),
    aFLSeparator        ( this, SVX_RES( FL_SEPARATOR ) ),

    // #116827#
    maFLEdgeStyle       ( this, SVX_RES( FL_EDGE_STYLE ) ),
    maFTEdgeStyle       ( this, SVX_RES( FT_EDGE_STYLE ) ),
    maLBEdgeStyle       ( this, SVX_RES( LB_EDGE_STYLE ) ),

    pSymbolList(NULL),
    bNewSize(false),
    nNumMenuGalleryItems(0),
    nSymbolType(SVX_SYMBOLTYPE_UNKNOWN), //unbekannt bzw. unchanged
    pSymbolAttr(NULL),
    //#58425# Symbole auf einer Linie (z.B. StarChart)
    aFlSymbol           ( this, SVX_RES(FL_SYMBOL_FORMAT)),
    aSymbolMB           ( this, SVX_RES(MB_SYMBOL_BITMAP)),
    aSymbolWidthFT      ( this, SVX_RES(FT_SYMBOL_WIDTH)),
    aSymbolWidthMF      ( this, SVX_RES(MF_SYMBOL_WIDTH)),
    aSymbolHeightFT     ( this, SVX_RES(FT_SYMBOL_HEIGHT)),
    aSymbolHeightMF     ( this, SVX_RES(MF_SYMBOL_HEIGHT)),
    aSymbolRatioCB      ( this, SVX_RES(CB_SYMBOL_RATIO)),

    bLastWidthModified(FALSE),
    aSymbolLastSize(Size(0,0)),
    bSymbols(FALSE),

    rOutAttrs           ( rInAttrs ),
    bObjSelected( sal_False ),

    pXPool              ( (XOutdevItemPool*) rInAttrs.GetPool() ),
    XOut                ( &aCtlPreview ),
    aXLStyle            ( XLINE_DASH ),
    aXWidth             ( 1 ),
    aXDash              ( String(), XDash( XDASH_RECT, 3, 7, 2, 40, 15 ) ),
    aXColor             ( String(), COL_LIGHTRED ),
    aXLineAttr          ( pXPool ),
    rXLSet              ( aXLineAttr.GetItemSet() ),
     pnLineEndListState( 0 ),
    pnDashListState( 0 ),
    pnColorTableState( 0 ),
   nPageType           ( 0 )//CHINA001 pPageType           ( NULL ),
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
            default: ;//prevent warning
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

    // #116827#
    Link aEdgeStyle = LINK( this, SvxLineTabPage, ChangeEdgeStyleHdl_Impl );
    maLBEdgeStyle.SetSelectHdl( aEdgeStyle );

    //#58425# Symbole auf einer Linie (z.B. StarChart) , MB-Handler setzen
    aSymbolMB.SetSelectHdl(LINK(this, SvxLineTabPage, GraphicHdl_Impl));
    aSymbolMB.SetActivateHdl(LINK(this, SvxLineTabPage, MenuCreateHdl_Impl));
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
    aFlSymbol.Show(bOn);
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
    // Farbtabelle
    aLbColor.Fill( pColorTab );
    FillListboxes();
}

void SvxLineTabPage::FillListboxes()
{
//  aCtlPreview.SetDrawMode( GetDisplayBackground().GetColor().IsDark() ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR );

    ResMgr& rMgr = DIALOG_MGR();

    // Linienstile
    USHORT nOldSelect = aLbLineStyle.GetSelectEntryPos();
    aLbLineStyle.Clear();
    aLbLineStyle.InsertEntry( String( ResId( RID_SVXSTR_INVISIBLE, rMgr ) ) );

    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    Bitmap aBitmap ( SVX_RES ( RID_SVXCTRL_LINECTRL ) );
    Color aSourceColors[2];
    Color aDestColors[2];

    aSourceColors[0] = Color( COL_WHITE );
    aSourceColors[1] = Color( COL_BLACK );

    aDestColors[0] = rStyles.GetFieldColor();
    aDestColors[1] = rStyles.GetFieldTextColor();

    aBitmap.Replace ( aSourceColors, aDestColors, 2 );
    Image aSolidLine ( aBitmap );
    aLbLineStyle.InsertEntry( String( ResId( RID_SVXSTR_SOLID, rMgr ) ), aSolidLine );
    aLbLineStyle.Fill( pDashList );
    aLbLineStyle.SelectEntryPos( nOldSelect );

    // LinienEndenStile
    String sNone( ResId( RID_SVXSTR_NONE, rMgr ) );
    nOldSelect = aLbStartStyle.GetSelectEntryPos();
    aLbStartStyle.Clear();
    aLbStartStyle.InsertEntry( sNone );
    aLbStartStyle.Fill( pLineEndList );
    aLbStartStyle.SelectEntryPos( nOldSelect );
    nOldSelect = aLbEndStyle.GetSelectEntryPos();
    aLbEndStyle.Clear();
    aLbEndStyle.InsertEntry( sNone );
    aLbEndStyle.Fill( pLineEndList, FALSE );
    aLbEndStyle.SelectEntryPos( nOldSelect );
}

// -----------------------------------------------------------------------

void SvxLineTabPage::ActivatePage( const SfxItemSet& rSet )
{
    SFX_ITEMSET_ARG (&rSet,pPageTypeItem,CntUInt16Item,SID_PAGE_TYPE,sal_False); //add CHINA001 begin
    if (pPageTypeItem)
        SetPageType(pPageTypeItem->GetValue()); //add CHINA001 end
    if( nDlgType == 0 && pDashList )  //CHINA001 if( *pDlgType == 0 && pDashList ) // Linien-Dialog
    {
        ResMgr& rMgr = DIALOG_MGR();
        USHORT nPos;
        USHORT nCount;

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
                String( ResId( RID_SVXSTR_INVISIBLE, rMgr ) ) );
            aLbLineStyle.InsertEntry(
                String( ResId( RID_SVXSTR_SOLID, rMgr ) ) );
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

        INetURLObject   aDashURL( pDashList->GetPath() );

        aDashURL.Append( pDashList->GetName() );
        DBG_ASSERT( aDashURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );
/*      // Ermitteln (evtl. abschneiden) des Namens und in
        // der GroupBox darstellen
        String          aString( ResId( RID_SVXSTR_TABLE, pMgr ) ); aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );

        if ( aDashURL.getBase().Len() > 18 )
        {
            aString += aDashURL.getBase().Copy( 0, 15 );
            aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
        }
        else
            aString += aDashURL.getBase();

        aGrpLine.SetText( aString );
*/
        // LineEndliste
        if( ( *pnLineEndListState & CT_MODIFIED ) ||
            ( *pnLineEndListState & CT_CHANGED ) )
        {
            if( *pnLineEndListState & CT_CHANGED )
                pLineEndList = ( (SvxLineTabDialog*) DLGWIN )->
                                        GetNewLineEndList();
            *pnLineEndListState = CT_NONE;

            nPos = aLbLineStyle.GetSelectEntryPos();
            String sNone( ResId( RID_SVXSTR_NONE, rMgr ) );
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
        INetURLObject aLineURL( pLineEndList->GetPath() );

        aLineURL.Append( pLineEndList->GetName() );
        DBG_ASSERT( aLineURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );
/*      // Ermitteln (evtl. abschneiden) des Namens und in
        // der GroupBox darstellen
        aString = String( ResId( RID_SVXSTR_TABLE, pMgr ) ); aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );

        if ( aLineURL.getBase().Len() > 18 )
        {
            aString += aLineURL.getBase().Copy( 0, 15 );
            aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
        }
        else
            aString += aLineURL.getBase();

        aGrpLineEnds.SetText( aString );
*/
        // Auswertung, ob von einer anderen TabPage ein anderer Fuelltyp gesetzt wurde
        if( aLbLineStyle.GetSelectEntryPos() != 0 )
        {
            if( nPageType == 2 ) // 1//CHINA001 if( *pPageType == 2 ) // 1
            {
                aLbLineStyle.SelectEntryPos( *pPosDashLb + 2 ); // +2 wegen SOLID und INVLISIBLE
                ChangePreviewHdl_Impl( this );
            }
            if( nPageType == 3 )//CHINA001 if( *pPageType == 3 )
            {
                aLbStartStyle.SelectEntryPos( *pPosLineEndLb + 1 );// +1 wegen SOLID
                aLbEndStyle.SelectEntryPos( *pPosLineEndLb + 1 );// +1 wegen SOLID
                ChangePreviewHdl_Impl( this );
            }
        }

            // ColorTable
            if( *pnColorTableState )
            {
                if( *pnColorTableState & CT_CHANGED )
                    pColorTab = ( (SvxLineTabDialog*) DLGWIN )->
                                            GetNewColorTable();
                // aLbColor
                sal_uInt16 nColorPos = aLbColor.GetSelectEntryPos();
                aLbColor.Clear();
                aLbColor.Fill( pColorTab );
                nCount = aLbColor.GetEntryCount();
                if( nCount == 0 )
                    ; // This case should never occur
                else if( nCount <= nColorPos )
                    aLbColor.SelectEntryPos( 0 );
                else
                    aLbColor.SelectEntryPos( nColorPos );

                ChangePreviewHdl_Impl( this );
            }

        nPageType = 0;//CHINA001 *pPageType = 0;
    }
    // Seite existiert im Ctor noch nicht, deswegen hier!

    else if ( nDlgType == 1100 || // Chart-Dialog//CHINA001 else if ( *pDlgType == 1100 || // Chart-Dialog
              nDlgType == 1101 )//CHINA001 *pDlgType == 1101 )
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
        aFlLineEnds.Hide();

        // #116827#
        maFLEdgeStyle.Hide();
        maFTEdgeStyle.Hide();
        maLBEdgeStyle.Hide();
    }
}

// -----------------------------------------------------------------------

int SvxLineTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( nDlgType == 0 ) // Linien-Dialog//CHINA001 if( *pDlgType == 0 ) // Linien-Dialog
    {
        nPageType = 1; // evtl. fuer Erweiterungen//CHINA001 *pPageType = 1; // evtl. fuer Erweiterungen
        *pPosDashLb = aLbLineStyle.GetSelectEntryPos() - 2;// erster Eintrag SOLID !!!
        USHORT nPos = aLbStartStyle.GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
            nPos--;
        *pPosLineEndLb = nPos;
    }

    if( _pSet )
        FillItemSet( *_pSet );

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
    if( nDlgType != 0 || nPageType != 2 )//CHINA001 if( *pDlgType != 0 || *pPageType != 2 )
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
                                        pDashList->GetDash( nPos - 2 )->GetDash() );
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

    if( nDlgType != 0 || nPageType != 3 )//CHINA001 if( *pDlgType != 0 || *pPageType != 3 )
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
                            pLineEndList->GetLineEnd( nPos - 1 )->GetLineEnd() );
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
                            pLineEndList->GetLineEnd( nPos - 1 )->GetLineEnd() );
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
        XLineStartCenterItem aItem( sal::static_int_cast< BOOL >( eState ) );
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
        XLineEndCenterItem aItem( sal::static_int_cast< BOOL >( eState ) );
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

    // #116827#
    nPos = maLBEdgeStyle.GetSelectEntryPos();
    if( LISTBOX_ENTRY_NOTFOUND != nPos && nPos != maLBEdgeStyle.GetSavedValue() )
    {
        XLineJointItem* pNew = 0L;

        switch(nPos)
        {
            case 0: // Rounded, default
            {
                pNew = new XLineJointItem(XLINEJOINT_ROUND);
                break;
            }
            case 1: // - none -
            {
                pNew = new XLineJointItem(XLINEJOINT_NONE);
                break;
            }
            case 2: // Miter
            {
                pNew = new XLineJointItem(XLINEJOINT_MITER);
                break;
            }
            case 3: // Bevel
            {
                pNew = new XLineJointItem(XLINEJOINT_BEVEL);
                break;
            }
        }

        if(pNew)
        {
            pOld = GetOldItem( rAttrs, XATTR_LINEJOINT );

            if(!pOld || !(*(const XLineJointItem*)pOld == *pNew))
            {
                rAttrs.Put( *pNew );
                bModified = TRUE;
            }

            delete pNew;
        }
    }

    if(nSymbolType!=SVX_SYMBOLTYPE_UNKNOWN || bNewSize)
    {
        //wurde also per Auswahl gesetzt oder Gr��e ist anders
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
        bool bNewType = pTOld == NULL || *(const SfxInt32Item*)pTOld != aTItem;
        if(bNewType && nSymbolType==SVX_SYMBOLTYPE_UNKNOWN)
            bNewType=false;//kleine Korrektur, Typ wurde garnicht gesetzt -> kein Type-Item erzeugen!
        if(bNewType)
        {
            rAttrs.Put(aTItem);
            bModified=TRUE;
        }

        if(nSymbolType!=SVX_SYMBOLTYPE_NONE)
        {
            SvxBrushItem aBItem(aSymbolGraphic,GPOS_MM,rAttrs.GetPool()->GetWhich(SID_ATTR_BRUSH));
            const SfxPoolItem* pBOld = GetOldItem( rAttrs, rAttrs.GetPool()->GetWhich(SID_ATTR_BRUSH) );
            bool bNewBrush =
                pBOld == NULL || *(const SvxBrushItem*)pBOld != aBItem;
            if(bNewBrush)
            {
                rAttrs.Put(aBItem);
                bModified=TRUE;
            }
        }
    }
    rAttrs.Put (CntUInt16Item(SID_PAGE_TYPE,nPageType));//add CHINA001
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
                            pDashList->GetDash( nPos - 2 )->GetDash() ) );
        }
    }

    nPos = aLbStartStyle.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        if( nPos == 0 )
            rXLSet.Put( XLineStartItem() );
        else
            rXLSet.Put( XLineStartItem( aLbStartStyle.GetSelectEntry(),
                        pLineEndList->GetLineEnd( nPos - 1 )->GetLineEnd() ) );
    }
    nPos = aLbEndStyle.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        if( nPos == 0 )
            rXLSet.Put( XLineEndItem() );
        else
            rXLSet.Put( XLineEndItem( aLbEndStyle.GetSelectEntry(),
                        pLineEndList->GetLineEnd( nPos - 1 )->GetLineEnd() ) );
    }

    // #116827#
    nPos = maLBEdgeStyle.GetSelectEntryPos();
    if(LISTBOX_ENTRY_NOTFOUND != nPos)
    {
        switch(nPos)
        {
            case 0: // Rounded, default
            {
                rXLSet.Put(XLineJointItem(XLINEJOINT_ROUND));
                break;
            }
            case 1: // - none -
            {
                rXLSet.Put(XLineJointItem(XLINEJOINT_NONE));
                break;
            }
            case 2: // Miter
            {
                rXLSet.Put(XLineJointItem(XLINEJOINT_MITER));
                break;
            }
            case 3: // Bevel
            {
                rXLSet.Put(XLineJointItem(XLINEJOINT_BEVEL));
                break;
            }
        }
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

    // #116827#
    aCtlPreview.SetLineAttributes(aXLineAttr.GetItemSet());

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
        bPrevSym=TRUE;
    }
    else if(nSymType == SVX_SYMBOLTYPE_NONE)
    {
        bEnable=FALSE;
        bIgnoreGraphic=TRUE;
        bIgnoreSize=TRUE;
    }
    else if(nSymType >= 0)
    {
        VirtualDevice aVDev;
        aVDev.SetMapMode(MapMode(MAP_100TH_MM));

        SdrModel* pModel = new SdrModel(NULL, NULL, LOADREFCOUNTS);
        pModel->GetItemPool().FreezeIdRanges();
        SdrPage* pPage = new SdrPage( *pModel, FALSE );
        pPage->SetSize(Size(1000,1000));
        pModel->InsertPage( pPage, 0 );
        SdrView* pView = new SdrView( pModel, &aVDev );
        pView->hideMarkHandles();
        SdrPageView* pPageView = pView->ShowSdrPage(pPage);
//      SdrPageView* pPageView = pView->ShowSdrPage(pPage, Point());
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
                        pObj->SetMergedItemSet(*pSymbolAttr);
                    }
                    else
                    {
                        pObj->SetMergedItemSet(rOutAttrs);
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
                    SdrObject::Free( pObj );
                }
            }
        }
        delete pView;
        delete pModel;
    }
    if(rAttrs.GetItemState(rAttrs.GetPool()->GetWhich(SID_ATTR_BRUSH),TRUE,&pPoolItem) == SFX_ITEM_SET)
    {
        const Graphic* pGraphic = ((const SvxBrushItem *)pPoolItem)->GetGraphic();
        if( pGraphic )
        {
            if(!bIgnoreGraphic)
            {
                aSymbolGraphic=*pGraphic;
            }
            if(!bIgnoreSize)
            {
                aSymbolSize=OutputDevice::LogicToLogic( pGraphic->GetPrefSize(),
                                                        pGraphic->GetPrefMapMode(),
                                                        MAP_100TH_MM );
            }
            bPrevSym=TRUE;
        }
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
        Color aCol = ( ( const XLineColorItem& ) rAttrs.Get( XATTR_LINECOLOR ) ).GetColorValue();
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
        // #86265# select entry using list and polygon, not string
        sal_Bool bSelected(FALSE);
        const basegfx::B2DPolyPolygon& rItemPolygon = ((const XLineStartItem&)rAttrs.Get(XATTR_LINESTART)).GetLineStartValue();

        for(sal_Int32 a(0);!bSelected &&  a < pLineEndList->Count(); a++)
        {
            XLineEndEntry* pEntry = pLineEndList->GetLineEnd(a);
            const basegfx::B2DPolyPolygon& rEntryPolygon = pEntry->GetLineEnd();

            if(rItemPolygon == rEntryPolygon)
            {
                // select this entry
                aLbStartStyle.SelectEntryPos((sal_uInt16)a + 1);
                bSelected = TRUE;
            }
        }

        if(!bSelected)
            aLbStartStyle.SelectEntryPos( 0 );
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
        // #86265# select entry using list and polygon, not string
        sal_Bool bSelected(FALSE);
        const basegfx::B2DPolyPolygon& rItemPolygon = ((const XLineEndItem&)rAttrs.Get(XATTR_LINEEND)).GetLineEndValue();

        for(sal_Int32 a(0);!bSelected &&  a < pLineEndList->Count(); a++)
        {
            XLineEndEntry* pEntry = pLineEndList->GetLineEnd(a);
            const basegfx::B2DPolyPolygon& rEntryPolygon = pEntry->GetLineEnd();

            if(rItemPolygon == rEntryPolygon)
            {
                // select this entry
                aLbEndStyle.SelectEntryPos((sal_uInt16)a + 1);
                bSelected = TRUE;
            }
        }

        if(!bSelected)
            aLbEndStyle.SelectEntryPos( 0 );
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
        aFlLineEnds.Disable();
    }

    // Synchronisieren
    // Jetzt wird der Wert aus der INI-Datei geholt (#42258#)
    String aStr = GetUserData();
    aCbxSynchronize.Check( (BOOL)aStr.ToInt32() );

    // #116827#
    if(bObjSelected && SFX_ITEM_DEFAULT == rAttrs.GetItemState(XATTR_LINEJOINT))
    {
        maFTEdgeStyle.Disable();
        maLBEdgeStyle.Disable();
    }
    else if(SFX_ITEM_DONTCARE != rAttrs.GetItemState(XATTR_LINEJOINT))
    {
        XLineJoint eLineJoint = ((const XLineJointItem&)(rAttrs.Get(XATTR_LINEJOINT))).GetValue();

        switch(eLineJoint)
        {
            case XLINEJOINT_ROUND : maLBEdgeStyle.SelectEntryPos(0); break;
            case XLINEJOINT_NONE : maLBEdgeStyle.SelectEntryPos(1); break;
            case XLINEJOINT_MITER : maLBEdgeStyle.SelectEntryPos(2); break;
            case XLINEJOINT_BEVEL : maLBEdgeStyle.SelectEntryPos(3); break;
            case XLINEJOINT_MIDDLE : break;
        }
    }
    else
    {
        maLBEdgeStyle.SetNoSelection();
    }

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

    // #116827#
    maLBEdgeStyle.SaveValue();

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

    const bool bHasLineStart = aLbStartStyle.GetSelectEntryPos() != 0;
    const bool bHasLineEnd = aLbEndStyle.GetSelectEntryPos() != 0;

    aFtLineEndsWidth.Enable( bHasLineStart || bHasLineEnd );
    aMtrStartWidth.Enable( bHasLineStart );
    aTsbCenterStart.Enable( bHasLineStart );
    aMtrEndWidth.Enable( bHasLineEnd );
    aTsbCenterEnd.Enable( bHasLineEnd );

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
// #116827#

IMPL_LINK( SvxLineTabPage, ChangeEdgeStyleHdl_Impl, void *, EMPTYARG )
{
    ChangePreviewHdl_Impl( this );

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxLineTabPage, ClickInvisibleHdl_Impl, void *, EMPTYARG )
{
    if( aLbLineStyle.GetSelectEntryPos() == 0 ) // unsichtbar
    {
        aFtColor.Disable();
        if(!bSymbols)
            aLbColor.Disable();
        aFtLineWidth.Disable();
        aMtrLineWidth.Disable();

        if( aFlLineEnds.IsEnabled() )
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

            // #116827#
            maFTEdgeStyle.Disable();
            maLBEdgeStyle.Disable();
        }
    }
    else
    {
        aFtColor.Enable();
        aLbColor.Enable();
        aFtLineWidth.Enable();
        aMtrLineWidth.Enable();

        if( aFlLineEnds.IsEnabled() )
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

            // #116827#
            maFTEdgeStyle.Enable();
            maLBEdgeStyle.Enable();
        }
    }
    ChangePreviewHdl_Impl( NULL );

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

    // #116827#
    FillXLSet_Impl();

    aCtlPreview.Invalidate();

    return( 0L );
}

//------------------------------------------------------------------------

void SvxLineTabPage::PointChanged( Window*, RECT_POINT eRcPt )
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
//Handler f�r Popup-Menue der Symbolauswahl (NumMenueButton)
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
            const String* pUIName = pGrfName;

            // convert URL encodings to UI characters (eg %20 for spaces)
            String aPhysicalName;
            if( ::utl::LocalFileHelper::ConvertURLToPhysicalName( *pGrfName, aPhysicalName ))
                pUIName = &aPhysicalName;

            SvxBrushItem* pBrushItem = new SvxBrushItem(*pGrfName, aEmptyStr, GPOS_AREA, SID_ATTR_BRUSH);
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
                pPopup->InsertItem(pInfo->nItemId, *pUIName, aImage );
            }
            else
            {
                Image aImage;
                pPopup->InsertItem(pInfo->nItemId, *pUIName, aImage );
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
        pView->hideMarkHandles();
//      SdrPageView* pPageView = pView->ShowSdrPage(pPage, Point());
        SdrPageView* pPageView = pView->ShowSdrPage(pPage);

        PopupMenu* pPopup = new PopupMenu;
        String aEmptyStr;
        for(long i=0;; ++i)
        {
            SdrObject *pObj=pSymbolList->GetObj(i);
            if(pObj==NULL)
                break;
            pObj=pObj->Clone();
            //const String* pGrfName = (const String*)aGrfNames.GetObject(i);
            String *pStr=new String();//String(i));
            aGrfNames.Insert(pStr,LIST_APPEND);
            //Rectangle aRect(pObj->GetLogicRect());
            pPage->NbcInsertObject(pObj);
            pView->MarkObj(pObj,pPageView);
            if(pSymbolAttr)
            {
                pObj->SetMergedItemSet(*pSymbolAttr);
            }
            else
            {
                pObj->SetMergedItemSet(rOutAttrs);
            }

            Bitmap aBitmap(pView->GetAllMarkedBitmap());
            GDIMetaFile aMeta(pView->GetAllMarkedMetaFile());
            pView->UnmarkAll();
            pObj=pPage->RemoveObject(0);
            SdrObject::Free(pObj);

            SvxBrushItem* pBrushItem = new SvxBrushItem(Graphic(aMeta), GPOS_AREA, SID_ATTR_BRUSH);
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
//Handler f�r Popup-Menue der Symbolauswahl (NumMenueButton)
//der folgende Link stammt urspruenglich aus SvxNumOptionsTabPage:
IMPL_STATIC_LINK(SvxLineTabPage, GraphicArrivedHdl_Impl, SvxBrushItem*, pItem)
{
    PopupMenu* pPopup = pThis->aSymbolMB.GetPopupMenu()->GetPopupMenu( MN_GALLERY );

    SvxBmpItemInfo* pBmpInfo = 0;
    for ( USHORT i = 0; i < pThis->aGrfBrushItems.Count(); i++ )
    {
        SvxBmpItemInfo* pInfo = (SvxBmpItemInfo*)pThis->aGrfBrushItems.GetObject(i);
        if( pInfo->pBrushItem == pItem )
        {
            pBmpInfo = pInfo; break;
        }
    }
    if( pBmpInfo )
    {
        if( pItem->GetGraphic() )
        {
            Bitmap aBitmap(pItem->GetGraphic()->GetBitmap());
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
            pPopup->SetItemImage( pBmpInfo->nItemId, aImage );
        }
    }

    return 0;
}

//#58425# Symbole auf einer Linie (z.B. StarChart)
//Handler f�r Menuebutton
IMPL_LINK( SvxLineTabPage, GraphicHdl_Impl, MenuButton *, pButton )
{
    USHORT nItemId = pButton->GetCurItemId();
    const Graphic* pGraphic = 0;
    Graphic aGraphic;
    String aGrfName;
    BOOL bResetSize = FALSE;
    BOOL bEnable=TRUE;
    long nPreviousSymbolType = nSymbolType;

    if(nItemId >= MN_GALLERY_ENTRY)
    {
        if( (nItemId-MN_GALLERY_ENTRY) >= nNumMenuGalleryItems)
        {
            nSymbolType=nItemId-MN_GALLERY_ENTRY-nNumMenuGalleryItems; //Index der Liste
        }
        else
        {
            nSymbolType=SVX_SYMBOLTYPE_BRUSHITEM;
            bResetSize = TRUE;
        }
        SvxBmpItemInfo* pInfo = (SvxBmpItemInfo*)aGrfBrushItems.GetObject(nItemId - MN_GALLERY_ENTRY);
        pGraphic = pInfo->pBrushItem->GetGraphic();
    }
    else switch(nItemId)
    {
        case MN_SYMBOLS_AUTO:
        {
            pGraphic=&aAutoSymbolGraphic;
            aAutoSymbolGraphic.SetPrefSize( Size(253,253) );
            nSymbolType=SVX_SYMBOLTYPE_AUTO;
        }
        break;

        case MN_SYMBOLS_NONE:
        {
            nSymbolType=SVX_SYMBOLTYPE_NONE;
            pGraphic=NULL;
            bEnable=FALSE;
        }
        break;
        default:
        {
            SvxOpenGraphicDialog aGrfDlg(SVX_RES(RID_STR_EDIT_GRAPHIC));
            aGrfDlg.EnableLink(sal_False);
            aGrfDlg.AsLink(sal_False);
            if( !aGrfDlg.Execute() )
            {
                // ausgewaehlten Filter merken
                aGrfName = aGrfDlg.GetPath();
                if( !aGrfDlg.GetGraphic(aGraphic) )
                {
                    nSymbolType=SVX_SYMBOLTYPE_BRUSHITEM;
                    pGraphic = &aGraphic;
                    bResetSize = TRUE;
                }
            }
            if( !pGraphic )
                return 0;
        }
        break;
    }

    if(pGraphic)
    {
        Size aSize = SvxNumberFormat::GetGraphicSizeMM100(pGraphic);
        aSize = OutputDevice::LogicToLogic(aSize, MAP_100TH_MM, (MapUnit)ePoolUnit);
        aSymbolGraphic=*pGraphic;
        if( bResetSize )
        {
            aSymbolSize=aSize;
        }
        else if( nPreviousSymbolType == SVX_SYMBOLTYPE_BRUSHITEM )
        {   //#i31097# Data Point Symbol size changes when a different symbol is choosen(maoyg)
            if( aSymbolSize.Width() != aSymbolSize.Height() )
            {
                aSize.setWidth( (long)( aSymbolSize.Width() + aSymbolSize.Height() )/2 );
                aSize.setHeight( (long)( aSymbolSize.Width() + aSymbolSize.Height() )/2 );
                aSymbolSize = aSize;
            }
        }
        aCtlPreview.SetSymbol(&aSymbolGraphic,aSymbolSize);
    }
    else
    {
        aSymbolGraphic=Graphic();
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

    return 0;
}
IMPL_LINK( SvxLineTabPage, SizeHdl_Impl, MetricField *, pField)
{
    bNewSize=true;
    BOOL bWidth = (BOOL)(pField == &aSymbolWidthMF);
    bLastWidthModified = bWidth;
    BOOL bRatio = aSymbolRatioCB.IsChecked();
    long nWidthVal = static_cast<long>(aSymbolWidthMF.Denormalize(aSymbolWidthMF.GetValue(FUNIT_100TH_MM)));
    long nHeightVal= static_cast<long>(aSymbolHeightMF.Denormalize(aSymbolHeightMF.GetValue(FUNIT_100TH_MM)));
    nWidthVal = OutputDevice::LogicToLogic(nWidthVal,MAP_100TH_MM,(MapUnit)ePoolUnit );
    nHeightVal = OutputDevice::LogicToLogic(nHeightVal,MAP_100TH_MM,(MapUnit)ePoolUnit);
    aSymbolSize=Size(nWidthVal,nHeightVal);
    double  fSizeRatio = (double)1;

    if(bRatio)
    {
        if (aSymbolLastSize.Height() && aSymbolLastSize.Width())
            fSizeRatio = (double)aSymbolLastSize.Width() / aSymbolLastSize.Height();
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


void SvxLineTabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    SvxTabPage::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        FillListboxes();
    }
}

void SvxLineTabPage::PageCreated (SfxAllItemSet aSet) //add CHINA001
{
    SFX_ITEMSET_ARG (&aSet,pColorTabItem,SvxColorTableItem,SID_COLOR_TABLE,sal_False);
    SFX_ITEMSET_ARG (&aSet,pDashListItem,SvxDashListItem,SID_DASH_LIST,sal_False);
    SFX_ITEMSET_ARG (&aSet,pLineEndListItem,SvxLineEndListItem,SID_LINEEND_LIST,sal_False);
    SFX_ITEMSET_ARG (&aSet,pPageTypeItem,SfxUInt16Item,SID_PAGE_TYPE,sal_False);
    SFX_ITEMSET_ARG (&aSet,pDlgTypeItem,SfxUInt16Item,SID_DLG_TYPE,sal_False);
    SFX_ITEMSET_ARG (&aSet,pSdrObjListItem,OfaPtrItem,SID_OBJECT_LIST,sal_False);
    SFX_ITEMSET_ARG (&aSet,pSymbolAttrItem,SfxTabDialogItem,SID_ATTR_SET,sal_False);
    SFX_ITEMSET_ARG (&aSet,pGraphicItem,SvxGraphicItem,SID_GRAPHIC,sal_False);

    if (pColorTabItem)
        SetColorTable(pColorTabItem->GetColorTable());
    if (pDashListItem)
        SetDashList(pDashListItem->GetDashList());
    if (pLineEndListItem)
        SetLineEndList(pLineEndListItem->GetLineEndList());
    if (pPageTypeItem)
        SetPageType(pPageTypeItem->GetValue());
    if (pDlgTypeItem)
        SetDlgType(pDlgTypeItem->GetValue());
    Construct();

    if(pSdrObjListItem) //symbols
    {
        ShowSymbolControls(TRUE);
        pSymbolList = static_cast<SdrObjList*>(pSdrObjListItem->GetValue());
        if (pSymbolAttrItem)
            pSymbolAttr = new SfxItemSet(pSymbolAttrItem->GetItemSet());
        if(pGraphicItem)
            aAutoSymbolGraphic = pGraphicItem->GetGraphic();
    }
}
