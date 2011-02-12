/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "hldocntp.hxx"
#include <sfx2/viewfrm.hxx>
#include <sfx2/docfac.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <unotools/localfilehelper.hxx>
#include <tools/config.hxx>
#include <vcl/image.hxx>
#include <tools/urlobj.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/dynamicmenuoptions.hxx>
#include <sfx2/filedlghelper.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/ucbhelper.hxx>

#include "hyperdlg.hrc"
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>

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
            bool wasAbs;
            INetURLObject base(rBase);
            base.setFinalSlash();
            aURLObject = base.smartRel2Abs(
                rPath, wasAbs, true, INetURLObject::ENCODE_ALL,
                RTL_TEXTENCODING_UTF8, true);
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
            USHORT nPos = maLbDocTypes.GetSelectEntryPos();
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
:   SvxHyperlinkTabPageBase ( pParent, CUI_RES( RID_SVXPAGE_HYPERLINK_NEWDOCUMENT ), rItemSet ),
    maGrpNewDoc     ( this, CUI_RES (GRP_NEWDOCUMENT) ),
    maRbtEditNow    ( this, CUI_RES (RB_EDITNOW) ),
    maRbtEditLater  ( this, CUI_RES (RB_EDITLATER) ),
    maFtPath        ( this, CUI_RES (FT_PATH_NEWDOC) ),
    maCbbPath       ( this, INET_PROT_FILE ),
    maBtCreate      ( this, CUI_RES (BTN_CREATE) ),
    maFtDocTypes    ( this, CUI_RES (FT_DOCUMENT_TYPES) ),
    maLbDocTypes    ( this, CUI_RES (LB_DOCUMENT_TYPES) )
{
    // Set HC bitmaps and disable display of bitmap names.
    maBtCreate.EnableTextDisplay (FALSE);

    InitStdControls();
    FreeResource();

    SetExchangeSupport ();

    maCbbPath.SetPosSizePixel ( LogicToPixel( Point( COL_2 , 25 ), MAP_APPFONT ),
                                LogicToPixel( Size ( 176 - COL_DIFF, 60), MAP_APPFONT ) );
    maCbbPath.Show();
    maCbbPath.SetBaseURL(SvtPathOptions().GetWorkPath());

    // set defaults
    maRbtEditNow.Check();

    maBtCreate.SetClickHdl        ( LINK ( this, SvxHyperlinkNewDocTp, ClickNewHdl_Impl ) );

    FillDocumentList ();
}

SvxHyperlinkNewDocTp::~SvxHyperlinkNewDocTp ()
{
    for ( USHORT n=0; n<maLbDocTypes.GetEntryCount(); n++ )
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


void SvxHyperlinkNewDocTp::FillDlgFields ( String& /*aStrURL*/ )
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

           for ( int e = 0; e < rDynamicMenuEntry.getLength(); e++ )
        {
            if ( rDynamicMenuEntry[ e ].Name == DYNAMICMENU_PROPERTYNAME_URL )
                rDynamicMenuEntry[ e ].Value >>= aDocumentUrl;
            else if ( rDynamicMenuEntry[e].Name == DYNAMICMENU_PROPERTYNAME_TITLE )
                rDynamicMenuEntry[e].Value >>= aTitle;
            else if ( rDynamicMenuEntry[e].Name == DYNAMICMENU_PROPERTYNAME_IMAGEIDENTIFIER )
                rDynamicMenuEntry[e].Value >>= aImageId;
            else if ( rDynamicMenuEntry[e].Name == DYNAMICMENU_PROPERTYNAME_TARGETNAME )
                rDynamicMenuEntry[e].Value >>= aTargetName;
        }
        //#i96822# business cards, labels and database should not be inserted here
        if( aDocumentUrl.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "private:factory/swriter?slot=21051" ) ) ||
                aDocumentUrl.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "private:factory/swriter?slot=21052" )) ||
                aDocumentUrl.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "private:factory/sdatabase?Interactive" )))
            continue;

        // Insert into listbox
        if ( aDocumentUrl.getLength() )
        {
            if ( aDocumentUrl.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "private:factory/simpress?slot=6686" ) ) )              // SJ: #106216# do not start
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
        WarningBox aWarning( this, WB_OK, CUI_RESSTR(RID_SVXSTR_HYPDLG_NOVALIDFILENAME) );
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
                    WarningBox aWarning( this, WB_YES_NO, CUI_RESSTR(RID_SVXSTR_HYPERDLG_QUERYOVERWRITE) );
                    bCreate = aWarning.Execute() == BUTTON_YES;
                }
            }

            if( bCreate )
            {
                // current document
                SfxViewFrame* pCurrentDocFrame = SfxViewFrame::Current();

                if ( aStrNewName != aEmptyStr )
                {
                    // get private-url
                    USHORT nPos = maLbDocTypes.GetSelectEntryPos();
                    if( nPos == LISTBOX_ENTRY_NOTFOUND )
                        nPos=0;
                    String aStrDocName ( ( ( DocumentTypeData* )
                                         maLbDocTypes.GetEntryData( nPos ) )->aStrURL );

                    // create items
                    SfxStringItem aName( SID_FILE_NAME, aStrDocName );
                    SfxStringItem aReferer( SID_REFERER, UniString::CreateFromAscii(
                                                RTL_CONSTASCII_STRINGPARAM( "private:user" ) ) );
                    SfxStringItem aFrame( SID_TARGETNAME, UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "_blank" ) ) );

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
                            SfxStringItem aNewName( SID_FILE_NAME, aURL.GetMainURL( INetURLObject::NO_DECODE ) );

                            pViewFrame->GetDispatcher()->Execute( SID_SAVEASDOC,
                                                                  SFX_CALLMODE_SYNCHRON,
                                                                  &aNewName, 0L );

                        }
                    }
                }

                if ( maRbtEditNow.IsChecked() && pCurrentDocFrame )
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
    uno::Reference < XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
    uno::Reference < XFolderPicker >            xFolderPicker( xFactory->createInstance( aService ), UNO_QUERY );

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
    DisableClose( sal_True );
    sal_Int16 nResult = xFolderPicker->execute();
    DisableClose( sal_False );
    if( ExecutableDialogResults::OK == nResult )
    {
        sal_Char const  sSlash[] = "/";

        INetURLObject   aURL( aStrURL, INET_PROT_FILE );
        String          aStrName;
        if( bHandleFileName )
            aStrName = bZeroPath? aTempStrURL : String(aURL.getName());

        maCbbPath.SetBaseURL( xFolderPicker->getDirectory() );
        String          aStrTmp( xFolderPicker->getDirectory() );

        if( aStrTmp.GetChar( aStrTmp.Len() - 1 ) != sSlash[0] )
            aStrTmp.AppendAscii( sSlash );

        // append old file name
        if( bHandleFileName )
            aStrTmp += aStrName;

        INetURLObject   aNewURL( aStrTmp );

        if( aStrName.Len() > 0 && aNewURL.getExtension().getLength() > 0 &&
            maLbDocTypes.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND )
        {
            // get private-url
            USHORT nPos = maLbDocTypes.GetSelectEntryPos();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
