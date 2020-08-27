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

#include <sal/config.h>

#include <comphelper/processfactory.hxx>
#include <tools/datetime.hxx>
#include <unotools/datetime.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <ucbhelper/content.hxx>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <sfx2/app.hxx>
#include <helpids.h>
#include <svx/gallery1.hxx>
#include <svx/galtheme.hxx>
#include <svx/galmisc.hxx>
#include "galbrws1.hxx"
#include <com/sun/star/util/DateTime.hpp>
#include <svx/strings.hrc>
#include <algorithm>
#include <svx/dialmgr.hxx>

#include <svx/svxdlg.hxx>
#include <memory>
#include <bitmaps.hlst>

using namespace ::com::sun::star;

GalleryBrowser1::GalleryBrowser1(
    weld::Builder& rBuilder,
    Gallery* pGallery,
    const std::function<void ()>& rThemeSelectionHandler)
    :
    mxNewTheme(rBuilder.weld_button("insert")),
    mxThemes(rBuilder.weld_tree_view("themelist")),
    mpGallery             ( pGallery ),
    mpExchangeData        ( new ExchangeData ),
    aImgNormal            ( RID_SVXBMP_THEME_NORMAL ),
    aImgDefault           ( RID_SVXBMP_THEME_DEFAULT ),
    aImgReadOnly          ( RID_SVXBMP_THEME_READONLY ),
    maThemeSelectionHandler(rThemeSelectionHandler)
{
    mxNewTheme->set_help_id(HID_GALLERY_NEWTHEME);
    mxNewTheme->connect_clicked( LINK( this, GalleryBrowser1, ClickNewThemeHdl ) );

    mxThemes->make_sorted();
    mxThemes->set_help_id( HID_GALLERY_THEMELIST );
    mxThemes->connect_changed( LINK( this, GalleryBrowser1, SelectThemeHdl ) );
    mxThemes->connect_popup_menu(LINK(this, GalleryBrowser1, PopupMenuHdl));
    mxThemes->connect_key_press(LINK(this, GalleryBrowser1, KeyInputHdl));
    mxThemes->set_size_request(-1, mxThemes->get_height_rows(6));

    // disable creation of new themes if a writable directory is not available
    if( mpGallery->GetUserURL().GetProtocol() == INetProtocol::NotValid )
        mxNewTheme->set_sensitive(false);

    StartListening( *mpGallery );

    for (size_t i = 0, nCount = mpGallery->GetThemeCount(); i < nCount; ++i)
        ImplInsertThemeEntry( mpGallery->GetThemeInfo( i ) );
}

GalleryBrowser1::~GalleryBrowser1()
{
    EndListening( *mpGallery );
    mpExchangeData.reset();
}

void GalleryBrowser1::ImplInsertThemeEntry( const GalleryThemeEntry* pEntry )
{
    static const bool bShowHiddenThemes = ( getenv( "GALLERY_SHOW_HIDDEN_THEMES" ) != nullptr );

    if( !(pEntry && ( !pEntry->IsHidden() || bShowHiddenThemes )) )
        return;

    const OUString* pImage;

    if( pEntry->IsReadOnly() )
        pImage = &aImgReadOnly;
    else if( pEntry->IsDefault() )
        pImage = &aImgDefault;
    else
        pImage = &aImgNormal;

    mxThemes->append("", pEntry->GetThemeName(), *pImage);
}

void GalleryBrowser1::ImplFillExchangeData( const GalleryTheme* pThm, ExchangeData& rData )
{
    rData.pTheme = const_cast<GalleryTheme*>(pThm);
    rData.aEditedTitle = pThm->GetName();

    try
    {
        DateTime aDateTime(pThm->getModificationDate());

        rData.aThemeChangeDate = aDateTime;
        rData.aThemeChangeTime = aDateTime;
    }
    catch( const ucb::ContentCreationException& )
    {
    }
    catch( const uno::RuntimeException& )
    {
    }
    catch( const uno::Exception& )
    {
    }
}

void GalleryBrowser1::ImplGetExecuteVector(std::vector<OString>& o_aExec)
{
    GalleryTheme*           pTheme = mpGallery->AcquireTheme( GetSelectedTheme(), *this );

    if( !pTheme )
        return;

    bool                bUpdateAllowed, bRenameAllowed, bRemoveAllowed;
    static const bool   bIdDialog = ( getenv( "GALLERY_ENABLE_ID_DIALOG" ) != nullptr );

    if( pTheme->IsReadOnly() )
        bUpdateAllowed = bRenameAllowed = bRemoveAllowed = false;
    else if( pTheme->IsDefault() )
    {
        bUpdateAllowed = bRenameAllowed = true;
        bRemoveAllowed = false;
    }
    else
        bUpdateAllowed = bRenameAllowed = bRemoveAllowed = true;

    if( bUpdateAllowed && pTheme->GetObjectCount() )
        o_aExec.emplace_back("update");

    if( bRenameAllowed )
        o_aExec.emplace_back("rename");

    if( bRemoveAllowed )
        o_aExec.emplace_back("delete");

    if( bIdDialog && !pTheme->IsReadOnly() )
        o_aExec.emplace_back("assign");

    o_aExec.emplace_back("properties");

    mpGallery->ReleaseTheme( pTheme, *this );
}

void GalleryBrowser1::ImplGalleryThemeProperties( const OUString & rThemeName, bool bCreateNew )
{
    DBG_ASSERT(!mpThemePropsDlgItemSet, "mpThemePropsDlgItemSet already set!");
    mpThemePropsDlgItemSet.reset(new SfxItemSet( SfxGetpApp()->GetPool() ));
    GalleryTheme*   pTheme = mpGallery->AcquireTheme( rThemeName, *this );

    ImplFillExchangeData( pTheme, *mpExchangeData );

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    VclPtr<VclAbstractDialog> xThemePropertiesDialog = pFact->CreateGalleryThemePropertiesDialog(mxThemes.get(), mpExchangeData.get(), mpThemePropsDlgItemSet.get());

    if ( bCreateNew )
    {
        xThemePropertiesDialog->StartExecuteAsync([xThemePropertiesDialog, this](sal_Int32 nResult){
            EndNewThemePropertiesDlgHdl(nResult);
            xThemePropertiesDialog->disposeOnce();
        });
    }
    else
    {
        xThemePropertiesDialog->StartExecuteAsync([xThemePropertiesDialog, this](sal_Int32 nResult){
            EndThemePropertiesDlgHdl(nResult);
            xThemePropertiesDialog->disposeOnce();
        });
    }
}

void GalleryBrowser1::ImplEndGalleryThemeProperties(bool bCreateNew, sal_Int32 nRet)
{
    if( nRet == RET_OK )
    {
        OUString aName( mpExchangeData->pTheme->GetName() );

        if( !mpExchangeData->aEditedTitle.isEmpty() && aName != mpExchangeData->aEditedTitle )
        {
            OUString            aTitle( mpExchangeData->aEditedTitle );
            sal_uInt16          nCount = 0;

            while( mpGallery->HasTheme( aTitle ) && ( nCount++ < 16000 ) )
            {
                aTitle = mpExchangeData->aEditedTitle + " " + OUString::number( nCount );
            }

            mpGallery->RenameTheme( aName, aTitle );
        }

        if ( bCreateNew )
        {
            mxThemes->select_text( mpExchangeData->pTheme->GetName() );
            SelectThemeHdl( *mxThemes );
        }
    }

    OUString aThemeName( mpExchangeData->pTheme->GetName() );
    mpGallery->ReleaseTheme( mpExchangeData->pTheme, *this );

    if ( bCreateNew && ( nRet != RET_OK ) )
    {
        mpGallery->RemoveTheme( aThemeName );
    }
}

void GalleryBrowser1::EndNewThemePropertiesDlgHdl(sal_Int32 nResult)
{
    ImplEndGalleryThemeProperties(true, nResult);
}

void GalleryBrowser1::EndThemePropertiesDlgHdl(sal_Int32 nResult)
{
    ImplEndGalleryThemeProperties(false, nResult);
}

void GalleryBrowser1::ImplExecute(const OString &rIdent)
{
    if (rIdent == "update")
    {
        GalleryTheme*       pTheme = mpGallery->AcquireTheme( GetSelectedTheme(), *this );

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        ScopedVclPtr<VclAbstractDialog> aActualizeProgress(pFact->CreateActualizeProgressDialog(mxThemes.get(), pTheme));

        aActualizeProgress->Execute();
        mpGallery->ReleaseTheme( pTheme, *this );
    }
    else if (rIdent == "delete")
    {
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(mxThemes.get(), "svx/ui/querydeletethemedialog.ui"));
        std::unique_ptr<weld::MessageDialog> xQuery(xBuilder->weld_message_dialog("QueryDeleteThemeDialog"));
        if (xQuery->run() == RET_YES)
            mpGallery->RemoveTheme( mxThemes->get_selected_text() );
    }
    else if (rIdent == "rename")
    {
        GalleryTheme*   pTheme = mpGallery->AcquireTheme( GetSelectedTheme(), *this );
        const OUString  aOldName( pTheme->GetName() );

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        ScopedVclPtr<AbstractTitleDialog> aDlg(pFact->CreateTitleDialog(mxThemes.get(), aOldName));

        if( aDlg->Execute() == RET_OK )
        {
            const OUString aNewName( aDlg->GetTitle() );

            if( !aNewName.isEmpty() && ( aNewName != aOldName ) )
            {
                OUString  aName( aNewName );
                sal_uInt16  nCount = 0;

                while( mpGallery->HasTheme( aName ) && ( nCount++ < 16000 ) )
                {
                    aName = aNewName + " " + OUString::number( nCount );
                }

                mpGallery->RenameTheme( aOldName, aName );
            }
        }
        mpGallery->ReleaseTheme( pTheme, *this );
    }
    else if (rIdent == "assign")
    {
        GalleryTheme* pTheme = mpGallery->AcquireTheme( GetSelectedTheme(), *this );

        if (pTheme && !pTheme->IsReadOnly())
        {

            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            ScopedVclPtr<AbstractGalleryIdDialog> aDlg(pFact->CreateGalleryIdDialog(mxThemes.get(), pTheme));
            if( aDlg->Execute() == RET_OK )
                pTheme->SetId( aDlg->GetId(), true );
        }

        mpGallery->ReleaseTheme( pTheme, *this );
    }
    else if (rIdent == "properties")
    {
        ImplGalleryThemeProperties( GetSelectedTheme(), false );
    }
}

void GalleryBrowser1::GrabFocus()
{
    if (mxNewTheme->get_sensitive())
        mxNewTheme->grab_focus();
    else
        mxThemes->grab_focus();
}

void GalleryBrowser1::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    const GalleryHint& rGalleryHint = static_cast<const GalleryHint&>(rHint);

    switch( rGalleryHint.GetType() )
    {
        case GalleryHintType::THEME_CREATED:
            ImplInsertThemeEntry( mpGallery->GetThemeInfo( rGalleryHint.GetThemeName() ) );
        break;

        case GalleryHintType::THEME_RENAMED:
        {
            const sal_Int32 nCurSelectPos = mxThemes->get_selected_index();
            const sal_Int32 nRenameEntryPos = mxThemes->find_text( rGalleryHint.GetThemeName() );

            mxThemes->remove_text( rGalleryHint.GetThemeName() );
            ImplInsertThemeEntry( mpGallery->GetThemeInfo( rGalleryHint.GetStringData() ) );

            if( nCurSelectPos == nRenameEntryPos )
            {
                mxThemes->select_text( rGalleryHint.GetStringData() );
                SelectThemeHdl( *mxThemes );
            }
        }
        break;

        case GalleryHintType::THEME_REMOVED:
        {
            mxThemes->remove_text( rGalleryHint.GetThemeName() );
        }
        break;

        case GalleryHintType::CLOSE_THEME:
        {
            const sal_Int32 nCurSelectPos = mxThemes->get_selected_index();
            const sal_Int32 nCloseEntryPos = mxThemes->find_text( rGalleryHint.GetThemeName() );

            if( nCurSelectPos == nCloseEntryPos )
            {
                if( nCurSelectPos < ( mxThemes->n_children() - 1 ) )
                    mxThemes->select( nCurSelectPos + 1 );
                else if( nCurSelectPos )
                    mxThemes->select( nCurSelectPos - 1 );
                else
                    mxThemes->select(-1);

                SelectThemeHdl( *mxThemes );
            }
        }
        break;

        default:
        break;
    }
}

IMPL_LINK(GalleryBrowser1, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    bool bRet = false;

    std::vector<OString> aExecVector;
    ImplGetExecuteVector(aExecVector);
    OString sExecuteIdent;
    bool bMod1 = rKEvt.GetKeyCode().IsMod1();

    switch( rKEvt.GetKeyCode().GetCode() )
    {
        case KEY_INSERT:
            ClickNewThemeHdl(*mxNewTheme);
        break;

        case KEY_I:
        {
            if( bMod1 )
               ClickNewThemeHdl(*mxNewTheme);
        }
        break;

        case KEY_U:
        {
            if( bMod1 )
                sExecuteIdent = "update";
        }
        break;

        case KEY_DELETE:
            sExecuteIdent = "delete";
        break;

        case KEY_D:
        {
            if( bMod1 )
                sExecuteIdent = "delete";
        }
        break;

        case KEY_R:
        {
            if( bMod1 )
                sExecuteIdent = "rename";
        }
        break;

        case KEY_RETURN:
        {
            if( bMod1 )
                sExecuteIdent = "properties";
        }
        break;
    }

    if (!sExecuteIdent.isEmpty() && (std::find( aExecVector.begin(), aExecVector.end(), sExecuteIdent) != aExecVector.end()))
    {
        ImplExecute(sExecuteIdent);
        bRet = true;
    }

    return bRet;
}

IMPL_LINK(GalleryBrowser1, PopupMenuHdl, const CommandEvent&, rCEvt, bool)
{
    if (rCEvt.GetCommand() != CommandEventId::ContextMenu)
        return false;

    std::vector<OString> aExecVector;
    ImplGetExecuteVector(aExecVector);

    if (aExecVector.empty())
        return true;

    std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(mxThemes.get(), "svx/ui/gallerymenu1.ui"));
    std::unique_ptr<weld::Menu> xMenu(xBuilder->weld_menu("menu"));

    xMenu->set_visible("update", std::find( aExecVector.begin(), aExecVector.end(), "update" ) != aExecVector.end());
    xMenu->set_visible("rename", std::find( aExecVector.begin(), aExecVector.end(), "rename" ) != aExecVector.end());
    xMenu->set_visible("delete", std::find( aExecVector.begin(), aExecVector.end(), "delete" ) != aExecVector.end());
    xMenu->set_visible("assign", std::find( aExecVector.begin(), aExecVector.end(), "assign" ) != aExecVector.end());
    xMenu->set_visible("properties", std::find( aExecVector.begin(), aExecVector.end(), "properties" ) != aExecVector.end());

    OString sCommand(xMenu->popup_at_rect(mxThemes.get(), tools::Rectangle(rCEvt.GetMousePosPixel(), Size(1,1))));
    ImplExecute(sCommand);

    return true;
}

IMPL_LINK_NOARG(GalleryBrowser1, SelectThemeHdl, weld::TreeView&, void)
{
    if (maThemeSelectionHandler)
        maThemeSelectionHandler();
}

IMPL_LINK_NOARG(GalleryBrowser1, ClickNewThemeHdl, weld::Button&, void)
{
    OUString  aNewTheme( SvxResId(RID_SVXSTR_GALLERY_NEWTHEME) );
    OUString  aName( aNewTheme );
    sal_uInt16 nCount = 0;

    while( mpGallery->HasTheme( aName ) && ( nCount++ < 16000 ) )
    {
        aName = aNewTheme + " " + OUString::number( nCount );
    }

    if( !mpGallery->HasTheme( aName ) && mpGallery->CreateTheme( aName ) )
    {
        ImplGalleryThemeProperties( aName, true );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
