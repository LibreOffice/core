/*************************************************************************
 *
 *  $RCSfile: filedlghelper.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2001-05-10 12:45:14 $
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

#include "filedlghelper.hxx"

#ifndef  _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef  _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

#ifndef  _COM_SUN_STAR_UI_FILEDIALOGRESULTS_HPP_
#include <com/sun/star/ui/FileDialogResults.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_XFILEPICKERCONTROLACCESS_HPP_
#include <com/sun/star/ui/XFilePickerControlAccess.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_FILEPICKERELEMENTID_HPP_
#include <com/sun/star/ui/FilePickerElementID.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_XFILEPICKER_HPP_
#include <com/sun/star/ui/XFilePicker.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_XFILEPICKERLISTENER_HPP_
#include <com/sun/star/ui/XFilePickerListener.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_XFILEPICKERNOTIFIER_HPP_
#include <com/sun/star/ui/XFilePickerNotifier.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_XFILTERMANAGER_HPP_
#include <com/sun/star/ui/XFilterManager.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
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
#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>
#ifndef _FILTER_HXX
#include <svtools/filter.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_VIEWOPTIONS_HXX
#include <svtools/viewoptions.hxx>
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

#include "sfxresid.hxx"
#include "sfxsids.hrc"
#include "explorer.hrc"

//-----------------------------------------------------------------------------

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui;
using namespace ::com::sun::star::uno;
using namespace ::rtl;
using namespace ::cppu;

//-----------------------------------------------------------------------------

#define IODLG_CONFIGNAME        String(DEFINE_CONST_UNICODE("FileDialog"))
#define IMPGRF_CONFIGNAME       String(DEFINE_CONST_UNICODE("ImportGraphicDialog"))

//-----------------------------------------------------------------------------

namespace sfx2 {

// ------------------------------------------------------------------------
class FileDialogHelper_Impl : public WeakImplHelper1< XFilePickerListener >
{
    friend class FileDialogHelper;

    Reference < XFilePicker >   mxFileDlg;

    SfxFilterMatcher       *mpMatcher;
    OUString                maPath;
    OUString                maCurFilter;
    ErrCode                 mnError;
    sal_Bool                mbHasPassword   : 1;
    sal_Bool                mbHasVersions   : 1;
    sal_Bool                mbHasAutoExt    : 1;
    sal_Bool                mbHasLink       : 1;
    sal_Bool                mbHasPreview    : 1;

    sal_Bool                mbDeleteMatcher : 1;
    sal_Bool                mbInsert        : 1;

private:
    void                    addFilters( sal_uInt32 nFlags,
                                        const SfxObjectFactory& rFactory );
    void                    addGraphicFilter();
    void                    enablePasswordBox();
    void                    updateVersions();
    void                    dispose();

    void                    loadConfig();
    void                    saveConfig();

public:
    // XFilePickerListener methods
    virtual void SAL_CALL   focusGained( const FilePickerEvent& aEvent ) throw ( RuntimeException );
    virtual void SAL_CALL   focusLost( const FilePickerEvent& aEvent ) throw ( RuntimeException );
    virtual void SAL_CALL   fileSelectionChanged( const FilePickerEvent& aEvent ) throw ( RuntimeException );
    virtual void SAL_CALL   directoryChanged( const FilePickerEvent& aEvent ) throw ( RuntimeException );
    virtual void SAL_CALL   helpRequested( const FilePickerEvent& aEvent ) throw ( RuntimeException );
    virtual void SAL_CALL   ctrlStateChanged( const FilePickerEvent& aEvent ) throw ( RuntimeException );

    // XEventListener methods
    virtual void SAL_CALL   disposing( const EventObject& Source ) throw ( RuntimeException );

    // Own methods
                            FileDialogHelper_Impl( sal_uInt32 nFlags );
                           ~FileDialogHelper_Impl();

    ErrCode                 execute( const String&   rPath,
                                     SvStringsDtor*& rpURLList,
                                     SfxItemSet *&   rpSet,
                                     String&         rFilter );
    ErrCode                 execute();

    void                    setPath( const OUString& rPath ) { maPath = rPath; }
    void                    setFilter( const OUString& rFilter ) { maCurFilter = rFilter; }

    OUString                getPath() const { return maPath; }
    OUString                getFilter() const { return maCurFilter; }
};

// ------------------------------------------------------------------------
// -----------      FileDialogHelper_Impl       ---------------------------
// ------------------------------------------------------------------------

// ------------------------------------------------------------------------
// XFilePickerListener Methods
// ------------------------------------------------------------------------
void SAL_CALL FileDialogHelper_Impl::focusGained( const FilePickerEvent& aEvent ) throw ( RuntimeException )
{}

// ------------------------------------------------------------------------
void SAL_CALL FileDialogHelper_Impl::focusLost( const FilePickerEvent& aEvent ) throw ( RuntimeException )
{}

// ------------------------------------------------------------------------
void SAL_CALL FileDialogHelper_Impl::fileSelectionChanged( const FilePickerEvent& aEvent ) throw ( RuntimeException )
{
    if ( mbHasVersions )
        updateVersions();
}

// ------------------------------------------------------------------------
void SAL_CALL FileDialogHelper_Impl::directoryChanged( const FilePickerEvent& aEvent ) throw ( RuntimeException )
{}

// ------------------------------------------------------------------------
void SAL_CALL FileDialogHelper_Impl::helpRequested( const FilePickerEvent& aEvent ) throw ( RuntimeException )
{}

// ------------------------------------------------------------------------
void SAL_CALL FileDialogHelper_Impl::ctrlStateChanged( const FilePickerEvent& aEvent ) throw ( RuntimeException )
{
    if ( ( aEvent.ElementId == FilePickerElementID::CBO_FILTER ) && mbHasPassword )
        enablePasswordBox();
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
    Reference< XFilterManager > xFltMgr( mxFileDlg, UNO_QUERY );
    OUString aFilterName = xFltMgr->getCurrentFilter();

    const SfxFilter* pFilter = mpMatcher->GetFilter(
            aFilterName, 0, SFX_FILTER_NOTINFILEDLG );

    BOOL bEnablePasswd = pFilter && pFilter->UsesStorage() &&
        pFilter->IsOwnFormat();

    Reference < XFilePickerControlAccess > xCtrlAccess( mxFileDlg, UNO_QUERY );

    xCtrlAccess->enableControl( FilePickerElementID::CBX_PASSWORD, bEnablePasswd );
}

// ------------------------------------------------------------------------
void FileDialogHelper_Impl::updateVersions()
{
    Sequence < OUString > aEntries;
    Sequence < OUString > aPathSeq = mxFileDlg->getPath();

    if ( aPathSeq.getLength() == 1 )
    {
        INetURLObject aObj( aPathSeq[0] );

        if ( aObj.GetProtocol() == INET_PROT_FILE )
        {
            SfxMedium aMed( aObj.GetMainURL(), SFX_STREAM_READONLY_MAKECOPY, TRUE );
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
                aEntries.realloc( 1 );
                aEntries[0] = OUString( String ( SfxResId( STR_SFX_FILEDLG_ACTUALVERSION ) ) );
            }
        }
    }

    Reference < XFilePickerControlAccess > xDlg( mxFileDlg, UNO_QUERY );
    Any aValue;
    aValue <<= aEntries;

    xDlg->setValue( FilePickerElementID::CBO_VERSION, aValue );
}

// ------------------------------------------------------------------------
// -----------      FileDialogHelper_Impl       ---------------------------
// ------------------------------------------------------------------------

FileDialogHelper_Impl::FileDialogHelper_Impl( sal_uInt32 nFlags )
{
    // create the file open dialog

    // a SFXWB_INSERT creates a insert file dialog ( a default FileOpen dialog
    // with a different title and renamed ok button )
    // Otherwise, a FileOpen_ReadonlyVersionBox dialog will be created

    mnError         = ERRCODE_NONE;
    mbHasAutoExt    = sal_False;
    mbHasPassword   = sal_False;
    mbHasVersions   = sal_False;
    mbHasPreview    = sal_False;
    mbHasLink       = sal_False;
    mbDeleteMatcher = sal_False;
    mbInsert        = SFXWB_INSERT == ( nFlags & SFXWB_INSERT );

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

    Sequence < Any > aServiceName(1);

    if ( nFlags & WB_SAVEAS )
    {
        if ( nFlags & SFXWB_PASSWORD )
        {
            mbHasPassword = sal_True;
            mbHasAutoExt = sal_True;
            aServiceName[0] <<= OUString( RTL_CONSTASCII_USTRINGPARAM( FILE_SAVE_AUTOEXT_PWDBOX ) );
        }
        else
            aServiceName[0] <<= OUString( RTL_CONSTASCII_USTRINGPARAM( FILE_SAVE ) );
    }
    else if ( nFlags & SFXWB_GRAPHIC )
    {
        aServiceName[0] <<= OUString( RTL_CONSTASCII_USTRINGPARAM( FILE_OPEN_LINK_PREVIEWBOX ) );
        mbHasPreview = sal_True;
        mbHasLink = sal_True;
    }
    else if ( !mbInsert )
    {
        aServiceName[0] <<= OUString( RTL_CONSTASCII_USTRINGPARAM( FILE_OPEN_READONLY_VERSIONBOX ) );
        mbHasVersions = sal_True;
    }
    else
        aServiceName[0] <<= OUString( RTL_CONSTASCII_USTRINGPARAM( FILE_OPEN ) );

    if ( xInit.is() )
        xInit->initialize( aServiceName );

    // set multiselection mode
    if ( nFlags & SFXWB_MULTISELECTION )
        mxFileDlg->setMultiSelectionMode( sal_True );

    // the "insert file" dialog needs another title
    if ( mbInsert )
    {
        mxFileDlg->setTitle( OUString( String( SfxResId( STR_SFX_EXPLORERFILE_INSERT ) ) ) );

        Reference < XFilePickerControlAccess > xExtDlg( mxFileDlg, UNO_QUERY );
        if ( xExtDlg.is() )
        {
            xExtDlg->setLabel( FilePickerElementID::BTN_OK,
                               OUString( String( SfxResId( STR_SFX_EXPLORERFILE_BUTTONINSERT ) ) ) );
        }
    }

    // add the event listener
    xNotifier->addFilePickerListener( this );
}

// ------------------------------------------------------------------------
FileDialogHelper_Impl::~FileDialogHelper_Impl()
{
    if ( mbDeleteMatcher )
        delete mpMatcher;
}

// ------------------------------------------------------------------------
ErrCode FileDialogHelper_Impl::execute( const String&   rPath,
                                        SvStringsDtor*& rpURLList,
                                        SfxItemSet *&   rpSet,
                                        String&         rFilter )
{
    rpSet = NULL;
    rpURLList = NULL;

    if ( ! mxFileDlg.is() )
        return ERRCODE_ABORT;

    // set the path
    if ( rPath.Len() )
        mxFileDlg->setDisplayDirectory( rPath );

    loadConfig();

    // show the dialog
    sal_Int16 nRet = mxFileDlg->execute();

    if ( nRet != FileDialogResults::CANCEL )
    {
        saveConfig();

        // create an itemset
        rpSet = new SfxAllItemSet( SFX_APP()->GetPool() );

        // set the read-only flag. When inserting a file, this flag is always set
        if ( mbInsert )
            rpSet->Put( SfxBoolItem( SID_DOC_READONLY, sal_True ) );
        else
        {
            Reference< XFilePickerControlAccess > xCtrlAccess( mxFileDlg, UNO_QUERY );
            if ( xCtrlAccess.is() )
            {
                try
                {
                    Any aValue = xCtrlAccess->getValue( FilePickerElementID::CBX_READONLY );
                    sal_Bool bReadOnly = sal_False;
                    if ( aValue >>= bReadOnly )
                        rpSet->Put( SfxBoolItem( SID_DOC_READONLY, bReadOnly ) );
                }
                catch( IllegalArgumentException ){}
            }
        }
        if ( mbHasVersions )
        {
            Reference< XFilePickerControlAccess > xCtrlAccess( mxFileDlg, UNO_QUERY );
            if ( xCtrlAccess.is() )
            {
                try
                {
                    Any aValue = xCtrlAccess->getValue( FilePickerElementID::CBO_VERSION );
                    sal_Int16 nVersion = 0;
                    if ( aValue >>= nVersion )
                        rpSet->Put( SfxInt16Item( SID_VERSION, nVersion ) );
                }
                catch( IllegalArgumentException ){}
            }
        }

        // set the filter
        Reference< XFilterManager > xFltMgr( mxFileDlg, UNO_QUERY );
        rFilter = xFltMgr->getCurrentFilter();

        // fill the rpURLList
        Sequence < OUString > aPathSeq = mxFileDlg->getPath();

        if ( aPathSeq.getLength() )
        {
            rpURLList = new SvStringsDtor;

            if ( aPathSeq.getLength() == 1 )
            {
                String* pURL = new String( aPathSeq[0] );
                rpURLList->Insert( pURL, 0 );
            }
            else
            {
                const OUString aPath = aPathSeq[0];
                for ( USHORT i = 1; i < aPathSeq.getLength(); ++i )
                {
                    OUString aFullPath( aPath );
                    aFullPath += aPathSeq[i];

                    String* pURL = new String( aFullPath );
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
    Reference< XFilterManager > xFltMgr( mxFileDlg, UNO_QUERY );

    if ( ! mxFileDlg.is() || !xFltMgr.is() )
        return ERRCODE_ABORT;

    if ( maPath.getLength() )
    {
        INetURLObject aObj( maPath, INET_PROT_FILE );
        OUString aTitle( aObj.getName( INetURLObject::LAST_SEGMENT, true,
                         INetURLObject::DECODE_WITH_CHARSET ) );
        aObj.removeSegment();
        mxFileDlg->setDisplayDirectory( aObj.GetMainURL() );
        mxFileDlg->setDefaultName( aTitle );
    }

    if ( maCurFilter.getLength() )
        xFltMgr->setCurrentFilter( maCurFilter );

    loadConfig();

    // show the dialog
    sal_Int16 nRet = mxFileDlg->execute();

    maPath = mxFileDlg->getDisplayDirectory();
    maCurFilter = xFltMgr->getCurrentFilter();

    if ( nRet == FileDialogResults::CANCEL )
        return ERRCODE_ABORT;
    else
    {
        saveConfig();
        return ERRCODE_NONE;
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

    for ( const SfxFilter* pFilter = pDef; pFilter; pFilter = aIter.Next() )
    {
        aUIName = pFilter->GetUIName();
        xFltMgr->appendFilter( aUIName, pFilter->GetWildcard().GetWildCard() );
        if ( aUIName == aAllFilterName )
            bHasAll = sal_True;
    }

    // Add the filter for displaying all files, if there is none
    if ( !bHasAll )
        xFltMgr->appendFilter( aAllFilterName, DEFINE_CONST_UNICODE( FILEDIALOG_FILTER_ALL ) );
}

// ------------------------------------------------------------------------
void FileDialogHelper_Impl::addGraphicFilter()
{
    Reference< XFilterManager > xFltMgr( mxFileDlg, UNO_QUERY );

    if ( ! xFltMgr.is() )
        return;

    // create the list of filters
    GraphicFilter*  pGraphicFilter = new GraphicFilter;
    USHORT          i, nCount = pGraphicFilter->GetImportFormatCount();

    // compute the extension string for all known import filters
    String aExtensions;

    for ( i = 0; i < nCount; i++ )
    {
        String aWildcard = pGraphicFilter->GetImportWildcard( i );

        if ( aExtensions.Search( aWildcard ) == STRING_NOTFOUND )
        {
            if ( aExtensions.Len() )
                aExtensions += sal_Unicode(';');
            aExtensions += aWildcard;
        }
    }

#if defined(WIN) || defined(WNT)
    if ( aExtensions.Len() > 240 )
        aExtensions = String::CreateFromAscii( FILEDIALOG_FILTER_ALL );
#endif

    xFltMgr->appendFilter( String( SfxResId( STR_SFX_IMPORT_ALL ) ),
                           aExtensions );

    // Now add the filter
    for ( i = 0; i < nCount; i++ )
    {
        String aName = pGraphicFilter->GetImportFormatName( i );
        String aWildcard = pGraphicFilter->GetImportWildcard( i );
        xFltMgr->appendFilter( aName, aWildcard );
    }

    delete pGraphicFilter;
}

/*
// ------------------------------------------------------------------------
void FileDialogHelper_Impl::initGraphic( sal_uInt32 nFlags )
{
#if 0   //--**--

    SetValue( PREVIEW_BOX, bShowPreview );

    // now insert all the filters
    const SfxStringItem* pPathItem =
        (const SfxStringItem*)pSfxApp->GetItem( SID_IMPORT_GRAPH_LASTPATH );
    const SfxStringItem* pFilterItem =
        (const SfxStringItem*)pSfxApp->GetItem( SID_IMPORT_GRAPH_LASTFILTER );

#endif  //--**--
}

*/


// ------------------------------------------------------------------------
void FileDialogHelper_Impl::saveConfig()
{
    Reference < XFilePickerControlAccess > xDlg( mxFileDlg, UNO_QUERY );
    Any aValue;

    if ( ! xDlg.is() )
        return;

    if ( mbHasAutoExt )
    {
        aValue = xDlg->getValue( FilePickerElementID::CBX_AUTO_FILE_EXTENSION );
        sal_Bool bAutoExt = sal_False;
        if ( aValue >>= bAutoExt )
        {
            SvtViewOptions aDlgOpt( E_DIALOG, IODLG_CONFIGNAME );
            if ( aDlgOpt.Exists() )
                aDlgOpt.SetUserData( String::CreateFromInt32( (sal_Int32) bAutoExt ) );
        }
    }

    if ( mbHasPreview || mbHasLink )
    {
        SvtViewOptions aDlgOpt( E_DIALOG, IMPGRF_CONFIGNAME );
        String aUserData;// = aDlgOpt.GetUserData();

        aValue = xDlg->getValue( FilePickerElementID::CBX_INSERT_AS_LINK );
        sal_Bool bValue = sal_False;
        if ( aValue >>= bValue )
            aUserData.SetToken( 0, ';', String::CreateFromInt32( (sal_Int32) bValue ) );

        aValue = xDlg->getValue( FilePickerElementID::CBX_PREVIEW );
        bValue = sal_False;
        if ( aValue >>= bValue )
            aUserData.SetToken( 1, ';', String::CreateFromInt32( (sal_Int32) bValue ) );

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

    if ( mbHasAutoExt )
    {
        // initialize from config
        SvtViewOptions aDlgOpt( E_DIALOG, IODLG_CONFIGNAME );

        if ( aDlgOpt.Exists() )
        {
            sal_Int32 nFlag = aDlgOpt.GetUserData().toInt32();
            aValue <<= (sal_Bool) nFlag;
            xDlg->setValue( FilePickerElementID::CBX_AUTO_FILE_EXTENSION, aValue );
        }
    }

    if ( mbHasPreview || mbHasLink )
    {
        SvtViewOptions aViewOpt( E_DIALOG, IMPGRF_CONFIGNAME );
        String aUserData;

        if ( aViewOpt.Exists() )
            aUserData = aViewOpt.GetUserData();

        if ( aUserData.Len() > 0 )
        {
            // respect the last "insert as link" state
            sal_Bool bLink = (sal_Bool) aUserData.GetToken(0).ToInt32();
            aValue <<= bLink;
            xDlg->setValue( FilePickerElementID::CBX_INSERT_AS_LINK, aValue );

            // respect the last "show preview" state
            sal_Bool bShowPreview = (sal_Bool) aUserData.GetToken(1).ToInt32();
            aValue <<= bShowPreview;
            xDlg->setValue( FilePickerElementID::CBX_PREVIEW, aValue );
        }
    }
}

// ------------------------------------------------------------------------
// -----------          FileDialogHelper        ---------------------------
// ------------------------------------------------------------------------

FileDialogHelper::FileDialogHelper( sal_uInt32 nFlags )
{
    mpImp = new FileDialogHelper_Impl( nFlags );
    mxImp = mpImp;

    if ( nFlags & SFXWB_GRAPHIC )
        mpImp->addGraphicFilter();
}

// ------------------------------------------------------------------------

FileDialogHelper::FileDialogHelper( sal_uInt32 nFlags,
                                    const SfxObjectFactory& rFact )
{
    mpImp = new FileDialogHelper_Impl( nFlags );
    mxImp = mpImp;

    // create the list of filters
    mpImp->addFilters( nFlags, rFact );
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
    return mpImp->execute( rPath, rpURLList, rpSet, rFilter );
}

// ------------------------------------------------------------------------
ErrCode FileDialogHelper::Execute()
{
    return mpImp->execute();
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
    if ( mpImp->mxFileDlg.is() )
    {
        Sequence < OUString > aPathSeq = mpImp->mxFileDlg->getPath();

        if ( aPathSeq.getLength() == 1 )
            return String( aPathSeq[0] );
    }

    return String();
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

// ------------------------------------------------------------------------
void FileDialogHelper::SetDisplayDirectory( const String& rPath )
{
    mpImp->setPath( rPath );
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
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

ErrCode FileOpenDialog_Impl( sal_uInt32 nFlags,
                             const SfxObjectFactory& rFact,
                             SvStringsDtor *& rpURLList,
                             String& rFilter,
                             SfxItemSet *& rpSet,
                             String aPath )
{
    FileDialogHelper aDialog( nFlags, rFact );
    return aDialog.Execute( aPath, rpURLList, rpSet, rFilter );
}


// ------------------------------------------------------------------------

}   // end of namespace sfx2
