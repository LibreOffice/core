/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filedlg.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 16:26:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef  _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef  _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_COMMONFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_EXECUTABLEDIALOGRESULTS_HPP_
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_LISTBOXCONTROLACTIONS_HPP_
#include <com/sun/star/ui/dialogs/ListboxControlActions.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_EXTENDEDFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_TEMPLATEDESCRIPTION_HPP_
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILEPICKERCONTROLACCESS_HPP_
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILEPICKERLISTENER_HPP_
#include <com/sun/star/ui/dialogs/XFilePickerListener.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILEPICKERNOTIFIER_HPP_
#include <com/sun/star/ui/dialogs/XFilePickerNotifier.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILEPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#endif

#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _VOS_THREAD_HXX_
#include <vos/thread.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif

#include <svx/impgrf.hxx>

#include <avmedia/mediawindow.hxx>
#include "filedlg.hxx"
#include "sdresid.hxx"
#include "strings.hrc"



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

    friend class SdExportFileDialog;
    friend class SdOpenSoundFileDialog;

    css::uno::Reference< css::ui::dialogs::XFilePickerControlAccess >   mxControlAccess;

    css::uno::Reference< css::media::XPlayer > mxPlayer;
    ULONG                       mnPlaySoundEvent;
    BOOL                        mbUsableSelection;
    BOOL                        mbLabelPlaying;

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

    sal_Bool                    SelectionBoxState() const;
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

            mbLabelPlaying = FALSE;
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

                    mbLabelPlaying = TRUE;
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
            mbLabelPlaying = FALSE;
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
                mxControlAccess->enableControl( css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_SELECTION, FALSE );
            else
                mxControlAccess->enableControl( css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_SELECTION, TRUE );
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
    mbLabelPlaying(FALSE)
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
                mxControlAccess->enableControl( css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_SELECTION, FALSE );
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

// ------------------------------------------------------------------------
sal_Bool SdFileDialog_Imp::SelectionBoxState() const
{
    if ( !mbUsableSelection || !mxControlAccess.is() )
        return sal_False;

    sal_Bool bState(0);
    try
    {
        mxControlAccess->getValue( css::ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_SELECTION, 0 ) >>= bState;
    }
    catch( css::lang::IllegalArgumentException )
    {
#ifdef DBG_UTIL
        DBG_ERROR( "Cannot access \"selection\" checkbox" );
#endif
    }

    return bState;
}


// --------------------------------------------------------------------
// -----------      SdExportFileDialog      ---------------------------
// --------------------------------------------------------------------

// these are simple forwarders
SdExportFileDialog::SdExportFileDialog(BOOL bHaveCheckbox) :
    mpImpl( new SdFileDialog_Imp( css::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION_SELECTION,
                                  bHaveCheckbox ) )
{
    // setup filter
    const String    aHTMLFilter( SdResId( STR_EXPORT_HTML_NAME ) );
    GraphicFilter*  pFilter = GetGrfFilter();
    const USHORT    nFilterCount = pFilter->GetExportFormatCount();

    // add HTML filter
    mpImpl->AddFilter( aHTMLFilter, String( SdResId( STR_EXPORT_HTML_FILTER ) ) );

    // add other graphic filters
    for ( USHORT i = 0; i < nFilterCount; i++ )
    {
        mpImpl->AddFilter( pFilter->GetExportFormatName( i ),
                           pFilter->GetExportWildcard( i ) );
    }

    // set dialog title
    mpImpl->SetTitle( String( SdResId( STR_EXPORT_DIALOG_TITLE ) ) );
}

// ------------------------------------------------------------------------
SdExportFileDialog::~SdExportFileDialog()
{
}

// ------------------------------------------------------------------------
ErrCode SdExportFileDialog::Execute()
{
    return mpImpl->Execute();
}

String SdExportFileDialog::GetPath() const
{
    return mpImpl->GetPath();
}

// ------------------------------------------------------------------------
void SdExportFileDialog::SetPath( const String& rPath )
{
    mpImpl->SetDisplayDirectory( rPath );
}

// ------------------------------------------------------------------------
String SdExportFileDialog::ReqCurrentFilter() const
{
    return mpImpl->GetCurrentFilter();
}

// ------------------------------------------------------------------------
BOOL SdExportFileDialog::IsExportSelection() const
{
    return mpImpl->SelectionBoxState();
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
