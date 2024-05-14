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

#include <rtl/uri.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ref.hxx>

#include <comphelper/diagnose_ex.hxx>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/ucb/IllegalIdentifierException.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/beans/IllegalTypeException.hpp>
#include <com/sun/star/io/XActiveDataStreamer.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <comphelper/fileurl.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <utility>
#include "filglob.hxx"
#include "filid.hxx"
#include "filrow.hxx"
#include "bc.hxx"
#include "prov.hxx"
#include "filerror.hxx"
#include "filinsreq.hxx"

using namespace fileaccess;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;

#if OSL_DEBUG_LEVEL > 0
#define THROW_WHERE SAL_WHERE
#else
#define THROW_WHERE ""
#endif

class fileaccess::PropertyListeners
{
    typedef comphelper::OInterfaceContainerHelper4<beans::XPropertiesChangeListener> ContainerHelper;
    std::unordered_map<OUString, ContainerHelper> m_aMap;

public:
    void disposeAndClear(std::unique_lock<std::mutex>& rGuard, const lang::EventObject& rEvt)
    {
        // create a copy, because do not fire event in a guarded section
        std::unordered_map<OUString, ContainerHelper> tempMap = std::move(m_aMap);
        for (auto& rPair : tempMap)
            rPair.second.disposeAndClear(rGuard, rEvt);
    }
    void addInterface(std::unique_lock<std::mutex>& rGuard, const OUString& rKey, const uno::Reference<beans::XPropertiesChangeListener>& rListener)
    {
        m_aMap[rKey].addInterface(rGuard, rListener);
    }
    void removeInterface(std::unique_lock<std::mutex>& rGuard, const OUString& rKey, const uno::Reference<beans::XPropertiesChangeListener>& rListener)
    {
        // search container with id rKey
        auto iter = m_aMap.find(rKey);
        // container found?
        if (iter != m_aMap.end())
            iter->second.removeInterface(rGuard, rListener);
    }
    std::vector< OUString > getContainedTypes(std::unique_lock<std::mutex>& rGuard) const
    {
        std::vector<OUString> aInterfaceTypes;
        aInterfaceTypes.reserve(m_aMap.size());
        for (const auto& rPair : m_aMap)
            // are interfaces added to this container?
            if (rPair.second.getLength(rGuard))
                // yes, put the type in the array
                aInterfaceTypes.push_back(rPair.first);
        return aInterfaceTypes;
    }
    comphelper::OInterfaceContainerHelper4<beans::XPropertiesChangeListener>* getContainer(std::unique_lock<std::mutex>& , const OUString& rKey)
    {
        auto iter = m_aMap.find(rKey);
        if (iter != m_aMap.end())
            return &iter->second;
        return nullptr;
    }
};


/****************************************************************************************/
/*                                                                                      */
/*                    BaseContent                                                       */
/*                                                                                      */
/****************************************************************************************/


// Private Constructor for just inserted Contents

BaseContent::BaseContent( TaskManager* pMyShell,
                          OUString parentName,
                          bool bFolder )
    : m_pMyShell( pMyShell ),
      m_aUncPath(std::move( parentName )),
      m_bFolder( bFolder ),
      m_nState( JustInserted )
{
    m_pMyShell->m_pProvider->acquire();
    // No registering, since we have no name
}


// Constructor for full featured Contents

BaseContent::BaseContent( TaskManager* pMyShell,
                          const Reference< XContentIdentifier >& xContentIdentifier,
                          OUString aUncPath )
    : m_pMyShell( pMyShell ),
      m_xContentIdentifier( xContentIdentifier ),
      m_aUncPath(std::move( aUncPath )),
      m_bFolder( false ),
      m_nState( FullFeatured )
{
    m_pMyShell->m_pProvider->acquire();
    m_pMyShell->registerNotifier( m_aUncPath,this );
    m_pMyShell->insertDefaultProperties( m_aUncPath );
}


BaseContent::~BaseContent( )
{
    if( ( m_nState & FullFeatured ) || ( m_nState & Deleted ) )
    {
        m_pMyShell->deregisterNotifier( m_aUncPath,this );
    }
    m_pMyShell->m_pProvider->release();
}


// XComponent


void SAL_CALL
BaseContent::addEventListener( const Reference< lang::XEventListener >& Listener )
{
    std::unique_lock aGuard( m_aMutex );

    m_aDisposeEventListeners.addInterface( aGuard, Listener );
}


void SAL_CALL
BaseContent::removeEventListener( const Reference< lang::XEventListener >& Listener )
{
    std::unique_lock aGuard( m_aMutex );

    m_aDisposeEventListeners.removeInterface( aGuard, Listener );
}


void SAL_CALL
BaseContent::dispose()
{
    lang::EventObject aEvt;
    aEvt.Source = static_cast< XContent* >( this );

    std::unique_lock aGuard( m_aMutex );

    std::unique_ptr<PropertyListeners> pPropertyListener = std::move(m_pPropertyListener);

    m_aDisposeEventListeners.disposeAndClear( aGuard, aEvt );
    m_aContentEventListeners.disposeAndClear( aGuard, aEvt );

    if( pPropertyListener )
        pPropertyListener->disposeAndClear( aGuard, aEvt );

    m_aPropertySetInfoChangeListeners.disposeAndClear( aGuard, aEvt );
}

//  XServiceInfo
OUString SAL_CALL
BaseContent::getImplementationName()
{
    return u"com.sun.star.comp.ucb.FileContent"_ustr;
}

sal_Bool SAL_CALL
BaseContent::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

Sequence< OUString > SAL_CALL
BaseContent::getSupportedServiceNames()
{
    Sequence<OUString> ret { u"com.sun.star.ucb.FileContent"_ustr };
    return ret;
}

//  XCommandProcessor


sal_Int32 SAL_CALL
BaseContent::createCommandIdentifier()
{
    return m_pMyShell->getCommandId();
}


void SAL_CALL
BaseContent::abort( sal_Int32 /*CommandId*/ )
{
}


Any SAL_CALL
BaseContent::execute( const Command& aCommand,
                      sal_Int32 CommandId,
                      const Reference< XCommandEnvironment >& Environment )
{
    if( ! CommandId )
        // A Command with commandid zero cannot be aborted
        CommandId = createCommandIdentifier();

    m_pMyShell->startTask( CommandId,
                           Environment );

    Any aAny;

    if (aCommand.Name == "getPropertySetInfo")  // No exceptions
    {
        aAny <<= getPropertySetInfo();
    }
    else if (aCommand.Name == "getCommandInfo")  // no exceptions
    {
        aAny <<= getCommandInfo();
    }
    else if ( aCommand.Name == "setPropertyValues" )
    {
        Sequence< beans::PropertyValue > sPropertyValues;

        if( ! ( aCommand.Argument >>= sPropertyValues ) )
            m_pMyShell->installError( CommandId,
                                      TASKHANDLING_WRONG_SETPROPERTYVALUES_ARGUMENT );
        else
            aAny <<= setPropertyValues( CommandId,sPropertyValues );  // calls endTask by itself
    }
    else if ( aCommand.Name == "getPropertyValues" )
    {
        Sequence< beans::Property > ListOfRequestedProperties;

        if( ! ( aCommand.Argument >>= ListOfRequestedProperties ) )
            m_pMyShell->installError( CommandId,
                                      TASKHANDLING_WRONG_GETPROPERTYVALUES_ARGUMENT );
        else
            aAny <<= getPropertyValues( CommandId,
                                        ListOfRequestedProperties );
    }
    else if ( aCommand.Name == "open" )
    {
        OpenCommandArgument2 aOpenArgument;
        if( ! ( aCommand.Argument >>= aOpenArgument ) )
            m_pMyShell->installError( CommandId,
                                      TASKHANDLING_WRONG_OPEN_ARGUMENT );
        else
        {
            Reference< XDynamicResultSet > result = open( CommandId,aOpenArgument );
            if( result.is() )
                aAny <<= result;
        }
    }
    else if ( aCommand.Name == "delete" )
    {
        if( ! aCommand.Argument.has< bool >() )
            m_pMyShell->installError( CommandId,
                                      TASKHANDLING_WRONG_DELETE_ARGUMENT );
        else
            deleteContent( CommandId );
    }
    else if ( aCommand.Name == "transfer" )
    {
        TransferInfo aTransferInfo;
        if( ! ( aCommand.Argument >>= aTransferInfo ) )
            m_pMyShell->installError( CommandId,
                                      TASKHANDLING_WRONG_TRANSFER_ARGUMENT );
        else
            transfer( CommandId, aTransferInfo );
    }
    else if ( aCommand.Name == "insert" )
    {
        InsertCommandArgument aInsertArgument;
        if( ! ( aCommand.Argument >>= aInsertArgument ) )
            m_pMyShell->installError( CommandId,
                                      TASKHANDLING_WRONG_INSERT_ARGUMENT );
        else
            insert( CommandId,aInsertArgument );
    }
    else if ( aCommand.Name == "getCasePreservingURL" )
    {
        Reference< sdbc::XRow > xRow = getPropertyValues( CommandId, { { u"CasePreservingURL"_ustr, -1, cppu::UnoType<sal_Bool>::get(), 0 } });
        OUString CasePreservingURL = xRow->getString(1);
        if(!xRow->wasNull())
            aAny <<= CasePreservingURL;
    }
    else if ( aCommand.Name == "createNewContent" )
    {
        ucb::ContentInfo aArg;
        if ( !( aCommand.Argument >>= aArg ) )
            m_pMyShell->installError( CommandId,
                                      TASKHANDLING_WRONG_CREATENEWCONTENT_ARGUMENT );
        else
            aAny <<= createNewContent( aArg );
    }
    else
        m_pMyShell->installError( CommandId,
                                  TASKHANDLER_UNSUPPORTED_COMMAND );


    // This is the only function allowed to throw an exception
    endTask( CommandId );

    return aAny;
}


void SAL_CALL
BaseContent::addPropertiesChangeListener(
    const Sequence< OUString >& PropertyNames,
    const Reference< beans::XPropertiesChangeListener >& Listener )
{
    if( ! Listener.is() )
        return;

    std::unique_lock aGuard( m_aMutex );

    if( ! m_pPropertyListener )
        m_pPropertyListener.reset( new PropertyListeners );


    if( !PropertyNames.hasElements() )
        m_pPropertyListener->addInterface( aGuard, OUString(),Listener );
    else
    {
        Reference< beans::XPropertySetInfo > xProp = m_pMyShell->info_p( m_aUncPath );
        for( const auto& rName : PropertyNames )
            if( xProp->hasPropertyByName( rName ) )
                m_pPropertyListener->addInterface( aGuard, rName,Listener );
    }
}


void SAL_CALL
BaseContent::removePropertiesChangeListener( const Sequence< OUString >& PropertyNames,
                                             const Reference< beans::XPropertiesChangeListener >& Listener )
{
    if( ! Listener.is() )
        return;

    std::unique_lock aGuard( m_aMutex );

    if( ! m_pPropertyListener )
        return;

    for( const auto& rName : PropertyNames )
        m_pPropertyListener->removeInterface( aGuard, rName,Listener );

    m_pPropertyListener->removeInterface( aGuard, OUString(), Listener );
}


// XContent


Reference< ucb::XContentIdentifier > SAL_CALL
BaseContent::getIdentifier()
{
    return m_xContentIdentifier;
}


OUString SAL_CALL
BaseContent::getContentType()
{
    if( !( m_nState & Deleted ) )
    {
        if( m_nState & JustInserted )
        {
            if ( m_bFolder )
                return TaskManager::FolderContentType;
            else
                return TaskManager::FileContentType;
        }
        else
        {
            try
            {
                // Who am I ?
                Reference< sdbc::XRow > xRow = getPropertyValues( -1, { { u"IsDocument"_ustr, -1, cppu::UnoType<sal_Bool>::get(), 0 } });
                bool IsDocument = xRow->getBoolean( 1 );

                if ( !xRow->wasNull() )
                {
                    if ( IsDocument )
                        return TaskManager::FileContentType;
                    else
                        return TaskManager::FolderContentType;
                }
                else
                {
                    OSL_FAIL( "BaseContent::getContentType - Property value was null!" );
                }
            }
            catch (const sdbc::SQLException&)
            {
                TOOLS_WARN_EXCEPTION("ucb.ucp.file", "");
            }
        }
    }

    return OUString();
}


void SAL_CALL
BaseContent::addContentEventListener(
    const Reference< XContentEventListener >& Listener )
{
    std::unique_lock aGuard( m_aMutex );

    m_aContentEventListeners.addInterface( aGuard, Listener );
}


void SAL_CALL
BaseContent::removeContentEventListener(
    const Reference< XContentEventListener >& Listener )
{
    std::unique_lock aGuard( m_aMutex );

    m_aContentEventListeners.removeInterface( aGuard, Listener );
}


// XPropertyContainer


void SAL_CALL
BaseContent::addProperty(
    const OUString& Name,
    sal_Int16 Attributes,
    const Any& DefaultValue )
{
    if( ( m_nState & JustInserted ) || ( m_nState & Deleted ) || Name.isEmpty() )
    {
        throw lang::IllegalArgumentException( THROW_WHERE, uno::Reference< uno::XInterface >(), 0 );
    }

    m_pMyShell->associate( m_aUncPath,Name,DefaultValue,Attributes );
}


void SAL_CALL
BaseContent::removeProperty( const OUString& Name )
{

    if( m_nState & Deleted )
        throw beans::UnknownPropertyException( Name );

    m_pMyShell->deassociate( m_aUncPath, Name );
}


// XContentCreator


Sequence< ContentInfo > SAL_CALL
BaseContent::queryCreatableContentsInfo()
{
    return TaskManager::queryCreatableContentsInfo();
}


Reference< XContent > SAL_CALL
BaseContent::createNewContent( const ContentInfo& Info )
{
    // Check type.
    if ( Info.Type.isEmpty() )
        return Reference< XContent >();

    bool bFolder = Info.Type == TaskManager::FolderContentType;
    if ( !bFolder )
    {
        if ( Info.Type != TaskManager::FileContentType )
        {
            // Neither folder nor file to create!
            return Reference< XContent >();
        }
    }

    // Who am I ?
    bool IsDocument = false;

    try
    {
        Reference< sdbc::XRow > xRow = getPropertyValues( -1, { { u"IsDocument"_ustr, -1, cppu::UnoType<sal_Bool>::get(), 0 } });
        IsDocument = xRow->getBoolean( 1 );

        if ( xRow->wasNull() )
        {
            IsDocument = false;
//              OSL_FAIL( //                          "BaseContent::createNewContent - Property value was null!" );
//              return Reference< XContent >();
        }
    }
    catch (const sdbc::SQLException&)
    {
        TOOLS_WARN_EXCEPTION("ucb.ucp.file", "");
        return Reference< XContent >();
    }

    OUString dstUncPath;

    if( IsDocument )
    {
        // KSO: Why is a document a XContentCreator? This is quite unusual.
        dstUncPath = getParentName( m_aUncPath );
    }
    else
        dstUncPath = m_aUncPath;

    return new BaseContent( m_pMyShell, dstUncPath, bFolder );
}


// XPropertySetInfoChangeNotifier


void SAL_CALL
BaseContent::addPropertySetInfoChangeListener(
    const Reference< beans::XPropertySetInfoChangeListener >& Listener )
{
    std::unique_lock aGuard( m_aMutex );

    m_aPropertySetInfoChangeListeners.addInterface( aGuard, Listener );
}


void SAL_CALL
BaseContent::removePropertySetInfoChangeListener(
    const Reference< beans::XPropertySetInfoChangeListener >& Listener )
{
    std::unique_lock aGuard( m_aMutex );

    m_aPropertySetInfoChangeListeners.removeInterface( aGuard, Listener );
}


// XChild


Reference< XInterface > SAL_CALL
BaseContent::getParent()
{
    OUString ParentUnq = getParentName( m_aUncPath );
    OUString ParentUrl;


    bool err = fileaccess::TaskManager::getUrlFromUnq( ParentUnq, ParentUrl );
    if( err )
        return Reference< XInterface >( nullptr );

    rtl::Reference<FileContentIdentifier> Identifier = new FileContentIdentifier( ParentUnq );

    try
    {
        return Reference<XInterface>( m_pMyShell->m_pProvider->queryContent( Identifier ), UNO_QUERY );
    }
    catch (const IllegalIdentifierException&)
    {
        return Reference< XInterface >();
    }
}


void SAL_CALL
BaseContent::setParent(
    const Reference< XInterface >& )
{
    throw lang::NoSupportException( THROW_WHERE );
}


// Private Methods


Reference< XCommandInfo >
BaseContent::getCommandInfo()
{
    if( m_nState & Deleted )
        return Reference< XCommandInfo >();

    return m_pMyShell->info_c();
}


Reference< beans::XPropertySetInfo >
BaseContent::getPropertySetInfo()
{
    if( m_nState & Deleted )
        return Reference< beans::XPropertySetInfo >();

    return m_pMyShell->info_p( m_aUncPath );
}

Reference< sdbc::XRow >
BaseContent::getPropertyValues(
    sal_Int32 nMyCommandIdentifier,
    const Sequence< beans::Property >& PropertySet )
{
    sal_Int32 nProps = PropertySet.getLength();
    if ( !nProps )
        return Reference< sdbc::XRow >();

    if( m_nState & Deleted )
    {
        Sequence< Any > aValues( nProps );
        return Reference< sdbc::XRow >( new XRow_impl( m_pMyShell, aValues ) );
    }

    if( m_nState & JustInserted )
    {
        Sequence< Any > aValues( nProps );
        Any* pValues = aValues.getArray();

        const beans::Property* pProps = PropertySet.getConstArray();

        for ( sal_Int32 n = 0; n < nProps; ++n )
        {
            const beans::Property& rProp = pProps[ n ];
            Any& rValue = pValues[ n ];

            if ( rProp.Name == "ContentType" )
            {
                rValue <<= (m_bFolder ? TaskManager::FolderContentType
                    : TaskManager::FileContentType);
            }
            else if ( rProp.Name == "IsFolder" )
            {
                rValue <<= m_bFolder;
            }
            else if ( rProp.Name == "IsDocument" )
            {
                rValue <<= !m_bFolder;
            }
        }

        return Reference< sdbc::XRow >(
            new XRow_impl( m_pMyShell, aValues ) );
    }

    return m_pMyShell->getv( nMyCommandIdentifier,
                             m_aUncPath,
                             PropertySet );
}


Sequence< Any >
BaseContent::setPropertyValues(
    sal_Int32 nMyCommandIdentifier,
    const Sequence< beans::PropertyValue >& Values )
{
    if( m_nState & Deleted )
    {   //  To do
        return Sequence< Any >( Values.getLength() );
    }

    static constexpr OUString Title(u"Title"_ustr);

    // Special handling for files which have to be inserted
    if( m_nState & JustInserted )
    {
        for( const auto& rValue : Values )
        {
            if( rValue.Name == Title )
            {
                OUString NewTitle;
                if( rValue.Value >>= NewTitle )
                {
                    if ( m_nState & NameForInsertionSet )
                    {
                        // User wants to set another Title before "insert".
                        // m_aUncPath contains previous own URI.

                        sal_Int32 nLastSlash = m_aUncPath.lastIndexOf( '/' );
                        bool bTrailingSlash = false;
                        if ( nLastSlash == m_aUncPath.getLength() - 1 )
                        {
                            bTrailingSlash = true;
                            nLastSlash
                                = m_aUncPath.lastIndexOf( '/', nLastSlash );
                        }

                        OSL_ENSURE( nLastSlash != -1,
                                    "BaseContent::setPropertyValues: "
                                    "Invalid URL!" );

                        OUStringBuffer aBuf(
                            m_aUncPath.subView( 0, nLastSlash + 1 ) );

                        if ( !NewTitle.isEmpty() )
                        {
                            aBuf.append( NewTitle );
                            if ( bTrailingSlash )
                                aBuf.append( '/' );
                        }
                        else
                        {
                            m_nState &= ~NameForInsertionSet;
                        }

                        m_aUncPath = aBuf.makeStringAndClear();
                    }
                    else
                    {
                        if ( !NewTitle.isEmpty() )
                        {
                            // Initial Title before "insert".
                            // m_aUncPath contains parent's URI.

                            if( !m_aUncPath.endsWith( "/" ) )
                                m_aUncPath += "/";

                            m_aUncPath += rtl::Uri::encode( NewTitle,
                                                            rtl_UriCharClassPchar,
                                                            rtl_UriEncodeIgnoreEscapes,
                                                            RTL_TEXTENCODING_UTF8 );
                            m_nState |= NameForInsertionSet;
                        }
                    }
                }
            }
        }

        return Sequence< Any >( Values.getLength() );
    }
    else
    {
        Sequence< Any > ret = m_pMyShell->setv( m_aUncPath,  // Does not handle Title
                                                Values );
        auto retRange = asNonConstRange(ret);

        // Special handling Title: Setting Title is equivalent to a renaming of the underlying file
        for( sal_Int32 i = 0; i < Values.getLength(); ++i )
        {
            if( Values[i].Name != Title )
                continue;                  // handled by setv

            OUString NewTitle;
            if( !( Values[i].Value >>= NewTitle ) )
            {
                retRange[i] <<= beans::IllegalTypeException( THROW_WHERE );
                break;
            }
            else if( NewTitle.isEmpty() )
            {
                retRange[i] <<= lang::IllegalArgumentException( THROW_WHERE, uno::Reference< uno::XInterface >(), 0 );
                break;
            }


            OUString aDstName = getParentName( m_aUncPath );
            if( !aDstName.endsWith("/") )
                aDstName += "/";

            aDstName += rtl::Uri::encode( NewTitle,
                                          rtl_UriCharClassPchar,
                                          rtl_UriEncodeIgnoreEscapes,
                                          RTL_TEXTENCODING_UTF8 );

            m_pMyShell->move( nMyCommandIdentifier,     // move notifies the children also;
                              m_aUncPath,
                              aDstName,
                              NameClash::KEEP );

            try
            {
                endTask( nMyCommandIdentifier );
            }
            catch(const Exception& e)
            {
                retRange[i] <<= e;
            }

            // NameChanges come back through a ContentEvent
            break; // only handling Title
        } // end for

        return ret;
    }
}


Reference< XDynamicResultSet >
BaseContent::open(
    sal_Int32 nMyCommandIdentifier,
    const OpenCommandArgument2& aCommandArgument )
{
    Reference< XDynamicResultSet > retValue;

    if( m_nState & Deleted )
    {
        m_pMyShell->installError( nMyCommandIdentifier,
                                  TASKHANDLING_DELETED_STATE_IN_OPEN_COMMAND );
    }
    else if( m_nState & JustInserted )
    {
        m_pMyShell->installError( nMyCommandIdentifier,
                                  TASKHANDLING_INSERTED_STATE_IN_OPEN_COMMAND );
    }
    else
    {
        if( aCommandArgument.Mode == OpenMode::DOCUMENT ||
            aCommandArgument.Mode == OpenMode::DOCUMENT_SHARE_DENY_NONE )

        {
            Reference< io::XOutputStream > outputStream( aCommandArgument.Sink,UNO_QUERY );
            if( outputStream.is() )
            {
                m_pMyShell->page( nMyCommandIdentifier,
                                  m_aUncPath,
                                  outputStream );
            }

            bool bLock = ( aCommandArgument.Mode != OpenMode::DOCUMENT_SHARE_DENY_NONE );

            Reference< io::XActiveDataSink > activeDataSink( aCommandArgument.Sink,UNO_QUERY );
            if( activeDataSink.is() )
            {
                activeDataSink->setInputStream( m_pMyShell->open( nMyCommandIdentifier,
                                                                  m_aUncPath,
                                                                  bLock ) );
            }

            Reference< io::XActiveDataStreamer > activeDataStreamer( aCommandArgument.Sink,UNO_QUERY );
            if( activeDataStreamer.is() )
            {
                activeDataStreamer->setStream( m_pMyShell->open_rw( nMyCommandIdentifier,
                                                                    m_aUncPath,
                                                                    bLock ) );
            }
        }
        else if ( aCommandArgument.Mode == OpenMode::ALL        ||
                  aCommandArgument.Mode == OpenMode::FOLDERS    ||
                  aCommandArgument.Mode == OpenMode::DOCUMENTS )
        {
            retValue = m_pMyShell->ls( nMyCommandIdentifier,
                                       m_aUncPath,
                                       aCommandArgument.Mode,
                                       aCommandArgument.Properties,
                                       aCommandArgument.SortingInfo );
        }
//          else if(  aCommandArgument.Mode ==
//                    OpenMode::DOCUMENT_SHARE_DENY_NONE  ||
//                    aCommandArgument.Mode ==
//                    OpenMode::DOCUMENT_SHARE_DENY_WRITE )
//              m_pMyShell->installError( nMyCommandIdentifier,
//                                        TASKHANDLING_UNSUPPORTED_OPEN_MODE,
//                                        aCommandArgument.Mode);
        else
            m_pMyShell->installError( nMyCommandIdentifier,
                                      TASKHANDLING_UNSUPPORTED_OPEN_MODE,
                                      aCommandArgument.Mode);
    }

    return retValue;
}


void
BaseContent::deleteContent( sal_Int32 nMyCommandIdentifier )
{
    if( m_nState & Deleted )
        return;

    if( m_pMyShell->remove( nMyCommandIdentifier,m_aUncPath ) )
    {
        std::unique_lock aGuard( m_aMutex );
        m_nState |= Deleted;
    }
}


void
BaseContent::transfer( sal_Int32 nMyCommandIdentifier,
                       const TransferInfo& aTransferInfo )
{
    if( m_nState & Deleted )
        return;

    if( !comphelper::isFileUrl(aTransferInfo.SourceURL) )
    {
        m_pMyShell->installError( nMyCommandIdentifier,
                                  TASKHANDLING_TRANSFER_INVALIDSCHEME );
        return;
    }

    OUString srcUnc;
    if( fileaccess::TaskManager::getUnqFromUrl( aTransferInfo.SourceURL,srcUnc ) )
    {
        m_pMyShell->installError( nMyCommandIdentifier,
                                  TASKHANDLING_TRANSFER_INVALIDURL );
        return;
    }

    OUString srcUncPath = srcUnc;

    // Determine the new title !
    OUString NewTitle;
    if( !aTransferInfo.NewTitle.isEmpty() )
        NewTitle = rtl::Uri::encode( aTransferInfo.NewTitle,
                                     rtl_UriCharClassPchar,
                                     rtl_UriEncodeIgnoreEscapes,
                                     RTL_TEXTENCODING_UTF8 );
    else
        NewTitle = srcUncPath.copy( 1 + srcUncPath.lastIndexOf( '/' ) );

    // Is destination a document or a folder ?
    Reference< sdbc::XRow > xRow = getPropertyValues( nMyCommandIdentifier,{ { u"IsDocument"_ustr, -1, cppu::UnoType<sal_Bool>::get(), 0 } } );
    bool IsDocument = xRow->getBoolean( 1 );
    if( xRow->wasNull() )
    {   // Destination file type could not be determined
        m_pMyShell->installError( nMyCommandIdentifier,
                                  TASKHANDLING_TRANSFER_DESTFILETYPE );
        return;
    }

    OUString dstUncPath;
    if( IsDocument )
    {   // as sibling
        sal_Int32 lastSlash = m_aUncPath.lastIndexOf( '/' );
        dstUncPath = m_aUncPath.copy(0,lastSlash );
    }
    else
        // as child
        dstUncPath = m_aUncPath;

    dstUncPath += "/" + NewTitle;

    sal_Int32 NameClash = aTransferInfo.NameClash;

    if( aTransferInfo.MoveData )
        m_pMyShell->move( nMyCommandIdentifier,srcUncPath,dstUncPath,NameClash );
    else
        m_pMyShell->copy( nMyCommandIdentifier,srcUncPath,dstUncPath,NameClash );
}


void BaseContent::insert( sal_Int32 nMyCommandIdentifier,
                                   const InsertCommandArgument& aInsertArgument )
{
    if( m_nState & FullFeatured )
    {
        m_pMyShell->write( nMyCommandIdentifier,
                           m_aUncPath,
                           aInsertArgument.ReplaceExisting,
                           aInsertArgument.Data );
        return;
    }

    if( ! ( m_nState & JustInserted ) )
    {
        m_pMyShell->installError( nMyCommandIdentifier,
                                  TASKHANDLING_NOFRESHINSERT_IN_INSERT_COMMAND );
        return;
    }

    // Inserts the content, which has the flag m_bIsFresh

    if( ! (m_nState & NameForInsertionSet) )
    {
        m_pMyShell->installError( nMyCommandIdentifier,
                                  TASKHANDLING_NONAMESET_INSERT_COMMAND );
        return;
    }

    // Inserting a document or a file?
    bool bDocument = false;

    Reference< sdbc::XRow > xRow = getPropertyValues( -1, { { u"IsDocument"_ustr, -1, cppu::UnoType<sal_Bool>::get(), 0 } });

    bool contentTypeSet = true;  // is set to false, if contentType not set
    try
    {
        bDocument = xRow->getBoolean( 1 );
        if( xRow->wasNull() )
            contentTypeSet = false;

    }
    catch (const sdbc::SQLException&)
    {
        TOOLS_WARN_EXCEPTION("ucb.ucp.file", "");
        contentTypeSet = false;
    }

    if( ! contentTypeSet )
    {
        m_pMyShell->installError( nMyCommandIdentifier,
                                  TASKHANDLING_NOCONTENTTYPE_INSERT_COMMAND );
        return;
    }


    bool success = false;
    if( bDocument )
        success = m_pMyShell->mkfil( nMyCommandIdentifier,
                                     m_aUncPath,
                                     aInsertArgument.ReplaceExisting,
                                     aInsertArgument.Data );
    else
    {
        while( ! success )
        {
            success = m_pMyShell->mkdir( nMyCommandIdentifier,
                                         m_aUncPath,
                                         aInsertArgument.ReplaceExisting );
            if( success )
                break;

            XInteractionRequestImpl aRequestImpl(
                    rtl::Uri::decode(
                        OUString(getTitle(m_aUncPath)),
                        rtl_UriDecodeWithCharset,
                        RTL_TEXTENCODING_UTF8),
                    getXWeak(),
                    m_pMyShell,nMyCommandIdentifier);
            uno::Reference<task::XInteractionRequest> const& xReq(aRequestImpl.getRequest());

            m_pMyShell->handleTask( nMyCommandIdentifier, xReq );
            if (aRequestImpl.aborted() || aRequestImpl.newName().isEmpty())
                // means aborting
                break;

            // determine new uncpath
            m_pMyShell->clearError( nMyCommandIdentifier );
            m_aUncPath = getParentName( m_aUncPath );
            if( !m_aUncPath.endsWith( "/" ) )
                m_aUncPath += "/";

            m_aUncPath += rtl::Uri::encode( aRequestImpl.newName(),
                                            rtl_UriCharClassPchar,
                                            rtl_UriEncodeIgnoreEscapes,
                                            RTL_TEXTENCODING_UTF8 );
        }
    }

    if ( ! success )
        return;

    m_xContentIdentifier.set( new FileContentIdentifier( m_aUncPath ) );

    m_pMyShell->registerNotifier( m_aUncPath,this );
    m_pMyShell->insertDefaultProperties( m_aUncPath );

    std::unique_lock aGuard( m_aMutex );
    m_nState = FullFeatured;
}


void BaseContent::endTask( sal_Int32 CommandId )
{
    // This is the only function allowed to throw an exception
    m_pMyShell->endTask( CommandId,m_aUncPath,this );
}


std::optional<ContentEventNotifier>
BaseContent::cDEL()
{
    std::unique_lock aGuard( m_aMutex );

    m_nState |= Deleted;

    if( m_aContentEventListeners.getLength(aGuard) == 0 )
        return {};

    return ContentEventNotifier( m_pMyShell,
                                  this,
                                  m_xContentIdentifier,
                                  m_aContentEventListeners.getElements(aGuard) );
}


std::optional<ContentEventNotifier>
BaseContent::cEXC( const OUString& aNewName )
{
    std::unique_lock aGuard( m_aMutex );

    Reference< XContentIdentifier > xOldRef = m_xContentIdentifier;
    m_aUncPath = aNewName;
    m_xContentIdentifier = new FileContentIdentifier( aNewName );

    if( m_aContentEventListeners.getLength(aGuard) == 0 )
        return {};
    return ContentEventNotifier( m_pMyShell,
                                  this,
                                  m_xContentIdentifier,
                                  xOldRef,
                                  m_aContentEventListeners.getElements(aGuard) );
}


std::optional<ContentEventNotifier>
BaseContent::cCEL()
{
    std::unique_lock aGuard( m_aMutex );
    if( m_aContentEventListeners.getLength(aGuard) == 0 )
        return {};
    return ContentEventNotifier( m_pMyShell,
                                      this,
                                      m_xContentIdentifier,
                                      m_aContentEventListeners.getElements(aGuard) );
}

std::optional<PropertySetInfoChangeNotifier>
BaseContent::cPSL()
{
    std::unique_lock aGuard( m_aMutex );
    if( m_aPropertySetInfoChangeListeners.getLength(aGuard) == 0  )
        return {};
    return PropertySetInfoChangeNotifier( this, m_aPropertySetInfoChangeListeners.getElements(aGuard) );
}


std::optional<PropertyChangeNotifier>
BaseContent::cPCL()
{
    std::unique_lock aGuard( m_aMutex );

    if (!m_pPropertyListener)
        return {};

    const std::vector< OUString > seqNames = m_pPropertyListener->getContainedTypes(aGuard);
    if( seqNames.empty() )
        return {};

    ListenerMap listener;
    for( const auto& rName : seqNames )
    {
        comphelper::OInterfaceContainerHelper4<beans::XPropertiesChangeListener>* pContainer = m_pPropertyListener->getContainer(aGuard, rName);
        if (!pContainer)
            continue;
        listener[rName] = pContainer->getElements(aGuard);
    }

    return PropertyChangeNotifier( this, std::move(listener) );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
