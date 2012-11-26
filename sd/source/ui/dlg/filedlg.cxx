/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    bool                        mbUsableSelection;
    bool                        mbLabelPlaying;

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

            mbLabelPlaying = false;
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

                    mbLabelPlaying = true;
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
            mbLabelPlaying = false;
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
                mxControlAccess->enableControl( css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_SELECTION, false );
            else
                mxControlAccess->enableControl( css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_SELECTION, true );
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
    mbLabelPlaying(false)
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
                mxControlAccess->enableControl( css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_SELECTION, false );
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
