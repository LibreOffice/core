/*************************************************************************
 *
 *  $RCSfile: bmkmenu.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: cd $ $Date: 2001-04-09 08:07:55 $
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

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <limits.h>

#include "classes/bmkmenu.hxx"

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_ASSERTION_HXX_
#include <macros/debug/assertion.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMANDABORTEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_ILLEGALIDENTIFIEREXCEPTION_HPP_
#include <com/sun/star/ucb/IllegalIdentifierException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_NAMECLASH_HPP_
#include <com/sun/star/ucb/NameClash.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_NUMBEREDSORTINGINFO_HPP_
#include <com/sun/star/ucb/NumberedSortingInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_TRANSFERINFO_HPP_
#include <com/sun/star/ucb/TransferInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XANYCOMPAREFACTORY_HPP_
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDINFO_HPP_
#include <com/sun/star/ucb/XCommandInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTACCESS_HPP_
#include <com/sun/star/ucb/XContentAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XDYNAMICRESULTSET_HPP_
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XSORTEDDYNAMICRESULTSETFACTORY_HPP_
#include <com/sun/star/ucb/XSortedDynamicResultSetFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#include <tools/urlobj.hxx>
#include <vcl/config.hxx>
#include <svtools/pathoptions.hxx>
#include <unotools/localfilehelper.hxx>

#include <ucbhelper/content.hxx>
#include <ucbhelper/commandenvironment.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

using namespace ::ucb;
using namespace ::rtl;
using namespace ::comphelper;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::ucb;
using namespace com::sun::star::sdbc;


#define INTERNETSHORTCUT_EXT            "url"
#define INTERNETSHORTCUT_ID_TAG         "InternetShortcut"
#define INTERNETSHORTCUT_URL_TAG        "URL"
#define INTERNETSHORTCUT_PASS_TAG       "Password"
#define INTERNETSHORTCUT_TITLE_TAG      "Title"
#define INTERNETSHORTCUT_TARGET_TAG     "Target"
#define INTERNETSHORTCUT_FOLDER_TAG     "Folder"
#define INTERNETSHORTCUT_FRAME_TAG      "Frame"
#define INTERNETSHORTCUT_BROWSER_TAG    "Browser"
#define INTERNETSHORTCUT_OPENAS_TAG     "OpenAs"
#define INTERNETSHORTCUT_TEMPLATE_TAG   "Template"
#define INTERNETSHORTCUT_DEFTEMPL_TAG   "DefaultTemplate"
#define INTERNETSHORTCUT_DEFURL_TAG     "DefaultURL"
#define INTERNETSHORTCUT_SOICON_TAG     "SOIcon"

#define URLFILE_CHARSET                 CHARSET_ANSI

#define S2U(STRING)                     ::rtl::OStringToOUString(STRING, RTL_TEXTENCODING_UTF8)
#define U2S(STRING)                     ::rtl::OUStringToOString(STRING, RTL_TEXTENCODING_UTF8)

void ReadURLFile
(
    const String& rFile,
    String& rTitle,
    String& rURL,
    String& rFrame,
    BOOL*   pShowAsFolder,
    USHORT& rImageId,
    String* pOpenAs=0,
    String* pDefTempl=0,
    String* pDefURL=0
);

Sequence < OUString > GetFolderContents( const String& rFolder, sal_Bool bFolder, sal_Bool bSorted );


class BmkMenu_Impl
{
    private:
        static USHORT       _nMID;

    public:
        String               _aURL;
        String               _aReferer;
        BmkMenu*             _pRoot;
        BOOL                 _bInitialized;

        BmkMenu_Impl( const String& rURL, BmkMenu* pRoot );
        BmkMenu_Impl( const String& rURL, const String& rReferer );
        ~BmkMenu_Impl();

        static USHORT       GetMID();
        static String       GetTitle( const String& rTitle );
};

USHORT BmkMenu_Impl::_nMID = BMKMENU_ITEMID_START;

BmkMenu_Impl::BmkMenu_Impl( const String& rURL, BmkMenu* pRoot ) :
    _aURL(rURL),
    _pRoot(pRoot),
    _bInitialized(FALSE)
{
}

BmkMenu_Impl::BmkMenu_Impl( const String& rURL, const String& rReferer ) :
    _aURL(rURL),
    _pRoot(0),
    _aReferer( rReferer ),
    _bInitialized(FALSE)
{
}

BmkMenu_Impl::~BmkMenu_Impl()
{
}

USHORT BmkMenu_Impl::GetMID()
{
    _nMID++;
    if( !_nMID )
        _nMID = BMKMENU_ITEMID_START;
    return _nMID;
}

String BmkMenu_Impl::GetTitle( const String& rStr )
{
    // "_..._" vorne rausschneiden
    String aTitle( rStr );
    if ( aTitle.Len() && aTitle.GetChar(0) == 0x005f)
    {
        aTitle.Erase( 0, 1 );
        while ( aTitle.Len() && aTitle.GetChar(0) != 0x005f )
            aTitle.Erase( 0, 1 );
        if ( aTitle.Len() )
            aTitle.Erase( 0, 1 );
        else
            aTitle = rStr;
    }

    return aTitle;
}

// ------------------------------------------------------------------------

//ImageList* BmkMenu::_pSmallImages = NULL;
//ImageList* BmkMenu::_pBigImages = NULL;

BmkMenu::BmkMenu( const String& rURL, BmkMenu* pRoot )
{
    _pImp = new BmkMenu_Impl( rURL, pRoot );
    Initialize();
}

/*
Image BmkMenu::GetImage( USHORT nId, BOOL bBig )
{
    ImageList* pList = NULL;
    if ( bBig )
    {
        if ( !_pBigImages )
            _pBigImages = new ImageList( SfxResId( RID_IMGLST_BIG ) );
        pList = _pBigImages;
    }
    else
    {
        if ( !_pSmallImages )
            _pSmallImages = new ImageList( SfxResId( RID_IMGLST_SMALL ) );
        pList = _pSmallImages;
    }
    return pList->GetImage( nId );
}
*/

BmkMenu::BmkMenu( const String& rURL, const String& rReferer )
{
    _pImp = new BmkMenu_Impl( rURL, rReferer );
    Initialize();
}

BmkMenu::~BmkMenu()
{
    delete _pImp;
}

/*
void BmkMenu::Activate()
{
    Initialize();
    _pImp->_bActivated = TRUE;
}

void BmkMenu::Deactivate()
{
    PopupMenu::Deactivate();
    _pImp->_bActivated = FALSE;
}
*/

/*
void BmkMenu::Select()
{
    String aURL( GetItemCommand( GetCurItemId() ) );
    if( !aURL.Len() )
        return;

    Reference < XFramesSupplier > xDesktop = Reference < XFramesSupplier >( ::comphelper::getProcessServiceFactory()->createInstance( DEFINE_CONST_UNICODE("com.sun.star.frame.Desktop") ), UNO_QUERY );
    Reference < XFrame > xFrame( xDesktop->getActiveFrame() );
    if ( !xFrame.is() )
        xFrame = Reference < XFrame >( xDesktop, UNO_QUERY );

    URL aTargetURL;
    aTargetURL.Complete = aURL;
    Reference < XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance( rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer" )), UNO_QUERY );
    xTrans->parseStrict( aTargetURL );

    Reference < XDispatchProvider > xProv( xFrame, UNO_QUERY );
    Reference < XDispatch > xDisp;
    if ( xProv.is() )
        if ( aTargetURL.Protocol.compareToAscii("slot:") == COMPARE_EQUAL )
            xDisp = xProv->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );
        else
            xDisp = xProv->queryDispatch( aTargetURL, ::rtl::OUString::createFromAscii("_blank"), 0 );
    if ( xDisp.is() )
    {
        SfxBmkMenu* pRoot = _pImp->_pRoot;
        if( !pRoot )
            pRoot = this;

        Sequence<PropertyValue> aArgs(1);
        PropertyValue* pArg = aArgs.getArray();
        pArg[0].Name = rtl::OUString::createFromAscii("Referer");
        pArg[0].Value <<= (::rtl::OUString) pRoot->_pImp->_aReferer;
        xDisp->dispatch( aTargetURL, aArgs );
    }
}
*/

void BmkMenu::Initialize()
{
    if( _pImp->_bInitialized )
        return;

    _pImp->_bInitialized = TRUE;

    Sequence< ::rtl::OUString > aFiles = GetFolderContents( _pImp->_aURL, sal_False, sal_True );
    const ::rtl::OUString* pFiles  = aFiles.getConstArray();
    UINT32 i, nCount = aFiles.getLength();
    for ( i = 0; i < nCount; ++i )
    {
        String aFileURL( pFiles[i] );
        INetURLObject aObj( aFileURL );

        String aTitle;
        String aURL;
        BOOL bShowAsFolder;
        String aFrame;
        USHORT nImageId = USHRT_MAX;

        ReadURLFile( aObj.GetMainURL(), aTitle, aURL, aFrame, &bShowAsFolder, nImageId );

        if ( !aTitle.Len() && !aURL.Len() )
            continue;

        if ( aTitle.GetChar(0) == '_' && aTitle.GetChar(3) == '_' )
            aTitle = aTitle.Erase( 0, 4 );

        if( aURL.EqualsAscii("private:separator") )
            InsertSeparator();
        else
        {
            aURL = SvtPathOptions().SubstituteVariable( aURL );
            USHORT nId = CreateMenuId();
            if( !bShowAsFolder )
            {
                USHORT nTitleLen = aTitle.Len();
                if( nTitleLen > 4  && aTitle.GetChar(nTitleLen-4) == '.' && aURL.EqualsAscii( "file:",0,5 ) )
                    aTitle.Erase( nTitleLen-4 );
//              Image aImage = GetImage( nImageId, FALSE );
//              InsertItem( nId, aTitle, aImage );
                InsertItem( nId, aTitle );
                SetItemCommand( nId, aURL );
            }
            else
            {
                InsertItem( nId, aTitle );
                BmkMenu* pSubMenu = new BmkMenu( aURL,
                        _pImp->_pRoot ? _pImp->_pRoot : this );
                SetPopupMenu( nId, pSubMenu );
            }
        }
    }
}

USHORT BmkMenu::CreateMenuId()
{
    return BmkMenu_Impl::GetMID();
}

String ReadURL_Impl( Config& rURLFile, const String& rFile )
{
    // read the URL from config file
    String aURL( String( rURLFile.ReadKey( INTERNETSHORTCUT_URL_TAG ), RTL_TEXTENCODING_UTF8 ) );
    aURL = SvtPathOptions().SubstituteVariable( aURL );

    // make it absolute
    INetURLObject aBase( rFile );
    LOG_ASSERT( aBase.GetProtocol() != INET_PROT_NOT_VALID, "Not a valid URL" );
    INetURLObject aOut;
    aBase.GetNewAbsURL( aURL, &aOut );
    return aOut.GetMainURL();
}

void ReadURLFile
(
    const String& rFile,      // Datei, aus der gelesen werden soll
    String& rTitle,           // Puffer fuer den Titel
    String& rURL,             // Puffer fuer den URL
    String& rFrame,           // Puffer fuer den Zielrahmen
    BOOL* pShowAsFolder,      /* Puffer fuer das Flag, ob die URL als Ordner
                                 dargestellt wird:
                                 NULL => wird nicht geliefert */
    USHORT& rImageId,         // zugeordnetes Image
    String* pOpenAs,          /* Puffer fuer die Oeffnunsart
                                 NULL => wird nicht geliefert */
    String* pDefTempl,         /* Standardvorlagenname */
    String* pDefURL
)
{
    // Oefnung der Datei
    String aPath;
    if ( ::utl::LocalFileHelper::ConvertURLToPhysicalName( rFile, aPath ) )
    {
        Config aCfg( aPath );
        aCfg.SetGroup( INTERNETSHORTCUT_ID_TAG );

        // Einlesung des URLs
        rURL = ReadURL_Impl( aCfg, rFile );

        // Einlesung des Ziels, in dem die URL angezeigt werden soll
        if ( pShowAsFolder )
        {
            ByteString aTemp( aCfg.ReadKey( INTERNETSHORTCUT_TARGET_TAG ) );
            *pShowAsFolder = aTemp == INTERNETSHORTCUT_FOLDER_TAG;
        }

        // Einlesung und Konvertierung des Zielrahmens
        rFrame = String( aCfg.ReadKey( INTERNETSHORTCUT_FRAME_TAG ), RTL_TEXTENCODING_UTF8 );

        String aImgStr = String( aCfg.ReadKey( INTERNETSHORTCUT_SOICON_TAG ), RTL_TEXTENCODING_UTF8 );
        if ( aImgStr.GetToken(0).EqualsAscii( "StarOfficeIcon" ) )
        {
            rImageId = aImgStr.GetToken(1).ToInt32();
        }

        // Einlesung des Oeffnungstyps
        if ( pOpenAs )
            *pOpenAs = String( aCfg.ReadKey( INTERNETSHORTCUT_OPENAS_TAG ), RTL_TEXTENCODING_UTF8 );

        if ( pDefTempl )
            *pDefTempl = String( aCfg.ReadKey( INTERNETSHORTCUT_DEFTEMPL_TAG ), RTL_TEXTENCODING_UTF8 );

        if ( pDefURL )
            *pDefURL = ReadURL_Impl( aCfg, rFile );

        // read and convert the title, dependent on the language
        String aLangStr = SvtPathOptions().SubstituteVariable( String::CreateFromAscii( "$(vlang)" ) );

        ByteString aLang = U2S( aLangStr );
        ByteString aGroup = INTERNETSHORTCUT_ID_TAG;
        ( ( aGroup += '-' ) += aLang ) += ".W";
        aCfg.SetGroup( aGroup );
        rTitle = String( aCfg.ReadKey( INTERNETSHORTCUT_TITLE_TAG ), RTL_TEXTENCODING_UTF7 );
    }
    else
    {
        LOG_ASSERT( sal_False, "Bookmark is not in the local file system!" );
    }
}


DECLARE_LIST( StringList_Impl, OUString* );

Sequence < OUString > GetFolderContents( const String& rFolder, sal_Bool bFolder, sal_Bool bSorted )
{
    StringList_Impl* pFiles = NULL;
    INetURLObject aFolderObj( rFolder );
    LOG_ASSERT( aFolderObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );
    try
    {
        Content aCnt( aFolderObj.GetMainURL(), Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
        Reference< XResultSet > xResultSet;
        Sequence< OUString > aProps(2);
        OUString* pProps = aProps.getArray();
        pProps[0] = OUString::createFromAscii( "Title" );
        pProps[1] = OUString::createFromAscii( "IsFolder" );

        try
        {
            ResultSetInclude eInclude = bFolder ? INCLUDE_FOLDERS_AND_DOCUMENTS : INCLUDE_DOCUMENTS_ONLY;
            if ( !bSorted )
            {
                xResultSet = aCnt.createCursor( aProps, eInclude );
            }
            else
            {
                Reference< com::sun::star::ucb::XDynamicResultSet > xDynResultSet;
                xDynResultSet = aCnt.createDynamicCursor( aProps, eInclude );

                Reference < com::sun::star::ucb::XAnyCompareFactory > xFactory;
                Reference < XMultiServiceFactory > xMgr = getProcessServiceFactory();
                Reference < com::sun::star::ucb::XSortedDynamicResultSetFactory > xSRSFac(
                    xMgr->createInstance( ::rtl::OUString::createFromAscii("com.sun.star.ucb.SortedDynamicResultSetFactory") ), UNO_QUERY );

                Sequence< com::sun::star::ucb::NumberedSortingInfo > aSortInfo( 2 );
                com::sun::star::ucb::NumberedSortingInfo* pInfo = aSortInfo.getArray();
                pInfo[ 0 ].ColumnIndex = 2;
                pInfo[ 0 ].Ascending   = sal_False;
                pInfo[ 1 ].ColumnIndex = 1;
                pInfo[ 1 ].Ascending   = sal_True;

                Reference< com::sun::star::ucb::XDynamicResultSet > xDynamicResultSet;
                xDynamicResultSet =
                    xSRSFac->createSortedDynamicResultSet( xDynResultSet, aSortInfo, xFactory );
                if ( xDynamicResultSet.is() )
                {
                    sal_Int16 nCaps = xDynamicResultSet->getCapabilities();
                    xResultSet = xDynamicResultSet->getStaticResultSet();
                }
            }
        }
        catch( ::com::sun::star::ucb::CommandAbortedException& )
        {
            LOG_ASSERT( sal_False, "createCursor: CommandAbortedException" );
        }
        catch( ... )
        {
            LOG_ASSERT( sal_False, "createCursor: Any other exception" );
        }

        if ( xResultSet.is() )
        {
            pFiles = new StringList_Impl;
            Reference< com::sun::star::ucb::XContentAccess > xContentAccess( xResultSet, UNO_QUERY );
            try
            {
                while ( xResultSet->next() )
                {
                    OUString aId = xContentAccess->queryContentIdentifierString();
                    OUString* pFile = new OUString( aId );
                    pFiles->Insert( pFile, LIST_APPEND );
                }
            }
            catch( ::com::sun::star::ucb::CommandAbortedException& )
            {
                LOG_ASSERT( sal_False, "XContentAccess::next(): CommandAbortedException" );
            }
            catch( ... )
            {
                LOG_ASSERT( sal_False, "XContentAccess::next(): Any other exception" );
            }
        }
    }
    catch( ... )
    {
        LOG_ASSERT( sal_False, "GetFolderContents: Any other exception" );
    }

    if ( pFiles )
    {
        ULONG nCount = pFiles->Count();
        Sequence < OUString > aRet( nCount );
        OUString* pRet = aRet.getArray();
        for ( ULONG i = 0; i < nCount; ++i )
        {
            OUString* pFile = pFiles->GetObject(i);
            pRet[i] = *( pFile );
            delete pFile;
        }
        delete pFiles;
        return aRet;
    }
    else
        return Sequence < OUString > ();
}
