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

#include <cuires.hrc>
#include "helpid.hrc"
#include "svx/xattr.hxx"
#include <svx/xpool.hxx>
#include <svx/xtable.hxx>
#include "svx/drawitem.hxx"
#include "cuitabarea.hxx"
#include "defdlgname.hxx"
#include "dlgname.hxx"
#include <svx/svxdlg.hxx>
#include <dialmgr.hxx>
#include <cuitabline.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>


XPropertyListRef SvxColorTabPage::GetList()
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

void SvxColorTabPage::SetEmbed( bool bEmbed )
{
    XPropertyListRef pList = GetList();
    if( pList.is() )
        pList->SetEmbedInDocument( bEmbed );
    m_pBoxEmbed->Check( bEmbed );
}

bool SvxColorTabPage::GetEmbed()
{
    XPropertyListRef pList = GetList();
    return pList.is() ? pList->IsEmbedInDocument() : 0;
}

IMPL_LINK_NOARG(SvxColorTabPage, EmbedToggleHdl_Impl)
{
    SetEmbed( m_pBoxEmbed->IsChecked() );
    return 0;
}

void SvxColorTabPage::HideLoadSaveEmbed()
{
    m_pBtnLoad->Hide();
    m_pBtnSave->Hide();
    m_pBoxEmbed->Hide();
}

void SvxColorTabPage::UpdateTableName()
{
    // Truncate the name if necessary ...
    OUString aString( CUI_RES( RID_SVXSTR_TABLE ) );
    aString += ": ";

    XPropertyListRef pList = GetList();
    if( !pList.is() )
        return;

    INetURLObject aURL( pList->GetPath() );
    aURL.Append( pList->GetName() );

    if ( aURL.getBase().getLength() > 18 )
    {
        aString += aURL.getBase().copy( 0, 15 );
        aString += "...";
    }
    else
        aString += aURL.getBase();

    m_pTableName->SetText( aString );
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxColorTabPage, ClickLoadHdl_Impl)
{
    sal_uInt16 nReturn = RET_YES;
    bool bLoaded = false;

    if( IsModified() && GetList()->Count() > 0 )
    {
        nReturn = MessageDialog( GetParentDialog()
                                ,"AskSaveList"
                                ,"cui/ui/querysavelistdialog.ui").Execute();

        if ( nReturn == RET_YES )
            GetList()->Save();
    }

    if ( nReturn != RET_CANCEL )
    {
        ::sfx2::FileDialogHelper aDlg( css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, 0 );
        String aStrFilterType( XPropertyList::GetDefaultExtFilter( meType ) );
        aDlg.AddFilter( aStrFilterType, aStrFilterType );

        INetURLObject aFile( SvtPathOptions().GetPalettePath() );
        aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::NO_DECODE ) );

        if ( aDlg.Execute() == ERRCODE_NONE )
        {
            XColorListRef pList = XPropertyList::CreatePropertyListFromURL(
                                        meType, aDlg.GetPath() )->AsColorList();
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
                MessageDialog( mpTopDlg
                              ,"NoLoadedFileDialog"
                              ,"cui/ui/querynoloadedfiledialog.ui").Execute();
            }
        }
    }
    Update( bLoaded );

    return 0;
}

void SvxColorTabPage::EnableSave( bool bCanSave )
{
    if ( bCanSave )
        m_pBtnSave->Enable();
    else
        m_pBtnSave->Disable();
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxColorTabPage, ClickSaveHdl_Impl)
{
    ::sfx2::FileDialogHelper aDlg(
        css::ui::dialogs::TemplateDescription::FILESAVE_SIMPLE, 0 );

    String aStrFilterType( XPropertyList::GetDefaultExtFilter( meType ) );
    aDlg.AddFilter( aStrFilterType, aStrFilterType );

    INetURLObject aFile( SvtPathOptions().GetPalettePath() );
    DBG_ASSERT( aFile.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    XPropertyListRef pList = GetList();

    if( !pList->GetName().isEmpty() )
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
            MessageDialog( mpTopDlg
                          ,"NoSaveFileDialog"
                          ,"cui/ui/querynosavefiledialog.ui").Execute();
        }
    }
    return 0;
}

void SvxColorTabPage::Update(bool bLoaded)
{
    pColorList = XColorListRef( static_cast<XColorList *>( GetList().get() ) );

    if (bLoaded)
    {
        m_pLbColor->Clear();
        m_pValSetColorList->Clear();
        Construct();
        Reset( rOutAttrs );

        if( m_pLbColor->GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND )
            m_pLbColor->SelectEntryPos( 0 );
        else
            m_pLbColor->SelectEntryPos( m_pLbColor->GetSelectEntryPos() );

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

SvxColorTabPage::SvxColorTabPage(Window* pParent, const SfxItemSet& rInAttrs)
    : SfxTabPage(pParent, "ColorPage", "cui/ui/colorpage.ui", rInAttrs)
    , meType( XCOLOR_LIST )
    , mpXPool( (XOutdevItemPool*) rInAttrs.GetPool() )
    , mpTopDlg( GetParentDialog() )
    , pShadow             ( new SvxColorTabPageShadow() )
    , rOutAttrs           ( rInAttrs )
    // All the horrific pointers we store and should not
    , pnColorListState( 0 )
    , pPageType( NULL )
    , pDlgType( NULL )
    , pPos( NULL )
    , pbAreaTP( NULL )
    , aXFStyleItem( XFILL_SOLID )
    , aXFillColorItem( String(), Color( COL_BLACK ) )
    , aXFillAttr( (XOutdevItemPool*) rInAttrs.GetPool() )
    , rXFSet( aXFillAttr.GetItemSet() )
    , eCM( CM_RGB )
{
    get(m_pBoxEmbed, "embed");
    get(m_pBtnLoad, "load");
    get(m_pBtnSave, "save");
    get(m_pTableName, "colortableft");

    get(m_pEdtName, "name");
    get(m_pLbColor, "colorlb");
    get(m_pValSetColorList, "colorset");
    Size aSize = LogicToPixel(Size(94 , 117), MAP_APPFONT);
    m_pValSetColorList->set_width_request(aSize.Width());
    m_pValSetColorList->set_height_request(aSize.Height());
    get(m_pCtlPreviewOld, "oldpreview");
    get(m_pCtlPreviewNew, "newpreview");
    aSize = LogicToPixel(Size(34 , 25), MAP_APPFONT);
    m_pCtlPreviewOld->set_width_request(aSize.Width());
    m_pCtlPreviewOld->set_height_request(aSize.Height());
    m_pCtlPreviewNew->set_width_request(aSize.Width());
    m_pCtlPreviewNew->set_height_request(aSize.Height());
    get(m_pLbColorModel, "modellb");
    get(m_pRGB, "rgb");
    get(m_pR, "R");
    get(m_pG, "G");
    get(m_pB, "B");
    get(m_pCYMK, "cymk");
    get(m_pC, "C");
    get(m_pY, "Y");
    get(m_pM, "M");
    get(m_pK, "K");
    get(m_pBtnAdd, "add");
    get(m_pBtnModify, "modify");
    get(m_pBtnWorkOn, "edit");
    get(m_pBtnDelete, "delete");

    m_pBoxEmbed->SetToggleHdl( LINK( this, SvxColorTabPage, EmbedToggleHdl_Impl ) );

    m_pBtnLoad->SetClickHdl( LINK( this, SvxColorTabPage, ClickLoadHdl_Impl ) );
    m_pBtnSave->SetClickHdl( LINK( this, SvxColorTabPage, ClickSaveHdl_Impl ) );

    SetEmbed( GetEmbed() );
    UpdateTableName();

    // this page needs ExchangeSupport
    SetExchangeSupport();

    // setting the output device
    rXFSet.Put( aXFStyleItem );
    rXFSet.Put( aXFillColorItem );
    m_pCtlPreviewOld->SetAttributes( aXFillAttr.GetItemSet() );
    m_pCtlPreviewNew->SetAttributes( aXFillAttr.GetItemSet() );

    // overload handler
    m_pLbColor->SetSelectHdl(
        LINK( this, SvxColorTabPage, SelectColorLBHdl_Impl ) );
    m_pValSetColorList->SetSelectHdl(
        LINK( this, SvxColorTabPage, SelectValSetHdl_Impl ) );
    m_pLbColorModel->SetSelectHdl(
        LINK( this, SvxColorTabPage, SelectColorModelHdl_Impl ) );

    Link aLink = LINK( this, SvxColorTabPage, ModifiedHdl_Impl );
    m_pR->SetModifyHdl( aLink );
    m_pG->SetModifyHdl( aLink );
    m_pB->SetModifyHdl( aLink );
    m_pC->SetModifyHdl( aLink );
    m_pY->SetModifyHdl( aLink );
    m_pM->SetModifyHdl( aLink );
    m_pK->SetModifyHdl( aLink );

    m_pBtnAdd->SetClickHdl( LINK( this, SvxColorTabPage, ClickAddHdl_Impl ) );
    m_pBtnModify->SetClickHdl( LINK( this, SvxColorTabPage, ClickModifyHdl_Impl ) );
    m_pBtnWorkOn->SetClickHdl( LINK( this, SvxColorTabPage, ClickWorkOnHdl_Impl ) );
    m_pBtnDelete->SetClickHdl( LINK( this, SvxColorTabPage, ClickDeleteHdl_Impl ) );

    // ValueSet
    m_pValSetColorList->SetStyle( m_pValSetColorList->GetStyle() | WB_ITEMBORDER );
    m_pValSetColorList->Show();
}

SvxColorTabPage::~SvxColorTabPage()
{
    delete pShadow;
}

// -----------------------------------------------------------------------

void SvxColorTabPage::ImpColorCountChanged()
{
    if (!pColorList.is())
        return;
    m_pValSetColorList->SetColCount(m_pValSetColorList->getColumnCount());
}

// -----------------------------------------------------------------------

void SvxColorTabPage::Construct()
{
    if (pColorList.is())
    {
        m_pLbColor->Fill(pColorList);
        m_pValSetColorList->addEntriesForXColorList(*pColorList);
        ImpColorCountChanged();
        UpdateTableName();
    }
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
                m_pLbColor->SelectEntryPos( *pPos );
                m_pValSetColorList->SelectItem( m_pLbColor->GetSelectEntryPos() + 1 );
                m_pEdtName->SetText( m_pLbColor->GetSelectEntry() );

                ChangeColorHdl_Impl( this );
            }
            else if( *pPageType == PT_COLOR && *pPos == LISTBOX_ENTRY_NOTFOUND )
            {
                const SfxPoolItem* pPoolItem = NULL;
                if( SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLCOLOR ), sal_True, &pPoolItem ) )
                {
                    m_pLbColorModel->SelectEntryPos( CM_RGB );

                    aAktuellColor.SetColor ( ( ( const XFillColorItem* ) pPoolItem )->GetColorValue().GetColor() );

                    m_pEdtName->SetText( ( ( const XFillColorItem* ) pPoolItem )->GetName() );

                    m_pR->SetValue( ColorToPercent_Impl( aAktuellColor.GetRed() ) );
                    m_pG->SetValue( ColorToPercent_Impl( aAktuellColor.GetGreen() ) );
                    m_pB->SetValue( ColorToPercent_Impl( aAktuellColor.GetBlue() ) );

                    // fill ItemSet and pass it on to XOut
                    rXFSet.Put( XFillColorItem( String(), aAktuellColor ) );
                    m_pCtlPreviewOld->SetAttributes( aXFillAttr.GetItemSet() );
                    m_pCtlPreviewNew->SetAttributes( aXFillAttr.GetItemSet() );

                    m_pCtlPreviewNew->Invalidate();
                    m_pCtlPreviewOld->Invalidate();
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

    sal_uInt16 nPos = m_pLbColor->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        Color aColor = pColorList->GetColor( nPos )->GetColor();
        OUString aString = m_pLbColor->GetSelectEntry();

        // aNewColor, because COL_USER != COL_something, even if RGB values are the same
        // Color aNewColor( aColor.GetRed(), aColor.GetGreen(), aColor.GetBlue() );

        if( ColorToPercent_Impl( aTmpColor.GetRed() ) != ColorToPercent_Impl( aColor.GetRed() ) ||
            ColorToPercent_Impl( aTmpColor.GetGreen() ) != ColorToPercent_Impl( aColor.GetGreen() ) ||
            ColorToPercent_Impl( aTmpColor.GetBlue() ) != ColorToPercent_Impl( aColor.GetBlue() ) ||
            aString != m_pEdtName->GetText() )
        {
            ResMgr& rMgr = CUI_MGR();
            Image aWarningBoxImage = WarningBox::GetStandardImage();
            SvxMessDialog* aMessDlg = new SvxMessDialog(GetParentDialog(),
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
                    nPos = m_pLbColor->GetSelectEntryPos();
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
        nPos = m_pLbColor->GetSelectEntryPos();
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

        sal_uInt16 nPos = m_pLbColor->GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            aColor  = pColorList->GetColor( nPos )->GetColor();
            aString = m_pLbColor->GetSelectEntry();
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
    m_pBtnModify->Enable( bEnable );
    m_pBtnWorkOn->Enable( bEnable );
    m_pBtnDelete->Enable( bEnable );
    EnableSave( bEnable );
}

// -----------------------------------------------------------------------

void SvxColorTabPage::Reset( const SfxItemSet& rSet )
{
    sal_uInt16 nState = rSet.GetItemState( XATTR_FILLCOLOR );

    if ( nState >= SFX_ITEM_DEFAULT )
    {
        XFillColorItem aColorItem( (const XFillColorItem&)rSet.Get( XATTR_FILLCOLOR ) );
        m_pLbColor->SelectEntry( aColorItem.GetColorValue() );
        m_pValSetColorList->SelectItem( m_pLbColor->GetSelectEntryPos() + 1 );
        m_pEdtName->SetText( m_pLbColor->GetSelectEntry() );
    }

    // set color model
    String aStr = GetUserData();
    m_pLbColorModel->SelectEntryPos( (sal_uInt16) aStr.ToInt32() );

    ChangeColorHdl_Impl( this );
    SelectColorModelHdl_Impl( this );

    m_pCtlPreviewOld->Invalidate();
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
    if (eCM == CM_RGB)
    {
        // read current MtrFields, if cmyk, then k-value as transparency
        aAktuellColor.SetColor ( Color( (sal_uInt8)PercentToColor_Impl( (sal_uInt16) m_pR->GetValue() ),
                                        (sal_uInt8)PercentToColor_Impl( (sal_uInt16) m_pG->GetValue() ),
                                        (sal_uInt8)PercentToColor_Impl( (sal_uInt16) m_pB->GetValue() ) ).GetColor() );
    }
    else
    {
        // read current MtrFields, if cmyk, then k-value as transparency
        aAktuellColor.SetColor ( Color( (sal_uInt8)PercentToColor_Impl( (sal_uInt16) m_pK->GetValue() ),
                                        (sal_uInt8)PercentToColor_Impl( (sal_uInt16) m_pC->GetValue() ),
                                        (sal_uInt8)PercentToColor_Impl( (sal_uInt16) m_pY->GetValue() ),
                                        (sal_uInt8)PercentToColor_Impl( (sal_uInt16) m_pM->GetValue() ) ).GetColor() );
    }

    Color aTmpColor(aAktuellColor);

    if (eCM != CM_RGB)
        ConvertColorValues (aTmpColor, CM_RGB);

    rXFSet.Put( XFillColorItem( String(), aTmpColor ) );
    m_pCtlPreviewNew->SetAttributes( aXFillAttr.GetItemSet() );

    m_pCtlPreviewNew->Invalidate();

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
    OUString aName( m_pEdtName->GetText() );
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
        MessageDialog aWarningBox( GetParentDialog()
                                   ,"DuplicateNameDialog"
                                   ,"cui/ui/queryduplicatedialog.ui");
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

        m_pLbColor->Append( *pEntry );
        m_pValSetColorList->InsertItem( m_pValSetColorList->GetItemCount() + 1, pEntry->GetColor(), pEntry->GetName() );
        ImpColorCountChanged();

        m_pLbColor->SelectEntryPos( m_pLbColor->GetEntryCount() - 1 );

        *pnColorListState |= CT_MODIFIED;

        SelectColorLBHdl_Impl( this );
    }
    UpdateModified();

    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxColorTabPage, ClickModifyHdl_Impl)
{
    sal_uInt16 nPos = m_pLbColor->GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        ResMgr& rMgr = CUI_MGR();
        String aDesc( ResId( RID_SVXSTR_DESC_COLOR, rMgr ) );
        OUString aName( m_pEdtName->GetText() );
        long nCount = pColorList->Count();
        sal_Bool bDifferent = sal_True;

        // check if name is already existing
        for ( long i = 0; i < nCount && bDifferent; i++ )
            if ( aName == pColorList->GetColor( i )->GetName() && nPos != i )
                bDifferent = sal_False;

        // if yes, it is repeated and a new name is demanded
        if ( !bDifferent )
        {
            MessageDialog aWarningBox( GetParentDialog()
                                      ,"DuplicateNameDialog"
                                      ,"cui/ui/queryduplicatedialog.ui");
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
            Color aTmpColor (aAktuellColor);

            if (eCM != CM_RGB)
                ConvertColorValues (aTmpColor, CM_RGB);

            const XColorEntry aEntry(aTmpColor, aName);

            m_pLbColor->Modify( aEntry, nPos );
            m_pLbColor->SelectEntryPos( nPos );

            m_pValSetColorList->SetItemColor( nPos + 1, aEntry.GetColor() );
            m_pValSetColorList->SetItemText( nPos + 1, aEntry.GetName() );
            m_pEdtName->SetText( aName );

            m_pCtlPreviewOld->Invalidate();

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
        {
            ConvertColorValues (aAktuellColor, eCM);
            m_pC->SetValue( ColorToPercent_Impl( aAktuellColor.GetRed() ) );
            m_pY->SetValue( ColorToPercent_Impl( aAktuellColor.GetGreen() ) );
            m_pM->SetValue( ColorToPercent_Impl( aAktuellColor.GetBlue() ) );
            m_pK->SetValue( ColorToPercent_Impl( nK ) );
        }
        else
        {
            m_pR->SetValue( ColorToPercent_Impl( aAktuellColor.GetRed() ) );
            m_pG->SetValue( ColorToPercent_Impl( aAktuellColor.GetGreen() ) );
            m_pB->SetValue( ColorToPercent_Impl( aAktuellColor.GetBlue() ) );
        }

        // fill ItemSet and pass it on to XOut
        rXFSet.Put( XFillColorItem( String(), aPreviewColor ) );
        //m_pCtlPreviewOld->SetAttributes( aXFillAttr );
        m_pCtlPreviewNew->SetAttributes( aXFillAttr.GetItemSet() );

        m_pCtlPreviewNew->Invalidate();
    }
    delete( pColorDlg );

    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxColorTabPage, ClickDeleteHdl_Impl)
{
    sal_uInt16 nPos = m_pLbColor->GetSelectEntryPos();

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
            m_pLbColor->RemoveEntry( nPos );
            m_pValSetColorList->Clear();
            m_pValSetColorList->addEntriesForXColorList(*pColorList);
            ImpColorCountChanged();
            //FillValueSet_Impl(*m_pValSetColorList);

            // positioning
            m_pLbColor->SelectEntryPos( nPos );
            SelectColorLBHdl_Impl( this );

            m_pCtlPreviewOld->Invalidate();

            *pnColorListState |= CT_MODIFIED;
        }
    }
    UpdateModified();

    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxColorTabPage, SelectColorLBHdl_Impl)
{
    sal_uInt16 nPos = m_pLbColor->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        m_pValSetColorList->SelectItem( nPos + 1 );
        m_pEdtName->SetText( m_pLbColor->GetSelectEntry() );

        rXFSet.Put( XFillColorItem( String(),
                                    m_pLbColor->GetSelectEntryColor() ) );
        m_pCtlPreviewOld->SetAttributes( aXFillAttr.GetItemSet() );
        m_pCtlPreviewNew->SetAttributes( aXFillAttr.GetItemSet() );

        m_pCtlPreviewOld->Invalidate();
        m_pCtlPreviewNew->Invalidate();

        ChangeColorHdl_Impl( this );
    }
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxColorTabPage, SelectValSetHdl_Impl)
{
    sal_uInt16 nPos = m_pValSetColorList->GetSelectItemId();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        m_pLbColor->SelectEntryPos( nPos - 1 );
        m_pEdtName->SetText( m_pLbColor->GetSelectEntry() );

        rXFSet.Put( XFillColorItem( String(),
                                    m_pLbColor->GetSelectEntryColor() ) );
        m_pCtlPreviewOld->SetAttributes( aXFillAttr.GetItemSet() );
        m_pCtlPreviewNew->SetAttributes( aXFillAttr.GetItemSet() );

        m_pCtlPreviewOld->Invalidate();
        m_pCtlPreviewNew->Invalidate();

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
    int nPos = m_pLbColorModel->GetSelectEntryPos();
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
                m_pRGB->Show();
                m_pCYMK->Hide();

                m_pR->SetValue( ColorToPercent_Impl( aAktuellColor.GetRed() ) );
                m_pG->SetValue( ColorToPercent_Impl( aAktuellColor.GetGreen() ) );
                m_pB->SetValue( ColorToPercent_Impl( aAktuellColor.GetBlue() ) );

            }
            break;

            case CM_CMYK:
            {
                m_pCYMK->Show();
                m_pRGB->Hide();

                m_pC->SetValue( ColorToPercent_Impl( aAktuellColor.GetRed() ) );
                m_pY->SetValue( ColorToPercent_Impl( aAktuellColor.GetGreen() ) );
                m_pM->SetValue( ColorToPercent_Impl( aAktuellColor.GetBlue() ) );
                m_pK->SetValue( ColorToPercent_Impl( aAktuellColor.GetTransparency() ) );

            }
            break;
        }

        ChangeColorHdl_Impl( this );

    }

    return 0;
}

//------------------------------------------------------------------------

long SvxColorTabPage::ChangeColorHdl_Impl( void* )
{
    int nPos = m_pLbColor->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        XColorEntry* pEntry = pColorList->GetColor( nPos );

        aAktuellColor.SetColor ( pEntry->GetColor().GetColor() );
        if (eCM != CM_RGB)
        {
            ConvertColorValues (aAktuellColor, eCM);
            m_pC->SetValue( ColorToPercent_Impl( aAktuellColor.GetRed() ) );
            m_pY->SetValue( ColorToPercent_Impl( aAktuellColor.GetGreen() ) );
            m_pM->SetValue( ColorToPercent_Impl( aAktuellColor.GetBlue() ) );
            m_pK->SetValue( ColorToPercent_Impl( aAktuellColor.GetTransparency() ) );
        }
        else
        {
            m_pR->SetValue( ColorToPercent_Impl( aAktuellColor.GetRed() ) );
            m_pG->SetValue( ColorToPercent_Impl( aAktuellColor.GetGreen() ) );
            m_pB->SetValue( ColorToPercent_Impl( aAktuellColor.GetBlue() ) );
        }

        // fill ItemSet and pass it on to XOut
        rXFSet.Put( XFillColorItem( String(), pEntry->GetColor() ) );
        m_pCtlPreviewOld->SetAttributes( aXFillAttr.GetItemSet() );
        m_pCtlPreviewNew->SetAttributes( aXFillAttr.GetItemSet() );

        m_pCtlPreviewNew->Invalidate();
    }
    return 0;
}

//------------------------------------------------------------------------

//void SvxColorTabPage::FillValueSet_Impl( ValueSet& rVs )
//{
//    long nCount = pColorList->Count();
//    XColorEntry* pColorEntry;
//
//    if( nCount > 104 )
//        rVs.SetStyle( rVs.GetStyle() | WB_VSCROLL );
//
//    for( long i = 0; i < nCount; i++ )
//    {
//        pColorEntry = pColorList->GetColor( i );
//        rVs.InsertItem( (sal_uInt16) i + 1, pColorEntry->GetColor(), pColorEntry->GetName() );
//    }
//}

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

    rK = std::min( std::min( nColor1, nColor2 ), nColor3 );

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
    SetUserData( OUString::number( eCM ) );
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

