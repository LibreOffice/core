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
#include <osl/file.hxx>

#include "osl/diagnose.h"
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/ucb/IllegalIdentifierException.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/beans/IllegalTypeException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/io/XActiveDataStreamer.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/ucb/NumberedSortingInfo.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/beans/PropertySetInfoChange.hpp>
#include <com/sun/star/ucb/ContentAction.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <comphelper/fileurl.hxx>
#include <cppuhelper/supportsservice.hxx>
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

typedef cppu::OMultiTypeInterfaceContainerHelperVar<OUString>
    PropertyListeners_impl;

class fileaccess::PropertyListeners
    : public PropertyListeners_impl
{
public:
    explicit PropertyListeners( ::osl::Mutex& aMutex )
        : PropertyListeners_impl( aMutex )
    {
    }
};


/****************************************************************************************/
/*                                                                                      */
/*                    BaseContent                                                       */
/*                                                                                      */
/****************************************************************************************/


// Private Constructor for just inserted Contents

BaseContent::BaseContent( TaskManager* pMyShell,
                          const OUString& parentName,
                          bool bFolder )
    : m_pMyShell( pMyShell ),
      m_xContentIdentifier( nullptr ),
      m_aUncPath( parentName ),
      m_bFolder( bFolder ),
      m_nState( JustInserted ),
      m_pDisposeEventListeners( nullptr ),
      m_pContentEventListeners( nullptr ),
      m_pPropertySetInfoChangeListeners( nullptr ),
      m_pPropertyListener( nullptr )
{
    m_pMyShell->m_pProvider->acquire();
    // No registering, since we have no name
}


// Constructor for full featured Contents

BaseContent::BaseContent( TaskManager* pMyShell,
                          const Reference< XContentIdentifier >& xContentIdentifier,
                          const OUString& aUncPath )
    : m_pMyShell( pMyShell ),
      m_xContentIdentifier( xContentIdentifier ),
      m_aUncPath( aUncPath ),
      m_bFolder( false ),
      m_nState( FullFeatured ),
      m_pDisposeEventListeners( nullptr ),
      m_pContentEventListeners( nullptr ),
      m_pPropertySetInfoChangeListeners( nullptr ),
      m_pPropertyListener( nullptr )
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

    delete m_pDisposeEventListeners;
    delete m_pContentEventListeners;
    delete m_pPropertyListener;
    delete m_pPropertySetInfoChangeListeners;
}


// XInterface


void SAL_CALL
BaseContent::acquire()
    throw()
{
    OWeakObject::acquire();
}


void SAL_CALL
BaseContent::release()
    throw()
{
    OWeakObject::release();
}


Any SAL_CALL
BaseContent::queryInterface( const Type& rType )
{
    Any aRet = cppu::queryInterface( rType,
                                     (static_cast< lang::XComponent* >(this)),
                                     (static_cast< lang::XTypeProvider* >(this)),
                                     (static_cast< lang::XServiceInfo* >(this)),
                                     (static_cast< XCommandProcessor* >(this)),
                                     (static_cast< container::XChild* >(this)),
                                     (static_cast< beans::XPropertiesChangeNotifier* >(this)),
                                     (static_cast< beans::XPropertyContainer* >(this)),
                                     (static_cast< XContentCreator* >(this)),
                                     (static_cast< beans::XPropertySetInfoChangeNotifier* >(this)),
                                     (static_cast< XContent* >(this)) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


// XComponent


void SAL_CALL
BaseContent::addEventListener( const Reference< lang::XEventListener >& Listener )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( ! m_pDisposeEventListeners )
        m_pDisposeEventListeners =
            new comphelper::OInterfaceContainerHelper2( m_aEventListenerMutex );

    m_pDisposeEventListeners->addInterface( Listener );
}


void SAL_CALL
BaseContent::removeEventListener( const Reference< lang::XEventListener >& Listener )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_pDisposeEventListeners )
        m_pDisposeEventListeners->removeInterface( Listener );
}


void SAL_CALL
BaseContent::dispose()
{
    lang::EventObject aEvt;
    comphelper::OInterfaceContainerHelper2* pDisposeEventListeners;
    comphelper::OInterfaceContainerHelper2* pContentEventListeners;
    comphelper::OInterfaceContainerHelper2* pPropertySetInfoChangeListeners;
    PropertyListeners* pPropertyListener;

    {
        osl::MutexGuard aGuard( m_aMutex );
        aEvt.Source = static_cast< XContent* >( this );


        pDisposeEventListeners = m_pDisposeEventListeners;
        m_pDisposeEventListeners = nullptr;

        pContentEventListeners = m_pContentEventListeners;
        m_pContentEventListeners = nullptr;

        pPropertySetInfoChangeListeners = m_pPropertySetInfoChangeListeners;
        m_pPropertySetInfoChangeListeners = nullptr;

        pPropertyListener = m_pPropertyListener;
        m_pPropertyListener = nullptr;
    }

    if ( pDisposeEventListeners && pDisposeEventListeners->getLength() )
        pDisposeEventListeners->disposeAndClear( aEvt );

    if ( pContentEventListeners && pContentEventListeners->getLength() )
        pContentEventListeners->disposeAndClear( aEvt );

    if( pPropertyListener )
        pPropertyListener->disposeAndClear( aEvt );

    if( pPropertySetInfoChangeListeners )
        pPropertySetInfoChangeListeners->disposeAndClear( aEvt );

    delete pDisposeEventListeners;
    delete pContentEventListeners;
    delete pPropertyListener;
    delete pPropertySetInfoChangeListeners;
}

//  XServiceInfo
OUString SAL_CALL
BaseContent::getImplementationName()
{
    return OUString("com.sun.star.comp.ucb.FileContent");
}

sal_Bool SAL_CALL
BaseContent::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

Sequence< OUString > SAL_CALL
BaseContent::getSupportedServiceNames()
{
    Sequence<OUString> ret { "com.sun.star.ucb.FileContent" };
    return ret;
}

//  XTypeProvider
XTYPEPROVIDER_IMPL_10( BaseContent,
                       lang::XComponent,
                       lang::XTypeProvider,
                       lang::XServiceInfo,
                       XCommandProcessor,
                       XContentCreator,
                       XContent,
                       container::XChild,
                       beans::XPropertiesChangeNotifier,
                       beans::XPropertyContainer,
                       beans::XPropertySetInfoChangeNotifier )


//  XCommandProcessor


sal_Int32 SAL_CALL
BaseContent::createCommandIdentifier()
{
    return m_pMyShell->getCommandId();
}


void SAL_CALL
BaseContent::abort( sal_Int32 CommandId )
{
    m_pMyShell->abort( CommandId );
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
        aAny <<= getPropertySetInfo( CommandId );
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
        if( ! aCommand.Argument.has< sal_Bool >() )
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
        Sequence< beans::Property > seq(1);
        seq[0] = beans::Property(
            "CasePreservingURL",
            -1,
            cppu::UnoType<sal_Bool>::get(),
            0 );
        Reference< sdbc::XRow > xRow = getPropertyValues( CommandId,seq );
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

    osl::MutexGuard aGuard( m_aMutex );

    if( ! m_pPropertyListener )
        m_pPropertyListener = new PropertyListeners( m_aEventListenerMutex );


    if( PropertyNames.getLength() == 0 )
        m_pPropertyListener->addInterface( OUString(),Listener );
    else
    {
        Reference< beans::XPropertySetInfo > xProp = m_pMyShell->info_p( m_aUncPath );
        for( sal_Int32 i = 0; i < PropertyNames.getLength(); ++i )
            if( xProp->hasPropertyByName( PropertyNames[i] ) )
                m_pPropertyListener->addInterface( PropertyNames[i],Listener );
    }
}


void SAL_CALL
BaseContent::removePropertiesChangeListener( const Sequence< OUString >& PropertyNames,
                                             const Reference< beans::XPropertiesChangeListener >& Listener )
{
    if( ! Listener.is() )
        return;

    osl::MutexGuard aGuard( m_aMutex );

    if( ! m_pPropertyListener )
        return;

    for( sal_Int32 i = 0; i < PropertyNames.getLength(); ++i )
        m_pPropertyListener->removeInterface( PropertyNames[i],Listener );

    m_pPropertyListener->removeInterface( OUString(), Listener );
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
                return m_pMyShell->FolderContentType;
            else
                return m_pMyShell->FileContentType;
        }
        else
        {
            try
            {
                // Who am I ?
                Sequence< beans::Property > seq(1);
                seq[0] = beans::Property( "IsDocument",
                                          -1,
                                          cppu::UnoType<sal_Bool>::get(),
                                          0 );
                Reference< sdbc::XRow > xRow = getPropertyValues( -1,seq );
                bool IsDocument = xRow->getBoolean( 1 );

                if ( !xRow->wasNull() )
                {
                    if ( IsDocument )
                        return m_pMyShell->FileContentType;
                    else
                        return m_pMyShell->FolderContentType;
                }
                else
                {
                    OSL_FAIL( "BaseContent::getContentType - Property value was null!" );
                }
            }
            catch (const sdbc::SQLException&)
            {
                OSL_FAIL( "BaseContent::getContentType - Caught SQLException!" );
            }
        }
    }

    return OUString();
}


void SAL_CALL
BaseContent::addContentEventListener(
    const Reference< XContentEventListener >& Listener )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( ! m_pContentEventListeners )
        m_pContentEventListeners =
            new comphelper::OInterfaceContainerHelper2( m_aEventListenerMutex );


    m_pContentEventListeners->addInterface( Listener );
}


void SAL_CALL
BaseContent::removeContentEventListener(
    const Reference< XContentEventListener >& Listener )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_pContentEventListeners )
        m_pContentEventListeners->removeInterface( Listener );
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
        throw beans::UnknownPropertyException( THROW_WHERE );

    m_pMyShell->deassociate( m_aUncPath, Name );
}


// XContentCreator


Sequence< ContentInfo > SAL_CALL
BaseContent::queryCreatableContentsInfo()
{
    return m_pMyShell->queryCreatableContentsInfo();
}


Reference< XContent > SAL_CALL
BaseContent::createNewContent( const ContentInfo& Info )
{
    // Check type.
    if ( Info.Type.isEmpty() )
        return Reference< XContent >();

    bool bFolder = Info.Type == m_pMyShell->FolderContentType;
    if ( !bFolder )
    {
        if ( Info.Type != m_pMyShell->FileContentType )
        {
            // Neither folder nor file to create!
            return Reference< XContent >();
        }
    }

    // Who am I ?
    bool IsDocument = false;

    try
    {
        Sequence< beans::Property > seq(1);
        seq[0] = beans::Property( "IsDocument",
                                  -1,
                                  cppu::UnoType<sal_Bool>::get(),
                                  0 );
        Reference< sdbc::XRow > xRow = getPropertyValues( -1,seq );
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
        OSL_FAIL( "BaseContent::createNewContent - Caught SQLException!" );
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

    BaseContent* p = new BaseContent( m_pMyShell, dstUncPath, bFolder );
    return Reference< XContent >( p );
}


// XPropertySetInfoChangeNotifier


void SAL_CALL
BaseContent::addPropertySetInfoChangeListener(
    const Reference< beans::XPropertySetInfoChangeListener >& Listener )
{
    osl::MutexGuard aGuard( m_aMutex );
    if( ! m_pPropertySetInfoChangeListeners )
        m_pPropertySetInfoChangeListeners = new comphelper::OInterfaceContainerHelper2( m_aEventListenerMutex );

    m_pPropertySetInfoChangeListeners->addInterface( Listener );
}


void SAL_CALL
BaseContent::removePropertySetInfoChangeListener(
    const Reference< beans::XPropertySetInfoChangeListener >& Listener )
{
    osl::MutexGuard aGuard( m_aMutex );

    if( m_pPropertySetInfoChangeListeners )
        m_pPropertySetInfoChangeListeners->removeInterface( Listener );
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

    FileContentIdentifier* p = new FileContentIdentifier( ParentUnq );
    Reference< XContentIdentifier > Identifier( p );

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


Reference< XCommandInfo > SAL_CALL
BaseContent::getCommandInfo()
{
    if( m_nState & Deleted )
        return Reference< XCommandInfo >();

    return m_pMyShell->info_c();
}


Reference< beans::XPropertySetInfo > SAL_CALL
BaseContent::getPropertySetInfo(
    sal_Int32 )
{
    if( m_nState & Deleted )
        return Reference< beans::XPropertySetInfo >();

    return m_pMyShell->info_p( m_aUncPath );
}

Reference< sdbc::XRow > SAL_CALL
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
                rValue <<= m_bFolder ? m_pMyShell->FolderContentType
                    : m_pMyShell->FileContentType;
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


Sequence< Any > SAL_CALL
BaseContent::setPropertyValues(
    sal_Int32 nMyCommandIdentifier,
    const Sequence< beans::PropertyValue >& Values )
{
    if( m_nState & Deleted )
    {   //  To do
        return Sequence< Any >( Values.getLength() );
    }

    const OUString Title("Title");

    // Special handling for files which have to be inserted
    if( m_nState & JustInserted )
    {
        for( sal_Int32 i = 0; i < Values.getLength(); ++i )
        {
            if( Values[i].Name == Title )
            {
                OUString NewTitle;
                if( Values[i].Value >>= NewTitle )
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
                            m_aUncPath.copy( 0, nLastSlash + 1 ) );

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

        // Special handling Title: Setting Title is equivalent to a renaming of the underlying file
        for( sal_Int32 i = 0; i < Values.getLength(); ++i )
        {
            if( Values[i].Name != Title )
                continue;                  // handled by setv

            OUString NewTitle;
            if( !( Values[i].Value >>= NewTitle ) )
            {
                ret[i] <<= beans::IllegalTypeException( THROW_WHERE );
                break;
            }
            else if( NewTitle.isEmpty() )
            {
                ret[i] <<= lang::IllegalArgumentException( THROW_WHERE, uno::Reference< uno::XInterface >(), 0 );
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
                ret[i] <<= e;
            }

            // NameChanges come back through a ContentEvent
            break; // only handling Title
        } // end for

        return ret;
    }
}


Reference< XDynamicResultSet > SAL_CALL
BaseContent::open(
    sal_Int32 nMyCommandIdentifier,
    const OpenCommandArgument2& aCommandArgument )
{
    Reference< XDynamicResultSet > retValue( nullptr );

    if( ( m_nState & Deleted ) )
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


void SAL_CALL
BaseContent::deleteContent( sal_Int32 nMyCommandIdentifier )
{
    if( m_nState & Deleted )
        return;

    if( m_pMyShell->remove( nMyCommandIdentifier,m_aUncPath ) )
    {
        osl::MutexGuard aGuard( m_aMutex );
        m_nState |= Deleted;
    }
}


void SAL_CALL
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
    Sequence< beans::Property > seq(1);
    seq[0] = beans::Property( "IsDocument",
                              -1,
                              cppu::UnoType<sal_Bool>::get(),
                              0 );
    Reference< sdbc::XRow > xRow = getPropertyValues( nMyCommandIdentifier,seq );
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


void SAL_CALL BaseContent::insert( sal_Int32 nMyCommandIdentifier,
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

    Sequence< beans::Property > seq(1);
    seq[0] = beans::Property( "IsDocument",
                              -1,
                              cppu::UnoType<sal_Bool>::get(),
                              0 );

    Reference< sdbc::XRow > xRow = getPropertyValues( -1,seq );

    bool contentTypeSet = true;  // is set to false, if contentType not set
    try
    {
        bDocument = xRow->getBoolean( 1 );
        if( xRow->wasNull() )
            contentTypeSet = false;

    }
    catch (const sdbc::SQLException&)
    {
        OSL_FAIL( "BaseContent::insert - Caught SQLException!" );
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
                        getTitle(m_aUncPath),
                        rtl_UriDecodeWithCharset,
                        RTL_TEXTENCODING_UTF8),
                    static_cast<cppu::OWeakObject*>(this),
                    m_pMyShell,nMyCommandIdentifier);
            uno::Reference<task::XInteractionRequest> const xReq(aRequestImpl.getRequest());

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

    osl::MutexGuard aGuard( m_aMutex );
    m_nState = FullFeatured;
}


void SAL_CALL BaseContent::endTask( sal_Int32 CommandId )
{
    // This is the only function allowed to throw an exception
    m_pMyShell->endTask( CommandId,m_aUncPath,this );
}


ContentEventNotifier*
BaseContent::cDEL()
{
    osl::MutexGuard aGuard( m_aMutex );

    m_nState |= Deleted;

    ContentEventNotifier* p;
    if( m_pContentEventListeners )
    {
        p = new ContentEventNotifier( m_pMyShell,
                                      this,
                                      m_xContentIdentifier,
                                      m_pContentEventListeners->getElements() );
    }
    else
        p = nullptr;

    return p;
}


ContentEventNotifier*
BaseContent::cEXC( const OUString& aNewName )
{
    osl::MutexGuard aGuard( m_aMutex );

    Reference< XContentIdentifier > xOldRef = m_xContentIdentifier;
    m_aUncPath = aNewName;
    FileContentIdentifier* pp = new FileContentIdentifier( aNewName );
    m_xContentIdentifier.set( pp );

    ContentEventNotifier* p = nullptr;
    if( m_pContentEventListeners )
        p = new ContentEventNotifier( m_pMyShell,
                                      this,
                                      m_xContentIdentifier,
                                      xOldRef,
                                      m_pContentEventListeners->getElements() );

    return p;
}


ContentEventNotifier*
BaseContent::cCEL()
{
    osl::MutexGuard aGuard( m_aMutex );
    ContentEventNotifier* p = nullptr;
    if( m_pContentEventListeners )
        p = new ContentEventNotifier( m_pMyShell,
                                      this,
                                      m_xContentIdentifier,
                                      m_pContentEventListeners->getElements() );

    return p;
}

PropertySetInfoChangeNotifier*
BaseContent::cPSL()
{
    osl::MutexGuard aGuard( m_aMutex );
    PropertySetInfoChangeNotifier* p = nullptr;
    if( m_pPropertySetInfoChangeListeners  )
        p = new PropertySetInfoChangeNotifier( this,
                                               m_pPropertySetInfoChangeListeners->getElements() );

    return p;
}


PropertyChangeNotifier*
BaseContent::cPCL()
{
    osl::MutexGuard aGuard( m_aMutex );

    if (!m_pPropertyListener)
        return nullptr;

    Sequence< OUString > seqNames = m_pPropertyListener->getContainedTypes();

    PropertyChangeNotifier* p = nullptr;

    sal_Int32 length = seqNames.getLength();

    if( length )
    {
        ListenerMap* listener = new ListenerMap();
        for( sal_Int32 i = 0; i < length; ++i )
        {
            cppu::OInterfaceContainerHelper* pContainer = m_pPropertyListener->getContainer(seqNames[i]);
            if (!pContainer)
                continue;
            (*listener)[seqNames[i]] = pContainer->getElements();
        }

        p = new PropertyChangeNotifier( this,
                                        listener );
    }

    return p;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
