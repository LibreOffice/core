/*************************************************************************
 *
 *  $RCSfile: hldocntp.cxx,v $
 *
 *  $Revision: 1.26 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 08:32:30 $
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

#include "hldocntp.hxx"

#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFX_OBJFAC_HXX
#include <sfx2/docfac.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include <com/sun/star/uno/Exception.hpp>
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif
#ifndef _SV_CONFIG_HXX
#include <vcl/config.hxx>
#endif
#ifndef _SV_SYSTEM_HXX
#include <vcl/system.hxx>
#endif
#ifndef _SV_IMAGE_HXX
#include <vcl/image.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_DYNAMICMENUOPTIONS_HXX
#include <svtools/dynamicmenuoptions.hxx>
#endif
#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#ifndef _UNOTOOLS_UCBHELPER_HXX
#include <unotools/ucbhelper.hxx>
#endif

#include "hyperdlg.hrc"

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef  _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFOLDERPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_EXECUTABLEDIALOGRESULTS_HPP_
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#endif

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;

using namespace ::rtl;
using namespace ::com::sun::star;

/*************************************************************************
|*
|* Data-struct for documenttypes in listbox
|*
|************************************************************************/

struct DocumentTypeData
{
    String aStrURL;
    String aStrExt;
    DocumentTypeData (String aURL, String aExt) : aStrURL(aURL), aStrExt(aExt)
    {}
};

sal_Bool SvxHyperlinkNewDocTp::ImplGetURLObject( const String& rPath, const String& rBase, INetURLObject& aURLObject ) const
{
    sal_Bool bIsValidURL = rPath.Len() != 0;
    if ( bIsValidURL )
    {
        aURLObject.SetURL( rPath );
        if ( aURLObject.GetProtocol() == INET_PROT_NOT_VALID )      // test if the source is already a valid url
        {                                                           // if not we have to create a url from a physical file name
            String aNewURL;
            if ( !rBase.Len() )
                utl::LocalFileHelper::ConvertPhysicalNameToURL( rPath, aNewURL );
            else
                utl::LocalFileHelper::ConvertSystemPathToURL( rPath, rBase, aNewURL );
            if ( aNewURL.Len() )
                aURLObject = INetURLObject( aNewURL );
            else
            {
                aURLObject.SetSmartURL( SvtPathOptions().GetWorkPath() );
                if( !aURLObject.hasFinalSlash() )
                    aURLObject.setFinalSlash();
                aURLObject.Append( rPath );
            }
        }
        bIsValidURL = aURLObject.GetProtocol() != INET_PROT_NOT_VALID;
        if ( bIsValidURL )
        {
            String aBase( aURLObject.getName( INetURLObject::LAST_SEGMENT, sal_False ) );
            if ( ( aBase.Len() == 0 ) || ( aBase.GetChar( 0 ) == '.' ) )
                bIsValidURL = sal_False;
        }
        if ( bIsValidURL )
        {
            sal_Int16 nPos = maLbDocTypes.GetSelectEntryPos();
            if ( nPos != LISTBOX_ENTRY_NOTFOUND )
                aURLObject.SetExtension( ((DocumentTypeData*)maLbDocTypes.GetEntryData( nPos ))->aStrExt );
        }

    }
    return bIsValidURL;
}

/*************************************************************************
|*
|* Contructor / Destructor
|*
|************************************************************************/

SvxHyperlinkNewDocTp::SvxHyperlinkNewDocTp ( Window *pParent, const SfxItemSet& rItemSet)
:   maGrpNewDoc     ( this, ResId (GRP_NEWDOCUMENT) ),
    maRbtEditNow    ( this, ResId (RB_EDITNOW) ),
    maRbtEditLater  ( this, ResId (RB_EDITLATER) ),
    maFtPath        ( this, ResId (FT_PATH_NEWDOC) ),
    maCbbPath       ( this, INET_PROT_FILE ),
    maFtDocTypes    ( this, ResId (FT_DOCUMENT_TYPES) ),
    maLbDocTypes    ( this, ResId (LB_DOCUMENT_TYPES) ),
    maBtCreate      ( this, ResId (BTN_CREATE) ),
    SvxHyperlinkTabPageBase ( pParent, SVX_RES( RID_SVXPAGE_HYPERLINK_NEWDOCUMENT ),
                              rItemSet )
{
    // Set HC bitmaps and disable display of bitmap names.
    maBtCreate.SetModeImage( Image( ResId( IMG_CREATE_HC ) ), BMP_COLOR_HIGHCONTRAST );
    maBtCreate.EnableTextDisplay (FALSE);

    InitStdControls();
    FreeResource();

    SetExchangeSupport ();

    maCbbPath.SetPosSizePixel ( LogicToPixel( Point( COL_2 , 25 ), MAP_APPFONT ),
                                LogicToPixel( Size ( 176 - COL_DIFF, 60), MAP_APPFONT ) );
    maCbbPath.Show();
    maCbbPath.SetBaseURL(SvtPathOptions().GetWorkPath());
//  maCbbPath.SetHelpId( HID_HYPERDLG_DOC_PATH );

    // set defaults
    maRbtEditNow.Check();

    maBtCreate.SetClickHdl        ( LINK ( this, SvxHyperlinkNewDocTp, ClickNewHdl_Impl ) );

    FillDocumentList ();
}

SvxHyperlinkNewDocTp::~SvxHyperlinkNewDocTp ()
{
    for ( int n=0; n<maLbDocTypes.GetEntryCount(); n++ )
    {
        DocumentTypeData* pTypeData = (DocumentTypeData*)
                                      maLbDocTypes.GetEntryData ( n );
        delete pTypeData;
    }
}

/*************************************************************************
|*
|* Fill the all dialog-controls except controls in groupbox "more..."
|*
|************************************************************************/


void SvxHyperlinkNewDocTp::FillDlgFields ( String& aStrURL )
{
}

#define INTERNETSHORTCUT_ID_TAG       "InternetShortcut"
#define INTERNETSHORTCUT_TITLE_TAG    "Title"
#define INTERNETSHORTCUT_TARGET_TAG   "Target"
#define INTERNETSHORTCUT_FOLDER_TAG   "Folder"
#define INTERNETSHORTCUT_URL_TAG      "URL"
#define INTERNETSHORTCUT_ICONID_TAG   "IconIndex"

void SvxHyperlinkNewDocTp::ReadURLFile( const String& rFile, String& rTitle, String& rURL, sal_Int32& rIconId, BOOL* pShowAsFolder )
{
    // Open file
    Config aCfg( rFile );
    aCfg.SetGroup( INTERNETSHORTCUT_ID_TAG );

    // read URL
    rURL = aCfg.ReadKey( ByteString( RTL_CONSTASCII_STRINGPARAM( INTERNETSHORTCUT_URL_TAG) ), RTL_TEXTENCODING_ASCII_US );
    SvtPathOptions aPathOpt;
    rURL = aPathOpt.SubstituteVariable( rURL );

    // read target
    if ( pShowAsFolder )
    {
        String aTemp( aCfg.ReadKey( ByteString( RTL_CONSTASCII_STRINGPARAM( INTERNETSHORTCUT_TARGET_TAG ) ), RTL_TEXTENCODING_ASCII_US ) );
        *pShowAsFolder = aTemp == String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( INTERNETSHORTCUT_FOLDER_TAG ) );
    }

    // read image-ID
    String aStrIconId( aCfg.ReadKey( ByteString( RTL_CONSTASCII_STRINGPARAM( INTERNETSHORTCUT_ICONID_TAG ) ), RTL_TEXTENCODING_ASCII_US ) );
    rIconId = aStrIconId.ToInt32();

    // read title
    String aLangStr = aPathOpt.SubstituteVariable( DEFINE_CONST_UNICODE("$(vlang)") );
    ByteString aLang( aLangStr, RTL_TEXTENCODING_UTF8 );
    ByteString aGroup = INTERNETSHORTCUT_ID_TAG;
    ( ( aGroup += '-' ) += aLang ) += ".W";
    aCfg.SetGroup( aGroup );
    rTitle = String( aCfg.ReadKey( INTERNETSHORTCUT_TITLE_TAG ), RTL_TEXTENCODING_UTF7 );
}

void SvxHyperlinkNewDocTp::FillDocumentList ()
{
    EnterWait();

    uno::Sequence< uno::Sequence< beans::PropertyValue > >
        aDynamicMenuEntries( SvtDynamicMenuOptions().GetMenu( E_NEWMENU ) );

    sal_uInt32 i, nCount = aDynamicMenuEntries.getLength();
    for ( i = 0; i < nCount; i++ )
    {
        uno::Sequence< beans::PropertyValue >& rDynamicMenuEntry = aDynamicMenuEntries[ i ];

        rtl::OUString aDocumentUrl, aTitle, aImageId, aTargetName;

           for ( int i = 0; i < rDynamicMenuEntry.getLength(); i++ )
        {
            if ( rDynamicMenuEntry[ i ].Name == DYNAMICMENU_PROPERTYNAME_URL )
                rDynamicMenuEntry[ i ].Value >>= aDocumentUrl;
            else if ( rDynamicMenuEntry[i].Name == DYNAMICMENU_PROPERTYNAME_TITLE )
                rDynamicMenuEntry[i].Value >>= aTitle;
            else if ( rDynamicMenuEntry[i].Name == DYNAMICMENU_PROPERTYNAME_IMAGEIDENTIFIER )
                rDynamicMenuEntry[i].Value >>= aImageId;
            else if ( rDynamicMenuEntry[i].Name == DYNAMICMENU_PROPERTYNAME_TARGETNAME )
                rDynamicMenuEntry[i].Value >>= aTargetName;
        }

        // Insert into listbox
        if ( aDocumentUrl.getLength() )
        {
            if ( aDocumentUrl.equalsAscii( "private:factory/simpress?slot=6686" ) )             // SJ: #106216# do not start
                aDocumentUrl = String( RTL_CONSTASCII_USTRINGPARAM( "private:factory/simpress" ) ); // the AutoPilot for impress

            // insert private-url and default-extension as user-data
            const SfxFilter* pFilter = SfxFilter::GetDefaultFilterFromFactory( aDocumentUrl );
            if ( pFilter )
            {
                // insert doc-name and image
                String aTitleName( aTitle );
                aTitleName.Erase( aTitleName.Search( (sal_Unicode)'~' ), 1 );

                sal_Int16 nPos = maLbDocTypes.InsertEntry ( aTitleName );
                String aStrDefExt( pFilter->GetDefaultExtension () );
                DocumentTypeData *pTypeData = new DocumentTypeData ( aDocumentUrl, aStrDefExt.Copy( 2, aStrDefExt.Len() ) );
                maLbDocTypes.SetEntryData ( nPos, pTypeData );
            }
        }
    }
    maLbDocTypes.SelectEntryPos ( 0 );

    LeaveWait();
}

/*************************************************************************
|*
|* retrieve and prepare data from dialog-fields
|*
|************************************************************************/

void SvxHyperlinkNewDocTp::GetCurentItemData ( String& aStrURL, String& aStrName,
                                               String& aStrIntName, String& aStrFrame,
                                               SvxLinkInsertMode& eMode )
{
    // get data from dialog-controls
    aStrURL = maCbbPath.GetText();
    INetURLObject aURL;
    if ( ImplGetURLObject( aStrURL, maCbbPath.GetBaseURL(), aURL ) )
    {
        aStrURL     = aURL.GetMainURL( INetURLObject::NO_DECODE );
    }

    GetDataFromCommonFields( aStrName, aStrIntName, aStrFrame, eMode );
}

/*************************************************************************
|*
|* static method to create Tabpage
|*
|************************************************************************/

IconChoicePage* SvxHyperlinkNewDocTp::Create( Window* pWindow, const SfxItemSet& rItemSet )
{
    return( new SvxHyperlinkNewDocTp( pWindow, rItemSet ) );
}

/*************************************************************************
|*
|* Set initial focus
|*
|************************************************************************/

void SvxHyperlinkNewDocTp::SetInitFocus()
{
    maCbbPath.GrabFocus();
}

/*************************************************************************
|*
|* Ask page whether an insert is possible
|*
\************************************************************************/

BOOL SvxHyperlinkNewDocTp::AskApply()
{
    INetURLObject aINetURLObject;
    sal_Bool bRet = ImplGetURLObject( maCbbPath.GetText(), maCbbPath.GetBaseURL(), aINetURLObject );
    if ( !bRet )
    {
        WarningBox aWarning( this, WB_OK, SVX_RESSTR(RID_SVXSTR_HYPDLG_NOVALIDFILENAME) );
        aWarning.Execute();
    }
    return bRet;
}

/*************************************************************************
|*
|* Any action to do after apply-button is pressed
|*
\************************************************************************/

void SvxHyperlinkNewDocTp::DoApply ()
{
    EnterWait();

    // get data from dialog-controls
    String aStrNewName = maCbbPath.GetText();

    if ( aStrNewName == aEmptyStr )
        aStrNewName = maStrInitURL;

    ///////////////////////////////////////////////////////
    // create a real URL-String

    INetURLObject aURL;
    if ( ImplGetURLObject( aStrNewName, maCbbPath.GetBaseURL(), aURL ) )
    {

        ///////////////////////////////////////////////////////
        // create Document

        aStrNewName = aURL.GetURLPath( INetURLObject::NO_DECODE );
        SfxViewFrame *pViewFrame = NULL;
        try
        {
            bool bCreate = true;

            // check if file exists, warn before we overwrite it
            {
                com::sun::star::uno::Reference < com::sun::star::task::XInteractionHandler > xHandler;
                SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ, xHandler );

                sal_Bool bOk = pIStm && ( pIStm->GetError() == 0);

                if( pIStm )
                    delete pIStm;

                if( bOk )
                {
                    WarningBox aWarning( this, WB_YES_NO, SVX_RESSTR(RID_SVXSTR_HYPERDLG_QUERYOVERWRITE) );
                    bCreate = aWarning.Execute() == BUTTON_YES;
                }
            }

            if( bCreate )
            {
                // current document
                SfxViewFrame* pCurrentDocFrame = SFX_APP()->GetViewFrame();

                if ( aStrNewName != aEmptyStr )
                {
                    // get private-url
                    int nPos = maLbDocTypes.GetSelectEntryPos();
                    if( nPos == LISTBOX_ENTRY_NOTFOUND )
                        nPos=0;
                    String aStrDocName ( ( ( DocumentTypeData* )
                                         maLbDocTypes.GetEntryData( nPos ) )->aStrURL );

                    // create items
                    SfxStringItem aName( SID_FILE_NAME, aStrDocName );
                    SfxStringItem aReferer( SID_REFERER, UniString::CreateFromAscii(
                                                RTL_CONSTASCII_STRINGPARAM( "private:user" ) ) );
                    SfxStringItem aFrame( SID_TARGETNAME, UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "_blank" ) ) );
                    //SfxBoolItem aFrame( SID_OPEN_NEW_VIEW, TRUE );

                    String aStrFlags ( sal_Unicode('S') );
                    if ( maRbtEditLater.IsChecked() )
                    {
                        aStrFlags += sal_Unicode('H');
                    }
                    SfxStringItem aFlags (SID_OPTIONS, aStrFlags);

                    // open url
                    const SfxPoolItem* pReturn = GetDispatcher()->Execute( SID_OPENDOC,
                                                                           SFX_CALLMODE_SYNCHRON,
                                                                           &aName, &aFlags,
                                                                           &aFrame, &aReferer, 0L );

                    // save new doc
                    const SfxViewFrameItem *pItem = PTR_CAST( SfxViewFrameItem, pReturn );  // SJ: pReturn is NULL if the Hyperlink
                    if ( pItem )                                                            // creation is cancelled #106216#
                    {
                        pViewFrame = pItem->GetFrame();
                        if (pViewFrame)
                        {
                            //SfxViewFrame *pViewFrame = pFrame->GetCurrentViewFrame();
                            SfxStringItem aNewName( SID_FILE_NAME, aURL.GetMainURL( INetURLObject::NO_DECODE ) );

                            pViewFrame->GetDispatcher()->Execute( SID_SAVEASDOC,
                                                                  SFX_CALLMODE_SYNCHRON,
                                                                  &aNewName, 0L );

                        }
                    }
                }

                if ( maRbtEditNow.IsChecked() )
                {
                    pCurrentDocFrame->ToTop();
                }
            }
        }
        catch( uno::Exception )
        {
        }

        if ( pViewFrame && maRbtEditLater.IsChecked() )
        {
            SfxObjectShell* pObjShell = pViewFrame->GetObjectShell();
            pObjShell->DoClose();
        }
    }

    LeaveWait();
}

/*************************************************************************
|*
|* Click on imagebutton : new
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkNewDocTp, ClickNewHdl_Impl, void *, EMPTYARG )
{
    rtl::OUString                       aService( RTL_CONSTASCII_USTRINGPARAM( FOLDER_PICKER_SERVICE_NAME ) );
    Reference < XMultiServiceFactory >  xFactory( ::comphelper::getProcessServiceFactory() );
    Reference < XFolderPicker >         xFolderPicker( xFactory->createInstance( aService ), UNO_QUERY );

    String              aStrURL;
    String              aTempStrURL( maCbbPath.GetText() );
    utl::LocalFileHelper::ConvertSystemPathToURL( aTempStrURL, maCbbPath.GetBaseURL(), aStrURL );

    String              aStrPath = aStrURL;
    BOOL                bZeroPath = ( aStrPath.Len() == 0 );
    BOOL                bHandleFileName = bZeroPath;    // when path has length of 0, then the rest should always be handled
                                                        //  as file name, otherwise we do not yet know

    if( bZeroPath )
        aStrPath = SvtPathOptions().GetWorkPath();
    else if( !::utl::UCBContentHelper::IsFolder( aStrURL ) )
        bHandleFileName = TRUE;

    xFolderPicker->setDisplayDirectory( aStrPath );

    if( xFolderPicker->execute() == ExecutableDialogResults::OK )
    {
        sal_Char const  sSlash[] = "/";

        INetURLObject   aURL( aStrURL, INET_PROT_FILE );
        String          aStrName;
        if( bHandleFileName )
            aStrName = bZeroPath? aTempStrURL : aURL.getName();

        maCbbPath.SetBaseURL( xFolderPicker->getDirectory() );
        String          aStrTmp( xFolderPicker->getDirectory() );

        if( aStrTmp.GetChar( aStrTmp.Len() - 1 ) != sSlash[0] )
            aStrTmp.AppendAscii( sSlash );

        // append old file name
        if( bHandleFileName )
            aStrTmp += aStrName;

        INetURLObject   aNewURL( aStrTmp );

        if( aStrName.Len() > 0 && aNewURL.getExtension().Len() > 0 &&
            maLbDocTypes.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND )
        {
            // get private-url
            int nPos = maLbDocTypes.GetSelectEntryPos();
            aNewURL.setExtension( ( ( DocumentTypeData* ) maLbDocTypes.GetEntryData( nPos ) )->aStrExt );
        }

        if( aNewURL.GetProtocol() == INET_PROT_FILE )
        {
            utl::LocalFileHelper::ConvertURLToSystemPath( aNewURL.GetMainURL( INetURLObject::NO_DECODE ), aStrTmp );
        }
        else
        {
            aStrTmp = aNewURL.GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS );
        }

        maCbbPath.SetText ( aStrTmp );
    }
    return( 0L );
}
