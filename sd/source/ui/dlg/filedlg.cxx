/*************************************************************************
 *
 *  $RCSfile: filedlg.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ka $ $Date: 2001-07-30 15:39:14 $
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

#include <svx/impgrf.hxx>

#include "filedlg.hxx"
#include "sdresid.hxx"
#include "strings.hrc"

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef  _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
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
#ifndef  _COM_SUN_STAR_UI_DIALOGS_EXTENDEDFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_FILEPREVIEWIMAGEFORMATS_HPP_
#include <com/sun/star/ui/dialogs/FilePreviewImageFormats.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_LISTBOXCONTROLACTIONS_HPP_
#include <com/sun/star/ui/dialogs/ListboxControlActions.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_TEMPLATEDESCRIPTION_HPP_
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILEPICKERCONTROLACCESS_HPP_
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILEPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILEPICKERLISTENER_HPP_
#include <com/sun/star/ui/dialogs/XFilePickerListener.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILEPICKERNOTIFIER_HPP_
#include <com/sun/star/ui/dialogs/XFilePickerNotifier.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILEPREVIEW_HPP_
#include <com/sun/star/ui/dialogs/XFilePreview.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILTERMANAGER_HPP_
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _SOUND_HXX //autogen
#include <vcl/sound.hxx>
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

#include "filedlg.hxx"


//-----------------------------------------------------------------------------

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;
using namespace ::rtl;
using namespace ::cppu;

//-----------------------------------------------------------------------------

class SdFileDialogHelper : public WeakImplHelper1< XFilePickerListener >
{
    friend class SdExportFileDialog;
    friend class SdOpenSoundFileDialog;

    Reference < XFilePicker >               mxFileDlg;
    Reference< XFilterManager >             mxFilterMgr;
    Reference< XFilePickerControlAccess >   mxControlAccess;

    Sound                       maSound;
    OUString                    maPath;
    OUString                    maCurFilter;
    BOOL                        mbUsableSelection;
    BOOL                        mbSoundPlaying;
    ErrCode                     mnError;

private:
    void                        checkSelectionState();
    void                        dispose();

    DECL_LINK( StopMusicHdl, void * );

public:
    // XFilePickerListener methods
    virtual void SAL_CALL       fileSelectionChanged( const FilePickerEvent& aEvent ) throw ( RuntimeException );
    virtual void SAL_CALL       directoryChanged( const FilePickerEvent& aEvent ) throw ( RuntimeException );
    virtual OUString SAL_CALL   helpRequested( const FilePickerEvent& aEvent ) throw ( RuntimeException );
    virtual void SAL_CALL       controlStateChanged( const FilePickerEvent& aEvent ) throw ( RuntimeException );
    virtual void SAL_CALL       dialogSizeChanged() throw ( RuntimeException );

    // XEventListener methods
    virtual void SAL_CALL       disposing( const EventObject& Source ) throw ( RuntimeException );

    // Own methods
                                SdFileDialogHelper( const short nDialogType );
                                   ~SdFileDialogHelper();

    ErrCode                     execute();

    void                        setPath( const OUString& rPath ) { maPath = rPath; }
    OUString                    getPath() const { return maPath; }

    OUString                    reqFile() const;

    void                        addFilter( const String& rFilter,
                                           const String& rType );
    sal_Bool                    selectionBoxState() const;
    sal_Bool                    extensionBoxState() const;
};

// ------------------------------------------------------------------------
// XFilePickerListener Methods
// ------------------------------------------------------------------------
void SAL_CALL SdFileDialogHelper::fileSelectionChanged( const FilePickerEvent& aEvent ) throw ( RuntimeException )
{
}

// ------------------------------------------------------------------------
void SAL_CALL SdFileDialogHelper::directoryChanged( const FilePickerEvent& aEvent ) throw ( RuntimeException )
{}

// ------------------------------------------------------------------------
OUString SAL_CALL SdFileDialogHelper::helpRequested( const FilePickerEvent& aEvent ) throw ( RuntimeException )
{
    return OUString();
}

// ------------------------------------------------------------------------
void SAL_CALL SdFileDialogHelper::controlStateChanged( const FilePickerEvent& aEvent ) throw ( RuntimeException )
{
    switch( aEvent.ElementId )
    {
        case CommonFilePickerElementIds::LISTBOX_FILTER:
            checkSelectionState();
            break;

        case ExtendedFilePickerElementIds::PUSHBUTTON_PLAY:
            if( mxFilterMgr.is() && mxControlAccess.is() )
            {
                if( mbSoundPlaying )
                {
                    mbSoundPlaying = FALSE;

                    // reset, so that sound file gets unlocked
                    maSound.SetSoundName( String() );

                    mxControlAccess->setLabel( ExtendedFilePickerElementIds::PUSHBUTTON_PLAY, String( SdResId( STR_PLAY ) ) );
                }
                else
                {
                    if( maSound.IsPlaying() )
                    {
                        maSound.Stop();
                    }
                    else
                    {
                        INetURLObject   aUrl( reqFile() );
                        String          aSoundFile( aUrl.GetMainURL( INetURLObject::NO_DECODE ) );

                        if( aSoundFile.Len() > 0 )
                        {
                            mbSoundPlaying = TRUE;

                            maSound.SetNotifyHdl( LINK( this, SdFileDialogHelper, StopMusicHdl ) );
                            maSound.SetSoundName( aSoundFile );
                            maSound.Play();

                            // guard against early stopping
                            if( maSound.IsPlaying() )
                                mxControlAccess->setLabel( ExtendedFilePickerElementIds::PUSHBUTTON_PLAY, String( SdResId( STR_STOP ) ) );
                        }
                    }
                }
            }
            break;
    }
}

// ------------------------------------------------------------------------
IMPL_LINK( SdFileDialogHelper, StopMusicHdl, void *, EMPTYARG )
{
    mbSoundPlaying = FALSE;

    // reset, so that sound file gets unlocked
    maSound.SetSoundName( String() );

    if( mxControlAccess.is() )
    {
        try
        {
            mxControlAccess->setLabel( ExtendedFilePickerElementIds::PUSHBUTTON_PLAY, String( SdResId( STR_PLAY ) ) );
        }
        catch(IllegalArgumentException)
        {
#ifdef DBG_UTIL
            DBG_ERROR( "Cannot access play button" );
#endif
        }
    }

    return( 0L );
}


// ------------------------------------------------------------------------
void SAL_CALL SdFileDialogHelper::dialogSizeChanged( ) throw ( RuntimeException )
{
}

// ------------------------------------------------------------------------
// XEventListener Methods
// ------------------------------------------------------------------------
void SAL_CALL SdFileDialogHelper::disposing( const EventObject& Source ) throw ( RuntimeException )
{
    dispose();
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
void SdFileDialogHelper::dispose()
{
    if ( mxFileDlg.is() )
    {
        // remove the event listener
        Reference< XFilePickerNotifier > xNotifier( mxFileDlg, UNO_QUERY );
        if ( xNotifier.is() )
            xNotifier->removeFilePickerListener( this );
        mxFileDlg.clear();
    }
}

// check whether to disable the "selection" checkbox
void SdFileDialogHelper::checkSelectionState()
{
    if( mbUsableSelection && mxFilterMgr.is() && mxControlAccess.is() )
    {
        String  aCurrFilter(mxFilterMgr->getCurrentFilter());

        try
        {
            if( !aCurrFilter.Len() || ( aCurrFilter == String( SdResId( STR_EXPORT_HTML_NAME ) ) ) )
                mxControlAccess->enableControl( ExtendedFilePickerElementIds::CHECKBOX_SELECTION, FALSE );
            else
                mxControlAccess->enableControl( ExtendedFilePickerElementIds::CHECKBOX_SELECTION, TRUE );
        }
        catch(IllegalArgumentException)
        {
#ifdef DBG_UTIL
            DBG_ERROR( "Cannot access \"selection\" checkbox" );
#endif
        }
    }
}

// ------------------------------------------------------------------------
// -----------      FileDialogHelper_Impl       ---------------------------
// ------------------------------------------------------------------------

SdFileDialogHelper::SdFileDialogHelper( const short  nDialogType ) :
    mnError(ERRCODE_NONE),
    mbUsableSelection(FALSE),
    mbSoundPlaying(FALSE)
{
    // create the file open dialog
    OUString aService( RTL_CONSTASCII_USTRINGPARAM( FILE_OPEN_SERVICE_NAME ) );
    Reference< XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );

    mxFileDlg = Reference < XFilePicker > ( xFactory->createInstance( aService ), UNO_QUERY );

    Reference< XFilePickerNotifier > xNotifier( mxFileDlg, UNO_QUERY );
    Reference< XInitialization > xInit( mxFileDlg, UNO_QUERY );

    if ( ! mxFileDlg.is() || ! xNotifier.is() )
    {
        mnError = ERRCODE_ABORT;
        return;
    }

    Sequence < Any > aServiceType(1);

    switch ( nDialogType )
    {
        case SDFILEDIALOG_EXPORT:
            aServiceType[0] <<= TemplateDescription::FILESAVE_AUTOEXTENSION_SELECTION;
            break;
        case SDFILEDIALOG_EXPORT_SELECTION:
            aServiceType[0] <<= TemplateDescription::FILESAVE_AUTOEXTENSION_SELECTION;
            mbUsableSelection = TRUE;
            break;
        case SDFILEDIALOG_OPEN_SOUND:
            aServiceType[0] <<= TemplateDescription::FILEOPEN_PLAY;
            break;
        default:
            aServiceType[0] <<= TemplateDescription::FILESAVE_SIMPLE;
            DBG_ERRORFILE( "SdFileDialogHelper::ctor with unknown type" );
    }

    if ( xInit.is() )
        xInit->initialize( aServiceType );

    // add the event listener
    xNotifier->addFilePickerListener( this );

    // get the filter manager
    mxFilterMgr = Reference< XFilterManager > ( mxFileDlg, UNO_QUERY );

    // get the control access
    mxControlAccess = Reference< XFilePickerControlAccess > ( mxFileDlg, UNO_QUERY );

    if( mxControlAccess.is() )
    {
        switch( nDialogType )
        {
            case SDFILEDIALOG_OPEN_SOUND:
                try
                {
                    mxControlAccess->setLabel( ExtendedFilePickerElementIds::PUSHBUTTON_PLAY, String( SdResId( STR_PLAY ) ) );
                }
                catch(IllegalArgumentException)
                {
#ifdef DBG_UTIL
                    DBG_ERROR( "Cannot set play button label" );
#endif
                }
                break;

            case SDFILEDIALOG_EXPORT:
                try
                {
                    mxControlAccess->enableControl( ExtendedFilePickerElementIds::CHECKBOX_SELECTION, FALSE );
                }
                catch(IllegalArgumentException)
                {
#ifdef DBG_UTIL
                    DBG_ERROR( "Cannot disable selection checkbox" );
#endif
                }
                break;
        }
    }
}


// ------------------------------------------------------------------------
SdFileDialogHelper::~SdFileDialogHelper()
{
}

// ------------------------------------------------------------------------
OUString SdFileDialogHelper::reqFile() const
{
    if ( mxFileDlg.is() )
    {
        Sequence < OUString > aPathSeq = mxFileDlg->getFiles();

        if ( aPathSeq.getLength() == 1 )
            return String( aPathSeq[0] );
    }

    return String();
}

// ------------------------------------------------------------------------
ErrCode SdFileDialogHelper::execute()
{
    if ( ! mxFileDlg.is() || !mxFilterMgr.is() )
        return ERRCODE_ABORT;

    if ( maPath.getLength() )
    {
        INetURLObject aObj( maPath, INET_PROT_FILE );
        OUString aTitle( aObj.getName( INetURLObject::LAST_SEGMENT, true,
                         INetURLObject::DECODE_WITH_CHARSET ) );
        aObj.removeSegment();
        try
        {
            mxFileDlg->setDisplayDirectory( aObj.GetMainURL( INetURLObject::NO_DECODE ) );
        }
        catch(IllegalArgumentException)
        {
#ifdef DBG_UTIL
            DBG_ERROR( "Cannot set display directory" );
#endif
        }
        mxFileDlg->setDefaultName( aTitle );
    }

    if ( maCurFilter.getLength() )
    {
        try
        {
            mxFilterMgr->setCurrentFilter( maCurFilter );
        }
        catch(IllegalArgumentException)
        {
#ifdef DBG_UTIL
            DBG_ERROR( "Cannot set current filter" );
#endif
        }
    }

    checkSelectionState();

    // show the dialog
    sal_Int16 nRet = mxFileDlg->execute();

    maPath = mxFileDlg->getDisplayDirectory();
    maCurFilter = mxFilterMgr->getCurrentFilter();

    if ( nRet == ExecutableDialogResults::CANCEL )
        return ERRCODE_ABORT;
    else
    {
        return ERRCODE_NONE;
    }
}

// ------------------------------------------------------------------------
void SdFileDialogHelper::addFilter( const String& rFilter,
                                    const String& rType     )
{
    // set the filter
    if( mxFilterMgr.is() )
    {
        try
        {
            mxFilterMgr->appendFilter( rFilter, rType );
        }
        catch(IllegalArgumentException)
        {
#ifdef DBG_UTIL
            DBG_ERROR( "Cannot append filter" );
#endif
        }
    }
}

sal_Bool SdFileDialogHelper::selectionBoxState() const
{
    if ( !mbUsableSelection || !mxFileDlg.is() || !mxFilterMgr.is() )
        return sal_False;

    sal_Bool bState(0);
    try
    {
        mxControlAccess->getValue(ExtendedFilePickerElementIds::CHECKBOX_SELECTION, 0) >>= bState;
    }
    catch(IllegalArgumentException)
    {
#ifdef DBG_UTIL
        DBG_ERROR( "Cannot access \"selection\" checkbox" );
#endif
    }

    return bState;
}

sal_Bool SdFileDialogHelper::extensionBoxState() const
{
    if ( !mxFileDlg.is() || !mxFilterMgr.is() )
        return sal_False;

    sal_Bool bState(0);
    try
    {
        mxControlAccess->getValue(ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION, 0) >>= bState;
    }
    catch(IllegalArgumentException)
    {
#ifdef DBG_UTIL
        DBG_ERROR( "Cannot access \"auto extension\" checkbox" );
#endif
    }

    return bState;
}


// ------------------------------------------------------------------------


SdExportFileDialog::SdExportFileDialog(BOOL haveCheckbox)
{
    if( haveCheckbox )
        mpImp = new SdFileDialogHelper( SDFILEDIALOG_EXPORT_SELECTION );
    else
        mpImp = new SdFileDialogHelper( SDFILEDIALOG_EXPORT );

    mxImp = mpImp;

    // setup filter
    const String    aHTMLFilter( SdResId( STR_EXPORT_HTML_NAME ) );
    GraphicFilter*  pFilter = GetGrfFilter();
    const USHORT    nFilterCount = pFilter->GetExportFormatCount();

    // add HTML filter
    mpImp->addFilter( aHTMLFilter, String( SdResId( STR_EXPORT_HTML_FILTER ) ) );

    // add other graphic filters
    for ( USHORT i = 0; i < nFilterCount; i++ )
    {
        mpImp->addFilter( pFilter->GetExportFormatName( i ),
                          pFilter->GetExportWildcard( i ) );
    }

    // set dialog title
    if ( mpImp->mxFileDlg.is() )
        mpImp->mxFileDlg->setTitle( String( SdResId( STR_EXPORT_DIALOG_TITLE ) ) );
}

// ------------------------------------------------------------------------
SdExportFileDialog::~SdExportFileDialog()
{
    mpImp->dispose();
    mxImp.clear();
}

// ------------------------------------------------------------------------
ErrCode SdExportFileDialog::Execute()
{
    return mpImp->execute();
}

String SdExportFileDialog::GetPath() const
{
    if ( mpImp->mxFileDlg.is() )
    {
        Sequence < OUString > aPathSeq = mpImp->mxFileDlg->getFiles();

        if ( aPathSeq.getLength() == 1 )
            return String( aPathSeq[0] );
    }

    return String();
}

// ------------------------------------------------------------------------
void SdExportFileDialog::SetPath( const String& rPath )
{
    mpImp->setPath( rPath );
}

// ------------------------------------------------------------------------
String SdExportFileDialog::ReqDisplayDirectory() const
{
    return mpImp->getPath();
}

// ------------------------------------------------------------------------
String SdExportFileDialog::ReqCurrFilter() const
{
    return mpImp ? (String)mpImp->maCurFilter : String();
}

// ------------------------------------------------------------------------
BOOL SdExportFileDialog::IsSelectedBoxChecked() const
{
    if( mpImp )
        return mpImp->selectionBoxState();

    return FALSE;
}

// ------------------------------------------------------------------------
BOOL SdExportFileDialog::IsExtensionBoxChecked() const
{
    if( mpImp )
        return mpImp->extensionBoxState();

    return FALSE;
}



// ------------------------------------------------------------------------


SdOpenSoundFileDialog::SdOpenSoundFileDialog()
{
    mpImp = new SdFileDialogHelper( SDFILEDIALOG_OPEN_SOUND );
    mxImp = mpImp;

    // setup filter
#if defined UNX
    String aDescr;
    aDescr = String(SdResId(STR_AU_FILE));
    mpImp->addFilter( aDescr, UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "*.au;*.snd" ) ) );
    aDescr = String(SdResId(STR_VOC_FILE));
    mpImp->addFilter( aDescr, UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "*.voc" ) ) );
    aDescr = String(SdResId(STR_WAV_FILE));
    mpImp->addFilter( aDescr, UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "*.wav" ) ) );
    aDescr = String(SdResId(STR_AIFF_FILE));
    mpImp->addFilter( aDescr, UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "*.aiff" ) ) );
    aDescr = String(SdResId(STR_SVX_FILE));
    mpImp->addFilter( aDescr, UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "*.svx" ) ) );
#else
    String aDescr;
    aDescr = String(SdResId(STR_WAV_FILE));
    mpImp->addFilter( aDescr, UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "*.wav" ) ) );
    aDescr = String(SdResId(STR_MIDI_FILE));
    mpImp->addFilter( aDescr, UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "*.mid" ) ) );
#endif
}

// ------------------------------------------------------------------------
SdOpenSoundFileDialog::~SdOpenSoundFileDialog()
{
    mpImp->dispose();
    mxImp.clear();
}

// ------------------------------------------------------------------------
ErrCode SdOpenSoundFileDialog::Execute()
{
    return mpImp->execute();
}

String SdOpenSoundFileDialog::GetPath() const
{
    if ( mpImp->mxFileDlg.is() )
    {
        Sequence < OUString > aPathSeq = mpImp->mxFileDlg->getFiles();

        if ( aPathSeq.getLength() == 1 )
            return String( aPathSeq[0] );
    }

    return String();
}

// ------------------------------------------------------------------------
void SdOpenSoundFileDialog::SetPath( const String& rPath )
{
    mpImp->setPath( rPath );
}

// ------------------------------------------------------------------------
String SdOpenSoundFileDialog::ReqDisplayDirectory() const
{
    return mpImp->getPath();
}
