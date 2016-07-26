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

using namespace com::sun::star;

XPropertyListRef SvxColorTabPage::GetList()
{
    SvxAreaTabDialog* pArea = dynamic_cast< SvxAreaTabDialog* >( mpTopDlg.get() );
    SvxLineTabDialog* pLine = dynamic_cast< SvxLineTabDialog* >( mpTopDlg.get() );

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
        pList = GetColorList();
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
    return pList.is() && pList->IsEmbedInDocument();
}

IMPL_LINK_NOARG_TYPED(SvxColorTabPage, EmbedToggleHdl_Impl, CheckBox&, void)
{
    SetEmbed( m_pBoxEmbed->IsChecked() );
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


IMPL_LINK_NOARG_TYPED(SvxColorTabPage, ClickLoadHdl_Impl, Button*, void)
{
    sal_uInt16 nReturn = RET_YES;
    bool bLoaded = false;

    if( IsModified() && GetList()->Count() > 0 )
    {
        nReturn = ScopedVclPtrInstance<MessageDialog>(GetParentDialog()
                                ,"AskSaveList"
                                ,"cui/ui/querysavelistdialog.ui")->Execute();

        if ( nReturn == RET_YES )
            GetList()->Save();
    }

    if ( nReturn != RET_CANCEL )
    {
        ::sfx2::FileDialogHelper aDlg( css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE );
        OUString aStrFilterType( XPropertyList::GetDefaultExtFilter( meType ) );
        aDlg.AddFilter( aStrFilterType, aStrFilterType );

        OUString aPalettePath(SvtPathOptions().GetPalettePath());
        OUString aLastDir;
        sal_Int32 nIndex = 0;
        do
        {
            aLastDir = aPalettePath.getToken(0, ';', nIndex);
        }
        while (nIndex >= 0);

        INetURLObject aFile(aLastDir);
        aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::NO_DECODE ) );

        if ( aDlg.Execute() == ERRCODE_NONE )
        {
            XColorListRef pList = XPropertyList::AsColorList(
                XPropertyList::CreatePropertyListFromURL(
                    meType, aDlg.GetPath()));
            if( pList->Load() )
            {
                // check whether the table may be deleted:
                SvxAreaTabDialog* pArea = dynamic_cast< SvxAreaTabDialog* >( mpTopDlg.get() );
                SvxLineTabDialog* pLine = dynamic_cast< SvxLineTabDialog* >( mpTopDlg.get() );

                // FIXME: want to have a generic set and get method by type ...
                if( pArea )
                    pArea->SetNewColorList(pList);
                else if( pLine )
                    pLine->SetNewColorList(pList);
                else
                    SetColorList(pList);

                bLoaded = true;
                UpdateTableName();

                AddState( ChangeType::CHANGED );
                SetModified( false );
                SetEmbed( true );
            }
            else
            {
                ScopedVclPtrInstance<MessageDialog>(mpTopDlg
                              ,"NoLoadedFileDialog"
                              ,"cui/ui/querynoloadedfiledialog.ui")->Execute();
            }
        }
    }
    Update( bLoaded );
}

void SvxColorTabPage::EnableSave( bool bCanSave )
{
    if ( bCanSave )
        m_pBtnSave->Enable();
    else
        m_pBtnSave->Disable();
}


IMPL_LINK_NOARG_TYPED(SvxColorTabPage, ClickSaveHdl_Impl, Button*, void)
{
    ::sfx2::FileDialogHelper aDlg(
        css::ui::dialogs::TemplateDescription::FILESAVE_SIMPLE );

    OUString aStrFilterType( XPropertyList::GetDefaultExtFilter( meType ) );
    aDlg.AddFilter( aStrFilterType, aStrFilterType );

    OUString aPalettePath(SvtPathOptions().GetPalettePath());
    OUString aLastDir;
    sal_Int32 nIndex = 0;
    do
    {
        aLastDir = aPalettePath.getToken(0, ';', nIndex);
    }
    while (nIndex >= 0);

    INetURLObject aFile(aLastDir);
    SAL_WARN_IF( aFile.GetProtocol() == INetProtocol::NotValid, "cui.tabpages", "invalid URL" );

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
            AddState( ChangeType::SAVED );
            SetModified( false );
        }
        else
        {
            ScopedVclPtrInstance<MessageDialog>(mpTopDlg
                          ,"NoSaveFileDialog"
                          ,"cui/ui/querynosavefiledialog.ui")->Execute();
        }
    }
}

void SvxColorTabPage::Update(bool bLoaded)
{
    pColorList = XColorListRef( static_cast<XColorList *>( GetList().get() ) );

    if (bLoaded)
    {
        m_pLbColor->Clear();
        m_pValSetColorList->Clear();
        Construct();
        Reset( &rOutAttrs );

        if( m_pLbColor->GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND )
            m_pLbColor->SelectEntryPos( 0 );
        else
            m_pLbColor->SelectEntryPos( m_pLbColor->GetSelectEntryPos() );

        sal_Int32 nPos = m_pLbColor->GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            XColorEntry* pEntry = pColorList->GetColor( nPos );
            ChangeColor(pEntry->GetColor());
        }
        SelectColorLBHdl_Impl( *m_pLbColor );
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
    sal_Int32  nUnknownPos;
    bool   bIsAreaTP;
    ChangeType nChangeType;
    SvxColorTabPageShadow()
        : nUnknownType( COLORPAGE_UNKNOWN )
        , nUnknownPos( LISTBOX_ENTRY_NOTFOUND )
        , bIsAreaTP( false )
        , nChangeType( ChangeType::NONE )
    {
    }
};


SvxColorTabPage::SvxColorTabPage(vcl::Window* pParent, const SfxItemSet& rInAttrs)
    : SfxTabPage(pParent, "ColorPage", "cui/ui/colorpage.ui", &rInAttrs)
    , mpTopDlg( GetParentDialog() )
    , pShadow             ( new SvxColorTabPageShadow() )
    , rOutAttrs           ( rInAttrs )
    // All the horrific pointers we store and should not
    , pnColorListState( nullptr )
    , pPageType( nullptr )
    , nDlgType( 0 )
    , pPos( nullptr )
    , pbAreaTP( nullptr )
    , aXFStyleItem( drawing::FillStyle_SOLID )
    , aXFillColorItem( OUString(), Color( COL_BLACK ) )
    , aXFillAttr( static_cast<XOutdevItemPool*>( rInAttrs.GetPool() ))
    , rXFSet( aXFillAttr.GetItemSet() )
    , eCM( CM_RGB )
{
    get(m_pBoxEmbed, "embed");
    get(m_pBtnLoad, "load");
    get(m_pBtnSave, "save");
    get(m_pTableName, "colortableft");

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
    get(m_pRbRGB, "RGB");
    get(m_pRbCMYK, "CMYK");
    get(m_pRGBcustom, "rgbcustom");
    get(m_pRGBpreset, "rgbpreset");
    get(m_pRcustom, "R_custom");
    get(m_pRpreset, "R_preset-nospin");
    get(m_pGcustom, "G_custom");
    get(m_pGpreset, "G_preset-nospin");
    get(m_pBcustom, "B_custom");
    get(m_pBpreset, "B_preset-nospin");
    get(m_pHexpreset, "hex_preset");
    get(m_pHexcustom, "hex_custom");
    get(m_pCMYKcustom, "cmykcustom");
    get(m_pCMYKpreset,  "cmykpreset");
    get(m_pCcustom, "C_custom");
    get(m_pCpreset, "C_preset-nospin");
    get(m_pYcustom, "Y_custom");
    get(m_pYpreset, "Y_preset-nospin");
    get(m_pMcustom, "M_custom");
    get(m_pMpreset, "M_preset-nospin");
    get(m_pKcustom, "K_custom");
    get(m_pKpreset, "K_preset-nospin");
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

    // set handler
    m_pLbColor->SetSelectHdl(
        LINK( this, SvxColorTabPage, SelectColorLBHdl_Impl ) );
    m_pValSetColorList->SetSelectHdl(
        LINK( this, SvxColorTabPage, SelectValSetHdl_Impl ) );

    Link<Edit&,void> aLink = LINK( this, SvxColorTabPage, ModifiedHdl_Impl );
    m_pRcustom->SetModifyHdl( aLink );
    m_pGcustom->SetModifyHdl( aLink );
    m_pBcustom->SetModifyHdl( aLink );
    m_pHexpreset->SetModifyHdl( aLink );
    m_pHexcustom->SetModifyHdl( aLink );
    m_pCcustom->SetModifyHdl( aLink );
    m_pYcustom->SetModifyHdl( aLink );
    m_pMcustom->SetModifyHdl( aLink );
    m_pKcustom->SetModifyHdl( aLink );

    Link<RadioButton&,void> aLink2 = LINK( this, SvxColorTabPage, SelectColorModeHdl_Impl );
    m_pRbRGB->SetToggleHdl( aLink2 );
    m_pRbCMYK->SetToggleHdl( aLink2 );
    SetColorModel( eCM );
    ChangeColorModel();

    m_pBtnAdd->SetClickHdl( LINK( this, SvxColorTabPage, ClickAddHdl_Impl ) );
    m_pBtnModify->SetClickHdl( LINK( this, SvxColorTabPage, ClickModifyHdl_Impl ) );
    m_pBtnWorkOn->SetClickHdl( LINK( this, SvxColorTabPage, ClickWorkOnHdl_Impl ) );
    m_pBtnDelete->SetClickHdl( LINK( this, SvxColorTabPage, ClickDeleteHdl_Impl ) );

    // disable preset color values
    m_pRGBpreset->Disable();
    m_pCMYKpreset->Disable();

    // ValueSet
    m_pValSetColorList->SetStyle( m_pValSetColorList->GetStyle() | WB_ITEMBORDER );
    m_pValSetColorList->Show();
}

SvxColorTabPage::~SvxColorTabPage()
{
    disposeOnce();
}

void SvxColorTabPage::dispose()
{
    delete pShadow;
    pShadow = nullptr;
    mpTopDlg.clear();
    m_pBoxEmbed.clear();
    m_pBtnLoad.clear();
    m_pBtnSave.clear();
    m_pTableName.clear();
    m_pLbColor.clear();
    m_pValSetColorList.clear();
    m_pCtlPreviewOld.clear();
    m_pCtlPreviewNew.clear();
    m_pRbRGB.clear();
    m_pRbCMYK.clear();
    m_pRGBcustom.clear();
    m_pRGBpreset.clear();
    m_pRcustom.clear();
    m_pRpreset.clear();
    m_pGcustom.clear();
    m_pGpreset.clear();
    m_pBcustom.clear();
    m_pBpreset.clear();
    m_pHexpreset.clear();
    m_pHexcustom.clear();
    m_pCMYKcustom.clear();
    m_pCMYKpreset.clear();
    m_pCcustom.clear();
    m_pCpreset.clear();
    m_pYcustom.clear();
    m_pYpreset.clear();
    m_pMcustom.clear();
    m_pMpreset.clear();
    m_pKcustom.clear();
    m_pKpreset.clear();
    m_pBtnAdd.clear();
    m_pBtnModify.clear();
    m_pBtnWorkOn.clear();
    m_pBtnDelete.clear();
    SfxTabPage::dispose();
}

void SvxColorTabPage::ImpColorCountChanged()
{
    if (!pColorList.is())
        return;
    m_pValSetColorList->SetColCount(SvxColorValueSet::getColumnCount());
}


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

void SvxColorTabPage::ActivatePage( const SfxItemSet& )
{
    if( nDlgType == 0 ) // area dialog
    {
        *pbAreaTP = false;

        if( pColorList.is() )
        {
            if( *pPageType == PT_COLOR && *pPos != LISTBOX_ENTRY_NOTFOUND )
            {
                m_pLbColor->SelectEntryPos( *pPos );
                m_pValSetColorList->SelectItem( m_pLbColor->GetSelectEntryPos() + 1 );
                XColorEntry* pEntry = pColorList->GetColor( *pPos );
                aPreviousColor = pEntry->GetColor();
                ChangeColor(pEntry->GetColor());
            }
            else if( *pPageType == PT_COLOR && *pPos == LISTBOX_ENTRY_NOTFOUND )
            {
                const SfxPoolItem* pPoolItem = nullptr;
                if( SfxItemState::SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLCOLOR ), true, &pPoolItem ) )
                {
                    SetColorModel( CM_RGB );
                    ChangeColorModel();

                    aPreviousColor = static_cast<const XFillColorItem*>(pPoolItem)->GetColorValue();
                    ChangeColor( aPreviousColor );


                    m_pRcustom->SetValue( ColorToPercent_Impl( aCurrentColor.GetRed() ) );
                    m_pGcustom->SetValue( ColorToPercent_Impl( aCurrentColor.GetGreen() ) );
                    m_pBcustom->SetValue( ColorToPercent_Impl( aCurrentColor.GetBlue() ) );
                    m_pHexcustom->SetColor( aCurrentColor.GetColor() );

                }
            }

            m_pCtlPreviewOld->SetAttributes( aXFillAttr.GetItemSet() );
            m_pCtlPreviewOld->Invalidate();

            SelectColorLBHdl_Impl( *m_pLbColor );

            *pPageType = PT_COLOR;
            *pPos = LISTBOX_ENTRY_NOTFOUND;
        }
    }
    else
        m_pBoxEmbed->Hide();
}

DeactivateRC SvxColorTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( CheckChanges_Impl() == -1L )
        return DeactivateRC::KeepPage;

    if( _pSet )
        FillItemSet( _pSet );

    return DeactivateRC::LeavePage;
}


long SvxColorTabPage::CheckChanges_Impl()
{
    // used to NOT lose changes
    sal_Int32 nPos = m_pLbColor->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        Color aColor = pColorList->GetColor( nPos )->GetColor();

        // aNewColor, because COL_USER != COL_something, even if RGB values are the same
        // Color aNewColor( aColor.GetRed(), aColor.GetGreen(), aColor.GetBlue() );

        if( ColorToPercent_Impl( aCurrentColor.GetRed() ) != ColorToPercent_Impl( aColor.GetRed() ) ||
            ColorToPercent_Impl( aCurrentColor.GetGreen() ) != ColorToPercent_Impl( aColor.GetGreen() ) ||
            ColorToPercent_Impl( aCurrentColor.GetBlue() ) != ColorToPercent_Impl( aColor.GetBlue() ) )
        {
            ResMgr& rMgr = CUI_MGR();
            Image aWarningBoxImage = WarningBox::GetStandardImage();
            ScopedVclPtrInstance<SvxMessDialog> aMessDlg( GetParentDialog(),
                                                          SVX_RESSTR( RID_SVXSTR_COLOR ),
                                                          ResId( RID_SVXSTR_ASK_CHANGE_COLOR, rMgr ),
                                                          &aWarningBoxImage );
            aMessDlg->SetButtonText( SvxMessDialogButton::N1,
                                    ResId( RID_SVXSTR_CHANGE, rMgr ) );
            aMessDlg->SetButtonText( SvxMessDialogButton::N2,
                                    ResId( RID_SVXSTR_ADD, rMgr ) );

            short nRet = aMessDlg->Execute();

            switch( nRet )
            {
                case RET_BTN_1:
                {
                    ClickModifyHdl_Impl( nullptr );
                    aColor = pColorList->GetColor( nPos )->GetColor();
                }
                break;

                case RET_BTN_2:
                {
                    ClickAddHdl_Impl( nullptr );
                    nPos = m_pLbColor->GetSelectEntryPos();
                    aColor = pColorList->GetColor( nPos )->GetColor();
                }
                break;

                case RET_CANCEL:
                break;
            }
        }
    }
    if( nDlgType == 0 ) // area dialog
    {
        nPos = m_pLbColor->GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            *pPos = nPos;
        }
    }
    return 0;
}


bool SvxColorTabPage::FillItemSet( SfxItemSet* rSet )
{
    if( ( nDlgType != 0 ) ||
        ( *pPageType == PT_COLOR && !*pbAreaTP ) )
    {
        OUString aString;
        Color  aColor;

        sal_Int32 nPos = m_pLbColor->GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            aColor  = pColorList->GetColor( nPos )->GetColor();
            aString = m_pLbColor->GetSelectEntry();
        }
        else
        {
            aColor.SetColor (aCurrentColor.GetColor());
        }
        rSet->Put( XFillColorItem( aString, aColor ) );
        rSet->Put( XFillStyleItem( drawing::FillStyle_SOLID ) );
    }

    return true;
}

void SvxColorTabPage::UpdateModified()
{
    bool bEnable = pColorList.is() && pColorList->Count();
    m_pBtnModify->Enable( bEnable );
    m_pBtnWorkOn->Enable( bEnable );
    m_pBtnDelete->Enable( bEnable );
    EnableSave( bEnable );
}

void SvxColorTabPage::Reset( const SfxItemSet* rSet )
{
    SfxItemState nState = rSet->GetItemState( XATTR_FILLCOLOR );

    Color aNewColor;

    if ( nState >= SfxItemState::DEFAULT )
    {
        XFillColorItem aColorItem( static_cast<const XFillColorItem&>(rSet->Get( XATTR_FILLCOLOR )) );
        aPreviousColor = aColorItem.GetColorValue();
        aNewColor = aColorItem.GetColorValue();
        m_pLbColor->SelectEntry(aNewColor);
        auto pos = m_pLbColor->GetSelectEntryPos();
        if (pos != LISTBOX_ENTRY_NOTFOUND)
        {
            m_pValSetColorList->SelectItem( pos + 1 );
        }
    }

    // set color model
    OUString aStr = GetUserData();
    eCM = (ColorModel)aStr.toInt32();
    SetColorModel( eCM );
    ChangeColorModel();

    ChangeColor(aNewColor);

    UpdateModified();
}


VclPtr<SfxTabPage> SvxColorTabPage::Create( vcl::Window* pWindow,
                                            const SfxItemSet* rOutAttrs )
{
    return VclPtr<SvxColorTabPage>::Create( pWindow, *rOutAttrs );
}

// is called when the content of the MtrFields is changed for color values
IMPL_LINK_TYPED(SvxColorTabPage, ModifiedHdl_Impl, Edit&, rEdit, void)
{
    if (eCM == CM_RGB)
    {
        // read current MtrFields, if cmyk, then k-value as transparency
        if(&rEdit == m_pHexcustom)
            aCurrentColor = Color(m_pHexcustom->GetColor());
        else
        {
            aCurrentColor.SetColor ( Color( (sal_uInt8)PercentToColor_Impl( (sal_uInt16) m_pRcustom->GetValue() ),
                                            (sal_uInt8)PercentToColor_Impl( (sal_uInt16) m_pGcustom->GetValue() ),
                                            (sal_uInt8)PercentToColor_Impl( (sal_uInt16) m_pBcustom->GetValue() ) ).GetColor() );

        }
        UpdateColorValues();
    }
    else
    {
        // read current MtrFields, if cmyk, then k-value as transparency
        aCurrentColor.SetColor ( Color( (sal_uInt8)PercentToColor_Impl( (sal_uInt16) m_pKcustom->GetValue() ),
                                        (sal_uInt8)PercentToColor_Impl( (sal_uInt16) m_pCcustom->GetValue() ),
                                        (sal_uInt8)PercentToColor_Impl( (sal_uInt16) m_pYcustom->GetValue() ),
                                        (sal_uInt8)PercentToColor_Impl( (sal_uInt16) m_pMcustom->GetValue() ) ).GetColor() );
        ConvertColorValues (aCurrentColor, CM_RGB);
    }

    rXFSet.Put( XFillColorItem( OUString(), aCurrentColor ) );
    m_pCtlPreviewNew->SetAttributes( aXFillAttr.GetItemSet() );

    m_pCtlPreviewNew->Invalidate();
}


IMPL_LINK_NOARG_TYPED(SvxColorTabPage, ClickAddHdl_Impl, Button*, void)
{
    OUString aNewName( SVX_RES( RID_SVXSTR_COLOR ) );
    OUString aDesc( CUI_RES( RID_SVXSTR_DESC_COLOR ) );
    OUString aName;

    long j = 1;
    bool bValidColorName = false;
    // check if name is already existing
    while (!bValidColorName)
    {
        aName = aNewName + " " + OUString::number( j++ );
        bValidColorName = (SearchColorList(aName) == LISTBOX_ENTRY_NOTFOUND);
    }

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    std::unique_ptr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc ));
    ScopedVclPtr<MessageDialog> pWarnBox;
    sal_uInt16 nError = 1;

    while (pDlg->Execute() == RET_OK)
    {
        pDlg->GetName( aName );

        bValidColorName = (SearchColorList(aName) == LISTBOX_ENTRY_NOTFOUND);
        if (bValidColorName)
        {
            nError = 0;
            break;
        }

        if( !pWarnBox )
        {
            pWarnBox.disposeAndReset(VclPtr<MessageDialog>::Create( GetParentDialog()
                                        ,"DuplicateNameDialog"
                                        ,"cui/ui/queryduplicatedialog.ui"));
        }

        if( pWarnBox->Execute() != RET_OK )
            break;
    }

    pDlg.reset();
    pWarnBox.disposeAndClear();

    if (!nError)
    {
        XColorEntry* pEntry = new XColorEntry( aCurrentColor, aName );

        pColorList->Insert( pEntry, pColorList->Count() );

        m_pLbColor->Append( *pEntry );
        m_pValSetColorList->InsertItem( m_pValSetColorList->GetItemCount() + 1, pEntry->GetColor(), pEntry->GetName() );
        m_pValSetColorList->Resize();
        ImpColorCountChanged();

        m_pLbColor->SelectEntryPos( m_pLbColor->GetEntryCount() - 1 );

        *pnColorListState |= ChangeType::MODIFIED;

        SelectColorLBHdl_Impl( *m_pLbColor );
    }

    UpdateModified();
}


IMPL_LINK_NOARG_TYPED(SvxColorTabPage, ClickModifyHdl_Impl, Button*, void)
{
    sal_Int32 nPos = m_pLbColor->GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        ResMgr& rMgr = CUI_MGR();
        OUString aDesc( ResId( RID_SVXSTR_DESC_COLOR, rMgr ) );
        OUString aName( m_pLbColor->GetSelectEntry() );

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        std::unique_ptr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc ));
        assert(pFact && "Dialog Creation Failed");

        bool bLoop = true;

        while( bLoop && pDlg->Execute() == RET_OK )
        {
            pDlg->GetName( aName );
            sal_Int32 nColorPos = SearchColorList(aName);
            bool bValidColorName = (nColorPos == nPos) || (nColorPos == LISTBOX_ENTRY_NOTFOUND);

            if( bValidColorName )
            {
                bLoop = false;
                // #123497# Need to replace the existing entry with a new one (old returned needs to be deleted)
                XColorEntry* pEntry = new XColorEntry(aCurrentColor, aName);
                delete pColorList->Replace(pEntry, nPos);

                m_pLbColor->Modify( *pEntry, nPos );
                m_pLbColor->SelectEntryPos( nPos );

                m_pValSetColorList->Clear();
                m_pValSetColorList->addEntriesForXColorList( *pColorList );
                m_pValSetColorList->SelectItem( nPos + 1 );

                m_pCtlPreviewOld->Invalidate();

                *pnColorListState |= ChangeType::MODIFIED;
            }

            // if yes, it is repeated and a new name is demanded
            else
            {
                ScopedVclPtrInstance<MessageDialog> aWarningBox( GetParentDialog()
                                                                 ,"DuplicateNameDialog"
                                                                 ,"cui/ui/queryduplicatedialog.ui");
                aWarningBox->Execute();
            }
        }
    }
}


IMPL_LINK_NOARG_TYPED(SvxColorTabPage, ClickWorkOnHdl_Impl, Button*, void)
{
    std::unique_ptr<SvColorDialog> pColorDlg(new SvColorDialog( GetParentDialog() ));

    pColorDlg->SetColor (aCurrentColor);
    pColorDlg->SetMode( svtools::ColorPickerMode_MODIFY );

    if( pColorDlg->Execute() == RET_OK )
    {
        Color aPreviewColor = pColorDlg->GetColor();
        aCurrentColor = aPreviewColor;
        UpdateColorValues();
        // fill ItemSet and pass it on to XOut
        rXFSet.Put( XFillColorItem( OUString(), aPreviewColor ) );
        //m_pCtlPreviewOld->SetAttributes( aXFillAttr );
        m_pCtlPreviewNew->SetAttributes( aXFillAttr.GetItemSet() );

        m_pCtlPreviewNew->Invalidate();
    }
}


IMPL_LINK_NOARG_TYPED(SvxColorTabPage, ClickDeleteHdl_Impl, Button*, void)
{
    sal_Int32 nPos = m_pLbColor->GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        ScopedVclPtrInstance< MessageDialog > aQueryBox( GetParentDialog(),"AskDelColorDialog","cui/ui/querydeletecolordialog.ui");

        if( aQueryBox->Execute() == RET_YES )
        {
            XColorEntry* pEntry = pColorList->Remove( nPos );
            assert( pEntry && "ColorEntry not found !" );
            delete pEntry;

            // update Listbox and ValueSet
            m_pLbColor->RemoveEntry( nPos );
            m_pValSetColorList->Clear();
            m_pValSetColorList->addEntriesForXColorList(*pColorList);
            m_pValSetColorList->Resize();
            ImpColorCountChanged();
            //FillValueSet_Impl(*m_pValSetColorList);

            // positioning
            long nColorCount = pColorList->Count();
            if( nColorCount != 0 )
            {
                if( nPos >= nColorCount  )
                    nPos = nColorCount - 1;
                m_pLbColor->SelectEntryPos( nPos );
            }
            else
                m_pLbColor->SetNoSelection();

            SelectColorLBHdl_Impl( *m_pLbColor );

            m_pCtlPreviewOld->Invalidate();

            *pnColorListState |= ChangeType::MODIFIED;
        }
    }
    UpdateModified();
}


IMPL_LINK_NOARG_TYPED(SvxColorTabPage, SelectColorLBHdl_Impl, ListBox&, void)
{
    sal_Int32 nPos = m_pLbColor->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        m_pValSetColorList->SelectItem( nPos + 1 );

        rXFSet.Put( XFillColorItem( OUString(),
                                    m_pLbColor->GetSelectEntryColor() ) );
        m_pCtlPreviewNew->SetAttributes( aXFillAttr.GetItemSet() );
        m_pCtlPreviewNew->Invalidate();

        XColorEntry* pEntry = pColorList->GetColor(nPos);
        ChangeColor(pEntry->GetColor());
    }
}


IMPL_LINK_NOARG_TYPED(SvxColorTabPage, SelectValSetHdl_Impl, ValueSet*, void)
{
    sal_Int32 nPos = m_pValSetColorList->GetSelectItemId();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        m_pLbColor->SelectEntryPos( nPos - 1 );

        rXFSet.Put( XFillColorItem( OUString(),
                                    m_pLbColor->GetSelectEntryColor() ) );
        m_pCtlPreviewNew->SetAttributes( aXFillAttr.GetItemSet() );
        m_pCtlPreviewNew->Invalidate();

        XColorEntry* pEntry = pColorList->GetColor(nPos-1);
        ChangeColor(pEntry->GetColor());
    }
}


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
IMPL_LINK_TYPED(SvxColorTabPage, SelectColorModeHdl_Impl, RadioButton&, rRadioButton, void)
{
    if( &rRadioButton == m_pRbRGB )
        eCM = CM_RGB;
    if( &rRadioButton == m_pRbCMYK )
        eCM = CM_CMYK;
    ChangeColorModel();
    UpdateColorValues();
}

void SvxColorTabPage::ChangeColor(const Color &rNewColor)
{
    aCurrentColor = rNewColor;
    UpdateColorValues();
    // fill ItemSet and pass it on to XOut
    rXFSet.Put( XFillColorItem( OUString(), aCurrentColor ) );
    m_pCtlPreviewNew->SetAttributes( aXFillAttr.GetItemSet() );
    m_pCtlPreviewNew->Invalidate();
}

void SvxColorTabPage::SetColorModel( ColorModel eModel )
{
    m_pRbRGB->SetState(false);
    m_pRbCMYK->SetState(false);
    if(eModel == CM_RGB)
        m_pRbRGB->SetState(true);
    if(eModel == CM_CMYK)
        m_pRbCMYK->SetState(true);
}

void SvxColorTabPage::ChangeColorModel()
{
    switch( eCM )
    {
        case CM_RGB:
        {
            m_pRGBcustom->Show();
            m_pRGBpreset->Show();
            m_pCMYKcustom->Hide();
            m_pCMYKpreset->Hide();
        }
        break;

        case CM_CMYK:
        {
            m_pCMYKcustom->Show();
            m_pCMYKpreset->Show();
            m_pRGBcustom->Hide();
            m_pRGBpreset->Hide();
        }
        break;
    }
}

void SvxColorTabPage::UpdateColorValues()
{
    if (eCM != CM_RGB)
    {
        ConvertColorValues (aPreviousColor, eCM );
        ConvertColorValues (aCurrentColor, eCM);

        m_pCcustom->SetValue( ColorToPercent_Impl( aCurrentColor.GetRed() ) );
        m_pCpreset->SetValue( ColorToPercent_Impl( aPreviousColor.GetRed() ) );
        m_pYcustom->SetValue( ColorToPercent_Impl( aCurrentColor.GetGreen() ) );
        m_pYpreset->SetValue( ColorToPercent_Impl( aPreviousColor.GetGreen() ) );
        m_pMcustom->SetValue( ColorToPercent_Impl( aCurrentColor.GetBlue() ) );
        m_pMpreset->SetValue( ColorToPercent_Impl( aPreviousColor.GetBlue() ) );
        m_pKcustom->SetValue( ColorToPercent_Impl( aCurrentColor.GetTransparency() ) );
        m_pKpreset->SetValue( ColorToPercent_Impl( aPreviousColor.GetTransparency() ) );

        ConvertColorValues (aPreviousColor, CM_RGB);
        ConvertColorValues (aCurrentColor, CM_RGB);
    }
    else
    {
        m_pRcustom->SetValue( ColorToPercent_Impl( aCurrentColor.GetRed() ) );
        m_pRpreset->SetValue( ColorToPercent_Impl( aPreviousColor.GetRed() ) );
        m_pGcustom->SetValue( ColorToPercent_Impl( aCurrentColor.GetGreen() ) );
        m_pGpreset->SetValue( ColorToPercent_Impl( aPreviousColor.GetGreen() ) );
        m_pBcustom->SetValue( ColorToPercent_Impl( aCurrentColor.GetBlue() ) );
        m_pBpreset->SetValue( ColorToPercent_Impl( aPreviousColor.GetBlue() ) );
        m_pHexcustom->SetColor( aCurrentColor.GetColor() );
        m_pHexpreset->SetColor( aPreviousColor.GetColor() );
    }
}

sal_Int32 SvxColorTabPage::SearchColorList(OUString const & aColorName)
{
    long nCount = pColorList->Count();
    bool bValidColorName = true;
    sal_Int32 nPos = LISTBOX_ENTRY_NOTFOUND;

    for(long i = 0;i < nCount && bValidColorName;i++)
    {
        if(aColorName == pColorList->GetColor( i )->GetName())
        {
            nPos = i;
            bValidColorName = false;
        }
    }
    return nPos;
}

//void SvxColorTabPage::FillValueSet_Impl( ValueSet& rVs )
//{
//    long nCount = pColorList->Count();
//    XColorEntry* pColorEntry;

//    if( nCount > 104 )
//        rVs.SetStyle( rVs.GetStyle() | WB_VSCROLL );

//    for( long i = 0; i < nCount; i++ )
//    {
//        pColorEntry = pColorList->GetColor( i );
//        rVs.InsertItem( i + 1, pColorEntry->GetColor(), pColorEntry->GetName() );
//    }
//}


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

    return nWert;
}


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

    return nWert;
}


void SvxColorTabPage::FillUserData()
{
    // the color model is saved in the Ini-file
    SetUserData( OUString::number( eCM ) );
}


void SvxColorTabPage::SetupForViewFrame( SfxViewFrame *pViewFrame )
{
    const OfaRefItem<XColorList> *pPtr = nullptr;
    if ( pViewFrame != nullptr && pViewFrame->GetDispatcher() )
        pPtr = static_cast<const OfaRefItem<XColorList> *>(pViewFrame->
            GetDispatcher()->Execute( SID_GET_COLORLIST,
                                      SfxCallMode::SYNCHRON ));
    pColorList = pPtr ? pPtr->GetValue() : XColorList::GetStdColorList();

    SetPageType( &pShadow->nUnknownType );
    SetDlgType( COLORPAGE_UNKNOWN );
    SetPos( &pShadow->nUnknownPos );
    SetAreaTP( &pShadow->bIsAreaTP );
    SetColorChgd( &pShadow->nChangeType );
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
    pPtr = static_cast<const OfaRefItem<XColorList>*>(pViewFrame->GetDispatcher()->Execute( SID_GET_COLORLIST, SfxCallMode::SYNCHRON ));
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

void SvxColorTabPage::SetColorList( const XColorListRef& pColList )
{
    SetPropertyList( XCOLOR_LIST, XPropertyListRef( ( pColList.get() ) ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

