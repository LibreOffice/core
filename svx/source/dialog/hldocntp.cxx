/*************************************************************************
 *
 *  $RCSfile: hldocntp.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: cl $ $Date: 2001-03-20 09:42:34 $
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
#ifndef _UCBHELPER_CONTENTBROKER_HXX
#include <ucbhelper/contentbroker.hxx>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTACCESS_HPP_
#include <com/sun/star/ucb/XContentAccess.hpp>
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

#include "hyperdlg.hrc"

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
    InitStdControls();
    FreeResource();

    SetExchangeSupport ();

    maCbbPath.SetPosSizePixel ( LogicToPixel( Point( 54 , 25 ), MAP_APPFONT ),
                                LogicToPixel( Size ( 176, 60), MAP_APPFONT ) );
    maCbbPath.Show();
    maCbbPath.SetBaseURL(SvtPathOptions().GetWorkPath());
//  maCbbPath.SetHelpId( HID_HYPERDLG_DOC_PATH );

    // set defaults
    maRbtEditNow.Check();

    maBtCreate.SetClickHdl        ( LINK ( this, SvxHyperlinkNewDocTp, ClickNewHdl_Impl ) );
    maCbbPath.SetLoseFocusHdl      ( LINK ( this, SvxHyperlinkNewDocTp, LostFocusTargetHdl_Impl ) );
    maCbbPath.SetModifyHdl        ( LINK ( this, SvxHyperlinkNewDocTp, ModifiedPathHdl_Impl ) );
    maLbDocTypes.SetDoubleClickHdl( LINK ( this, SvxHyperlinkNewDocTp, DClickDocTypeHdl_Impl ) );
    maLbDocTypes.SetSelectHdl( LINK ( this, SvxHyperlinkNewDocTp, SelectDocTypeHdl_Impl ) );


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
    SvtPathOptions aPathOpt;
    String aStrDirName( aPathOpt.GetNewMenuPath() );
    ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aStrDirName, aStrDirName );
    INetURLObject aFolderObj( aStrDirName );
    try
    {
        ::ucb::Content aCnt( aFolderObj.GetMainURL(), uno::Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
        uno::Reference< sdbc::XResultSet > xResultSet;
        uno::Sequence< OUString > aProps(1);
        OUString* pProps = aProps.getArray();
        pProps[0] == OUString::createFromAscii( "Url" );

        try
        {
            xResultSet = aCnt.createCursor( aProps, ::ucb::INCLUDE_DOCUMENTS_ONLY );
        }
        catch ( uno::Exception )
        {
            DBG_ERRORFILE( "Hyperlink-dialog:'New Document':create cursor failed!" );
        }

        if ( xResultSet.is() )
        {
            uno::Reference< com::sun::star::ucb::XContentAccess > xContentAccess( xResultSet, uno::UNO_QUERY );
            try
            {
                while ( xResultSet->next() )
                {
                    String aFileURL = xContentAccess->queryContentIdentifierString();

                    String aTitle;
                    String aURL;
                    BOOL bShowAsFolder;
                    sal_Int32 nIconId;

                    ::utl::LocalFileHelper::ConvertURLToPhysicalName( aFileURL, aFileURL );
                    ReadURLFile( aFileURL, aTitle, aURL, nIconId, &bShowAsFolder);//, nImageId );

                    if( aURL.Len() && aTitle.Len() && !aURL.EqualsAscii("private:separator") &&
                        aURL.SearchAscii( "slot" ) == STRING_NOTFOUND && aURL.SearchAscii( "?" ) == STRING_NOTFOUND )
                    {
                        if ( aTitle.GetChar(0) == '_' && aTitle.GetChar(3) == '_' )
                            aTitle = aTitle.Erase( 0, 4 );

                        sal_Char const sTilde[] = "~";
                        aTitle.Erase ( aTitle.SearchAscii( sTilde ), 1 );

                        aURL = aPathOpt.SubstituteVariable( aURL );

                        if( !bShowAsFolder )
                        {
                            USHORT nTitleLen = aTitle.Len();
                            if( nTitleLen > 4  && aTitle.GetChar(nTitleLen-4) == '.' && aURL.EqualsAscii( "file:",0,5 ) )
                                aTitle.Erase( nTitleLen-4 );

                            // Insert into listbox
                            const SfxObjectFactory* pFactory = SfxObjectFactory::GetFactory ( aURL );
                            if ( pFactory )
                            {
                                // insert doc-name and image
                                int nPos = maLbDocTypes.InsertEntry ( aTitle );//, aImage );

                                // insert private-url and default-extension as user-data
                                String aStrDefExt ( pFactory->GetFilter (0)->GetDefaultExtension () );
                                DocumentTypeData *pTypeData = new DocumentTypeData ( aURL,
                                                                  aStrDefExt.Copy( 2, aStrDefExt.Len() ) );
                                maLbDocTypes.SetEntryData ( nPos, pTypeData );
                            }
                        }
                    }
                }
            }
            catch ( uno::Exception )
            {
                DBG_ERRORFILE( "Hyperlink-dialog:'New Document':Retrieve fileinfos failed!" );
            }
        }
    }
    catch( uno::Exception )
    {
        DBG_ERRORFILE( "Hyperlink-dialog:'New Document':Fill listfield failed!" );
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
    if( aStrURL.Len() )
    {
        utl::LocalFileHelper::ConvertSystemPathToURL( aStrURL, maCbbPath.GetBaseURL(), aStrURL );
        if( aStrURL.Len() == 0 )
        {
            INetURLObject aTmpURL( SvtPathOptions().GetWorkPath(), INET_PROT_FILE );
            if( !aTmpURL.hasFinalSlash() )
                aTmpURL.setFinalSlash();
            aTmpURL.Append( maCbbPath.GetText() );
            aStrURL = aTmpURL.GetMainURL();
        }
    }

    // does the filename have got a valid extension ?
    INetURLObject aURL( aStrURL );

    if ( GetName( aStrURL ) != aEmptyStr &&
         maLbDocTypes.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND )
    {
        // get private-url
        int nPos = maLbDocTypes.GetSelectEntryPos();
        aURL.setExtension ( ( ( DocumentTypeData* )
                                 maLbDocTypes.GetEntryData( nPos ) )->aStrExt );
    }

    sal_Char const sSep[] = ".";

    if( aURL.getFSysPath( INetURLObject::FSYS_DETECT ).Len() - aURL.getName().Len() <= 1 ||
        aURL.getFSysPath( INetURLObject::FSYS_DETECT ).Search( '.' ) == 0 )
    {
        INetURLObject aTmpURL( SvtPathOptions().GetWorkPath(), INET_PROT_FILE );
        if( !aTmpURL.hasFinalSlash() )
            aTmpURL.setFinalSlash();
        aTmpURL.Append( aURL.getName() );
        aURL = aTmpURL;
    }

    // get data from standard-fields
    aStrIntName = mpEdText->GetText();
    aStrName    = mpEdIndication->GetText();
    aStrFrame   = mpCbbFrame->GetText();
    eMode       = (SvxLinkInsertMode) (mpLbForm->GetSelectEntryPos()+1);
    if( IsHTMLDoc() )
        eMode = (SvxLinkInsertMode) ( UINT16(eMode) | HLINK_HTMLMODE );

    if ( aStrURL != aEmptyStr )
        aStrURL = aURL.GetMainURL( INetURLObject::DECODE_WITH_CHARSET );

    if ( aStrName == aEmptyStr )
        aStrName = aStrURL;
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
|* Activate / Deactivate Tabpage
|*
|************************************************************************/

void SvxHyperlinkNewDocTp::ActivatePage( const SfxItemSet& rItemSet )
{
    ///////////////////////////////////////
    // Set dialog-fields from input-itemset
    SvxHyperlinkItem *pHyperlinkItem = (SvxHyperlinkItem *)
                                       rItemSet.GetItem (SID_HYPERLINK_GETLINK);

    if ( pHyperlinkItem )
    {
        // standard-fields
        FillStandardDlgFields (pHyperlinkItem);

        mbNewName = ( pHyperlinkItem->GetName() == aEmptyStr );
    }
}

int SvxHyperlinkNewDocTp::DeactivatePage( SfxItemSet* pSet )
{
    String aStrURL, aStrName, aStrIntName, aStrFrame;
    SvxLinkInsertMode eMode;

    GetCurentItemData ( aStrURL, aStrName, aStrIntName, aStrFrame, eMode);

    USHORT nEvents = GetMacroEvents();
    SvxMacroTableDtor* pTable = GetMacroTable();

    SvxHyperlinkItem aItem( SID_HYPERLINK_GETLINK, aStrName, aStrURL, aStrFrame,
                            aStrIntName, eMode, nEvents, pTable );
    pSet->Put (aItem);

    return( LEAVE_PAGE );
}

/*************************************************************************
|*
|* Fill output-ItemSet
|*
|*
|************************************************************************/

BOOL SvxHyperlinkNewDocTp::FillItemSet( SfxItemSet& rOut)
{
    String aStrURL, aStrName, aStrIntName, aStrFrame;
    SvxLinkInsertMode eMode;

    GetCurentItemData ( aStrURL, aStrName, aStrIntName, aStrFrame, eMode);

    // put data into itemset
    USHORT nEvents = GetMacroEvents();
    SvxMacroTableDtor* pTable = GetMacroTable();

    SvxHyperlinkItem aItem( SID_HYPERLINK_SETLINK, aStrName, aStrURL, aStrFrame,
                            aStrIntName, eMode, nEvents, pTable );
    rOut.Put (aItem);

    return TRUE;
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

BOOL SvxHyperlinkNewDocTp::AskApply ()
{
    if( GetName( maCbbPath.GetText() ) != aEmptyStr )
        return TRUE;

    WarningBox aWarning( this, WB_OK, SVX_RESSTR(RID_SVXSTR_HYPDLG_NOVALIDFILENAME) );
    aWarning.Execute();

    return FALSE;
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

    if( aStrNewName.Len() )
    {
        String aTempName;
        utl::LocalFileHelper::ConvertSystemPathToURL( aStrNewName, maBaseURL, aTempName );
        if( aTempName.Len() == 0 )
        {
            INetURLObject aTmpURL( SvtPathOptions().GetWorkPath() );
            if( !aTmpURL.hasFinalSlash() )
                aTmpURL.setFinalSlash();
            aTmpURL.Append( aStrNewName );
            aTempName = aTmpURL.GetMainURL();
        }

        aStrNewName = aTempName;
    }

    INetURLObject aURL( aStrNewName );

    int nPos = maLbDocTypes.GetSelectEntryPos();
    if( nPos == LISTBOX_ENTRY_NOTFOUND )
        nPos=0;
    aURL.setExtension ( ( ( DocumentTypeData* )
                             maLbDocTypes.GetEntryData( nPos ) )->aStrExt );

    if( aURL.getFSysPath( INetURLObject::FSYS_DETECT ).Len() - aURL.getName().Len() <= 1 ||
        aURL.getFSysPath( INetURLObject::FSYS_DETECT ).Search( '.' ) == 0 )
    {
        INetURLObject aTmpURL( SvtPathOptions().GetWorkPath() );
        if( !aTmpURL.hasFinalSlash() )
            aTmpURL.setFinalSlash();
        aTmpURL.Append( aURL.getName() );
        aURL = aTmpURL;
    }
    ///////////////////////////////////////////////////////
    // create Document
    aStrNewName = aURL.GetURLPath( INetURLObject::DECODE_WITH_CHARSET );//INetURLObject::FSYS_DETECT );
    sal_Char const sSlash[] = "/";
    if( aStrNewName.SearchAscii( sSlash ) == 0 )
        aStrNewName.Erase( 0, 1 );

    SfxViewFrame *pViewFrame = NULL;
    try
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
            const SfxViewFrameItem *pItem = PTR_CAST( SfxViewFrameItem, pReturn );
            pViewFrame = pItem->GetFrame();
            if (pViewFrame)
            {
                //SfxViewFrame *pViewFrame = pFrame->GetCurrentViewFrame();
                SfxStringItem aNewName( SID_FILE_NAME, aURL.GetMainURL() );

                pViewFrame->GetDispatcher()->Execute( SID_SAVEASDOC,
                                                      SFX_CALLMODE_SYNCHRON,
                                                      &aNewName, 0L );

            }
        }

        if ( maRbtEditNow.IsChecked() )
        {
            pCurrentDocFrame->ToTop();
        }
    }
    catch( uno::Exception )
    {
    }

    if ( pViewFrame && maRbtEditLater.IsChecked() )
    {
        SfxObjectShell* pObjShell = pViewFrame->GetObjectShell();
        BOOL bResult = pObjShell->DoClose();
        pObjShell->OwnerLock(FALSE);
    }

    LeaveWait();
}

/*************************************************************************
|*
|* reset dialog-fields
|*
|************************************************************************/

void SvxHyperlinkNewDocTp::Reset( const SfxItemSet& rItemSet)
{
    ///////////////////////////////////////
    // Set dialog-fields from create-itemset
    maStrInitURL = aEmptyStr;

    SvxHyperlinkItem *pHyperlinkItem = (SvxHyperlinkItem *)
                                       rItemSet.GetItem (SID_HYPERLINK_GETLINK);

    if ( pHyperlinkItem )
    {
        // set dialog-fields
        FillStandardDlgFields (pHyperlinkItem);

        // set all other fields
        FillDlgFields ( (String&)pHyperlinkItem->GetURL() );

        // Store initial URL
        maStrInitURL = pHyperlinkItem->GetURL();
        maBaseURL = pHyperlinkItem->GetURL();

        mbNewName = ( pHyperlinkItem->GetName() == aEmptyStr );
    }
}

/*************************************************************************
|*
|* Click on imagebutton : new
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkNewDocTp, ClickNewHdl_Impl, void *, EMPTYARG )
{
    SfxFileDialog aDlg( SFX_APP()->GetTopWindow(), WB_3DLOOK | WB_OPEN |
                        SFXWB_PATHDIALOG );

    String aStrURL( maCbbPath.GetText() );
    utl::LocalFileHelper::ConvertSystemPathToURL( aStrURL, maCbbPath.GetBaseURL(), aStrURL );

    String aStrPath = GetPath ( aStrURL );
    String aStrName = GetName ( aStrURL );

    if ( aStrPath == aEmptyStr )
        aStrPath = SvtPathOptions().GetWorkPath();

    aDlg.SetPath ( aStrPath );

    if ( aDlg.Execute() == RET_OK )
    {
        sal_Char const sSlash[] = "/";

        maCbbPath.SetBaseURL( aDlg.GetPath() );
        String aStrTmp( aDlg.GetPath() );

        if( aStrTmp.GetChar( aStrTmp.Len() - 1 ) != sSlash[0] )
            aStrTmp.AppendAscii( sSlash );

        aStrTmp += aStrName;
        INetURLObject aNewURL( aStrTmp );

        if ( aStrName != aEmptyStr && aNewURL.getExtension() != aEmptyStr &&
             maLbDocTypes.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND )
        {
            // get private-url
            int nPos = maLbDocTypes.GetSelectEntryPos();
            aNewURL.setExtension ( ( ( DocumentTypeData* )
                                     maLbDocTypes.GetEntryData( nPos ) )->aStrExt );
        }

        aStrTmp = aNewURL.GetURLPath( INetURLObject::DECODE_WITH_CHARSET );
        if( aStrTmp.SearchAscii( sSlash ) == 0 )
            aStrTmp.Erase( 0, 1 );

        maCbbPath.SetText ( aStrTmp );
    }

    return( 0L );
}

/*************************************************************************
|*
|* Contens of combobox "Path" modified
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkNewDocTp, ModifiedPathHdl_Impl, void *, EMPTYARG )
{
    if ( mbNewName )
        mpEdIndication->SetText ( maCbbPath.GetText() );

    return( 0L );
}

/*************************************************************************
|*
|* Combobox Target lost the focus
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkNewDocTp, LostFocusTargetHdl_Impl, void *, EMPTYARG )
{
    UpdateExtension();

    return (0L);
}

/*************************************************************************
|*
|* Double-Click in Listbox
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkNewDocTp, DClickDocTypeHdl_Impl, void *, EMPTYARG )
{
    if (maCbbPath.GetText().Len() )
        LostFocusTargetHdl_Impl (NULL);

    return (0L);
}

IMPL_LINK ( SvxHyperlinkNewDocTp, SelectDocTypeHdl_Impl, void *, EMPTYARG )
{

    UpdateExtension();

    return (0L);
}

/*************************************************************************
|*
|* Sets the extension according to the selected doc type
|*
|************************************************************************/
void SvxHyperlinkNewDocTp::UpdateExtension()
{
    String aStrURL( maCbbPath.GetText() );

    const int nDocTypePos = maLbDocTypes.GetSelectEntryPos();

    // no file? no doctype? no extension!
    if( aStrURL.Len() == 0 || nDocTypePos == LISTBOX_ENTRY_NOTFOUND )
        return;

    utl::LocalFileHelper::ConvertSystemPathToURL( aStrURL, maCbbPath.GetBaseURL(), aStrURL );

    // if we have a valid url, use the INetURLObject way
    if( aStrURL.Len() )
    {
        // check for file name
        if ( GetName ( aStrURL ) != aEmptyStr )
        {
            INetURLObject aURL( aStrURL );

            // get private-url
            aURL.setExtension(((DocumentTypeData*)maLbDocTypes.GetEntryData( nDocTypePos ) )->aStrExt);

            String aStrTmp( aURL.GetURLPath( INetURLObject::DECODE_WITH_CHARSET ) );
            sal_Char const sSlash[] = "/";
            if( aStrTmp.SearchAscii( sSlash ) == 0 )
                aStrTmp.Erase( 0, 1 );
            maCbbPath.SetText ( aStrTmp );

            ModifiedPathHdl_Impl ( NULL );
        }
    }
    else
    {
        aStrURL = maCbbPath.GetText();
        // since we have no valid url yet, maybe just a file name
        // we must add the extensions ourselfs

        xub_StrLen nIndex = aStrURL.SearchBackward( sal_Unicode('.') );
        if( nIndex != -1 )
            aStrURL = aStrURL.Copy( 0, nIndex );

        aStrURL += sal_Unicode( '.' );
        aStrURL += ((DocumentTypeData*)maLbDocTypes.GetEntryData( nDocTypePos ) )->aStrExt;
        maCbbPath.SetText( aStrURL );

        ModifiedPathHdl_Impl ( NULL );
    }
}

/*************************************************************************
|*
|* retrieve path
|*
|************************************************************************/

String SvxHyperlinkNewDocTp::GetPath ( const String& aStrFull ) const
{
    String aStrPath;
    aStrPath = aStrFull;
    return aStrPath;
}

/*************************************************************************
|*
|* retrieve filename
|*
|************************************************************************/

String SvxHyperlinkNewDocTp::GetName ( const String& aStrFull ) const
{
    String aStrName;
    INetURLObject aURL( aStrFull, INET_PROT_FILE );
    aStrName = aURL.getName();

    return aStrName;
}

