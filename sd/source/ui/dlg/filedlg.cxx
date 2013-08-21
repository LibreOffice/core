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

#include <tools/debug.hxx>
#include <cppuhelper/implbase1.hxx>
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
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <vcl/msgbox.hxx>
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


// --------------------------------------------------------------------
// -----------      SdFileDialog_Imp        ---------------------------
// --------------------------------------------------------------------
class SdFileDialog_Imp : public sfx2::FileDialogHelper
{
private:
#if defined __SUNPRO_CC
    using sfx2::FileDialogHelper::Execute;
#endif

    friend class SdOpenSoundFileDialog;

    css::uno::Reference< css::ui::dialogs::XFilePickerControlAccess >   mxControlAccess;

    css::uno::Reference< css::media::XPlayer > mxPlayer;
    sal_uLong                       mnPlaySoundEvent;
    sal_Bool                        mbUsableSelection;
    sal_Bool                        mbLabelPlaying;

    void                        CheckSelectionState();

                                DECL_LINK( PlayMusicHdl, void * );

    Timer                       maUpdateTimer;

                                DECL_LINK( IsMusicStoppedHdl, void * );

public:
                                SdFileDialog_Imp( const short nDialogType, sal_Bool bUsableSelection );
                                   ~SdFileDialog_Imp();

    ErrCode                     Execute();

    // overwritten from FileDialogHelper, to receive user feedback
    virtual void SAL_CALL       ControlStateChanged( const css::ui::dialogs::FilePickerEvent& aEvent );
};

// ------------------------------------------------------------------------
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

// ------------------------------------------------------------------------
IMPL_LINK_NOARG(SdFileDialog_Imp, PlayMusicHdl)
{
    maUpdateTimer.Stop();
    mnPlaySoundEvent = 0;

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

            mbLabelPlaying = sal_False;
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
                mxPlayer.set( avmedia::MediaWindow::createPlayer( aUrl ), css::uno::UNO_QUERY_THROW );
                mxPlayer->start();
                maUpdateTimer.SetTimeout( 100 );
                maUpdateTimer.Start();
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

                    mbLabelPlaying = sal_True;
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

    return 0;
}

// ------------------------------------------------------------------------
IMPL_LINK_NOARG(SdFileDialog_Imp, IsMusicStoppedHdl)
{
    SolarMutexGuard aGuard;

    if (
        mxPlayer.is() && mxPlayer->isPlaying() &&
        mxPlayer->getMediaTime() < mxPlayer->getDuration()
       )
    {
        maUpdateTimer.Start();
        return 0L;
    }


    if( mxControlAccess.is() )
    {
        try
        {
            mxControlAccess->setLabel( css::ui::dialogs::ExtendedFilePickerElementIds::PUSHBUTTON_PLAY,
                                       SD_RESSTR( STR_PLAY ) );
            mbLabelPlaying = sal_False;
        }
        catch (const css::lang::IllegalArgumentException&)
        {
#ifdef DBG_UTIL
            OSL_FAIL( "Cannot access play button" );
#endif
        }
    }

    return( 0L );
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
                mxControlAccess->enableControl( css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_SELECTION, sal_False );
            else
                mxControlAccess->enableControl( css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_SELECTION, sal_True );
        }
        catch (const css::lang::IllegalArgumentException&)
        {
#ifdef DBG_UTIL
            OSL_FAIL( "Cannot access \"selection\" checkbox" );
#endif
        }
    }
}

//-----------------------------------------------------------------------------
SdFileDialog_Imp::SdFileDialog_Imp( const short     nDialogType,
                                    sal_Bool        bUsableSelection    ) :
    FileDialogHelper( nDialogType, 0 ),
    mnPlaySoundEvent( 0 ),
    mbUsableSelection( bUsableSelection ),
    mbLabelPlaying(sal_False)
{
    maUpdateTimer.SetTimeoutHdl(LINK(this, SdFileDialog_Imp, IsMusicStoppedHdl));

    css::uno::Reference < ::com::sun::star::ui::dialogs::XFilePicker > xFileDlg = GetFilePicker();

    // get the control access
    mxControlAccess = css::uno::Reference< css::ui::dialogs::XFilePickerControlAccess > ( xFileDlg, css::uno::UNO_QUERY );

    if( mxControlAccess.is() )
    {
        if( nDialogType ==
            css::ui::dialogs::TemplateDescription::FILEOPEN_PLAY )
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
        else if( mbUsableSelection != sal_True )
        {
            try
            {
                mxControlAccess->enableControl( css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_SELECTION, sal_False );
            }
            catch (const css::lang::IllegalArgumentException&)
            {
#ifdef DBG_UTIL
                OSL_FAIL( "Cannot disable selection checkbox" );
#endif
            }
        }
    }
}


// ------------------------------------------------------------------------
SdFileDialog_Imp::~SdFileDialog_Imp()
{
    if( mnPlaySoundEvent )
        Application::RemoveUserEvent( mnPlaySoundEvent );
}

// ------------------------------------------------------------------------
ErrCode SdFileDialog_Imp::Execute()
{
    // make sure selection checkbox is disabled if
    // HTML is current filter!
    CheckSelectionState();
    return FileDialogHelper::Execute();
}

// --------------------------------------------------------------------
// -----------      SdOpenSoundFileDialog       -----------------------
// --------------------------------------------------------------------

// these are simple forwarders
SdOpenSoundFileDialog::SdOpenSoundFileDialog() :
    mpImpl(
        new SdFileDialog_Imp(
            css::ui::dialogs::TemplateDescription::FILEOPEN_PLAY, sal_False ) )
{
    OUString aDescr;
    aDescr = SD_RESSTR(STR_ALL_FILES);
    mpImpl->AddFilter( aDescr, OUString("*.*"));

    // setup filter
#if defined UNX
    aDescr = SD_RESSTR(STR_AU_FILE);
    mpImpl->AddFilter( aDescr, OUString("*.au;*.snd" ));
    aDescr = SD_RESSTR(STR_VOC_FILE);
    mpImpl->AddFilter( aDescr, OUString("*.voc" ));
    aDescr = SD_RESSTR(STR_WAV_FILE);
    mpImpl->AddFilter( aDescr, OUString("*.wav" ));
    aDescr = SD_RESSTR(STR_AIFF_FILE);
    mpImpl->AddFilter( aDescr, OUString("*.aiff" ));
    aDescr = SD_RESSTR(STR_SVX_FILE);
    mpImpl->AddFilter( aDescr, OUString("*.svx" ));
#else
    aDescr = SD_RESSTR(STR_WAV_FILE);
    mpImpl->AddFilter( aDescr, OUString("*.wav;*.mp3;*.ogg" ));
    aDescr = SD_RESSTR(STR_MIDI_FILE);
    mpImpl->AddFilter( aDescr, OUString("*.mid" ));
#endif
}

// ------------------------------------------------------------------------
SdOpenSoundFileDialog::~SdOpenSoundFileDialog()
{
}

// ------------------------------------------------------------------------
ErrCode SdOpenSoundFileDialog::Execute()
{
    return mpImpl->Execute();
}

// ------------------------------------------------------------------------
OUString SdOpenSoundFileDialog::GetPath() const
{
    return mpImpl->GetPath();
}

// ------------------------------------------------------------------------
void SdOpenSoundFileDialog::SetPath( const OUString& rPath )
{
    mpImpl->SetDisplayDirectory( rPath );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
