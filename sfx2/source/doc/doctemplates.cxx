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

#include <osl/mutex.hxx>
#include <tools/diagnose_ex.h>
#include <tools/urlobj.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/resary.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <unotools/pathoptions.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/string.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/beans/IllegalTypeException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyExistException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/util/theMacroExpander.hpp>
#include <com/sun/star/util/theOfficeInstallationDirectories.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/document/XTypeDetection.hpp>
#include <com/sun/star/document/DocumentProperties.hpp>
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
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/XDocumentTemplates.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XLocalizable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/thePathSettings.hpp>

#include <rtl/ref.hxx>
#include <svtools/templatefoldercache.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/ucbhelper.hxx>

#include <sfx2/sfxresid.hxx>
#include "sfxurlrelocator.hxx"
#include "doctemplateslocal.hxx"
#include <sfx2/docfac.hxx>
#include <sfx2/docfile.hxx>
#include "doc.hrc"

#include <vector>

#define SERVICENAME_TYPEDETECTION "com.sun.star.document.TypeDetection"

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

#define STANDARD_FOLDER         "standard"

#define C_DELIM                 ';'

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

using namespace ::ucbhelper;
using namespace ::comphelper;

using ::std::vector;

namespace {

class WaitWindow_Impl : public WorkWindow
{
    Rectangle     maRect;
    OUString      maText;
    const DrawTextFlags mnTextStyle = DrawTextFlags::Center | DrawTextFlags::VCenter | DrawTextFlags::WordBreak | DrawTextFlags::MultiLine;

public:
    WaitWindow_Impl();
    virtual ~WaitWindow_Impl() override;
    virtual void dispose() override;
    virtual void Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect) override;
};

#define X_OFFSET 15
#define Y_OFFSET 15


struct NamePair_Impl
{
    OUString maShortName;
    OUString maLongName;
};

class DocTemplates_EntryData_Impl;
class GroupData_Impl;

typedef vector< GroupData_Impl* > GroupList_Impl;


class TplTaskEnvironment : public ::cppu::WeakImplHelper< ucb::XCommandEnvironment >
{
    uno::Reference< task::XInteractionHandler >               m_xInteractionHandler;
    uno::Reference< ucb::XProgressHandler >                   m_xProgressHandler;

public:
    explicit TplTaskEnvironment( const uno::Reference< task::XInteractionHandler>& rxInteractionHandler )
                                : m_xInteractionHandler( rxInteractionHandler )
                            {}

    virtual uno::Reference<task::XInteractionHandler> SAL_CALL getInteractionHandler() override
    { return m_xInteractionHandler; }

    virtual uno::Reference<ucb::XProgressHandler> SAL_CALL    getProgressHandler() override
    { return m_xProgressHandler; }
};

class SfxDocTplService_Impl
{
    uno::Reference< XComponentContext >              mxContext;
    uno::Reference< XCommandEnvironment >            maCmdEnv;
    uno::Reference< XDocumentProperties>             m_xDocProps;
    uno::Reference< XTypeDetection >                 mxType;

    ::osl::Mutex                maMutex;
    Sequence< OUString >        maTemplateDirs;
    Sequence< OUString >        maInternalTemplateDirs;
    OUString                    maRootURL;
    std::vector< NamePair_Impl* > maNames;
    lang::Locale                maLocale;
    Content                     maRootContent;
    bool                        mbIsInitialized : 1;
    bool                        mbLocaleSet     : 1;

    SfxURLRelocator_Impl        maRelocator;

    void                        init_Impl();
    void                        getDefaultLocale();
    void                        getDirList();
    void                        readFolderList();
    bool                        needsUpdate();
    OUString                    getLongName( const OUString& rShortName );
    bool                    setTitleForURL( const OUString& rURL, const OUString& aTitle );
    bool                    getTitleFromURL( const OUString& rURL, OUString& aTitle, OUString& aType, bool& bDocHasTitle );

    bool                    addEntry( Content& rParentFolder,
                                          const OUString& rTitle,
                                          const OUString& rTargetURL,
                                          const OUString& rType );

    bool                    createFolder( const OUString& rNewFolderURL,
                                              bool  bCreateParent,
                                              bool  bFsysFolder,
                                              Content   &rNewFolder );

    static bool             CreateNewUniqueFolderWithPrefix( const OUString& aPath,
                                                                const OUString& aPrefix,
                                                                OUString& aNewFolderName,
                                                                OUString& aNewFolderURL,
                                                                Content& aNewFolder );
    static OUString         CreateNewUniqueFileWithPrefix( const OUString& aPath,
                                                                const OUString& aPrefix,
                                                                const OUString& aExt );

    std::vector< beans::StringPair > ReadUINamesForTemplateDir_Impl( const OUString& aUserPath );
    bool                    UpdateUINamesForTemplateDir_Impl( const OUString& aUserPath,
                                                                  const OUString& aGroupName,
                                                                  const OUString& aNewFolderName );
    bool                    ReplaceUINamesForTemplateDir_Impl( const OUString& aUserPath,
                                                                  const OUString& aFsysGroupName,
                                                                  const OUString& aOldGroupName,
                                                                  const OUString& aNewGroupName );
    void                    RemoveUINamesForTemplateDir_Impl( const OUString& aUserPath,
                                                                  const OUString& aGroupName );
    bool                    WriteUINamesForTemplateDir_Impl( const OUString& aUserPath,
                                                                const std::vector< beans::StringPair >& aUINames );

    OUString                CreateNewGroupFsys( const OUString& rGroupName, Content& aGroup );

    static bool             removeContent( Content& rContent );
    bool                    removeContent( const OUString& rContentURL );

    bool                    setProperty( Content& rContent,
                                             const OUString& rPropName,
                                             const Any& rPropValue );
    bool                    getProperty( Content& rContent,
                                             const OUString& rPropName,
                                             Any& rPropValue );

    void                        createFromContent( GroupList_Impl& rList,
                                                   Content &rContent,
                                                   bool bHierarchy,
                                                   bool bWriteableContent );
    void                        addHierGroup( GroupList_Impl& rList,
                                              const OUString& rTitle,
                                              const OUString& rOwnURL );
    void                        addFsysGroup( GroupList_Impl& rList,
                                              const OUString& rTitle,
                                              const OUString& rUITitle,
                                              const OUString& rOwnURL,
                                              bool bWriteableGroup );
    void                        removeFromHierarchy( DocTemplates_EntryData_Impl *pData );
    void                        addToHierarchy( GroupData_Impl *pGroup,
                                                DocTemplates_EntryData_Impl *pData );

    void                        removeFromHierarchy( GroupData_Impl *pGroup );
    void                        addGroupToHierarchy( GroupData_Impl *pGroup );

    void                        updateData( DocTemplates_EntryData_Impl *pData );

    //See: #i66157# and rhbz#1065807
    //return which template dir the rURL is a subpath of
    OUString                    findParentTemplateDir(const OUString& rURL) const;

    //See: #i66157# and rhbz#1065807
    //return true if rURL is a path (or subpath of) a dir which is not a user path
    //which implies neither it or its contents can be removed
    bool                        isInternalTemplateDir(const OUString& rURL) const;
public:
    explicit                    SfxDocTplService_Impl( const uno::Reference< XComponentContext > & xContext );
                                ~SfxDocTplService_Impl();

    bool                        init() { if ( !mbIsInitialized ) init_Impl(); return mbIsInitialized; }
    const Content&              getContent() const { return maRootContent; }

    void                        setLocale( const lang::Locale & rLocale );
    lang::Locale                getLocale();

    bool                        storeTemplate( const OUString& rGroupName,
                                               const OUString& rTemplateName,
                                               const uno::Reference< frame::XStorable >& rStorable );

    bool                        addTemplate( const OUString& rGroupName,
                                             const OUString& rTemplateName,
                                             const OUString& rSourceURL );
    bool                        removeTemplate( const OUString& rGroupName,
                                                const OUString& rTemplateName );
    bool                        renameTemplate( const OUString& rGroupName,
                                                const OUString& rOldName,
                                                const OUString& rNewName );

    bool                        addGroup( const OUString& rGroupName );
    bool                        removeGroup( const OUString& rGroupName );
    bool                        renameGroup( const OUString& rOldName,
                                             const OUString& rNewName );

    void                        update();
    void                        doUpdate();
};


class DocTemplates_EntryData_Impl
{
    OUString            maTitle;
    OUString            maType;
    OUString            maTargetURL;
    OUString            maHierarchyURL;

    bool            mbInHierarchy   : 1;
    bool            mbInUse         : 1;
    bool            mbUpdateType    : 1;
    bool            mbUpdateLink    : 1;

public:
   explicit             DocTemplates_EntryData_Impl( const OUString& rTitle );

    void                setInUse() { mbInUse = true; }
    void                setHierarchy( bool bInHierarchy ) { mbInHierarchy = bInHierarchy; }
    void                setUpdateLink( bool bUpdateLink ) { mbUpdateLink = bUpdateLink; }
    void                setUpdateType( bool bUpdateType ) { mbUpdateType = bUpdateType; }

    bool                getInUse() const { return mbInUse; }
    bool                getInHierarchy() const { return mbInHierarchy; }
    bool                getUpdateLink() const { return mbUpdateLink; }
    bool                getUpdateType() const { return mbUpdateType; }

    const OUString&     getHierarchyURL() const { return maHierarchyURL; }
    const OUString&     getTargetURL() const { return maTargetURL; }
    const OUString&     getTitle() const { return maTitle; }
    const OUString&     getType() const { return maType; }

    void                setHierarchyURL( const OUString& rURL ) { maHierarchyURL = rURL; }
    void                setTargetURL( const OUString& rURL ) { maTargetURL = rURL; }
    void                setType( const OUString& rType ) { maType = rType; }
};


class GroupData_Impl
{
    vector< DocTemplates_EntryData_Impl* > maEntries;
    OUString            maTitle;
    OUString            maHierarchyURL;
    OUString            maTargetURL;
    bool            mbInUse         : 1;
    bool            mbInHierarchy   : 1;

public:
    explicit            GroupData_Impl( const OUString& rTitle );
                        ~GroupData_Impl();

    void                setInUse() { mbInUse = true; }
    void                setHierarchy( bool bInHierarchy ) { mbInHierarchy = bInHierarchy; }
    void                setHierarchyURL( const OUString& rURL ) { maHierarchyURL = rURL; }
    void                setTargetURL( const OUString& rURL ) { maTargetURL = rURL; }

    bool            getInUse() const { return mbInUse; }
    bool            getInHierarchy() const { return mbInHierarchy; }
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


// private SfxDocTplService_Impl

void SfxDocTplService_Impl::init_Impl()
{
    uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    uno::Reference < task::XInteractionHandler > xInteractionHandler(
        task::InteractionHandler::createWithParent(xContext, nullptr), uno::UNO_QUERY_THROW );
    maCmdEnv = new TplTaskEnvironment( xInteractionHandler );

    ::osl::ClearableMutexGuard aGuard( maMutex );
    bool bIsInitialized = false;
    bool bNeedsUpdate   = false;

    if ( !mbLocaleSet )
        getDefaultLocale();

    // convert locale to string
    OUString aLang = LanguageTag::convertToBcp47( maLocale);

    // set maRootContent to the root of the templates hierarchy. Create the
    // entry if necessary

    maRootURL = ( TEMPLATE_ROOT_URL "/" ) + aLang;

    OUString aTemplVersPropName( TEMPLATE_VERSION  );
    OUString aTemplVers( TEMPLATE_VERSION_VALUE  );
    if ( Content::create( maRootURL, maCmdEnv, comphelper::getProcessComponentContext(), maRootContent ) )
    {
        uno::Any aValue;
        OUString aPropValue;
        if ( getProperty( maRootContent, aTemplVersPropName, aValue )
          && ( aValue >>= aPropValue )
          && aPropValue.equals( aTemplVers ) )
        {
            bIsInitialized = true;
        }
        else
            removeContent( maRootContent );
    }

    if ( !bIsInitialized )
    {
        if ( createFolder( maRootURL, true, false, maRootContent )
          && setProperty( maRootContent, aTemplVersPropName, uno::makeAny( aTemplVers ) ) )
            bIsInitialized = true;

        bNeedsUpdate = true;
    }

    if ( bIsInitialized )
    {
        try {
            m_xDocProps.set(document::DocumentProperties::create(
                        ::comphelper::getProcessComponentContext()));
        } catch (uno::RuntimeException const& e) {
            SAL_WARN("sfx.doc", "SfxDocTplService_Impl::init_Impl: "
                "cannot create DocumentProperties service:" << e.Message);
        }

        OUString const aService = SERVICENAME_TYPEDETECTION;
        mxType.set( mxContext->getServiceManager()->createInstanceWithContext(aService, mxContext), UNO_QUERY );

        getDirList();
        readFolderList();

        if ( bNeedsUpdate )
        {
            aGuard.clear();
            SolarMutexClearableGuard aSolarGuard;

            VclPtrInstance< WaitWindow_Impl > pWin;
            aSolarGuard.clear();
            ::osl::ClearableMutexGuard anotherGuard( maMutex );

            update();

            anotherGuard.clear();
            SolarMutexGuard aSecondSolarGuard;

            pWin.disposeAndClear();
        }
        else if ( needsUpdate() )
            // the UI should be shown only on the first update
            update();
    }
    else
    {
        SAL_WARN( "sfx.doc", "init_Impl(): Could not create root" );
    }

    mbIsInitialized = bIsInitialized;
}


void SfxDocTplService_Impl::getDefaultLocale()
{
    if ( !mbLocaleSet )
    {
        ::osl::MutexGuard aGuard( maMutex );
        if ( !mbLocaleSet )
        {
            maLocale = LanguageTag::convertToLocale( utl::ConfigManager::getLocale(), false);
            mbLocaleSet = true;
        }
    }
}


void SfxDocTplService_Impl::readFolderList()
{
    SolarMutexGuard aGuard;

    ResStringArray  aShortNames( SfxResId( TEMPLATE_SHORT_NAMES_ARY ) );
    ResStringArray  aLongNames( SfxResId( TEMPLATE_LONG_NAMES_ARY ) );


    sal_uInt16 nCount = (sal_uInt16)( std::min( aShortNames.Count(), aLongNames.Count() ) );

    for ( sal_uInt16 i=0; i<nCount; i++ )
    {
        NamePair_Impl* pPair = new NamePair_Impl;
        pPair->maShortName  = aShortNames.GetString( i );
        pPair->maLongName   = aLongNames.GetString( i );

        maNames.push_back( pPair );
    }
}


OUString SfxDocTplService_Impl::getLongName( const OUString& rShortName )
{
    OUString         aRet;

    for (NamePair_Impl* pPair : maNames)
    {
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


void SfxDocTplService_Impl::getDirList()
{
    OUString aPropName( PROPERTY_DIRLIST  );
    Any      aValue;

    // Get the template dir list
    // TODO/LATER: let use service, register listener
    INetURLObject   aURL;
    OUString    aDirs = SvtPathOptions().GetTemplatePath();
    sal_Int32 nCount = comphelper::string::getTokenCount(aDirs, C_DELIM);

    maTemplateDirs = Sequence< OUString >( nCount );

    uno::Reference< util::XMacroExpander > xExpander = util::theMacroExpander::get(mxContext);
    const OUString aPrefix(
        "vnd.sun.star.expand:"  );

    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        aURL.SetSmartProtocol( INetProtocol::File );
        aURL.SetURL( aDirs.getToken( i, C_DELIM ) );
        maTemplateDirs[i] = aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );

        sal_Int32 nIndex = maTemplateDirs[i].indexOf( aPrefix );
        if ( nIndex != -1 && xExpander.is() )
        {
            maTemplateDirs[i] = maTemplateDirs[i].replaceAt(nIndex,
                                                            aPrefix.getLength(),
                                                            OUString());
            maTemplateDirs[i] = xExpander->expandMacros( maTemplateDirs[i] );
        }
    }

    aValue <<= maTemplateDirs;

    css::uno::Reference< css::util::XPathSettings > xPathSettings =
        css::util::thePathSettings::get(mxContext);

    // load internal paths
    OUString sProp( "Template_internal" );
    Any aAny = xPathSettings->getPropertyValue( sProp );
    aAny >>= maInternalTemplateDirs;

    nCount = maInternalTemplateDirs.getLength();
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        //expand vnd.sun.star.expand: and remove "..." from them
        //to normalize into the expected url patterns
        maRelocator.makeRelocatableURL(maInternalTemplateDirs[i]);
        maRelocator.makeAbsoluteURL(maInternalTemplateDirs[i]);
    }

    // Store the template dir list
    setProperty( maRootContent, aPropName, aValue );
}


bool SfxDocTplService_Impl::needsUpdate()
{
    OUString aPropName( PROPERTY_NEEDSUPDATE  );
    bool bNeedsUpdate = true;
    Any      aValue;

    // Get the template dir list
    bool bHasProperty = getProperty( maRootContent, aPropName, aValue );

    if ( bHasProperty )
        aValue >>= bNeedsUpdate;

    // the old template component also checks this state, but it is initialized from this component
    // so if this component was already updated the old component does not need such an update
    ::svt::TemplateFolderCache aTempCache;
    if ( !bNeedsUpdate )
        bNeedsUpdate = aTempCache.needsUpdate();

    if ( bNeedsUpdate )
        aTempCache.storeState();

    return bNeedsUpdate;
}


bool SfxDocTplService_Impl::setTitleForURL( const OUString& rURL, const OUString& aTitle )
{
    if (m_xDocProps.is())
    {
        try
        {
            m_xDocProps->loadFromMedium(rURL, Sequence<PropertyValue>());
            m_xDocProps->setTitle(aTitle);

            uno::Reference< embed::XStorage > xStorage = ::comphelper::OStorageHelper::GetStorageFromURL(
                    rURL, embed::ElementModes::READWRITE);

            uno::Sequence<beans::PropertyValue> medium(2);
            medium[0].Name = "DocumentBaseURL";
            medium[0].Value <<= rURL;
            medium[1].Name = "URL";
            medium[1].Value <<= rURL;

            m_xDocProps->storeToStorage(xStorage, medium);
            return true;
        }
        catch ( Exception& )
        {
        }
    }
    return false;
}


bool SfxDocTplService_Impl::getTitleFromURL( const OUString& rURL, OUString& aTitle, OUString& aType, bool& bDocHasTitle )
{
    bDocHasTitle = false;

    if (m_xDocProps.is())
    {
        try
        {
            m_xDocProps->loadFromMedium(rURL, Sequence<PropertyValue>());
            aTitle = m_xDocProps->getTitle();
        }
        catch ( Exception& )
        {
        }
    }

    if ( aType.isEmpty() && mxType.is() )
    {
        OUString aDocType = mxType->queryTypeByURL( rURL );
        if ( !aDocType.isEmpty() )
            try
            {
                uno::Reference< container::XNameAccess > xTypeDetection( mxType, uno::UNO_QUERY_THROW );
                SequenceAsHashMap aTypeProps( xTypeDetection->getByName( aDocType ) );
                aType = aTypeProps.getUnpackedValueOrDefault(
                            "MediaType",
                            OUString() );
            }
            catch( uno::Exception& )
            {}
    }

    if ( aTitle.isEmpty() )
    {
        INetURLObject aURL( rURL );
        aURL.CutExtension();
        aTitle = aURL.getName( INetURLObject::LAST_SEGMENT, true,
                               INetURLObject::DecodeMechanism::WithCharset );
    }
    else
        bDocHasTitle = true;

    return true;
}


bool SfxDocTplService_Impl::addEntry( Content& rParentFolder,
                                          const OUString& rTitle,
                                          const OUString& rTargetURL,
                                          const OUString& rType )
{
    bool bAddedEntry = false;

    INetURLObject aLinkObj( rParentFolder.getURL() );
    aLinkObj.insertName( rTitle, false,
                      INetURLObject::LAST_SEGMENT,
                      INetURLObject::EncodeMechanism::All );
    OUString aLinkURL = aLinkObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );

    Content aLink;

    if ( ! Content::create( aLinkURL, maCmdEnv, comphelper::getProcessComponentContext(), aLink ) )
    {
        Sequence< OUString > aNames(3);
        aNames[0] = TITLE;
        aNames[1] = IS_FOLDER;
        aNames[2] = TARGET_URL;

        Sequence< Any > aValues(3);
        aValues[0] = makeAny( rTitle );
        aValues[1] = makeAny( false );
        aValues[2] = makeAny( rTargetURL );

        OUString aType( TYPE_LINK  );
        OUString aAdditionalProp( PROPERTY_TYPE  );

        try
        {
            rParentFolder.insertNewContent( aType, aNames, aValues, aLink );
            setProperty( aLink, aAdditionalProp, makeAny( rType ) );
            bAddedEntry = true;
        }
        catch( Exception& )
        {}
    }
    return bAddedEntry;
}


bool SfxDocTplService_Impl::createFolder( const OUString& rNewFolderURL,
                                              bool  bCreateParent,
                                              bool  bFsysFolder,
                                              Content   &rNewFolder )
{
    Content         aParent;
    bool        bCreatedFolder = false;
    INetURLObject   aParentURL( rNewFolderURL );
    OUString        aFolderName = aParentURL.getName( INetURLObject::LAST_SEGMENT, true,
                                                      INetURLObject::DecodeMechanism::WithCharset );

    // compute the parent folder url from the new folder url
    // and remove the final slash, because Content::create doesn't
    // like it
    aParentURL.removeSegment();
    if ( aParentURL.getSegmentCount() >= 1 )
        aParentURL.removeFinalSlash();

    // if the parent exists, we can continue with the creation of the
    // new folder, we have to create the parent otherwise ( as long as
    // bCreateParent is set to true )
    if ( Content::create( aParentURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), maCmdEnv, comphelper::getProcessComponentContext(), aParent ) )
    {
        try
        {
            Sequence< OUString > aNames(2);
            aNames[0] = TITLE;
            aNames[1] = IS_FOLDER;

            Sequence< Any > aValues(2);
            aValues[0] = makeAny( aFolderName );
            aValues[1] = makeAny( true );

            OUString aType;

            if ( bFsysFolder )
                aType = TYPE_FSYS_FOLDER;
            else
                aType = TYPE_FOLDER;

            aParent.insertNewContent( aType, aNames, aValues, rNewFolder );
            bCreatedFolder = true;
        }
        catch( RuntimeException& )
        {
            SAL_WARN( "sfx.doc", "createFolder(): got runtime exception" );
        }
        catch( Exception& )
        {
            SAL_WARN( "sfx.doc", "createFolder(): Could not create new folder" );
        }
    }
    else if ( bCreateParent )
    {
        // if the parent doesn't exists and bCreateParent is set to true,
        // we try to create the parent and if this was successful, we
        // try to create the new folder again ( but this time, we set
        // bCreateParent to false to avoid endless recursions )
        if ( ( aParentURL.getSegmentCount() >= 1 ) &&
               createFolder( aParentURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), bCreateParent, bFsysFolder, aParent ) )
        {
            bCreatedFolder = createFolder( rNewFolderURL, false, bFsysFolder, rNewFolder );
        }
    }

    return bCreatedFolder;
}


bool SfxDocTplService_Impl::CreateNewUniqueFolderWithPrefix( const OUString& aPath,
                                                                const OUString& aPrefix,
                                                                OUString& aNewFolderName,
                                                                OUString& aNewFolderURL,
                                                                Content& aNewFolder )
{
    bool bCreated = false;
    INetURLObject aDirPath( aPath );

    Content aParent;
    uno::Reference< XCommandEnvironment > aQuietEnv;
       if ( Content::create( aDirPath.GetMainURL( INetURLObject::DecodeMechanism::NONE ), aQuietEnv, comphelper::getProcessComponentContext(), aParent ) )
       {
        for ( sal_Int32 nInd = 0; nInd < 32000; nInd++ )
        {
            OUString aTryName = aPrefix;
            if ( nInd )
                aTryName += OUString::number( nInd );

            try
            {
                Sequence< OUString > aNames(2);
                aNames[0] = TITLE;
                aNames[1] = IS_FOLDER;

                Sequence< Any > aValues(2);
                aValues[0] = makeAny( aTryName );
                aValues[1] = makeAny( true );

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
                      INetURLObject::LAST_SEGMENT,
                      INetURLObject::EncodeMechanism::All );
                // if there is already an element, retry
                // if there was another error, do not try any more
                if ( !::utl::UCBContentHelper::Exists( aObjPath.GetMainURL( INetURLObject::DecodeMechanism::NONE ) ) )
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


OUString SfxDocTplService_Impl::CreateNewUniqueFileWithPrefix( const OUString& aPath,
                                                                        const OUString& aPrefix,
                                                                        const OUString& aExt )
{
    OUString aNewFileURL;
    INetURLObject aDirPath( aPath );

       Content aParent;

    uno::Reference< XCommandEnvironment > aQuietEnv;
    if ( Content::create( aDirPath.GetMainURL( INetURLObject::DecodeMechanism::NONE ), aQuietEnv, comphelper::getProcessComponentContext(), aParent ) )
       {
        for ( sal_Int32 nInd = 0; nInd < 32000; nInd++ )
        {
            Content aNewFile;
            bool bCreated = false;
            OUString aTryName = aPrefix;
            if ( nInd )
                aTryName += OUString::number( nInd );
            if ( aExt.toChar() != '.' )
                aTryName += ".";
            aTryName += aExt;

            try
            {
                Sequence< OUString > aNames(2);
                aNames[0] = TITLE;
                aNames[1] = IS_DOCUMENT;

                Sequence< Any > aValues(2);
                aValues[0] = makeAny( aTryName );
                aValues[1] = makeAny( true );

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
                      INetURLObject::LAST_SEGMENT,
                      INetURLObject::EncodeMechanism::All );
                // if there is already an element, retry
                // if there was another error, do not try any more
                if ( !::utl::UCBContentHelper::Exists( aObjPath.GetMainURL( INetURLObject::DecodeMechanism::NONE ) ) )
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


bool SfxDocTplService_Impl::removeContent( Content& rContent )
{
    bool bRemoved = false;
    try
    {
        OUString aCmd( COMMAND_DELETE  );
        Any aArg = makeAny( true );

        rContent.executeCommand( aCmd, aArg );
        bRemoved = true;
    }
    catch ( RuntimeException& ) {}
    catch ( Exception& ) {}

    return bRemoved;
}


bool SfxDocTplService_Impl::removeContent( const OUString& rContentURL )
{
    Content aContent;

    if ( Content::create( rContentURL, maCmdEnv, comphelper::getProcessComponentContext(), aContent ) )
        return removeContent( aContent );
    else
        return false;
}


bool SfxDocTplService_Impl::setProperty( Content& rContent,
                                             const OUString& rPropName,
                                             const Any& rPropValue )
{
    bool bPropertySet = false;

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
                    SAL_WARN( "sfx.doc", "IllegalTypeException" );
                }
                catch( IllegalArgumentException& ) {
                    SAL_WARN( "sfx.doc", "IllegalArgumentException" );
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
        bPropertySet = true;
    }
    catch ( RuntimeException& ) {}
    catch ( Exception& ) {}

    return bPropertySet;
}


bool SfxDocTplService_Impl::getProperty(Content& rContent, const OUString& rPropName, Any& rPropValue)
{
    bool bGotProperty = false;

    // Get the property
    try
    {
        uno::Reference< XPropertySetInfo > aPropInfo = rContent.getProperties();

        // check, whether or not the property exists
        if ( !aPropInfo.is() || !aPropInfo->hasPropertyByName( rPropName ) )
        {
            return false;
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

        bGotProperty = true;
    }
    catch ( RuntimeException& ) {}
    catch ( Exception& ) {}

    return bGotProperty;
}

SfxDocTplService_Impl::SfxDocTplService_Impl( const uno::Reference< XComponentContext > & xContext )
    : maRelocator(xContext)
{
    mxContext       = xContext;
    mbIsInitialized = false;
    mbLocaleSet     = false;
}


SfxDocTplService_Impl::~SfxDocTplService_Impl()
{
    ::osl::MutexGuard aGuard( maMutex );

    for (NamePair_Impl* p : maNames)
        delete p;
    maNames.clear();
}


lang::Locale SfxDocTplService_Impl::getLocale()
{
    ::osl::MutexGuard aGuard( maMutex );

    if ( !mbLocaleSet )
        getDefaultLocale();

    return maLocale;
}


void SfxDocTplService_Impl::setLocale( const lang::Locale &rLocale )
{
    ::osl::MutexGuard aGuard( maMutex );

    if ( mbLocaleSet && (
         ( maLocale.Language != rLocale.Language ) ||
         ( maLocale.Country  != rLocale.Country  ) ||
         ( maLocale.Variant  != rLocale.Variant  ) ) )
        mbIsInitialized = false;

    maLocale    = rLocale;
    mbLocaleSet = true;
}


void SfxDocTplService_Impl::update()
{
    ::osl::MutexGuard aGuard( maMutex );

    doUpdate();
}


void SfxDocTplService_Impl::doUpdate()
{
    ::osl::MutexGuard aGuard( maMutex );

    OUString aPropName( PROPERTY_NEEDSUPDATE  );
    Any      aValue;

    aValue <<= true;
    setProperty( maRootContent, aPropName, aValue );

    GroupList_Impl  aGroupList;

    // get the entries from the hierarchy
    createFromContent( aGroupList, maRootContent, true, false );

    // get the entries from the template directories
    sal_Int32   nCountDir = maTemplateDirs.getLength();
    OUString*   pDirs = maTemplateDirs.getArray();
    Content     aDirContent;

    // the last directory in the list must be writable
    bool bWriteableDirectory = true;

    // the target folder might not exist, for this reason no interaction handler should be used
    uno::Reference< XCommandEnvironment > aQuietEnv;

    while ( nCountDir )
    {
        nCountDir--;
        if ( Content::create( pDirs[ nCountDir ], aQuietEnv, comphelper::getProcessComponentContext(), aDirContent ) )
        {
            createFromContent( aGroupList, aDirContent, false, bWriteableDirectory );
        }

        bWriteableDirectory = false;
    }

    // now check the list
    for(GroupData_Impl* pGroup : aGroupList)
    {
        if ( pGroup->getInUse() )
        {
            if ( pGroup->getInHierarchy() )
            {
                Content aGroup;
                if ( Content::create( pGroup->getHierarchyURL(), maCmdEnv, comphelper::getProcessComponentContext(), aGroup ) )
                    setProperty( aGroup,
                                 TARGET_DIR_URL,
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

    aValue <<= false;
    setProperty( maRootContent, aPropName, aValue );
}


std::vector< beans::StringPair > SfxDocTplService_Impl::ReadUINamesForTemplateDir_Impl( const OUString& aUserPath )
{
    INetURLObject aLocObj( aUserPath );
    aLocObj.insertName( "groupuinames.xml", false,
                      INetURLObject::LAST_SEGMENT,
                      INetURLObject::EncodeMechanism::All );
    Content aLocContent;

    // TODO/LATER: Use hashmap in future
    std::vector< beans::StringPair > aUINames;
    if ( Content::create( aLocObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ), uno::Reference < ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext(), aLocContent ) )
    {
        try
        {
            uno::Reference< io::XInputStream > xLocStream = aLocContent.openStream();
            if ( xLocStream.is() )
                aUINames = DocTemplLocaleHelper::ReadGroupLocalizationSequence( xLocStream, mxContext );
        }
        catch( uno::Exception& )
        {}
    }

    return aUINames;
}


bool SfxDocTplService_Impl::UpdateUINamesForTemplateDir_Impl( const OUString& aUserPath,
                                                                  const OUString& aGroupName,
                                                                  const OUString& aNewFolderName )
{
    std::vector< beans::StringPair > aUINames = ReadUINamesForTemplateDir_Impl( aUserPath );
    sal_Int32 nLen = aUINames.size();

    // it is possible that the name is used already, but it should be checked before
    for ( sal_Int32 nInd = 0; nInd < nLen; nInd++ )
        if ( aUINames[nInd].First.equals( aNewFolderName ) )
            return false;

    aUINames.resize( ++nLen );
    aUINames[nLen-1].First = aNewFolderName;
    aUINames[nLen-1].Second = aGroupName;

    return WriteUINamesForTemplateDir_Impl( aUserPath, aUINames );
}


bool SfxDocTplService_Impl::ReplaceUINamesForTemplateDir_Impl( const OUString& aUserPath,
                                                                  const OUString& aDefaultFsysGroupName,
                                                                  const OUString& aOldGroupName,
                                                                  const OUString& aNewGroupName )
{
    std::vector< beans::StringPair > aUINames = ReadUINamesForTemplateDir_Impl( aUserPath );
    sal_Int32 nLen = aUINames.size();

    bool bChanged = false;
    for ( sal_Int32 nInd = 0; nInd < nLen; nInd++ )
        if ( aUINames[nInd].Second.equals( aOldGroupName ) )
        {
            aUINames[nInd].Second = aNewGroupName;
            bChanged = true;
        }

    if ( !bChanged )
    {
        aUINames.resize( ++nLen );
        aUINames[nLen-1].First = aDefaultFsysGroupName;
        aUINames[nLen-1].Second = aNewGroupName;
    }
    return WriteUINamesForTemplateDir_Impl( aUserPath, aUINames );
}


void SfxDocTplService_Impl::RemoveUINamesForTemplateDir_Impl( const OUString& aUserPath,
                                                                  const OUString& aGroupName )
{
    std::vector< beans::StringPair > aUINames = ReadUINamesForTemplateDir_Impl( aUserPath );
    sal_Int32 nLen = aUINames.size();
    std::vector< beans::StringPair > aNewUINames( nLen );
    sal_Int32 nNewLen = 0;

    bool bChanged = false;
    for ( sal_Int32 nInd = 0; nInd < nLen; nInd++ )
        if ( aUINames[nInd].Second.equals( aGroupName ) )
            bChanged = true;
        else
        {
            nNewLen++;
            aNewUINames[nNewLen-1].First = aUINames[nInd].First;
            aNewUINames[nNewLen-1].Second = aUINames[nInd].Second;
        }

    aNewUINames.resize( nNewLen );

    !bChanged || WriteUINamesForTemplateDir_Impl( aUserPath, aNewUINames );
}


bool SfxDocTplService_Impl::WriteUINamesForTemplateDir_Impl( const OUString& aUserPath,
                                                             const std::vector< beans::StringPair >& aUINames )
{
    bool bResult = false;
    try {
        uno::Reference< beans::XPropertySet > xTempFile(
                io::TempFile::create(mxContext),
                uno::UNO_QUERY_THROW );

        OUString aTempURL;
        uno::Any aUrl = xTempFile->getPropertyValue("Uri");
        aUrl >>= aTempURL;

        uno::Reference< io::XStream > xStream( xTempFile, uno::UNO_QUERY_THROW );
        uno::Reference< io::XOutputStream > xOutStream = xStream->getOutputStream();
        if ( !xOutStream.is() )
            throw uno::RuntimeException();

        DocTemplLocaleHelper::WriteGroupLocalizationSequence( xOutStream, aUINames, mxContext);
        try {
            // the SAX writer might close the stream
            xOutStream->closeOutput();
        } catch( uno::Exception& )
        {}

        Content aTargetContent( aUserPath, maCmdEnv, comphelper::getProcessComponentContext() );
        Content aSourceContent( aTempURL, maCmdEnv, comphelper::getProcessComponentContext() );
        aTargetContent.transferContent( aSourceContent,
                                        InsertOperation::Copy,
                                        "groupuinames.xml",
                                        ucb::NameClash::OVERWRITE,
                                        "text/xml" );
        bResult = true;
    }
    catch ( uno::Exception& )
    {
    }

    return bResult;
}


OUString SfxDocTplService_Impl::CreateNewGroupFsys( const OUString& rGroupName, Content& aGroup )
{
    OUString aResultURL;

    if ( maTemplateDirs.getLength() )
    {
        OUString aTargetPath = maTemplateDirs[ maTemplateDirs.getLength() - 1 ];

        // create a new folder with the given name
        Content aNewFolder;
        OUString aNewFolderName;

        // the Fsys name instead of GroupName should be used, the groupuinames must be added also
        if ( !CreateNewUniqueFolderWithPrefix( aTargetPath,
                                                rGroupName,
                                                aNewFolderName,
                                                aResultURL,
                                                aNewFolder )
          && !CreateNewUniqueFolderWithPrefix( aTargetPath,
                                                "UserGroup",
                                                aNewFolderName,
                                                aResultURL,
                                                aNewFolder ) )

            return OUString();

        if ( !UpdateUINamesForTemplateDir_Impl( aTargetPath, rGroupName, aNewFolderName ) )
        {
            // we could not create the groupuinames for the folder, so we delete the group in the
            // the folder and return
            removeContent( aNewFolder );
            return OUString();
        }

        // Now set the target url for this group and we are done
        OUString aPropName( TARGET_DIR_URL  );
        Any aValue = makeAny( aResultURL );

        if ( ! setProperty( aGroup, aPropName, aValue ) )
        {
            removeContent( aNewFolder );
            return OUString();
        }
    }

    return aResultURL;
}


bool SfxDocTplService_Impl::addGroup( const OUString& rGroupName )
{
    ::osl::MutexGuard aGuard( maMutex );

    // Check, whether or not there is a group with this name
    Content      aNewGroup;
    OUString        aNewGroupURL;
    INetURLObject   aNewGroupObj( maRootURL );

    aNewGroupObj.insertName( rGroupName, false,
                      INetURLObject::LAST_SEGMENT,
                      INetURLObject::EncodeMechanism::All );

    aNewGroupURL = aNewGroupObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );

    if ( Content::create( aNewGroupURL, maCmdEnv, comphelper::getProcessComponentContext(), aNewGroup ) ||
         ! createFolder( aNewGroupURL, false, false, aNewGroup ) )
    {
        // if there already was a group with this name or the new group
        // could not be created, we return here
        return false;
    }

    // Get the user template path entry ( new group will always
    // be added in the user template path )
    sal_Int32   nIndex;
    OUString    aUserPath;

    nIndex = maTemplateDirs.getLength();
    if ( nIndex )
        nIndex--;
    else
        return false;   // We don't know where to add the group

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
                                            "UserGroup",
                                            aNewFolderName,
                                            aNewFolderURL,
                                            aNewFolder ) )
    {
        // we could not create the folder, so we delete the group in the
        // hierarchy and return
        removeContent( aNewGroup );
        return false;
    }

    if ( !UpdateUINamesForTemplateDir_Impl( aUserPath, rGroupName, aNewFolderName ) )
    {
        // we could not create the groupuinames for the folder, so we delete the group in the
        // hierarchy, the folder and return
        removeContent( aNewGroup );
        removeContent( aNewFolder );
        return false;
    }

    // Now set the target url for this group and we are done
    OUString aPropName( TARGET_DIR_URL  );
    Any aValue = makeAny( aNewFolderURL );

    if ( ! setProperty( aNewGroup, aPropName, aValue ) )
    {
        removeContent( aNewGroup );
        removeContent( aNewFolder );
        return false;
    }

    return true;
}


bool SfxDocTplService_Impl::removeGroup( const OUString& rGroupName )
{
    // remove all the elements that have the prefix aTargetURL
    // if the group does not have other elements remove it

    ::osl::MutexGuard aGuard( maMutex );

    bool bResult = false;

    // create the group url
    INetURLObject aGroupObj( maRootURL );
    aGroupObj.insertName( rGroupName, false,
                      INetURLObject::LAST_SEGMENT,
                      INetURLObject::EncodeMechanism::All );

    // Get the target url
    Content  aGroup;
    OUString    aGroupURL = aGroupObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );

    if ( Content::create( aGroupURL, maCmdEnv, comphelper::getProcessComponentContext(), aGroup ) )
    {
        OUString    aPropName( TARGET_DIR_URL  );
        Any      aValue;

        OUString    aGroupTargetURL;
        if ( getProperty( aGroup, aPropName, aValue ) )
            aValue >>= aGroupTargetURL;

        if ( aGroupTargetURL.isEmpty() )
            return false; // nothing is allowed to be removed

        if ( !maTemplateDirs.getLength() )
            return false;

        // check that the fs location is in writable folder and this is not a "My templates" folder
        INetURLObject aGroupParentFolder( aGroupTargetURL );
        if (!aGroupParentFolder.removeSegment())
            return false;

        OUString aGeneralTempPath = findParentTemplateDir(
            aGroupParentFolder.GetMainURL(INetURLObject::DecodeMechanism::NONE));

        if (aGeneralTempPath.isEmpty())
            return false;

        // now get the content of the Group
        uno::Reference< XResultSet > xResultSet;
        Sequence< OUString > aProps { TARGET_URL };

        try
        {
            ResultSetInclude eInclude = INCLUDE_DOCUMENTS_ONLY;
            xResultSet = aGroup.createCursor( aProps, eInclude );

            if ( xResultSet.is() )
            {
                bool bHasNonRemovable = false;
                bool bHasShared = false;

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
                            bHasNonRemovable = true;
                    }
                    else
                        bHasShared = true;
                }

                if ( !bHasNonRemovable && !bHasShared )
                {
                    if ( removeContent( aGroupTargetURL )
                      || !::utl::UCBContentHelper::Exists( aGroupTargetURL ) )
                    {
                        removeContent( aGroupURL );
                        RemoveUINamesForTemplateDir_Impl( aGeneralTempPath, rGroupName );
                        bResult = true; // the operation is successful only if the whole group is removed
                    }
                }
                else if ( !bHasNonRemovable )
                {
                    if ( removeContent( aGroupTargetURL )
                      || !::utl::UCBContentHelper::Exists( aGroupTargetURL ) )
                    {
                        RemoveUINamesForTemplateDir_Impl( aGeneralTempPath, rGroupName );
                        setProperty( aGroup, aPropName, uno::makeAny( OUString() ) );
                    }
                }
            }
        }
        catch ( Exception& ) {}
    }

    return bResult;
}


bool SfxDocTplService_Impl::renameGroup( const OUString& rOldName,
                                             const OUString& rNewName )
{
    ::osl::MutexGuard aGuard( maMutex );

    // create the group url
    Content         aGroup;
    INetURLObject   aGroupObj( maRootURL );
                    aGroupObj.insertName( rNewName, false,
                                          INetURLObject::LAST_SEGMENT,
                                          INetURLObject::EncodeMechanism::All );
    OUString        aGroupURL = aGroupObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );

    // Check, if there is a group with the new name, return false
    // if there is one.
    if ( Content::create( aGroupURL, maCmdEnv, comphelper::getProcessComponentContext(), aGroup ) )
        return false;

    aGroupObj.removeSegment();
    aGroupObj.insertName( rOldName, false,
                      INetURLObject::LAST_SEGMENT,
                      INetURLObject::EncodeMechanism::All );
    aGroupURL = aGroupObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );

    // When there is no group with the old name, we can't rename it
    if ( ! Content::create( aGroupURL, maCmdEnv, comphelper::getProcessComponentContext(), aGroup ) )
        return false;

    OUString aGroupTargetURL;
    // there is no need to check whether target dir url is in target path, since if the target path is changed
    // the target dir url should be already generated new
    OUString    aPropName( TARGET_DIR_URL  );
    Any      aValue;
    if ( getProperty( aGroup, aPropName, aValue ) )
        aValue >>= aGroupTargetURL;

    if ( aGroupTargetURL.isEmpty() )
        return false;

    if ( !maTemplateDirs.getLength() )
        return false;

    // check that the fs location is in writable folder and this is not a "My templates" folder
    INetURLObject aGroupParentFolder( aGroupTargetURL );
    if (!aGroupParentFolder.removeSegment() ||
        isInternalTemplateDir(aGroupParentFolder.GetMainURL(INetURLObject::DecodeMechanism::NONE)))
    {
        return false;
    }

    // check that the group can be renamed ( all the contents must be in target location )
    bool bCanBeRenamed = false;
       try
       {
        uno::Reference< XResultSet > xResultSet;
        Sequence< OUString > aProps { TARGET_URL };
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

            bCanBeRenamed = true;
        }
    }
    catch ( Exception& ) {}

    if ( bCanBeRenamed )
    {
        INetURLObject aGroupTargetObj( aGroupTargetURL );
        OUString aFsysName = aGroupTargetObj.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::WithCharset );

        if ( aGroupTargetObj.removeSegment()
          && ReplaceUINamesForTemplateDir_Impl( aGroupTargetObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ),
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

    return false;
}


bool SfxDocTplService_Impl::storeTemplate( const OUString& rGroupName,
                                               const OUString& rTemplateName,
                                               const uno::Reference< frame::XStorable >& rStorable )
{
    ::osl::MutexGuard aGuard( maMutex );

    // Check, whether or not there is a group with this name
    // Return false, if there is no group with the given name
    Content         aGroup, aTemplate, aTargetGroup, aTemplateToRemove;
    OUString        aGroupURL, aTemplateURL, aTemplateToRemoveTargetURL;
    INetURLObject   aGroupObj( maRootURL );
    bool        bRemoveOldTemplateContent = false;
    OUString sDocServiceName;

    aGroupObj.insertName( rGroupName, false,
                      INetURLObject::LAST_SEGMENT,
                      INetURLObject::EncodeMechanism::All );
    aGroupURL = aGroupObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );

    if ( ! Content::create( aGroupURL, maCmdEnv, comphelper::getProcessComponentContext(), aGroup ) )
        return false;

    OUString aGroupTargetURL;
    OUString aPropName( TARGET_DIR_URL  );
    Any      aValue;
    if ( getProperty( aGroup, aPropName, aValue ) )
        aValue >>= aGroupTargetURL;


    // Check, if there's a template with the given name in this group
    // the target template should be overwritten if it is imported by user
    // in case the template is installed by office installation of by an add-in
    // it can not be replaced
    aGroupObj.insertName( rTemplateName, false,
                      INetURLObject::LAST_SEGMENT,
                      INetURLObject::EncodeMechanism::All );
    aTemplateURL = aGroupObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );

    if ( Content::create( aTemplateURL, maCmdEnv, comphelper::getProcessComponentContext(), aTemplateToRemove ) )
    {
        OUString    aTargetTemplPropName( TARGET_URL  );

        bRemoveOldTemplateContent = true;
        if ( getProperty( aTemplateToRemove, aTargetTemplPropName, aValue ) )
            aValue >>= aTemplateToRemoveTargetURL;

        if ( aGroupTargetURL.isEmpty() || !maTemplateDirs.getLength()
          || (!aTemplateToRemoveTargetURL.isEmpty() && isInternalTemplateDir(aTemplateToRemoveTargetURL)) )
            return false; // it is not allowed to remove the template
    }

    try
    {
        uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();

        // get document service name
        uno::Reference< frame::XModuleManager2 > xModuleManager( frame::ModuleManager::create(xContext) );
        sDocServiceName = xModuleManager->identify( uno::Reference< uno::XInterface >( rStorable, uno::UNO_QUERY ) );
        if ( sDocServiceName.isEmpty() )
            throw uno::RuntimeException();

        // get the actual filter name
        OUString aFilterName;

        uno::Reference< lang::XMultiServiceFactory > xConfigProvider =
                configuration::theDefaultProvider::get( xContext );

        uno::Sequence< uno::Any > aArgs( 1 );
        beans::PropertyValue aPathProp;
        aPathProp.Name = "nodepath";
        aPathProp.Value <<= OUString( "/org.openoffice.Setup/Office/Factories/"  );
        aArgs[0] <<= aPathProp;

        uno::Reference< container::XNameAccess > xSOFConfig(
            xConfigProvider->createInstanceWithArguments(
                                    "com.sun.star.configuration.ConfigurationAccess",
                                    aArgs ),
            uno::UNO_QUERY_THROW );

        uno::Reference< container::XNameAccess > xApplConfig;
        xSOFConfig->getByName( sDocServiceName ) >>= xApplConfig;
        if ( !xApplConfig.is() )
            throw uno::RuntimeException();

        xApplConfig->getByName("ooSetupFactoryActualTemplateFilter") >>= aFilterName;
        if ( aFilterName.isEmpty() )
            throw uno::RuntimeException();

        // find the related type name
        OUString aTypeName;
        uno::Reference< container::XNameAccess > xFilterFactory(
            mxContext->getServiceManager()->createInstanceWithContext("com.sun.star.document.FilterFactory", mxContext),
            uno::UNO_QUERY_THROW );

        uno::Sequence< beans::PropertyValue > aFilterData;
        xFilterFactory->getByName( aFilterName ) >>= aFilterData;
        for ( sal_Int32 nInd = 0; nInd < aFilterData.getLength(); nInd++ )
            if ( aFilterData[nInd].Name == "Type" )
                aFilterData[nInd].Value >>= aTypeName;

        if ( aTypeName.isEmpty() )
            throw uno::RuntimeException();

        // find the mediatype and extension
        uno::Reference< container::XNameAccess > xTypeDetection;

        xTypeDetection =
            mxType.is() ?
                uno::Reference< container::XNameAccess >( mxType, uno::UNO_QUERY_THROW ) :
                uno::Reference< container::XNameAccess >(
                    mxContext->getServiceManager()->createInstanceWithContext("com.sun.star.document.TypeDetection", mxContext),
                    uno::UNO_QUERY_THROW );

        SequenceAsHashMap aTypeProps( xTypeDetection->getByName( aTypeName ) );
        uno::Sequence< OUString > aAllExt =
            aTypeProps.getUnpackedValueOrDefault("Extensions", Sequence< OUString >() );
        if ( !aAllExt.getLength() )
            throw uno::RuntimeException();

        OUString aMediaType = aTypeProps.getUnpackedValueOrDefault("MediaType", OUString() );
        OUString aExt = aAllExt[0];

        if ( aMediaType.isEmpty() || aExt.isEmpty() )
            throw uno::RuntimeException();

        // construct destination url
        if ( aGroupTargetURL.isEmpty() )
        {
            aGroupTargetURL = CreateNewGroupFsys( rGroupName, aGroup );

            if ( aGroupTargetURL.isEmpty() )
                throw uno::RuntimeException();
        }

        OUString aNewTemplateTargetURL = CreateNewUniqueFileWithPrefix( aGroupTargetURL, rTemplateName, aExt );
        if ( aNewTemplateTargetURL.isEmpty() )
        {
            aNewTemplateTargetURL = CreateNewUniqueFileWithPrefix( aGroupTargetURL, "UserTemplate", aExt );

            if ( aNewTemplateTargetURL.isEmpty() )
                throw uno::RuntimeException();
        }

        // store template
        uno::Sequence< PropertyValue > aStoreArgs( 2 );
        aStoreArgs[0].Name = "FilterName";
        aStoreArgs[0].Value <<= aFilterName;
        aStoreArgs[1].Name = "DocumentTitle";
        aStoreArgs[1].Value <<= rTemplateName;

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
            OUString sStdTmplFile = SfxObjectFactory::GetStandardTemplate( sDocServiceName );
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
        return false;
    }
}


bool SfxDocTplService_Impl::addTemplate( const OUString& rGroupName,
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
                      INetURLObject::LAST_SEGMENT,
                      INetURLObject::EncodeMechanism::All );
    aGroupURL = aGroupObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );

    if ( ! Content::create( aGroupURL, maCmdEnv, comphelper::getProcessComponentContext(), aGroup ) )
        return false;

    // Check, if there's a template with the given name in this group
    // Return false, if there already is a template
    aGroupObj.insertName( rTemplateName, false,
                      INetURLObject::LAST_SEGMENT,
                      INetURLObject::EncodeMechanism::All );
    aTemplateURL = aGroupObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );

    if ( Content::create( aTemplateURL, maCmdEnv, comphelper::getProcessComponentContext(), aTemplate ) )
        return false;

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
            return false;
    }

    // Get the content type
    OUString aTitle, aType, aTargetURL2;

    bool bDocHasTitle = false;
    if( !getTitleFromURL( rSourceURL, aTitle, aType, bDocHasTitle ) )
        return false;

    INetURLObject   aSourceObj( rSourceURL );
    if ( rTemplateName.equals( aTitle ) )
    {
        // addTemplate will sometimes be called just to add an entry in the
        // hierarchy; the target URL and the source URL will be the same in
        // this scenario
        // TODO/LATER: get rid of this old hack

        INetURLObject   aTargetObj( aTargetURL );

        aTargetObj.insertName( rTemplateName, false,
                      INetURLObject::LAST_SEGMENT,
                      INetURLObject::EncodeMechanism::All );
        aTargetObj.setExtension( aSourceObj.getExtension() );

        aTargetURL2 = aTargetObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );

        if ( aTargetURL2 == rSourceURL )
            return addEntry( aGroup, rTemplateName, aTargetURL2, aType );
    }

    // copy the template into the new group (targeturl)

    INetURLObject aTmpURL( aSourceObj );
    aTmpURL.CutExtension();
    OUString aPattern = aTmpURL.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::WithCharset );

    OUString aNewTemplateTargetURL = CreateNewUniqueFileWithPrefix( aTargetURL, aPattern, aSourceObj.getExtension() );
    INetURLObject aNewTemplateTargetObj( aNewTemplateTargetURL );
    OUString aNewTemplateTargetName = aNewTemplateTargetObj.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::WithCharset );
    if ( aNewTemplateTargetURL.isEmpty() || aNewTemplateTargetName.isEmpty() )
        return false;

    // get access to source file
    Content aSourceContent;
    uno::Reference < ucb::XCommandEnvironment > xEnv;
    INetURLObject   aSourceURL( rSourceURL );
    if( ! Content::create( aSourceURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), xEnv, comphelper::getProcessComponentContext(), aSourceContent ) )
        return false;

    if( ! Content::create( aTargetURL, xEnv, comphelper::getProcessComponentContext(), aTargetGroup ) )
        return false;

    // transfer source file
    try
    {
        if( ! aTargetGroup.transferContent( aSourceContent,
                                                InsertOperation::Copy,
                                                aNewTemplateTargetName,
                                                NameClash::OVERWRITE,
                                                aType ) )
            return false;

        // allow to edit the added template
        Content aResultContent;
        if ( Content::create( aNewTemplateTargetURL, xEnv, comphelper::getProcessComponentContext(), aResultContent ) )
        {
            OUString aPropertyName( "IsReadOnly"  );
            uno::Any aProperty;
            bool bReadOnly = false;
            if ( getProperty( aResultContent, aPropertyName, aProperty ) && ( aProperty >>= bReadOnly ) && bReadOnly )
                setProperty( aResultContent, aPropertyName, uno::makeAny( false ) );
        }
    }
    catch ( ContentCreationException& )
    { return false; }
    catch ( Exception& )
    { return false; }


    // either the document has title and it is the same as requested, or we have to set it
    bool bCorrectTitle = ( bDocHasTitle && aTitle.equals( rTemplateName ) );
    if ( !bCorrectTitle )
    {
        if ( !bDocHasTitle )
        {
            INetURLObject aNewTmpObj( aNewTemplateTargetObj );
            aNewTmpObj.CutExtension();
            bCorrectTitle = ( aNewTmpObj.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::WithCharset ).equals( rTemplateName ) );
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
    return false;
}

bool SfxDocTplService_Impl::isInternalTemplateDir(const OUString& rURL) const
{
    const sal_Int32 nDirs = maInternalTemplateDirs.getLength();
    const OUString* pDirs = maInternalTemplateDirs.getConstArray();
    for (sal_Int32 i = 0; i < nDirs; ++i, ++pDirs)
    {
        if (::utl::UCBContentHelper::IsSubPath(*pDirs, rURL))
            return true;
    }
    return false;
}

OUString SfxDocTplService_Impl::findParentTemplateDir(const OUString& rURL) const
{
    const sal_Int32 nDirs = maTemplateDirs.getLength();
    const OUString* pDirs = maTemplateDirs.getConstArray();
    for (sal_Int32 i = 0; i < nDirs; ++i, ++pDirs)
    {
        if (::utl::UCBContentHelper::IsSubPath(*pDirs, rURL))
            return *pDirs;
    }
    return OUString();
}

bool SfxDocTplService_Impl::removeTemplate( const OUString& rGroupName,
                                                const OUString& rTemplateName )
{
    ::osl::MutexGuard aGuard( maMutex );

    // Check, whether or not there is a group with this name
    // Return false, if there is no group with the given name
    Content         aGroup, aTemplate;
    OUString        aGroupURL, aTemplateURL;
    INetURLObject   aGroupObj( maRootURL );

    aGroupObj.insertName( rGroupName, false,
                      INetURLObject::LAST_SEGMENT,
                      INetURLObject::EncodeMechanism::All );
    aGroupURL = aGroupObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );

    if ( ! Content::create( aGroupURL, maCmdEnv, comphelper::getProcessComponentContext(), aGroup ) )
        return false;

    // Check, if there's a template with the given name in this group
    // Return false, if there is no template
    aGroupObj.insertName( rTemplateName, false,
                      INetURLObject::LAST_SEGMENT,
                      INetURLObject::EncodeMechanism::All );
    aTemplateURL = aGroupObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );

    if ( !Content::create( aTemplateURL, maCmdEnv, comphelper::getProcessComponentContext(), aTemplate ) )
        return false;

    // get the target URL from the template
    OUString    aTargetURL;
    OUString    aPropName( TARGET_URL  );
    Any         aValue;

    if ( getProperty( aTemplate, aPropName, aValue ) )
        aValue >>= aTargetURL;

    // delete the target template
    if ( !aTargetURL.isEmpty() )
    {
        if (isInternalTemplateDir(aTargetURL))
            return false;

        removeContent( aTargetURL );
    }

    // delete the template entry
    return removeContent( aTemplate );
}


bool SfxDocTplService_Impl::renameTemplate( const OUString& rGroupName,
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
                      INetURLObject::LAST_SEGMENT,
                      INetURLObject::EncodeMechanism::All );
    aGroupURL = aGroupObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );

    if ( ! Content::create( aGroupURL, maCmdEnv, comphelper::getProcessComponentContext(), aGroup ) )
        return false;

    // Check, if there's a template with the new name in this group
    // Return false, if there is one
    aGroupObj.insertName( rNewName, false,
                      INetURLObject::LAST_SEGMENT,
                      INetURLObject::EncodeMechanism::All );
    aTemplateURL = aGroupObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );

    if ( Content::create( aTemplateURL, maCmdEnv, comphelper::getProcessComponentContext(), aTemplate ) )
        return false;

    // Check, if there's a template with the old name in this group
    // Return false, if there is no template
    aGroupObj.removeSegment();
    aGroupObj.insertName( rOldName, false,
                      INetURLObject::LAST_SEGMENT,
                      INetURLObject::EncodeMechanism::All );
    aTemplateURL = aGroupObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );

    if ( !Content::create( aTemplateURL, maCmdEnv, comphelper::getProcessComponentContext(), aTemplate ) )
        return false;

    OUString    aTemplateTargetURL;
    OUString    aTargetProp( TARGET_URL  );
    Any         aTargetValue;

    if ( getProperty( aTemplate, aTargetProp, aTargetValue ) )
        aTargetValue >>= aTemplateTargetURL;

    if ( !setTitleForURL( aTemplateTargetURL, rNewName ) )
        return false;

    // rename the template entry in the cache
    OUString    aTitleProp( TITLE  );
    Any         aTitleValue;
    aTitleValue <<= rNewName;

    return setProperty( aTemplate, aTitleProp, aTitleValue );
}


class SfxDocTplService: public ::cppu::WeakImplHelper< css::lang::XLocalizable, css::frame::XDocumentTemplates, css::lang::XServiceInfo >
{
    std::unique_ptr<SfxDocTplService_Impl>      pImp;

public:
    explicit SfxDocTplService( const css::uno::Reference < uno::XComponentContext >& xContext );

    virtual OUString SAL_CALL getImplementationName() override
    {
        return OUString("com.sun.star.comp.sfx2.DocumentTemplates");
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        css::uno::Sequence< OUString > aSeq { "com.sun.star.frame.DocumentTemplates" };
        return aSeq;
    }


    // --- XLocalizable ---
    void SAL_CALL                   setLocale( const css::lang::Locale & eLocale ) override;
    css::lang::Locale SAL_CALL              getLocale() override;

    // --- XDocumentTemplates ---
    css::uno::Reference< css::ucb::XContent > SAL_CALL  getContent() override;
    sal_Bool SAL_CALL               storeTemplate( const OUString& GroupName,
                                                   const OUString& TemplateName,
                                                   const css::uno::Reference< css::frame::XStorable >& Storable ) override;
    sal_Bool SAL_CALL               addTemplate( const OUString& GroupName,
                                                 const OUString& TemplateName,
                                                 const OUString& SourceURL ) override;
    sal_Bool SAL_CALL               removeTemplate( const OUString& GroupName,
                                                    const OUString& TemplateName ) override;
    sal_Bool SAL_CALL               renameTemplate( const OUString& GroupName,
                                                    const OUString& OldTemplateName,
                                                    const OUString& NewTemplateName ) override;
    sal_Bool SAL_CALL               addGroup( const OUString& GroupName ) override;
    sal_Bool SAL_CALL               removeGroup( const OUString& GroupName ) override;
    sal_Bool SAL_CALL               renameGroup( const OUString& OldGroupName,
                                                 const OUString& NewGroupName ) override;
    void SAL_CALL                   update() override;
};


SfxDocTplService::SfxDocTplService( const uno::Reference< XComponentContext >& xContext )
{
    pImp.reset( new SfxDocTplService_Impl(xContext) );
}



//--- XLocalizable ---


lang::Locale SAL_CALL SfxDocTplService::getLocale()
{
    return pImp->getLocale();
}


void SAL_CALL SfxDocTplService::setLocale( const lang::Locale & rLocale )
{
    pImp->setLocale( rLocale );
}


//--- XDocumentTemplates ---

uno::Reference< ucb::XContent > SAL_CALL SfxDocTplService::getContent()
{
    if ( pImp->init() )
        return pImp->getContent().get();
    else
        return nullptr;
}


sal_Bool SAL_CALL SfxDocTplService::storeTemplate( const OUString& GroupName,
                                                   const OUString& TemplateName,
                                                   const uno::Reference< frame::XStorable >& Storable )
{
    if ( pImp->init() )
        return pImp->storeTemplate( GroupName, TemplateName, Storable );
    else
        return false;
}


sal_Bool SAL_CALL SfxDocTplService::addTemplate( const OUString& rGroupName,
                                                 const OUString& rTemplateName,
                                                 const OUString& rSourceURL )
{
    if ( pImp->init() )
        return pImp->addTemplate( rGroupName, rTemplateName, rSourceURL );
    else
        return false;
}


sal_Bool SAL_CALL SfxDocTplService::removeTemplate( const OUString& rGroupName,
                                                    const OUString& rTemplateName )
{
    if ( pImp->init() )
        return pImp->removeTemplate( rGroupName, rTemplateName );
    else
        return false;
}


sal_Bool SAL_CALL SfxDocTplService::renameTemplate( const OUString& rGroupName,
                                                    const OUString& rOldName,
                                                    const OUString& rNewName )
{
    if ( rOldName == rNewName )
        return true;

    if ( pImp->init() )
        return pImp->renameTemplate( rGroupName, rOldName, rNewName );
    else
        return false;
}


sal_Bool SAL_CALL SfxDocTplService::addGroup( const OUString& rGroupName )
{
    if ( pImp->init() )
        return pImp->addGroup( rGroupName );
    else
        return false;
}


sal_Bool SAL_CALL SfxDocTplService::removeGroup( const OUString& rGroupName )
{
    if ( pImp->init() )
        return pImp->removeGroup( rGroupName );
    else
        return false;
}


sal_Bool SAL_CALL SfxDocTplService::renameGroup( const OUString& rOldName,
                                                 const OUString& rNewName )
{
    if ( rOldName == rNewName )
        return true;

    if ( pImp->init() )
        return pImp->renameGroup( rOldName, rNewName );
    else
        return false;
}


void SAL_CALL SfxDocTplService::update()
{
    if ( pImp->init() )
        pImp->update();
}

WaitWindow_Impl::WaitWindow_Impl() : WorkWindow(nullptr, WB_BORDER | WB_3DLOOK)
{
    Rectangle aRect = Rectangle(0, 0, 300, 30000);
    maText = SfxResId(RID_CNT_STR_WAITING).toString();
    maRect = GetTextRect(aRect, maText, mnTextStyle);
    aRect = maRect;
    aRect.Right() += 2 * X_OFFSET;
    aRect.Bottom() += 2 * Y_OFFSET;
    maRect.SetPos(Point(X_OFFSET, Y_OFFSET));
    SetOutputSizePixel(aRect.GetSize());

    Show();
    Update();
    Flush();
}


WaitWindow_Impl::~WaitWindow_Impl()
{
    disposeOnce();
}

void  WaitWindow_Impl::dispose()
{
    Hide();
    WorkWindow::dispose();
}


void WaitWindow_Impl::Paint(vcl::RenderContext& rRenderContext, const Rectangle& /*rRect*/)
{
    rRenderContext.DrawText(maRect, maText, mnTextStyle);
}

void SfxDocTplService_Impl::addHierGroup( GroupList_Impl& rList,
                                          const OUString& rTitle,
                                          const OUString& rOwnURL )
{
    // now get the content of the Group
    Content aContent;
    uno::Reference<XResultSet> xResultSet;
    Sequence<OUString> aProps(3);

    aProps[0] = TITLE;
    aProps[1] = TARGET_URL;
    aProps[2] = PROPERTY_TYPE;

    try
    {
        aContent = Content(rOwnURL, maCmdEnv, comphelper::getProcessComponentContext());
        ResultSetInclude eInclude = INCLUDE_DOCUMENTS_ONLY;
        xResultSet = aContent.createCursor( aProps, eInclude );
    }
    catch (ContentCreationException&)
    {
        SAL_WARN( "sfx.doc", "addHierGroup: ContentCreationException" );
    }
    catch (Exception&) {}

    if ( xResultSet.is() )
    {
        GroupData_Impl *pGroup = new GroupData_Impl( rTitle );
        pGroup->setHierarchy( true );
        pGroup->setHierarchyURL( rOwnURL );
        rList.push_back( pGroup );

        uno::Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY );
        uno::Reference< XRow > xRow( xResultSet, UNO_QUERY );

        try
        {
            while ( xResultSet->next() )
            {
                bool             bUpdateType = false;
                DocTemplates_EntryData_Impl  *pData;

                OUString aTitle( xRow->getString( 1 ) );
                OUString aTargetDir( xRow->getString( 2 ) );
                OUString aType( xRow->getString( 3 ) );
                OUString aHierURL = xContentAccess->queryContentIdentifierString();

                if ( aType.isEmpty() )
                {
                    OUString aTmpTitle;

                    bool bDocHasTitle = false;
                    if( !getTitleFromURL( aTargetDir, aTmpTitle, aType, bDocHasTitle ) )
                    {
                        SAL_WARN( "sfx.doc", "addHierGroup(): template of alien format" );
                        continue;
                    }

                    if ( !aType.isEmpty() )
                        bUpdateType = true;
                }

                pData = pGroup->addEntry( aTitle, aTargetDir, aType, aHierURL );
                pData->setUpdateType( bUpdateType );
            }
        }
        catch ( Exception& ) {}
    }
}


void SfxDocTplService_Impl::addFsysGroup( GroupList_Impl& rList,
                                          const OUString& rTitle,
                                          const OUString& rUITitle,
                                          const OUString& rOwnURL,
                                          bool bWriteableGroup )
{
    OUString aTitle;

    if ( rUITitle.isEmpty() )
    {
        // reserved FS names that should not be used
        if ( rTitle == "wizard" )
            return;
        else if ( rTitle == "internal" )
            return;

        aTitle = getLongName( rTitle );
    }
    else
        aTitle = rUITitle;

    if ( aTitle.isEmpty() )
        return;

    GroupData_Impl* pGroup = nullptr;
    for (GroupData_Impl* i : rList)
    {
        if ( i->getTitle() == aTitle )
        {
            pGroup = i;
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
    Sequence< OUString >    aProps { TITLE };

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

                if ( aChildTitle == "sfx.tlx" || aChildTitle == "groupuinames.xml" )
                    continue;

                bool bDocHasTitle = false;
                if( !getTitleFromURL( aTargetURL, aChildTitle, aType, bDocHasTitle ) )
                    continue;

                pGroup->addEntry( aChildTitle, aTargetURL, aType, aHierURL );
            }
        }
        catch ( Exception& ) {}
    }
}


void SfxDocTplService_Impl::createFromContent( GroupList_Impl& rList,
                                               Content &rContent,
                                               bool bHierarchy,
                                               bool bWriteableContent )
{
    OUString aTargetURL = rContent.get()->getIdentifier()->getContentIdentifier();

    // when scanning the file system, we have to add the 'standard' group, too
    if ( ! bHierarchy )
    {
        OUString aUIStdTitle = getLongName( STANDARD_FOLDER );
        addFsysGroup( rList, OUString(), aUIStdTitle, aTargetURL, bWriteableContent );
    }

    // search for predefined UI names
    INetURLObject aLayerObj( aTargetURL );

    // TODO/LATER: Use hashmap in future
    std::vector< beans::StringPair > aUINames;
    if ( !bHierarchy )
        aUINames = ReadUINamesForTemplateDir_Impl( aLayerObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );

    uno::Reference< XResultSet > xResultSet;
    Sequence< OUString > aProps { TITLE };

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
                    OUString aUITitle;
                    for (beans::StringPair & rUIName : aUINames)
                        if ( rUIName.First.equals( aTitle ) )
                        {
                            aUITitle = rUIName.Second;
                            break;
                        }

                    addFsysGroup( rList, aTitle, aUITitle, aTargetSubfolderURL, bWriteableContent );
                }
            }
        }
        catch ( Exception& ) {}
    }
}


void SfxDocTplService_Impl::removeFromHierarchy( DocTemplates_EntryData_Impl *pData )
{
    Content aTemplate;

    if ( Content::create( pData->getHierarchyURL(), maCmdEnv, comphelper::getProcessComponentContext(), aTemplate ) )
    {
        removeContent( aTemplate );
    }
}


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
                      INetURLObject::LAST_SEGMENT,
                      INetURLObject::EncodeMechanism::All );

    OUString aTemplateURL = aGroupObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );

    if ( Content::create( aTemplateURL, maCmdEnv, comphelper::getProcessComponentContext(), aTemplate ) )
        return;

    addEntry( aGroup, pData->getTitle(),
              pData->getTargetURL(),
              pData->getType() );
}


void SfxDocTplService_Impl::updateData( DocTemplates_EntryData_Impl *pData )
{
    Content aTemplate;

    if ( ! Content::create( pData->getHierarchyURL(), maCmdEnv, comphelper::getProcessComponentContext(), aTemplate ) )
        return;

    OUString aPropName;

    if ( pData->getUpdateType() )
    {
        aPropName =  PROPERTY_TYPE;
        setProperty( aTemplate, aPropName, makeAny( pData->getType() ) );
    }

    if ( pData->getUpdateLink() )
    {
        aPropName = TARGET_URL;
        setProperty( aTemplate, aPropName, makeAny( pData->getTargetURL() ) );
    }
}


void SfxDocTplService_Impl::addGroupToHierarchy( GroupData_Impl *pGroup )
{
    OUString aAdditionalProp( TARGET_DIR_URL  );
    Content aGroup;

    INetURLObject aNewGroupObj( maRootURL );
    aNewGroupObj.insertName( pGroup->getTitle(), false,
          INetURLObject::LAST_SEGMENT,
          INetURLObject::EncodeMechanism::All );

    OUString aNewGroupURL = aNewGroupObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );

    if ( createFolder( aNewGroupURL, false, false, aGroup ) )
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


void SfxDocTplService_Impl::removeFromHierarchy( GroupData_Impl *pGroup )
{
    Content aGroup;

    if ( Content::create( pGroup->getHierarchyURL(), maCmdEnv, comphelper::getProcessComponentContext(), aGroup ) )
    {
        removeContent( aGroup );
    }
}


GroupData_Impl::GroupData_Impl( const OUString& rTitle )
{
    maTitle = rTitle;
    mbInUse = false;
    mbInHierarchy = false;
}


GroupData_Impl::~GroupData_Impl()
{
    for (DocTemplates_EntryData_Impl* p : maEntries)
        delete p;
    maEntries.clear();
}


DocTemplates_EntryData_Impl* GroupData_Impl::addEntry( const OUString& rTitle,
                                          const OUString& rTargetURL,
                                          const OUString& rType,
                                          const OUString& rHierURL )
{
    DocTemplates_EntryData_Impl* pData = nullptr;
    bool EntryFound = false;

    for (DocTemplates_EntryData_Impl* p : maEntries)
    {
        pData = p;
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
            pData->setHierarchy( true );
        }
        maEntries.push_back( pData );
    }
    else
    {
        if ( !rHierURL.isEmpty() )
        {
            pData->setHierarchyURL( rHierURL );
            pData->setHierarchy( true );
        }

        if ( pData->getInHierarchy() )
            pData->setInUse();

        if ( rTargetURL != pData->getTargetURL() )
        {
            pData->setTargetURL( rTargetURL );
            pData->setUpdateLink( true );
        }
    }

    return pData;
}


DocTemplates_EntryData_Impl::DocTemplates_EntryData_Impl( const OUString& rTitle )
{
    maTitle         = rTitle;
    mbInUse         = false;
    mbInHierarchy   = false;
    mbUpdateType    = false;
    mbUpdateLink    = false;
}

}

// static
bool SfxURLRelocator_Impl::propertyCanContainOfficeDir(
                                        const OUString & rPropName )
{
    // Note: TargetURL is handled by UCB itself (because it is a property
    //       with a predefined semantic). Additional Core properties introduced
    //       be a client app must be handled by the client app itself, because
    //       the UCB does not know the semantics of those properties.
    return ( rPropName == TARGET_DIR_URL || rPropName == PROPERTY_DIRLIST );
}


SfxURLRelocator_Impl::SfxURLRelocator_Impl( const uno::Reference< XComponentContext > & xContext )
: mxContext( xContext )
{
}


SfxURLRelocator_Impl::~SfxURLRelocator_Impl()
{
}


void SfxURLRelocator_Impl::initOfficeInstDirs()
{
    if ( !mxOfficeInstDirs.is() )
    {
        osl::MutexGuard aGuard( maMutex );
        if ( !mxOfficeInstDirs.is() )
        {
            OSL_ENSURE( mxContext.is(), "No service manager!" );

            mxOfficeInstDirs = theOfficeInstallationDirectories::get(mxContext);
        }
    }
}


void SfxURLRelocator_Impl::implExpandURL( OUString& io_url )
{
    const INetURLObject aParser( io_url );
    if ( aParser.GetProtocol() != INetProtocol::VndSunStarExpand )
        return;

    io_url = aParser.GetURLPath( INetURLObject::DecodeMechanism::WithCharset );
    try
    {
        if ( !mxMacroExpander.is() )
        {
            mxMacroExpander.set( theMacroExpander::get(mxContext), UNO_QUERY_THROW );
        }
        io_url = mxMacroExpander->expandMacros( io_url );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}


void SfxURLRelocator_Impl::makeRelocatableURL( OUString & rURL )
{
    if ( !rURL.isEmpty() )
    {
        initOfficeInstDirs();
        implExpandURL( rURL );
        rURL = mxOfficeInstDirs->makeRelocatableURL( rURL );
    }
}


void SfxURLRelocator_Impl::makeAbsoluteURL( OUString & rURL )
{
    if ( !rURL.isEmpty() )
    {
        initOfficeInstDirs();
        implExpandURL( rURL );
        rURL = mxOfficeInstDirs->makeAbsoluteURL( rURL );
    }
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_sfx2_DocumentTemplates_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SfxDocTplService(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
