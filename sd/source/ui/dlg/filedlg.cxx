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

#include <config_features.h>

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/FilePickerEvent.hpp>
#include <vcl/idle.hxx>
#include <osl/diagnose.h>
#include <vcl/svapp.hxx>
#include <sfx2/filedlghelper.hxx>
#include <avmedia/mediawindow.hxx>
#include <filedlg.hxx>
#include <sdresid.hxx>
#include <strings.hrc>
#include <officecfg/Office/Impress.hxx>

// -----------      SdFileDialog_Imp        ---------------------------

class SdFileDialog_Imp : public sfx2::FileDialogHelper
{
private:
    friend class SdOpenSoundFileDialog;

    css::uno::Reference< css::ui::dialogs::XFilePickerControlAccess >   mxControlAccess;

    css::uno::Reference< css::media::XPlayer > mxPlayer;
    ImplSVEvent * mnPlaySoundEvent;
    bool mbLabelPlaying;
    Idle maUpdateIdle;

    DECL_LINK( PlayMusicHdl, void *, void );
    DECL_LINK( IsMusicStoppedHdl, Timer *, void );

public:
    explicit SdFileDialog_Imp(weld::Window *pParent);
    virtual ~SdFileDialog_Imp() override;

    // overwritten from FileDialogHelper, to receive user feedback
    virtual void ControlStateChanged( const css::ui::dialogs::FilePickerEvent& aEvent ) override;
};

void SdFileDialog_Imp::ControlStateChanged( const css::ui::dialogs::FilePickerEvent& aEvent )
{
    SolarMutexGuard aGuard;

    switch( aEvent.ElementId )
    {
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

IMPL_LINK_NOARG(SdFileDialog_Imp, PlayMusicHdl, void*, void)
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
                                       SdResId( STR_PLAY ) );
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
#if HAVE_FEATURE_AVMEDIA
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
#endif
            if (mxPlayer.is())
            {
                try
                {
                    mxControlAccess->setLabel( css::ui::dialogs::ExtendedFilePickerElementIds::PUSHBUTTON_PLAY,
                                               SdResId( STR_STOP ) );
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

IMPL_LINK_NOARG(SdFileDialog_Imp, IsMusicStoppedHdl, Timer *, void)
{
    SolarMutexGuard aGuard;

    if (mxPlayer.is() && mxPlayer->isPlaying() &&
            mxPlayer->getMediaTime() < mxPlayer->getDuration())
    {
        maUpdateIdle.Start();
        return;
    }

    if( !mxControlAccess.is() )
        return;

    try
    {
        mxControlAccess->setLabel( css::ui::dialogs::ExtendedFilePickerElementIds::PUSHBUTTON_PLAY,
                                   SdResId( STR_PLAY ) );
        mbLabelPlaying = false;
    }
    catch (const css::lang::IllegalArgumentException&)
    {
#ifdef DBG_UTIL
        OSL_FAIL( "Cannot access play button" );
#endif
    }
}

SdFileDialog_Imp::SdFileDialog_Imp(weld::Window* pParent)
    : FileDialogHelper(css::ui::dialogs::TemplateDescription::FILEOPEN_LINK_PLAY, FileDialogFlags::NONE, pParent)
    , mnPlaySoundEvent(nullptr)
    , mbLabelPlaying(false)
{
    maUpdateIdle.SetInvokeHandler(LINK(this, SdFileDialog_Imp, IsMusicStoppedHdl));
    maUpdateIdle.SetDebugName( "SdFileDialog_Imp maUpdateIdle" );

    css::uno::Reference < css::ui::dialogs::XFilePicker3 > xFileDlg = GetFilePicker();

    // get the control access
    mxControlAccess.set( xFileDlg, css::uno::UNO_QUERY );

    if( !mxControlAccess.is() )
        return;

    try
    {
        mxControlAccess->setLabel( css::ui::dialogs::ExtendedFilePickerElementIds::PUSHBUTTON_PLAY,
                                   SdResId( STR_PLAY ) );
    }
    catch (const css::lang::IllegalArgumentException&)
    {
#ifdef DBG_UTIL
        OSL_FAIL( "Cannot set play button label" );
#endif
    }
}

SdFileDialog_Imp::~SdFileDialog_Imp()
{
    if( mnPlaySoundEvent )
        Application::RemoveUserEvent( mnPlaySoundEvent );
}

// -----------      SdOpenSoundFileDialog       -----------------------

// these are simple forwarders
SdOpenSoundFileDialog::SdOpenSoundFileDialog(weld::Window *pParent)
    : mpImpl(new SdFileDialog_Imp(pParent))
{
    OUString aDescr;
    aDescr = SdResId(STR_ALL_FILES);
    mpImpl->AddFilter( aDescr, "*.*");

    // setup filter
#if defined UNX
    aDescr = SdResId(STR_AU_FILE);
    mpImpl->AddFilter( aDescr, "*.au;*.snd");
    aDescr = SdResId(STR_VOC_FILE);
    mpImpl->AddFilter( aDescr, "*.voc");
    aDescr = SdResId(STR_WAV_FILE);
    mpImpl->AddFilter( aDescr, "*.wav");
    aDescr = SdResId(STR_AIFF_FILE);
    mpImpl->AddFilter( aDescr, "*.aiff");
    aDescr = SdResId(STR_SVX_FILE);
    mpImpl->AddFilter( aDescr, "*.svx");
#else
    aDescr = SdResId(STR_WAV_FILE);
    mpImpl->AddFilter( aDescr, "*.wav;*.mp3;*.ogg" );
    aDescr = SdResId(STR_MIDI_FILE);
    mpImpl->AddFilter( aDescr, "*.mid" );
#endif

    // Restore last selected path
    mpImpl->SetDisplayDirectory(officecfg::Office::Impress::Sound::Path::get());
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
    // Save last selected path
    std::shared_ptr<comphelper::ConfigurationChanges> batch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Impress::Sound::Path::set(mpImpl->GetPath(), batch);
    batch->commit();

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
    css::uno::Reference<css::ui::dialogs::XFilePicker3> const xFilePicker(mpImpl->GetFilePicker());
    css::uno::Reference<css::ui::dialogs::XFilePickerControlAccess> const xControlAccess(xFilePicker, css::uno::UNO_QUERY_THROW);
    xControlAccess->getValue(css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_LINK, 0) >>= bInsertAsLinkSelected;
    return bInsertAsLinkSelected;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
