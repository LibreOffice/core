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

#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/ListboxControlActions.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerListener.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerNotifier.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker2.hpp>
#include <vcl/msgbox.hxx>
#include <vcl/idle.hxx>
#include <sal/types.h>
#include <osl/thread.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/filedlghelper.hxx>
#include <avmedia/mediawindow.hxx>
#include "filedlg.hxx"
#include "sdresid.hxx"
#include "strings.hrc"
#include <vcl/graphicfilter.hxx>

// -----------      SdFileDialog_Imp        ---------------------------

class SdFileDialog_Imp : public sfx2::FileDialogHelper
{
private:
    friend class SdOpenSoundFileDialog;

    css::uno::Reference< css::ui::dialogs::XFilePickerControlAccess >   mxControlAccess;

    css::uno::Reference< css::media::XPlayer > mxPlayer;
    ImplSVEvent * mnPlaySoundEvent;
    bool mbUsableSelection;
    bool mbLabelPlaying;
    Idle maUpdateIdle;

    void CheckSelectionState();

    DECL_LINK_TYPED( PlayMusicHdl, void *, void );
    DECL_LINK_TYPED( IsMusicStoppedHdl, Idle *, void );

public:
    explicit SdFileDialog_Imp();
    virtual ~SdFileDialog_Imp() override;

    ErrCode Execute();

    // overwritten from FileDialogHelper, to receive user feedback
    virtual void SAL_CALL ControlStateChanged( const css::ui::dialogs::FilePickerEvent& aEvent ) override;
};

void SAL_CALL SdFileDialog_Imp::ControlStateChanged( const css::ui::dialogs::FilePickerEvent& aEvent )
{
    SolarMutexGuard aGuard;

    switch( aEvent.ElementId )
    {
        case css::ui::dialogs::CommonFilePickerElementIds::LISTBOX_FILTER:
            CheckSelectionState();
            break;

        case css::ui::dialogs::ExtendedFilePickerElementIds::PUSHBUTTON_PLAY:
            if( mxControlAccess.is() )
            {
                if( mnPlaySoundEvent )
                    Application::RemoveUserEvent( mnPlaySoundEvent );

                mnPlaySoundEvent = Application::PostUserEvent( LINK( this, SdFileDialog_Imp, PlayMusicHdl ) );
            }
            break;
    }
}

IMPL_LINK_NOARG_TYPED(SdFileDialog_Imp, PlayMusicHdl, void*, void)
{
    maUpdateIdle.Stop();
    mnPlaySoundEvent = nullptr;

    if (mxPlayer.is())
    {
        if (mxPlayer->isPlaying())
            mxPlayer->stop();
        mxPlayer.clear();
    }

    if( mbLabelPlaying )
    {
        try
        {
            mxControlAccess->setLabel( css::ui::dialogs::ExtendedFilePickerElementIds::PUSHBUTTON_PLAY,
                                       SD_RESSTR( STR_PLAY ) );
            mbLabelPlaying = false;
        }
        catch(const css::lang::IllegalArgumentException&)
        {
#ifdef DBG_UTIL
            OSL_FAIL( "Cannot access play button" );
#endif
        }
    }
    else
    {
        OUString aUrl( GetPath() );
        if ( !aUrl.isEmpty() )
        {
            try
            {
                mxPlayer.set( avmedia::MediaWindow::createPlayer( aUrl, "" ), css::uno::UNO_QUERY_THROW );
                mxPlayer->start();
                maUpdateIdle.Start();
            }
            catch (const css::uno::Exception&)
            {
                mxPlayer.clear();
            }

            if (mxPlayer.is())
            {
                try
                {
                    mxControlAccess->setLabel( css::ui::dialogs::ExtendedFilePickerElementIds::PUSHBUTTON_PLAY,
                                               SD_RESSTR( STR_STOP ) );
                    mbLabelPlaying = true;
                }
                catch (const css::lang::IllegalArgumentException&)
                {
#ifdef DBG_UTIL
                    OSL_FAIL( "Cannot access play button" );
#endif
                }
            }
        }
    }
}

IMPL_LINK_NOARG_TYPED(SdFileDialog_Imp, IsMusicStoppedHdl, Idle *, void)
{
    SolarMutexGuard aGuard;

    if (mxPlayer.is() && mxPlayer->isPlaying() &&
            mxPlayer->getMediaTime() < mxPlayer->getDuration())
    {
        maUpdateIdle.Start();
        return;
    }

    if( mxControlAccess.is() )
    {
        try
        {
            mxControlAccess->setLabel( css::ui::dialogs::ExtendedFilePickerElementIds::PUSHBUTTON_PLAY,
                                       SD_RESSTR( STR_PLAY ) );
            mbLabelPlaying = false;
        }
        catch (const css::lang::IllegalArgumentException&)
        {
#ifdef DBG_UTIL
            OSL_FAIL( "Cannot access play button" );
#endif
        }
    }
}

// check whether to disable the "selection" checkbox
void SdFileDialog_Imp::CheckSelectionState()
{
    if( mbUsableSelection && mxControlAccess.is() )
    {
        OUString  aCurrFilter( GetCurrentFilter() );

        try
        {
            if( aCurrFilter.isEmpty() || ( aCurrFilter == SD_RESSTR( STR_EXPORT_HTML_NAME ) ) )
                mxControlAccess->enableControl( css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_SELECTION, false );
            else
                mxControlAccess->enableControl( css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_SELECTION, true );
        }
        catch (const css::lang::IllegalArgumentException&)
        {
#ifdef DBG_UTIL
            OSL_FAIL( "Cannot access \"selection\" checkbox" );
#endif
        }
    }
}

SdFileDialog_Imp::SdFileDialog_Imp() :
    FileDialogHelper( css::ui::dialogs::TemplateDescription::FILEOPEN_LINK_PLAY ),
    mnPlaySoundEvent( nullptr ),
    mbUsableSelection( false ),
    mbLabelPlaying(false)
{
    maUpdateIdle.SetIdleHdl(LINK(this, SdFileDialog_Imp, IsMusicStoppedHdl));
    maUpdateIdle.SetDebugName( "SdFileDialog_Imp maUpdateIdle" );

    css::uno::Reference < css::ui::dialogs::XFilePicker2 > xFileDlg = GetFilePicker();

    // get the control access
    mxControlAccess.set( xFileDlg, css::uno::UNO_QUERY );

    if( mxControlAccess.is() )
    {
        try
        {
            mxControlAccess->setLabel( css::ui::dialogs::ExtendedFilePickerElementIds::PUSHBUTTON_PLAY,
                                       SD_RESSTR( STR_PLAY ) );
        }
        catch (const css::lang::IllegalArgumentException&)
        {
#ifdef DBG_UTIL
            OSL_FAIL( "Cannot set play button label" );
#endif
        }
    }
}

SdFileDialog_Imp::~SdFileDialog_Imp()
{
    if( mnPlaySoundEvent )
        Application::RemoveUserEvent( mnPlaySoundEvent );
}

ErrCode SdFileDialog_Imp::Execute()
{
    // make sure selection checkbox is disabled if HTML is current filter!
    CheckSelectionState();
    return FileDialogHelper::Execute();
}

// -----------      SdOpenSoundFileDialog       -----------------------

// these are simple forwarders
SdOpenSoundFileDialog::SdOpenSoundFileDialog() :
    mpImpl( new SdFileDialog_Imp() )
{
    OUString aDescr;
    aDescr = SD_RESSTR(STR_ALL_FILES);
    mpImpl->AddFilter( aDescr, "*.*");

    // setup filter
#if defined UNX
    aDescr = SD_RESSTR(STR_AU_FILE);
    mpImpl->AddFilter( aDescr, "*.au;*.snd");
    aDescr = SD_RESSTR(STR_VOC_FILE);
    mpImpl->AddFilter( aDescr, "*.voc");
    aDescr = SD_RESSTR(STR_WAV_FILE);
    mpImpl->AddFilter( aDescr, "*.wav");
    aDescr = SD_RESSTR(STR_AIFF_FILE);
    mpImpl->AddFilter( aDescr, "*.aiff");
    aDescr = SD_RESSTR(STR_SVX_FILE);
    mpImpl->AddFilter( aDescr, "*.svx");
#else
    aDescr = SD_RESSTR(STR_WAV_FILE);
    mpImpl->AddFilter( aDescr, OUString("*.wav;*.mp3;*.ogg" ));
    aDescr = SD_RESSTR(STR_MIDI_FILE);
    mpImpl->AddFilter( aDescr, OUString("*.mid" ));
#endif
}

SdOpenSoundFileDialog::~SdOpenSoundFileDialog()
{
}

ErrCode SdOpenSoundFileDialog::Execute()
{
    return mpImpl->Execute();
}

OUString SdOpenSoundFileDialog::GetPath() const
{
    return mpImpl->GetPath();
}

void SdOpenSoundFileDialog::SetPath( const OUString& rPath )
{
    mpImpl->SetDisplayDirectory( rPath );
}

// WIP, please don't remove, dear Clang plugins
bool SdOpenSoundFileDialog::IsInsertAsLinkSelected()
{
    bool bInsertAsLinkSelected = false;
    css::uno::Reference<css::ui::dialogs::XFilePicker2> const xFilePicker(mpImpl->GetFilePicker());
    css::uno::Reference<css::ui::dialogs::XFilePickerControlAccess> const xControlAccess(xFilePicker, css::uno::UNO_QUERY_THROW);
    xControlAccess->getValue(css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_LINK, 0) >>= bInsertAsLinkSelected;
    return bInsertAsLinkSelected;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
