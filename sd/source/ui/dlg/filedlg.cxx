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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"
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
#include <tools/urlobj.hxx>
#include <vos/thread.hxx>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/filedlghelper.hxx>
#include <avmedia/mediawindow.hxx>
#include "filedlg.hxx"
#include "sdresid.hxx"
#include "strings.hrc"
#include <svtools/filter.hxx>


//-----------------------------------------------------------------------------

namespace css = ::com::sun::star;


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
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

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
IMPL_LINK( SdFileDialog_Imp, PlayMusicHdl, void *, EMPTYARG )
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
                                       String( SdResId( STR_PLAY ) ) );

            mbLabelPlaying = sal_False;
        }
        catch( css::lang::IllegalArgumentException )
        {
#ifdef DBG_UTIL
            DBG_ERROR( "Cannot access play button" );
#endif
        }
    }
    else
    {
        rtl::OUString aUrl( GetPath() );
        if ( aUrl.getLength() )
        {
            try
            {
                mxPlayer.set( avmedia::MediaWindow::createPlayer( aUrl ), css::uno::UNO_QUERY_THROW );
                mxPlayer->start();
                maUpdateTimer.SetTimeout( 100 );
                maUpdateTimer.Start();
            }
            catch( css::uno::Exception& e )
            {
                (void)e;
                mxPlayer.clear();
            }

            if (mxPlayer.is())
            {
                try
                {
                    mxControlAccess->setLabel( css::ui::dialogs::ExtendedFilePickerElementIds::PUSHBUTTON_PLAY,
                                               String( SdResId( STR_STOP ) ) );

                    mbLabelPlaying = sal_True;
                }
                catch( css::lang::IllegalArgumentException )
                {
#ifdef DBG_UTIL
                    DBG_ERROR( "Cannot access play button" );
#endif
                }
            }
        }
    }

    return 0;
}

// ------------------------------------------------------------------------
IMPL_LINK( SdFileDialog_Imp, IsMusicStoppedHdl, void *, EMPTYARG )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

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
                                       String( SdResId( STR_PLAY ) ) );
            mbLabelPlaying = sal_False;
        }
        catch( css::lang::IllegalArgumentException )
        {
#ifdef DBG_UTIL
            DBG_ERROR( "Cannot access play button" );
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
        String  aCurrFilter( GetCurrentFilter() );

        try
        {
            if( !aCurrFilter.Len() || ( aCurrFilter == String( SdResId( STR_EXPORT_HTML_NAME ) ) ) )
                mxControlAccess->enableControl( css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_SELECTION, sal_False );
            else
                mxControlAccess->enableControl( css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_SELECTION, sal_True );
        }
        catch( css::lang::IllegalArgumentException )
        {
#ifdef DBG_UTIL
            DBG_ERROR( "Cannot access \"selection\" checkbox" );
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
                                           String( SdResId( STR_PLAY ) ) );
            }
            catch( css::lang::IllegalArgumentException )
            {
#ifdef DBG_UTIL
                DBG_ERROR( "Cannot set play button label" );
#endif
            }
        }
        else if( mbUsableSelection != sal_True )
        {
            try
            {
                mxControlAccess->enableControl( css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_SELECTION, sal_False );
            }
            catch( css::lang::IllegalArgumentException )
            {
#ifdef DBG_UTIL
                DBG_ERROR( "Cannot disable selection checkbox" );
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
    String aDescr;
    aDescr = String(SdResId(STR_ALL_FILES));
    mpImpl->AddFilter( aDescr, UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "*.*" ) ) );

    // setup filter
#if defined UNX
    aDescr = String(SdResId(STR_AU_FILE));
    mpImpl->AddFilter( aDescr, UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "*.au;*.snd" ) ) );
    aDescr = String(SdResId(STR_VOC_FILE));
    mpImpl->AddFilter( aDescr, UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "*.voc" ) ) );
    aDescr = String(SdResId(STR_WAV_FILE));
    mpImpl->AddFilter( aDescr, UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "*.wav" ) ) );
    aDescr = String(SdResId(STR_AIFF_FILE));
    mpImpl->AddFilter( aDescr, UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "*.aiff" ) ) );
    aDescr = String(SdResId(STR_SVX_FILE));
    mpImpl->AddFilter( aDescr, UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "*.svx" ) ) );
#else
    aDescr = String(SdResId(STR_WAV_FILE));
    mpImpl->AddFilter( aDescr, UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "*.wav;*.mp3;*.ogg" ) ) );
    aDescr = String(SdResId(STR_MIDI_FILE));
    mpImpl->AddFilter( aDescr, UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "*.mid" ) ) );
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
String SdOpenSoundFileDialog::GetPath() const
{
    return mpImpl->GetPath();
}

// ------------------------------------------------------------------------
void SdOpenSoundFileDialog::SetPath( const String& rPath )
{
    mpImpl->SetDisplayDirectory( rPath );
}
