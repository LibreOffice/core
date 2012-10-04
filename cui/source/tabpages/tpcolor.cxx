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

#include <tools/shl.hxx>
#include <tools/urlobj.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <svtools/colrdlg.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/filedlghelper.hxx>
#include <svx/ofaitem.hxx>
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


// Load save embed functionality
SvxLoadSaveEmbed::SvxLoadSaveEmbed( Window *pParent, Window *pDialog,
                                    const ResId &rLoad, const ResId &rSave,
                                    const ResId &rEmbed, const ResId &rTableName,
                                    XPropertyListType t, XOutdevItemPool* pXPool )
    : meType( t )
    , mpXPool( pXPool )
    , mpTopDlg( pDialog )
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

    // URGH - abstract this nicely ... for re-using SvxLoadSaveEmbed
    if( !pList.is() ) {
        SvxColorTabPage *pPage = dynamic_cast< SvxColorTabPage *>( this );
        if( pPage )
            pList = pPage->GetColorList();
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

IMPL_LINK_NOARG(SvxLoadSaveEmbed, EmbedToggleHdl_Impl)
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
    if( !pList.is() )
        return;

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

IMPL_LINK_NOARG(SvxLoadSaveEmbed, ClickLoadHdl_Impl)
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
        String aStrFilterType( XPropertyList::GetDefaultExtFilter( meType ) );
        aDlg.AddFilter( aStrFilterType, aStrFilterType );

        INetURLObject aFile( SvtPathOptions().GetPalettePath() );
        aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::NO_DECODE ) );

        if ( aDlg.Execute() == ERRCODE_NONE )
        {
            XColorListRef pList = XPropertyList::CreatePropertyListFromURL(
                                        meType, aDlg.GetPath(), mpXPool )->AsColorList();
            if( pList->Load() )
            {
                // check whether the table may be deleted:
                SvxAreaTabDialog* pArea = dynamic_cast< SvxAreaTabDialog* >( mpTopDlg );
                SvxLineTabDialog* pLine = dynamic_cast< SvxLineTabDialog* >( mpTopDlg );

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

IMPL_LINK_NOARG(SvxLoadSaveEmbed, ClickSaveHdl_Impl)
{
    ::sfx2::FileDialogHelper aDlg(
        css::ui::dialogs::TemplateDescription::FILESAVE_SIMPLE, 0 );

    String aStrFilterType( XPropertyList::GetDefaultExtFilter( meType ) );
    aDlg.AddFilter( aStrFilterType, aStrFilterType );

    INetURLObject aFile( SvtPathOptions().GetPalettePath() );
    DBG_ASSERT( aFile.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    XPropertyListRef pList = GetList();

    if( pList->GetName().Len() )
    {
        aFile.Append( pList->GetName() );

        if( aFile.getExtension().isEmpty() )
            aFile.SetExtension( XPropertyList::GetDefaultExt( meType ) );
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

// FIXME: you have to hate yourself for this - all this
// horrible and broadly unused pointer based coupling
// needs to die. cf SetupForViewFrame
#define COLORPAGE_UNKNOWN ((sal_uInt16)0xFFFF)

struct SvxColorTabPageShadow
{
    sal_uInt16 nUnknownType;
    sal_uInt16 nUnknownPos;
    sal_Bool   bIsAreaTP;
    sal_uInt16 nChangeType;
    SvxColorTabPageShadow()
        : nUnknownType( COLORPAGE_UNKNOWN )
        , nUnknownPos( COLORPAGE_UNKNOWN )
        , bIsAreaTP( sal_False )
        , nChangeType( 0 )
    {
    }
};

// -----------------------------------------------------------------------

SvxColorTabPage::SvxColorTabPage
(
    Window* pParent,
    const SfxItemSet& rInAttrs
) :

    SfxTabPage          ( pParent, CUI_RES( RID_SVXPAGE_COLOR ), rInAttrs ),
    SvxLoadSaveEmbed    ( this, GetParentDialog(),
                          CUI_RES( BTN_LOAD ), CUI_RES( BTN_SAVE ),
                          CUI_RES( BTN_EMBED ), CUI_RES( FT_TABLE_NAME ),
                          XCOLOR_LIST, (XOutdevItemPool*) rInAttrs.GetPool() ),

    pShadow             ( new SvxColorTabPageShadow() ),
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

    // All the horrific pointers we store and should not
    pPageType           ( NULL ),
    pDlgType            ( NULL ),
    pPos                ( NULL ),
    pbAreaTP            ( NULL ),

    aXFStyleItem        ( XFILL_SOLID ),
    aXFillColorItem     ( String(), Color( COL_BLACK ) ),
    aXFillAttr          ( (XOutdevItemPool*) rInAttrs.GetPool() ),
    rXFSet              ( aXFillAttr.GetItemSet() ),
    eCM                 ( CM_RGB )
{
    FreeResource();

    // this page needs ExchangeSupport
    SetExchangeSupport();

    // setting the output device
    rXFSet.Put( aXFStyleItem );
    rXFSet.Put( aXFillColorItem );
    aCtlPreviewOld.SetAttributes( aXFillAttr.GetItemSet() );
    aCtlPreviewNew.SetAttributes( aXFillAttr.GetItemSet() );

    // overload handler
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

SvxColorTabPage::~SvxColorTabPage()
{
    delete pShadow;
}

// -----------------------------------------------------------------------

void SvxColorTabPage::Construct()
{
    aLbColor.Fill( pColorList );
    FillValueSet_Impl( aValSetColorList );
    UpdateTableName();
}

// -----------------------------------------------------------------------

void SvxColorTabPage::ActivatePage( const SfxItemSet& )
{
    if( *pDlgType == 0 ) // area dialog
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

                    // fill ItemSet and pass it on to XOut
                    rXFSet.Put( XFillColorItem( String(), aAktuellColor ) );
                    aCtlPreviewOld.SetAttributes( aXFillAttr.GetItemSet() );
                    aCtlPreviewNew.SetAttributes( aXFillAttr.GetItemSet() );

                    aCtlPreviewNew.Invalidate();
                    aCtlPreviewOld.Invalidate();
                }
            }

            // so that the possibly changed color is discarded
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
    // used to NOT lose changes

    Color aTmpColor (aAktuellColor);
    if (eCM != CM_RGB)
        ConvertColorValues (aTmpColor, CM_RGB);

    sal_uInt16 nPos = aLbColor.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        Color aColor = pColorList->GetColor( nPos )->GetColor();
        String aString = aLbColor.GetSelectEntry();

        // aNewColor, because COL_USER != COL_something, even if RGB values are the same
        // Color aNewColor( aColor.GetRed(), aColor.GetGreen(), aColor.GetBlue() );

        if( ColorToPercent_Impl( aTmpColor.GetRed() ) != ColorToPercent_Impl( aColor.GetRed() ) ||
            ColorToPercent_Impl( aTmpColor.GetGreen() ) != ColorToPercent_Impl( aColor.GetGreen() ) ||
            ColorToPercent_Impl( aTmpColor.GetBlue() ) != ColorToPercent_Impl( aColor.GetBlue() ) ||
            aString != aEdtName.GetText() )
        {
            ResMgr& rMgr = CUI_MGR();
            Image aWarningBoxImage = WarningBox::GetStandardImage();
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            AbstractSvxMessDialog* aMessDlg = pFact->CreateSvxMessDialog( GetParentDialog(), RID_SVXDLG_MESSBOX,
                                                        SVX_RESSTR( RID_SVXSTR_COLOR ),
                                                        String( ResId( RID_SVXSTR_ASK_CHANGE_COLOR, rMgr ) ),
                                                        &aWarningBoxImage );
            aMessDlg->SetButtonText( MESS_BTN_1,
                                    String( ResId( RID_SVXSTR_CHANGE, rMgr ) ) );
            aMessDlg->SetButtonText( MESS_BTN_2,
                                    String( ResId( RID_SVXSTR_ADD, rMgr ) ) );

            short nRet = aMessDlg->Execute();

            switch( nRet )
            {
                case RET_BTN_1:
                {
                    ClickModifyHdl_Impl( this );
                    aColor = pColorList->GetColor( nPos )->GetColor();
                }
                break;

                case RET_BTN_2:
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
    if( *pDlgType == 0 ) // area dialog
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

    // set color model
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
// is called when the content of the MtrFields is changed for color values
//
IMPL_LINK_NOARG(SvxColorTabPage, ModifiedHdl_Impl)
{
    // read current MtrFields, if cmyk, then k-value as transparency
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

IMPL_LINK_NOARG(SvxColorTabPage, ClickAddHdl_Impl)
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

    // check if name is already existing
    for ( long i = 0; i < nCount && bDifferent; i++ )
        if ( aName == pColorList->GetColor( i )->GetName() )
            bDifferent = sal_False;

    // if yes, it is repeated and a new name is demanded
    if ( !bDifferent )
    {
        WarningBox aWarningBox( GetParentDialog(), WinBits( WB_OK ),
            String( ResId( RID_SVXSTR_WARN_NAME_DUPLICATE, rMgr ) ) );
        aWarningBox.SetHelpId( HID_WARN_NAME_DUPLICATE );
        aWarningBox.Execute();

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        AbstractSvxNameDialog* pDlg = pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc );
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

    // if not existing the entry is entered
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

        *pnColorListState |= CT_MODIFIED;

        SelectColorLBHdl_Impl( this );
    }
    UpdateModified();

    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxColorTabPage, ClickModifyHdl_Impl)
{
    sal_uInt16 nPos = aLbColor.GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        ResMgr& rMgr = CUI_MGR();
        String aDesc( ResId( RID_SVXSTR_DESC_COLOR, rMgr ) );
        String aName( aEdtName.GetText() );
        long nCount = pColorList->Count();
        sal_Bool bDifferent = sal_True;

        // check if name is already existing
        for ( long i = 0; i < nCount && bDifferent; i++ )
            if ( aName == pColorList->GetColor( i )->GetName() && nPos != i )
                bDifferent = sal_False;

        // if yes, it is repeated and a new name is demanded
        if ( !bDifferent )
        {
            WarningBox aWarningBox( GetParentDialog(), WinBits( WB_OK ),
                String( ResId( RID_SVXSTR_WARN_NAME_DUPLICATE, rMgr ) ) );
            aWarningBox.SetHelpId( HID_WARN_NAME_DUPLICATE );
            aWarningBox.Execute();

            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            AbstractSvxNameDialog* pDlg = pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc );
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

        // if not existing the entry is entered
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

            *pnColorListState |= CT_MODIFIED;
        }
    }
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxColorTabPage, ClickWorkOnHdl_Impl)
{
    SvColorDialog* pColorDlg = new SvColorDialog( GetParentDialog() );

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

        // fill ItemSet and pass it on to XOut
        rXFSet.Put( XFillColorItem( String(), aPreviewColor ) );
        //aCtlPreviewOld.SetAttributes( aXFillAttr );
        aCtlPreviewNew.SetAttributes( aXFillAttr.GetItemSet() );

        aCtlPreviewNew.Invalidate();
    }
    delete( pColorDlg );

    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxColorTabPage, ClickDeleteHdl_Impl)
{
    sal_uInt16 nPos = aLbColor.GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        QueryBox aQueryBox( GetParentDialog(), WinBits( WB_YES_NO | WB_DEF_NO ),
            String( CUI_RES( RID_SVXSTR_ASK_DEL_COLOR ) ) );

        if( aQueryBox.Execute() == RET_YES )
        {
            XColorEntry* pEntry = pColorList->Remove( nPos );
            DBG_ASSERT( pEntry, "ColorEntry not found !" );
            delete pEntry;

            // update Listbox and ValueSet
            aLbColor.RemoveEntry( nPos );
            aValSetColorList.Clear();
            FillValueSet_Impl( aValSetColorList );

            // positioning
            aLbColor.SelectEntryPos( nPos );
            SelectColorLBHdl_Impl( this );

            aCtlPreviewOld.Invalidate();

            *pnColorListState |= CT_MODIFIED;
        }
    }
    UpdateModified();

    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxColorTabPage, SelectColorLBHdl_Impl)
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

IMPL_LINK_NOARG(SvxColorTabPage, SelectValSetHdl_Impl)
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

IMPL_LINK_NOARG(SvxColorTabPage, SelectColorModelHdl_Impl)
{
    int nPos = aLbColorModel.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        if (eCM != (ColorModel) nPos)
        {
            ConvertColorValues (aAktuellColor, (ColorModel) nPos);
        }

        eCM = (ColorModel) nPos;

        switch( eCM )
        {
            case CM_RGB:
            {
                String aStr( aLbColorModel.GetSelectEntry() );
                String aColorStr(rtl::OUString("~X"));
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

                // Because the old HelpText is still at the Control
                // a change of the HelpID alone would not work
                aMtrFldColorModel1.SetHelpText( String() );
                aMtrFldColorModel2.SetHelpText( String() );
                aMtrFldColorModel3.SetHelpText( String() );

                // handle RGB-values (0..255, not in %),
                // and adjust MetricFields respectively
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
                String aColorStr(rtl::OUString("~X"));
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

                // handle CMYK-values (0..100%)
                // and adjust MetricFields respectively
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

        // fill ItemSet and pass it on to XOut
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

// A RGB value is converted to a CMYK value - not in an ideal way as
// R is converted into C, G into M and B into Y. The K value is held in an
// extra variable. For further color models one should develop own
// classes which should contain the respective casts.

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

// reverse case to RgbToCmyk_Impl (see above)

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
    // the color model is saved in the Ini-file
    SetUserData( UniString::CreateFromInt32( eCM ) );
}

//------------------------------------------------------------------------

void SvxColorTabPage::SetupForViewFrame( SfxViewFrame *pViewFrame )
{
    const OfaRefItem<XColorList> *pPtr = NULL;
    if ( pViewFrame != NULL && pViewFrame->GetDispatcher() )
        pPtr = (const OfaRefItem<XColorList> *)pViewFrame->
            GetDispatcher()->Execute( SID_GET_COLORLIST,
                                      SFX_CALLMODE_SYNCHRON );
    pColorList = pPtr ? pPtr->GetValue() : XColorList::GetStdColorList();

    SetPageType( &pShadow->nUnknownType );
    SetDlgType( &pShadow->nUnknownType );
    SetPos( &pShadow->nUnknownPos );
    SetAreaTP( &pShadow->bIsAreaTP );
    SetColorChgd( (ChangeType*)&pShadow->nChangeType );
    Construct();
}

void SvxColorTabPage::SaveToViewFrame( SfxViewFrame *pViewFrame )
{
    if( !pColorList.is() )
        return;

    pColorList->Save();

    if( !pViewFrame )
        return;

    // notify current viewframe that it uses the same color table
    if ( !pViewFrame->GetDispatcher() )
        return;

    const OfaRefItem<XColorList> * pPtr;
    pPtr = (const OfaRefItem<XColorList>*)pViewFrame->GetDispatcher()->Execute( SID_GET_COLORLIST, SFX_CALLMODE_SYNCHRON );
    if( pPtr )
    {
        XColorListRef pReference = pPtr->GetValue();

        if( pReference.is() &&
            pReference->GetPath() == pColorList->GetPath() &&
            pReference->GetName() == pColorList->GetName() )
            SfxObjectShell::Current()->PutItem( SvxColorListItem( pColorList,
                                                                  SID_COLOR_TABLE ) );
    }
}

void SvxColorTabPage::SetPropertyList( XPropertyListType t, const XPropertyListRef &xRef )
{
    (void) t;
    OSL_ASSERT( t == XCOLOR_LIST );
    pColorList = XColorListRef( static_cast<XColorList *>(xRef.get() ) );
}

void SvxColorTabPage::SetColorList( XColorListRef pColList )
{
    SetPropertyList( XCOLOR_LIST, XPropertyListRef( ( pColList.get() ) ) );
}

XPropertyListRef SvxColorTabPage::GetPropertyList( XPropertyListType t )
{
    (void) t;
    OSL_ASSERT( t == XCOLOR_LIST );
    return XPropertyListRef( pColorList.get() );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

