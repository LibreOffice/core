/*************************************************************************
 *
 *  $RCSfile: filedlg.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: thb $ $Date: 2001-09-04 16:36:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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

#ifndef _SOUND_HXX //autogen
#include <vcl/sound.hxx>
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
    friend class SdExportFileDialog;
    friend class SdOpenSoundFileDialog;

    css::uno::Reference< css::ui::dialogs::XFilePickerControlAccess >   mxControlAccess;

    Sound                       maSound;
    BOOL                        mbUsableSelection;
    BOOL                        mbSoundPlaying;

    void                        CheckSelectionState();
    DECL_LINK( StopMusicHdl, void * );

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
                if( mbSoundPlaying )
                {
                    mbSoundPlaying = FALSE;

                    // reset, so that sound file gets unlocked
                    maSound.SetSoundName( String() );

                    mxControlAccess->setLabel( css::ui::dialogs::ExtendedFilePickerElementIds::PUSHBUTTON_PLAY,
                                               String( SdResId( STR_PLAY ) ) );
                }
                else
                {
                    if( maSound.IsPlaying() )
                    {
                        maSound.Stop();
                    }
                    else
                    {
                        INetURLObject   aUrl( GetPath() );
                        String          aSoundFile( aUrl.GetMainURL( INetURLObject::NO_DECODE ) );

                        if( aSoundFile.Len() > 0 )
                        {
                            mbSoundPlaying = TRUE;

                            maSound.SetNotifyHdl( LINK( this, SdFileDialog_Imp, StopMusicHdl ) );
                            maSound.SetSoundName( aSoundFile );
                            maSound.Play();

                            // guard against early stopping
                            if( maSound.IsPlaying() )
                                mxControlAccess->setLabel( css::ui::dialogs::ExtendedFilePickerElementIds::PUSHBUTTON_PLAY,
                                                           String( SdResId( STR_STOP ) ) );
                        }
                    }
                }
            }
            break;
    }
}

// ------------------------------------------------------------------------
IMPL_LINK( SdFileDialog_Imp, StopMusicHdl, void *, EMPTYARG )
{
     ::vos::OGuard aGuard( Application::GetSolarMutex() );

    mbSoundPlaying = FALSE;

    // reset, so that sound file gets unlocked
    maSound.SetSoundName( String() );

    if( mxControlAccess.is() )
    {
        try
        {
            mxControlAccess->setLabel( css::ui::dialogs::ExtendedFilePickerElementIds::PUSHBUTTON_PLAY,
                                       String( SdResId( STR_PLAY ) ) );
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
    mbUsableSelection( bUsableSelection ),
    mbSoundPlaying(FALSE)
{
    css::uno::Reference < ::com::sun::star::ui::dialogs::XFilePicker > xFileDlg = GetFilePicker();

    // get the control access
    mxControlAccess = css::uno::Reference< css::ui::dialogs::XFilePickerControlAccess > ( xFileDlg, css::uno::UNO_QUERY );

    if( mxControlAccess.is() )
    {
        if( nDialogType == FILEOPEN_PLAY )
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
    mpImpl( new SdFileDialog_Imp( FILESAVE_AUTOEXTENSION_SELECTION,
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
    mpImpl( new SdFileDialog_Imp( FILEOPEN_PLAY, sal_False ) )
{
    // setup filter
#if defined UNX
    String aDescr;
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
    String aDescr;
    aDescr = String(SdResId(STR_WAV_FILE));
    mpImpl->AddFilter( aDescr, UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "*.wav" ) ) );
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
