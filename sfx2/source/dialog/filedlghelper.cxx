/*************************************************************************
 *
 *  $RCSfile: filedlghelper.cxx,v $
 *
 *  $Revision: 1.50 $
 *
 *  last change: $Author: mba $ $Date: 2001-09-06 08:47:58 $
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


#include <sal/types.h>

#include <list>

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

#include "filedlghelper.hxx"

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _SV_HELP_HXX
#include <vcl/help.hxx>
#endif

#include <unotools/ucbstreamhelper.hxx>
#include <unotools/ucbhelper.hxx>
#include <unotools/localfilehelper.hxx>

#include <vcl/cvtgrf.hxx>

#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>
#ifndef _FILTER_HXX
#include <svtools/filter.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_VIEWOPTIONS_HXX
#include <svtools/viewoptions.hxx>
#endif
#ifndef _SVT_HELPID_HRC
#include <svtools/helpid.hrc>
#endif

#ifndef _SFXAPP_HXX
#include "app.hxx"
#endif
#ifndef _SFXDOCFILE_HXX
#include "docfile.hxx"
#endif
#ifndef _SFX_OBJFAC_HXX
#include "docfac.hxx"
#endif
#ifndef _SFX_FCONTNR_HXX
#include "fcontnr.hxx"
#endif
#ifndef _SFX_OPENFLAG_HXX
#include "openflag.hxx"
#endif
#ifndef _SFX_PASSWD_HXX
#include <passwd.hxx>
#endif

#include "sfxresid.hxx"
#include "sfxsids.hrc"
#include "explorer.hrc"
#include "filedlghelper.hrc"

//-----------------------------------------------------------------------------

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;
using namespace ::rtl;
using namespace ::cppu;

//-----------------------------------------------------------------------------

#define IODLG_CONFIGNAME        String(DEFINE_CONST_UNICODE("FilePicker_Save"))
#define IMPGRF_CONFIGNAME       String(DEFINE_CONST_UNICODE("FilePicker_Graph"))

//-----------------------------------------------------------------------------

namespace sfx2 {

String EncodeSpaces_Impl( const String& rSource );
String DecodeSpaces_Impl( const String& rSource );

// struct FilterEntry_Impl -----------------------------------------------

struct FilterEntry_Impl
{
    OUString            maTitle;
    OUString            maFilter;

    FilterEntry_Impl( const OUString& rTitle, const OUString& rFilter ) :
        maTitle( rTitle ), maFilter( rFilter ) {}
};

typedef std::list < FilterEntry_Impl > FilterList;

// ------------------------------------------------------------------------
class FileDialogHelper_Impl : public WeakImplHelper1< XFilePickerListener >
{
    friend class FileDialogHelper;

    Reference < XFilePicker >   mxFileDlg;

    SfxFilterMatcher       *mpMatcher;
    GraphicFilter          *mpGraphicFilter;

    OUString                maPath;
    OUString                maCurFilter;
    OUString                maSelectFilter;
    Timer                   maPreViewTimer;
    Graphic                 maGraphic;

    ErrCode                 mnError;
    sal_Bool                mbHasPassword   : 1;
    sal_Bool                mbIsPwdEnabled  : 1;
    sal_Bool                mbHasVersions   : 1;
    sal_Bool                mbHasAutoExt    : 1;
    sal_Bool                mbHasLink       : 1;
    sal_Bool                mbHasPreview    : 1;
    sal_Bool                mbShowPreview   : 1;
    sal_Bool                mbIsSaveDlg     : 1;

    sal_Bool                mbDeleteMatcher : 1;
    sal_Bool                mbInsert        : 1;

private:
    void                    addFilters( sal_uInt32 nFlags,
                                        const SfxObjectFactory& rFactory );
    void                    addFilter( const OUString& rFilterName,
                                       const OUString& rExtension );
    void                    addGraphicFilter();
    void                    enablePasswordBox();
    void                    updateVersions();
    void                    dispose();

    void                    loadConfig();
    void                    saveConfig();

    ErrCode                 getGraphic( const OUString& rURL, Graphic& rGraphic ) const;
    void                    setDefaultValues();

    DECL_LINK( TimeOutHdl_Impl, Timer* );

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
                                FileDialogHelper_Impl( const short nDialogType, sal_uInt32 nFlags );
                               ~FileDialogHelper_Impl();

    ErrCode                 execute( SvStringsDtor*& rpURLList,
                                     SfxItemSet *&   rpSet,
                                     String&         rFilter );
    ErrCode                 execute();

    void                    setPath( const OUString& rPath );
    void                    setFilter( const OUString& rFilter );

    OUString                getPath() const;
    OUString                getFilter() const;
    OUString                getRealFilter() const;

    ErrCode                 getGraphic( Graphic& rGraphic ) const;
};

// ------------------------------------------------------------------------
// -----------      FileDialogHelper_Impl       ---------------------------
// ------------------------------------------------------------------------

// ------------------------------------------------------------------------
// XFilePickerListener Methods
// ------------------------------------------------------------------------
void SAL_CALL FileDialogHelper_Impl::fileSelectionChanged( const FilePickerEvent& aEvent ) throw ( RuntimeException )
{
    if ( mbHasVersions )
        updateVersions();

    if ( mbShowPreview )
        maPreViewTimer.Start();
}

// ------------------------------------------------------------------------
void SAL_CALL FileDialogHelper_Impl::directoryChanged( const FilePickerEvent& aEvent ) throw ( RuntimeException )
{
    if ( mbShowPreview )
        TimeOutHdl_Impl( NULL );
}

// ------------------------------------------------------------------------
OUString SAL_CALL FileDialogHelper_Impl::helpRequested( const FilePickerEvent& aEvent ) throw ( RuntimeException )
{
    //!!! todo: cache the help strings (here or TRA)

    ULONG nHelpId = 0;
    // mapping frrom element id -> help id
    switch ( aEvent.ElementId )
    {
        case ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION :
            nHelpId = HID_FILESAVE_AUTOEXTENSION;
            break;

        case ExtendedFilePickerElementIds::CHECKBOX_PASSWORD :
            nHelpId = HID_FILESAVE_SAVEWITHPASSWORD;
            break;

        case ExtendedFilePickerElementIds::CHECKBOX_FILTEROPTIONS :
            nHelpId = HID_FILESAVE_CUSTOMIZEFILTER;
            break;

        case ExtendedFilePickerElementIds::CHECKBOX_READONLY :
            nHelpId = HID_FILEOPEN_READONLY;
            break;

        case ExtendedFilePickerElementIds::CHECKBOX_LINK :
            nHelpId = HID_FILEDLG_LINK_CB;
            break;

        case ExtendedFilePickerElementIds::CHECKBOX_PREVIEW :
            nHelpId = HID_FILEDLG_PREVIEW_CB;
            break;

        case ExtendedFilePickerElementIds::PUSHBUTTON_PLAY :
            nHelpId = HID_FILESAVE_DOPLAY;
            break;

        case ExtendedFilePickerElementIds::LISTBOX_VERSION :
            nHelpId = HID_FILEOPEN_VERSION;
            break;

        case ExtendedFilePickerElementIds::LISTBOX_TEMPLATE :
            nHelpId = HID_FILESAVE_TEMPLATE;
            break;

        case ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE :
            nHelpId = HID_FILEOPEN_IMAGE_TEMPLATE;
            break;

        case ExtendedFilePickerElementIds::CHECKBOX_SELECTION :
            nHelpId = HID_FILESAVE_SELECTION;
            break;

        default:
            DBG_ERRORFILE( "invalid element id" );
    }

    OUString aHelpText;
    Help* pHelp = Application::GetHelp();
    if ( pHelp )
        aHelpText = String( pHelp->GetHelpText( nHelpId, NULL ) );
    return aHelpText;
}

// ------------------------------------------------------------------------
void SAL_CALL FileDialogHelper_Impl::controlStateChanged( const FilePickerEvent& aEvent ) throw ( RuntimeException )
{
    if ( ( aEvent.ElementId == CommonFilePickerElementIds::LISTBOX_FILTER ) && mbHasPassword )
        enablePasswordBox();
    if ( ( aEvent.ElementId == ExtendedFilePickerElementIds::CHECKBOX_PREVIEW ) && mbHasPreview )
    {
        Reference< XFilePickerControlAccess > xCtrlAccess( mxFileDlg, UNO_QUERY );

        // check, wether or not we have to display a preview
        if ( xCtrlAccess.is() )
        {
            try
            {
                Any aValue = xCtrlAccess->getValue( ExtendedFilePickerElementIds::CHECKBOX_PREVIEW, 0 );
                sal_Bool bShowPreview = sal_False;

                if ( aValue >>= bShowPreview )
                {
                    mbShowPreview = bShowPreview;
                    TimeOutHdl_Impl( NULL );
                }
            }
            catch( IllegalArgumentException ){}
        }
    }
}

// ------------------------------------------------------------------------
void SAL_CALL FileDialogHelper_Impl::dialogSizeChanged() throw ( RuntimeException )
{
    if ( mbShowPreview )
        TimeOutHdl_Impl( NULL );
}

// ------------------------------------------------------------------------
// XEventListener Methods
// ------------------------------------------------------------------------
void SAL_CALL FileDialogHelper_Impl::disposing( const EventObject& Source ) throw ( RuntimeException )
{
    dispose();
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
void FileDialogHelper_Impl::dispose()
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

// ------------------------------------------------------------------------
void FileDialogHelper_Impl::enablePasswordBox()
{
    if ( ! mbHasPassword )
        return;

    Reference< XFilterManager > xFltMgr( mxFileDlg, UNO_QUERY );
    OUString aFilterName = xFltMgr->getCurrentFilter();

    mbIsPwdEnabled = sal_False;

    if ( mpMatcher )
    {
        const SfxFilter* pFilter = mpMatcher->GetFilter4UIName(
                            aFilterName, 0, SFX_FILTER_NOTINFILEDLG );

        BOOL bEnablePasswd = pFilter &&
                             ( SOFFICE_FILEFORMAT_60 <= pFilter->GetVersion() );

        Reference < XFilePickerControlAccess > xCtrlAccess( mxFileDlg, UNO_QUERY );

        try
        {
            xCtrlAccess->enableControl( ExtendedFilePickerElementIds::CHECKBOX_PASSWORD, bEnablePasswd );
            mbIsPwdEnabled = bEnablePasswd;
        }
        catch( IllegalArgumentException ){}
    }
}

// ------------------------------------------------------------------------
void FileDialogHelper_Impl::updateVersions()
{
    Sequence < OUString > aEntries;
    Sequence < OUString > aPathSeq = mxFileDlg->getFiles();

    if ( aPathSeq.getLength() == 1 )
    {
        INetURLObject aObj( aPathSeq[0] );

        if ( ( aObj.GetProtocol() == INET_PROT_FILE ) &&
            ( utl::UCBContentHelper::IsDocument( aObj.GetMainURL( INetURLObject::NO_DECODE ) ) ) )
        {
            SfxMedium aMed( aObj.GetMainURL( INetURLObject::NO_DECODE ),
                            SFX_STREAM_READONLY_MAKECOPY, TRUE );
            const SfxVersionTableDtor* pVerTable = aMed.GetVersionList();

            if ( pVerTable )
            {
                SvStringsDtor* pVersions = pVerTable->GetVersions();

                aEntries.realloc( pVersions->Count() + 1 );
                aEntries[0] = OUString( String ( SfxResId( STR_SFX_FILEDLG_ACTUALVERSION ) ) );

                for ( USHORT i = 0; i < pVersions->Count(); i++ )
                    aEntries[ i + 1 ] = OUString( *(pVersions->GetObject(i)) );

                delete pVersions;
            }
            else if ( aMed.GetStorage() )
            {
                SfxFilterFlags nMust = SFX_FILTER_IMPORT | SFX_FILTER_OWN, nDont = SFX_FILTER_NOTINSTALLED | SFX_FILTER_STARONEFILTER;
                if ( SFX_APP()->GetFilterMatcher().GetFilter4ClipBoardId( aMed.GetStorage()->GetFormat(), nMust, nDont ) )
                {
                    aEntries.realloc( 1 );
                    aEntries[0] = OUString( String ( SfxResId( STR_SFX_FILEDLG_ACTUALVERSION ) ) );
                }
            }
        }
    }

    Reference < XFilePickerControlAccess > xDlg( mxFileDlg, UNO_QUERY );
    Any aValue;

    try
    {
        xDlg->setValue( ExtendedFilePickerElementIds::LISTBOX_VERSION,
                        ListboxControlActions::DELETE_ITEMS, aValue );
    }
    catch( IllegalArgumentException ){}

    sal_Int32 nCount = aEntries.getLength();

    if ( nCount )
    {
        try
        {
            aValue <<= aEntries;
            xDlg->setValue( ExtendedFilePickerElementIds::LISTBOX_VERSION,
                            ListboxControlActions::ADD_ITEMS, aValue );

            Any aPos;
            aPos <<= (sal_Int32) 0;
            xDlg->setValue( ExtendedFilePickerElementIds::LISTBOX_VERSION,
                            ListboxControlActions::SET_SELECT_ITEM, aPos );
        }
        catch( IllegalArgumentException ){}
    }
}


// -----------------------------------------------------------------------
IMPL_LINK( FileDialogHelper_Impl, TimeOutHdl_Impl, Timer*, EMPTYARG )
{
    if ( !mbHasPreview )
        return 0;

    maGraphic.Clear();

    Any aAny;
    Reference < XFilePreview > xFilePicker( mxFileDlg, UNO_QUERY );

    if ( ! xFilePicker.is() )
        return 0;

    Sequence < OUString > aPathSeq = mxFileDlg->getFiles();

    if ( mbShowPreview && ( aPathSeq.getLength() == 1 ) )
    {
        OUString    aURL = aPathSeq[0];

        if ( ERRCODE_NONE == getGraphic( aURL, maGraphic ) )
        {
            Bitmap aBmp = maGraphic.GetBitmap();

            // scale the bitmap to the correct size
            sal_Int32 nOutWidth  = xFilePicker->getAvailableWidth();
            sal_Int32 nOutHeight = xFilePicker->getAvailableHeight();
            sal_Int32 nBmpWidth  = aBmp.GetSizePixel().Width();
            sal_Int32 nBmpHeight = aBmp.GetSizePixel().Height();

            double nXRatio = (double) nOutWidth / nBmpWidth;
            double nYRatio = (double) nOutHeight / nBmpHeight;

            if ( nXRatio < nYRatio )
                aBmp.Scale( nXRatio, nXRatio );
            else
                aBmp.Scale( nYRatio, nYRatio );

            nBmpWidth  = aBmp.GetSizePixel().Width();
            nBmpHeight = aBmp.GetSizePixel().Height();

            sal_Int32 nMidX = ( nOutWidth - nBmpWidth ) / 2;
            sal_Int32 nMidY = ( nOutHeight - nBmpHeight ) / 2;

            Rectangle aSrcRect( 0, 0, nBmpWidth, nBmpHeight );
            Rectangle aDstRect( nMidX, nMidY, nMidX + nBmpWidth, nMidY + nBmpHeight );

            Bitmap aScaledBmp( Size( nOutWidth, nOutHeight ), aBmp.GetBitCount() );
            aScaledBmp.Erase( Color( COL_WHITE ) );
            aScaledBmp.CopyPixel( aDstRect, aSrcRect, &aBmp );

            // and copy it into the Any
            SvMemoryStream aData;

            aData << aScaledBmp;

            Sequence < sal_Int8 > aBuffer( (sal_Int8*) aData.GetData(), aData.GetSize() );

            aAny <<= aBuffer;
        }
    }

    try
    {
        // clear the preview window
        xFilePicker->setImage( FilePreviewImageFormats::BITMAP, aAny );
    }
    catch( IllegalArgumentException ){}

    return 0;
}

// ------------------------------------------------------------------------
ErrCode FileDialogHelper_Impl::getGraphic( const OUString& rURL,
                                           Graphic& rGraphic ) const
{
    if ( utl::UCBContentHelper::IsFolder( rURL ) )
        return ERRCODE_IO_NOTAFILE;

    if ( !mpGraphicFilter )
        return ERRCODE_IO_NOTSUPPORTED;

    // select graphic filter from dialog filter selection
    OUString aCurFilter( getFilter() );

    sal_uInt16 nFilter = aCurFilter.getLength() && mpGraphicFilter->GetImportFormatCount()
                    ? mpGraphicFilter->GetImportFormatNumber( aCurFilter )
                    : GRFILTER_FORMAT_DONTKNOW;

    INetURLObject aURLObj( rURL );

    if ( aURLObj.HasError() || INET_PROT_NOT_VALID == aURLObj.GetProtocol() )
    {
        aURLObj.SetSmartProtocol( INET_PROT_FILE );
        aURLObj.SetSmartURL( rURL );
    }

    ErrCode nRet = ERRCODE_NONE;

    sal_uInt32 nFilterImportFlags = GRFILTER_I_FLAGS_SET_LOGSIZE_FOR_JPEG;

    // non-local?
    if ( INET_PROT_FILE != aURLObj.GetProtocol() )
    {
        SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( rURL, STREAM_READ );

        if( pStream )
            nRet = mpGraphicFilter->ImportGraphic( rGraphic, rURL, *pStream, nFilter, NULL, nFilterImportFlags );
        else
            nRet = mpGraphicFilter->ImportGraphic( rGraphic, aURLObj, nFilter, NULL, nFilterImportFlags );

        delete pStream;
    }
    else
    {
        nRet = mpGraphicFilter->ImportGraphic( rGraphic, aURLObj, nFilter, NULL, nFilterImportFlags );
    }

    return nRet;
}

// ------------------------------------------------------------------------
ErrCode FileDialogHelper_Impl::getGraphic( Graphic& rGraphic ) const
{
    ErrCode nRet = ERRCODE_NONE;

    if ( ! maGraphic )
    {
        OUString aPath;;
        Sequence < OUString > aPathSeq = mxFileDlg->getFiles();

        if ( aPathSeq.getLength() == 1 )
        {
            aPath = aPathSeq[0];
        }

        if ( aPath.getLength() )
            nRet = getGraphic( aPath, rGraphic );
        else
            nRet = ERRCODE_IO_GENERAL;
    }
    else
        rGraphic = maGraphic;

    return nRet;
}

// ------------------------------------------------------------------------
// -----------      FileDialogHelper_Impl       ---------------------------
// ------------------------------------------------------------------------

FileDialogHelper_Impl::FileDialogHelper_Impl( const short nDialogType,
                                              sal_uInt32 nFlags )
{
    OUString aService( RTL_CONSTASCII_USTRINGPARAM( FILE_OPEN_SERVICE_NAME ) );

    Reference< XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );

    // create the file open dialog
    // the flags can be SFXWB_INSERT or SFXWB_MULTISELECTION

    mnError         = ERRCODE_NONE;
    mbHasAutoExt    = sal_False;
    mbHasPassword   = sal_False;
    mbIsPwdEnabled  = sal_True;
    mbHasVersions   = sal_False;
    mbHasPreview    = sal_False;
    mbShowPreview   = sal_False;
    mbHasLink       = sal_False;
    mbDeleteMatcher = sal_False;
    mbInsert        = SFXWB_INSERT == ( nFlags & SFXWB_INSERT );
    mbIsSaveDlg     = sal_False;

    mpMatcher = NULL;
    mpGraphicFilter = NULL;

    mxFileDlg = Reference < XFilePicker > ( xFactory->createInstance( aService ), UNO_QUERY );

    Reference< XFilePickerNotifier > xNotifier( mxFileDlg, UNO_QUERY );
    Reference< XInitialization > xInit( mxFileDlg, UNO_QUERY );

    if ( ! mxFileDlg.is() || ! xNotifier.is() )
    {
        mnError = ERRCODE_ABORT;
        return;
    }

    Sequence < Any > aServiceType(1);

    switch ( nDialogType ) {
    case FILEOPEN_SIMPLE:
        aServiceType[0] <<= TemplateDescription::FILEOPEN_SIMPLE;
        break;
    case FILESAVE_SIMPLE:
        aServiceType[0] <<= TemplateDescription::FILESAVE_SIMPLE;
        mbIsSaveDlg = sal_True;
        break;
    case FILESAVE_AUTOEXTENSION_PASSWORD:
        aServiceType[0] <<= TemplateDescription::FILESAVE_AUTOEXTENSION_PASSWORD;
        mbHasPassword = sal_True;
        mbHasAutoExt = sal_True;
        mbIsSaveDlg = sal_True;
        break;
    case FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS:
        aServiceType[0] <<= TemplateDescription::FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS;
        mbHasPassword = sal_True;
        mbHasAutoExt = sal_True;
        mbIsSaveDlg = sal_True;
        break;
    case FILESAVE_AUTOEXTENSION_SELECTION:
        aServiceType[0] <<= TemplateDescription::FILESAVE_AUTOEXTENSION_SELECTION;
        mbIsSaveDlg = sal_True;
        break;
    case FILESAVE_AUTOEXTENSION_TEMPLATE:
        aServiceType[0] <<= TemplateDescription::FILESAVE_AUTOEXTENSION_TEMPLATE;
        mbIsSaveDlg = sal_True;
        break;
    case FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE:
        aServiceType[0] <<= TemplateDescription::FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE;
        mbHasPreview = sal_True;
        mbHasLink = sal_True;

        // aPreviewTimer
          maPreViewTimer.SetTimeout( 500 );
        maPreViewTimer.SetTimeoutHdl( LINK( this, FileDialogHelper_Impl, TimeOutHdl_Impl ) );

        break;

    case FILEOPEN_PLAY:
        aServiceType[0] <<= TemplateDescription::FILEOPEN_PLAY;
        break;
    case FILEOPEN_READONLY_VERSION:
        aServiceType[0] <<= TemplateDescription::FILEOPEN_READONLY_VERSION;
        mbHasVersions = sal_True;
        break;
    case FILEOPEN_LINK_PREVIEW:
        aServiceType[0] <<= TemplateDescription::FILEOPEN_LINK_PREVIEW;
        mbHasPreview = sal_True;
        mbHasLink = sal_True;

        // aPreviewTimer
          maPreViewTimer.SetTimeout( 500 );
        maPreViewTimer.SetTimeoutHdl( LINK( this, FileDialogHelper_Impl, TimeOutHdl_Impl ) );

        break;

    default:
        aServiceType[0] <<= TemplateDescription::FILEOPEN_SIMPLE;
        DBG_ERRORFILE( "FileDialogHelper::ctor with unknown type" );
    }

    if ( xInit.is() )
        xInit->initialize( aServiceType );

    // set multiselection mode
    if ( nFlags & SFXWB_MULTISELECTION )
        mxFileDlg->setMultiSelectionMode( sal_True );

    if ( mbHasLink )        // generate graphic filter only on demand
        addGraphicFilter();


    // the "insert file" dialog needs another title
    if ( mbInsert )
    {
        mxFileDlg->setTitle( OUString( String( SfxResId( STR_SFX_EXPLORERFILE_INSERT ) ) ) );

        Reference < XFilePickerControlAccess > xExtDlg( mxFileDlg, UNO_QUERY );
        if ( xExtDlg.is() )
        {
            try
            {
                xExtDlg->setLabel( CommonFilePickerElementIds::PUSHBUTTON_OK,
                                   OUString( String( SfxResId( STR_SFX_EXPLORERFILE_BUTTONINSERT ) ) ) );
            }
            catch( IllegalArgumentException ){}
        }
    }

    // add the event listener
    xNotifier->addFilePickerListener( this );
}

// ------------------------------------------------------------------------
FileDialogHelper_Impl::~FileDialogHelper_Impl()
{
    delete mpGraphicFilter;

    if ( mbDeleteMatcher )
        delete mpMatcher;

    maPreViewTimer.SetTimeoutHdl( Link() );
}

// ------------------------------------------------------------------------
ErrCode FileDialogHelper_Impl::execute( SvStringsDtor*& rpURLList,
                                        SfxItemSet *&   rpSet,
                                        String&         rFilter )
{
    rpSet = NULL;
    rpURLList = NULL;

    if ( ! mxFileDlg.is() )
        return ERRCODE_ABORT;

    loadConfig();
    setDefaultValues();
    enablePasswordBox();

    // show the dialog
    sal_Int16 nRet = mxFileDlg->execute();

    saveConfig();

    if ( nRet != ExecutableDialogResults::CANCEL )
    {

        // create an itemset
        rpSet = new SfxAllItemSet( SFX_APP()->GetPool() );

        Reference< XFilePickerControlAccess > xCtrlAccess( mxFileDlg, UNO_QUERY );

        // check, wether or not we have to display a password box
        if ( mbHasPassword && mbIsPwdEnabled && xCtrlAccess.is() )
        {
            try
            {
                Any aValue = xCtrlAccess->getValue( ExtendedFilePickerElementIds::CHECKBOX_PASSWORD, 0 );
                sal_Bool bPassWord = sal_False;
                if ( ( aValue >>= bPassWord ) && bPassWord )
                {
                    // ask for the password
                    SfxPasswordDialog aPasswordDlg( NULL );
                    aPasswordDlg.ShowExtras( SHOWEXTRAS_CONFIRM );
                    BOOL bOK = FALSE;
                    short nRet = aPasswordDlg.Execute();
                    if ( RET_OK == nRet )
                    {
                        String aPasswd = aPasswordDlg.GetPassword();
                        rpSet->Put( SfxStringItem( SID_PASSWORD, aPasswd ) );
                    }
                    else
                        return ERRCODE_ABORT;
                }
            }
            catch( IllegalArgumentException ){}
        }

        // set the read-only flag. When inserting a file, this flag is always set
        if ( mbInsert )
            rpSet->Put( SfxBoolItem( SID_DOC_READONLY, sal_True ) );
        else
        {
            if ( xCtrlAccess.is() )
            {
                try
                {
                    Any aValue = xCtrlAccess->getValue( ExtendedFilePickerElementIds::CHECKBOX_READONLY, 0 );
                    sal_Bool bReadOnly = sal_False;
                    if ( ( aValue >>= bReadOnly ) && bReadOnly )
                        rpSet->Put( SfxBoolItem( SID_DOC_READONLY, bReadOnly ) );
                }
                catch( IllegalArgumentException ){}
            }
        }
        if ( mbHasVersions && xCtrlAccess.is() )
        {
            try
            {
                Any aValue = xCtrlAccess->getValue( ExtendedFilePickerElementIds::LISTBOX_VERSION,
                                                    ListboxControlActions::GET_SELECTED_ITEM );
                sal_Int16 nVersion = 0;
                if ( aValue >>= nVersion )
                    rpSet->Put( SfxInt16Item( SID_VERSION, nVersion ) );
            }
            catch( IllegalArgumentException ){}
        }

        // set the filter
        rFilter = getRealFilter();

        // fill the rpURLList
        Sequence < OUString > aPathSeq = mxFileDlg->getFiles();

        if ( aPathSeq.getLength() )
        {
            rpURLList = new SvStringsDtor;

            if ( aPathSeq.getLength() == 1 )
            {
                OUString aFileURL( aPathSeq[0] );

                String* pURL = new String( aFileURL );
                rpURLList->Insert( pURL, 0 );
            }
            else
            {
                INetURLObject aPath( aPathSeq[0] );
                aPath.setFinalSlash();

                for ( USHORT i = 1; i < aPathSeq.getLength(); ++i )
                {
                    if ( i == 1 )
                        aPath.Append( aPathSeq[i] );
                    else
                        aPath.setName( aPathSeq[i] );

                    String* pURL = new String( aPath.GetMainURL( INetURLObject::NO_DECODE ) );
                    rpURLList->Insert( pURL, rpURLList->Count() );
                }
            }
            return ERRCODE_NONE;
        }
        else
            return ERRCODE_ABORT;
    }
    else
        return ERRCODE_ABORT;
}

// ------------------------------------------------------------------------
ErrCode FileDialogHelper_Impl::execute()
{
    if ( ! mxFileDlg.is() )
        return ERRCODE_ABORT;

    loadConfig();
    setDefaultValues();
    enablePasswordBox();

    // show the dialog
    sal_Int16 nRet = mxFileDlg->execute();

    maPath = mxFileDlg->getDisplayDirectory();

    saveConfig();

    if ( nRet == ExecutableDialogResults::CANCEL )
        return ERRCODE_ABORT;
    else
        return ERRCODE_NONE;
}

// ------------------------------------------------------------------------
OUString FileDialogHelper_Impl::getPath() const
{
    OUString aPath;

    if ( mxFileDlg.is() )
        aPath = mxFileDlg->getDisplayDirectory();

    if ( !aPath.getLength() )
        aPath = maPath;

    return aPath;
}

// ------------------------------------------------------------------------
OUString FileDialogHelper_Impl::getFilter() const
{
    OUString aFilter;
    Reference< XFilterManager > xFltMgr( mxFileDlg, UNO_QUERY );

    if ( xFltMgr.is() )
        aFilter = xFltMgr->getCurrentFilter();
    else
        aFilter = maCurFilter;

    return aFilter;
}

// ------------------------------------------------------------------------
OUString FileDialogHelper_Impl::getRealFilter() const
{
    OUString aFilter;
    Reference< XFilterManager > xFltMgr( mxFileDlg, UNO_QUERY );

    if ( xFltMgr.is() )
        aFilter = xFltMgr->getCurrentFilter();

    if ( ! aFilter.getLength() )
        aFilter = maCurFilter;

    if ( aFilter.getLength() && mpMatcher )
    {
        const SfxFilter* pFilter = mpMatcher->GetFilter4UIName(
                                        aFilter, 0, SFX_FILTER_NOTINFILEDLG );
        if ( pFilter )
            aFilter = pFilter->GetName();
    }

    return aFilter;
}

// ------------------------------------------------------------------------
void FileDialogHelper_Impl::setPath( const OUString& rPath )
{
    // We set the display directory only, when it is on a local / remote(?)
    // filesystem
    /*
    String aTmp;
    utl::LocalFileHelper::ConvertURLToSystemPath( aPath, aTmp );
    if ( aTmp.Len() )
        ...
    */
    if ( ! rPath.getLength() ||
         ! utl::LocalFileHelper::IsLocalFile( rPath ) )
    {
        return;
    }

    OUString aName;

    INetURLObject aObj( rPath );

    // if the given path isn't a folder, we cut off the last part
    // and take it as filename and the rest of the path should be
    // the folder

    if ( ! utl::UCBContentHelper::IsFolder( rPath ) )
    {
        aName = aObj.GetName( INetURLObject::DECODE_WITH_CHARSET );
        aObj.removeSegment();
    }

    maPath = aObj.GetMainURL( INetURLObject::NO_DECODE );

    // set the path
    if ( mxFileDlg.is() )
    {
        try
        {
            if ( maPath.getLength() )
                mxFileDlg->setDisplayDirectory( maPath );
            if ( aName.getLength() )
                mxFileDlg->setDefaultName( aName );
        }
        catch( IllegalArgumentException ){}
    }
}

// ------------------------------------------------------------------------
void FileDialogHelper_Impl::setFilter( const OUString& rFilter )
{
    maCurFilter = rFilter;

    if ( rFilter.getLength() && mpMatcher )
    {
        const SfxFilter* pFilter = mpMatcher->GetFilter(
                                        rFilter, 0, SFX_FILTER_NOTINFILEDLG );
        if ( pFilter )
            maCurFilter = pFilter->GetUIName();
    }

    Reference< XFilterManager > xFltMgr( mxFileDlg, UNO_QUERY );

    if ( maCurFilter.getLength() && xFltMgr.is() )
    {
        try
        {
            xFltMgr->setCurrentFilter( maCurFilter );
        }
        catch( IllegalArgumentException ){}
    }
}

// ------------------------------------------------------------------------
void FileDialogHelper_Impl::addFilters( sal_uInt32 nFlags,
                                        const SfxObjectFactory& rFactory )
{
    Reference< XFilterManager > xFltMgr( mxFileDlg, UNO_QUERY );

    if ( ! xFltMgr.is() )
        return;

    // create the list of filters

    if ( !&rFactory )
    {
        SfxApplication *pSfxApp = SFX_APP();

        mpMatcher = &pSfxApp->GetFilterMatcher();
        mbDeleteMatcher = sal_False;
    }
    else
    {
        mpMatcher = new SfxFilterMatcher( rFactory.GetFilterContainer() );
        mbDeleteMatcher = sal_True;
    }

    USHORT nFilterFlags = SFX_FILTER_EXPORT;

    if( WB_OPEN == ( nFlags & WB_OPEN ) )
        nFilterFlags = SFX_FILTER_IMPORT;

    sal_Bool    bHasAll = sal_False;
    OUString    aAllFilterName = OUString( String( SfxResId( STR_FILTERNAME_ALL ) ) );
    OUString    aUIName;
    SfxFilterMatcherIter aIter( mpMatcher, nFilterFlags, SFX_FILTER_INTERNAL | SFX_FILTER_NOTINFILEDLG );
    const SfxFilter* pDef = aIter.First();

    // when in file open mode, we have to check, wether there is a filter named <ALL>
    if ( WB_OPEN == ( nFlags & WB_OPEN ) )
    {
        for ( const SfxFilter* pFilter = pDef; pFilter; pFilter = aIter.Next() )
        {
            OUString aUIName( pFilter->GetUIName() );
            if ( aUIName == aAllFilterName )
                bHasAll = sal_True;
        }

        // Add the filter for displaying all files, if there is none
        if ( !bHasAll )
        {
            try
            {
                xFltMgr->appendFilter( aAllFilterName, DEFINE_CONST_UNICODE( FILEDIALOG_FILTER_ALL ) );
                maSelectFilter = aAllFilterName;
            }
            catch( IllegalArgumentException )
            {
                ByteString aMsg( "Could not append Filter" );
                aMsg += ByteString( String( aAllFilterName ), RTL_TEXTENCODING_UTF8 );
                DBG_ERRORFILE( aMsg.GetBuffer() );
            }
        }
    }

    pDef = aIter.First();

    for ( const SfxFilter* pFilter = pDef; pFilter; pFilter = aIter.Next() )
    {
        aUIName = pFilter->GetUIName();
        try
        {
            xFltMgr->appendFilter( aUIName, pFilter->GetWildcard().GetWildCard() );
            if ( !maSelectFilter.getLength() )
                maSelectFilter = aUIName;
        }
        catch( IllegalArgumentException )
        {
            ByteString aMsg( "Could not append Filter" );
            aMsg += ByteString( String( aUIName ), RTL_TEXTENCODING_UTF8 );
            DBG_ERRORFILE( aMsg.GetBuffer() );
        }
    }
}

// ------------------------------------------------------------------------
void FileDialogHelper_Impl::addFilter( const OUString& rFilterName,
                                       const OUString& rExtension )
{
    Reference< XFilterManager > xFltMgr( mxFileDlg, UNO_QUERY );

    if ( ! xFltMgr.is() )
        return;

    try
    {
        xFltMgr->appendFilter( rFilterName, rExtension );

        if ( !maSelectFilter.getLength() )
            maSelectFilter = rFilterName;
    }
    catch( IllegalArgumentException )
    {
        ByteString aMsg( "Could not append Filter" );
        aMsg += ByteString( String( rFilterName ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
    }
}

// ------------------------------------------------------------------------
void FileDialogHelper_Impl::addGraphicFilter()
{
    Reference< XFilterManager > xFltMgr( mxFileDlg, UNO_QUERY );

    if ( ! xFltMgr.is() )
        return;

    // create the list of filters
    mpGraphicFilter = new GraphicFilter;
    USHORT i, j, nCount = mpGraphicFilter->GetImportFormatCount();

    // compute the extension string for all known import filters
    String aExtensions;

    for ( i = 0; i < nCount; i++ )
    {
        j = 0;
        while( TRUE )
        {
            String aWildcard( mpGraphicFilter->GetImportWildcard( i, j++ ) );
            if ( !aWildcard.Len() )
                break;
            if ( aExtensions.Search( aWildcard ) == STRING_NOTFOUND )
            {
                if ( aExtensions.Len() )
                    aExtensions += sal_Unicode(';');
                aExtensions += aWildcard;
            }
        }
    }

#if defined(WIN) || defined(WNT)
    if ( aExtensions.Len() > 240 )
        aExtensions = String::CreateFromAscii( FILEDIALOG_FILTER_ALL );
#endif

    try
    {
        OUString aAllFilterName = String( SfxResId( STR_SFX_IMPORT_ALL ) );

        xFltMgr->appendFilter( aAllFilterName, aExtensions );
        maSelectFilter = aAllFilterName;
    }
    catch( IllegalArgumentException )
    {
        DBG_ERRORFILE( "Could not append Filter" );
    }

    // Now add the filter
    for ( i = 0; i < nCount; i++ )
    {
        String aName = mpGraphicFilter->GetImportFormatName( i );
        String aExtensions;
        j = 0;
        while( TRUE )
        {
            String aWildcard( mpGraphicFilter->GetImportWildcard( i, j++ ) );
            if ( !aWildcard.Len() )
                break;
            if ( aExtensions.Search( aWildcard ) == STRING_NOTFOUND )
            {
                if ( aExtensions.Len() )
                    aExtensions += sal_Unicode(';');
                aExtensions += aWildcard;
            }
        }
        try
        {
            xFltMgr->appendFilter( aName, aExtensions );
        }
        catch( IllegalArgumentException )
        {
            DBG_ERRORFILE( "Could not append Filter" );
        }
    }
}

// ------------------------------------------------------------------------
#define GRF_CONFIG_STR      "   "
#define STD_CONFIG_STR      "1 "

void FileDialogHelper_Impl::saveConfig()
{
    Reference < XFilePickerControlAccess > xDlg( mxFileDlg, UNO_QUERY );
    Any aValue;

    if ( ! xDlg.is() )
        return;

    if ( mbHasPreview )
    {
        SvtViewOptions aDlgOpt( E_DIALOG, IMPGRF_CONFIGNAME );
        String aUserData = String::CreateFromAscii( GRF_CONFIG_STR );

        try
        {
            aValue = xDlg->getValue( ExtendedFilePickerElementIds::CHECKBOX_LINK, 0 );
            sal_Bool bValue = sal_False;
            aValue >>= bValue;
            aUserData.SetToken( 0, ' ', String::CreateFromInt32( (sal_Int32) bValue ) );

            aValue = xDlg->getValue( ExtendedFilePickerElementIds::CHECKBOX_PREVIEW, 0 );
            bValue = sal_False;
            aValue >>= bValue;
            aUserData.SetToken( 1, ' ', String::CreateFromInt32( (sal_Int32) bValue ) );

            INetURLObject aObj( getPath() );

            if ( aObj.GetProtocol() == INET_PROT_FILE )
                aUserData.SetToken( 2, ' ', aObj.GetMainURL( INetURLObject::NO_DECODE ) );

            String aFilter = getFilter();
            aFilter = EncodeSpaces_Impl( aFilter );
            aUserData.SetToken( 3, ' ', aFilter );

            aDlgOpt.SetUserData( aUserData );
        }
        catch( IllegalArgumentException ){}
    }
    else
    {
        sal_Bool bWriteConfig = sal_False;
        SvtViewOptions aDlgOpt( E_DIALOG, IODLG_CONFIGNAME );
           String aUserData = String::CreateFromAscii( STD_CONFIG_STR );

        if ( aDlgOpt.Exists() )
        {
            aUserData = aDlgOpt.GetUserData();
        }

        if ( mbHasAutoExt )
        {
            try
            {
                aValue = xDlg->getValue( ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION, 0 );
                sal_Bool bAutoExt = sal_True;
                aValue >>= bAutoExt;
                aUserData.SetToken( 0, ' ', String::CreateFromInt32( (sal_Int32) bAutoExt ) );
                bWriteConfig = sal_True;
            }
            catch( IllegalArgumentException ){}
        }

        if ( ! mbIsSaveDlg )
        {
            OUString aPath = getPath();
            if ( aPath.getLength() &&
                 utl::LocalFileHelper::IsLocalFile( aPath ) )
            {
                aUserData.SetToken( 1, ' ', aPath );
                bWriteConfig = sal_True;
            }
        }

        if ( bWriteConfig )
            aDlgOpt.SetUserData( aUserData );
    }
}

// ------------------------------------------------------------------------
void FileDialogHelper_Impl::loadConfig()
{
    Reference < XFilePickerControlAccess > xDlg( mxFileDlg, UNO_QUERY );
    Any aValue;

    if ( ! xDlg.is() )
        return;

    if ( mbHasPreview )
    {
        SvtViewOptions aViewOpt( E_DIALOG, IMPGRF_CONFIGNAME );
        String aUserData;

        if ( aViewOpt.Exists() )
            aUserData = aViewOpt.GetUserData();

        if ( aUserData.Len() > 0 )
        {
            try
            {
                // respect the last "insert as link" state
                sal_Bool bLink = (sal_Bool) aUserData.GetToken( 0, ' ' ).ToInt32();
                aValue <<= bLink;
                xDlg->setValue( ExtendedFilePickerElementIds::CHECKBOX_LINK, 0, aValue );

                // respect the last "show preview" state
                sal_Bool bShowPreview = (sal_Bool) aUserData.GetToken( 1, ' ' ).ToInt32();
                aValue <<= bShowPreview;
                xDlg->setValue( ExtendedFilePickerElementIds::CHECKBOX_PREVIEW, 0, aValue );

                if ( ! maPath.getLength() )
                    setPath( aUserData.GetToken( 2, ' ' ) );

                if ( ! maCurFilter.getLength() )
                {
                    String aFilter = aUserData.GetToken( 3, ' ' );
                    aFilter = DecodeSpaces_Impl( aFilter );
                    setFilter( aFilter );
                }

                // set the member so we know that we have to show the preview
                mbShowPreview = bShowPreview;
            }
            catch( IllegalArgumentException ){}
        }
    }
    else
    {
        SvtViewOptions aViewOpt( E_DIALOG, IODLG_CONFIGNAME );
        String aUserData;

        if ( aViewOpt.Exists() )
            aUserData = aViewOpt.GetUserData();

        if ( ! aUserData.Len() )
            aUserData = String::CreateFromAscii( STD_CONFIG_STR );

        if ( ! maPath.getLength() )
            setPath( aUserData.GetToken( 1, ' ' ) );

        if ( mbHasAutoExt )
        {
            sal_Int32 nFlag = aUserData.GetToken( 0, ' ' ).ToInt32();
            aValue <<= (sal_Bool) nFlag;
            try
            {
                xDlg->setValue( ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION, 0, aValue );
            }
            catch( IllegalArgumentException ){}
        }
    }
}

// ------------------------------------------------------------------------
void FileDialogHelper_Impl::setDefaultValues()
{
    // when no filter is set, we set the curentFilter to <all>
    if ( !maCurFilter.getLength() && maSelectFilter.getLength() )
    {
        Reference< XFilterManager > xFltMgr( mxFileDlg, UNO_QUERY );
        try
        {
            xFltMgr->setCurrentFilter( maSelectFilter );
        }
        catch( IllegalArgumentException )
        {}
    }

    // when no path is set, we use the standard 'work' folder
    if ( ! maPath.getLength() )
    {
        OUString aWorkFolder = SvtPathOptions().GetWorkPath();
        mxFileDlg->setDisplayDirectory( aWorkFolder );

        // INetURLObject aStdDirObj( SvtPathOptions().GetWorkPath() );
        //SetStandardDir( aStdDirObj.GetMainURL( INetURLObject::NO_DECODE ) );
    }
}

// ------------------------------------------------------------------------
// -----------          FileDialogHelper        ---------------------------
// ------------------------------------------------------------------------

FileDialogHelper::FileDialogHelper( sal_uInt32 nFlags,
                                    const SfxObjectFactory& rFact )
{
    mpImp = new FileDialogHelper_Impl( getDialogType( nFlags ), nFlags );
    mxImp = mpImp;

    // create the list of filters
    mpImp->addFilters( nFlags, rFact );
}

// ------------------------------------------------------------------------
FileDialogHelper::FileDialogHelper( sal_uInt32 nFlags )
{
    const short nDialogType = getDialogType( nFlags );

    mpImp = new FileDialogHelper_Impl( nDialogType, nFlags );
    mxImp = mpImp;
}

// ------------------------------------------------------------------------
FileDialogHelper::FileDialogHelper( const short nDialogType,
                                    sal_uInt32 nFlags,
                                    const SfxObjectFactory& rFact )
{
    mpImp = new FileDialogHelper_Impl( nDialogType, nFlags );
    mxImp = mpImp;

    // create the list of filters
    mpImp->addFilters( nFlags, rFact );
}

// ------------------------------------------------------------------------
FileDialogHelper::FileDialogHelper( const short nDialogType,
                                    sal_uInt32 nFlags )
{
    mpImp = new FileDialogHelper_Impl( nDialogType, nFlags );
    mxImp = mpImp;
}

// ------------------------------------------------------------------------
FileDialogHelper::~FileDialogHelper()
{
    mpImp->dispose();
    mxImp.clear();
}

// ------------------------------------------------------------------------
ErrCode FileDialogHelper::Execute( const String&   rPath,
                                   SvStringsDtor*& rpURLList,
                                   SfxItemSet *&   rpSet,
                                   String&         rFilter )
{
    SetDisplayDirectory( rPath );

    return mpImp->execute( rpURLList, rpSet, rFilter );
}

// ------------------------------------------------------------------------
ErrCode FileDialogHelper::Execute()
{
    return mpImp->execute();
}

// ------------------------------------------------------------------------
ErrCode FileDialogHelper::Execute( SfxItemSet *&   rpSet,
                                   String&         rFilter)
{
    ErrCode nRet;
    SvStringsDtor* pURLList;

    nRet = mpImp->execute( pURLList, rpSet, rFilter );

    delete pURLList;

    return nRet;
}

// ------------------------------------------------------------------------
void FileDialogHelper::SetTitle( const String& rNewTitle )
{
    if ( mpImp->mxFileDlg.is() )
        mpImp->mxFileDlg->setTitle( rNewTitle );
}

// ------------------------------------------------------------------------
String FileDialogHelper::GetPath() const
{
    OUString aPath;

    if ( mpImp->mxFileDlg.is() )
    {
        Sequence < OUString > aPathSeq = mpImp->mxFileDlg->getFiles();

        if ( aPathSeq.getLength() == 1 )
        {
            aPath = aPathSeq[0];
        }
    }

    return aPath;
}

// ------------------------------------------------------------------------
Sequence < OUString > FileDialogHelper::GetMPath() const
{
    if ( mpImp->mxFileDlg.is() )
        return mpImp->mxFileDlg->getFiles();
    else
    {
        Sequence < OUString > aEmpty;
        return aEmpty;
    }
}

// ------------------------------------------------------------------------
String FileDialogHelper::GetDisplayDirectory() const
{
    return mpImp->getPath();
}

// ------------------------------------------------------------------------
String FileDialogHelper::GetCurrentFilter() const
{
    return mpImp->getFilter();
}

#if SUPD < 639
// ------------------------------------------------------------------------
Graphic FileDialogHelper::GetGraphic() const
{
    Graphic aGraphic;

    mpImp->getGraphic( aGraphic );

    return aGraphic;
}
#endif

// ------------------------------------------------------------------------
ErrCode FileDialogHelper::GetGraphic( Graphic& rGraphic ) const
{
    return mpImp->getGraphic( rGraphic );
}

// ------------------------------------------------------------------------
void FileDialogHelper::SetDisplayDirectory( const String& rPath )
{
    mpImp->setPath( rPath );
}

// ------------------------------------------------------------------------
void FileDialogHelper::AddFilter( const String& rFilterName,
                                  const String& rExtension )
{
    mpImp->addFilter( rFilterName, rExtension );
}

// ------------------------------------------------------------------------
void FileDialogHelper::SetCurrentFilter( const String& rFilter )
{
    mpImp->setFilter( rFilter );
}

// ------------------------------------------------------------------------
Reference < XFilePicker > FileDialogHelper::GetFilePicker() const
{
    return mpImp->mxFileDlg;
}

// ------------------------------------------------------------------------
const short FileDialogHelper::getDialogType( sal_uInt32 nFlags ) const
{
    short nDialogType = FILEOPEN_SIMPLE;

    if ( nFlags & WB_SAVEAS )
    {
        if ( nFlags & SFXWB_PASSWORD )
            nDialogType = FILESAVE_AUTOEXTENSION_PASSWORD;
        else
            nDialogType = FILESAVE_SIMPLE;
    }
    else if ( nFlags & SFXWB_GRAPHIC )
    {
        if ( nFlags & SFXWB_SHOWSTYLES )
            nDialogType = FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE;
        else
            nDialogType = FILEOPEN_LINK_PREVIEW;
    }
    else if ( SFXWB_INSERT != ( nFlags & SFXWB_INSERT ) )
        nDialogType = FILEOPEN_READONLY_VERSION;

    return nDialogType;
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

ErrCode FileOpenDialog_Impl( sal_uInt32 nFlags,
                             const SfxObjectFactory& rFact,
                             SvStringsDtor *& rpURLList,
                             String& rFilter,
                             SfxItemSet *& rpSet,
                             String aPath )
{
    ErrCode nRet;
    FileDialogHelper aDialog( nFlags, rFact );

    nRet = aDialog.Execute( aPath, rpURLList, rpSet, rFilter );

    aPath = aDialog.GetDisplayDirectory();

    return nRet;
}


// ------------------------------------------------------------------------
String EncodeSpaces_Impl( const String& rSource )
{
    String aRet( rSource );
    aRet.SearchAndReplaceAll( String::CreateFromAscii( " " ),
                              String::CreateFromAscii( "%20" ) );
    return aRet;
}

// ------------------------------------------------------------------------
String DecodeSpaces_Impl( const String& rSource )
{
    String aRet( rSource );
    aRet.SearchAndReplaceAll( String::CreateFromAscii( "%20" ),
                              String::CreateFromAscii( " " ) );
    return aRet;
}

// ------------------------------------------------------------------------

}   // end of namespace sfx2
