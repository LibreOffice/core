/*************************************************************************
 *
 *  $RCSfile: doctemplates.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: dv $ $Date: 2001-04-02 09:16:43 $
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


#include "doctemplates.hxx"

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef  _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _SV_RESARY_HXX
#include <vcl/resary.hxx>
#endif
#ifndef _SV_SETTINGS_HXX
#include <vcl/settings.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif

#ifndef  _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef  _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef  _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef  _COM_SUN_STAR_BEANS_XPROPERTYCONTAINER_HPP_
#include <com/sun/star/beans/XPropertyContainer.hpp>
#endif

#ifndef  _COM_SUN_STAR_IO_XPERSIST_HPP_
#include <com/sun/star/io/XPersist.hpp>
#endif

#ifndef  _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef  _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif

#ifndef  _COM_SUN_STAR_UCB_COMMANDABORTEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#endif
#ifndef  _COM_SUN_STAR_UCB_NAMECLASH_HPP_
#include <com/sun/star/ucb/NameClash.hpp>
#endif
#ifndef  _COM_SUN_STAR_UCB_TRANSFERINFO_HPP_
#include <com/sun/star/ucb/TransferInfo.hpp>
#endif
#ifndef  _COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTACCESS_HPP_
#include <com/sun/star/ucb/XContentAccess.hpp>
#endif

#include "sfxresid.hxx"
#include "doc.hrc"

//-----------------------------------------------------------------------------

//=============================================================================

#define TEMPLATE_SERVICE_NAME               "com.sun.star.frame.DocumentTemplates"
#define TEMPLATE_IMPLEMENTATION_NAME        "com.sun.star.comp.sfx2.DocumentTemplates"

#define HIERARCHIE_ROOT_URL     "vnd.sun.star.hier:/"
#define TEMPLATE_ROOT_URL       "vnd.sun.star.hier:/templates"
#define TEMPLATE_DIR_NAME       "templates"
#define TITLE                   "Title"
#define IS_FOLDER               "IsFolder"
#define TARGET_URL              "TargetURL"
#define TYPE_FOLDER             "application/vnd.sun.star.hier-folder"
#define TYPE_LINK               "application/vnd.sun.star.hier-link"
#define TYPE_FSYS_FOLDER        "application/vnd.sun.staroffice.fsys-folder"

#define PROPERTY_DIRLIST        "DirectoryList"
#define PROPERTY_TYPE           "TypeDescription"

#define TARGET_DIR_URL          "TargetDirURL"
#define COMMAND_DELETE          "delete"
#define COMMAND_TRANSFER        "transfer"

#define STANDARD_FOLDER         "standard"

#define SERVICENAME_TYPEDETECTION       "com.sun.star.document.TypeDetection"
#define TYPEDETECTION_PARAMETER         "FileName"
#define SERVICENAME_OLD_TYPEDETECTION   "com.sun.star.frame.FrameLoaderFactory"
#define PARAMETER_OLD_TYPEDETECTION     "DeepDetection"
#define SERVICENAME_DOCINFO             "com.sun.star.document.DocumentProperties"


#define C_DELIM                 ';'

//=============================================================================

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;

using namespace rtl;
using namespace ucb;

//=============================================================================

class WaitWindow_Impl : public WorkWindow
{
    Rectangle   _aRect;
    USHORT      _nTextStyle;
    String      _aText;

    public:
                     WaitWindow_Impl();
                    ~WaitWindow_Impl();
    virtual void     Paint( const Rectangle& rRect );
};

#define X_OFFSET 15
#define Y_OFFSET 15

//=============================================================================

struct NamePair_Impl
{
    OUString maShortName;
    OUString maLongName;
};

DECLARE_LIST( NameList_Impl, NamePair_Impl* );

class Updater_Impl;
class GroupList_Impl;
class EntryData_Impl;
class GroupData_Impl;

//=============================================================================

class SfxDocTplService_Impl
{
    Reference< XMultiServiceFactory >   mxFactory;
    Reference < XCommandEnvironment >   maCmdEnv;

    Reference< XPersist >       mxInfo;
    ::osl::Mutex                maMutex;
    Sequence< OUString >        maTemplateDirs;
    OUString                    maRootURL;
    NameList_Impl               maNames;
    Locale                      maLocale;
    Content                     maRootContent;
    Updater_Impl*               mpUpdater;
    sal_Bool                    mbIsInitialized : 1;
    sal_Bool                    mbLocaleSet     : 1;


    void                        init_Impl();
    void                        getDefaultLocale();
    void                        getDirList();
    void                        readFolderList();
    OUString                    getLongName( const OUString& rShortName );
    void                        getTitleFromURL( const OUString& rURL, OUString& aTitle, OUString& aType );

    void                        getFolders( Content& rRoot,
                                            Content& rFolder );
    void                        getTemplates( Content& rTargetFolder,
                                              Content& rParentFolder );
    sal_Bool                    addEntry( Content& rParentFolder,
                                          const OUString& rTitle,
                                          const OUString& rTargetURL,
                                          const OUString& rType );
    void                        addToStandard( Content& rRoot,
                                               Content& rFolder );

    sal_Bool                    createFolder( const OUString& rNewFolderURL,
                                              sal_Bool  bCreateParent,
                                              sal_Bool  bFsysFolder,
                                              Content   &rNewFolder );
    sal_Bool                    removeContent( Content& rContent );
    sal_Bool                    removeContent( const OUString& rContentURL );

    sal_Bool                    setProperty( Content& rContent,
                                             const OUString& rPropName,
                                             const Any& rPropValue );
    sal_Bool                    getProperty( Content& rContent,
                                             const OUString& rPropName,
                                             Any& rPropValue );

    void                        createFromContent( GroupList_Impl& rList,
                                                   Content &rContent,
                                                   sal_Bool bHierarchy );
    void                        addHierGroup( GroupList_Impl& rList,
                                              const OUString& rTitle,
                                              const OUString& rOwnURL );
    void                        addFsysGroup( GroupList_Impl& rList,
                                              const OUString& rTitle,
                                              const OUString& rOwnURL );
    void                        removeFromHierarchy( EntryData_Impl *pData );
    void                        addToHierarchy( GroupData_Impl *pGroup,
                                                EntryData_Impl *pData );

    void                        removeFromHierarchy( GroupData_Impl *pGroup );
    void                        addGroupToHierarchy( GroupData_Impl *pGroup );
public:
                                 SfxDocTplService_Impl( Reference< XMultiServiceFactory > xFactory );
                                ~SfxDocTplService_Impl();

    sal_Bool                    init() { if ( !mbIsInitialized ) init_Impl(); return mbIsInitialized; }
    Content                     getContent() { return maRootContent; }

    void                        setLocale( const LOCALE & rLocale );
    Locale                      getLocale();

    sal_Bool                    addTemplate( const OUString& rGroupName,
                                             const OUString& rTemplateName,
                                             const OUString& rSourceURL );
    sal_Bool                    removeTemplate( const OUString& rGroupName,
                                                const OUString& rTemplateName );
    sal_Bool                    renameTemplate( const OUString& rGroupName,
                                                const OUString& rOldName,
                                                const OUString& rNewName );

    sal_Bool                    addGroup( const OUString& rGroupName );
    sal_Bool                    removeGroup( const OUString& rGroupName );
    sal_Bool                    renameGroup( const OUString& rOldName,
                                             const OUString& rNewName );

    void                        update( sal_Bool bUpdateNow );
    void                        doUpdate();
    void                        finished() { mpUpdater = NULL; }
};

//=============================================================================

class Updater_Impl : public ::vos::OThread
{
private:
    SfxDocTplService_Impl   *mpDocTemplates;

public:
                             Updater_Impl( SfxDocTplService_Impl* pTemplates );
                            ~Updater_Impl();

    virtual void SAL_CALL   run();
    virtual void SAL_CALL   onTerminated();
};

//=============================================================================

class EntryData_Impl
{
    OUString            maTitle;
    OUString            maType;
    OUString            maTargetURL;
    OUString            maHierarchyURL;

    sal_Bool            mbInHierarchy   : 1;
    sal_Bool            mbInUse         : 1;
    sal_Bool            mbUpdateType    : 1;
    sal_Bool            mbUpdateLink    : 1;

public:
                        EntryData_Impl( const OUString& rTitle );

    void                setInUse() { mbInUse = sal_True; }
    void                setHierarchy( sal_Bool bInHierarchy ) { mbInHierarchy = bInHierarchy; }
    void                setUpdateLink( sal_Bool bUpdateLink ) { mbUpdateLink = bUpdateLink; }
    void                setUpdateType( sal_Bool bUpdateType ) { mbUpdateType = bUpdateType; }

    sal_Bool            getInUse() const { return mbInUse; }
    sal_Bool            getInHierarchy() const { return mbInHierarchy; }
    sal_Bool            getUpdateLink() const { return mbUpdateLink; }
    sal_Bool            getUpdateType() const { return mbUpdateType; }

    const OUString&     getHierarchyURL() const { return maHierarchyURL; }
    const OUString&     getTargetURL() const { return maTargetURL; }
    const OUString&     getTitle() const { return maTitle; }
    const OUString&     getType() const { return maType; }

    void                setHierarchyURL( const OUString& rURL ) { maHierarchyURL = rURL; }
    void                setTargetURL( const OUString& rURL ) { maTargetURL = rURL; }
    void                setType( const OUString& rType ) { maType = rType; }

    int                 Compare( const OUString& rTitle ) const;
};

DECLARE_LIST( EntryList_Impl, EntryData_Impl* );

//=============================================================================

class GroupData_Impl
{
    EntryList_Impl      maEntries;
    OUString            maTitle;
    OUString            maHierarchyURL;
    OUString            maTargetURL;
    sal_Bool            mbInUse         : 1;
    sal_Bool            mbInHierarchy   : 1;

public:
                        GroupData_Impl( const OUString& rTitle );
                        ~GroupData_Impl();

    void                setInUse() { mbInUse = sal_True; }
    void                setHierarchy( sal_Bool bInHierarchy ) { mbInHierarchy = bInHierarchy; }
    void                setHierarchyURL( const OUString& rURL ) { maHierarchyURL = rURL; }
    void                setTargetURL( const OUString& rURL ) { maTargetURL = rURL; }

    sal_Bool            getInUse() { return mbInUse; }
    sal_Bool            getInHierarchy() { return mbInHierarchy; }
    const OUString&     getHierarchyURL() const { return maHierarchyURL; }
    const OUString&     getTargetURL() const { return maTargetURL; }
    const OUString&     getTitle() const { return maTitle; }

    EntryData_Impl*     addEntry( const OUString& rTitle,
                                  const OUString& rTargetURL,
                                  const OUString& rType,
                                  const OUString& rHierURL );
    ULONG               count() { return maEntries.Count(); }
    EntryData_Impl*     getEntry( ULONG nPos ) { return maEntries.GetObject( nPos ); }





    EntryData_Impl*     GetEntry( ULONG nIndex ) const;
    EntryData_Impl*     GetEntry( const OUString& rName ) const;
    EntryData_Impl*     GetByTargetURL( const OUString& rName ) const;


    ULONG               GetCount() const;

    void                SetTitle( const OUString& rTitle ) { maTitle = rTitle; }


    void                DeleteEntry( ULONG nIndex );

    int                 Compare( const OUString& rTitle ) const
                            { return maTitle.compareTo( rTitle ); }
    int                 Compare( GroupData_Impl* pCompareWith ) const;
};

DECLARE_LIST( GroupList_Impl, GroupData_Impl* );

//=============================================================================
//=============================================================================
//=============================================================================

//-----------------------------------------------------------------------------
// private SfxDocTplService_Impl
//-----------------------------------------------------------------------------
void SfxDocTplService_Impl::init_Impl()
{
    ::osl::MutexGuard aGuard( maMutex );

    if ( !mbLocaleSet )
        getDefaultLocale();

    // convert locale to string
    OUString aLang = maLocale.Language;
    aLang += OUString( '-' );
    aLang += maLocale.Country;

    // set maRootContent to the root of the templates hierarchy. Create the
    // entry if necessary

    sal_Bool    bRootExists = sal_False;

    maRootURL = OUString( RTL_CONSTASCII_USTRINGPARAM( TEMPLATE_ROOT_URL ) );
    maRootURL += OUString( '/' );
    maRootURL += aLang;

    if ( Content::create( maRootURL, maCmdEnv, maRootContent ) )
    {
        mbIsInitialized = sal_True;
        bRootExists = sal_True;
    }
    else
    {
        mbIsInitialized = createFolder( maRootURL, sal_True, sal_False, maRootContent );
    }

    if ( mbIsInitialized )
    {
        OUString aService( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME_DOCINFO ) );
        Reference< XPersist > xInfo( mxFactory->createInstance( aService ), UNO_QUERY );
        mxInfo = xInfo;

        getDirList();
        readFolderList();

        if ( bRootExists )
            update( sal_False );
        else
        {
            WaitWindow_Impl aMsgWin;
            update( sal_True );
        }
    }
    else
    {
        DBG_ERRORFILE( "init_Impl(): Could not create root" );
    }
}
//-----------------------------------------------------------------------------
void SfxDocTplService_Impl::getDefaultLocale()
{
    AllSettings aSettings;

    maLocale    = aSettings.GetLocale();
    mbLocaleSet = sal_True;
}

// -----------------------------------------------------------------------
void SfxDocTplService_Impl::readFolderList()
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    ResStringArray  aShortNames( SfxResId( TEMPLATE_SHORT_NAMES_ARY ) );
    ResStringArray  aLongNames( SfxResId( TEMPLATE_LONG_NAMES_ARY ) );

    NamePair_Impl*  pPair;

    USHORT nCount = Min( aShortNames.Count(), aLongNames.Count() );

    for ( USHORT i=0; i<nCount; i++ )
    {
        pPair = new NamePair_Impl;
        pPair->maShortName  = aShortNames.GetString( i );
        pPair->maLongName   = aLongNames.GetString( i );

        maNames.Insert( pPair, LIST_APPEND );
    }
}

// -----------------------------------------------------------------------
OUString SfxDocTplService_Impl::getLongName( const OUString& rShortName )
{
    OUString         aRet;
    NamePair_Impl   *pPair = maNames.First();

    while ( pPair )
    {
        if ( pPair->maShortName == rShortName )
        {
            aRet = pPair->maLongName;
            break;
        }
        else
            pPair = maNames.Next();
    }

    if ( !aRet.len() )
        aRet = rShortName;

    return aRet;
}

//-----------------------------------------------------------------------------
void SfxDocTplService_Impl::getDirList()
{
    OUString aPropName( RTL_CONSTASCII_USTRINGPARAM( PROPERTY_DIRLIST ) );
    sal_Bool bHasProperty = sal_False;
    Any      aValue;

    // Get the template dir list
    bHasProperty = getProperty( maRootContent, aPropName, aValue );

    if ( bHasProperty )
    {
        aValue >>= maTemplateDirs;
        if ( maTemplateDirs.getLength() )
            return;
    }

    INetURLObject   aURL;
    String          aDirs = SvtPathOptions().GetTemplatePath();
    USHORT          nCount = aDirs.GetTokenCount( C_DELIM );

    maTemplateDirs = Sequence< OUString >( nCount );

    OUString* pDirs = maTemplateDirs.getArray();

    for ( USHORT i=0; i<nCount; i++ )
    {
        aURL.SetSmartProtocol( INET_PROT_FILE );
        aURL.SetURL( aDirs.GetToken( i, C_DELIM ) );
        pDirs[i] = aURL.GetMainURL();
    }

    // Store the template dir list
    setProperty( maRootContent, aPropName, aValue );
}

// -----------------------------------------------------------------------
void SfxDocTplService_Impl::getFolders( Content& rRoot,
                                        Content& rFolder )
{
    Reference< XResultSet > xResultSet;
    Sequence< OUString > aProps(1);
    OUString* pProps = aProps.getArray();
    pProps[0] = OUString::createFromAscii( TITLE );

    addToStandard( rRoot, rFolder );

    try
    {
        ResultSetInclude eInclude = INCLUDE_FOLDERS_ONLY;
        xResultSet = rFolder.createCursor( aProps, eInclude );
    }
    catch( CommandAbortedException& )
    {
        DBG_ERRORFILE( "createCursor: CommandAbortedException" );
    }
    catch ( Exception& ) {}

    if ( xResultSet.is() )
    {
        Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY );
        Reference< XRow > xRow( xResultSet, UNO_QUERY );

        Content aFolder;
        OUString aAdditionalProp( RTL_CONSTASCII_USTRINGPARAM( TARGET_DIR_URL ) );

        try
        {
            while ( xResultSet->next() )
            {
                OUString aTitle( xRow->getString(1) );
                OUString aId = xContentAccess->queryContentIdentifierString();

                if ( aTitle.compareToAscii( "wizard" ) == 0 )
                    continue;
                else if ( aTitle.compareToAscii( "internal" ) == 0 )
                    continue;

                aTitle = getLongName( aTitle );

                INetURLObject aNewFolderObj( maRootURL );
                aNewFolderObj.insertName( aTitle, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );

                OUString aNewFolderURL = aNewFolderObj.GetMainURL();

                if ( ! Content::create( aNewFolderURL, maCmdEnv, aFolder ) &&
                     createFolder( aNewFolderURL, sal_False, sal_False, aFolder ) )
                {
                    setProperty( aFolder, aAdditionalProp, makeAny( aId ) );
                }

                Content aSubFolder( xContentAccess->queryContent(), maCmdEnv );
                getTemplates( aSubFolder, aFolder );
            }
        }
        catch ( Exception& ) {}
    }
}

// -----------------------------------------------------------------------
void SfxDocTplService_Impl::getTemplates( Content& rTargetFolder,
                                          Content& rParentFolder )
{
    Reference< XResultSet > xResultSet;
    Sequence< OUString >    aProps(1);
    OUString* pProps = aProps.getArray();

    pProps[0] = OUString::createFromAscii( TITLE );

    try
    {
        ResultSetInclude eInclude = INCLUDE_DOCUMENTS_ONLY;
        xResultSet = rTargetFolder.createCursor( aProps, eInclude );
    }
    catch( CommandAbortedException& )
    {
        DBG_ERRORFILE( "createCursor: CommandAbortedException" );
    }
    catch ( Exception& ) {}

    if ( xResultSet.is() )
    {
        Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY );
        Reference< XRow > xRow( xResultSet, UNO_QUERY );

        try
        {
            while ( xResultSet->next() )
            {
                OUString aTitle( xRow->getString(1) );

                if ( aTitle.compareToAscii( "sfx.tlx" ) == 0 )
                    continue;

                OUString aId = xContentAccess->queryContentIdentifierString();

                OUString aFullTitle;
                OUString aType;
                getTitleFromURL( aId, aFullTitle, aType );

                if ( aFullTitle.len() )
                    aTitle = aFullTitle;

                addEntry( rParentFolder, aTitle, aId, aType );
            }
        }
        catch( CommandAbortedException& )
        {
            DBG_ERRORFILE( "XContentAccess::next(): CommandAbortedException" );
        }
        catch ( Exception& ) {}
    }
}

// -----------------------------------------------------------------------
void SfxDocTplService_Impl::getTitleFromURL( const OUString& rURL, OUString& aTitle, OUString& aType )
{
    if ( mxInfo.is() )
    {
        try
        {
            mxInfo->read( rURL );

            Reference< XPropertySet > aPropSet( mxInfo, UNO_QUERY );
            if ( aPropSet.is() )
            {
                OUString aPropName( RTL_CONSTASCII_USTRINGPARAM( TITLE ) );
                Any aValue = aPropSet->getPropertyValue( aPropName );
                aValue >>= aTitle;

                aPropName = OUString( RTL_CONSTASCII_USTRINGPARAM( "MIMEType" ) );
                aValue = aPropSet->getPropertyValue( aPropName );
                aValue >>= aType;
            }
        }
        catch ( IOException& ) {}
        catch ( UnknownPropertyException& ) {}
        catch ( Exception& ) {}
    }

    if ( ! aTitle.len() )
    {
        INetURLObject aURL( rURL );
        aURL.CutExtension();
        aTitle = aURL.getName( INetURLObject::LAST_SEGMENT, true,
                               INetURLObject::DECODE_WITH_CHARSET );
    }
}

// -----------------------------------------------------------------------
sal_Bool SfxDocTplService_Impl::addEntry( Content& rParentFolder,
                                          const OUString& rTitle,
                                          const OUString& rTargetURL,
                                          const OUString& rType )
{
    sal_Bool bAddedEntry = sal_False;

    INetURLObject aLinkObj( rParentFolder.getURL() );
    aLinkObj.insertName( rTitle, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
    OUString aLinkURL = aLinkObj.GetMainURL();

    Content aLink;

    if ( ! Content::create( aLinkURL, maCmdEnv, aLink ) )
    {
        Sequence< OUString > aNames(3);
        OUString* pNames = aNames.getArray();
        pNames[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( TITLE ) );
        pNames[1] = OUString( RTL_CONSTASCII_USTRINGPARAM( IS_FOLDER ) );
        pNames[2] = OUString( RTL_CONSTASCII_USTRINGPARAM( TARGET_URL ) );

        Sequence< Any > aValues(3);
        Any* pValues = aValues.getArray();
        pValues[0] = makeAny( rTitle );
        pValues[1] = makeAny( sal_Bool( sal_False ) );
        pValues[2] = makeAny( rTargetURL );

        OUString aType( RTL_CONSTASCII_USTRINGPARAM( TYPE_LINK ) );
        OUString aAdditionalProp( RTL_CONSTASCII_USTRINGPARAM( PROPERTY_TYPE ) );

        try
        {
            rParentFolder.insertNewContent( aType, aNames, aValues, aLink );
            setProperty( aLink, aAdditionalProp, makeAny( rType ) );
            bAddedEntry = sal_True;
        }
        catch( CommandAbortedException& )
        {
            DBG_ERRORFILE( "CommandAbortedException" );
        }
    }
    return bAddedEntry;
}

// -----------------------------------------------------------------------
void SfxDocTplService_Impl::addToStandard( Content& rRoot,
                                           Content& rFolder )
{
    OUString aNewFolderURL;
    OUString aTitle = getLongName( OUString( RTL_CONSTASCII_USTRINGPARAM( STANDARD_FOLDER ) ) );
    OUString aFolderURL = rFolder.get()->getIdentifier()->getContentIdentifier();
    Content  aFolder;

    INetURLObject aNewFolderObj( maRootURL );
    aNewFolderObj.insertName( aTitle, false,
          INetURLObject::LAST_SEGMENT, true,
          INetURLObject::ENCODE_ALL );

    aNewFolderURL = aNewFolderObj.GetMainURL();

    if ( ! Content::create( aNewFolderURL, maCmdEnv, aFolder ) )
    {
        if ( ! createFolder( aNewFolderURL, sal_False, sal_False, aFolder ) )
        {
            DBG_ERRORFILE( "addToStandard(): Could not create Folder!" );
            return;
        }

        OUString aAdditionalProp( RTL_CONSTASCII_USTRINGPARAM( TARGET_DIR_URL ) );
        setProperty( aFolder, aAdditionalProp, makeAny( aFolderURL ) );
    }

    getTemplates( rFolder, aFolder );
}


// -----------------------------------------------------------------------
sal_Bool SfxDocTplService_Impl::createFolder( const OUString& rNewFolderURL,
                                              sal_Bool  bCreateParent,
                                              sal_Bool  bFsysFolder,
                                              Content   &rNewFolder )
{
    Content         aParent;
    sal_Bool        bCreatedFolder = sal_False;
    INetURLObject   aParentURL( rNewFolderURL );
    OUString        aFolderName = aParentURL.getName( INetURLObject::LAST_SEGMENT, true,
                                                      INetURLObject::DECODE_WITH_CHARSET );

    // compute the parent folder url from the new folder url
    // and remove the final slash, because Content::create doesn't
    // like it
    aParentURL.removeSegment();
    if ( aParentURL.getSegmentCount() >= 1 )
        aParentURL.removeFinalSlash();

    // if the parent exists, we can continue with the creation of the
    // new folder, we have to create the parent otherwise ( as long as
    // bCreateParent is set to true )
    if ( Content::create( aParentURL.GetMainURL(), maCmdEnv, aParent ) )
    {
        try
        {
            Sequence< OUString > aNames(2);
            OUString* pNames = aNames.getArray();
            pNames[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( TITLE ) );
            pNames[1] = OUString( RTL_CONSTASCII_USTRINGPARAM( IS_FOLDER ) );

            Sequence< Any > aValues(2);
            Any* pValues = aValues.getArray();
            pValues[0] = makeAny( aFolderName );
            pValues[1] = makeAny( sal_Bool( sal_True ) );

            OUString aType;

            if ( bFsysFolder )
                aType = OUString( RTL_CONSTASCII_USTRINGPARAM( TYPE_FSYS_FOLDER ) );
            else
                aType = OUString( RTL_CONSTASCII_USTRINGPARAM( TYPE_FOLDER ) );

            aParent.insertNewContent( aType, aNames, aValues, rNewFolder );
            bCreatedFolder = sal_True;
        }
        catch( CommandAbortedException& )
        {
            DBG_ERRORFILE( "createFolder(): Could not create new folder" );
        }
        catch( RuntimeException& )
        {
            DBG_ERRORFILE( "createFolder(): got runtime exception" );
        }
    }
    else if ( bCreateParent )
    {
        // if the parent doesn't exists and bCreateParent is set to true,
        // we try to create the parent and if this was successful, we
        // try to create the new folder again ( but this time, we set
        // bCreateParent to false to avoid endless recusions )
        if ( ( aParentURL.getSegmentCount() >= 1 ) &&
               createFolder( aParentURL.GetMainURL(), bCreateParent, bFsysFolder, aParent ) )
        {
            bCreatedFolder = createFolder( rNewFolderURL, sal_False, bFsysFolder, rNewFolder );
        }
    }

    return bCreatedFolder;
}

// -----------------------------------------------------------------------
sal_Bool SfxDocTplService_Impl::removeContent( Content& rContent )
{
    sal_Bool bRemoved = sal_False;
    try
    {
        OUString aCmd( RTL_CONSTASCII_USTRINGPARAM( COMMAND_DELETE ) );
        Any aArg = makeAny( sal_Bool( sal_True ) );

        rContent.executeCommand( aCmd, aArg );
        bRemoved = sal_True;
    }
    catch ( CommandAbortedException& ) {}
    catch ( RuntimeException& ) {}
    catch ( Exception& ) {}

    return bRemoved;
}

// -----------------------------------------------------------------------
sal_Bool SfxDocTplService_Impl::removeContent( const OUString& rContentURL )
{
    Content aContent;

    if ( Content::create( rContentURL, maCmdEnv, aContent ) )
        return removeContent( aContent );
    else
        return sal_False;
}

// -----------------------------------------------------------------------
sal_Bool SfxDocTplService_Impl::setProperty( Content& rContent,
                                             const OUString& rPropName,
                                             const Any& rPropValue )
{
    sal_Bool bPropertySet = sal_False;

    // Store the property
    try
    {
        Reference< XPropertySetInfo > aPropInfo = rContent.getProperties();

        // check, wether or not the property exists, create it, when not
        if ( !aPropInfo.is() || !aPropInfo->hasPropertyByName( rPropName ) )
        {
            Reference< XPropertyContainer > xProperties( rContent.get(), UNO_QUERY );
            if ( xProperties.is() )
            {
                try
                {
                    xProperties->addProperty( rPropName, PropertyAttribute::MAYBEVOID, rPropValue );
                }
                catch( PropertyExistException& ) {}
                catch( IllegalTypeException& ) { DBG_ERRORFILE( "IllegalTypeException" ); }
                catch( IllegalArgumentException& ) { DBG_ERRORFILE( "IllegalArgumentException" ); }
            }
        }

        // now set the property

        rContent.setPropertyValue( rPropName, rPropValue );
        bPropertySet = sal_True;
    }
    catch ( CommandAbortedException& ) {}
    catch ( RuntimeException& ) {}
    catch ( Exception& ) {}

    return bPropertySet;
}

// -----------------------------------------------------------------------
sal_Bool SfxDocTplService_Impl::getProperty( Content& rContent,
                                             const OUString& rPropName,
                                             Any& rPropValue )
{
    sal_Bool bGotProperty = sal_False;

    // Get the property
    try
    {
        Reference< XPropertySetInfo > aPropInfo = rContent.getProperties();

        // check, wether or not the property exists
        if ( !aPropInfo.is() || !aPropInfo->hasPropertyByName( rPropName ) )
        {
            return sal_False;
        }

        // now get the property

        rPropValue = rContent.getPropertyValue( rPropName );
        bGotProperty = sal_True;
    }
    catch ( CommandAbortedException& ) {}
    catch ( RuntimeException& ) {}
    catch ( Exception& ) {}

    return bGotProperty;
}

//-----------------------------------------------------------------------------
// public SfxDocTplService_Impl
//-----------------------------------------------------------------------------

SfxDocTplService_Impl::SfxDocTplService_Impl( Reference< XMultiServiceFactory > xFactory )
{
    mxFactory       = xFactory;
    mpUpdater       = NULL;
    mbIsInitialized = sal_False;
    mbLocaleSet     = sal_False;
}

//-----------------------------------------------------------------------------
SfxDocTplService_Impl::~SfxDocTplService_Impl()
{
    if ( mpUpdater )
    {
        mpUpdater->kill();
        delete mpUpdater;
    }
}

//-----------------------------------------------------------------------------
Locale SfxDocTplService_Impl::getLocale()
{
    ::osl::MutexGuard aGuard( maMutex );

    if ( !mbLocaleSet )
        getDefaultLocale();

    return maLocale;
}

//-----------------------------------------------------------------------------
void SfxDocTplService_Impl::setLocale( const Locale &rLocale )
{
    ::osl::MutexGuard aGuard( maMutex );

    if ( mbLocaleSet &&
         ( maLocale.Language != rLocale.Language ) &&
         ( maLocale.Country != rLocale.Country ) )
        mbIsInitialized = sal_False;

    maLocale    = rLocale;
    mbLocaleSet = sal_True;
}

//-----------------------------------------------------------------------------
void SfxDocTplService_Impl::update( sal_Bool bUpdateNow )
{
    if ( bUpdateNow )
        doUpdate();
    else
    {
        mpUpdater = new Updater_Impl( this );
        mpUpdater->create();
    }
}

//-----------------------------------------------------------------------------
void SfxDocTplService_Impl::doUpdate()
{
    ::osl::MutexGuard aGuard( maMutex );

    GroupList_Impl  aGroupList;

    // get the entries from the hierarchy
    createFromContent( aGroupList, maRootContent, sal_True );

    // get the entries from the template directories
    sal_Int32   nCount = maTemplateDirs.getLength();
    OUString*   pDirs = maTemplateDirs.getArray();
    Content     aDirContent;

    while ( nCount )
    {
        nCount--;
        if ( Content::create( pDirs[ nCount ], maCmdEnv, aDirContent ) )
        {
            createFromContent( aGroupList, aDirContent, sal_False );
//          getFolders( maRootContent, aDirContent );
        }
    }

    // now check the list
    GroupData_Impl *pGroup = aGroupList.First();
    while ( pGroup )
    {
        if ( pGroup->getInUse() )
        {
            if ( pGroup->getInHierarchy() )
            {
                ULONG nCount = pGroup->count();
                for ( ULONG i=0; i<nCount; i++ )
                {
                    EntryData_Impl *pData = pGroup->getEntry( i );
                    if ( ! pData->getInUse() )
                    {
                        if ( pData->getInHierarchy() )
                            removeFromHierarchy( pData ); // delete entry in hierarchy
                        else
                            addToHierarchy( pGroup, pData ); // add entry to hierarchy
                    }
                }
            }
            else
            {
                addGroupToHierarchy( pGroup ); // add group to hierarchy
            }
        }
        else
            removeFromHierarchy( pGroup ); // delete group from hierarchy

        delete pGroup;
        pGroup = aGroupList.Next();
    }
}

//-----------------------------------------------------------------------------
sal_Bool SfxDocTplService_Impl::addGroup( const OUString& rGroupName )
{
    ::osl::MutexGuard aGuard( maMutex );

    // Check, wether or not there is a group with this name
    Content         aNewGroup;
    OUString        aNewGroupURL;
    INetURLObject   aNewGroupObj( maRootURL );

    aNewGroupObj.insertName( rGroupName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );

    aNewGroupURL = aNewGroupObj.GetMainURL();

    if ( Content::create( aNewGroupURL, maCmdEnv, aNewGroup ) ||
         ! createFolder( aNewGroupURL, sal_False, sal_False, aNewGroup ) )
    {
        // if there already was a group with this name or the new group
        // could not be created, we return here
        return sal_False;
    }

    // Get the user template path entry ( new group will always
    // be added in the user template path )
    sal_Int32   nIndex;
    OUString    aUserPath;

    nIndex = maTemplateDirs.getLength();
    if ( nIndex )
        nIndex--;
    else
        return sal_False;   // We don't know where to add the group

    aUserPath = maTemplateDirs[ nIndex ];

    // create a new folder with the given name
    Content         aNewFolder;
    OUString        aNewFolderURL;
    INetURLObject   aNewFolderObj( aUserPath );

    aNewFolderObj.insertName( rGroupName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );

    aNewFolderURL = aNewFolderObj.GetMainURL();

    if ( ! createFolder( aNewFolderURL, sal_False, sal_True, aNewFolder ) )
    {
        // we could not create the folder, so we delete the group in the
        // hierarchy and return
        removeContent( aNewGroup );
        return sal_False;
    }

    // Now set the target url for this group and we are done
    OUString aPropName( RTL_CONSTASCII_USTRINGPARAM( TARGET_DIR_URL ) );
    Any aValue = makeAny( aNewFolderURL );

    if ( ! setProperty( aNewGroup, aPropName, aValue ) )
    {
        removeContent( aNewGroup );
        removeContent( aNewFolder );
        return sal_False;
    }

    return sal_True;
}

//-----------------------------------------------------------------------------
sal_Bool SfxDocTplService_Impl::removeGroup( const OUString& rGroupName )
{
    ::osl::MutexGuard aGuard( maMutex );

    // create the group url
    INetURLObject aGroupObj( maRootURL );
    aGroupObj.insertName( rGroupName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );

    // Get the target url
    Content     aGroup;
    OUString    aGroupURL = aGroupObj.GetMainURL();
    OUString    aTargetURL;

    if ( Content::create( aGroupURL, maCmdEnv, aGroup ) )
    {
        OUString    aPropName( RTL_CONSTASCII_USTRINGPARAM( TARGET_DIR_URL ) );
        Any         aValue;

        if ( getProperty( aGroup, aPropName, aValue ) )
            aValue >>= aTargetURL;
    }
    else
    {
        // could not create a content, so probably the group doesn't exist
        return sal_False;
    }

    // delete the group in the hierarchy
    if ( removeContent( aGroup ) && aTargetURL.getLength() )
    {
        // delete the folder associated with the group
        return removeContent( aTargetURL );
    }
    else
        return sal_False;
}

//-----------------------------------------------------------------------------
sal_Bool SfxDocTplService_Impl::renameGroup( const OUString& rOldName,
                                             const OUString& rNewName )
{
    ::osl::MutexGuard aGuard( maMutex );

    // create the group url
    Content         aGroup;
    INetURLObject   aGroupObj( maRootURL );
                    aGroupObj.insertName( rNewName, false,
                                          INetURLObject::LAST_SEGMENT, true,
                                          INetURLObject::ENCODE_ALL );
    OUString        aGroupURL = aGroupObj.GetMainURL();

    // Check, if there is a group with the new name, return false
    // if there is one.
    if ( Content::create( aGroupURL, maCmdEnv, aGroup ) )
        return sal_False;

    aGroupObj.removeSegment();
    aGroupObj.insertName( rOldName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
    aGroupURL = aGroupObj.GetMainURL();

    // When there is no group with the old name, we can't rename it
    if ( ! Content::create( aGroupURL, maCmdEnv, aGroup ) )
        return sal_False;

    // rename the group in the hierarchy
    OUString    aTitleProp( RTL_CONSTASCII_USTRINGPARAM( TITLE ) );
    Any         aTitleValue;

    aTitleValue <<= rNewName;

    if ( setProperty( aGroup, aTitleProp, aTitleValue ) )
    {
        // Get the target url
        OUString    aTargetProp( RTL_CONSTASCII_USTRINGPARAM( TARGET_DIR_URL ) );
        OUString    aTargetURL;
        Any         aTargetValue;

        if ( getProperty( aGroup, aTargetProp, aTargetValue ) )
            aTargetValue >>= aTargetURL;

        if ( aTargetURL.getLength() )
        {
            // rename the folder associated with the group
            Content aTarget;
            if ( Content::create( aTargetURL, maCmdEnv, aTarget ) )
            {
                if ( setProperty( aTarget, aTitleProp, aTitleValue ) )
                {
                    aTargetURL = aTarget.get()->getIdentifier()->getContentIdentifier();
                    aTargetValue <<= aTargetURL;
                    setProperty( aGroup, aTargetProp, aTargetValue );
                }
            }
        }
        return sal_True;
    }
    else
        return sal_False;
}

//-----------------------------------------------------------------------------
sal_Bool SfxDocTplService_Impl::addTemplate( const OUString& rGroupName,
                                             const OUString& rTemplateName,
                                             const OUString& rSourceURL )
{
    ::osl::MutexGuard aGuard( maMutex );

    // Check, wether or not there is a group with this name
    // Return false, if there is no group with the given name
    Content         aGroup, aTemplate, aTargetGroup;
    OUString        aGroupURL, aTemplateURL;
    INetURLObject   aGroupObj( maRootURL );

    aGroupObj.insertName( rGroupName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
    aGroupURL = aGroupObj.GetMainURL();

    if ( ! Content::create( aGroupURL, maCmdEnv, aGroup ) )
        return sal_False;

    // Check, if there's a template with the given name in this group
    // Return false, if there already is a template
    aGroupObj.insertName( rTemplateName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
    aTemplateURL = aGroupObj.GetMainURL();

    if ( Content::create( aTemplateURL, maCmdEnv, aTemplate ) )
        return sal_False;

    // get the target url of the group
    OUString    aTargetURL;
    OUString    aPropName( RTL_CONSTASCII_USTRINGPARAM( TARGET_DIR_URL ) );
    Any         aValue;

    if ( getProperty( aGroup, aPropName, aValue ) )
        aValue >>= aTargetURL;

    if ( !aTargetURL.getLength() )
        return sal_False;

    if ( ! Content::create( aTargetURL, maCmdEnv, aTargetGroup ) )
        return sal_False;

    // Get the content type
    OUString aTitle, aType, aTargetURL2, aFullName;

    getTitleFromURL( rSourceURL, aTitle, aType );

    // addTemplate will sometimes be called just to add an entry in the
    // hierarchy; the target URL and the source URL will be the same in
    // this scenario

    INetURLObject   aTargetObj( aTargetURL );
    INetURLObject   aSourceObj( rSourceURL );

    aTargetObj.insertName( rTemplateName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
    aTargetObj.setExtension( aSourceObj.getExtension() );

    aTargetURL2 = aTargetObj.GetMainURL();

    if ( aTargetURL2 == rSourceURL )
        return addEntry( aGroup, rTemplateName, rSourceURL, aType );

    // copy the template into the new group (targeturl)

    // we need to keep the extension for the file, so we use the
    // name from the INetURLObject here
    aFullName = aTargetObj.getName( INetURLObject::LAST_SEGMENT, true,
                                    INetURLObject::DECODE_WITH_CHARSET );
    try
    {
        TransferInfo aTransferInfo;
        aTransferInfo.MoveData = sal_False;
        aTransferInfo.SourceURL = rSourceURL;
        aTransferInfo.NewTitle = aFullName;
        aTransferInfo.NameClash = NameClash::RENAME;

        Any aArg = makeAny( aTransferInfo );
        OUString aCmd( RTL_CONSTASCII_USTRINGPARAM( COMMAND_TRANSFER ) );

        aTargetGroup.executeCommand( aCmd, aArg );
    }
    catch ( ContentCreationException& )
    { return FALSE; }
    catch ( CommandAbortedException& )
    { return FALSE; }
    catch ( Exception& )
    { return FALSE; }

    // create a new entry in the hierarchy
    return addEntry( aGroup, rTemplateName, rSourceURL, aType );
}

//-----------------------------------------------------------------------------
sal_Bool SfxDocTplService_Impl::removeTemplate( const OUString& rGroupName,
                                                const OUString& rTemplateName )
{
    ::osl::MutexGuard aGuard( maMutex );

    // Check, wether or not there is a group with this name
    // Return false, if there is no group with the given name
    Content         aGroup, aTemplate;
    OUString        aGroupURL, aTemplateURL;
    INetURLObject   aGroupObj( maRootURL );

    aGroupObj.insertName( rGroupName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
    aGroupURL = aGroupObj.GetMainURL();

    if ( ! Content::create( aGroupURL, maCmdEnv, aGroup ) )
        return sal_False;

    // Check, if there's a template with the given name in this group
    // Return false, if there is no template
    aGroupObj.insertName( rTemplateName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
    aTemplateURL = aGroupObj.GetMainURL();

    if ( !Content::create( aTemplateURL, maCmdEnv, aTemplate ) )
        return sal_False;

    // get the target URL from the template
    OUString    aTargetURL;
    OUString    aPropName( RTL_CONSTASCII_USTRINGPARAM( TARGET_URL ) );
    Any         aValue;

    if ( getProperty( aTemplate, aPropName, aValue ) )
        aValue >>= aTargetURL;

    // delete the target template
    if ( aTargetURL.getLength() )
    {
        removeContent( aTargetURL );
    }

    // delete the template entry
    return removeContent( aTemplate );
}

//-----------------------------------------------------------------------------
sal_Bool SfxDocTplService_Impl::renameTemplate( const OUString& rGroupName,
                                                const OUString& rOldName,
                                                const OUString& rNewName )
{
    ::osl::MutexGuard aGuard( maMutex );

    // Check, wether or not there is a group with this name
    // Return false, if there is no group with the given name
    Content         aGroup, aTemplate;
    OUString        aGroupURL, aTemplateURL;
    INetURLObject   aGroupObj( maRootURL );

    aGroupObj.insertName( rGroupName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
    aGroupURL = aGroupObj.GetMainURL();

    if ( ! Content::create( aGroupURL, maCmdEnv, aGroup ) )
        return sal_False;

    // Check, if there's a template with the new name in this group
    // Return false, if there is one
    aGroupObj.insertName( rNewName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
    aTemplateURL = aGroupObj.GetMainURL();

    if ( Content::create( aTemplateURL, maCmdEnv, aTemplate ) )
        return sal_False;

    // Check, if there's a template with the old name in this group
    // Return false, if there is no template
    aGroupObj.removeSegment();
    aGroupObj.insertName( rOldName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
    aTemplateURL = aGroupObj.GetMainURL();

    if ( !Content::create( aTemplateURL, maCmdEnv, aTemplate ) )
        return sal_False;

    // rename the template entry
    OUString    aTitleProp( RTL_CONSTASCII_USTRINGPARAM( TITLE ) );
    Any         aTitleValue;

    aTitleValue <<= rNewName;

    if ( setProperty( aTemplate, aTitleProp, aTitleValue ) )
    {
        // rename the target template:
        // get the target URL from the template,
        // rename the target and set the new taget url as property
        OUString    aTargetURL;
        OUString    aTargetProp( RTL_CONSTASCII_USTRINGPARAM( TARGET_URL ) );
        Any         aTargetValue;

        if ( getProperty( aTemplate, aTargetProp, aTargetValue ) )
            aTargetValue >>= aTargetURL;

        if ( aTargetURL.getLength() )
        {
            Content aTarget;
            if ( Content::create( aTargetURL, maCmdEnv, aTarget ) )
            {
                // get the original extension and set it again if there is none
                INetURLObject aTargetObj( aTargetURL );
                OUString aExtension = aTargetObj.getExtension();
                aTargetObj.removeSegment();
                aTargetObj.insertName( rNewName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
                if ( ! aTargetObj.hasExtension() )
                {
                    OUString aTitleWithExt;
                    aTargetObj.setExtension( aExtension );
                    aTitleWithExt = aTargetObj.getName( INetURLObject::LAST_SEGMENT, true,
                                                        INetURLObject::DECODE_WITH_CHARSET );
                    aTitleValue <<= aTitleWithExt;
                }

                if ( setProperty( aTarget, aTitleProp, aTitleValue ) )
                {
                    aTargetURL = aTarget.get()->getIdentifier()->getContentIdentifier();
                    aTargetValue <<= aTargetURL;
                    setProperty( aTemplate, aTargetProp, aTargetValue );
                }
            }
        }
        return sal_True;
    }
    else
        return sal_False;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

SFX_IMPL_XSERVICEINFO( SfxDocTplService, TEMPLATE_SERVICE_NAME, TEMPLATE_IMPLEMENTATION_NAME )
SFX_IMPL_SINGLEFACTORY( SfxDocTplService )

//-----------------------------------------------------------------------------
SfxDocTplService::SfxDocTplService( const Reference< XMultiServiceFactory >& xFactory )
{
    pImp = new SfxDocTplService_Impl( xFactory );
}

//-----------------------------------------------------------------------------

SfxDocTplService::~SfxDocTplService()
{
    delete pImp;
}

//-----------------------------------------------------------------------------
//--- XLocalizable ---
//-----------------------------------------------------------------------------

LOCALE SAL_CALL SfxDocTplService::getLocale()
    throw( RUNTIMEEXCEPTION )
{
    return pImp->getLocale();
}

//-----------------------------------------------------------------------------

void SAL_CALL SfxDocTplService::setLocale( const LOCALE & rLocale )
    throw( RUNTIMEEXCEPTION )
{
    pImp->setLocale( rLocale );
}

//-----------------------------------------------------------------------------
//--- XDocumentTemplates ---
//-----------------------------------------------------------------------------
Reference< XCONTENT > SAL_CALL SfxDocTplService::getContent()
    throw( RUNTIMEEXCEPTION )
{
    if ( pImp->init() )
        return pImp->getContent().get();
    else
        return NULL;
}

//-----------------------------------------------------------------------------
sal_Bool SAL_CALL SfxDocTplService::storeTemplate( const OUString& GroupName,
                                                   const OUString& TemplateName,
                                                   const Reference< XSTORABLE >& Storable )
    throw( RUNTIMEEXCEPTION )
{
    return sal_False;
}

//-----------------------------------------------------------------------------
sal_Bool SAL_CALL SfxDocTplService::addTemplate( const OUString& rGroupName,
                                                 const OUString& rTemplateName,
                                                 const OUString& rSourceURL )
    throw( RUNTIMEEXCEPTION )
{
    if ( pImp->init() )
        return pImp->addTemplate( rGroupName, rTemplateName, rSourceURL );
    else
        return sal_False;
}

//-----------------------------------------------------------------------------
sal_Bool SAL_CALL SfxDocTplService::removeTemplate( const OUString& rGroupName,
                                                    const OUString& rTemplateName )
    throw( RUNTIMEEXCEPTION )
{
    if ( pImp->init() )
        return pImp->removeTemplate( rGroupName, rTemplateName );
    else
        return sal_False;
}

//-----------------------------------------------------------------------------
sal_Bool SAL_CALL SfxDocTplService::renameTemplate( const OUString& rGroupName,
                                                    const OUString& rOldName,
                                                    const OUString& rNewName )
    throw( RUNTIMEEXCEPTION )
{
    if ( rOldName == rNewName )
        return sal_True;

    if ( pImp->init() )
        return pImp->renameTemplate( rGroupName, rOldName, rNewName );
    else
        return sal_False;
}

//-----------------------------------------------------------------------------
sal_Bool SAL_CALL SfxDocTplService::addGroup( const OUString& rGroupName )
    throw( RUNTIMEEXCEPTION )
{
    if ( pImp->init() )
        return pImp->addGroup( rGroupName );
    else
        return sal_False;
}

//-----------------------------------------------------------------------------
sal_Bool SAL_CALL SfxDocTplService::removeGroup( const OUString& rGroupName )
    throw( RUNTIMEEXCEPTION )
{
    if ( pImp->init() )
        return pImp->removeGroup( rGroupName );
    else
        return sal_False;
}

//-----------------------------------------------------------------------------
sal_Bool SAL_CALL SfxDocTplService::renameGroup( const OUString& rOldName,
                                                 const OUString& rNewName )
    throw( RUNTIMEEXCEPTION )
{
    if ( rOldName == rNewName )
        return sal_True;

    if ( pImp->init() )
        return pImp->renameGroup( rOldName, rNewName );
    else
        return sal_False;
}

//-----------------------------------------------------------------------------
void SAL_CALL SfxDocTplService::update()
    throw( RUNTIMEEXCEPTION )
{
    if ( pImp->init() )
        pImp->update( sal_True );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//------------------------------------------------------------------------

Updater_Impl::Updater_Impl( SfxDocTplService_Impl* pTemplates )
{
    mpDocTemplates = pTemplates;
}

//------------------------------------------------------------------------
Updater_Impl::~Updater_Impl()
{
}

//------------------------------------------------------------------------
void SAL_CALL Updater_Impl::run()
{
    mpDocTemplates->doUpdate();
}

//------------------------------------------------------------------------
void SAL_CALL Updater_Impl::onTerminated()
{
    mpDocTemplates->finished();
    delete this;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
WaitWindow_Impl::WaitWindow_Impl()
    : WorkWindow( NULL, WB_BORDER | WB_3DLOOK )
{
    Rectangle aRect = Rectangle( 0, 0, 300, 30000 );
    _nTextStyle = TEXT_DRAW_CENTER | TEXT_DRAW_VCENTER | TEXT_DRAW_WORDBREAK | TEXT_DRAW_MULTILINE;
    _aText = String( SfxResId( RID_CNT_STR_WAITING ) );
    _aRect = GetTextRect( aRect, _aText, _nTextStyle );
    aRect = _aRect;
    aRect.Right() += 2*X_OFFSET;
    aRect.Bottom() += 2*Y_OFFSET;
    _aRect.SetPos( Point( X_OFFSET, Y_OFFSET ) );
    SetOutputSizePixel( aRect.GetSize() );
    Show();
    Update();
    Flush();
}

//-----------------------------------------------------------------------------
WaitWindow_Impl::~WaitWindow_Impl()
{
    Hide();
}

//-----------------------------------------------------------------------------
void WaitWindow_Impl::Paint( const Rectangle& rRect )
{
    DrawText( _aRect, _aText, _nTextStyle );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void SfxDocTplService_Impl::addHierGroup( GroupList_Impl& rList,
                                          const OUString& rTitle,
                                          const OUString& rOwnURL )
{
    // now get the content of the Group
    Content                 aContent;
    Reference< XResultSet > xResultSet;
    Sequence< OUString >    aProps(3);

    OUString* pProps = aProps.getArray();
    pProps[0] = OUString::createFromAscii( TITLE );
    pProps[1] = OUString::createFromAscii( TARGET_URL );
    pProps[2] = OUString::createFromAscii( PROPERTY_TYPE );

    try
    {
        aContent = Content( rOwnURL, maCmdEnv );
        ResultSetInclude eInclude = INCLUDE_DOCUMENTS_ONLY;
        xResultSet = aContent.createCursor( aProps, eInclude );
    }
    catch ( ContentCreationException& )
    {
        DBG_ERRORFILE( "addHierGroup: ContentCreationException" );
    }
    catch( CommandAbortedException& )
    {
        DBG_ERRORFILE( "addHierGroup: CommandAbortedException" );
    }
    catch ( Exception& ) {}

    if ( xResultSet.is() )
    {
        GroupData_Impl *pGroup = new GroupData_Impl( rTitle );
        pGroup->setHierarchy( sal_True );
        pGroup->setHierarchyURL( rOwnURL );
        rList.Insert( pGroup );

        Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY );
        Reference< XRow > xRow( xResultSet, UNO_QUERY );

        try
        {
            while ( xResultSet->next() )
            {
                BOOL             bUpdateType = sal_False;
                EntryData_Impl  *pData;

                OUString aTitle( xRow->getString( 1 ) );
                OUString aTargetDir( xRow->getString( 2 ) );
                OUString aType( xRow->getString( 3 ) );
                OUString aHierURL = xContentAccess->queryContentIdentifierString();

                if ( !aType.getLength() )
                {
                    OUString aTmpTitle;
                    getTitleFromURL( aTargetDir, aTmpTitle, aType );
                    if ( aType.getLength() )
                        bUpdateType = sal_True;
                }

                pData = pGroup->addEntry( aTitle, aTargetDir, aType, aHierURL );
                pData->setUpdateType( bUpdateType );
            }
        }
        catch( CommandAbortedException& )
        {
            DBG_ERRORFILE( "XContentAccess::next(): CommandAbortedException" );
        }
        catch ( Exception& ) {}
    }
}

//-----------------------------------------------------------------------------
void SfxDocTplService_Impl::addFsysGroup( GroupList_Impl& rList,
                                          const OUString& rTitle,
                                          const OUString& rOwnURL )
{
    if ( rTitle.compareToAscii( "wizard" ) == 0 )
        return;
    else if ( rTitle.compareToAscii( "internal" ) == 0 )
        return;

    // First, get the long name of the group
    OUString        aTitle = getLongName( rTitle );
    GroupData_Impl *pGroup = rList.First();

    while ( pGroup && pGroup->getTitle() != aTitle )
        pGroup = rList.Next();

    if ( !pGroup )
    {
        pGroup = new GroupData_Impl( aTitle );
        pGroup->setTargetURL( rOwnURL );
        rList.Insert( pGroup );
    }

    pGroup->setInUse();

    // now get the content of the Group
    Content                 aContent;
    Reference< XResultSet > xResultSet;
    Sequence< OUString >    aProps(1);
    OUString* pProps = aProps.getArray();
    pProps[0] = OUString::createFromAscii( TITLE );

    try
    {
        aContent = Content( rOwnURL, maCmdEnv );
        ResultSetInclude eInclude = INCLUDE_DOCUMENTS_ONLY;
        xResultSet = aContent.createCursor( aProps, eInclude );
    }
    catch( CommandAbortedException& )
    {
        DBG_ERRORFILE( "createCursor: CommandAbortedException" );
    }
    catch ( Exception& ) {}

    if ( xResultSet.is() )
    {
        Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY );
        Reference< XRow > xRow( xResultSet, UNO_QUERY );

        try
        {
            while ( xResultSet->next() )
            {
                OUString aTitle( xRow->getString( 1 ) );
                OUString aTargetURL = xContentAccess->queryContentIdentifierString();
                OUString aType;
                OUString aHierURL;

                getTitleFromURL( aTargetURL, aTitle, aType );

                pGroup->addEntry( aTitle, aTargetURL, aType, aHierURL );
            }
        }
        catch( CommandAbortedException& )
        {
            DBG_ERRORFILE( "XContentAccess::next(): CommandAbortedException" );
        }
        catch ( Exception& ) {}
    }
}

// -----------------------------------------------------------------------
void SfxDocTplService_Impl::createFromContent( GroupList_Impl& rList,
                                               Content &rContent,
                                               sal_Bool bHierarchy )
{
    // when scanning the file system, we have to add the 'standard' group, too
    if ( ! bHierarchy )
    {
        OUString aStdTitle = getLongName( OUString( RTL_CONSTASCII_USTRINGPARAM( STANDARD_FOLDER ) ) );
        OUString aTargetURL = rContent.get()->getIdentifier()->getContentIdentifier();
        addFsysGroup( rList, aStdTitle, aTargetURL );
    }

    Reference< XResultSet > xResultSet;
    Sequence< OUString > aProps(1);
    OUString* pProps = aProps.getArray();
    pProps[0] = OUString::createFromAscii( TITLE );

    try
    {
        ResultSetInclude eInclude = INCLUDE_FOLDERS_ONLY;
        xResultSet = rContent.createCursor( aProps, eInclude );
    }
    catch( CommandAbortedException& )
    {
        DBG_ERRORFILE( "createCursor: CommandAbortedException" );
    }
    catch ( Exception& ) {}

    if ( xResultSet.is() )
    {
        Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY );
        Reference< XRow > xRow( xResultSet, UNO_QUERY );

        try
        {
            while ( xResultSet->next() )
            {
                OUString aTitle( xRow->getString( 1 ) );
                OUString aTargetURL( xContentAccess->queryContentIdentifierString() );

                if ( bHierarchy )
                    addHierGroup( rList, aTitle, aTargetURL );
                else
                    addFsysGroup( rList, aTitle, aTargetURL );
            }
        }
        catch( CommandAbortedException& )
        {
            DBG_ERRORFILE( "XContentAccess::next(): CommandAbortedException" );
        }
        catch ( Exception& ) {}
    }
}

//-----------------------------------------------------------------------------
void SfxDocTplService_Impl::removeFromHierarchy( EntryData_Impl *pData )
{
    Content aTemplate;

    if ( Content::create( pData->getHierarchyURL(), maCmdEnv, aTemplate ) )
    {
        removeContent( aTemplate );
    }
}

//-----------------------------------------------------------------------------
void SfxDocTplService_Impl::addToHierarchy( GroupData_Impl *pGroup,
                                            EntryData_Impl *pData )
{
    Content aGroup, aTemplate;

    if ( ! Content::create( pGroup->getHierarchyURL(), maCmdEnv, aGroup ) )
        return;

    // Check, if there's a template with the given name in this group
    // Return if there is already a template
    INetURLObject aGroupObj( pGroup->getHierarchyURL() );

    aGroupObj.insertName( pData->getTitle(), false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );

    OUString aTemplateURL = aGroupObj.GetMainURL();

    if ( Content::create( aTemplateURL, maCmdEnv, aTemplate ) )
        return;

    addEntry( aGroup, pData->getTitle(),
              pData->getTargetURL(),
              pData->getType() );
}

//-----------------------------------------------------------------------------
void SfxDocTplService_Impl::addGroupToHierarchy( GroupData_Impl *pGroup )
{
    OUString aAdditionalProp( RTL_CONSTASCII_USTRINGPARAM( TARGET_DIR_URL ) );
    Content aGroup;

    INetURLObject aNewFolderObj( maRootURL );
    aNewFolderObj.insertName( pGroup->getTitle(), false,
          INetURLObject::LAST_SEGMENT, true,
          INetURLObject::ENCODE_ALL );

    OUString aNewFolderURL = aNewFolderObj.GetMainURL();

    if ( createFolder( aNewFolderURL, sal_False, sal_False, aGroup ) )
    {
        setProperty( aGroup, aAdditionalProp, makeAny( pGroup->getTargetURL() ) );
        Content aTargetDir;
        if ( Content::create( pGroup->getTargetURL(), maCmdEnv, aTargetDir ) )
            getTemplates( aTargetDir, aGroup );
    }
}

//-----------------------------------------------------------------------------
void SfxDocTplService_Impl::removeFromHierarchy( GroupData_Impl *pGroup )
{
    Content aGroup;

    if ( Content::create( pGroup->getHierarchyURL(), maCmdEnv, aGroup ) )
    {
        removeContent( aGroup );
    }
}

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
GroupData_Impl::GroupData_Impl( const OUString& rTitle )
{
    maTitle = rTitle;
    mbInUse = sal_False;
    mbInHierarchy = sal_False;
}

// -----------------------------------------------------------------------
GroupData_Impl::~GroupData_Impl()
{
    EntryData_Impl *pData = maEntries.First();
    while ( pData )
    {
        delete pData;
        pData = maEntries.Next();
    }
}

// -----------------------------------------------------------------------
EntryData_Impl* GroupData_Impl::addEntry( const OUString& rTitle,
                                          const OUString& rTargetURL,
                                          const OUString& rType,
                                          const OUString& rHierURL )
{
    EntryData_Impl *pData = maEntries.First();

    while ( pData && pData->getTitle() != rTitle )
        pData = maEntries.Next();

    if ( !pData )
    {
        pData = new EntryData_Impl( rTitle );
        pData->setTargetURL( rTargetURL );
        pData->setType( rType );
        if ( rHierURL.getLength() )
        {
            pData->setHierarchyURL( rHierURL );
            pData->setHierarchy( sal_True );
        }
        maEntries.Insert( pData );
    }
    else
    {
        pData->setInUse();
    }

    return pData;
}

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
EntryData_Impl::EntryData_Impl( const OUString& rTitle )
{
    maTitle = rTitle;
    mbInUse = sal_False;
    mbInHierarchy = sal_False;
}
