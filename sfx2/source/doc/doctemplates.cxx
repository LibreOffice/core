/*************************************************************************
 *
 *  $RCSfile: doctemplates.cxx,v $
 *
 *  $Revision: 1.27 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-05 10:35:39 $
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
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _SV_RESARY_HXX
#include <tools/resary.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif

#ifndef _COMPHELPER_SEQUENCEASHASHMAP_HXX_
#include <comphelper/sequenceashashmap.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
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

#ifndef  _COM_SUN_STAR_CONTAINER_XCONTAINERQUERY_HPP_
#include <com/sun/star/container/XContainerQuery.hpp>
#endif

#ifndef  _COM_SUN_STAR_DOCUMENT_XTYPEDETECTION_HPP_
#include <com/sun/star/document/XTypeDetection.hpp>
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

#ifndef  _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include <com/sun/star/uno/Exception.hpp>
#endif

#ifndef  _COM_SUN_STAR_UTIL_XOFFICEINSTALLATIONDIRECTORIES_HPP_
#include <com/sun/star/util/XOfficeInstallationDirectories.hpp>
#endif

#include "unotools/configmgr.hxx"

#include "sfxresid.hxx"
#include "doc.hrc"

//-----------------------------------------------------------------------------

//=============================================================================

#define TEMPLATE_SERVICE_NAME               "com.sun.star.frame.DocumentTemplates"
#define TEMPLATE_IMPLEMENTATION_NAME        "com.sun.star.comp.sfx2.DocumentTemplates"

#define SERVICENAME_TYPEDETECTION           "com.sun.star.document.TypeDetection"
#define SERVICENAME_DOCINFO                 "com.sun.star.document.DocumentProperties"

#define TEMPLATE_ROOT_URL       "vnd.sun.star.hier:/templates"
#define TITLE                   "Title"
#define IS_FOLDER               "IsFolder"
#define TARGET_URL              "TargetURL"
#define TYPE_FOLDER             "application/vnd.sun.star.hier-folder"
#define TYPE_LINK               "application/vnd.sun.star.hier-link"
#define TYPE_FSYS_FOLDER        "application/vnd.sun.staroffice.fsys-folder"

#define PROPERTY_DIRLIST        "DirectoryList"
#define PROPERTY_NEEDSUPDATE    "NeedsUpdate"
#define PROPERTY_TYPE           "TypeDescription"

#define TARGET_DIR_URL          "TargetDirURL"
#define COMMAND_DELETE          "delete"
#define COMMAND_TRANSFER        "transfer"

#define STANDARD_FOLDER         "standard"

#define C_DELIM                 ';'

//=============================================================================

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;

using namespace rtl;
using namespace ucb;
using namespace comphelper;

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
class DocTemplates_EntryData_Impl;
class GroupData_Impl;

//=============================================================================

class SfxDocTplService_Impl
{
    Reference< XMultiServiceFactory >           mxFactory;
    Reference< XCommandEnvironment >            maCmdEnv;
    Reference< XPersist >                       mxInfo;
    Reference< XTypeDetection >                 mxType;
    Reference< XOfficeInstallationDirectories > mxOfficeInstDirs;

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
    sal_Bool                    needsUpdate();
    OUString                    getLongName( const OUString& rShortName );
    sal_Bool                    getTitleFromURL( const OUString& rURL, OUString& aTitle, OUString& aType );

    sal_Bool                    addEntry( Content& rParentFolder,
                                          const OUString& rTitle,
                                          const OUString& rTargetURL,
                                          const OUString& rType );

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
    void                        removeFromHierarchy( DocTemplates_EntryData_Impl *pData );
    void                        addToHierarchy( GroupData_Impl *pGroup,
                                                DocTemplates_EntryData_Impl *pData );

    void                        removeFromHierarchy( GroupData_Impl *pGroup );
    void                        addGroupToHierarchy( GroupData_Impl *pGroup );

    void                        updateData( DocTemplates_EntryData_Impl *pData );

    static bool                 propertyCanContainOfficeDir( const rtl::OUString & rPropName );
    void                        initOfficeInstDirs();
    void                        makeRelocatableURL( rtl::OUString & rURL );
    void                        makeAbsoluteURL( rtl::OUString & rURL );

public:
                                 SfxDocTplService_Impl( Reference< XMultiServiceFactory > xFactory );
                                ~SfxDocTplService_Impl();

    sal_Bool                    init() { if ( !mbIsInitialized ) init_Impl(); return mbIsInitialized; }
    Content                     getContent() { return maRootContent; }

    void                        setLocale( const Locale & rLocale );
    Locale                      getLocale();

    sal_Bool                    storeTemplate( const OUString& rGroupName,
                                               const OUString& rTemplateName,
                                               const Reference< XSTORABLE >& rStorable );

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

class DocTemplates_EntryData_Impl
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
                        DocTemplates_EntryData_Impl( const OUString& rTitle );

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
};

DECLARE_LIST( EntryList_Impl, DocTemplates_EntryData_Impl* );

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

    DocTemplates_EntryData_Impl*     addEntry( const OUString& rTitle,
                                  const OUString& rTargetURL,
                                  const OUString& rType,
                                  const OUString& rHierURL );
    ULONG               count() { return maEntries.Count(); }
    DocTemplates_EntryData_Impl*     getEntry( ULONG nPos ) { return maEntries.GetObject( nPos ); }
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
    ::osl::ClearableMutexGuard aGuard( maMutex );
    sal_Bool bIsInitialized = sal_False;
    sal_Bool bNeedsUpdate   = sal_False;

    if ( !mbLocaleSet )
        getDefaultLocale();

    // convert locale to string
    OUString aLang = maLocale.Language;
    aLang += OUString( '-' );
    aLang += maLocale.Country;

    // set maRootContent to the root of the templates hierarchy. Create the
    // entry if necessary

    maRootURL = OUString( RTL_CONSTASCII_USTRINGPARAM( TEMPLATE_ROOT_URL ) );
    maRootURL += OUString( '/' );
    maRootURL += aLang;

    if ( Content::create( maRootURL, maCmdEnv, maRootContent ) )
        bIsInitialized = sal_True;
    else
    {
        bIsInitialized = createFolder( maRootURL, sal_True, sal_False, maRootContent );
        bNeedsUpdate = sal_True;
    }

    if ( bIsInitialized )
    {
        OUString aService( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME_DOCINFO ) );
        mxInfo = Reference< XPersist > ( mxFactory->createInstance( aService ), UNO_QUERY );

        aService = OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME_TYPEDETECTION ) );
        mxType = Reference< XTypeDetection > ( mxFactory->createInstance( aService ), UNO_QUERY );

        getDirList();
        readFolderList();

        if ( bNeedsUpdate || needsUpdate() )
        {
            aGuard.clear();
            ::vos::OClearableGuard aSolarGuard( Application::GetSolarMutex() );

            WaitWindow_Impl* pWin = new WaitWindow_Impl();

            aSolarGuard.clear();
            ::osl::ClearableMutexGuard anotherGuard( maMutex );

            update( sal_True );

            anotherGuard.clear();
            ::vos::OGuard aSecondSolarGuard( Application::GetSolarMutex() );

            delete pWin;
        }
    }
    else
    {
        DBG_ERRORFILE( "init_Impl(): Could not create root" );
    }

    mbIsInitialized = bIsInitialized;
}

//-----------------------------------------------------------------------------
void SfxDocTplService_Impl::getDefaultLocale()
{
    if ( !mbLocaleSet )
    {
        ::osl::MutexGuard aGuard( maMutex );
        if ( !mbLocaleSet )
        {
            rtl::OUString aLocale;
            utl::ConfigManager::GetDirectConfigProperty( utl::ConfigManager::LOCALE )
                >>= aLocale;

            if ( aLocale.getLength() > 0 )
            {
                sal_Int32 nPos = aLocale.indexOf( sal_Unicode( '-' ) );
                if ( nPos != -1 )
                {
                    maLocale.Language = aLocale.copy( 0, nPos );
                    nPos = aLocale.indexOf( sal_Unicode( '_' ), nPos + 1 );
                    if ( nPos != -1 )
                    {
                        maLocale.Country
                            = aLocale.copy( maLocale.Language.getLength() + 1,
                                            nPos - maLocale.Language.getLength() - 1 );
                        maLocale.Variant
                            = aLocale.copy( nPos + 1 );
                    }
                    else
                    {
                        maLocale.Country
                            = aLocale.copy( maLocale.Language.getLength() + 1 );
                    }
                }

            }

            mbLocaleSet = sal_True;
        }
    }
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

    if ( !aRet.getLength() )
        aRet = rShortName;

    return aRet;
}

//-----------------------------------------------------------------------------
void SfxDocTplService_Impl::getDirList()
{
    OUString aPropName( RTL_CONSTASCII_USTRINGPARAM( PROPERTY_DIRLIST ) );
    Any      aValue;

    // Get the template dir list
    INetURLObject   aURL;
    String          aDirs = SvtPathOptions().GetTemplatePath();
    USHORT          nCount = aDirs.GetTokenCount( C_DELIM );

    maTemplateDirs = Sequence< OUString >( nCount );

    for ( USHORT i=0; i<nCount; i++ )
    {
        aURL.SetSmartProtocol( INET_PROT_FILE );
        aURL.SetURL( aDirs.GetToken( i, C_DELIM ) );
        maTemplateDirs[i] = aURL.GetMainURL( INetURLObject::NO_DECODE );
    }

    aValue <<= maTemplateDirs;

    // Store the template dir list
    setProperty( maRootContent, aPropName, aValue );
}

//-----------------------------------------------------------------------------
sal_Bool SfxDocTplService_Impl::needsUpdate()
{
    OUString aPropName( RTL_CONSTASCII_USTRINGPARAM( PROPERTY_NEEDSUPDATE ) );
    sal_Bool bHasProperty = sal_False;
    sal_Bool bNeedsUpdate = sal_True;
    Any      aValue;

    // Get the template dir list
    bHasProperty = getProperty( maRootContent, aPropName, aValue );

    if ( bHasProperty )
        aValue >>= bNeedsUpdate;

    return bNeedsUpdate;
}

// -----------------------------------------------------------------------
sal_Bool SfxDocTplService_Impl::getTitleFromURL( const OUString& rURL, OUString& aTitle, OUString& aType )
{
    if ( mxInfo.is() )
    {
        try
        {
            mxInfo->read( rURL );
        }
        catch ( Exception& )
        {
            // the document is not a StarOffice document
            return sal_False;
        }

        try
        {
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
        catch ( UnknownPropertyException& ) {}
        catch ( Exception& ) {}
    }

    if ( ! aType.getLength() && mxType.is() )
    {
        aType = mxType->queryTypeByURL( rURL );
    }

    if ( ! aTitle.getLength() )
    {
        INetURLObject aURL( rURL );
        aURL.CutExtension();
        aTitle = aURL.getName( INetURLObject::LAST_SEGMENT, true,
                               INetURLObject::DECODE_WITH_CHARSET );
    }

    return sal_True;
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
    OUString aLinkURL = aLinkObj.GetMainURL( INetURLObject::NO_DECODE );

    Content aLink;

    if ( ! Content::create( aLinkURL, maCmdEnv, aLink ) )
    {
        Sequence< OUString > aNames(3);
        aNames[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( TITLE ) );
        aNames[1] = OUString( RTL_CONSTASCII_USTRINGPARAM( IS_FOLDER ) );
        aNames[2] = OUString( RTL_CONSTASCII_USTRINGPARAM( TARGET_URL ) );

        Sequence< Any > aValues(3);
        aValues[0] = makeAny( rTitle );
        aValues[1] = makeAny( sal_Bool( sal_False ) );
        aValues[2] = makeAny( rTargetURL );

        OUString aType( RTL_CONSTASCII_USTRINGPARAM( TYPE_LINK ) );
        OUString aAdditionalProp( RTL_CONSTASCII_USTRINGPARAM( PROPERTY_TYPE ) );

        try
        {
            rParentFolder.insertNewContent( aType, aNames, aValues, aLink );
            setProperty( aLink, aAdditionalProp, makeAny( rType ) );
            bAddedEntry = sal_True;
        }
        catch( Exception& )
        {}
    }
    return bAddedEntry;
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
    if ( Content::create( aParentURL.GetMainURL( INetURLObject::NO_DECODE ), maCmdEnv, aParent ) )
    {
        try
        {
            Sequence< OUString > aNames(2);
            aNames[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( TITLE ) );
            aNames[1] = OUString( RTL_CONSTASCII_USTRINGPARAM( IS_FOLDER ) );

            Sequence< Any > aValues(2);
            aValues[0] = makeAny( aFolderName );
            aValues[1] = makeAny( sal_Bool( sal_True ) );

            OUString aType;

            if ( bFsysFolder )
                aType = OUString( RTL_CONSTASCII_USTRINGPARAM( TYPE_FSYS_FOLDER ) );
            else
                aType = OUString( RTL_CONSTASCII_USTRINGPARAM( TYPE_FOLDER ) );

            aParent.insertNewContent( aType, aNames, aValues, rNewFolder );
            bCreatedFolder = sal_True;
        }
        catch( RuntimeException& )
        {
            DBG_ERRORFILE( "createFolder(): got runtime exception" );
        }
        catch( Exception& )
        {
            DBG_ERRORFILE( "createFolder(): Could not create new folder" );
        }
    }
    else if ( bCreateParent )
    {
        // if the parent doesn't exists and bCreateParent is set to true,
        // we try to create the parent and if this was successful, we
        // try to create the new folder again ( but this time, we set
        // bCreateParent to false to avoid endless recusions )
        if ( ( aParentURL.getSegmentCount() >= 1 ) &&
               createFolder( aParentURL.GetMainURL( INetURLObject::NO_DECODE ), bCreateParent, bFsysFolder, aParent ) )
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
        Any aPropValue( rPropValue );
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

        // To ensure a reloctable office installation, the path to the
        // office installtion directory must never be stored directly.
        if ( propertyCanContainOfficeDir( rPropName ) )
        {
            OUString aValue;
            if ( rPropValue >>= aValue )
            {
                makeRelocatableURL( aValue );
                aPropValue = makeAny( aValue );
            }
            else
            {
                Sequence< OUString > aValues;
                if ( rPropValue >>= aValues )
                {
                    for ( sal_Int32 n = 0; n < aValues.getLength(); n++ )
                    {
                        makeRelocatableURL( aValues[ n ] );
                    }
                    aPropValue = makeAny( aValues );
                }
                else
                {
                    OSL_ENSURE( false, "Unsupported property value type" );
                }
            }
        }

        // now set the property

        rContent.setPropertyValue( rPropName, aPropValue );
        bPropertySet = sal_True;
    }
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

        // To ensure a reloctable office installation, the path to the
        // office installtion directory must never be stored directly.
        if ( propertyCanContainOfficeDir( rPropName ) )
        {
            OUString aValue;
            if ( rPropValue >>= aValue )
            {
                makeAbsoluteURL( aValue );
                rPropValue = makeAny( aValue );
            }
            else
            {
                Sequence< OUString > aValues;
                if ( rPropValue >>= aValues )
                {
                    for ( sal_Int32 n = 0; n < aValues.getLength(); n++ )
                    {
                        makeAbsoluteURL( aValues[ n ] );
                    }
                    rPropValue = makeAny( aValues );
                }
                else
                {
                    OSL_ENSURE( false, "Unsupported property value type" );
                }
            }
        }

        bGotProperty = sal_True;
    }
    catch ( RuntimeException& ) {}
    catch ( Exception& ) {}

    return bGotProperty;
}

// -----------------------------------------------------------------------
// static
bool SfxDocTplService_Impl::propertyCanContainOfficeDir(
                                        const rtl::OUString & rPropName )
{
    // Note: TargetURL is handled by UCB itself (because it is a property
    //       with a predefined semantic). Additional Core properties introduced
    //       be a client app must be handled by the client app itself, because
    //       the UCB does not know the semantics of those properties.
    return ( rPropName.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( TARGET_DIR_URL ) ) ||
             rPropName.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( PROPERTY_DIRLIST ) ) );
}

// -----------------------------------------------------------------------
void SfxDocTplService_Impl::initOfficeInstDirs()
{
    if ( !mxOfficeInstDirs.is() )
    {
        osl::MutexGuard aGuard( maMutex );
        if ( !mxOfficeInstDirs.is() )
        {
            OSL_ENSURE( mxFactory.is(), "No service manager!" );

            Reference< XComponentContext > xCtx;
            Reference< XPropertySet > xPropSet( mxFactory, UNO_QUERY );
            if ( xPropSet.is() )
            {
                xPropSet->getPropertyValue(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ) ) )
                >>= xCtx;
            }

            OSL_ENSURE( xCtx.is(),
                        "Unable to obtain component context from "
                        "service manager!" );

            if ( xCtx.is() )
            {
                xCtx->getValueByName(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                        "/singletons/"
                        "com.sun.star.util.theOfficeInstallationDirectories" ) ) )
                >>= mxOfficeInstDirs;
            }

            OSL_ENSURE( mxOfficeInstDirs.is(),
                        "Unable to obtain office installation directory "
                        "singleton!" );
        }
    }
}

// -----------------------------------------------------------------------
void SfxDocTplService_Impl::makeRelocatableURL( rtl::OUString & rURL )
{
    if ( rURL.getLength() > 0 )
    {
        initOfficeInstDirs();
        rURL = mxOfficeInstDirs->makeRelocatableURL( rURL );
    }
}

// -----------------------------------------------------------------------
void SfxDocTplService_Impl::makeAbsoluteURL( rtl::OUString & rURL )
{
    if ( rURL.getLength() > 0 )
    {
        initOfficeInstDirs();
        rURL = mxOfficeInstDirs->makeAbsoluteURL( rURL );
    }
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
    ::osl::MutexGuard aGuard( maMutex );

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
    ::osl::MutexGuard aGuard( maMutex );

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

    OUString aPropName( RTL_CONSTASCII_USTRINGPARAM( PROPERTY_NEEDSUPDATE ) );
    Any      aValue;

    aValue <<= sal_True;
    setProperty( maRootContent, aPropName, aValue );

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
                    DocTemplates_EntryData_Impl *pData = pGroup->getEntry( i );
                    if ( ! pData->getInUse() )
                    {
                        if ( pData->getInHierarchy() )
                            removeFromHierarchy( pData ); // delete entry in hierarchy
                        else
                            addToHierarchy( pGroup, pData ); // add entry to hierarchy
                    }
                    else if ( pData->getUpdateType() ||
                              pData->getUpdateLink() )
                    {
                        updateData( pData );
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

       aValue <<= sal_False;
    setProperty( maRootContent, aPropName, aValue );
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

    aNewGroupURL = aNewGroupObj.GetMainURL( INetURLObject::NO_DECODE );

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

    aNewFolderURL = aNewFolderObj.GetMainURL( INetURLObject::NO_DECODE );

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
    OUString    aGroupURL = aGroupObj.GetMainURL( INetURLObject::NO_DECODE );
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
    OUString        aGroupURL = aGroupObj.GetMainURL( INetURLObject::NO_DECODE );

    // Check, if there is a group with the new name, return false
    // if there is one.
    if ( Content::create( aGroupURL, maCmdEnv, aGroup ) )
        return sal_False;

    aGroupObj.removeSegment();
    aGroupObj.insertName( rOldName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
    aGroupURL = aGroupObj.GetMainURL( INetURLObject::NO_DECODE );

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
sal_Bool SfxDocTplService_Impl::storeTemplate( const OUString& rGroupName,
                                               const OUString& rTemplateName,
                                               const Reference< XSTORABLE >& rStorable )
{
    int ind = 0;
    ::osl::MutexGuard aGuard( maMutex );

    // Check, wether or not there is a group with this name
    // Return false, if there is no group with the given name
    Content         aGroup, aTemplate, aTargetGroup;
    OUString        aGroupURL, aTemplateURL;
    INetURLObject   aGroupObj( maRootURL );

    aGroupObj.insertName( rGroupName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
    aGroupURL = aGroupObj.GetMainURL( INetURLObject::NO_DECODE );

    if ( ! Content::create( aGroupURL, maCmdEnv, aGroup ) )
        return sal_False;

    // Check, if there's a template with the given name in this group
    // Return false, if there already is a template
    aGroupObj.insertName( rTemplateName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
    aTemplateURL = aGroupObj.GetMainURL( INetURLObject::NO_DECODE );

    if ( Content::create( aTemplateURL, maCmdEnv, aTemplate ) )
        return sal_False;

    // The object provided with XStorable interface must
    // support one of the listed below services

    // Find service name of the document
    // and construct a query for it
    Reference< XSERVICEINFO > rServiceInfo( rStorable, UNO_QUERY );
    if( !rServiceInfo.is() )
        return sal_False;

    OUStringBuffer sQueryBuffer;
    sQueryBuffer.appendAscii("matchByDocumentService=");
    if( rServiceInfo->supportsService( OUString::createFromAscii( "com.sun.star.text.TextDocument" ) ) )
        sQueryBuffer.appendAscii("com.sun.star.text.TextDocument");
    else if( rServiceInfo->supportsService( OUString::createFromAscii( "com.sun.star.formula.FormulaProperties" ) ) )
        sQueryBuffer.appendAscii("com.sun.star.formula.FormulaProperties");
    else if( rServiceInfo->supportsService( OUString::createFromAscii( "com.sun.star.presentation.PresentationDocument" ) ) )
        sQueryBuffer.appendAscii("com.sun.star.presentation.PresentationDocument");
    else if( rServiceInfo->supportsService( OUString::createFromAscii( "com.sun.star.drawing.DrawingDocument" ) ) )
        sQueryBuffer.appendAscii("com.sun.star.drawing.DrawingDocument");
    else if( rServiceInfo->supportsService( OUString::createFromAscii( "com.sun.star.sheet.SpreadsheetDocument" ) ) )
        sQueryBuffer.appendAscii("com.sun.star.sheet.SpreadsheetDocument");
    else if( rServiceInfo->supportsService( OUString::createFromAscii( "com.sun.star.text.WebDocument" ) ) )
        sQueryBuffer.appendAscii("com.sun.star.text.WebDocument");
    else if( rServiceInfo->supportsService( OUString::createFromAscii( "com.sun.star.text.GlobalDocument" ) ) )
        sQueryBuffer.appendAscii("com.sun.star.text.GlobalDocument");
    else
        return sal_False;

    sQueryBuffer.appendAscii( ":iflags=54:eflags=64" ); // template export filter
    OUString aQueryForFilter = sQueryBuffer.makeStringAndClear();

    // Find a template filter for the document type
    Reference< XMULTISERVICEFACTORY > rServiceManager = ::comphelper::getProcessServiceFactory();
    if( !rServiceManager.is() )
        return sal_False;

    Reference< XContainerQuery > rFilterCFG(
        rServiceManager->createInstance( OUString::createFromAscii( "com.sun.star.document.FilterFactory" ) ),
        UNO_QUERY );

    if( !rFilterCFG.is())
        return sal_False;

    Reference< XEnumeration > rResult = rFilterCFG->createSubSetEnumerationByQuery(aQueryForFilter);
    if( !rResult.is() || !rResult->hasMoreElements() )
        return sal_False;

    // use first template only!
    SequenceAsHashMap aFilterProps(rResult->nextElement());

    // find a type (and the filter name)
    OUString aFilterName = aFilterProps.getUnpackedValueOrDefault( OUString::createFromAscii( "Name" ), OUString() );
    OUString aTypeName   = aFilterProps.getUnpackedValueOrDefault( OUString::createFromAscii( "Type" ), OUString() );
    if( !aTypeName.getLength() || !aFilterName.getLength() )
        return sal_False;

    // Find an extension and a mime-type for the type
    Reference< XNAMEACCESS > rTypeDetection(
        rServiceManager->createInstance( OUString::createFromAscii( "com.sun.star.document.TypeDetection" ) ),
        UNO_QUERY );

    if( !rTypeDetection.is() )
        return sal_False;

    SequenceAsHashMap aTypeProps( rTypeDetection->getByName( aTypeName ) );
    Sequence< OUString > aAllExt = aTypeProps.getUnpackedValueOrDefault( OUString::createFromAscii( "Extensions" ), Sequence< OUString >() );
    if (!aAllExt.getLength())
        return sal_False;

    OUString aMimeType = aTypeProps.getUnpackedValueOrDefault( OUString::createFromAscii( "MediaType"  ), OUString() );
    OUString aExt      = aAllExt[0];

    // construct destination url
    OUString    aTargetURL;
    OUString    aPropName( RTL_CONSTASCII_USTRINGPARAM( TARGET_DIR_URL ) );
    Any         aValue;
    if ( getProperty( aGroup, aPropName, aValue ) )
        aValue >>= aTargetURL;
    if ( !aTargetURL.getLength() )
        return sal_False;

    INetURLObject   aTargetObj( aTargetURL );
    aTargetObj.insertName( rTemplateName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
    aTargetObj.setExtension( aExt );
    OUString aTargetURL2 = aTargetObj.GetMainURL( INetURLObject::NO_DECODE );

    // store template
    Sequence< PropertyValue > aArgs(1);
    aArgs[0].Name = OUString::createFromAscii( "FilterName" );
    aArgs[0].Value <<= aFilterName;

    try {
        rStorable->storeToURL( aTargetURL2, aArgs );
    }
    catch( Exception& )
    {
        // the template was not stored
        return sal_False;
    }

    // add the template to hierarchy
    return addEntry( aGroup, rTemplateName, aTargetURL2, aMimeType );
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
    aGroupURL = aGroupObj.GetMainURL( INetURLObject::NO_DECODE );

    if ( ! Content::create( aGroupURL, maCmdEnv, aGroup ) )
        return sal_False;

    // Check, if there's a template with the given name in this group
    // Return false, if there already is a template
    aGroupObj.insertName( rTemplateName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
    aTemplateURL = aGroupObj.GetMainURL( INetURLObject::NO_DECODE );

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

    // only StarOffice documents are acceptable
    if( !getTitleFromURL( rSourceURL, aTitle, aType ) )
        return sal_False;

    // addTemplate will sometimes be called just to add an entry in the
    // hierarchy; the target URL and the source URL will be the same in
    // this scenario

    INetURLObject   aTargetObj( aTargetURL );
    INetURLObject   aSourceObj( rSourceURL );

    aTargetObj.insertName( rTemplateName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
    aTargetObj.setExtension( aSourceObj.getExtension() );

    aTargetURL2 = aTargetObj.GetMainURL( INetURLObject::NO_DECODE );

    if ( aTargetURL2 == rSourceURL )
        return addEntry( aGroup, rTemplateName, aTargetURL2, aType );

    // copy the template into the new group (targeturl)

    // we need to keep the extension for the file, so we use the
    // name from the INetURLObject here
    aFullName = aTargetObj.getName( INetURLObject::LAST_SEGMENT, true,
                                    INetURLObject::DECODE_WITH_CHARSET );

    // get access to source file
    Content aSourceContent;
    Reference < ::com::sun::star::ucb::XCommandEnvironment > xEnv;
    INetURLObject   aSourceURL( rSourceURL );
    if( ! Content::create( aSourceURL.GetMainURL( INetURLObject::NO_DECODE ), xEnv, aSourceContent ) )
        return sal_False;

    // transfer source file
    try
    {
        if( ! aTargetGroup.transferContent( aSourceContent,
                                                InsertOperation_COPY,
                                                aFullName,
                                                NameClash::ERROR ) )
            return sal_False;
    }
    catch ( ContentCreationException& )
    { return FALSE; }
    catch ( Exception& )
    { return FALSE; }

    // create a new entry in the hierarchy
    return addEntry( aGroup, rTemplateName, aTargetURL2, aType );
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
    aGroupURL = aGroupObj.GetMainURL( INetURLObject::NO_DECODE );

    if ( ! Content::create( aGroupURL, maCmdEnv, aGroup ) )
        return sal_False;

    // Check, if there's a template with the given name in this group
    // Return false, if there is no template
    aGroupObj.insertName( rTemplateName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
    aTemplateURL = aGroupObj.GetMainURL( INetURLObject::NO_DECODE );

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
    aGroupURL = aGroupObj.GetMainURL( INetURLObject::NO_DECODE );

    if ( ! Content::create( aGroupURL, maCmdEnv, aGroup ) )
        return sal_False;

    // Check, if there's a template with the new name in this group
    // Return false, if there is one
    aGroupObj.insertName( rNewName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
    aTemplateURL = aGroupObj.GetMainURL( INetURLObject::NO_DECODE );

    if ( Content::create( aTemplateURL, maCmdEnv, aTemplate ) )
        return sal_False;

    // Check, if there's a template with the old name in this group
    // Return false, if there is no template
    aGroupObj.removeSegment();
    aGroupObj.insertName( rOldName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
    aTemplateURL = aGroupObj.GetMainURL( INetURLObject::NO_DECODE );

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

Locale SAL_CALL SfxDocTplService::getLocale()
    throw( RUNTIMEEXCEPTION )
{
    return pImp->getLocale();
}

//-----------------------------------------------------------------------------

void SAL_CALL SfxDocTplService::setLocale( const Locale & rLocale )
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
    if ( pImp->init() )
        return pImp->storeTemplate( GroupName, TemplateName, Storable );
    else
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

    aProps[0] = OUString::createFromAscii( TITLE );
    aProps[1] = OUString::createFromAscii( TARGET_URL );
    aProps[2] = OUString::createFromAscii( PROPERTY_TYPE );

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
                DocTemplates_EntryData_Impl  *pData;

                OUString aTitle( xRow->getString( 1 ) );
                OUString aTargetDir( xRow->getString( 2 ) );
                OUString aType( xRow->getString( 3 ) );
                OUString aHierURL = xContentAccess->queryContentIdentifierString();

                if ( !aType.getLength() )
                {
                    OUString aTmpTitle;

                    if( !getTitleFromURL( aTargetDir, aTmpTitle, aType ) )
                    {
                        DBG_ERRORFILE( "addHierGroup(): template of alien format" );
                        continue;
                    }

                    if ( aType.getLength() )
                        bUpdateType = sal_True;
                }

                pData = pGroup->addEntry( aTitle, aTargetDir, aType, aHierURL );
                pData->setUpdateType( bUpdateType );
            }
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
    aProps[0] = OUString::createFromAscii( TITLE );

    try
    {
        aContent = Content( rOwnURL, maCmdEnv );
        ResultSetInclude eInclude = INCLUDE_DOCUMENTS_ONLY;
        xResultSet = aContent.createCursor( aProps, eInclude );
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

                if ( aTitle.compareToAscii( "sfx.tlx" ) == 0 )
                    continue;

                // only StarOffice templates are accepted
                if( !getTitleFromURL( aTargetURL, aTitle, aType ) )
                    continue;

                pGroup->addEntry( aTitle, aTargetURL, aType, aHierURL );
            }
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
    aProps[0] = OUString::createFromAscii( TITLE );

    try
    {
        ResultSetInclude eInclude = INCLUDE_FOLDERS_ONLY;
        xResultSet = rContent.createCursor( aProps, eInclude );
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
        catch ( Exception& ) {}
    }
}

//-----------------------------------------------------------------------------
void SfxDocTplService_Impl::removeFromHierarchy( DocTemplates_EntryData_Impl *pData )
{
    Content aTemplate;

    if ( Content::create( pData->getHierarchyURL(), maCmdEnv, aTemplate ) )
    {
        removeContent( aTemplate );
    }
}

//-----------------------------------------------------------------------------
void SfxDocTplService_Impl::addToHierarchy( GroupData_Impl *pGroup,
                                            DocTemplates_EntryData_Impl *pData )
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

    OUString aTemplateURL = aGroupObj.GetMainURL( INetURLObject::NO_DECODE );

    if ( Content::create( aTemplateURL, maCmdEnv, aTemplate ) )
        return;

    addEntry( aGroup, pData->getTitle(),
              pData->getTargetURL(),
              pData->getType() );
}

//-----------------------------------------------------------------------------
void SfxDocTplService_Impl::updateData( DocTemplates_EntryData_Impl *pData )
{
    Content aTemplate;

    if ( ! Content::create( pData->getHierarchyURL(), maCmdEnv, aTemplate ) )
        return;

    OUString aPropName;

    if ( pData->getUpdateType() )
    {
        aPropName = OUString( RTL_CONSTASCII_USTRINGPARAM( PROPERTY_TYPE ) );
        setProperty( aTemplate, aPropName, makeAny( pData->getType() ) );
    }

    if ( pData->getUpdateLink() )
    {
        aPropName = OUString( RTL_CONSTASCII_USTRINGPARAM( TARGET_URL ) );
        setProperty( aTemplate, aPropName, makeAny( pData->getTargetURL() ) );
    }
}

//-----------------------------------------------------------------------------
void SfxDocTplService_Impl::addGroupToHierarchy( GroupData_Impl *pGroup )
{
    OUString aAdditionalProp( RTL_CONSTASCII_USTRINGPARAM( TARGET_DIR_URL ) );
    Content aGroup;

    INetURLObject aNewGroupObj( maRootURL );
    aNewGroupObj.insertName( pGroup->getTitle(), false,
          INetURLObject::LAST_SEGMENT, true,
          INetURLObject::ENCODE_ALL );

    OUString aNewGroupURL = aNewGroupObj.GetMainURL( INetURLObject::NO_DECODE );

    if ( createFolder( aNewGroupURL, sal_False, sal_False, aGroup ) )
    {
        setProperty( aGroup, aAdditionalProp, makeAny( pGroup->getTargetURL() ) );
        pGroup->setHierarchyURL( aNewGroupURL );

        ULONG nCount = pGroup->count();
        for ( ULONG i=0; i<nCount; i++ )
        {
            DocTemplates_EntryData_Impl *pData = pGroup->getEntry( i );
            addToHierarchy( pGroup, pData ); // add entry to hierarchy
        }
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
    DocTemplates_EntryData_Impl *pData = maEntries.First();
    while ( pData )
    {
        delete pData;
        pData = maEntries.Next();
    }
}

// -----------------------------------------------------------------------
DocTemplates_EntryData_Impl* GroupData_Impl::addEntry( const OUString& rTitle,
                                          const OUString& rTargetURL,
                                          const OUString& rType,
                                          const OUString& rHierURL )
{
    DocTemplates_EntryData_Impl *pData = maEntries.First();

    while ( pData && pData->getTitle() != rTitle )
        pData = maEntries.Next();

    if ( !pData )
    {
        pData = new DocTemplates_EntryData_Impl( rTitle );
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
        if ( rTargetURL != pData->getTargetURL() )
        {
            pData->setTargetURL( rTargetURL );
            pData->setUpdateLink( sal_True );
        }
    }

    return pData;
}

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
DocTemplates_EntryData_Impl::DocTemplates_EntryData_Impl( const OUString& rTitle )
{
    maTitle         = rTitle;
    mbInUse         = sal_False;
    mbInHierarchy   = sal_False;
    mbUpdateType    = sal_False;
    mbUpdateLink    = sal_False;
}
