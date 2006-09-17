/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tphatch.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 04:44:38 $
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

#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFXMODULE_HXX
#include <sfx2/module.hxx>
#endif
#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif

#define _SVX_TPHATCH_CXX

#include "dialogs.hrc"
#include "tabarea.hrc"
#include "dlgname.hrc"
#include "helpid.hrc"

#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE
#define ITEMID_HATCH_LIST       SID_HATCH_LIST

#include "xattr.hxx"
#include "xpool.hxx"
#include "xtable.hxx"
#include "xoutx.hxx"

#include "drawitem.hxx"
#include "cuitabarea.hxx"
#include "defdlgname.hxx" //CHINA001 #include "dlgname.hxx"
#include "svxdlg.hxx" //CHINA001
#include "dialmgr.hxx"
#include "dlgutil.hxx"

#define DLGWIN this->GetParent()->GetParent()

#define BITMAP_WIDTH  32
#define BITMAP_HEIGHT 12

/*************************************************************************
|*
|*  Dialog zum Aendern und Definieren der Schraffuren
|*
\************************************************************************/

SvxHatchTabPage::SvxHatchTabPage
(
    Window* pParent,
    const SfxItemSet& rInAttrs
) :

    SvxTabPage          ( pParent, SVX_RES( RID_SVXPAGE_HATCH ), rInAttrs ),

    aFtDistance         ( this, ResId( FT_LINE_DISTANCE ) ),
    aMtrDistance        ( this, ResId( MTR_FLD_DISTANCE ) ),
    aFtAngle            ( this, ResId( FT_LINE_ANGLE ) ),
    aMtrAngle           ( this, ResId( MTR_FLD_ANGLE ) ),
    aCtlAngle           ( this, ResId( CTL_ANGLE ),
                                    RP_RB, 200, 80, CS_ANGLE ),
    aFlProp             ( this, ResId( FL_PROP ) ),
    aFtLineType         ( this, ResId( FT_LINE_TYPE ) ),
    aLbLineType         ( this, ResId( LB_LINE_TYPE ) ),
    aFtLineColor        ( this, ResId( FT_LINE_COLOR ) ),
    aLbLineColor        ( this, ResId( LB_LINE_COLOR ) ),
    aLbHatchings        ( this, ResId( LB_HATCHINGS ) ),
    aCtlPreview         ( this, ResId( CTL_PREVIEW ), &XOut ),
    aBtnAdd             ( this, ResId( BTN_ADD ) ),
    aBtnModify          ( this, ResId( BTN_MODIFY ) ),
    aBtnDelete          ( this, ResId( BTN_DELETE ) ),
    aBtnLoad            ( this, ResId( BTN_LOAD ) ),
    aBtnSave            ( this, ResId( BTN_SAVE ) ),

    rOutAttrs           ( rInAttrs ),
    pColorTab( NULL ),
    pHatchingList( NULL ),

    pXPool              ( (XOutdevItemPool*) rInAttrs.GetPool() ),
    XOut                ( &aCtlPreview ),
    aXFStyleItem        ( XFILL_HATCH ),
    aXHatchItem         ( String(), XHatch() ),
    aXFillAttr          ( pXPool ),
    rXFSet              ( aXFillAttr.GetItemSet() )

{
    aBtnLoad.SetModeImage( Image( ResId( RID_SVXIMG_LOAD_H ) ), BMP_COLOR_HIGHCONTRAST );
    aBtnSave.SetModeImage( Image( ResId( RID_SVXIMG_SAVE_H ) ), BMP_COLOR_HIGHCONTRAST );

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
            break;
        default: ;//prevent warning
    }
    SetFieldUnit( aMtrDistance, eFUnit );

    // PoolUnit ermitteln
    SfxItemPool* pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool?" );
    ePoolUnit = pPool->GetMetric( SID_ATTR_FILL_HATCH );

    // Setzen des Output-Devices
    rXFSet.Put( aXFStyleItem );
    rXFSet.Put( aXHatchItem );
    XOut.SetFillAttr( aXFillAttr.GetItemSet() );

    // Set line at the OutputDevice
    XLineAttrSetItem aXLineAttr( pXPool );
    aXLineAttr.GetItemSet().Put( XLineStyleItem( XLINE_SOLID ) );
    aXLineAttr.GetItemSet().Put( XLineWidthItem( 1 ));
    XOut.SetLineAttr( aXLineAttr.GetItemSet() );

    aLbHatchings.SetSelectHdl( LINK( this, SvxHatchTabPage, ChangeHatchHdl_Impl ) );

    Link aLink = LINK( this, SvxHatchTabPage, ModifiedHdl_Impl );
    aMtrDistance.SetModifyHdl( aLink );
    aMtrAngle.SetModifyHdl( aLink );
    aLbLineType.SetSelectHdl( aLink );
    aLbLineColor.SetSelectHdl( aLink );

    aBtnAdd.SetClickHdl( LINK( this, SvxHatchTabPage, ClickAddHdl_Impl ) );
    aBtnModify.SetClickHdl(
        LINK( this, SvxHatchTabPage, ClickModifyHdl_Impl ) );
    aBtnDelete.SetClickHdl(
        LINK( this, SvxHatchTabPage, ClickDeleteHdl_Impl ) );
    aBtnLoad.SetClickHdl( LINK( this, SvxHatchTabPage, ClickLoadHdl_Impl ) );
    aBtnSave.SetClickHdl( LINK( this, SvxHatchTabPage, ClickSaveHdl_Impl ) );

    aCtlPreview.SetDrawMode( GetDisplayBackground().GetColor().IsDark() ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR );
}

// -----------------------------------------------------------------------

void SvxHatchTabPage::Construct()
{
    // Farbtabelle
    aLbLineColor.Fill( pColorTab );

    // Schraffurentabelle
    aLbHatchings.Fill( pHatchingList );
}

// -----------------------------------------------------------------------

void SvxHatchTabPage::ActivatePage( const SfxItemSet& rSet )
{
    int nPos;
    int nCount;

    if( *pDlgType == 0 ) // Flaechen-Dialog
    {
        *pbAreaTP = FALSE;

        if( pColorTab )
        {
            // ColorTable
            if( *pnColorTableState & CT_CHANGED ||
                *pnColorTableState & CT_MODIFIED )
            {
                if( *pnColorTableState & CT_CHANGED )
                    pColorTab = ( (SvxAreaTabDialog*) DLGWIN )->GetNewColorTable();

                // LbLineColor
                nPos = aLbLineColor.GetSelectEntryPos();
                aLbLineColor.Clear();
                aLbLineColor.Fill( pColorTab );
                nCount = aLbLineColor.GetEntryCount();
                if( nCount == 0 )
                    ; // Dieser Fall sollte nicht auftreten
                else if( nCount <= nPos )
                    aLbLineColor.SelectEntryPos( 0 );
                else
                    aLbLineColor.SelectEntryPos( nPos );

                ModifiedHdl_Impl( this );
            }

            // Ermitteln (evtl. abschneiden) des Namens und in
            // der GroupBox darstellen
            String          aString( SVX_RES( RID_SVXSTR_TABLE ) ); aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );
            INetURLObject   aURL( pHatchingList->GetPath() );

            aURL.Append( pHatchingList->GetName() );
            DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

            if ( aURL.getBase().getLength() > 18 )
            {
                aString += String(aURL.getBase()).Copy( 0, 15 );
                aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
            }
            else
                aString += String(aURL.getBase());

            if( *pPageType == PT_HATCH && *pPos != LISTBOX_ENTRY_NOTFOUND )
            {
                aLbHatchings.SelectEntryPos( *pPos );
            }
            // Farben koennten geloescht worden sein
            ChangeHatchHdl_Impl( this );

            *pPageType = PT_HATCH;
            *pPos = LISTBOX_ENTRY_NOTFOUND;
        }
    }

    rXFSet.Put ( ( XFillColorItem& )    rSet.Get(XATTR_FILLCOLOR) );
    rXFSet.Put ( ( XFillBackgroundItem&)rSet.Get(XATTR_FILLBACKGROUND) );
    XOut.SetFillAttr( aXFillAttr.GetItemSet() );
    aCtlPreview.Invalidate();
}

// -----------------------------------------------------------------------

int SvxHatchTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( CheckChanges_Impl() == -1L )
        return KEEP_PAGE;

    if( _pSet )
        FillItemSet( *_pSet );

    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

long SvxHatchTabPage::CheckChanges_Impl()
{
    if( aMtrDistance.GetText()           != aMtrDistance.GetSavedValue() ||
        aMtrAngle.GetText()              != aMtrAngle.GetSavedValue() ||
        aLbLineType.GetSelectEntryPos()  != aLbLineType.GetSavedValue()  ||
        aLbLineColor.GetSelectEntryPos() != aLbLineColor.GetSavedValue() ||
        aLbHatchings.GetSelectEntryPos() != aLbHatchings.GetSavedValue() )
    {
        ResMgr* pMgr = DIALOG_MGR();
        Image aWarningBoxImage = WarningBox::GetStandardImage();
        //CHINA001 SvxMessDialog aMessDlg( DLGWIN,
        //CHINA001  String( ResId( RID_SVXSTR_HATCH, pMgr ) ),
        //CHINA001  String( ResId( RID_SVXSTR_ASK_CHANGE_HATCH, pMgr ) ),
        //CHINA001  &aWarningBoxImage );
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "Dialogdiet fail!");//CHINA001
        AbstractSvxMessDialog* aMessDlg = pFact->CreateSvxMessDialog( DLGWIN, ResId(RID_SVXDLG_MESSBOX),
                                                        String( ResId( RID_SVXSTR_HATCH, pMgr ) ),
                                                        String( ResId( RID_SVXSTR_ASK_CHANGE_HATCH, pMgr ) ),
                                                        &aWarningBoxImage );
        DBG_ASSERT(aMessDlg, "Dialogdiet fail!");//CHINA001
        aMessDlg->SetButtonText( MESS_BTN_1, //CHINA001 aMessDlg.SetButtonText( MESS_BTN_1,
                                String( ResId( RID_SVXSTR_CHANGE, pMgr ) ) );
        aMessDlg->SetButtonText( MESS_BTN_2, //CHINA001 aMessDlg.SetButtonText( MESS_BTN_2,
                                String( ResId( RID_SVXSTR_ADD, pMgr ) ) );

        short nRet = aMessDlg->Execute(); //CHINA001 short nRet = aMessDlg.Execute();

        switch( nRet )
        {
            case RET_BTN_1: // Aendern
            {
                ClickModifyHdl_Impl( this );
            }
            break;

            case RET_BTN_2: // Hinzufuegen
            {
                ClickAddHdl_Impl( this );
            }
            break;

            case RET_CANCEL:
                // return( -1L ); <-- wuerde die Seite nicht verlassen
            break;
            // return( TRUE ); // Abbruch
        }
        delete aMessDlg; //add by CHINA001
    }

    USHORT nPos = aLbHatchings.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
        *pPos = nPos;
    return 0L;
}

// -----------------------------------------------------------------------

BOOL SvxHatchTabPage::FillItemSet( SfxItemSet& rSet )
{
    if( *pDlgType == 0 && *pbAreaTP == FALSE ) // Flaechen-Dialog
    {
        if( *pPageType == PT_HATCH )
        {
            // CheckChanges(); <-- doppelte Abfrage ?

            XHatch* pXHatch = NULL;
            String  aString;
            USHORT  nPos = aLbHatchings.GetSelectEntryPos();
            if( nPos != LISTBOX_ENTRY_NOTFOUND )
            {
                pXHatch = new XHatch( pHatchingList->GetHatch( nPos )->GetHatch() );
                aString = aLbHatchings.GetSelectEntry();
            }
            // Farbverlauf wurde (unbekannt) uebergeben
            else
            {
                pXHatch = new XHatch( aLbLineColor.GetSelectEntryColor(),
                                 (XHatchStyle) aLbLineType.GetSelectEntryPos(),
                                 GetCoreValue( aMtrDistance, ePoolUnit ),
                                 aMtrAngle.GetValue() * 10 );
            }
            DBG_ASSERT( pXHatch, "XHatch konnte nicht erzeugt werden" );
            rSet.Put( XFillStyleItem( XFILL_HATCH ) );
            rSet.Put( XFillHatchItem( aString, *pXHatch ) );

            delete pXHatch;
        }
    }
    return TRUE;
}

// -----------------------------------------------------------------------

void SvxHatchTabPage::Reset( const SfxItemSet& rSet )
{
    // aLbHatchings.SelectEntryPos( 0 );
    ChangeHatchHdl_Impl( this );

    // Status der Buttons ermitteln
    if( pHatchingList->Count() )
    {
        aBtnModify.Enable();
        aBtnDelete.Enable();
        aBtnSave.Enable();
    }
    else
    {
        aBtnModify.Disable();
        aBtnDelete.Disable();
        aBtnSave.Disable();
    }

    rXFSet.Put ( ( XFillColorItem& )    rSet.Get(XATTR_FILLCOLOR) );
    rXFSet.Put ( ( XFillBackgroundItem&)rSet.Get(XATTR_FILLBACKGROUND) );
    XOut.SetFillAttr( aXFillAttr.GetItemSet() );
    aCtlPreview.Invalidate();
}

// -----------------------------------------------------------------------

SfxTabPage* SvxHatchTabPage::Create( Window* pWindow,
                const SfxItemSet& rSet )
{
    return new SvxHatchTabPage( pWindow, rSet );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxHatchTabPage, ModifiedHdl_Impl, void *, p )
{
    if( p == &aMtrAngle )
    {
        // Setzen des Winkels im AngleControl
        switch( aMtrAngle.GetValue() )
        {
            case 135: aCtlAngle.SetActualRP( RP_LT ); break;
            case  90: aCtlAngle.SetActualRP( RP_MT ); break;
            case  45: aCtlAngle.SetActualRP( RP_RT ); break;
            case 180: aCtlAngle.SetActualRP( RP_LM ); break;
            case   0: aCtlAngle.SetActualRP( RP_RM ); break;
            case 225: aCtlAngle.SetActualRP( RP_LB ); break;
            case 270: aCtlAngle.SetActualRP( RP_MB ); break;
            case 315: aCtlAngle.SetActualRP( RP_RB ); break;
            default:  aCtlAngle.SetActualRP( RP_MM ); break;
        }
    }

    XHatch aXHatch( aLbLineColor.GetSelectEntryColor(),
                    (XHatchStyle) aLbLineType.GetSelectEntryPos(),
                    GetCoreValue( aMtrDistance, ePoolUnit ),
                    aMtrAngle.GetValue() * 10 );

    rXFSet.Put( XFillHatchItem( String(), aXHatch ) );
    XOut.SetFillAttr( aXFillAttr.GetItemSet() );

    aCtlPreview.Invalidate();

    return 0L;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxHatchTabPage, ChangeHatchHdl_Impl, void *, EMPTYARG )
{
    XHatch* pHatch = NULL;
    int nPos = aLbHatchings.GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
        pHatch = new XHatch( ( (XHatchEntry*) pHatchingList->GetHatch( nPos ) )->GetHatch() );
    else
    {
        const SfxPoolItem* pPoolItem = NULL;
        if( SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLSTYLE ), TRUE, &pPoolItem ) )
        {
            if( ( XFILL_HATCH == (XFillStyle) ( ( const XFillStyleItem* ) pPoolItem )->GetValue() ) &&
                ( SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLHATCH ), TRUE, &pPoolItem ) ) )
            {
                pHatch = new XHatch( ( ( const XFillHatchItem* ) pPoolItem )->GetHatchValue() );
            }
        }
        if( !pHatch )
        {
            aLbHatchings.SelectEntryPos( 0 );
            nPos = aLbHatchings.GetSelectEntryPos();
            if( nPos != LISTBOX_ENTRY_NOTFOUND )
                pHatch = new XHatch( ( (XHatchEntry*) pHatchingList->GetHatch( nPos ) )->GetHatch() );
        }
    }
    if( pHatch )
    {
        aLbLineType.SelectEntryPos( pHatch->GetHatchStyle() );
        // Wenn der Eintrag nicht in der Listbox ist, wird die Farbe
        // temporaer hinzugenommen
        aLbLineColor.SetNoSelection();
        aLbLineColor.SelectEntry( pHatch->GetColor() );
        if( aLbLineColor.GetSelectEntryCount() == 0 )
        {
            aLbLineColor.InsertEntry( pHatch->GetColor(), String() );
            aLbLineColor.SelectEntry( pHatch->GetColor() );
        }
        SetMetricValue( aMtrDistance, pHatch->GetDistance(), ePoolUnit );
        aMtrAngle.SetValue( pHatch->GetAngle() / 10 );

        // Setzen des Winkels im AngleControl
        switch( aMtrAngle.GetValue() )
        {
            case 135: aCtlAngle.SetActualRP( RP_LT ); break;
            case  90: aCtlAngle.SetActualRP( RP_MT ); break;
            case  45: aCtlAngle.SetActualRP( RP_RT ); break;
            case 180: aCtlAngle.SetActualRP( RP_LM ); break;
            case   0: aCtlAngle.SetActualRP( RP_RM ); break;
            case 225: aCtlAngle.SetActualRP( RP_LB ); break;
            case 270: aCtlAngle.SetActualRP( RP_MB ); break;
            case 315: aCtlAngle.SetActualRP( RP_RB ); break;
            default:  aCtlAngle.SetActualRP( RP_MM ); break;
        }

        // Backgroundcolor
        /*
        const SfxPoolItem* pPoolItem = NULL;
        if( SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLBACKGROUND ), TRUE, &pPoolItem ) )
        {
            rXFSet.Put ( XFillBackgroundItem( ( ( XFillBackgroundItem* )pPoolItem)->GetValue() ) );
            if( SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLCOLOR ), TRUE, &pPoolItem ) )
            {
                Color aColor( ( ( const XFillColorItem* ) pPoolItem )->GetValue() );
                rXFSet.Put( XFillColorItem( String(), aColor ) );
            }
        }
        */
        // ItemSet fuellen und an XOut weiterleiten
        rXFSet.Put( XFillHatchItem( String(), *pHatch ) );
        XOut.SetFillAttr( aXFillAttr.GetItemSet() );

        aCtlPreview.Invalidate();
        delete pHatch;
    }
    aMtrDistance.SaveValue();
    aMtrAngle.SaveValue();
    aLbLineType.SaveValue();
    aLbLineColor.SaveValue();
    aLbHatchings.SaveValue();

    return 0L;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxHatchTabPage, ClickAddHdl_Impl, void *, EMPTYARG )
{
    ResMgr* pMgr = DIALOG_MGR();
    String aNewName( ResId( RID_SVXSTR_HATCH, pMgr ) );
    String aDesc( ResId( RID_SVXSTR_DESC_HATCH, pMgr ) );
    String aName;

    long nCount = pHatchingList->Count();
    long j = 1;
    BOOL bDifferent = FALSE;

    while( !bDifferent )
    {
        aName  = aNewName;
        aName += sal_Unicode(' ');
        aName += UniString::CreateFromInt32( j++ );
        bDifferent = TRUE;

        for( long i = 0; i < nCount && bDifferent; i++ )
            if( aName == pHatchingList->GetHatch( i )->GetName() )
                bDifferent = FALSE;
    }

    //CHINA001 SvxNameDialog* pDlg     = new SvxNameDialog( DLGWIN, aName, aDesc );
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "Dialogdiet fail!");//CHINA001
    AbstractSvxNameDialog* pDlg = pFact->CreateSvxNameDialog( DLGWIN, aName, aDesc, ResId(RID_SVXDLG_NAME) );
    DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
    WarningBox*    pWarnBox = NULL;
    USHORT         nError   = RID_SVXSTR_WARN_NAME_DUPLICATE;

    while( pDlg->Execute() == RET_OK )
    {
        pDlg->GetName( aName );

        bDifferent = TRUE;

        for( long i = 0; i < nCount && bDifferent; i++ )
            if( aName == pHatchingList->GetHatch( i )->GetName() )
                bDifferent = FALSE;

        if( bDifferent ) {
            nError = 0;
            break;
        }

        if( !pWarnBox )
        {
            pWarnBox = new WarningBox( DLGWIN,
                                       WinBits( WB_OK_CANCEL ),
                                       String( ResId( nError, pMgr ) ) );
            pWarnBox->SetHelpId( HID_WARN_NAME_DUPLICATE );
        }

        if( pWarnBox->Execute() != RET_OK )
            break;
    }
    //Rectangle aDlgRect( pDlg->GetPosPixel(), pDlg->GetSizePixel() );
    delete pDlg;
    delete pWarnBox;

    if( !nError )
    {
        XHatch aXHatch( aLbLineColor.GetSelectEntryColor(),
                        (XHatchStyle) aLbLineType.GetSelectEntryPos(),
                        GetCoreValue( aMtrDistance, ePoolUnit ),
                        aMtrAngle.GetValue() * 10 );
        XHatchEntry* pEntry = new XHatchEntry( aXHatch, aName );

        pHatchingList->Insert( pEntry, nCount );

        aLbHatchings.Append( pEntry );

        aLbHatchings.SelectEntryPos( aLbHatchings.GetEntryCount() - 1 );

#ifdef WNT
        // hack: #31355# W.P.
        Rectangle aRect( aLbHatchings.GetPosPixel(), aLbHatchings.GetSizePixel() );
        if( TRUE ) {                // ??? overlapped with pDlg
                                    // and srolling
            Invalidate( aRect );
            //aLbHatchings.Invalidate();
        }
#endif

        // Flag fuer modifiziert setzen
        *pnHatchingListState |= CT_MODIFIED;

        ChangeHatchHdl_Impl( this );
    }

    // Status der Buttons ermitteln
    if( pHatchingList->Count() )
    {
        aBtnModify.Enable();
        aBtnDelete.Enable();
        aBtnSave.Enable();
    }
    return 0L;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxHatchTabPage, ClickModifyHdl_Impl, void *, EMPTYARG )
{
    int nPos = aLbHatchings.GetSelectEntryPos();

    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        ResMgr* pMgr = DIALOG_MGR();
        String aNewName( ResId( RID_SVXSTR_HATCH, pMgr ) );
        String aDesc( ResId( RID_SVXSTR_DESC_HATCH, pMgr ) );
        String aName( pHatchingList->GetHatch( nPos )->GetName() );
        String aOldName = aName;

        //CHINA001 SvxNameDialog* pDlg = new SvxNameDialog( DLGWIN, aName, aDesc );
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "Dialogdiet fail!");//CHINA001
        AbstractSvxNameDialog* pDlg = pFact->CreateSvxNameDialog( DLGWIN, aName, aDesc, ResId(RID_SVXDLG_NAME) );
        DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001

        long nCount = pHatchingList->Count();
        BOOL bDifferent = FALSE;
        BOOL bLoop = TRUE;
        while( bLoop && pDlg->Execute() == RET_OK )
        {
            pDlg->GetName( aName );
            bDifferent = TRUE;

            for( long i = 0; i < nCount && bDifferent; i++ )
            {
                if( aName == pHatchingList->GetHatch( i )->GetName() &&
                    aName != aOldName )
                    bDifferent = FALSE;
            }

            if( bDifferent )
            {
                bLoop = FALSE;
                XHatch aXHatch( aLbLineColor.GetSelectEntryColor(),
                                (XHatchStyle) aLbLineType.GetSelectEntryPos(),
                                 GetCoreValue( aMtrDistance, ePoolUnit ),
                                aMtrAngle.GetValue() * 10 );

                XHatchEntry* pEntry = new XHatchEntry( aXHatch, aName );

                delete pHatchingList->Replace( pEntry, nPos );

                aLbHatchings.Modify( pEntry, nPos );

                aLbHatchings.SelectEntryPos( nPos );

                // Werte sichern fuer Changes-Erkennung ( -> Methode )
                aMtrDistance.SaveValue();
                aMtrAngle.SaveValue();
                aLbLineType.SaveValue();
                aLbLineColor.SaveValue();
                aLbHatchings.SaveValue();

                // Flag fuer modifiziert setzen
                *pnHatchingListState |= CT_MODIFIED;
            }
            else
            {
                WarningBox aBox( DLGWIN, WinBits( WB_OK ),String( ResId( RID_SVXSTR_WARN_NAME_DUPLICATE, pMgr ) ) );
                aBox.SetHelpId( HID_WARN_NAME_DUPLICATE );
                aBox.Execute();
            }
        }
        delete( pDlg );
    }
    return 0L;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxHatchTabPage, ClickDeleteHdl_Impl, void *, EMPTYARG )
{
    int nPos = aLbHatchings.GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        QueryBox aQueryBox( DLGWIN, WinBits( WB_YES_NO | WB_DEF_NO ),
//!         SVX_RES( RID_SVXSTR_DEL_HATCH ),
            String( SVX_RES( RID_SVXSTR_ASK_DEL_HATCH ) ) );

        if( aQueryBox.Execute() == RET_YES )
        {
            delete pHatchingList->Remove( nPos );
            aLbHatchings.RemoveEntry( nPos );
            aLbHatchings.SelectEntryPos( 0 );

            aCtlPreview.Invalidate();

            ChangeHatchHdl_Impl( this );

            // Flag fuer modifiziert setzen
            *pnHatchingListState |= CT_MODIFIED;
        }
    }
    // Status der Buttons ermitteln
    if( !pHatchingList->Count() )
    {
        aBtnModify.Disable();
        aBtnDelete.Disable();
        aBtnSave.Disable();
    }
    return 0L;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxHatchTabPage, ClickLoadHdl_Impl, void *, EMPTYARG )
{
    ResMgr* pMgr = DIALOG_MGR();
    USHORT nReturn = RET_YES;

    if ( *pnHatchingListState & CT_MODIFIED )
    {
        nReturn = WarningBox( DLGWIN, WinBits( WB_YES_NO_CANCEL ),
            String( ResId( RID_SVXSTR_WARN_TABLE_OVERWRITE, pMgr ) ) ).Execute();

        if ( nReturn == RET_YES )
            pHatchingList->Save();
    }

    if ( nReturn != RET_CANCEL )
    {
        ::sfx2::FileDialogHelper aDlg( ::sfx2::FILEOPEN_SIMPLE, 0 );
        String aStrFilterType( RTL_CONSTASCII_USTRINGPARAM( "*.soh" ) );
        aDlg.AddFilter( aStrFilterType, aStrFilterType );
        INetURLObject aFile( SvtPathOptions().GetPalettePath() );
        aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::NO_DECODE ) );

        if( aDlg.Execute() == ERRCODE_NONE )
        {
            INetURLObject aURL( aDlg.GetPath() );
            INetURLObject aPathURL( aURL );

            aPathURL.removeSegment();
            aPathURL.removeFinalSlash();

            // Liste speichern
            XHatchList* pHatchList = new XHatchList( aPathURL.GetMainURL( INetURLObject::NO_DECODE ), pXPool );
            pHatchList->SetName( aURL.getName() );
            if( pHatchList->Load() )
            {
                if( pHatchList )
                {
                    // Pruefen, ob Tabelle geloescht werden darf:
                    if( pHatchingList != ( (SvxAreaTabDialog*) DLGWIN )->GetHatchingList() )
                        delete pHatchingList;

                    pHatchingList = pHatchList;
                    ( (SvxAreaTabDialog*) DLGWIN )->SetNewHatchingList( pHatchingList );

                    aLbHatchings.Clear();
                    aLbHatchings.Fill( pHatchingList );
                    Reset( rOutAttrs );

                    pHatchingList->SetName( aURL.getName() );

                    // Ermitteln (evtl. abschneiden) des Namens und in
                    // der GroupBox darstellen
                    String aString( ResId( RID_SVXSTR_TABLE, pMgr ) );
                    aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );

                    if ( aURL.getBase().getLength() > 18 )
                    {
                        aString += String(aURL.getBase()).Copy( 0, 15 );
                        aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
                    }
                    else
                        aString += String(aURL.getBase());

                    // Flag fuer gewechselt setzen
                    *pnHatchingListState |= CT_CHANGED;
                    // Flag fuer modifiziert entfernen
                    *pnHatchingListState &= ~CT_MODIFIED;
                }
            }
            else
                ErrorBox( DLGWIN, WinBits( WB_OK ),
                    String( ResId( RID_SVXSTR_READ_DATA_ERROR, pMgr ) ) ).Execute();
        }
    }

    // Status der Buttons ermitteln
    if ( pHatchingList->Count() )
    {
        aBtnModify.Enable();
        aBtnDelete.Enable();
        aBtnSave.Enable();
    }
    else
    {
        aBtnModify.Disable();
        aBtnDelete.Disable();
        aBtnSave.Disable();
    }
    return 0L;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxHatchTabPage, ClickSaveHdl_Impl, void *, EMPTYARG )
{
       ::sfx2::FileDialogHelper aDlg( ::sfx2::FILESAVE_SIMPLE, 0 );
    String aStrFilterType( RTL_CONSTASCII_USTRINGPARAM( "*.soh" ) );
    aDlg.AddFilter( aStrFilterType, aStrFilterType );

    INetURLObject aFile( SvtPathOptions().GetPalettePath() );
    DBG_ASSERT( aFile.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    if( pHatchingList->GetName().Len() )
    {
        aFile.Append( pHatchingList->GetName() );

        if( !aFile.getExtension().getLength() )
            aFile.SetExtension( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "soh" ) ) );
    }

    aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::NO_DECODE ) );
    if ( aDlg.Execute() == ERRCODE_NONE )
    {
        INetURLObject aURL( aDlg.GetPath() );
        INetURLObject aPathURL( aURL );

        aPathURL.removeSegment();
        aPathURL.removeFinalSlash();

        pHatchingList->SetName( aURL.getName() );
        pHatchingList->SetPath( aPathURL.GetMainURL( INetURLObject::NO_DECODE ) );

        if( pHatchingList->Save() )
        {
            // Ermitteln (evtl. abschneiden) des Namens und in
            // der GroupBox darstellen
            String aString( SVX_RES( RID_SVXSTR_TABLE ) );
            aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );

            if ( aURL.getBase().getLength() > 18 )
            {
                aString += String(aURL.getBase()).Copy( 0, 15 );
                aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
            }
            else
                aString += String(aURL.getBase());

            // Flag fuer gespeichert setzen
            *pnHatchingListState |= CT_SAVED;
            // Flag fuer modifiziert entfernen
            *pnHatchingListState &= ~CT_MODIFIED;
        }
        else
        {
            ErrorBox( DLGWIN, WinBits( WB_OK ),
                String( SVX_RES( RID_SVXSTR_WRITE_DATA_ERROR ) ) ).Execute();
        }
    }

    return 0L;
}

//------------------------------------------------------------------------

void SvxHatchTabPage::PointChanged( Window* pWindow, RECT_POINT eRcPt )
{
    if( pWindow == &aCtlAngle )
    {
        switch( eRcPt )
        {
            case RP_LT: aMtrAngle.SetValue( 135 ); break;
            case RP_MT: aMtrAngle.SetValue( 90 );  break;
            case RP_RT: aMtrAngle.SetValue( 45 );  break;
            case RP_LM: aMtrAngle.SetValue( 180 ); break;
            case RP_RM: aMtrAngle.SetValue( 0 );   break;
            case RP_LB: aMtrAngle.SetValue( 225 ); break;
            case RP_MB: aMtrAngle.SetValue( 270 ); break;
            case RP_RB: aMtrAngle.SetValue( 315 ); break;
            case RP_MM: break;
        }
        ModifiedHdl_Impl( this );
    }
}


void SvxHatchTabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
        aCtlPreview.SetDrawMode( GetDisplayBackground().GetColor().IsDark() ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR );

    SvxTabPage::DataChanged( rDCEvt );
}

