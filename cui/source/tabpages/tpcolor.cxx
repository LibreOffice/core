/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <tools/shl.hxx>
#include <tools/urlobj.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <svtools/colrdlg.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/filedlghelper.hxx>
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"

#define _SVX_TPCOLOR_CXX

#include <cuires.hrc>
#include "helpid.hrc"
#include "svx/xattr.hxx"
#include <svx/xpool.hxx>
#include <svx/xtable.hxx>
#include "svx/drawitem.hxx"
#include "cuitabarea.hxx"
#include "tabarea.hrc"
#include "defdlgname.hxx"
#include <svx/svxdlg.hxx>
#include <dialmgr.hxx>
#include <cuitabline.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>

namespace css = ::com::sun::star;

#define DLGWIN GetParentDialog( this )

static Window* GetParentDialog( Window* pWindow )
{
    while( pWindow )
    {
        if( pWindow->IsDialog() )
            break;

        pWindow = pWindow->GetParent();
    }

    return pWindow;
}

// Load save embed functionality
SvxLoadSaveEmbed::SvxLoadSaveEmbed( Window *pParent, const ResId &rLoad,
                                    const ResId &rSave, const ResId &rEmbed,
                                    const ResId &rTableName,
                                    XPropertyListType t, XOutdevItemPool* pXPool )
    : meType( t )
    , mpXPool( pXPool )
    , mpTopDlg( GetParentDialog( pParent ) )
    , maBoxEmbed( pParent, rEmbed )
    , maBtnLoad( pParent, rLoad )
    , maBtnSave( pParent, rSave )
    , maTableName( pParent, rTableName )
{
    maBoxEmbed.SetToggleHdl( LINK( this, SvxLoadSaveEmbed, EmbedToggleHdl_Impl ) );

    maBtnLoad.SetClickHdl( LINK( this, SvxLoadSaveEmbed, ClickLoadHdl_Impl ) );
    maBtnSave.SetClickHdl( LINK( this, SvxLoadSaveEmbed, ClickSaveHdl_Impl ) );

    SetEmbed( GetEmbed() );
    UpdateTableName();
}

XPropertyListRef SvxLoadSaveEmbed::GetList()
{
    SvxAreaTabDialog* pArea = dynamic_cast< SvxAreaTabDialog* >( mpTopDlg );
    SvxLineTabDialog* pLine = dynamic_cast< SvxLineTabDialog* >( mpTopDlg );

    XColorListRef pList;
    if( pArea )
        pList = pArea->GetNewColorList();
    if( pLine )
        pList = pLine->GetNewColorList();

    if( !pList.is() ) {
        if( pArea )
            pList = pArea->GetColorList();
        if( pLine )
            pList = pLine->GetColorList();
    }

    return XPropertyListRef( static_cast< XPropertyList * >( pList.get() ) );
}

void SvxLoadSaveEmbed::SetEmbed( bool bEmbed )
{
    XPropertyListRef pList = GetList();
    if( pList.is() )
        pList->SetEmbedInDocument( bEmbed );
    maBoxEmbed.Check( bEmbed );
}

bool SvxLoadSaveEmbed::GetEmbed()
{
    XPropertyListRef pList = GetList();
    return pList.is() ? pList->IsEmbedInDocument() : 0;
}

IMPL_LINK( SvxLoadSaveEmbed, EmbedToggleHdl_Impl, void *, EMPTYARG )
{
    SetEmbed( maBoxEmbed.IsChecked() );
    return 0;
}

void SvxLoadSaveEmbed::HideLoadSaveEmbed()
{
    maBtnLoad.Hide();
    maBtnSave.Hide();
    maBoxEmbed.Hide();
}

void SvxLoadSaveEmbed::UpdateTableName()
{
    // Truncate the name if necessary ...
    String aString( CUI_RES( RID_SVXSTR_TABLE ) );
    aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );

    XPropertyListRef pList = GetList();

    INetURLObject aURL( pList->GetPath() );
    aURL.Append( pList->GetName() );

    if ( aURL.getBase().getLength() > 18 )
    {
        aString += String(aURL.getBase()).Copy( 0, 15 );
        aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
    }
    else
        aString += String (aURL.getBase());

    maTableName.SetText( aString );
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxLoadSaveEmbed, ClickLoadHdl_Impl, void *, EMPTYARG )
{
    ResMgr& rMgr = CUI_MGR();
    sal_uInt16 nReturn = RET_YES;
    bool bLoaded = false;

    if( IsModified() && GetList()->Count() > 0 )
    {
        nReturn = WarningBox( mpTopDlg, WinBits( WB_YES_NO_CANCEL ),
                    String( ResId( RID_SVXSTR_WARN_TABLE_OVERWRITE, rMgr ) ) ).Execute();

        if ( nReturn == RET_YES )
            GetList()->Save();
    }

    if ( nReturn != RET_CANCEL )
    {
        ::sfx2::FileDialogHelper aDlg(
            css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
            0 );
        String aStrFilterType( RTL_CONSTASCII_USTRINGPARAM( "*.soc" ) );
        aDlg.AddFilter( aStrFilterType, aStrFilterType );
        INetURLObject aFile( SvtPathOptions().GetPalettePath() );
        aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::NO_DECODE ) );

        if ( aDlg.Execute() == ERRCODE_NONE )
        {
            INetURLObject aURL( aDlg.GetPath() );
            INetURLObject aPathURL( aURL );

            aPathURL.removeSegment();
            aPathURL.removeFinalSlash();

            XColorListRef pList = XPropertyList::CreatePropertyList(
                meType, aPathURL.GetMainURL( INetURLObject::NO_DECODE ), mpXPool )->AsColorList();
            pList->SetName( aURL.getName() );
            if( pList->Load() )
            {
                // Pruefen, ob Tabelle geloescht werden darf:
                SvxAreaTabDialog* pArea = dynamic_cast< SvxAreaTabDialog* >( mpTopDlg );
                SvxLineTabDialog* pLine = dynamic_cast< SvxLineTabDialog* >( mpTopDlg );

                pList->SetName( aURL.getName() );

                // FIXME: want to have a generic set and get method by type ...
                if( pArea )
                    pArea->SetNewColorList( pList );
                else if( pLine )
                    pLine->SetNewColorList( pList );



                bLoaded = true;
                UpdateTableName();

                AddState( CT_CHANGED );
                SetModified( false );
                SetEmbed( true );
            }
            else
            {
                ErrorBox aErrorBox( mpTopDlg, WinBits( WB_OK ),
                    String( ResId( RID_SVXSTR_READ_DATA_ERROR, rMgr ) ) );
                aErrorBox.Execute();
            }
        }
    }
    Update( bLoaded );

    return 0;
}

void SvxLoadSaveEmbed::EnableSave( bool bCanSave )
{
    if ( bCanSave )
        maBtnSave.Enable();
    else
        maBtnSave.Disable();
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxLoadSaveEmbed, ClickSaveHdl_Impl, void *, EMPTYARG )
{
    ::sfx2::FileDialogHelper aDlg(
        css::ui::dialogs::TemplateDescription::FILESAVE_SIMPLE, 0 );
    String aStrFilterType( RTL_CONSTASCII_USTRINGPARAM( "*.soc" ) );
    aDlg.AddFilter( aStrFilterType, aStrFilterType );

    INetURLObject aFile( SvtPathOptions().GetPalettePath() );
    DBG_ASSERT( aFile.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    XPropertyListRef pList = GetList();

    if( pList->GetName().Len() )
    {
        aFile.Append( pList->GetName() );

        if( !aFile.getExtension().getLength() )
            aFile.SetExtension( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "soc" ) ) );
    }

    aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::NO_DECODE ) );
    if ( aDlg.Execute() == ERRCODE_NONE )
    {
        INetURLObject aURL( aDlg.GetPath() );
        INetURLObject aPathURL( aURL );

        aPathURL.removeSegment();
        aPathURL.removeFinalSlash();

        pList->SetName( aURL.getName() );
        pList->SetPath( aPathURL.GetMainURL( INetURLObject::NO_DECODE ) );

        if( pList->Save() )
        {
            UpdateTableName();
            AddState( CT_SAVED );
            SetModified( false );
        }
        else
        {
            ErrorBox aErrorBox( mpTopDlg, WinBits( WB_OK ),
                String( CUI_RES( RID_SVXSTR_WRITE_DATA_ERROR ) ) );
            aErrorBox.Execute();
        }
    }
    return 0;
}

void SvxColorTabPage::Update(bool bLoaded)
{
    pColorList = XColorListRef( static_cast<XColorList *>( GetList().get() ) );

    if (bLoaded)
    {
        aLbColor.Clear();
        aValSetColorList.Clear();
        Construct();
        Reset( rOutAttrs );

        if( aLbColor.GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND )
            aLbColor.SelectEntryPos( 0 );
        else
            aLbColor.SelectEntryPos( aLbColor.GetSelectEntryPos() );

        ChangeColorHdl_Impl( this );
        SelectColorLBHdl_Impl( this );
    }

    UpdateModified();
}

// -----------------------------------------------------------------------

SvxColorTabPage::SvxColorTabPage
(
    Window* pParent,
    const SfxItemSet& rInAttrs
) :

    SfxTabPage          ( pParent, CUI_RES( RID_SVXPAGE_COLOR ), rInAttrs ),
    SvxLoadSaveEmbed    ( this, CUI_RES( BTN_LOAD ), CUI_RES( BTN_SAVE ),
                          CUI_RES( BTN_EMBED ), CUI_RES( FT_TABLE_NAME ),
                          XCOLOR_LIST, (XOutdevItemPool*) rInAttrs.GetPool() ),

    aFlProp             ( this, CUI_RES( FL_PROP ) ),
    aFtName             ( this, CUI_RES( FT_NAME ) ),
    aEdtName            ( this, CUI_RES( EDT_NAME ) ),
    aFtColor            ( this, CUI_RES( FT_COLOR ) ),
    aLbColor            ( this, CUI_RES( LB_COLOR ) ),

    aValSetColorList   ( this, CUI_RES( CTL_COLORTABLE ) ),

    aCtlPreviewOld      ( this, CUI_RES( CTL_PREVIEW_OLD ) ),
    aCtlPreviewNew      ( this, CUI_RES( CTL_PREVIEW_NEW ) ),

    aLbColorModel       ( this, CUI_RES( LB_COLORMODEL ) ),
    aFtColorModel1      ( this, CUI_RES( FT_1 ) ),
    aMtrFldColorModel1  ( this, CUI_RES( MTR_FLD_1 ) ),
    aFtColorModel2      ( this, CUI_RES( FT_2 ) ),
    aMtrFldColorModel2  ( this, CUI_RES( MTR_FLD_2 ) ),
    aFtColorModel3      ( this, CUI_RES( FT_3 ) ),
    aMtrFldColorModel3  ( this, CUI_RES( MTR_FLD_3 ) ),
    aFtColorModel4      ( this, CUI_RES( FT_4 ) ),
    aMtrFldColorModel4  ( this, CUI_RES( MTR_FLD_4 ) ),
    aBtnAdd             ( this, CUI_RES( BTN_ADD ) ),
    aBtnModify          ( this, CUI_RES( BTN_MODIFY ) ),
    aBtnWorkOn          ( this, CUI_RES( BTN_WORK_ON ) ),
    aBtnDelete          ( this, CUI_RES( BTN_DELETE ) ),

    rOutAttrs           ( rInAttrs ),

    aXFStyleItem        ( XFILL_SOLID ),
    aXFillColorItem     ( String(), Color( COL_BLACK ) ),
    aXFillAttr          ( (XOutdevItemPool*) rInAttrs.GetPool() ),
    rXFSet              ( aXFillAttr.GetItemSet() ),

    eCM                 ( CM_RGB )
{
    FreeResource();

    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    // Setzen des Output-Devices
    rXFSet.Put( aXFStyleItem );
    rXFSet.Put( aXFillColorItem );
    aCtlPreviewOld.SetAttributes( aXFillAttr.GetItemSet() );
    aCtlPreviewNew.SetAttributes( aXFillAttr.GetItemSet() );

    // Handler ueberladen
    aLbColor.SetSelectHdl(
        LINK( this, SvxColorTabPage, SelectColorLBHdl_Impl ) );
    aValSetColorList.SetSelectHdl(
        LINK( this, SvxColorTabPage, SelectValSetHdl_Impl ) );
    aLbColorModel.SetSelectHdl(
        LINK( this, SvxColorTabPage, SelectColorModelHdl_Impl ) );

    Link aLink = LINK( this, SvxColorTabPage, ModifiedHdl_Impl );
    aMtrFldColorModel1.SetModifyHdl( aLink );
    aMtrFldColorModel2.SetModifyHdl( aLink );
    aMtrFldColorModel3.SetModifyHdl( aLink );
    aMtrFldColorModel4.SetModifyHdl( aLink );

    aBtnAdd.SetClickHdl( LINK( this, SvxColorTabPage, ClickAddHdl_Impl ) );
    aBtnModify.SetClickHdl(
        LINK( this, SvxColorTabPage, ClickModifyHdl_Impl ) );
    aBtnWorkOn.SetClickHdl(
        LINK( this, SvxColorTabPage, ClickWorkOnHdl_Impl ) );
    aBtnDelete.SetClickHdl(
        LINK( this, SvxColorTabPage, ClickDeleteHdl_Impl ) );

    // ValueSet
    aValSetColorList.SetStyle( aValSetColorList.GetStyle() | WB_VSCROLL | WB_ITEMBORDER );
    aValSetColorList.SetColCount( 8 );
    aValSetColorList.SetLineCount( 13 );
    aValSetColorList.SetExtraSpacing( 0 );
    aValSetColorList.Show();

    aLbColorModel.SetAccessibleName( String( CUI_RES(STR_CUI_COLORMODEL) ) );
    aBtnAdd.SetAccessibleRelationMemberOf( &aFlProp );
    aBtnModify.SetAccessibleRelationMemberOf( &aFlProp );
    aBtnWorkOn.SetAccessibleRelationMemberOf( &aFlProp );
    aBtnDelete.SetAccessibleRelationMemberOf( &aFlProp );
}

// -----------------------------------------------------------------------

void SvxColorTabPage::Construct()
{
    aLbColor.Fill( pColorList );
    FillValueSet_Impl( aValSetColorList );
}

// -----------------------------------------------------------------------

void SvxColorTabPage::ActivatePage( const SfxItemSet& )
{
    if( *pDlgType == 0 ) // Flaechen-Dialog
    {
        *pbAreaTP = sal_False;

        if( pColorList.is() )
        {
            if( *pPageType == PT_COLOR && *pPos != LISTBOX_ENTRY_NOTFOUND )
            {
                aLbColor.SelectEntryPos( *pPos );
                aValSetColorList.SelectItem( aLbColor.GetSelectEntryPos() + 1 );
                aEdtName.SetText( aLbColor.GetSelectEntry() );

                ChangeColorHdl_Impl( this );
            }
            else if( *pPageType == PT_COLOR && *pPos == LISTBOX_ENTRY_NOTFOUND )
            {
                const SfxPoolItem* pPoolItem = NULL;
                if( SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLCOLOR ), sal_True, &pPoolItem ) )
                {
                    aLbColorModel.SelectEntryPos( CM_RGB );

                    aAktuellColor.SetColor ( ( ( const XFillColorItem* ) pPoolItem )->GetColorValue().GetColor() );

                    aEdtName.SetText( ( ( const XFillColorItem* ) pPoolItem )->GetName() );

                    aMtrFldColorModel1.SetValue( ColorToPercent_Impl( aAktuellColor.GetRed() ) );
                    aMtrFldColorModel2.SetValue( ColorToPercent_Impl( aAktuellColor.GetGreen() ) );
                    aMtrFldColorModel3.SetValue( ColorToPercent_Impl( aAktuellColor.GetBlue() ) );

                    // ItemSet fuellen und an XOut weiterleiten
                    rXFSet.Put( XFillColorItem( String(), aAktuellColor ) );
                    aCtlPreviewOld.SetAttributes( aXFillAttr.GetItemSet() );
                    aCtlPreviewNew.SetAttributes( aXFillAttr.GetItemSet() );

                    aCtlPreviewNew.Invalidate();
                    aCtlPreviewOld.Invalidate();
                }
            }

            // Damit evtl. geaenderte Farbe verworfen wird
            SelectColorLBHdl_Impl( this );

            *pPageType = PT_COLOR;
            *pPos = LISTBOX_ENTRY_NOTFOUND;
        }
    }
    else
        HideLoadSaveEmbed();
}

// -----------------------------------------------------------------------

int SvxColorTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( CheckChanges_Impl() == -1L )
        return( KEEP_PAGE );

    if( _pSet )
        FillItemSet( *_pSet );

    return( LEAVE_PAGE );
}

// -----------------------------------------------------------------------

long SvxColorTabPage::CheckChanges_Impl()
{
    // wird hier benutzt, um Aenderungen NICHT zu verlieren

    Color aTmpColor (aAktuellColor);
    if (eCM != CM_RGB)
        ConvertColorValues (aTmpColor, CM_RGB);

    sal_uInt16 nPos = aLbColor.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        Color aColor = pColorList->GetColor( nPos )->GetColor();
        String aString = aLbColor.GetSelectEntry();

        // aNewColor, da COL_USER != COL_irgendwas, auch wenn RGB-Werte gleich
        // Color aNewColor( aColor.GetRed(), aColor.GetGreen(), aColor.GetBlue() );

        if( ColorToPercent_Impl( aTmpColor.GetRed() ) != ColorToPercent_Impl( aColor.GetRed() ) ||
            ColorToPercent_Impl( aTmpColor.GetGreen() ) != ColorToPercent_Impl( aColor.GetGreen() ) ||
            ColorToPercent_Impl( aTmpColor.GetBlue() ) != ColorToPercent_Impl( aColor.GetBlue() ) ||
            aString != aEdtName.GetText() )
        {
            ResMgr& rMgr = CUI_MGR();
            Image aWarningBoxImage = WarningBox::GetStandardImage();
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            DBG_ASSERT(pFact, "Dialogdiet fail!");
            AbstractSvxMessDialog* aMessDlg = pFact->CreateSvxMessDialog( DLGWIN, RID_SVXDLG_MESSBOX,
                                                        SVX_RESSTR( RID_SVXSTR_COLOR ),
                                                        String( ResId( RID_SVXSTR_ASK_CHANGE_COLOR, rMgr ) ),
                                                        &aWarningBoxImage );
            DBG_ASSERT(aMessDlg, "Dialogdiet fail!");
            aMessDlg->SetButtonText( MESS_BTN_1,
                                    String( ResId( RID_SVXSTR_CHANGE, rMgr ) ) );
            aMessDlg->SetButtonText( MESS_BTN_2,
                                    String( ResId( RID_SVXSTR_ADD, rMgr ) ) );

            short nRet = aMessDlg->Execute();

            switch( nRet )
            {
                case RET_BTN_1: // Aendern
                {
                    ClickModifyHdl_Impl( this );
                    aColor = pColorList->GetColor( nPos )->GetColor();
                }
                break;

                case RET_BTN_2: // Hinzufuegen
                {
                    ClickAddHdl_Impl( this );
                    nPos = aLbColor.GetSelectEntryPos();
                    aColor = pColorList->GetColor( nPos )->GetColor();
                }
                break;

                case RET_CANCEL:
                break;
            }
            delete aMessDlg;
        }
    }
    if( *pDlgType == 0 ) // Flaechen-Dialog
    {
        nPos = aLbColor.GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            *pPos = nPos;
        }
    }
    return 0;
}

// -----------------------------------------------------------------------

sal_Bool SvxColorTabPage::FillItemSet( SfxItemSet& rSet )
{
    if( ( *pDlgType != 0 ) ||
        ( *pPageType == PT_COLOR && *pbAreaTP == sal_False ) )
    {
        String aString;
        Color  aColor;

        sal_uInt16 nPos = aLbColor.GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            aColor  = pColorList->GetColor( nPos )->GetColor();
            aString = aLbColor.GetSelectEntry();
        }
        else
        {
            aColor.SetColor (aAktuellColor.GetColor());
            if (eCM != CM_RGB)
                ConvertColorValues (aColor, CM_RGB);
        }
        rSet.Put( XFillColorItem( aString, aColor ) );
        rSet.Put( XFillStyleItem( XFILL_SOLID ) );
    }

    return( sal_True );
}

void SvxColorTabPage::UpdateModified()
{
    bool bEnable = pColorList->Count();
    aBtnModify.Enable( bEnable );
    aBtnWorkOn.Enable( bEnable );
    aBtnDelete.Enable( bEnable );
    EnableSave( bEnable );
}

// -----------------------------------------------------------------------

void SvxColorTabPage::Reset( const SfxItemSet& rSet )
{
    sal_uInt16 nState = rSet.GetItemState( XATTR_FILLCOLOR );

    if ( nState >= SFX_ITEM_DEFAULT )
    {
        XFillColorItem aColorItem( (const XFillColorItem&)rSet.Get( XATTR_FILLCOLOR ) );
        aLbColor.SelectEntry( aColorItem.GetColorValue() );
        aValSetColorList.SelectItem( aLbColor.GetSelectEntryPos() + 1 );
        aEdtName.SetText( aLbColor.GetSelectEntry() );
    }

    // Farbmodell setzen
    String aStr = GetUserData();
    aLbColorModel.SelectEntryPos( (sal_uInt16) aStr.ToInt32() );

    ChangeColorHdl_Impl( this );
    SelectColorModelHdl_Impl( this );

    aCtlPreviewOld.Invalidate();
    UpdateModified();
}

// -----------------------------------------------------------------------

SfxTabPage* SvxColorTabPage::Create( Window* pWindow,
                const SfxItemSet& rOutAttrs )
{
    return( new SvxColorTabPage( pWindow, rOutAttrs ) );
}

//------------------------------------------------------------------------

//
// Wird aufgerufen, wenn Inhalt der MtrFileds f�r Farbwerte ver�ndert wird
//
IMPL_LINK( SvxColorTabPage, ModifiedHdl_Impl, void *, EMPTYARG )
{
    // lese aktuelle MtrFields aus, wenn cmyk, dann k-Wert als Trans.-Farbe
    aAktuellColor.SetColor ( Color( (sal_uInt8)PercentToColor_Impl( (sal_uInt16) aMtrFldColorModel4.GetValue() ),
                                    (sal_uInt8)PercentToColor_Impl( (sal_uInt16) aMtrFldColorModel1.GetValue() ),
                                    (sal_uInt8)PercentToColor_Impl( (sal_uInt16) aMtrFldColorModel2.GetValue() ),
                                    (sal_uInt8)PercentToColor_Impl( (sal_uInt16) aMtrFldColorModel3.GetValue() ) ).GetColor() );

    Color aTmpColor(aAktuellColor);
    if (eCM != CM_RGB)
        ConvertColorValues (aTmpColor, CM_RGB);

    rXFSet.Put( XFillColorItem( String(), aTmpColor ) );
    aCtlPreviewNew.SetAttributes( aXFillAttr.GetItemSet() );

    aCtlPreviewNew.Invalidate();

    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxColorTabPage, ClickAddHdl_Impl, void *, EMPTYARG )
{
    Window *pWindow = this;
    while( pWindow )
    {
        pWindow = pWindow->GetParent();
    }

    ResMgr& rMgr = CUI_MGR();
    String aDesc( ResId( RID_SVXSTR_DESC_COLOR, rMgr ) );
    String aName( aEdtName.GetText() );
    XColorEntry* pEntry;
    long nCount = pColorList->Count();
    sal_Bool bDifferent = sal_True;

    // Pruefen, ob Name schon vorhanden ist
    for ( long i = 0; i < nCount && bDifferent; i++ )
        if ( aName == pColorList->GetColor( i )->GetName() )
            bDifferent = sal_False;

    // Wenn ja, wird wiederholt ein neuer Name angefordert
    if ( !bDifferent )
    {
        WarningBox aWarningBox( DLGWIN, WinBits( WB_OK ),
            String( ResId( RID_SVXSTR_WARN_NAME_DUPLICATE, rMgr ) ) );
        aWarningBox.SetHelpId( HID_WARN_NAME_DUPLICATE );
        aWarningBox.Execute();

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "Dialogdiet fail!");
        AbstractSvxNameDialog* pDlg = pFact->CreateSvxNameDialog( DLGWIN, aName, aDesc );
        DBG_ASSERT(pDlg, "Dialogdiet fail!");
        sal_Bool bLoop = sal_True;

        while ( !bDifferent && bLoop && pDlg->Execute() == RET_OK )
        {
            pDlg->GetName( aName );
            bDifferent = sal_True;

            for( long i = 0; i < nCount && bDifferent; i++ )
            {
                if( aName == pColorList->GetColor( i )->GetName() )
                    bDifferent = sal_False;
            }

            if( bDifferent )
                bLoop = sal_False;
            else
                aWarningBox.Execute();
        }
        delete( pDlg );
    }

    // Wenn nicht vorhanden, wird Eintrag aufgenommen
    if( bDifferent )
    {
        if (eCM != CM_RGB)
            ConvertColorValues (aAktuellColor, CM_RGB);
        pEntry = new XColorEntry( aAktuellColor, aName );

        pColorList->Insert( pEntry, pColorList->Count() );

        aLbColor.Append( pEntry );
        aValSetColorList.InsertItem( aValSetColorList.GetItemCount() + 1,
                pEntry->GetColor(), pEntry->GetName() );

        aLbColor.SelectEntryPos( aLbColor.GetEntryCount() - 1 );

        // Flag fuer modifiziert setzen
        *pnColorListState |= CT_MODIFIED;

        SelectColorLBHdl_Impl( this );
    }
    UpdateModified();

    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxColorTabPage, ClickModifyHdl_Impl, void *, EMPTYARG )
{
    sal_uInt16 nPos = aLbColor.GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        ResMgr& rMgr = CUI_MGR();
        String aDesc( ResId( RID_SVXSTR_DESC_COLOR, rMgr ) );
        String aName( aEdtName.GetText() );
        long nCount = pColorList->Count();
        sal_Bool bDifferent = sal_True;

        // Pruefen, ob Name schon vorhanden ist
        for ( long i = 0; i < nCount && bDifferent; i++ )
            if ( aName == pColorList->GetColor( i )->GetName() && nPos != i )
                bDifferent = sal_False;

        // Wenn ja, wird wiederholt ein neuer Name angefordert
        if ( !bDifferent )
        {
            WarningBox aWarningBox( DLGWIN, WinBits( WB_OK ),
                String( ResId( RID_SVXSTR_WARN_NAME_DUPLICATE, rMgr ) ) );
            aWarningBox.SetHelpId( HID_WARN_NAME_DUPLICATE );
            aWarningBox.Execute();

            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            DBG_ASSERT(pFact, "Dialogdiet fail!");
            AbstractSvxNameDialog* pDlg = pFact->CreateSvxNameDialog( DLGWIN, aName, aDesc );
            DBG_ASSERT(pDlg, "Dialogdiet fail!");
            sal_Bool bLoop = sal_True;

            while ( !bDifferent && bLoop && pDlg->Execute() == RET_OK )
            {
                pDlg->GetName( aName );
                bDifferent = sal_True;

                for ( long i = 0; i < nCount && bDifferent; i++ )
                    if( aName == pColorList->GetColor( i )->GetName() && nPos != i )
                        bDifferent = sal_False;

                if( bDifferent )
                    bLoop = sal_False;
                else
                    aWarningBox.Execute();
            }
            delete( pDlg );
        }

        // Wenn nicht vorhanden, wird Eintrag aufgenommen
        if( bDifferent )
        {
            XColorEntry* pEntry = pColorList->GetColor( nPos );

            Color aTmpColor (aAktuellColor);
            if (eCM != CM_RGB)
                ConvertColorValues (aTmpColor, CM_RGB);

            pEntry->SetColor( aTmpColor );
            pEntry->SetName( aName );

            aLbColor.Modify( pEntry, nPos );
            aLbColor.SelectEntryPos( nPos );
            /////
            aValSetColorList.SetItemColor( nPos + 1, pEntry->GetColor() );
            aValSetColorList.SetItemText( nPos + 1, pEntry->GetName() );
            aEdtName.SetText( aName );

            aCtlPreviewOld.Invalidate();

            // Flag fuer modifiziert setzen
            *pnColorListState |= CT_MODIFIED;
        }
    }
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxColorTabPage, ClickWorkOnHdl_Impl, void *, EMPTYARG )
{
    SvColorDialog* pColorDlg = new SvColorDialog( DLGWIN );

    Color aTmpColor (aAktuellColor);
    if (eCM != CM_RGB)
        ConvertColorValues (aTmpColor, CM_RGB);

    pColorDlg->SetColor (aTmpColor);
    pColorDlg->SetMode( svtools::ColorPickerMode_MODIFY );

    if( pColorDlg->Execute() == RET_OK )
    {
        sal_uInt16 nK = 0;
        Color aPreviewColor = pColorDlg->GetColor();
        aAktuellColor = aPreviewColor;
        if (eCM != CM_RGB)
            ConvertColorValues (aAktuellColor, eCM);

        aMtrFldColorModel1.SetValue( ColorToPercent_Impl( aAktuellColor.GetRed() ) );
        aMtrFldColorModel2.SetValue( ColorToPercent_Impl( aAktuellColor.GetGreen() ) );
        aMtrFldColorModel3.SetValue( ColorToPercent_Impl( aAktuellColor.GetBlue() ) );
        aMtrFldColorModel4.SetValue( ColorToPercent_Impl( nK ) );

        // ItemSet fuellen und an XOut weiterleiten
        rXFSet.Put( XFillColorItem( String(), aPreviewColor ) );
        //aCtlPreviewOld.SetAttributes( aXFillAttr );
        aCtlPreviewNew.SetAttributes( aXFillAttr.GetItemSet() );

        aCtlPreviewNew.Invalidate();
    }
    delete( pColorDlg );

    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxColorTabPage, ClickDeleteHdl_Impl, void *, EMPTYARG )
{
    sal_uInt16 nPos = aLbColor.GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        QueryBox aQueryBox( DLGWIN, WinBits( WB_YES_NO | WB_DEF_NO ),
            String( CUI_RES( RID_SVXSTR_ASK_DEL_COLOR ) ) );

        if( aQueryBox.Execute() == RET_YES )
        {
            XColorEntry* pEntry = pColorList->Remove( nPos );
            DBG_ASSERT( pEntry, "ColorEntry not found !" );
            delete pEntry;

            // Listbox und ValueSet aktualisieren
            aLbColor.RemoveEntry( nPos );
            aValSetColorList.Clear();
            FillValueSet_Impl( aValSetColorList );

            // Positionieren
            aLbColor.SelectEntryPos( nPos );
            SelectColorLBHdl_Impl( this );

            aCtlPreviewOld.Invalidate();

            // Flag fuer modifiziert setzen
            *pnColorListState |= CT_MODIFIED;
        }
    }
    UpdateModified();

    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxColorTabPage, SelectColorLBHdl_Impl, void *, EMPTYARG )
{
    sal_uInt16 nPos = aLbColor.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        aValSetColorList.SelectItem( nPos + 1 );
        aEdtName.SetText( aLbColor.GetSelectEntry() );

        rXFSet.Put( XFillColorItem( String(),
                                    aLbColor.GetSelectEntryColor() ) );
        aCtlPreviewOld.SetAttributes( aXFillAttr.GetItemSet() );
        aCtlPreviewNew.SetAttributes( aXFillAttr.GetItemSet() );

        aCtlPreviewOld.Invalidate();
        aCtlPreviewNew.Invalidate();

        ChangeColorHdl_Impl( this );
    }
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxColorTabPage, SelectValSetHdl_Impl, void *, EMPTYARG )
{
    sal_uInt16 nPos = aValSetColorList.GetSelectItemId();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        aLbColor.SelectEntryPos( nPos - 1 );
        aEdtName.SetText( aLbColor.GetSelectEntry() );

        rXFSet.Put( XFillColorItem( String(),
                                    aLbColor.GetSelectEntryColor() ) );
        aCtlPreviewOld.SetAttributes( aXFillAttr.GetItemSet() );
        aCtlPreviewNew.SetAttributes( aXFillAttr.GetItemSet() );

        aCtlPreviewOld.Invalidate();
        aCtlPreviewNew.Invalidate();

        ChangeColorHdl_Impl( this );
    }
    return 0;
}

//------------------------------------------------------------------------

//
// Farbwerte je nach �bergebenes Farbmodell umrechnen
//
void SvxColorTabPage::ConvertColorValues (Color& rColor, ColorModel eModell)
{
    switch (eModell)
    {
        case CM_RGB:
        {
            CmykToRgb_Impl (rColor, (sal_uInt16)rColor.GetTransparency() );
            rColor.SetTransparency ((sal_uInt8) 0);
        }
        break;

        case CM_CMYK:
        {
            sal_uInt16 nK;
            RgbToCmyk_Impl (rColor, nK );
            rColor.SetTransparency ((sal_uInt8) nK);
        }
        break;
    }
}

//
// Auswahl Listbox 'Farbmodell' (RGB/CMY)
//
IMPL_LINK( SvxColorTabPage, SelectColorModelHdl_Impl, void *, EMPTYARG )
{
    int nPos = aLbColorModel.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        if (eCM != (ColorModel) nPos)
        {
            // wenn Farbmodell geaendert wurde, dann Werte umrechnen
            ConvertColorValues (aAktuellColor, (ColorModel) nPos);
        }

        eCM = (ColorModel) nPos;

        switch( eCM )
        {
            case CM_RGB:
            {
                String aStr( aLbColorModel.GetSelectEntry() );
                String aColorStr( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "~X" ) ) );
                xub_StrLen nIdx = 1;
                aColorStr.SetChar( nIdx, aStr.GetChar( 0 ) );
                aFtColorModel1.SetText( aColorStr );
                aColorStr.SetChar( nIdx, aStr.GetChar( 1 ) );
                aFtColorModel2.SetText( aColorStr );
                aColorStr.SetChar( nIdx, aStr.GetChar( 2 ) );
                aFtColorModel3.SetText( aColorStr );

                aFtColorModel4.Hide();
                aMtrFldColorModel4.Hide();
                aMtrFldColorModel4.SetValue( 0L );

                aMtrFldColorModel1.SetHelpId( HID_TPCOLOR_RGB_1 );
                aMtrFldColorModel2.SetHelpId( HID_TPCOLOR_RGB_2 );
                aMtrFldColorModel3.SetHelpId( HID_TPCOLOR_RGB_3 );

                // Da der alte HelpText noch am Control steht wuerde
                // ein Umsetzen der HelpID alleine nichts bewirken
                aMtrFldColorModel1.SetHelpText( String() );
                aMtrFldColorModel2.SetHelpText( String() );
                aMtrFldColorModel3.SetHelpText( String() );

                // RGB-Werte im Bereich 0..255 verarbeiten (nicht in %),
                // dazu MetricField's entsprechend einstellen
                aMtrFldColorModel1.SetUnit(FUNIT_NONE);
                aMtrFldColorModel1.SetMin(0);
                aMtrFldColorModel1.SetMax(255);
                aMtrFldColorModel1.SetLast(255);

                aMtrFldColorModel2.SetUnit(FUNIT_NONE);
                aMtrFldColorModel2.SetMin(0);
                aMtrFldColorModel2.SetMax(255);
                aMtrFldColorModel2.SetLast(255);

                aMtrFldColorModel3.SetUnit(FUNIT_NONE);
                aMtrFldColorModel3.SetMin(0);
                aMtrFldColorModel3.SetMax(255);
                aMtrFldColorModel3.SetLast(255);
            }
            break;

            case CM_CMYK:
            {
                String aStr( aLbColorModel.GetSelectEntry() );
                String aColorStr( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "~X" ) ) );
                xub_StrLen nIdx = 1;
                aColorStr.SetChar( nIdx, aStr.GetChar( 0 ) );
                aFtColorModel1.SetText( aColorStr );
                aColorStr.SetChar( nIdx, aStr.GetChar( 1 ) );
                aFtColorModel2.SetText( aColorStr );
                aColorStr.SetChar( nIdx,aStr.GetChar( 2 ) );
                aFtColorModel3.SetText( aColorStr );
                aColorStr.SetChar( nIdx,aStr.GetChar( 3 ) );
                aFtColorModel4.SetText( aColorStr );

                aFtColorModel4.Show();
                aMtrFldColorModel4.Show();

                aMtrFldColorModel1.SetHelpId( HID_TPCOLOR_CMYK_1 );
                aMtrFldColorModel2.SetHelpId( HID_TPCOLOR_CMYK_2 );
                aMtrFldColorModel3.SetHelpId( HID_TPCOLOR_CMYK_3 );

                // s.o.
                aMtrFldColorModel1.SetHelpText( String() );
                aMtrFldColorModel2.SetHelpText( String() );
                aMtrFldColorModel3.SetHelpText( String() );

                // CMYK-Werte im Bereich 0..100% verarbeiten,
                // dazu MetricField's entsprechend einstellen
                String aStrUnit( RTL_CONSTASCII_USTRINGPARAM( " %" ) );

                aMtrFldColorModel1.SetUnit(FUNIT_CUSTOM);
                aMtrFldColorModel1.SetCustomUnitText( aStrUnit );
                aMtrFldColorModel1.SetMin(0);
                aMtrFldColorModel1.SetMax(100);
                aMtrFldColorModel1.SetLast(100);

                aMtrFldColorModel2.SetUnit(FUNIT_CUSTOM);
                aMtrFldColorModel2.SetCustomUnitText( aStrUnit );
                aMtrFldColorModel2.SetMin(0);
                aMtrFldColorModel2.SetMax(100);
                aMtrFldColorModel2.SetLast(100);

                aMtrFldColorModel3.SetUnit(FUNIT_CUSTOM);
                aMtrFldColorModel3.SetCustomUnitText( aStrUnit );
                aMtrFldColorModel3.SetMin(0);
                aMtrFldColorModel3.SetMax(100);
                aMtrFldColorModel3.SetLast(100);
            }
            break;
        }

        aMtrFldColorModel1.SetValue( ColorToPercent_Impl( aAktuellColor.GetRed() ) );
        aMtrFldColorModel2.SetValue( ColorToPercent_Impl( aAktuellColor.GetGreen() ) );
        aMtrFldColorModel3.SetValue( ColorToPercent_Impl( aAktuellColor.GetBlue() ) );
        aMtrFldColorModel4.SetValue( ColorToPercent_Impl( aAktuellColor.GetTransparency() ) );
    }

    aMtrFldColorModel1.SetAccessibleName( GetNonMnemonicString(aFtColorModel1.GetText()) );
    aMtrFldColorModel2.SetAccessibleName( GetNonMnemonicString(aFtColorModel2.GetText()) );
    aMtrFldColorModel3.SetAccessibleName( GetNonMnemonicString(aFtColorModel3.GetText()) );
    aMtrFldColorModel4.SetAccessibleName( GetNonMnemonicString(aFtColorModel4.GetText()) );

    return 0;
}

//------------------------------------------------------------------------

long SvxColorTabPage::ChangeColorHdl_Impl( void* )
{
    int nPos = aLbColor.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        XColorEntry* pEntry = pColorList->GetColor( nPos );

        aAktuellColor.SetColor ( pEntry->GetColor().GetColor() );
        if (eCM != CM_RGB)
            ConvertColorValues (aAktuellColor, eCM);

        aMtrFldColorModel1.SetValue( ColorToPercent_Impl( aAktuellColor.GetRed() ) );
        aMtrFldColorModel2.SetValue( ColorToPercent_Impl( aAktuellColor.GetGreen() ) );
        aMtrFldColorModel3.SetValue( ColorToPercent_Impl( aAktuellColor.GetBlue() ) );
        aMtrFldColorModel4.SetValue( ColorToPercent_Impl( aAktuellColor.GetTransparency() ) );

        // ItemSet fuellen und an XOut weiterleiten
        rXFSet.Put( XFillColorItem( String(), pEntry->GetColor() ) );
        aCtlPreviewOld.SetAttributes( aXFillAttr.GetItemSet() );
        aCtlPreviewNew.SetAttributes( aXFillAttr.GetItemSet() );

        aCtlPreviewNew.Invalidate();
    }
    return 0;
}

//------------------------------------------------------------------------

void SvxColorTabPage::FillValueSet_Impl( ValueSet& rVs )
{
    long nCount = pColorList->Count();
    XColorEntry* pColorEntry;

    if( nCount > 104 )
        rVs.SetStyle( rVs.GetStyle() | WB_VSCROLL );

    for( long i = 0; i < nCount; i++ )
    {
        pColorEntry = pColorList->GetColor( i );
        rVs.InsertItem( (sal_uInt16) i + 1, pColorEntry->GetColor(), pColorEntry->GetName() );
    }
}

//------------------------------------------------------------------------

// Ein RGB-Wert wird in einen CMYK-Wert konvertiert, wobei die Color-
// Klasse vergewaltigt wird, da R in C, G in M und B in Y umgewandelt
// wird. Der Wert K wird in einer Extra-Variablen gehalten.
// Bei weiteren Farbmodellen sollte man hierfuer eigene Klassen entwickeln,
// die dann auch entsprechende Casts enthalten.

void SvxColorTabPage::RgbToCmyk_Impl( Color& rColor, sal_uInt16& rK )
{
    sal_uInt16 const nColor1 = 255 - rColor.GetRed();
    sal_uInt16 const nColor2 = 255 - rColor.GetGreen();
    sal_uInt16 const nColor3 = 255 - rColor.GetBlue();

    rK = Min( Min( nColor1, nColor2 ), nColor3 );

    rColor.SetRed( sal::static_int_cast< sal_uInt8 >( nColor1 - rK ) );
    rColor.SetGreen( sal::static_int_cast< sal_uInt8 >( nColor2 - rK ) );
    rColor.SetBlue( sal::static_int_cast< sal_uInt8 >( nColor3 - rK ) );
}

//------------------------------------------------------------------------

// Umgekehrter Fall zu RgbToCmyk_Impl (s.o.)

void SvxColorTabPage::CmykToRgb_Impl( Color& rColor, const sal_uInt16 nK )
{
    long lTemp;

    lTemp = 255 - ( rColor.GetRed() + nK );

    if( lTemp < 0L )
        lTemp = 0L;
    rColor.SetRed( (sal_uInt8)lTemp );

    lTemp = 255 - ( rColor.GetGreen() + nK );

    if( lTemp < 0L )
        lTemp = 0L;
    rColor.SetGreen( (sal_uInt8)lTemp );

    lTemp = 255 - ( rColor.GetBlue() + nK );

    if( lTemp < 0L )
        lTemp = 0L;
    rColor.SetBlue( (sal_uInt8)lTemp );
}

//------------------------------------------------------------------------

sal_uInt16 SvxColorTabPage::ColorToPercent_Impl( sal_uInt16 nColor )
{
    sal_uInt16 nWert = 0;

    switch (eCM)
    {
        case CM_RGB :
            nWert = nColor;
            break;

        case CM_CMYK:
            nWert = (sal_uInt16) ( (double) nColor * 100.0 / 255.0 + 0.5 );
            break;
    }

    return ( nWert );
}

//------------------------------------------------------------------------

sal_uInt16 SvxColorTabPage::PercentToColor_Impl( sal_uInt16 nPercent )
{
    sal_uInt16 nWert = 0;

    switch (eCM)
    {
        case CM_RGB :
            nWert = nPercent;
            break;

        case CM_CMYK:
            nWert = (sal_uInt16) ( (double) nPercent * 255.0 / 100.0 + 0.5 );
            break;
    }

    return ( nWert );
}

//------------------------------------------------------------------------

void SvxColorTabPage::FillUserData()
{
    // Das Farbmodell wird in der Ini-Datei festgehalten
    SetUserData( UniString::CreateFromInt32( eCM ) );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
