/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "doctemplates.hxx"
#include <osl/mutex.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <tools/urlobj.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/resary.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <unotools/pathoptions.hxx>
#include <comphelper/componentcontext.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/string.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>
#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/document/XTypeDetection.hpp>
#include <com/sun/star/document/XStandaloneDocumentInfo.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/ucb/NameClashException.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/uno/Exception.hpp>

#include <svtools/templatefoldercache.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/ucbhelper.hxx>

#include "sfx2/sfxresid.hxx"
#include "sfxurlrelocator.hxx"
#include "doctemplateslocal.hxx"
#include <sfx2/docfac.hxx>
#include <sfx2/docfile.hxx>
#include "doc.hrc"

#include <vector>

//-----------------------------------------------------------------------------

//=============================================================================

#define TEMPLATE_SERVICE_NAME               "com.sun.star.frame.DocumentTemplates"
#define TEMPLATE_IMPLEMENTATION_NAME        "com.sun.star.comp.sfx2.DocumentTemplates"

#define SERVICENAME_TYPEDETECTION           "com.sun.star.document.TypeDetection"
#define SERVICENAME_DOCINFO                 "com.sun.star.document.StandaloneDocumentInfo"

#define TEMPLATE_ROOT_URL       "vnd.sun.star.hier:/templates"
#define TITLE                   "Title"
#define IS_FOLDER               "IsFolder"
#define IS_DOCUMENT             "IsDocument"
#define TARGET_URL              "TargetURL"
#define TEMPLATE_VERSION        "TemplateComponentVersion"
#define TEMPLATE_VERSION_VALUE  "2"
#define TYPE_FOLDER             "application/vnd.sun.star.hier-folder"
#define TYPE_LINK               "application/vnd.sun.star.hier-link"
#define TYPE_FSYS_FOLDER        "application/vnd.sun.staroffice.fsys-folder"
#define TYPE_FSYS_FILE          "application/vnd.sun.staroffice.fsys-file"

#define PROPERTY_DIRLIST        "DirectoryList"
#define PROPERTY_NEEDSUPDATE    "NeedsUpdate"
#define PROPERTY_TYPE           "TypeDescription"

#define TARGET_DIR_URL          "TargetDirURL"
#define COMMAND_DELETE          "delete"
#define COMMAND_TRANSFER        "transfer"

#define STANDARD_FOLDER         "standard"

#define C_DELIM                 ';'

//=============================================================================

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;

using namespace ::rtl;
using namespace ::ucbhelper;
using namespace ::comphelper;

using ::std::vector;

//=============================================================================

class WaitWindow_Impl : public WorkWindow
{
    Rectangle   _aRect;
    sal_uInt16      _nTextStyle;
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

class Updater_Impl;
class DocTemplates_EntryData_Impl;
class GroupData_Impl;

typedef vector< NamePair_Impl* > NameList_Impl;
typedef vector< GroupData_Impl* > GroupList_Impl;

//=============================================================================
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>

class TplTaskEnvironment : public ::cppu::WeakImplHelper1< ucb::XCommandEnvironment >
{
    uno::Reference< task::XInteractionHandler >               m_xInteractionHandler;
    uno::Reference< ucb::XProgressHandler >                   m_xProgressHandler;

public:
    TplTaskEnvironment( const uno::Reference< task::XInteractionHandler>& rxInteractionHandler )
                                : m_xInteractionHandler( rxInteractionHandler )
                            {}

    virtual uno::Reference<task::XInteractionHandler> SAL_CALL getInteractionHandler() throw (uno::RuntimeException)
    { return m_xInteractionHandler; }

    virtual uno::Reference<ucb::XProgressHandler> SAL_CALL    getProgressHandler() throw (uno::RuntimeException)
    { return m_xProgressHandler; }
};

class SfxDocTplService_Impl
{
    uno::Reference< XMultiServiceFactory >           mxFactory;
    uno::Reference< XCommandEnvironment >            maCmdEnv;
    uno::Reference< XStandaloneDocumentInfo >        mxInfo;
    uno::Reference< XTypeDetection >                 mxType;

    ::osl::Mutex                maMutex;
    Sequence< OUString >        maTemplateDirs;
    OUString                    maRootURL;
    NameList_Impl               maNames;
    Locale                      maLocale;
    Content                     maRootContent;
    Updater_Impl*               mpUpdater;
    sal_Bool                    mbIsInitialized : 1;
    sal_Bool                    mbLocaleSet     : 1;

    SfxURLRelocator_Impl        maRelocator;

    void                        init_Impl();
    void                        getDefaultLocale();
    void                        getDirList();
    void                        readFolderList();
    sal_Bool                    needsUpdate();
    OUString                    getLongName( const OUString& rShortName );
    sal_Bool                    setTitleForURL( const OUString& rURL, const OUString& aTitle );
    sal_Bool                    getTitleFromURL( const OUString& rURL, OUString& aTitle, OUString& aType, sal_Bool& bDocHasTitle );

    sal_Bool                    addEntry( Content& rParentFolder,
                                          const OUString& rTitle,
                                          const OUString& rTargetURL,
                                          const OUString& rType );

    sal_Bool                    createFolder( const OUString& rNewFolderURL,
                                              sal_Bool  bCreateParent,
                                              sal_Bool  bFsysFolder,
                                              Content   &rNewFolder );

    sal_Bool                    CreateNewUniqueFolderWithPrefix( const ::rtl::OUString& aPath,
                                                                const ::rtl::OUString& aPrefix,
                                                                ::rtl::OUString& aNewFolderName,
                                                                ::rtl::OUString& aNewFolderURL,
                                                                Content& aNewFolder );
    ::rtl::OUString             CreateNewUniqueFileWithPrefix( const ::rtl::OUString& aPath,
                                                                const ::rtl::OUString& aPrefix,
                                                                const ::rtl::OUString& aExt );

    uno::Sequence< beans::StringPair > ReadUINamesForTemplateDir_Impl( const ::rtl::OUString& aUserPath );
    sal_Bool                    UpdateUINamesForTemplateDir_Impl( const ::rtl::OUString& aUserPath,
                                                                  const ::rtl::OUString& aGroupName,
                                                                  const ::rtl::OUString& aNewFolderName );
    sal_Bool                    ReplaceUINamesForTemplateDir_Impl( const ::rtl::OUString& aUserPath,
                                                                  const ::rtl::OUString& aFsysGroupName,
                                                                  const ::rtl::OUString& aOldGroupName,
                                                                  const ::rtl::OUString& aNewGroupName );
    sal_Bool                    RemoveUINamesForTemplateDir_Impl( const ::rtl::OUString& aUserPath,
                                                                  const ::rtl::OUString& aGroupName );
    sal_Bool                    WriteUINamesForTemplateDir_Impl( const ::rtl::OUString& aUserPath,
                                                                const uno::Sequence< beans::StringPair >& aUINames );

    ::rtl::OUString             CreateNewGroupFsys( const ::rtl::OUString& rGroupName, Content& aGroup );

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
                                                   sal_Bool bHierarchy,
                                                   sal_Bool bWriteableContent = sal_False );
    void                        addHierGroup( GroupList_Impl& rList,
                                              const OUString& rTitle,
                                              const OUString& rOwnURL );
    void                        addFsysGroup( GroupList_Impl& rList,
                                              const OUString& rTitle,
                                              const OUString& rUITitle,
                                              const OUString& rOwnURL,
                                              sal_Bool bWriteableGroup = sal_False );
    void                        removeFromHierarchy( DocTemplates_EntryData_Impl *pData );
    void                        addToHierarchy( GroupData_Impl *pGroup,
                                                DocTemplates_EntryData_Impl *pData );

    void                        removeFromHierarchy( GroupData_Impl *pGroup );
    void                        addGroupToHierarchy( GroupData_Impl *pGroup );

    void                        updateData( DocTemplates_EntryData_Impl *pData );

public:
                                 SfxDocTplService_Impl( uno::Reference< XMultiServiceFactory > xFactory );
                                ~SfxDocTplService_Impl();

    sal_Bool                    init() { if ( !mbIsInitialized ) init_Impl(); return mbIsInitialized; }
    Content                     getContent() const { return maRootContent; }

    void                        setLocale( const Locale & rLocale );
    Locale                      getLocale();

    sal_Bool                    storeTemplate( const OUString& rGroupName,
                                               const OUString& rTemplateName,
                                               const uno::Reference< XSTORABLE >& rStorable );

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

class Updater_Impl : public ::osl::Thread
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

//=============================================================================

class GroupData_Impl
{
    vector< DocTemplates_EntryData_Impl* > maEntries;
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

    sal_Bool            getInUse() const { return mbInUse; }
    sal_Bool            getInHierarchy() const { return mbInHierarchy; }
    const OUString&     getHierarchyURL() const { return maHierarchyURL; }
    const OUString&     getTargetURL() const { return maTargetURL; }
    const OUString&     getTitle() const { return maTitle; }

    DocTemplates_EntryData_Impl*     addEntry( const OUString& rTitle,
                                  const OUString& rTargetURL,
                                  const OUString& rType,
                                  const OUString& rHierURL );
    size_t                          count() { return maEntries.size(); }
    DocTemplates_EntryData_Impl*    getEntry( size_t nPos ) { return maEntries[ nPos ]; }
};

//-----------------------------------------------------------------------------
// private SfxDocTplService_Impl
//-----------------------------------------------------------------------------
void SfxDocTplService_Impl::init_Impl()
{
    uno::Reference< lang::XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
    if ( xFactory.is() )
    {
        uno::Reference < task::XInteractionHandler > xInteractionHandler( xFactory->createInstance( DEFINE_CONST_UNICODE("com.sun.star.task.InteractionHandler") ), uno::UNO_QUERY );
        maCmdEnv = new TplTaskEnvironment( xInteractionHandler );
    }

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

    maRootURL = OUString( TEMPLATE_ROOT_URL  );
    maRootURL += OUString( '/' );
    maRootURL += aLang;

    ::rtl::OUString aTemplVersPropName( TEMPLATE_VERSION  );
    ::rtl::OUString aTemplVers( TEMPLATE_VERSION_VALUE  );
    if ( Content::create( maRootURL, maCmdEnv, comphelper::getProcessComponentContext(), maRootContent ) )
    {
        uno::Any aValue;
        ::rtl::OUString aPropValue;
        if ( getProperty( maRootContent, aTemplVersPropName, aValue )
          && ( aValue >>= aPropValue )
          && aPropValue.equals( aTemplVers ) )
        {
            bIsInitialized = sal_True;
        }
        else
            removeContent( maRootContent );
    }

    if ( !bIsInitialized )
    {
        if ( createFolder( maRootURL, sal_True, sal_False, maRootContent )
          && setProperty( maRootContent, aTemplVersPropName, uno::makeAny( aTemplVers ) ) )
            bIsInitialized = sal_True;

        bNeedsUpdate = sal_True;
    }

    if ( bIsInitialized )
    {
        OUString aService( SERVICENAME_DOCINFO  );
        try {
            mxInfo = uno::Reference< XStandaloneDocumentInfo > (
                mxFactory->createInstance( aService ), UNO_QUERY );
        } catch (uno::RuntimeException &) {
            OSL_FAIL("SfxDocTplService_Impl::init_Impl: "
                "cannot create DocumentProperties service");
        }

        aService = OUString( SERVICENAME_TYPEDETECTION  );
        mxType = uno::Reference< XTypeDetection > ( mxFactory->createInstance( aService ), UNO_QUERY );

        getDirList();
        readFolderList();

        if ( bNeedsUpdate )
        {
            aGuard.clear();
            SolarMutexClearableGuard aSolarGuard;

            WaitWindow_Impl* pWin = new WaitWindow_Impl();

            aSolarGuard.clear();
            ::osl::ClearableMutexGuard anotherGuard( maMutex );

            update( sal_True );

            anotherGuard.clear();
            SolarMutexGuard aSecondSolarGuard;

            delete pWin;
        }
        else if ( needsUpdate() )
            // the UI should be shown only on the first update
            update( sal_True );
    }
    else
    {
        SAL_WARN( "sfx2.doc", "init_Impl(): Could not create root" );
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
            rtl::OUString aLocale( utl::ConfigManager::getLocale() );
            if ( !aLocale.isEmpty() )
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
    SolarMutexGuard aGuard;

    ResStringArray  aShortNames( SfxResId( TEMPLATE_SHORT_NAMES_ARY ) );
    ResStringArray  aLongNames( SfxResId( TEMPLATE_LONG_NAMES_ARY ) );

    NamePair_Impl*  pPair;

    sal_uInt16 nCount = (sal_uInt16)( Min( aShortNames.Count(), aLongNames.Count() ) );

    for ( sal_uInt16 i=0; i<nCount; i++ )
    {
        pPair = new NamePair_Impl;
        pPair->maShortName  = aShortNames.GetString( i );
        pPair->maLongName   = aLongNames.GetString( i );

        maNames.push_back( pPair );
    }
}

// -----------------------------------------------------------------------
OUString SfxDocTplService_Impl::getLongName( const OUString& rShortName )
{
    OUString         aRet;

    for ( size_t i = 0, n = maNames.size(); i < n; ++i )
    {
        NamePair_Impl* pPair = maNames[ i ];
        if ( pPair->maShortName == rShortName )
        {
            aRet = pPair->maLongName;
            break;
        }
    }

    if ( aRet.isEmpty() )
        aRet = rShortName;

    return aRet;
}

//-----------------------------------------------------------------------------
void SfxDocTplService_Impl::getDirList()
{
    OUString aPropName( PROPERTY_DIRLIST  );
    Any      aValue;

    // Get the template dir list
    // TODO/LATER: let use service, register listener
    INetURLObject   aURL;
    String          aDirs = SvtPathOptions().GetTemplatePath();
    sal_uInt16  nCount = comphelper::string::getTokenCount(aDirs, C_DELIM);

    maTemplateDirs = Sequence< OUString >( nCount );

    uno::Reference< XComponentContext > xCtx(
        comphelper::getComponentContext( mxFactory ) );
    uno::Reference< util::XMacroExpander > xExpander;
    const rtl::OUString aPrefix(
        "vnd.sun.star.expand:"  );

    xCtx->getValueByName(
        rtl::OUString( "/singletons/com.sun.star.util.theMacroExpander"  ) )
        >>= xExpander;

    OSL_ENSURE( xExpander.is(),
                "Unable to obtain macro expander singleton!" );

    for ( sal_uInt16 i=0; i<nCount; i++ )
    {
        aURL.SetSmartProtocol( INET_PROT_FILE );
        aURL.SetURL( aDirs.GetToken( i, C_DELIM ) );
        maTemplateDirs[i] = aURL.GetMainURL( INetURLObject::NO_DECODE );

        sal_Int32 nIndex = maTemplateDirs[i].indexOf( aPrefix );
        if ( nIndex != -1 && xExpander.is() )
        {
            maTemplateDirs[i] = maTemplateDirs[i].replaceAt(nIndex,
                                                            aPrefix.getLength(),
                                                            rtl::OUString());
            maTemplateDirs[i] = xExpander->expandMacros( maTemplateDirs[i] );
        }
    }

    aValue <<= maTemplateDirs;

    // Store the template dir list
    setProperty( maRootContent, aPropName, aValue );
}

//-----------------------------------------------------------------------------
sal_Bool SfxDocTplService_Impl::needsUpdate()
{
    OUString aPropName( PROPERTY_NEEDSUPDATE  );
    sal_Bool bHasProperty = sal_False;
    sal_Bool bNeedsUpdate = sal_True;
    Any      aValue;

    // Get the template dir list
    bHasProperty = getProperty( maRootContent, aPropName, aValue );

    if ( bHasProperty )
        aValue >>= bNeedsUpdate;

    // the old template component also checks this state, but it is initialized from this component
    // so if this componend was already updated the old component does not need such an update
    ::svt::TemplateFolderCache aTempCache;
    if ( !bNeedsUpdate )
        bNeedsUpdate = aTempCache.needsUpdate();

    if ( bNeedsUpdate )
        aTempCache.storeState();

    return bNeedsUpdate;
}

// -----------------------------------------------------------------------
sal_Bool SfxDocTplService_Impl::setTitleForURL( const OUString& rURL, const OUString& aTitle )
{
    sal_Bool bResult = sal_False;
    if ( mxInfo.is() )
    {
        try
        {
            mxInfo->loadFromURL( rURL );
            uno::Reference< XPropertySet > xPropSet( mxInfo, UNO_QUERY_THROW );
            OUString aPropName( TITLE  );
            xPropSet->setPropertyValue( aPropName, uno::makeAny( aTitle ) );
            mxInfo->storeIntoURL( rURL );
            bResult = sal_True;
        }
        catch ( Exception& )
        {
        }
    }

    return bResult;
}

// -----------------------------------------------------------------------
sal_Bool SfxDocTplService_Impl::getTitleFromURL( const OUString& rURL, OUString& aTitle, OUString& aType, sal_Bool& bDocHasTitle )
{
    bDocHasTitle = sal_False;

    if ( mxInfo.is() )
    {
        try
        {
            mxInfo->loadFromURL( rURL );
        }
        catch ( Exception& )
        {
        }

        try
        {
            uno::Reference< XPropertySet > aPropSet( mxInfo, UNO_QUERY );
            if ( aPropSet.is() )
            {
                OUString aPropName( TITLE  );
                Any aValue = aPropSet->getPropertyValue( aPropName );
                aValue >>= aTitle;

                aPropName = OUString( "MIMEType"  );
                aValue = aPropSet->getPropertyValue( aPropName );
                aValue >>= aType;
            }
        }
        catch ( UnknownPropertyException& ) {}
        catch ( Exception& ) {}
    }

    if ( aType.isEmpty() && mxType.is() )
    {
        ::rtl::OUString aDocType = mxType->queryTypeByURL( rURL );
        if ( !aDocType.isEmpty() )
            try
            {
                uno::Reference< container::XNameAccess > xTypeDetection( mxType, uno::UNO_QUERY_THROW );
                SequenceAsHashMap aTypeProps( xTypeDetection->getByName( aDocType ) );
                aType = aTypeProps.getUnpackedValueOrDefault(
                            ::rtl::OUString("MediaType"),
                            ::rtl::OUString() );
            }
            catch( uno::Exception& )
            {}
    }

    if ( aTitle.isEmpty() )
    {
        INetURLObject aURL( rURL );
        aURL.CutExtension();
        aTitle = aURL.getName( INetURLObject::LAST_SEGMENT, true,
                               INetURLObject::DECODE_WITH_CHARSET );
    }
    else
        bDocHasTitle = sal_True;

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

    if ( ! Content::create( aLinkURL, maCmdEnv, comphelper::getProcessComponentContext(), aLink ) )
    {
        Sequence< OUString > aNames(3);
        aNames[0] = OUString( TITLE  );
        aNames[1] = OUString( IS_FOLDER  );
        aNames[2] = OUString( TARGET_URL  );

        Sequence< Any > aValues(3);
        aValues[0] = makeAny( rTitle );
        aValues[1] = makeAny( sal_Bool( sal_False ) );
        aValues[2] = makeAny( rTargetURL );

        OUString aType( TYPE_LINK  );
        OUString aAdditionalProp( PROPERTY_TYPE  );

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
    if ( Content::create( aParentURL.GetMainURL( INetURLObject::NO_DECODE ), maCmdEnv, comphelper::getProcessComponentContext(), aParent ) )
    {
        try
        {
            Sequence< OUString > aNames(2);
            aNames[0] = OUString( TITLE  );
            aNames[1] = OUString( IS_FOLDER  );

            Sequence< Any > aValues(2);
            aValues[0] = makeAny( aFolderName );
            aValues[1] = makeAny( sal_Bool( sal_True ) );

            OUString aType;

            if ( bFsysFolder )
                aType = OUString( TYPE_FSYS_FOLDER  );
            else
                aType = OUString( TYPE_FOLDER  );

            aParent.insertNewContent( aType, aNames, aValues, rNewFolder );
            bCreatedFolder = sal_True;
        }
        catch( RuntimeException& )
        {
            SAL_WARN( "sfx2.doc", "createFolder(): got runtime exception" );
        }
        catch( Exception& )
        {
            SAL_WARN( "sfx2.doc", "createFolder(): Could not create new folder" );
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
sal_Bool SfxDocTplService_Impl::CreateNewUniqueFolderWithPrefix( const ::rtl::OUString& aPath,
                                                                const ::rtl::OUString& aPrefix,
                                                                ::rtl::OUString& aNewFolderName,
                                                                ::rtl::OUString& aNewFolderURL,
                                                                Content& aNewFolder )
{
    sal_Bool bCreated = sal_False;
    INetURLObject aDirPath( aPath );

    Content aParent;
    uno::Reference< XCommandEnvironment > aQuietEnv;
       if ( Content::create( aDirPath.GetMainURL( INetURLObject::NO_DECODE ), aQuietEnv, comphelper::getProcessComponentContext(), aParent ) )
       {
        for ( sal_Int32 nInd = 0; nInd < 32000; nInd++ )
        {
            ::rtl::OUString aTryName = aPrefix;
            if ( nInd )
                aTryName += ::rtl::OUString::valueOf( nInd );

            try
            {
                Sequence< OUString > aNames(2);
                aNames[0] = OUString( TITLE  );
                aNames[1] = OUString( IS_FOLDER  );

                Sequence< Any > aValues(2);
                aValues[0] = makeAny( aTryName );
                aValues[1] = makeAny( sal_Bool( sal_True ) );

                OUString aType( TYPE_FSYS_FOLDER  );

                bCreated = aParent.insertNewContent( aType, aNames, aValues, aNewFolder );
            }
            catch( ucb::NameClashException& )
            {
                // if there is already an element, retry
            }
            catch( Exception& )
            {
                INetURLObject aObjPath( aDirPath );
                aObjPath.insertName( aTryName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
                // if there is already an element, retry
                // if there was another error, do not try any more
                if ( !::utl::UCBContentHelper::Exists( aObjPath.GetMainURL( INetURLObject::NO_DECODE ) ) )
                    break;
            }

            if ( bCreated )
            {
                aNewFolderName = aTryName;
                aNewFolderURL = aNewFolder.get()->getIdentifier()->getContentIdentifier();
                break;
            }
        }
    }

    return bCreated;
}

// -----------------------------------------------------------------------
::rtl::OUString SfxDocTplService_Impl::CreateNewUniqueFileWithPrefix( const ::rtl::OUString& aPath,
                                                                        const ::rtl::OUString& aPrefix,
                                                                        const ::rtl::OUString& aExt )
{
    ::rtl::OUString aNewFileURL;
    INetURLObject aDirPath( aPath );

       Content aParent;

    uno::Reference< XCommandEnvironment > aQuietEnv;
    if ( Content::create( aDirPath.GetMainURL( INetURLObject::NO_DECODE ), aQuietEnv, comphelper::getProcessComponentContext(), aParent ) )
       {
        for ( sal_Int32 nInd = 0; nInd < 32000; nInd++ )
        {
            Content aNewFile;
            sal_Bool bCreated = sal_False;
            ::rtl::OUString aTryName = aPrefix;
            if ( nInd )
                aTryName += ::rtl::OUString::valueOf( nInd );
            if ( aExt.toChar() != '.' )
                aTryName += ::rtl::OUString( "."  );
            aTryName += aExt;

            try
            {
                Sequence< OUString > aNames(2);
                aNames[0] = OUString( TITLE  );
                aNames[1] = OUString( IS_DOCUMENT  );

                Sequence< Any > aValues(2);
                aValues[0] = makeAny( aTryName );
                aValues[1] = makeAny( sal_Bool( sal_True ) );

                OUString aType( TYPE_FSYS_FILE  );

                bCreated = aParent.insertNewContent( aType, aNames, aValues, aNewFile );
            }
            catch( ucb::NameClashException& )
            {
                // if there is already an element, retry
            }
            catch( Exception& )
            {
                INetURLObject aObjPath( aPath );
                aObjPath.insertName( aTryName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
                // if there is already an element, retry
                // if there was another error, do not try any more
                if ( !::utl::UCBContentHelper::Exists( aObjPath.GetMainURL( INetURLObject::NO_DECODE ) ) )
                    break;
            }

            if ( bCreated )
            {
                aNewFileURL = aNewFile.get()->getIdentifier()->getContentIdentifier();
                break;
            }
        }
    }

    return aNewFileURL;
}

// -----------------------------------------------------------------------
sal_Bool SfxDocTplService_Impl::removeContent( Content& rContent )
{
    sal_Bool bRemoved = sal_False;
    try
    {
        OUString aCmd( COMMAND_DELETE  );
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

    if ( Content::create( rContentURL, maCmdEnv, comphelper::getProcessComponentContext(), aContent ) )
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
        uno::Reference< XPropertySetInfo > aPropInfo = rContent.getProperties();

        // check, whether or not the property exists, create it, when not
        if ( !aPropInfo.is() || !aPropInfo->hasPropertyByName( rPropName ) )
        {
            uno::Reference< XPropertyContainer > xProperties( rContent.get(), UNO_QUERY );
            if ( xProperties.is() )
            {
                try
                {
                    xProperties->addProperty( rPropName, PropertyAttribute::MAYBEVOID, rPropValue );
                }
                catch( PropertyExistException& ) {}
                catch( IllegalTypeException& ) {
                    SAL_WARN( "sfx2.doc", "IllegalTypeException" );
                }
                catch( IllegalArgumentException& ) {
                    SAL_WARN( "sfx2.doc", "IllegalArgumentException" );
                }
            }
        }

        // To ensure a reloctable office installation, the path to the
        // office installtion directory must never be stored directly.
        if ( SfxURLRelocator_Impl::propertyCanContainOfficeDir( rPropName ) )
        {
            OUString aValue;
            if ( rPropValue >>= aValue )
            {
                maRelocator.makeRelocatableURL( aValue );
                aPropValue = makeAny( aValue );
            }
            else
            {
                Sequence< OUString > aValues;
                if ( rPropValue >>= aValues )
                {
                    for ( sal_Int32 n = 0; n < aValues.getLength(); n++ )
                    {
                        maRelocator.makeRelocatableURL( aValues[ n ] );
                    }
                    aPropValue = makeAny( aValues );
                }
                else
                {
                    OSL_FAIL( "Unsupported property value type" );
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
        uno::Reference< XPropertySetInfo > aPropInfo = rContent.getProperties();

        // check, whether or not the property exists
        if ( !aPropInfo.is() || !aPropInfo->hasPropertyByName( rPropName ) )
        {
            return sal_False;
        }

        // now get the property

        rPropValue = rContent.getPropertyValue( rPropName );

        // To ensure a reloctable office installation, the path to the
        // office installtion directory must never be stored directly.
        if ( SfxURLRelocator_Impl::propertyCanContainOfficeDir( rPropName ) )
        {
            OUString aValue;
            if ( rPropValue >>= aValue )
            {
                maRelocator.makeAbsoluteURL( aValue );
                rPropValue = makeAny( aValue );
            }
            else
            {
                Sequence< OUString > aValues;
                if ( rPropValue >>= aValues )
                {
                    for ( sal_Int32 n = 0; n < aValues.getLength(); n++ )
                    {
                        maRelocator.makeAbsoluteURL( aValues[ n ] );
                    }
                    rPropValue = makeAny( aValues );
                }
                else
                {
                    OSL_FAIL( "Unsupported property value type" );
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
bool SfxURLRelocator_Impl::propertyCanContainOfficeDir(
                                        const rtl::OUString & rPropName )
{
    // Note: TargetURL is handled by UCB itself (because it is a property
    //       with a predefined semantic). Additional Core properties introduced
    //       be a client app must be handled by the client app itself, because
    //       the UCB does not know the semantics of those properties.
    return ( rPropName == TARGET_DIR_URL || rPropName == PROPERTY_DIRLIST );
}

//-----------------------------------------------------------------------------
// public SfxDocTplService_Impl
//-----------------------------------------------------------------------------

SfxDocTplService_Impl::SfxDocTplService_Impl( uno::Reference< XMultiServiceFactory > xFactory )
: maRelocator( xFactory )
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
        mpUpdater->terminate();
        mpUpdater->join();
        delete mpUpdater;
    }

    for ( size_t i = 0, n = maNames.size(); i < n; ++i )
        delete maNames[ i ];
    maNames.clear();
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

    OUString aPropName( PROPERTY_NEEDSUPDATE  );
    Any      aValue;

    aValue <<= sal_True;
    setProperty( maRootContent, aPropName, aValue );

    GroupList_Impl  aGroupList;

    // get the entries from the hierarchy
    createFromContent( aGroupList, maRootContent, sal_True );

    // get the entries from the template directories
    sal_Int32   nCountDir = maTemplateDirs.getLength();
    OUString*   pDirs = maTemplateDirs.getArray();
    Content     aDirContent;

    // the last directory in the list must be writable
    sal_Bool bWriteableDirectory = sal_True;

    // the target folder might not exist, for this reason no interaction handler should be used
    uno::Reference< XCommandEnvironment > aQuietEnv;

    while ( nCountDir )
    {
        nCountDir--;
        if ( Content::create( pDirs[ nCountDir ], aQuietEnv, comphelper::getProcessComponentContext(), aDirContent ) )
        {
            createFromContent( aGroupList, aDirContent, sal_False, bWriteableDirectory );
        }

        bWriteableDirectory = sal_False;
    }

    // now check the list
    for( size_t j = 0, n = aGroupList.size(); j < n; ++j )
    {
        GroupData_Impl *pGroup = aGroupList[ j ];
        if ( pGroup->getInUse() )
        {
            if ( pGroup->getInHierarchy() )
            {
                Content aGroup;
                if ( Content::create( pGroup->getHierarchyURL(), maCmdEnv, comphelper::getProcessComponentContext(), aGroup ) )
                    setProperty( aGroup,
                                 OUString( TARGET_DIR_URL  ),
                                 makeAny( pGroup->getTargetURL() ) );

                size_t nCount = pGroup->count();
                for ( size_t i=0; i<nCount; i++ )
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
    }
    aGroupList.clear();

    aValue <<= sal_False;
    setProperty( maRootContent, aPropName, aValue );
}

//-----------------------------------------------------------------------------
uno::Sequence< beans::StringPair > SfxDocTplService_Impl::ReadUINamesForTemplateDir_Impl( const ::rtl::OUString& aUserPath )
{
    INetURLObject aLocObj( aUserPath );
    aLocObj.insertName( ::rtl::OUString( "groupuinames.xml"  ), false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
    Content aLocContent;

    // TODO/LATER: Use hashmap in future
    uno::Sequence< beans::StringPair > aUINames;
    if ( Content::create( aLocObj.GetMainURL( INetURLObject::NO_DECODE ), uno::Reference < ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext(), aLocContent ) )
    {
        try
        {
            uno::Reference< io::XInputStream > xLocStream = aLocContent.openStream();
            if ( xLocStream.is() )
                aUINames = DocTemplLocaleHelper::ReadGroupLocalizationSequence( xLocStream, mxFactory );
        }
        catch( uno::Exception& )
        {}
    }

    return aUINames;
}

//-----------------------------------------------------------------------------
sal_Bool SfxDocTplService_Impl::UpdateUINamesForTemplateDir_Impl( const ::rtl::OUString& aUserPath,
                                                                  const ::rtl::OUString& aGroupName,
                                                                  const ::rtl::OUString& aNewFolderName )
{
    uno::Sequence< beans::StringPair > aUINames = ReadUINamesForTemplateDir_Impl( aUserPath );
    sal_Int32 nLen = aUINames.getLength();

    // it is possible that the name is used already, but it should be checked before
    for ( sal_Int32 nInd = 0; nInd < nLen; nInd++ )
        if ( aUINames[nInd].First.equals( aNewFolderName ) )
            return sal_False;

    aUINames.realloc( ++nLen );
    aUINames[nLen-1].First = aNewFolderName;
    aUINames[nLen-1].Second = aGroupName;

    return WriteUINamesForTemplateDir_Impl( aUserPath, aUINames );
}

//-----------------------------------------------------------------------------
sal_Bool SfxDocTplService_Impl::ReplaceUINamesForTemplateDir_Impl( const ::rtl::OUString& aUserPath,
                                                                  const ::rtl::OUString& aDefaultFsysGroupName,
                                                                  const ::rtl::OUString& aOldGroupName,
                                                                  const ::rtl::OUString& aNewGroupName )
{
    uno::Sequence< beans::StringPair > aUINames = ReadUINamesForTemplateDir_Impl( aUserPath );
    sal_Int32 nLen = aUINames.getLength();

    sal_Bool bChanged = sal_False;
    for ( sal_Int32 nInd = 0; nInd < nLen; nInd++ )
        if ( aUINames[nInd].Second.equals( aOldGroupName ) )
        {
            aUINames[nInd].Second = aNewGroupName;
            bChanged = sal_True;
        }

    if ( !bChanged )
    {
        aUINames.realloc( ++nLen );
        aUINames[nLen-1].First = aDefaultFsysGroupName;
        aUINames[nLen-1].Second = aNewGroupName;
    }
    return WriteUINamesForTemplateDir_Impl( aUserPath, aUINames );
}

//-----------------------------------------------------------------------------
sal_Bool SfxDocTplService_Impl::RemoveUINamesForTemplateDir_Impl( const ::rtl::OUString& aUserPath,
                                                                  const ::rtl::OUString& aGroupName )
{
    uno::Sequence< beans::StringPair > aUINames = ReadUINamesForTemplateDir_Impl( aUserPath );
    sal_Int32 nLen = aUINames.getLength();
    uno::Sequence< beans::StringPair > aNewUINames( nLen );
    sal_Int32 nNewLen = 0;

    sal_Bool bChanged = sal_False;
    for ( sal_Int32 nInd = 0; nInd < nLen; nInd++ )
        if ( aUINames[nInd].Second.equals( aGroupName ) )
            bChanged = sal_True;
        else
        {
            nNewLen++;
            aNewUINames[nNewLen-1].First = aUINames[nInd].First;
            aNewUINames[nNewLen-1].Second = aUINames[nInd].Second;
        }

    aNewUINames.realloc( nNewLen );

    return bChanged ? WriteUINamesForTemplateDir_Impl( aUserPath, aNewUINames ) : sal_True;
}


//-----------------------------------------------------------------------------
sal_Bool SfxDocTplService_Impl::WriteUINamesForTemplateDir_Impl( const ::rtl::OUString& aUserPath,
                                                                const uno::Sequence< beans::StringPair >& aUINames )
{
    sal_Bool bResult = sal_False;
    try {
        uno::Reference< beans::XPropertySet > xTempFile(
                io::TempFile::create(comphelper::getComponentContext(mxFactory)),
                uno::UNO_QUERY_THROW );

        ::rtl::OUString aTempURL;
        uno::Any aUrl = xTempFile->getPropertyValue( ::rtl::OUString("Uri") );
        aUrl >>= aTempURL;

        uno::Reference< io::XStream > xStream( xTempFile, uno::UNO_QUERY_THROW );
        uno::Reference< io::XOutputStream > xOutStream = xStream->getOutputStream();
        if ( !xOutStream.is() )
            throw uno::RuntimeException();

        DocTemplLocaleHelper::WriteGroupLocalizationSequence( xOutStream, aUINames, mxFactory );
        try {
            // the SAX writer might close the stream
            xOutStream->closeOutput();
        } catch( uno::Exception& )
        {}

        Content aTargetContent( aUserPath, maCmdEnv, comphelper::getProcessComponentContext() );
        Content aSourceContent( aTempURL, maCmdEnv, comphelper::getProcessComponentContext() );
        aTargetContent.transferContent( aSourceContent,
                                        InsertOperation_COPY,
                                        ::rtl::OUString( "groupuinames.xml"  ),
                                        ucb::NameClash::OVERWRITE );
        bResult = sal_True;
    }
    catch ( uno::Exception& )
    {
    }

    return bResult;
}

//-----------------------------------------------------------------------------
::rtl::OUString SfxDocTplService_Impl::CreateNewGroupFsys( const ::rtl::OUString& rGroupName, Content& aGroup )
{
    ::rtl::OUString aResultURL;

    if ( maTemplateDirs.getLength() )
    {
        ::rtl::OUString aTargetPath = maTemplateDirs[ maTemplateDirs.getLength() - 1 ];

        // create a new folder with the given name
        Content aNewFolder;
        ::rtl::OUString aNewFolderName;

        // the Fsys name instead of GroupName should be used, the groupuinames must be added also
        if ( !CreateNewUniqueFolderWithPrefix( aTargetPath,
                                                rGroupName,
                                                aNewFolderName,
                                                aResultURL,
                                                aNewFolder )
          && !CreateNewUniqueFolderWithPrefix( aTargetPath,
                                                ::rtl::OUString( "UserGroup"  ),
                                                aNewFolderName,
                                                aResultURL,
                                                aNewFolder ) )

            return ::rtl::OUString();

        if ( !UpdateUINamesForTemplateDir_Impl( aTargetPath, rGroupName, aNewFolderName ) )
        {
            // we could not create the groupuinames for the folder, so we delete the group in the
            // the folder and return
            removeContent( aNewFolder );
            return ::rtl::OUString();
        }

        // Now set the target url for this group and we are done
        OUString aPropName( TARGET_DIR_URL  );
        Any aValue = makeAny( aResultURL );

        if ( ! setProperty( aGroup, aPropName, aValue ) )
        {
            removeContent( aNewFolder );
            return ::rtl::OUString();
        }
    }

    return aResultURL;
}

//-----------------------------------------------------------------------------
sal_Bool SfxDocTplService_Impl::addGroup( const OUString& rGroupName )
{
    ::osl::MutexGuard aGuard( maMutex );

    // Check, whether or not there is a group with this name
    Content      aNewGroup;
    OUString        aNewGroupURL;
    INetURLObject   aNewGroupObj( maRootURL );

    aNewGroupObj.insertName( rGroupName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );

    aNewGroupURL = aNewGroupObj.GetMainURL( INetURLObject::NO_DECODE );

    if ( Content::create( aNewGroupURL, maCmdEnv, comphelper::getProcessComponentContext(), aNewGroup ) ||
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
    Content      aNewFolder;
    OUString        aNewFolderName;
    OUString        aNewFolderURL;

    // the Fsys name instead of GroupName should be used, the groupuinames must be added also
    if ( !CreateNewUniqueFolderWithPrefix( aUserPath,
                                            rGroupName,
                                            aNewFolderName,
                                            aNewFolderURL,
                                            aNewFolder )
      && !CreateNewUniqueFolderWithPrefix( aUserPath,
                                            ::rtl::OUString( "UserGroup"  ),
                                            aNewFolderName,
                                            aNewFolderURL,
                                            aNewFolder ) )
    {
        // we could not create the folder, so we delete the group in the
        // hierarchy and return
        removeContent( aNewGroup );
        return sal_False;
    }

    if ( !UpdateUINamesForTemplateDir_Impl( aUserPath, rGroupName, aNewFolderName ) )
    {
        // we could not create the groupuinames for the folder, so we delete the group in the
        // hierarchy, the folder and return
        removeContent( aNewGroup );
        removeContent( aNewFolder );
        return sal_False;
    }

    // Now set the target url for this group and we are done
    OUString aPropName( TARGET_DIR_URL  );
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
    // remove all the elements that have the prefix aTargetURL
    // if the group does not have other elements remove it

    ::osl::MutexGuard aGuard( maMutex );

    sal_Bool bResult = sal_False;

    // create the group url
    INetURLObject aGroupObj( maRootURL );
    aGroupObj.insertName( rGroupName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );

    // Get the target url
    Content  aGroup;
    OUString    aGroupURL = aGroupObj.GetMainURL( INetURLObject::NO_DECODE );

    if ( Content::create( aGroupURL, maCmdEnv, comphelper::getProcessComponentContext(), aGroup ) )
    {
        OUString    aPropName( TARGET_DIR_URL  );
        Any      aValue;

        OUString    aGroupTargetURL;
        if ( getProperty( aGroup, aPropName, aValue ) )
            aValue >>= aGroupTargetURL;

        if ( aGroupTargetURL.isEmpty() )
            return sal_False; // nothing is allowed to be removed

        if ( !maTemplateDirs.getLength() )
            return sal_False;
        ::rtl::OUString aGeneralTempPath = maTemplateDirs[ maTemplateDirs.getLength() - 1 ];

        // check that the fs location is in writeble folder and this is not a "My templates" folder
        INetURLObject aGroupParentFolder( aGroupTargetURL );
        if ( !aGroupParentFolder.removeSegment()
          || !::utl::UCBContentHelper::IsSubPath( aGeneralTempPath,
                                                      aGroupParentFolder.GetMainURL( INetURLObject::NO_DECODE ) ) )
            return sal_False;

        // now get the content of the Group
        uno::Reference< XResultSet > xResultSet;
        Sequence< OUString > aProps( 1 );

        aProps[0] = OUString(TARGET_URL );

        try
        {
            sal_Bool bHasNonRemovable = sal_False;
            sal_Bool bHasShared = sal_False;

            ResultSetInclude eInclude = INCLUDE_DOCUMENTS_ONLY;
            xResultSet = aGroup.createCursor( aProps, eInclude );

            if ( xResultSet.is() )
            {
                uno::Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY_THROW );
                uno::Reference< XRow > xRow( xResultSet, UNO_QUERY_THROW );

                while ( xResultSet->next() )
                {
                    OUString aTemplTargetURL( xRow->getString( 1 ) );
                    OUString aHierURL = xContentAccess->queryContentIdentifierString();

                    if ( ::utl::UCBContentHelper::IsSubPath( aGroupTargetURL, aTemplTargetURL ) )
                    {
                        // this is a user template, and it can be removed
                        if ( removeContent( aTemplTargetURL ) )
                            removeContent( aHierURL );
                        else
                            bHasNonRemovable = sal_True;
                    }
                    else
                        bHasShared = sal_True;
                }

                if ( !bHasNonRemovable && !bHasShared )
                {
                    if ( removeContent( aGroupTargetURL )
                      || !::utl::UCBContentHelper::Exists( aGroupTargetURL ) )
                    {
                        removeContent( aGroupURL );
                        RemoveUINamesForTemplateDir_Impl( aGeneralTempPath, rGroupName );
                        bResult = sal_True; // the operation is successful only if the whole group is removed
                    }
                }
                else if ( !bHasNonRemovable )
                {
                    if ( removeContent( aGroupTargetURL )
                      || !::utl::UCBContentHelper::Exists( aGroupTargetURL ) )
                    {
                        RemoveUINamesForTemplateDir_Impl( aGeneralTempPath, rGroupName );
                        setProperty( aGroup, aPropName, uno::makeAny( ::rtl::OUString() ) );
                    }
                }
            }
        }
        catch ( Exception& ) {}
    }

    return bResult;
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
    if ( Content::create( aGroupURL, maCmdEnv, comphelper::getProcessComponentContext(), aGroup ) )
        return sal_False;

    aGroupObj.removeSegment();
    aGroupObj.insertName( rOldName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
    aGroupURL = aGroupObj.GetMainURL( INetURLObject::NO_DECODE );

    // When there is no group with the old name, we can't rename it
    if ( ! Content::create( aGroupURL, maCmdEnv, comphelper::getProcessComponentContext(), aGroup ) )
        return sal_False;

    OUString aGroupTargetURL;
    // there is no need to check whether target dir url is in target path, since if the target path is changed
    // the target dir url should be already generated new
    OUString    aPropName( TARGET_DIR_URL  );
    Any      aValue;
    if ( getProperty( aGroup, aPropName, aValue ) )
        aValue >>= aGroupTargetURL;

    if ( aGroupTargetURL.isEmpty() )
        return sal_False;

    if ( !maTemplateDirs.getLength() )
        return sal_False;
    ::rtl::OUString aGeneralTempPath = maTemplateDirs[ maTemplateDirs.getLength() - 1 ];

    // check that the fs location is in writeble folder and this is not a "My templates" folder
    INetURLObject aGroupParentFolder( aGroupTargetURL );
    if ( !aGroupParentFolder.removeSegment()
      || !::utl::UCBContentHelper::IsSubPath( aGeneralTempPath,
                                                  aGroupParentFolder.GetMainURL( INetURLObject::NO_DECODE ) ) )
        return sal_False;

    // check that the group can be renamed ( all the contents must be in target location )
    sal_Bool bCanBeRenamed = sal_False;
       try
       {
        uno::Reference< XResultSet > xResultSet;
        Sequence< OUString > aProps( 1 );

        aProps[0] = OUString(TARGET_URL );
        ResultSetInclude eInclude = INCLUDE_DOCUMENTS_ONLY;
        xResultSet = aGroup.createCursor( aProps, eInclude );

        if ( xResultSet.is() )
        {
               uno::Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY_THROW );
               uno::Reference< XRow > xRow( xResultSet, UNO_QUERY_THROW );

               while ( xResultSet->next() )
               {
                   OUString aTemplTargetURL( xRow->getString( 1 ) );

                if ( !::utl::UCBContentHelper::IsSubPath( aGroupTargetURL, aTemplTargetURL ) )
                    throw uno::Exception();
            }

            bCanBeRenamed = sal_True;
        }
    }
    catch ( Exception& ) {}

    if ( bCanBeRenamed )
    {
        INetURLObject aGroupTargetObj( aGroupTargetURL );
        ::rtl::OUString aFsysName = aGroupTargetObj.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );

        if ( aGroupTargetObj.removeSegment()
          && ReplaceUINamesForTemplateDir_Impl( aGroupTargetObj.GetMainURL( INetURLObject::NO_DECODE ),
                                                  aFsysName,
                                                rOldName,
                                                rNewName ) )
        {
            // rename the group in the hierarchy
            OUString aTitleProp( TITLE  );
            Any aTitleValue;
            aTitleValue <<= rNewName;

            return setProperty( aGroup, aTitleProp, aTitleValue );
        }
    }

    return sal_False;
}

//-----------------------------------------------------------------------------
sal_Bool SfxDocTplService_Impl::storeTemplate( const OUString& rGroupName,
                                               const OUString& rTemplateName,
                                               const uno::Reference< XSTORABLE >& rStorable )
{
    ::osl::MutexGuard aGuard( maMutex );

    // Check, whether or not there is a group with this name
    // Return false, if there is no group with the given name
    Content         aGroup, aTemplate, aTargetGroup, aTemplateToRemove;
    OUString        aGroupURL, aTemplateURL, aTemplateToRemoveTargetURL;
    INetURLObject   aGroupObj( maRootURL );
    sal_Bool        bRemoveOldTemplateContent = sal_False;
    ::rtl::OUString sDocServiceName;

    aGroupObj.insertName( rGroupName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
    aGroupURL = aGroupObj.GetMainURL( INetURLObject::NO_DECODE );

    if ( ! Content::create( aGroupURL, maCmdEnv, comphelper::getProcessComponentContext(), aGroup ) )
        return sal_False;

    ::rtl::OUString aGroupTargetURL;
    ::rtl::OUString aPropName( TARGET_DIR_URL  );
    Any      aValue;
    if ( getProperty( aGroup, aPropName, aValue ) )
        aValue >>= aGroupTargetURL;


    // Check, if there's a template with the given name in this group
    // the target template should be overwritten if it is imported by user
    // in case the template is installed by office installation of by an add-in
    // it can not be replaced
    aGroupObj.insertName( rTemplateName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
    aTemplateURL = aGroupObj.GetMainURL( INetURLObject::NO_DECODE );

    if ( Content::create( aTemplateURL, maCmdEnv, comphelper::getProcessComponentContext(), aTemplateToRemove ) )
    {
        OUString    aTargetTemplPropName( TARGET_URL  );

        bRemoveOldTemplateContent = sal_True;
        if ( getProperty( aTemplateToRemove, aTargetTemplPropName, aValue ) )
            aValue >>= aTemplateToRemoveTargetURL;

        if ( aGroupTargetURL.isEmpty() || !maTemplateDirs.getLength()
          || (!aTemplateToRemoveTargetURL.isEmpty() && !::utl::UCBContentHelper::IsSubPath( maTemplateDirs[ maTemplateDirs.getLength() - 1 ], aTemplateToRemoveTargetURL )) )
            return sal_False; // it is not allowed to remove the template
    }

    try
    {
        uno::Reference< lang::XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
        if ( !xFactory.is() )
            throw uno::RuntimeException();

        // get document service name
        uno::Reference< frame::XModuleManager2 > xModuleManager(
            frame::ModuleManager::create(comphelper::getComponentContext(xFactory)) );
        sDocServiceName = xModuleManager->identify( uno::Reference< uno::XInterface >( rStorable, uno::UNO_QUERY ) );
        if ( sDocServiceName.isEmpty() )
            throw uno::RuntimeException();

        // get the actual filter name
        ::rtl::OUString aFilterName;

        uno::Reference< lang::XMultiServiceFactory > xConfigProvider(
                xFactory->createInstance(
                    ::rtl::OUString("com.sun.star.configuration.ConfigurationProvider") ),
                uno::UNO_QUERY_THROW );

        uno::Sequence< uno::Any > aArgs( 1 );
        beans::PropertyValue aPathProp;
        aPathProp.Name = ::rtl::OUString("nodepath");
        aPathProp.Value <<= ::rtl::OUString( "/org.openoffice.Setup/Office/Factories/"  );
        aArgs[0] <<= aPathProp;

        uno::Reference< container::XNameAccess > xSOFConfig(
            xConfigProvider->createInstanceWithArguments(
                                    ::rtl::OUString("com.sun.star.configuration.ConfigurationAccess"),
                                    aArgs ),
            uno::UNO_QUERY_THROW );

        uno::Reference< container::XNameAccess > xApplConfig;
        xSOFConfig->getByName( sDocServiceName ) >>= xApplConfig;
        if ( !xApplConfig.is() )
            throw uno::RuntimeException();

        xApplConfig->getByName( ::rtl::OUString( "ooSetupFactoryActualTemplateFilter"  ) ) >>= aFilterName;
        if ( aFilterName.isEmpty() )
            throw uno::RuntimeException();

        // find the related type name
        ::rtl::OUString aTypeName;
        uno::Reference< container::XNameAccess > xFilterFactory(
            xFactory->createInstance( ::rtl::OUString("com.sun.star.document.FilterFactory") ),
            uno::UNO_QUERY_THROW );

        uno::Sequence< beans::PropertyValue > aFilterData;
        xFilterFactory->getByName( aFilterName ) >>= aFilterData;
        for ( sal_Int32 nInd = 0; nInd < aFilterData.getLength(); nInd++ )
            if ( aFilterData[nInd].Name == "Type" )
                aFilterData[nInd].Value >>= aTypeName;

        if ( aTypeName.isEmpty() )
            throw uno::RuntimeException();

        // find the mediatype and extension
        uno::Reference< container::XNameAccess > xTypeDetection =
            mxType.is() ?
                uno::Reference< container::XNameAccess >( mxType, uno::UNO_QUERY_THROW ) :
                uno::Reference< container::XNameAccess >(
                    xFactory->createInstance( ::rtl::OUString("com.sun.star.document.TypeDetection") ),
                    uno::UNO_QUERY_THROW );

        SequenceAsHashMap aTypeProps( xTypeDetection->getByName( aTypeName ) );
        uno::Sequence< ::rtl::OUString > aAllExt =
            aTypeProps.getUnpackedValueOrDefault( ::rtl::OUString("Extensions"), Sequence< ::rtl::OUString >() );
        if ( !aAllExt.getLength() )
            throw uno::RuntimeException();

        ::rtl::OUString aMediaType = aTypeProps.getUnpackedValueOrDefault( ::rtl::OUString("MediaType"), ::rtl::OUString() );
        ::rtl::OUString aExt = aAllExt[0];

        if ( aMediaType.isEmpty() || aExt.isEmpty() )
            throw uno::RuntimeException();

        // construct destination url
        if ( aGroupTargetURL.isEmpty() )
        {
            aGroupTargetURL = CreateNewGroupFsys( rGroupName, aGroup );

            if ( aGroupTargetURL.isEmpty() )
                throw uno::RuntimeException();
        }

        ::rtl::OUString aNewTemplateTargetURL = CreateNewUniqueFileWithPrefix( aGroupTargetURL, rTemplateName, aExt );
        if ( aNewTemplateTargetURL.isEmpty() )
        {
            aNewTemplateTargetURL = CreateNewUniqueFileWithPrefix( aGroupTargetURL, ::rtl::OUString( "UserTemplate"  ), aExt );

            if ( aNewTemplateTargetURL.isEmpty() )
                throw uno::RuntimeException();
        }

        // store template
        uno::Sequence< PropertyValue > aStoreArgs( 2 );
        aStoreArgs[0].Name = ::rtl::OUString("FilterName");
        aStoreArgs[0].Value <<= aFilterName;
        aStoreArgs[1].Name = ::rtl::OUString("DocumentTitle");
        aStoreArgs[1].Value <<= rTemplateName;

        ::rtl::OUString aCurrentDocumentURL = rStorable->getLocation();
        if( !::utl::UCBContentHelper::EqualURLs( aNewTemplateTargetURL, rStorable->getLocation() ))
            rStorable->storeToURL( aNewTemplateTargetURL, aStoreArgs );
        else
            rStorable->store();

        // the storing was successful, now the old template with the same name can be removed if it existed
        if ( !aTemplateToRemoveTargetURL.isEmpty() )
        {
            removeContent( aTemplateToRemoveTargetURL );

            /*
             * pb: #i79496#
             * if the old template was the standard template
             * it is necessary to change the standard template with the new file name
             */
            String sStdTmplFile = SfxObjectFactory::GetStandardTemplate( sDocServiceName );
            if ( INetURLObject( sStdTmplFile ) == INetURLObject( aTemplateToRemoveTargetURL ) )
            {
                SfxObjectFactory::SetStandardTemplate( sDocServiceName, aNewTemplateTargetURL );
            }
        }

        if ( bRemoveOldTemplateContent )
            removeContent( aTemplateToRemove );

        // add the template to hierarchy
        return addEntry( aGroup, rTemplateName, aNewTemplateTargetURL, aMediaType );
    }
    catch( Exception& )
    {
        // the template was not stored
        return sal_False;
    }
}

//-----------------------------------------------------------------------------
sal_Bool SfxDocTplService_Impl::addTemplate( const OUString& rGroupName,
                                             const OUString& rTemplateName,
                                             const OUString& rSourceURL )
{
    ::osl::MutexGuard aGuard( maMutex );

    // Check, whether or not there is a group with this name
    // Return false, if there is no group with the given name
    Content         aGroup, aTemplate, aTargetGroup;
    OUString        aGroupURL, aTemplateURL;
    INetURLObject   aGroupObj( maRootURL );

    aGroupObj.insertName( rGroupName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
    aGroupURL = aGroupObj.GetMainURL( INetURLObject::NO_DECODE );

    if ( ! Content::create( aGroupURL, maCmdEnv, comphelper::getProcessComponentContext(), aGroup ) )
        return sal_False;

    // Check, if there's a template with the given name in this group
    // Return false, if there already is a template
    aGroupObj.insertName( rTemplateName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
    aTemplateURL = aGroupObj.GetMainURL( INetURLObject::NO_DECODE );

    if ( Content::create( aTemplateURL, maCmdEnv, comphelper::getProcessComponentContext(), aTemplate ) )
        return sal_False;

    // get the target url of the group
    OUString    aTargetURL;
    OUString    aPropName( TARGET_DIR_URL  );
    Any         aValue;

    if ( getProperty( aGroup, aPropName, aValue ) )
        aValue >>= aTargetURL;

    if ( aTargetURL.isEmpty() )
    {
        aTargetURL = CreateNewGroupFsys( rGroupName, aGroup );

        if ( aTargetURL.isEmpty() )
            return sal_False;
    }

    // Get the content type
    OUString aTitle, aType, aTargetURL2, aFullName;

    sal_Bool bDocHasTitle = sal_False;
    if( !getTitleFromURL( rSourceURL, aTitle, aType, bDocHasTitle ) )
        return sal_False;

    INetURLObject   aSourceObj( rSourceURL );
    if ( rTemplateName.equals( aTitle ) )
    {
        // addTemplate will sometimes be called just to add an entry in the
        // hierarchy; the target URL and the source URL will be the same in
        // this scenario
        // TODO/LATER: get rid of this old hack

        INetURLObject   aTargetObj( aTargetURL );

        aTargetObj.insertName( rTemplateName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
        aTargetObj.setExtension( aSourceObj.getExtension() );

        aTargetURL2 = aTargetObj.GetMainURL( INetURLObject::NO_DECODE );

        if ( aTargetURL2 == rSourceURL )
            return addEntry( aGroup, rTemplateName, aTargetURL2, aType );
    }

    // copy the template into the new group (targeturl)

    INetURLObject aTmpURL( aSourceObj );
    aTmpURL.CutExtension();
    ::rtl::OUString aPattern = aTmpURL.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );

    ::rtl::OUString aNewTemplateTargetURL = CreateNewUniqueFileWithPrefix( aTargetURL, aPattern, aSourceObj.getExtension() );
    INetURLObject aNewTemplateTargetObj( aNewTemplateTargetURL );
    ::rtl::OUString aNewTemplateTargetName = aNewTemplateTargetObj.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );
    if ( aNewTemplateTargetURL.isEmpty() || aNewTemplateTargetName.isEmpty() )
        return sal_False;

    // get access to source file
    Content aSourceContent;
    uno::Reference < ucb::XCommandEnvironment > xEnv;
    INetURLObject   aSourceURL( rSourceURL );
    if( ! Content::create( aSourceURL.GetMainURL( INetURLObject::NO_DECODE ), xEnv, comphelper::getProcessComponentContext(), aSourceContent ) )
        return sal_False;

    if( ! Content::create( aTargetURL, xEnv, comphelper::getProcessComponentContext(), aTargetGroup ) )
        return sal_False;

    // transfer source file
    try
    {
        if( ! aTargetGroup.transferContent( aSourceContent,
                                                InsertOperation_COPY,
                                                aNewTemplateTargetName,
                                                NameClash::OVERWRITE ) )
            return sal_False;

        // allow to edit the added template
        Content aResultContent;
        if ( Content::create( aNewTemplateTargetURL, xEnv, comphelper::getProcessComponentContext(), aResultContent ) )
        {
            ::rtl::OUString aPropertyName( "IsReadOnly"  );
            uno::Any aProperty;
            sal_Bool bReadOnly = sal_False;
            if ( getProperty( aResultContent, aPropertyName, aProperty ) && ( aProperty >>= bReadOnly ) && bReadOnly )
                setProperty( aResultContent, aPropertyName, uno::makeAny( (sal_Bool)sal_False ) );
        }
    }
    catch ( ContentCreationException& )
    { return sal_False; }
    catch ( Exception& )
    { return sal_False; }


    // either the document has title and it is the same as requested, or we have to set it
    sal_Bool bCorrectTitle = ( bDocHasTitle && aTitle.equals( rTemplateName ) );
    if ( !bCorrectTitle )
    {
        if ( !bDocHasTitle )
        {
            INetURLObject aNewTmpObj( aNewTemplateTargetObj );
            aNewTmpObj.CutExtension();
            bCorrectTitle = ( aNewTmpObj.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET ).equals( rTemplateName ) );
        }

        if ( !bCorrectTitle )
            bCorrectTitle = setTitleForURL( aNewTemplateTargetURL, rTemplateName );
    }

    if ( bCorrectTitle )
    {
        // create a new entry in the hierarchy
        return addEntry( aGroup, rTemplateName, aNewTemplateTargetURL, aType );
    }

    // TODO/LATER: The user could be notified here that the renaming has failed
    // create a new entry in the hierarchy
    addEntry( aGroup, aTitle, aNewTemplateTargetURL, aType );
    return sal_False;
}

//-----------------------------------------------------------------------------
sal_Bool SfxDocTplService_Impl::removeTemplate( const OUString& rGroupName,
                                                const OUString& rTemplateName )
{
    ::osl::MutexGuard aGuard( maMutex );

    // Check, whether or not there is a group with this name
    // Return false, if there is no group with the given name
    Content         aGroup, aTemplate;
    OUString        aGroupURL, aTemplateURL;
    INetURLObject   aGroupObj( maRootURL );

    aGroupObj.insertName( rGroupName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
    aGroupURL = aGroupObj.GetMainURL( INetURLObject::NO_DECODE );

    if ( ! Content::create( aGroupURL, maCmdEnv, comphelper::getProcessComponentContext(), aGroup ) )
        return sal_False;

    // Check, if there's a template with the given name in this group
    // Return false, if there is no template
    aGroupObj.insertName( rTemplateName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
    aTemplateURL = aGroupObj.GetMainURL( INetURLObject::NO_DECODE );

    if ( !Content::create( aTemplateURL, maCmdEnv, comphelper::getProcessComponentContext(), aTemplate ) )
        return sal_False;

    // get the target URL from the template
    OUString    aTargetURL;
    OUString    aPropName( TARGET_URL  );
    Any         aValue;

    if ( getProperty( aTemplate, aPropName, aValue ) )
        aValue >>= aTargetURL;

    // delete the target template
    if ( !aTargetURL.isEmpty() )
    {
        if ( !maTemplateDirs.getLength()
          || !::utl::UCBContentHelper::IsSubPath( maTemplateDirs[ maTemplateDirs.getLength() - 1 ], aTargetURL ) )
            return sal_False;

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

    // Check, whether or not there is a group with this name
    // Return false, if there is no group with the given name
    Content         aGroup, aTemplate;
    OUString        aGroupURL, aTemplateURL;
    INetURLObject   aGroupObj( maRootURL );

    aGroupObj.insertName( rGroupName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
    aGroupURL = aGroupObj.GetMainURL( INetURLObject::NO_DECODE );

    if ( ! Content::create( aGroupURL, maCmdEnv, comphelper::getProcessComponentContext(), aGroup ) )
        return sal_False;

    // Check, if there's a template with the new name in this group
    // Return false, if there is one
    aGroupObj.insertName( rNewName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
    aTemplateURL = aGroupObj.GetMainURL( INetURLObject::NO_DECODE );

    if ( Content::create( aTemplateURL, maCmdEnv, comphelper::getProcessComponentContext(), aTemplate ) )
        return sal_False;

    // Check, if there's a template with the old name in this group
    // Return false, if there is no template
    aGroupObj.removeSegment();
    aGroupObj.insertName( rOldName, false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );
    aTemplateURL = aGroupObj.GetMainURL( INetURLObject::NO_DECODE );

    if ( !Content::create( aTemplateURL, maCmdEnv, comphelper::getProcessComponentContext(), aTemplate ) )
        return sal_False;

    OUString    aTemplateTargetURL;
    OUString    aTargetProp( TARGET_URL  );
    Any         aTargetValue;

    if ( getProperty( aTemplate, aTargetProp, aTargetValue ) )
        aTargetValue >>= aTemplateTargetURL;

    if ( !setTitleForURL( aTemplateTargetURL, rNewName ) )
        return sal_False;

    // rename the template entry in the cache
    OUString    aTitleProp( TITLE  );
    Any         aTitleValue;
    aTitleValue <<= rNewName;

    return setProperty( aTemplate, aTitleProp, aTitleValue );
}

//-----------------------------------------------------------------------------

SFX_IMPL_XSERVICEINFO( SfxDocTplService, TEMPLATE_SERVICE_NAME, TEMPLATE_IMPLEMENTATION_NAME )
SFX_IMPL_SINGLEFACTORY( SfxDocTplService )

//-----------------------------------------------------------------------------
SfxDocTplService::SfxDocTplService( const uno::Reference< XMultiServiceFactory >& xFactory )
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
uno::Reference< XCONTENT > SAL_CALL SfxDocTplService::getContent()
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
                                                   const uno::Reference< XSTORABLE >& Storable )
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
WaitWindow_Impl::WaitWindow_Impl()
    : WorkWindow( NULL, WB_BORDER | WB_3DLOOK )
{
    Rectangle aRect = Rectangle( 0, 0, 300, 30000 );
    _nTextStyle = TEXT_DRAW_CENTER | TEXT_DRAW_VCENTER | TEXT_DRAW_WORDBREAK | TEXT_DRAW_MULTILINE;
    _aText = SfxResId( RID_CNT_STR_WAITING ).toString();
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
void WaitWindow_Impl::Paint( const Rectangle& /*rRect*/ )
{
    DrawText( _aRect, _aText, _nTextStyle );
}

//-----------------------------------------------------------------------------
void SfxDocTplService_Impl::addHierGroup( GroupList_Impl& rList,
                                          const OUString& rTitle,
                                          const OUString& rOwnURL )
{
    // now get the content of the Group
    Content                 aContent;
    uno::Reference< XResultSet > xResultSet;
    Sequence< OUString >    aProps(3);

    aProps[0] = OUString(TITLE );
    aProps[1] = OUString(TARGET_URL );
    aProps[2] = OUString(PROPERTY_TYPE );

    try
    {
        aContent = Content( rOwnURL, maCmdEnv, comphelper::getProcessComponentContext() );
        ResultSetInclude eInclude = INCLUDE_DOCUMENTS_ONLY;
        xResultSet = aContent.createCursor( aProps, eInclude );
    }
    catch ( ContentCreationException& )
    {
        SAL_WARN( "sfx2.doc", "addHierGroup: ContentCreationException" );
    }
    catch ( Exception& ) {}

    if ( xResultSet.is() )
    {
        GroupData_Impl *pGroup = new GroupData_Impl( rTitle );
        pGroup->setHierarchy( sal_True );
        pGroup->setHierarchyURL( rOwnURL );
        rList.push_back( pGroup );

        uno::Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY );
        uno::Reference< XRow > xRow( xResultSet, UNO_QUERY );

        try
        {
            while ( xResultSet->next() )
            {
                sal_Bool             bUpdateType = sal_False;
                DocTemplates_EntryData_Impl  *pData;

                OUString aTitle( xRow->getString( 1 ) );
                OUString aTargetDir( xRow->getString( 2 ) );
                OUString aType( xRow->getString( 3 ) );
                OUString aHierURL = xContentAccess->queryContentIdentifierString();

                if ( aType.isEmpty() )
                {
                    OUString aTmpTitle;

                    sal_Bool bDocHasTitle = sal_False;
                    if( !getTitleFromURL( aTargetDir, aTmpTitle, aType, bDocHasTitle ) )
                    {
                        SAL_WARN( "sfx2.doc", "addHierGroup(): template of alien format" );
                        continue;
                    }

                    if ( !aType.isEmpty() )
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
                                          const OUString& rUITitle,
                                          const OUString& rOwnURL,
                                          sal_Bool bWriteableGroup )
{
    ::rtl::OUString aTitle;

    if ( rUITitle.isEmpty() )
    {
        // reserved FS names that should not be used
        if ( rTitle.compareToAscii( "wizard" ) == 0 )
            return;
        else if ( rTitle.compareToAscii( "internal" ) == 0 )
            return;

        aTitle = getLongName( rTitle );
    }
    else
        aTitle = rUITitle;

    if ( aTitle.isEmpty() )
        return;

    GroupData_Impl* pGroup = NULL;
    for ( size_t i = 0, n = rList.size(); i < n; ++i )
    {
        if ( rList[ i ]->getTitle() == aTitle )
        {
            pGroup = rList[ i ];
            break;
        }
    }

    if ( !pGroup )
    {
        pGroup = new GroupData_Impl( aTitle );
        rList.push_back( pGroup );
    }

    if ( bWriteableGroup )
        pGroup->setTargetURL( rOwnURL );

    pGroup->setInUse();

    // now get the content of the Group
    Content                 aContent;
    uno::Reference< XResultSet > xResultSet;
    Sequence< OUString >    aProps(1);
    aProps[0] = OUString(TITLE );

    try
    {
        // this method is only used during checking of the available template-folders
        // that should happen quietly
        uno::Reference< XCommandEnvironment > aQuietEnv;
        aContent = Content( rOwnURL, aQuietEnv, comphelper::getProcessComponentContext() );
        ResultSetInclude eInclude = INCLUDE_DOCUMENTS_ONLY;
        xResultSet = aContent.createCursor( aProps, eInclude );
    }
    catch ( Exception& ) {}

    if ( xResultSet.is() )
    {
        uno::Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY );
        uno::Reference< XRow > xRow( xResultSet, UNO_QUERY );

        try
        {
            while ( xResultSet->next() )
            {
                OUString aChildTitle( xRow->getString( 1 ) );
                OUString aTargetURL = xContentAccess->queryContentIdentifierString();
                OUString aType;
                OUString aHierURL;

                if ( aChildTitle.compareToAscii( "sfx.tlx" ) == 0 || aChildTitle == "groupuinames.xml" )
                    continue;

                sal_Bool bDocHasTitle = sal_False;
                if( !getTitleFromURL( aTargetURL, aChildTitle, aType, bDocHasTitle ) )
                    continue;

                pGroup->addEntry( aChildTitle, aTargetURL, aType, aHierURL );
            }
        }
        catch ( Exception& ) {}
    }
}

// -----------------------------------------------------------------------
void SfxDocTplService_Impl::createFromContent( GroupList_Impl& rList,
                                               Content &rContent,
                                               sal_Bool bHierarchy,
                                               sal_Bool bWriteableContent )
{
    OUString aTargetURL = rContent.get()->getIdentifier()->getContentIdentifier();

    // when scanning the file system, we have to add the 'standard' group, too
    if ( ! bHierarchy )
    {
        OUString aUIStdTitle = getLongName( OUString( STANDARD_FOLDER  ) );
        addFsysGroup( rList, ::rtl::OUString(), aUIStdTitle, aTargetURL, bWriteableContent );
    }

    // search for predefined UI names
    INetURLObject aLayerObj( aTargetURL );

    // TODO/LATER: Use hashmap in future
    uno::Sequence< beans::StringPair > aUINames;
    if ( !bHierarchy )
        aUINames = ReadUINamesForTemplateDir_Impl( aLayerObj.GetMainURL( INetURLObject::NO_DECODE ) );

    uno::Reference< XResultSet > xResultSet;
    Sequence< OUString > aProps(1);
    aProps[0] = OUString(TITLE );

    try
    {
        ResultSetInclude eInclude = INCLUDE_FOLDERS_ONLY;
        xResultSet = rContent.createCursor( aProps, eInclude );
    }
    catch ( Exception& ) {}

    if ( xResultSet.is() )
    {
        uno::Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY );
        uno::Reference< XRow > xRow( xResultSet, UNO_QUERY );

        try
        {
            while ( xResultSet->next() )
            {
                // TODO/LATER: clarify the encoding of the Title
                OUString aTitle( xRow->getString( 1 ) );
                OUString aTargetSubfolderURL( xContentAccess->queryContentIdentifierString() );

                if ( bHierarchy )
                    addHierGroup( rList, aTitle, aTargetSubfolderURL );
                else
                {
                    ::rtl::OUString aUITitle;
                    for ( sal_Int32 nInd = 0; nInd < aUINames.getLength(); nInd++ )
                        if ( aUINames[nInd].First.equals( aTitle ) )
                        {
                            aUITitle = aUINames[nInd].Second;
                            break;
                        }

                    addFsysGroup( rList, aTitle, aUITitle, aTargetSubfolderURL, bWriteableContent );
                }
            }
        }
        catch ( Exception& ) {}
    }
}

//-----------------------------------------------------------------------------
void SfxDocTplService_Impl::removeFromHierarchy( DocTemplates_EntryData_Impl *pData )
{
    Content aTemplate;

    if ( Content::create( pData->getHierarchyURL(), maCmdEnv, comphelper::getProcessComponentContext(), aTemplate ) )
    {
        removeContent( aTemplate );
    }
}

//-----------------------------------------------------------------------------
void SfxDocTplService_Impl::addToHierarchy( GroupData_Impl *pGroup,
                                            DocTemplates_EntryData_Impl *pData )
{
    Content aGroup, aTemplate;

    if ( ! Content::create( pGroup->getHierarchyURL(), maCmdEnv, comphelper::getProcessComponentContext(), aGroup ) )
        return;

    // Check, if there's a template with the given name in this group
    // Return if there is already a template
    INetURLObject aGroupObj( pGroup->getHierarchyURL() );

    aGroupObj.insertName( pData->getTitle(), false,
                      INetURLObject::LAST_SEGMENT, true,
                      INetURLObject::ENCODE_ALL );

    OUString aTemplateURL = aGroupObj.GetMainURL( INetURLObject::NO_DECODE );

    if ( Content::create( aTemplateURL, maCmdEnv, comphelper::getProcessComponentContext(), aTemplate ) )
        return;

    addEntry( aGroup, pData->getTitle(),
              pData->getTargetURL(),
              pData->getType() );
}

//-----------------------------------------------------------------------------
void SfxDocTplService_Impl::updateData( DocTemplates_EntryData_Impl *pData )
{
    Content aTemplate;

    if ( ! Content::create( pData->getHierarchyURL(), maCmdEnv, comphelper::getProcessComponentContext(), aTemplate ) )
        return;

    OUString aPropName;

    if ( pData->getUpdateType() )
    {
        aPropName = OUString( PROPERTY_TYPE  );
        setProperty( aTemplate, aPropName, makeAny( pData->getType() ) );
    }

    if ( pData->getUpdateLink() )
    {
        aPropName = OUString( TARGET_URL  );
        setProperty( aTemplate, aPropName, makeAny( pData->getTargetURL() ) );
    }
}

//-----------------------------------------------------------------------------
void SfxDocTplService_Impl::addGroupToHierarchy( GroupData_Impl *pGroup )
{
    OUString aAdditionalProp( TARGET_DIR_URL  );
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

        sal_uIntPtr nCount = pGroup->count();
        for ( sal_uIntPtr i=0; i<nCount; i++ )
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

    if ( Content::create( pGroup->getHierarchyURL(), maCmdEnv, comphelper::getProcessComponentContext(), aGroup ) )
    {
        removeContent( aGroup );
    }
}

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
    for ( size_t i = 0, n = maEntries.size(); i < n; ++i )
        delete maEntries[ i ];
    maEntries.clear();
}

// -----------------------------------------------------------------------
DocTemplates_EntryData_Impl* GroupData_Impl::addEntry( const OUString& rTitle,
                                          const OUString& rTargetURL,
                                          const OUString& rType,
                                          const OUString& rHierURL )
{
    DocTemplates_EntryData_Impl* pData = NULL;
    bool EntryFound = false;

    for ( size_t i = 0, n = maEntries.size(); i < n; ++i )
    {
        pData = maEntries[ i ];
        if ( pData->getTitle() == rTitle )
        {
            EntryFound = true;
            break;
        }
    }

    if ( !EntryFound )
    {
        pData = new DocTemplates_EntryData_Impl( rTitle );
        pData->setTargetURL( rTargetURL );
        pData->setType( rType );
        if ( !rHierURL.isEmpty() )
        {
            pData->setHierarchyURL( rHierURL );
            pData->setHierarchy( sal_True );
        }
        maEntries.push_back( pData );
    }
    else
    {
        if ( !rHierURL.isEmpty() )
        {
            pData->setHierarchyURL( rHierURL );
            pData->setHierarchy( sal_True );
        }

        if ( pData->getInHierarchy() )
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
DocTemplates_EntryData_Impl::DocTemplates_EntryData_Impl( const OUString& rTitle )
{
    maTitle         = rTitle;
    mbInUse         = sal_False;
    mbInHierarchy   = sal_False;
    mbUpdateType    = sal_False;
    mbUpdateLink    = sal_False;
}

// -----------------------------------------------------------------------
SfxURLRelocator_Impl::SfxURLRelocator_Impl( uno::Reference< XMultiServiceFactory > xFactory )
: mxFactory( xFactory )
{
}

// -----------------------------------------------------------------------
SfxURLRelocator_Impl::~SfxURLRelocator_Impl()
{
}

// -----------------------------------------------------------------------
void SfxURLRelocator_Impl::initOfficeInstDirs()
{
    if ( !mxOfficeInstDirs.is() )
    {
        osl::MutexGuard aGuard( maMutex );
        if ( !mxOfficeInstDirs.is() )
        {
            OSL_ENSURE( mxFactory.is(), "No service manager!" );

            uno::Reference< XComponentContext > xCtx(
                comphelper::getComponentContext( mxFactory ) );

            xCtx->getValueByName(
                rtl::OUString(
                    "/singletons/"
                    "com.sun.star.util.theOfficeInstallationDirectories"  ) )
                >>= mxOfficeInstDirs;

            OSL_ENSURE( mxOfficeInstDirs.is(),
                        "Unable to obtain office installation directory "
                        "singleton!" );
        }
    }
}

// -----------------------------------------------------------------------
void SfxURLRelocator_Impl::implExpandURL( ::rtl::OUString& io_url )
{
    const INetURLObject aParser( io_url );
    if ( aParser.GetProtocol() != INET_PROT_VND_SUN_STAR_EXPAND )
        return;

    io_url = aParser.GetURLPath( INetURLObject::DECODE_WITH_CHARSET );
    try
    {
        if ( !mxMacroExpander.is() )
        {
            ::comphelper::ComponentContext aContext( mxFactory );
            mxMacroExpander.set( aContext.getSingleton( "com.sun.star.util.theMacroExpander" ), UNO_QUERY_THROW );
        }
        io_url = mxMacroExpander->expandMacros( io_url );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

// -----------------------------------------------------------------------
void SfxURLRelocator_Impl::makeRelocatableURL( rtl::OUString & rURL )
{
    if ( !rURL.isEmpty() )
    {
        initOfficeInstDirs();
        implExpandURL( rURL );
        rURL = mxOfficeInstDirs->makeRelocatableURL( rURL );
    }
}

// -----------------------------------------------------------------------
void SfxURLRelocator_Impl::makeAbsoluteURL( rtl::OUString & rURL )
{
    if ( !rURL.isEmpty() )
    {
        initOfficeInstDirs();
        implExpandURL( rURL );
        rURL = mxOfficeInstDirs->makeAbsoluteURL( rURL );
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
