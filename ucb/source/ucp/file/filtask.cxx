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

#include <config_features.h>

#include <sal/config.h>
#include <sal/log.hxx>

#if HAVE_FEATURE_MACOSX_SANDBOX
#include <sys/stat.h>
#endif

#include <com/sun/star/beans/IllegalTypeException.hpp>
#include <com/sun/star/beans/NotRemoveableException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyExistException.hpp>
#include <com/sun/star/io/BufferSizeExceededException.hpp>
#include <com/sun/star/io/NotConnectedException.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/lang/IllegalAccessException.hpp>
#include <com/sun/star/task/InteractionClassification.hpp>
#include <com/sun/star/ucb/ContentInfoAttribute.hpp>
#include <com/sun/star/ucb/IOErrorCode.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/ucb/OpenCommandArgument.hpp>
#include <com/sun/star/ucb/Store.hpp>
#include <comphelper/propertysequence.hxx>
#include <rtl/uri.hxx>

#include "filtask.hxx"
#include "filcmd.hxx"
#include "filglob.hxx"
#include "filinpstr.hxx"
#include "filprp.hxx"
#include "filrset.hxx"
#include "filstr.hxx"
#include "prov.hxx"

/******************************************************************************/
/*                                                                            */
/*                              TaskHandling                                  */
/*                                                                            */
/******************************************************************************/


using namespace fileaccess;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;

#if OSL_DEBUG_LEVEL > 0
#define THROW_WHERE SAL_WHERE
#else
#define THROW_WHERE ""
#endif

TaskManager::UnqPathData::UnqPathData() = default;

TaskManager::UnqPathData::UnqPathData(TaskManager::UnqPathData&&) = default;


TaskManager::UnqPathData::~UnqPathData()
{
}

TaskManager::MyProperty::MyProperty( const OUString&                         thePropertyName )
    : PropertyName( thePropertyName )
    , Handle(-1)
    , isNative(false)
    , State(beans::PropertyState_AMBIGUOUS_VALUE)
    , Attributes(0)
{
    // empty
}

TaskManager::MyProperty::MyProperty( bool                               theisNative,
                               const OUString&                    thePropertyName,
                               sal_Int32                          theHandle,
                               const css::uno::Type&              theTyp,
                               const css::uno::Any&               theValue,
                               const css::beans::PropertyState&   theState,
                               sal_Int16                          theAttributes )
    : PropertyName( thePropertyName ),
      Handle( theHandle ),
      isNative( theisNative ),
      Typ( theTyp ),
      Value( theValue ),
      State( theState ),
      Attributes( theAttributes )
{
    // empty
}

TaskManager::MyProperty::~MyProperty()
{
    // empty for now
}

#include "filinl.hxx"

        // Default properties

static const OUStringLiteral Title( "Title" );
static const OUStringLiteral CasePreservingURL( "CasePreservingURL" );
static const OUStringLiteral IsDocument( "IsDocument" );
static const OUStringLiteral IsFolder( "IsFolder" );
static const OUStringLiteral DateModified( "DateModified" );
static const OUStringLiteral Size( "Size" );
static const OUStringLiteral IsVolume( "IsVolume" );
static const OUStringLiteral IsRemoveable( "IsRemoveable" );
static const OUStringLiteral IsRemote( "IsRemote" );
static const OUStringLiteral IsCompactDisc( "IsCompactDisc" );
static const OUStringLiteral IsFloppy( "IsFloppy" );
static const OUStringLiteral IsHidden( "IsHidden" );
static const OUStringLiteral ContentType( "ContentType" );
static const OUStringLiteral IsReadOnly( "IsReadOnly" );
static const OUStringLiteral CreatableContentsInfo( "CreatableContentsInfo" );
const OUStringLiteral TaskManager::FolderContentType( "application/vnd.sun.staroffice.fsys-folder" );
const OUStringLiteral TaskManager::FileContentType( "application/vnd.sun.staroffice.fsys-file" );

TaskManager::TaskManager( const uno::Reference< uno::XComponentContext >& rxContext,
              FileProvider* pProvider, bool bWithConfig )
    : m_nCommandId( 0 ),
      m_pProvider( pProvider ),
      m_xContext( rxContext ),
      m_sCommandInfo( 9 )
{
    // Title
    m_aDefaultProperties.insert( MyProperty( true,
                                             Title,
                                             -1 ,
                                             cppu::UnoType<OUString>::get(),
                                             uno::Any(),
                                             beans::PropertyState_DEFAULT_VALUE,
                                             beans::PropertyAttribute::MAYBEVOID
                                             | beans::PropertyAttribute::BOUND ) );

    // CasePreservingURL
    m_aDefaultProperties.insert(
        MyProperty( true,
                    CasePreservingURL,
                    -1 ,
                    cppu::UnoType<OUString>::get(),
                    uno::Any(),
                    beans::PropertyState_DEFAULT_VALUE,
                    beans::PropertyAttribute::MAYBEVOID
                    | beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY ) );


    // IsFolder
    m_aDefaultProperties.insert( MyProperty( true,
                                             IsFolder,
                                             -1 ,
                                             cppu::UnoType<sal_Bool>::get(),
                                             uno::Any(),
                                             beans::PropertyState_DEFAULT_VALUE,
                                             beans::PropertyAttribute::MAYBEVOID
                                             | beans::PropertyAttribute::BOUND
                                             | beans::PropertyAttribute::READONLY ) );


    // IsDocument
    m_aDefaultProperties.insert( MyProperty( true,
                                             IsDocument,
                                             -1 ,
                                             cppu::UnoType<sal_Bool>::get(),
                                             uno::Any(),
                                             beans::PropertyState_DEFAULT_VALUE,
                                             beans::PropertyAttribute::MAYBEVOID
                                             | beans::PropertyAttribute::BOUND
                                             | beans::PropertyAttribute::READONLY ) );

    // Removable
    m_aDefaultProperties.insert( MyProperty( true,
                                             IsVolume,
                                             -1 ,
                                             cppu::UnoType<sal_Bool>::get(),
                                             uno::Any(),
                                             beans::PropertyState_DEFAULT_VALUE,
                                             beans::PropertyAttribute::MAYBEVOID
                                             | beans::PropertyAttribute::BOUND
                                             | beans::PropertyAttribute::READONLY ) );


    // Removable
    m_aDefaultProperties.insert( MyProperty( true,
                                             IsRemoveable,
                                             -1 ,
                                             cppu::UnoType<sal_Bool>::get(),
                                             uno::Any(),
                                             beans::PropertyState_DEFAULT_VALUE,
                                             beans::PropertyAttribute::MAYBEVOID
                                             | beans::PropertyAttribute::BOUND
                                             | beans::PropertyAttribute::READONLY ) );

    // Remote
    m_aDefaultProperties.insert( MyProperty( true,
                                             IsRemote,
                                             -1 ,
                                             cppu::UnoType<sal_Bool>::get(),
                                             uno::Any(),
                                             beans::PropertyState_DEFAULT_VALUE,
                                             beans::PropertyAttribute::MAYBEVOID
                                             | beans::PropertyAttribute::BOUND
                                             | beans::PropertyAttribute::READONLY ) );

    // CompactDisc
    m_aDefaultProperties.insert( MyProperty( true,
                                             IsCompactDisc,
                                             -1 ,
                                             cppu::UnoType<sal_Bool>::get(),
                                             uno::Any(),
                                             beans::PropertyState_DEFAULT_VALUE,
                                             beans::PropertyAttribute::MAYBEVOID
                                             | beans::PropertyAttribute::BOUND
                                             | beans::PropertyAttribute::READONLY ) );

    // Floppy
    m_aDefaultProperties.insert( MyProperty( true,
                                             IsFloppy,
                                             -1 ,
                                             cppu::UnoType<sal_Bool>::get(),
                                             uno::Any(),
                                             beans::PropertyState_DEFAULT_VALUE,
                                             beans::PropertyAttribute::MAYBEVOID
                                             | beans::PropertyAttribute::BOUND
                                             | beans::PropertyAttribute::READONLY ) );

    // Hidden
    m_aDefaultProperties.insert(
        MyProperty(
            true,
            IsHidden,
            -1 ,
            cppu::UnoType<sal_Bool>::get(),
            uno::Any(),
            beans::PropertyState_DEFAULT_VALUE,
            beans::PropertyAttribute::MAYBEVOID
            | beans::PropertyAttribute::BOUND
#if defined(_WIN32)
        ));
#else
    | beans::PropertyAttribute::READONLY)); // under unix/linux only readable
#endif


    // ContentType
    m_aDefaultProperties.insert( MyProperty( false,
                                             ContentType,
                                             -1 ,
                                             cppu::UnoType<OUString>::get(),
                                             uno::Any(OUString()),
                                             beans::PropertyState_DEFAULT_VALUE,
                                             beans::PropertyAttribute::MAYBEVOID
                                             | beans::PropertyAttribute::BOUND
                                             | beans::PropertyAttribute::READONLY ) );


    // DateModified
    m_aDefaultProperties.insert( MyProperty( true,
                                             DateModified,
                                             -1 ,
                                             cppu::UnoType<util::DateTime>::get(),
                                             uno::Any(),
                                             beans::PropertyState_DEFAULT_VALUE,
                                             beans::PropertyAttribute::MAYBEVOID
                                             | beans::PropertyAttribute::BOUND ) );

    // Size
    m_aDefaultProperties.insert( MyProperty( true,
                                             Size,
                                             -1,
                                             cppu::UnoType<sal_Int64>::get(),
                                             uno::Any(),
                                             beans::PropertyState_DEFAULT_VALUE,
                                             beans::PropertyAttribute::MAYBEVOID
                                             | beans::PropertyAttribute::BOUND ) );

    // IsReadOnly
    m_aDefaultProperties.insert( MyProperty( true,
                                             IsReadOnly,
                                             -1 ,
                                             cppu::UnoType<sal_Bool>::get(),
                                             uno::Any(),
                                             beans::PropertyState_DEFAULT_VALUE,
                                             beans::PropertyAttribute::MAYBEVOID
                                             | beans::PropertyAttribute::BOUND ) );


    // CreatableContentsInfo
    m_aDefaultProperties.insert( MyProperty( true,
                                             CreatableContentsInfo,
                                             -1 ,
                                             cppu::UnoType<uno::Sequence< ucb::ContentInfo >>::get(),
                                             uno::Any(),
                                             beans::PropertyState_DEFAULT_VALUE,
                                             beans::PropertyAttribute::MAYBEVOID
                                             | beans::PropertyAttribute::BOUND
                                             | beans::PropertyAttribute::READONLY ) );

    // Commands
    m_sCommandInfo[0].Name = "getCommandInfo";
    m_sCommandInfo[0].Handle = -1;
    m_sCommandInfo[0].ArgType = cppu::UnoType<void>::get();

    m_sCommandInfo[1].Name = "getPropertySetInfo";
    m_sCommandInfo[1].Handle = -1;
    m_sCommandInfo[1].ArgType = cppu::UnoType<void>::get();

    m_sCommandInfo[2].Name = "getPropertyValues";
    m_sCommandInfo[2].Handle = -1;
    m_sCommandInfo[2].ArgType = cppu::UnoType<uno::Sequence< beans::Property >>::get();

    m_sCommandInfo[3].Name = "setPropertyValues";
    m_sCommandInfo[3].Handle = -1;
    m_sCommandInfo[3].ArgType = cppu::UnoType<uno::Sequence< beans::PropertyValue >>::get();

    m_sCommandInfo[4].Name = "open";
    m_sCommandInfo[4].Handle = -1;
    m_sCommandInfo[4].ArgType = cppu::UnoType<OpenCommandArgument>::get();

    m_sCommandInfo[5].Name = "transfer";
    m_sCommandInfo[5].Handle = -1;
    m_sCommandInfo[5].ArgType = cppu::UnoType<TransferInfo>::get();

    m_sCommandInfo[6].Name = "delete";
    m_sCommandInfo[6].Handle = -1;
    m_sCommandInfo[6].ArgType = cppu::UnoType<sal_Bool>::get();

    m_sCommandInfo[7].Name = "insert";
    m_sCommandInfo[7].Handle = -1;
    m_sCommandInfo[7].ArgType = cppu::UnoType<InsertCommandArgument>::get();

    m_sCommandInfo[8].Name = "createNewContent";
    m_sCommandInfo[8].Handle = -1;
    m_sCommandInfo[8].ArgType = cppu::UnoType<ucb::ContentInfo>::get();

    if(bWithConfig)
    {
        uno::Reference< XPropertySetRegistryFactory > xRegFac = ucb::Store::create( m_xContext );
        // Open/create a registry
        m_xFileRegistry = xRegFac->createPropertySetRegistry( OUString() );
    }
}


TaskManager::~TaskManager()
{
}


void
TaskManager::startTask(
    sal_Int32 CommandId,
    const uno::Reference< XCommandEnvironment >& xCommandEnv )
{
    osl::MutexGuard aGuard( m_aMutex );
    TaskMap::iterator it = m_aTaskMap.find( CommandId );
    if( it != m_aTaskMap.end() )
    {
        throw DuplicateCommandIdentifierException( OSL_LOG_PREFIX );
    }
    m_aTaskMap.emplace( CommandId, TaskHandling( xCommandEnv ));
}


void
TaskManager::endTask( sal_Int32 CommandId,
                      const OUString& aUncPath,
                      BaseContent* pContent)
{
    osl::ClearableMutexGuard aGuard( m_aMutex );
    TaskMap::iterator it = m_aTaskMap.find( CommandId );
    if( it == m_aTaskMap.end() )
        return;

    sal_Int32 ErrorCode = it->second.getInstalledError();
    sal_Int32 MinorCode = it->second.getMinorErrorCode();
    bool isHandled = it->second.isHandled();

    Reference< XCommandEnvironment > xComEnv
        = it->second.getCommandEnvironment();

    m_aTaskMap.erase( it );

    aGuard.clear();

    if( ErrorCode != TASKHANDLER_NO_ERROR )
        throw_handler(
            ErrorCode,
            MinorCode,
            xComEnv,
            aUncPath,
            pContent,
            isHandled);
}


void TaskManager::clearError( sal_Int32 CommandId )
{
    osl::MutexGuard aGuard( m_aMutex );
    TaskMap::iterator it = m_aTaskMap.find( CommandId );
    if( it != m_aTaskMap.end() )
        it->second.clearError();
}


void TaskManager::retrieveError( sal_Int32 CommandId,
                                          sal_Int32 &ErrorCode,
                                          sal_Int32 &minorCode)
{
    osl::MutexGuard aGuard( m_aMutex );
    TaskMap::iterator it = m_aTaskMap.find( CommandId );
    if( it != m_aTaskMap.end() )
    {
        ErrorCode = it->second.getInstalledError();
        minorCode = it->second. getMinorErrorCode();
    }
}


void TaskManager::installError( sal_Int32 CommandId,
                                         sal_Int32 ErrorCode,
                                         sal_Int32 MinorCode )
{
    osl::MutexGuard aGuard( m_aMutex );
    TaskMap::iterator it = m_aTaskMap.find( CommandId );
    if( it != m_aTaskMap.end() )
        it->second.installError( ErrorCode,MinorCode );
}


sal_Int32
TaskManager::getCommandId()
{
    osl::MutexGuard aGuard( m_aMutex );
    return ++m_nCommandId;
}


void TaskManager::handleTask(
    sal_Int32 CommandId,
    const uno::Reference< task::XInteractionRequest >& request )
{
    osl::MutexGuard aGuard( m_aMutex );
    TaskMap::iterator it = m_aTaskMap.find( CommandId );
    uno::Reference< task::XInteractionHandler > xInt;
    if( it != m_aTaskMap.end() )
    {
        xInt = it->second.getInteractionHandler();
        if( xInt.is() )
            xInt->handle( request );
        it->second.setHandled();
    }
}

/*********************************************************************************/
/*                                                                               */
/*                     de/registerNotifier-Implementation                        */
/*                                                                               */
/*********************************************************************************/


//  This two methods register and deregister a change listener for the content belonging
//  to URL aUnqPath


void
TaskManager::registerNotifier( const OUString& aUnqPath, Notifier* pNotifier )
{
    osl::MutexGuard aGuard( m_aMutex );

    ContentMap::iterator it =
        m_aContent.emplace( aUnqPath, UnqPathData() ).first;

    if( ! it->second.notifier )
        it->second.notifier.reset( new NotifierList );

    std::vector< Notifier* >& nlist = *( it->second.notifier );

    std::vector<Notifier*>::iterator it1 = std::find(nlist.begin(), nlist.end(), pNotifier);
    if( it1 != nlist.end() )               // Every "Notifier" only once
    {
        return;
    }
    nlist.push_back( pNotifier );
}


void
TaskManager::deregisterNotifier( const OUString& aUnqPath,Notifier* pNotifier )
{
    osl::MutexGuard aGuard( m_aMutex );

    ContentMap::iterator it = m_aContent.find( aUnqPath );
    if( it == m_aContent.end() )
        return;

    it->second.notifier->erase(std::remove(it->second.notifier->begin(), it->second.notifier->end(), pNotifier), it->second.notifier->end());

    if( it->second.notifier->empty() )
        m_aContent.erase( it );
}


/*********************************************************************************/
/*                                                                               */
/*                     de/associate-Implementation                               */
/*                                                                               */
/*********************************************************************************/

//  Used to associate and deassociate a new property with
//  the content belonging to URL UnqPath.
//  The default value and the attributes are input


void
TaskManager::associate( const OUString& aUnqPath,
                  const OUString& PropertyName,
                  const uno::Any& DefaultValue,
                  const sal_Int16 Attributes )
{
    MyProperty newProperty( false,
                            PropertyName,
                            -1,
                            DefaultValue.getValueType(),
                            DefaultValue,
                            beans::PropertyState_DEFAULT_VALUE,
                            Attributes );

    TaskManager::PropertySet::iterator it1 = m_aDefaultProperties.find( newProperty );
    if( it1 != m_aDefaultProperties.end() )
        throw beans::PropertyExistException( THROW_WHERE );

    {
        osl::MutexGuard aGuard( m_aMutex );

        ContentMap::iterator it = m_aContent.emplace( aUnqPath,UnqPathData() ).first;

        // Load the XPersistentPropertySetInfo and create it, if it does not exist
        load( it,true );

        PropertySet& properties = *(it->second.properties);
        it1 = properties.find( newProperty );
        if( it1 != properties.end() )
            throw beans::PropertyExistException(THROW_WHERE );

        // Property does not exist
        properties.insert( newProperty );
        it->second.xC->addProperty( PropertyName,Attributes,DefaultValue );
    }
    notifyPropertyAdded( getPropertySetListeners( aUnqPath ), PropertyName );
}


void
TaskManager::deassociate( const OUString& aUnqPath,
            const OUString& PropertyName )
{
    MyProperty oldProperty( PropertyName );

    TaskManager::PropertySet::iterator it1 = m_aDefaultProperties.find( oldProperty );
    if( it1 != m_aDefaultProperties.end() )
        throw beans::NotRemoveableException( THROW_WHERE );

    osl::MutexGuard aGuard( m_aMutex );

    ContentMap::iterator it = m_aContent.emplace( aUnqPath,UnqPathData() ).first;

    load( it,false );

    PropertySet& properties = *(it->second.properties);

    it1 = properties.find( oldProperty );
    if( it1 == properties.end() )
        throw beans::UnknownPropertyException( THROW_WHERE );

    properties.erase( it1 );

    if( it->second.xC.is() )
        it->second.xC->removeProperty( PropertyName );

    if( properties.size() == 9 )
    {
        MyProperty ContentTProperty( ContentType );

        if( properties.find( ContentTProperty )->getState() == beans::PropertyState_DEFAULT_VALUE )
        {
            it->second.xS = nullptr;
            it->second.xC = nullptr;
            it->second.xA = nullptr;
            if(m_xFileRegistry.is())
                m_xFileRegistry->removePropertySet( aUnqPath );
        }
    }
    notifyPropertyRemoved( getPropertySetListeners( aUnqPath ), PropertyName );
}


/*********************************************************************************/
/*                                                                               */
/*                     page-Implementation                                       */
/*                                                                               */
/*********************************************************************************/

//  Given an xOutputStream, this method writes the content of the file belonging to
//  URL aUnqPath into the XOutputStream


void TaskManager::page( sal_Int32 CommandId,
                           const OUString& aUnqPath,
                           const uno::Reference< io::XOutputStream >& xOutputStream )
{
    osl::File aFile( aUnqPath );
    osl::FileBase::RC err = aFile.open( osl_File_OpenFlag_Read );

    if( err != osl::FileBase::E_None )
    {
        aFile.close();
        installError( CommandId,
                      TASKHANDLING_OPEN_FILE_FOR_PAGING,
                      err );
        return;
    }

    const sal_uInt64 bfz = 4*1024;
    sal_Int8 BFF[bfz];
    sal_uInt64 nrc;  // Retrieved number of Bytes;

    do
    {
        err = aFile.read( static_cast<void*>(BFF),bfz,nrc );
        if(  err == osl::FileBase::E_None )
        {
            uno::Sequence< sal_Int8 > seq( BFF, static_cast<sal_uInt32>(nrc) );
            try
            {
                xOutputStream->writeBytes( seq );
            }
            catch (const io::NotConnectedException&)
            {
                installError( CommandId,
                              TASKHANDLING_NOTCONNECTED_FOR_PAGING );
                break;
            }
            catch (const io::BufferSizeExceededException&)
            {
                installError( CommandId,
                              TASKHANDLING_BUFFERSIZEEXCEEDED_FOR_PAGING );
                break;
            }
            catch (const io::IOException&)
            {
                installError( CommandId,
                              TASKHANDLING_IOEXCEPTION_FOR_PAGING );
                break;
            }
        }
        else
        {
            installError( CommandId,
                          TASKHANDLING_READING_FILE_FOR_PAGING,
                          err );
            break;
        }
    } while( nrc == bfz );


    aFile.close();


    try
    {
        xOutputStream->closeOutput();
    }
    catch (const io::NotConnectedException&)
    {
    }
    catch (const io::BufferSizeExceededException&)
    {
    }
    catch (const io::IOException&)
    {
    }
}


/*********************************************************************************/
/*                                                                               */
/*                     open-Implementation                                       */
/*                                                                               */
/*********************************************************************************/

//  Given a file URL aUnqPath, this methods returns a XInputStream which reads from the open file.


uno::Reference< io::XInputStream >
TaskManager::open( sal_Int32 CommandId,
             const OUString& aUnqPath,
             bool bLock )
{
    std::unique_ptr<XInputStream_impl> pInputStream(new XInputStream_impl( aUnqPath, bLock )); // from filinpstr.hxx

    sal_Int32 ErrorCode = pInputStream->CtorSuccess();

    if( ErrorCode != TASKHANDLER_NO_ERROR )
    {
        installError( CommandId,
                      ErrorCode,
                      pInputStream->getMinorError() );

        pInputStream.reset();
    }

    return uno::Reference< io::XInputStream >( pInputStream.release() );
}


/*********************************************************************************/
/*                                                                               */
/*                     open for read/write access-Implementation                 */
/*                                                                               */
/*********************************************************************************/

//  Given a file URL aUnqPath, this methods returns a XStream which can be used
//  to read and write from/to the file.


uno::Reference< io::XStream >
TaskManager::open_rw( sal_Int32 CommandId,
                const OUString& aUnqPath,
                bool bLock )
{
    std::unique_ptr<XStream_impl> pStream(new XStream_impl( aUnqPath, bLock ));  // from filstr.hxx

    sal_Int32 ErrorCode = pStream->CtorSuccess();

    if( ErrorCode != TASKHANDLER_NO_ERROR )
    {
        installError( CommandId,
                      ErrorCode,
                      pStream->getMinorError() );

        pStream.reset();
    }
    return uno::Reference< io::XStream >( pStream.release() );
}


/*********************************************************************************/
/*                                                                               */
/*                       ls-Implementation                                       */
/*                                                                               */
/*********************************************************************************/

//  This method returns the result set containing the children of the directory belonging
//  to file URL aUnqPath


uno::Reference< XDynamicResultSet >
TaskManager::ls( sal_Int32 CommandId,
           const OUString& aUnqPath,
           const sal_Int32 OpenMode,
           const uno::Sequence< beans::Property >& seq,
           const uno::Sequence< NumberedSortingInfo >& seqSort )
{
    std::unique_ptr<XResultSet_impl> p(new XResultSet_impl( this,aUnqPath,OpenMode,seq,seqSort ));

    sal_Int32 ErrorCode = p->CtorSuccess();

    if( ErrorCode != TASKHANDLER_NO_ERROR )
    {
        installError( CommandId,
                      ErrorCode,
                      p->getMinorError() );

        p.reset();
    }

    return uno::Reference< XDynamicResultSet > ( p.release() );
}


/*********************************************************************************/
/*                                                                               */
/*                          info_c implementation                                */
/*                                                                               */
/*********************************************************************************/
// Info for commands

uno::Reference< XCommandInfo >
TaskManager::info_c()
{
    XCommandInfo_impl* p = new XCommandInfo_impl( this );
    return uno::Reference< XCommandInfo >( p );
}


/*********************************************************************************/
/*                                                                               */
/*                     info_p-Implementation                                     */
/*                                                                               */
/*********************************************************************************/
// Info for the properties

uno::Reference< beans::XPropertySetInfo >
TaskManager::info_p( const OUString& aUnqPath )
{
    osl::MutexGuard aGuard( m_aMutex );
    XPropertySetInfo_impl* p = new XPropertySetInfo_impl( this,aUnqPath );
    return uno::Reference< beans::XPropertySetInfo >( p );
}


/*********************************************************************************/
/*                                                                               */
/*                     setv-Implementation                                       */
/*                                                                               */
/*********************************************************************************/

//  Sets the values of the properties belonging to fileURL aUnqPath


uno::Sequence< uno::Any >
TaskManager::setv( const OUString& aUnqPath,
             const uno::Sequence< beans::PropertyValue >& values )
{
    osl::MutexGuard aGuard( m_aMutex );

    sal_Int32 propChanged = 0;
    uno::Sequence< uno::Any > ret( values.getLength() );
    uno::Sequence< beans::PropertyChangeEvent > seqChanged( values.getLength() );

    TaskManager::ContentMap::iterator it = m_aContent.find( aUnqPath );
    PropertySet& properties = *( it->second.properties );
    TaskManager::PropertySet::iterator it1;
    uno::Any aAny;

    for( sal_Int32 i = 0; i < values.getLength(); ++i )
    {
        MyProperty toset( values[i].Name );
        it1 = properties.find( toset );
        if( it1 == properties.end() )
        {
            ret[i] <<= beans::UnknownPropertyException( THROW_WHERE );
            continue;
        }

        aAny = it1->getValue();
        if( aAny == values[i].Value )
            continue;  // nothing needs to be changed

        if( it1->getAttributes() & beans::PropertyAttribute::READONLY )
        {
            ret[i] <<= lang::IllegalAccessException( THROW_WHERE );
            continue;
        }

        seqChanged[ propChanged   ].PropertyName = values[i].Name;
        seqChanged[ propChanged   ].PropertyHandle   = -1;
        seqChanged[ propChanged   ].Further   = false;
        seqChanged[ propChanged   ].OldValue = aAny;
        seqChanged[ propChanged++ ].NewValue = values[i].Value;

        it1->setValue( values[i].Value );  // Put the new value into the local cash

        if( ! it1->IsNative() )
        {
            // Also put logical properties into storage
            if( !it->second.xS.is() )
                load( it,true );

            if( ( values[i].Name == ContentType ) &&
                it1->getState() == beans::PropertyState_DEFAULT_VALUE )
            {   // Special logic for ContentType
                //  09.07.01: Not reached anymore, because ContentType is readonly
                it1->setState( beans::PropertyState_DIRECT_VALUE );
                it->second.xC->addProperty( values[i].Name,
                                            beans::PropertyAttribute::MAYBEVOID,
                                            values[i].Value );
            }

            try
            {
                it->second.xS->setPropertyValue( values[i].Name,values[i].Value );
            }
            catch (const uno::Exception&e)
            {
                --propChanged; // unsuccessful setting
                ret[i] <<= e;
            }
        }
        else
        {
            // native properties
            // Setting of physical file properties
            if( values[i].Name == Size )
            {
                sal_Int64 newSize = 0;
                if( values[i].Value >>= newSize )
                {   // valid value for the size
                    osl::File aFile(aUnqPath);
                    bool err =
                        aFile.open(osl_File_OpenFlag_Write) != osl::FileBase::E_None ||
                        aFile.setSize(sal_uInt64(newSize)) != osl::FileBase::E_None ||
                        aFile.close() != osl::FileBase::E_None;

                    if( err )
                    {
                        --propChanged; // unsuccessful setting
                        uno::Sequence<uno::Any> names(comphelper::InitAnyPropertySequence(
                        {
                            {"Uri", uno::Any(aUnqPath)}
                        }));
                        ret[i] <<= InteractiveAugmentedIOException(
                            OUString(),
                            nullptr,
                            task::InteractionClassification_ERROR,
                            IOErrorCode_GENERAL,
                            names );
                    }
                }
                else
                    ret[i] <<= beans::IllegalTypeException( THROW_WHERE );
            }
            else if(values[i].Name == IsReadOnly ||
                    values[i].Name == IsHidden)
            {
                bool value = false;
                if( values[i].Value >>= value )
                {
                    osl::DirectoryItem aDirItem;
                    osl::FileBase::RC err =
                        osl::DirectoryItem::get(aUnqPath,aDirItem);
                    sal_uInt64 nAttributes(0);
                    if(err == osl::FileBase::E_None)
                    {
                        osl::FileStatus aFileStatus(osl_FileStatus_Mask_Attributes);
                        err = aDirItem.getFileStatus(aFileStatus);
                        if(err == osl::FileBase::E_None &&
                           aFileStatus.isValid(osl_FileStatus_Mask_Attributes))
                            nAttributes = aFileStatus.getAttributes();
                    }
                    // now we have the attributes provided all went well.
                    if(err == osl::FileBase::E_None) {
                        if(values[i].Name == IsReadOnly)
                        {
                            nAttributes &= ~(osl_File_Attribute_OwnWrite |
                                             osl_File_Attribute_GrpWrite |
                                             osl_File_Attribute_OthWrite |
                                             osl_File_Attribute_ReadOnly);
                            if(value)
                                nAttributes |= osl_File_Attribute_ReadOnly;
                            else
                                nAttributes |= (
                                    osl_File_Attribute_OwnWrite |
                                    osl_File_Attribute_GrpWrite |
                                    osl_File_Attribute_OthWrite);
                        }
                        else if(values[i].Name == IsHidden)
                        {
                            nAttributes &= ~(osl_File_Attribute_Hidden);
                            if(value)
                                nAttributes |= osl_File_Attribute_Hidden;
                        }
                        err = osl::File::setAttributes(
                            aUnqPath,nAttributes);
                    }

                    if( err != osl::FileBase::E_None )
                    {
                        --propChanged; // unsuccessful setting
                        uno::Sequence<uno::Any> names(comphelper::InitAnyPropertySequence(
                        {
                            {"Uri", uno::Any(aUnqPath)}
                        }));
                        IOErrorCode ioError;
                        switch( err )
                        {
                        case osl::FileBase::E_NOMEM:
                            // not enough memory for allocating structures <br>
                            ioError = IOErrorCode_OUT_OF_MEMORY;
                            break;
                        case osl::FileBase::E_INVAL:
                            // the format of the parameters was not valid<p>
                            ioError = IOErrorCode_INVALID_PARAMETER;
                            break;
                        case osl::FileBase::E_NAMETOOLONG:
                            // File name too long<br>
                            ioError = IOErrorCode_NAME_TOO_LONG;
                            break;
                        case osl::FileBase::E_NOENT:
                            // No such file or directory<br>
                        case osl::FileBase::E_NOLINK:
                            // Link has been severed<br>
                            ioError = IOErrorCode_NOT_EXISTING;
                            break;
                        case osl::FileBase::E_ROFS:
                            // #i4735# handle ROFS transparently
                            // as ACCESS_DENIED
                        case  osl::FileBase::E_PERM:
                        case osl::FileBase::E_ACCES:
                            // permission denied<br>
                            ioError = IOErrorCode_ACCESS_DENIED;
                            break;
                        case osl::FileBase::E_LOOP:
                            // Too many symbolic links encountered<br>
                        case osl::FileBase::E_FAULT:
                            // Bad address<br>
                        case osl::FileBase::E_IO:
                            // I/O error<br>
                        case osl::FileBase::E_NOSYS:
                            // Function not implemented<br>
                        case osl::FileBase::E_MULTIHOP:
                            // Multihop attempted<br>
                        case osl::FileBase::E_INTR:
                            // function call was interrupted<p>
                        default:
                            ioError = IOErrorCode_GENERAL;
                            break;
                        }
                        ret[i] <<= InteractiveAugmentedIOException(
                            OUString(),
                            nullptr,
                            task::InteractionClassification_ERROR,
                            ioError,
                            names );
                    }
                }
                else
                    ret[i] <<= beans::IllegalTypeException( THROW_WHERE );
            }
        }
    }   // end for

    if( propChanged )
    {
        seqChanged.realloc( propChanged );
        notifyPropertyChanges( getPropertyChangeNotifier( aUnqPath ),seqChanged );
    }

    return ret;
}

/*********************************************************************************/
/*                                                                               */
/*                     getv-Implementation                                       */
/*                                                                               */
/*********************************************************************************/

//  Reads the values of the properties belonging to fileURL aUnqPath;
//  Returns an XRow object containing the values in the requested order.


uno::Reference< sdbc::XRow >
TaskManager::getv( sal_Int32 CommandId,
             const OUString& aUnqPath,
             const uno::Sequence< beans::Property >& properties )
{
    uno::Sequence< uno::Any > seq( properties.getLength() );

    sal_Int32 n_Mask;
    getMaskFromProperties( n_Mask,properties );
    osl::FileStatus aFileStatus( n_Mask );

    osl::DirectoryItem aDirItem;
    osl::FileBase::RC nError1 = osl::DirectoryItem::get( aUnqPath,aDirItem );
    if( nError1 != osl::FileBase::E_None )
        installError(CommandId,
                     TASKHANDLING_OPEN_FILE_FOR_PAGING, // BEAWARE, REUSED
                     nError1);

    osl::FileBase::RC nError2 = aDirItem.getFileStatus( aFileStatus );
    if( nError1 == osl::FileBase::E_None &&
        nError2 != osl::FileBase::E_None )
        installError(CommandId,
                     TASKHANDLING_OPEN_FILE_FOR_PAGING, // BEAWARE, REUSED
                     nError2);

    {
        osl::MutexGuard aGuard( m_aMutex );

        TaskManager::ContentMap::iterator it = m_aContent.find( aUnqPath );
        commit( it,aFileStatus );

        TaskManager::PropertySet::iterator it1;
        PropertySet& propset = *(it->second.properties);

        for( sal_Int32 i = 0; i < seq.getLength(); ++i )
        {
            MyProperty readProp( properties[i].Name );
            it1 = propset.find( readProp );
            if( it1 == propset.end() )
                seq[i] = uno::Any();
            else
                seq[i] = it1->getValue();
        }
    }

    XRow_impl* p = new XRow_impl( this,seq );
    return uno::Reference< sdbc::XRow >( p );
}


/********************************************************************************/
/*                                                                              */
/*                         transfer-commandos                                   */
/*                                                                              */
/********************************************************************************/


/********************************************************************************/
/*                                                                              */
/*                         move-implementation                                  */
/*                                                                              */
/********************************************************************************/

//  Moves the content belonging to fileURL srcUnqPath to fileURL dstUnqPath.


void
TaskManager::move( sal_Int32 CommandId,
             const OUString& srcUnqPath,
             const OUString& dstUnqPathIn,
             const sal_Int32 NameClash )
{
    // --> #i88446# Method notifyContentExchanged( getContentExchangedEventListeners( srcUnqPath,dstUnqPath,!isDocument ) ); crashes if
    // srcUnqPath and dstUnqPathIn are equal
    if( srcUnqPath == dstUnqPathIn )
        return;

    osl::FileBase::RC nError;
    OUString dstUnqPath( dstUnqPathIn );

    switch( NameClash )
    {
    case NameClash::KEEP:
        {
            nError = osl_File_move( srcUnqPath,dstUnqPath,true );
            if( nError != osl::FileBase::E_None && nError != osl::FileBase::E_EXIST )
            {
                installError( CommandId,
                              TASKHANDLING_KEEPERROR_FOR_MOVE,
                              nError );
                return;
            }
            break;
        }
    case NameClash::OVERWRITE:
        {
            // stat to determine whether we have a symlink
            OUString targetPath(dstUnqPath);

            osl::FileStatus aStatus(osl_FileStatus_Mask_Type|osl_FileStatus_Mask_LinkTargetURL);
            osl::DirectoryItem aItem;
            osl::DirectoryItem::get(dstUnqPath,aItem);
            aItem.getFileStatus(aStatus);

            if( aStatus.isValid(osl_FileStatus_Mask_Type)          &&
                aStatus.isValid(osl_FileStatus_Mask_LinkTargetURL) &&
                aStatus.getFileType() == osl::FileStatus::Link )
                targetPath = aStatus.getLinkTargetURL();

            // Will do nothing if file does not exist.
            osl::File::remove( targetPath );

            nError = osl_File_move( srcUnqPath,targetPath );
            if( nError != osl::FileBase::E_None )
            {
                installError( CommandId,
                              TASKHANDLING_OVERWRITE_FOR_MOVE,
                              nError );
                return;
            }
            break;
        }
    case NameClash::RENAME:
        {
            OUString newDstUnqPath;
            nError = osl_File_move( srcUnqPath,dstUnqPath,true );
            if( nError == osl::FileBase::E_EXIST )
            {
                // "invent" a new valid title.

                sal_Int32 nPos = -1;
                sal_Int32 nLastDot = dstUnqPath.lastIndexOf( '.' );
                sal_Int32 nLastSlash = dstUnqPath.lastIndexOf( '/' );
                if( ( nLastSlash < nLastDot )                  // dot is part of last(!) path segment
                    && ( nLastSlash != ( nLastDot - 1 ) ) )    // file name does not start with a dot
                    nPos = nLastDot;
                else
                    nPos = dstUnqPath.getLength();

                sal_Int32 nTry = 0;

                do
                {
                    newDstUnqPath = dstUnqPath;

                    OUString aPostfix =  "_" + OUString::number( ++nTry );

                    newDstUnqPath = newDstUnqPath.replaceAt( nPos, 0, aPostfix );

                    nError = osl_File_move( srcUnqPath,newDstUnqPath,true );
                }
                while( ( nError == osl::FileBase::E_EXIST ) && ( nTry < 10000 ) );
            }

            if( nError == osl::FileBase::E_EXIST )
            {
                installError( CommandId,
                              TASKHANDLING_RENAME_FOR_MOVE );
                return;
            }
            else if( nError != osl::FileBase::E_None )
            {
                installError( CommandId,
                              TASKHANDLING_RENAMEMOVE_FOR_MOVE,
                              nError );
                return;
            }
            else
                dstUnqPath = newDstUnqPath;

            break;
        }
    case NameClash::ERROR:
        {
            nError = osl_File_move( srcUnqPath,dstUnqPath,true );
            if( nError == osl::FileBase::E_EXIST )
            {
                installError( CommandId,
                              TASKHANDLING_NAMECLASH_FOR_MOVE );
                return;
            }
            else if( nError != osl::FileBase::E_None )
            {
                installError( CommandId,
                              TASKHANDLING_NAMECLASHMOVE_FOR_MOVE,
                              nError );
                return;
            }
            break;
        }
        case NameClash::ASK:
        default:
        {
            nError = osl_File_move( srcUnqPath,dstUnqPath,true );
            if( nError == osl::FileBase::E_EXIST )
            {
                installError( CommandId,
                              TASKHANDLING_NAMECLASHSUPPORT_FOR_MOVE,
                              NameClash::ASK);
                return;
            }
        }
        break;
    }

    // Determine, whether we have moved a file or a folder
    osl::DirectoryItem aItem;
    nError = osl::DirectoryItem::get( dstUnqPath,aItem );
    if( nError != osl::FileBase::E_None )
    {
        installError( CommandId,
                      TASKHANDLING_TRANSFER_BY_MOVE_SOURCE,
                      nError );
        return;
    }
    osl::FileStatus aStatus( osl_FileStatus_Mask_Type );
    nError = aItem.getFileStatus( aStatus );
    if( nError != osl::FileBase::E_None || ! aStatus.isValid( osl_FileStatus_Mask_Type ) )
    {
        installError( CommandId,
                      TASKHANDLING_TRANSFER_BY_MOVE_SOURCESTAT,
                      nError );
        return;
    }
    bool isDocument = ( aStatus.getFileType() == osl::FileStatus::Regular );


    copyPersistentSet( srcUnqPath,dstUnqPath,!isDocument );

    OUString aDstParent = getParentName( dstUnqPath );
    OUString aSrcParent = getParentName( srcUnqPath );

    notifyInsert( getContentEventListeners( aDstParent ),dstUnqPath );
    if(  aDstParent != aSrcParent )
        notifyContentRemoved( getContentEventListeners( aSrcParent ),srcUnqPath );

    notifyContentExchanged( getContentExchangedEventListeners( srcUnqPath,dstUnqPath,!isDocument ) );
    erasePersistentSet( srcUnqPath,!isDocument );
}


/********************************************************************************/
/*                                                                              */
/*                         copy-implementation                                  */
/*                                                                              */
/********************************************************************************/

//  Copies the content belonging to fileURL srcUnqPath to fileURL dstUnqPath ( files and directories )


namespace {

bool getType(
    TaskManager & task, sal_Int32 id, OUString const & fileUrl,
    osl::DirectoryItem * item, osl::FileStatus::Type * type)
{
    OSL_ASSERT(item != nullptr && type != nullptr);
    osl::FileBase::RC err = osl::DirectoryItem::get(fileUrl, *item);
    if (err != osl::FileBase::E_None) {
        task.installError(id, TASKHANDLING_TRANSFER_BY_COPY_SOURCE, err);
        return false;
    }
    osl::FileStatus stat(osl_FileStatus_Mask_Type);
    err = item->getFileStatus(stat);
    if (err != osl::FileBase::E_None) {
        task.installError(id, TASKHANDLING_TRANSFER_BY_COPY_SOURCESTAT, err);
        return false;
    }
    *type = stat.getFileType();
    return true;
}

}

void
TaskManager::copy(
    sal_Int32 CommandId,
    const OUString& srcUnqPath,
    const OUString& dstUnqPathIn,
    sal_Int32 NameClash )
{
    osl::FileBase::RC nError;
    OUString dstUnqPath( dstUnqPathIn );

    // Resolve symbolic links within the source path.  If srcUnqPath denotes a
    // symbolic link (targeting either a file or a folder), the contents of the
    // target is copied (recursively, in the case of a folder).  However, if
    // recursively copying the contents of a folder causes a symbolic link to be
    // copied, the symbolic link itself is copied.
    osl::DirectoryItem item;
    osl::FileStatus::Type type;
    if (!getType(*this, CommandId, srcUnqPath, &item, &type)) {
        return;
    }
    OUString rslvdSrcUnqPath;
    if (type == osl::FileStatus::Link) {
        osl::FileStatus stat(osl_FileStatus_Mask_LinkTargetURL);
        nError = item.getFileStatus(stat);
        if (nError != osl::FileBase::E_None) {
            installError(
                CommandId, TASKHANDLING_TRANSFER_BY_COPY_SOURCESTAT, nError);
            return;
        }
        rslvdSrcUnqPath = stat.getLinkTargetURL();
        if (!getType(*this, CommandId, srcUnqPath, &item, &type)) {
            return;
        }
    } else {
        rslvdSrcUnqPath = srcUnqPath;
    }

    bool isDocument
        = type != osl::FileStatus::Directory && type != osl::FileStatus::Volume;
    FileUrlType IsWhat = isDocument ? FileUrlType::File : FileUrlType::Folder;

    switch( NameClash )
    {
        case NameClash::KEEP:
        {
            nError = copy_recursive( rslvdSrcUnqPath,dstUnqPath,IsWhat,true );
            if( nError != osl::FileBase::E_None && nError != osl::FileBase::E_EXIST )
            {
                installError( CommandId,
                              TASKHANDLING_KEEPERROR_FOR_COPY,
                              nError );
                return;
            }
            break;
        }
        case NameClash::OVERWRITE:
        {
            // remove (..., MustExist = sal_False).
            remove( CommandId, dstUnqPath, IsWhat, false );

            // copy.
            nError = copy_recursive( rslvdSrcUnqPath,dstUnqPath,IsWhat,false );
            if( nError != osl::FileBase::E_None )
            {
                installError( CommandId,
                              TASKHANDLING_OVERWRITE_FOR_COPY,
                              nError );
                return;
            }
            break;
        }
        case NameClash::RENAME:
        {
            OUString newDstUnqPath = dstUnqPath;
            nError = copy_recursive( rslvdSrcUnqPath,dstUnqPath,IsWhat,true );

            if( nError == osl::FileBase::E_EXIST )
            {
                // "invent" a new valid title.

                sal_Int32 nPos = -1;
                sal_Int32 nLastDot = dstUnqPath.lastIndexOf( '.' );
                sal_Int32 nLastSlash = dstUnqPath.lastIndexOf( '/' );
                if ( ( nLastSlash < nLastDot ) // dot is part of last(!) path segment
                     && ( nLastSlash != ( nLastDot - 1 ) ) ) // file name does not start with a dot
                    nPos = nLastDot;
                else
                    nPos = dstUnqPath.getLength();

                sal_Int32 nTry = 0;

                do
                {
                    newDstUnqPath = dstUnqPath;

                    OUString aPostfix =  "_" + OUString::number( ++nTry );

                    newDstUnqPath = newDstUnqPath.replaceAt( nPos, 0, aPostfix );

                    nError = copy_recursive( rslvdSrcUnqPath,newDstUnqPath,IsWhat,true );
                }
                while( ( nError == osl::FileBase::E_EXIST ) && ( nTry < 10000 ) );
            }

            if( nError == osl::FileBase::E_EXIST )
            {
                installError( CommandId,
                              TASKHANDLING_RENAME_FOR_COPY );
                return;
            }
            else if( nError != osl::FileBase::E_None )
            {
                installError( CommandId,
                              TASKHANDLING_RENAMEMOVE_FOR_COPY,
                              nError );
                return;
            }
            else
                dstUnqPath = newDstUnqPath;

            break;
        }
        case NameClash::ERROR:
        {
            nError = copy_recursive( rslvdSrcUnqPath,dstUnqPath,IsWhat,true );

            if( nError == osl::FileBase::E_EXIST )
            {
                installError( CommandId,
                              TASKHANDLING_NAMECLASH_FOR_COPY );
                return;
            }
            else if( nError != osl::FileBase::E_None )
            {
                installError( CommandId,
                              TASKHANDLING_NAMECLASHMOVE_FOR_COPY,
                              nError );
                return;
            }
            break;
        }
        case NameClash::ASK:
        default:
        {
            nError = copy_recursive( rslvdSrcUnqPath,dstUnqPath,IsWhat,true );

            if( nError == osl::FileBase::E_EXIST )
            {
                installError( CommandId,
                              TASKHANDLING_NAMECLASHSUPPORT_FOR_COPY,
                              NameClash);
                return;
            }
            break;
        }
    }

    copyPersistentSet( srcUnqPath,dstUnqPath, !isDocument );
    notifyInsert( getContentEventListeners( getParentName( dstUnqPath ) ),dstUnqPath );
}


/********************************************************************************/
/*                                                                              */
/*                         remove-implementation                                */
/*                                                                              */
/********************************************************************************/

//  Deletes the content belonging to fileURL aUnqPath( recursively in case of directory )
//  Return: success of operation


bool
TaskManager::remove( sal_Int32 CommandId,
               const OUString& aUnqPath,
               FileUrlType IsWhat,
               bool  MustExist )
{
    sal_Int32 nMask = osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileURL;

    osl::DirectoryItem aItem;
    osl::FileStatus aStatus( nMask );
    osl::FileBase::RC nError;

    if( IsWhat == FileUrlType::Unknown ) // Determine whether we are removing a directory or a file
    {
        nError = osl::DirectoryItem::get( aUnqPath, aItem );
        if( nError != osl::FileBase::E_None )
        {
            if (MustExist)
            {
                installError( CommandId,
                              TASKHANDLING_NOSUCHFILEORDIR_FOR_REMOVE,
                              nError );
            }
            return (!MustExist);
        }

        nError = aItem.getFileStatus( aStatus );
        if( nError != osl::FileBase::E_None || ! aStatus.isValid( nMask ) )
        {
            installError( CommandId,
                          TASKHANDLING_VALIDFILESTATUS_FOR_REMOVE,
                          nError != osl::FileBase::E_None ? nError : TASKHANDLER_NO_ERROR );
            return false;
        }

        if( aStatus.getFileType() == osl::FileStatus::Regular ||
            aStatus.getFileType() == osl::FileStatus::Link )
            IsWhat = FileUrlType::File;
        else if(  aStatus.getFileType() == osl::FileStatus::Directory ||
                  aStatus.getFileType() == osl::FileStatus::Volume )
            IsWhat = FileUrlType::Folder;
    }


    if( IsWhat == FileUrlType::File )
    {
        nError = osl::File::remove( aUnqPath );
        if( nError != osl::FileBase::E_None )
        {
            if (MustExist)
            {
                installError( CommandId,
                              TASKHANDLING_DELETEFILE_FOR_REMOVE,
                              nError );
            }
            return (!MustExist);
        }
        else
        {
            notifyContentDeleted( getContentDeletedEventListeners(aUnqPath) );
            erasePersistentSet( aUnqPath ); // Removes from XPersistentPropertySet
        }
    }
    else if( IsWhat == FileUrlType::Folder )
    {
        osl::Directory aDirectory( aUnqPath );

        nError = aDirectory.open();
        if( nError != osl::FileBase::E_None )
        {
            if (MustExist)
            {
                installError( CommandId,
                              TASKHANDLING_OPENDIRECTORY_FOR_REMOVE,
                              nError );
            }
            return (!MustExist);
        }

        bool whileSuccess = true;
        FileUrlType recurse = FileUrlType::Unknown;
        OUString name;

        nError = aDirectory.getNextItem( aItem );
        while( nError == osl::FileBase::E_None )
        {
            nError = aItem.getFileStatus( aStatus );
            if( nError != osl::FileBase::E_None || ! aStatus.isValid( nMask ) )
            {
                installError( CommandId,
                              TASKHANDLING_VALIDFILESTATUSWHILE_FOR_REMOVE,
                              nError != osl::FileBase::E_None ? nError : TASKHANDLER_NO_ERROR );
                whileSuccess = false;
                break;
            }

            if( aStatus.getFileType() == osl::FileStatus::Regular ||
                aStatus.getFileType() == osl::FileStatus::Link )
                recurse = FileUrlType::File;
            else if( aStatus.getFileType() == osl::FileStatus::Directory ||
                     aStatus.getFileType() == osl::FileStatus::Volume )
                recurse = FileUrlType::Folder;

            name = aStatus.getFileURL();
            whileSuccess = remove( CommandId, name, recurse, MustExist );
            if( !whileSuccess )
                break;

            nError = aDirectory.getNextItem( aItem );
        }

        aDirectory.close();

        if( ! whileSuccess )
            return false;     // error code is installed

        if( nError != osl::FileBase::E_NOENT )
        {
            installError( CommandId,
                          TASKHANDLING_DIRECTORYEXHAUSTED_FOR_REMOVE,
                          nError );
            return false;
        }

        nError = osl::Directory::remove( aUnqPath );
        if( nError != osl::FileBase::E_None )
        {
            if (MustExist)
            {
                installError( CommandId,
                              TASKHANDLING_DELETEDIRECTORY_FOR_REMOVE,
                              nError );
            }
            return (!MustExist);
        }
        else
        {
            notifyContentDeleted( getContentDeletedEventListeners(aUnqPath) );
            erasePersistentSet( aUnqPath );
        }
    }
    else   // Don't know what to remove
    {
        installError( CommandId,
                      TASKHANDLING_FILETYPE_FOR_REMOVE );
        return false;
    }

    return true;
}


/********************************************************************************/
/*                                                                              */
/*                         mkdir-implementation                                 */
/*                                                                              */
/********************************************************************************/

//  Creates new directory with given URL, recursively if necessary
//  Return:: success of operation


bool
TaskManager::mkdir( sal_Int32 CommandId,
              const OUString& rUnqPath,
              bool OverWrite )
{
    OUString aUnqPath;

    // remove trailing slash
    if ( rUnqPath.endsWith("/") )
        aUnqPath = rUnqPath.copy( 0, rUnqPath.getLength() - 1 );
    else
        aUnqPath = rUnqPath;

    osl::FileBase::RC nError = osl::Directory::create( aUnqPath );

    switch ( nError )
    {
        case osl::FileBase::E_EXIST:   // Directory cannot be overwritten
        {
            if( !OverWrite )
            {
                installError( CommandId,
                              TASKHANDLING_FOLDER_EXISTS_MKDIR );
                return false;
            }
            else
                return true;
        }
        case osl::FileBase::E_INVAL:
        {
            installError(CommandId,
                         TASKHANDLING_INVALID_NAME_MKDIR);
            return false;
        }
        case osl::FileBase::E_None:
        {
            OUString aPrtPath = getParentName( aUnqPath );
            notifyInsert( getContentEventListeners( aPrtPath ),aUnqPath );
            return true;
        }
        default:
            return ensuredir(
                CommandId,
                aUnqPath,
                TASKHANDLING_CREATEDIRECTORY_MKDIR );
    }
}


/********************************************************************************/
/*                                                                              */
/*                         mkfil-implementation                                 */
/*                                                                              */
/********************************************************************************/

//  Creates new file with given URL.
//  The content of aInputStream becomes the content of the file
//  Return:: success of operation


bool
TaskManager::mkfil( sal_Int32 CommandId,
              const OUString& aUnqPath,
              bool Overwrite,
              const uno::Reference< io::XInputStream >& aInputStream )
{
    // return value unimportant
    bool bSuccess = write( CommandId,
                               aUnqPath,
                               Overwrite,
                               aInputStream );
    if ( bSuccess )
    {
        OUString aPrtPath = getParentName( aUnqPath );
        notifyInsert( getContentEventListeners( aPrtPath ),aUnqPath );
    }
    return bSuccess;
}


/********************************************************************************/
/*                                                                              */
/*                         write-implementation                                 */
/*                                                                              */
/********************************************************************************/

//  writes to the file with given URL.
//  The content of aInputStream becomes the content of the file
//  Return:: success of operation


bool
TaskManager::write( sal_Int32 CommandId,
              const OUString& aUnqPath,
              bool OverWrite,
              const uno::Reference< io::XInputStream >& aInputStream )
{
    if( ! aInputStream.is() )
    {
        installError( CommandId,
                      TASKHANDLING_INPUTSTREAM_FOR_WRITE );
        return false;
    }

    // Create parent path, if necessary.
    if ( ! ensuredir( CommandId,
                      getParentName( aUnqPath ),
                      TASKHANDLING_ENSUREDIR_FOR_WRITE ) )
        return false;

    osl::FileBase::RC err;
    osl::File aFile( aUnqPath );

    if( OverWrite )
    {
        err = aFile.open( osl_File_OpenFlag_Write | osl_File_OpenFlag_Create );

        if( err != osl::FileBase::E_None )
        {
            aFile.close();
            err = aFile.open( osl_File_OpenFlag_Write );

            if( err != osl::FileBase::E_None )
            {
                installError( CommandId,
                              TASKHANDLING_NO_OPEN_FILE_FOR_OVERWRITE,
                              err );
                return false;
            }

            // the existing file was just opened and should be overwritten now,
            // truncate it first

            err = aFile.setSize( 0 );
            if( err != osl::FileBase::E_None  )
            {
                installError( CommandId,
                              TASKHANDLING_FILESIZE_FOR_WRITE,
                              err );
                return false;
            }
        }
    }
    else
    {
        err = aFile.open( osl_File_OpenFlag_Read | osl_File_OpenFlag_NoLock );
        if( err == osl::FileBase::E_None )  // The file exists and shall not be overwritten
        {
            installError( CommandId,
                          TASKHANDLING_NOREPLACE_FOR_WRITE,  // Now an exception
                          err );

            aFile.close();
            return false;
        }

        // as a temporary solution the creation does not lock the file at all
        // in future it should be possible to create the file without lock explicitly
        err = aFile.open( osl_File_OpenFlag_Write | osl_File_OpenFlag_Create | osl_File_OpenFlag_NoLock );

        if( err != osl::FileBase::E_None )
        {
            aFile.close();
            installError( CommandId,
                          TASKHANDLING_NO_OPEN_FILE_FOR_WRITE,
                          err );
            return false;
        }
    }

    bool bSuccess = true;

    sal_uInt64 nWrittenBytes;
    sal_Int32 nReadBytes = 0, nRequestedBytes = 32768 /*32k*/;
    uno::Sequence< sal_Int8 > seq( nRequestedBytes );

    do
    {
        try
        {
            nReadBytes = aInputStream->readBytes( seq,
                                                  nRequestedBytes );
        }
        catch( const io::NotConnectedException& )
        {
            installError( CommandId,
                          TASKHANDLING_NOTCONNECTED_FOR_WRITE );
            bSuccess = false;
            break;
        }
        catch( const io::BufferSizeExceededException& )
        {
            installError( CommandId,
                          TASKHANDLING_BUFFERSIZEEXCEEDED_FOR_WRITE );
            bSuccess = false;
            break;
        }
        catch( const io::IOException& )
        {
            installError( CommandId,
                          TASKHANDLING_IOEXCEPTION_FOR_WRITE );
            bSuccess = false;
            break;
        }

        if( nReadBytes )
        {
            const sal_Int8* p = seq.getConstArray();

            err = aFile.write( static_cast<void const *>(p),
                               sal_uInt64( nReadBytes ),
                               nWrittenBytes );

            if( err != osl::FileBase::E_None )
            {
                installError( CommandId,
                              TASKHANDLING_FILEIOERROR_FOR_WRITE,
                              err );
                bSuccess = false;
                break;
            }
            else if( nWrittenBytes != sal_uInt64( nReadBytes ) )
            {
                installError( CommandId,
                              TASKHANDLING_FILEIOERROR_FOR_NO_SPACE );
                bSuccess = false;
                break;
            }
        }
    } while( nReadBytes == nRequestedBytes );

    err = aFile.close();
    if( err != osl::FileBase::E_None  )
    {
        installError( CommandId,
                      TASKHANDLING_FILEIOERROR_FOR_WRITE,
                      err );
        bSuccess = false;
    }

    return bSuccess;
}


/*********************************************************************************/
/*                                                                               */
/*                 insertDefaultProperties-Implementation                        */
/*                                                                               */
/*********************************************************************************/


void TaskManager::insertDefaultProperties( const OUString& aUnqPath )
{
    osl::MutexGuard aGuard( m_aMutex );

    ContentMap::iterator it =
        m_aContent.emplace( aUnqPath,UnqPathData() ).first;

    load( it,false );

    MyProperty ContentTProperty( ContentType );

    PropertySet& properties = *(it->second.properties);
    bool ContentNotDefau = properties.find( ContentTProperty ) != properties.end();

    for (auto const& defaultprop : m_aDefaultProperties)
    {
        if( !ContentNotDefau || defaultprop.getPropertyName() != ContentType )
            properties.insert( defaultprop );
    }
}


/******************************************************************************/
/*                                                                            */
/*                          mapping of file urls                              */
/*                          to uncpath and vice versa                         */
/*                                                                            */
/******************************************************************************/


bool TaskManager::getUnqFromUrl( const OUString& Url, OUString& Unq )
{
    if ( Url == "file:///" || Url == "file://localhost/" || Url == "file://127.0.0.1/" )
    {
        Unq = "file:///";
        return false;
    }

    bool err = osl::FileBase::E_None != osl::FileBase::getSystemPathFromFileURL( Url,Unq );

    Unq = Url;

    sal_Int32 l = Unq.getLength()-1;
    if( ! err && Unq.endsWith("/") &&
        Unq.indexOf( '/', RTL_CONSTASCII_LENGTH("//") ) != -1 )
        Unq = Unq.copy(0, l);

    return err;
}


bool TaskManager::getUrlFromUnq( const OUString& Unq,OUString& Url )
{
    bool err = osl::FileBase::E_None != osl::FileBase::getSystemPathFromFileURL( Unq,Url );

    Url = Unq;

    return err;
}


// Helper function for public copy

osl::FileBase::RC
TaskManager::copy_recursive( const OUString& srcUnqPath,
                       const OUString& dstUnqPath,
                       FileUrlType TypeToCopy,
                       bool testExistBeforeCopy )
{
    osl::FileBase::RC err = osl::FileBase::E_None;

    if( TypeToCopy == FileUrlType::File ) // Document
    {
        err = osl_File_copy( srcUnqPath,dstUnqPath,testExistBeforeCopy );
    }
    else if( TypeToCopy == FileUrlType::Folder )
    {
        osl::Directory aDir( srcUnqPath );
        aDir.open();

        err = osl::Directory::create( dstUnqPath );
        osl::FileBase::RC next = err;
        if( err == osl::FileBase::E_None )
        {
            sal_Int32 const n_Mask = osl_FileStatus_Mask_FileURL | osl_FileStatus_Mask_FileName | osl_FileStatus_Mask_Type;

            osl::DirectoryItem aDirItem;

            while( err == osl::FileBase::E_None && ( next = aDir.getNextItem( aDirItem ) ) == osl::FileBase::E_None )
            {
                bool IsDoc = false;
                osl::FileStatus aFileStatus( n_Mask );
                aDirItem.getFileStatus( aFileStatus );
                if( aFileStatus.isValid( osl_FileStatus_Mask_Type ) )
                    IsDoc = aFileStatus.getFileType() == osl::FileStatus::Regular;

                // Getting the information for the next recursive copy
                FileUrlType newTypeToCopy = IsDoc ? FileUrlType::File : FileUrlType::Folder;

                OUString newSrcUnqPath;
                if( aFileStatus.isValid( osl_FileStatus_Mask_FileURL ) )
                    newSrcUnqPath = aFileStatus.getFileURL();

                OUString newDstUnqPath = dstUnqPath;
                OUString tit;
                if( aFileStatus.isValid( osl_FileStatus_Mask_FileName ) )
                    tit = rtl::Uri::encode( aFileStatus.getFileName(),
                                          rtl_UriCharClassPchar,
                                          rtl_UriEncodeIgnoreEscapes,
                                          RTL_TEXTENCODING_UTF8 );

                if( !newDstUnqPath.endsWith( "/" ) )
                    newDstUnqPath += "/";

                newDstUnqPath += tit;

                if ( newSrcUnqPath != dstUnqPath )
                    err = copy_recursive( newSrcUnqPath,newDstUnqPath,newTypeToCopy,false );
            }

            if( err == osl::FileBase::E_None && next != osl::FileBase::E_NOENT )
                err = next;
        }
        aDir.close();
    }

    return err;
}


// Helper function for mkfil,mkdir and write
// Creates whole path
// returns success of the operation


bool TaskManager::ensuredir( sal_Int32 CommandId,
                                    const OUString& rUnqPath,
                                    sal_Int32 errorCode )
{
    OUString aPath;

    if ( rUnqPath.isEmpty() )
        return false;

    if ( rUnqPath.endsWith("/") )
        aPath = rUnqPath.copy( 0, rUnqPath.getLength() - 1 );
    else
        aPath = rUnqPath;

#if HAVE_FEATURE_MACOSX_SANDBOX

    // Avoid annoying sandbox messages in the system.log from the
    // below aDirectory.open(), which ends up calling opendir().
    // Surely it is easier to just call stat()? Calling stat() on an
    // arbitrary (?) directory does not seem to cause any sandbox
    // violation, while opendir() does. (Sorry I could not be bothered
    // to use some complex cross-platform abstraction over stat() here
    // in this macOS specific code block.)

    OUString aDirName;
    struct stat s;
    if( osl::FileBase::getSystemPathFromFileURL( aPath, aDirName ) == osl::FileBase::E_None &&
        stat(OUStringToOString( aDirName, RTL_TEXTENCODING_UTF8).getStr(), &s ) == 0 &&
        S_ISDIR( s.st_mode ) )
        return sal_True;
#endif

    // HACK: create directory on a mount point with nobrowse option
    // returns ENOSYS in any case !!
    osl::Directory aDirectory( aPath );
    osl::FileBase::RC nError = aDirectory.open();
    aDirectory.close();

    if( nError == osl::File::E_None )
        return true;

    nError = osl::Directory::create( aPath );

    if( nError == osl::File::E_None )
        notifyInsert( getContentEventListeners( getParentName( aPath ) ),aPath );

    bool  bSuccess = ( nError == osl::File::E_None || nError == osl::FileBase::E_EXIST );

    if( ! bSuccess )
    {
        OUString aParentDir = getParentName( aPath );

        if ( aParentDir != aPath )
        {   // Create first the parent directory
            bSuccess = ensuredir( CommandId,
                                  getParentName( aPath ),
                                  errorCode );

            // After parent directory structure exists try it one's more

            if ( bSuccess )
            {   // Parent directory exists, retry creation of directory
                nError = osl::Directory::create( aPath );

                if( nError == osl::File::E_None )
                    notifyInsert( getContentEventListeners( getParentName( aPath ) ),aPath );

                bSuccess =( nError == osl::File::E_None || nError == osl::FileBase::E_EXIST );
            }
        }
    }

    if( ! bSuccess )
        installError( CommandId,
                      errorCode,
                      nError );

    return bSuccess;
}


//  Given a sequence of properties seq, this method determines the mask
//  used to instantiate a osl::FileStatus, so that a call to
//  osl::DirectoryItem::getFileStatus fills the required fields.


void
TaskManager::getMaskFromProperties(
    sal_Int32& n_Mask,
    const uno::Sequence< beans::Property >& seq )
{
    n_Mask = 0;
    for(sal_Int32 j = 0; j < seq.getLength(); ++j) {
        if(seq[j].Name == Title)
            n_Mask |= osl_FileStatus_Mask_FileName;
        else if(seq[j].Name == CasePreservingURL)
            n_Mask |= osl_FileStatus_Mask_FileURL;
        else if(seq[j].Name == IsDocument ||
                seq[j].Name == IsFolder ||
                seq[j].Name == IsVolume ||
                seq[j].Name == IsRemoveable ||
                seq[j].Name == IsRemote ||
                seq[j].Name == IsCompactDisc ||
                seq[j].Name == IsFloppy ||
                seq[j].Name == ContentType)
            n_Mask |= (osl_FileStatus_Mask_Type | osl_FileStatus_Mask_LinkTargetURL);
        else if(seq[j].Name == Size)
            n_Mask |= (osl_FileStatus_Mask_FileSize |
                      osl_FileStatus_Mask_Type |
                      osl_FileStatus_Mask_LinkTargetURL);
        else if(seq[j].Name == IsHidden ||
                seq[j].Name == IsReadOnly)
            n_Mask |= osl_FileStatus_Mask_Attributes;
        else if(seq[j].Name == DateModified)
            n_Mask |= osl_FileStatus_Mask_ModifyTime;
    }
}


/*********************************************************************************/
/*                                                                               */
/*                     load-Implementation                                       */
/*                                                                               */
/*********************************************************************************/

//  Load the properties from configuration, if create == true create them.
//  The Properties are stored under the url belonging to it->first.


void
TaskManager::load( const ContentMap::iterator& it, bool create )
{
    if( ! it->second.properties )
        it->second.properties.reset( new PropertySet );

    if( ( ! it->second.xS.is() ||
          ! it->second.xC.is() ||
          ! it->second.xA.is() )
        && m_xFileRegistry.is() )
    {

        uno::Reference< ucb::XPersistentPropertySet > xS = m_xFileRegistry->openPropertySet( it->first,create );
        if( xS.is() )
        {
            uno::Reference< beans::XPropertyContainer > xC( xS,uno::UNO_QUERY );
            uno::Reference< beans::XPropertyAccess >    xA( xS,uno::UNO_QUERY );

            it->second.xS = xS;
            it->second.xC = xC;
            it->second.xA = xA;

            // Now put in all values in the storage in the local hash;

            PropertySet& properties = *(it->second.properties);
            uno::Sequence< beans::Property > seq = xS->getPropertySetInfo()->getProperties();

            for( sal_Int32 i = 0; i < seq.getLength(); ++i )
            {
                MyProperty readProp( false,
                                     seq[i].Name,
                                     seq[i].Handle,
                                     seq[i].Type,
                                     xS->getPropertyValue( seq[i].Name ),
                                     beans::PropertyState_DIRECT_VALUE,
                                     seq[i].Attributes );
                if( properties.find( readProp ) == properties.end() )
                    properties.insert( readProp );
            }
        }
        else if( create )
        {
            // Catastrophic error
        }
    }
}


/*********************************************************************************/
/*                                                                               */
/*                     commit-Implementation                                     */
/*                                                                               */
/*********************************************************************************/
// Commit inserts the determined properties in the filestatus object into
// the internal map, so that is possible to determine on a subsequent
// setting of file properties which properties have changed without filestat


void
TaskManager::commit( const TaskManager::ContentMap::iterator& it,
               const osl::FileStatus& aFileStatus )
{
    TaskManager::PropertySet::iterator it1;

    if( it->second.properties == nullptr )
    {
        OUString aPath = it->first;
        insertDefaultProperties( aPath );
    }

    PropertySet& properties = *( it->second.properties );

    it1 = properties.find( MyProperty( Title ) );
    if( it1 != properties.end() )
    {
        if( aFileStatus.isValid( osl_FileStatus_Mask_FileName ) )
        {
            it1->setValue( uno::Any(aFileStatus.getFileName()) );
        }
    }

    it1 = properties.find( MyProperty( CasePreservingURL ) );
    if( it1 != properties.end() )
    {
        if( aFileStatus.isValid( osl_FileStatus_Mask_FileURL ) )
        {
            it1->setValue( uno::Any(aFileStatus.getFileURL()) );
        }
    }


    bool isDirectory,isFile,isVolume,isRemoveable,isRemote,isFloppy,isCompactDisc;

    sal_Int64 dirSize = 0;

    if( aFileStatus.isValid( osl_FileStatus_Mask_FileSize ) )
        dirSize = aFileStatus.getFileSize();

    if( aFileStatus.isValid( osl_FileStatus_Mask_Type ) )
    {
        if( osl::FileStatus::Link == aFileStatus.getFileType() &&
            aFileStatus.isValid( osl_FileStatus_Mask_LinkTargetURL ) )
        {
            osl::DirectoryItem aDirItem;
            osl::FileStatus aFileStatus2( osl_FileStatus_Mask_Type );
            if( osl::FileBase::E_None == osl::DirectoryItem::get( aFileStatus.getLinkTargetURL(),aDirItem ) &&
                osl::FileBase::E_None == aDirItem.getFileStatus( aFileStatus2 )    &&
                aFileStatus2.isValid( osl_FileStatus_Mask_Type ) )
            {
                isVolume = osl::FileStatus::Volume == aFileStatus2.getFileType();
                isDirectory =
                    osl::FileStatus::Volume == aFileStatus2.getFileType() ||
                    osl::FileStatus::Directory == aFileStatus2.getFileType();
                isFile =
                    osl::FileStatus::Regular == aFileStatus2.getFileType();

                if( aFileStatus2.isValid( osl_FileStatus_Mask_FileSize ) )
                    dirSize = aFileStatus2.getFileSize();
            }
            else
            {
                // extremely ugly, but otherwise default construction
                // of aDirItem and aFileStatus2
                // before the preceding if
                isVolume = osl::FileStatus::Volume == aFileStatus.getFileType();
                isDirectory =
                    osl::FileStatus::Volume == aFileStatus.getFileType() ||
                    osl::FileStatus::Directory == aFileStatus.getFileType();
                isFile =
                    osl::FileStatus::Regular == aFileStatus.getFileType();
            }
        }
        else
        {
            isVolume = osl::FileStatus::Volume == aFileStatus.getFileType();
            isDirectory =
                osl::FileStatus::Volume == aFileStatus.getFileType() ||
                osl::FileStatus::Directory == aFileStatus.getFileType();
            isFile =
                osl::FileStatus::Regular == aFileStatus.getFileType();
        }

        it1 = properties.find( MyProperty( IsVolume ) );
        if( it1 != properties.end() )
            it1->setValue( uno::makeAny( isVolume ) );

        it1 = properties.find( MyProperty( IsFolder ) );
        if( it1 != properties.end() )
            it1->setValue( uno::makeAny( isDirectory ) );

        it1 = properties.find( MyProperty( IsDocument ) );
        if( it1 != properties.end() )
            it1->setValue( uno::makeAny( isFile ) );

        osl::VolumeInfo aVolumeInfo( osl_VolumeInfo_Mask_Attributes );
        if( isVolume &&
            osl::FileBase::E_None == osl::Directory::getVolumeInfo( it->first,aVolumeInfo ) &&
            aVolumeInfo.isValid( osl_VolumeInfo_Mask_Attributes ) )
        {
            // Retrieve the flags;
            isRemote = aVolumeInfo.getRemoteFlag();
            isRemoveable = aVolumeInfo.getRemoveableFlag();
            isCompactDisc = aVolumeInfo.getCompactDiscFlag();
            isFloppy = aVolumeInfo.getFloppyDiskFlag();

            it1 = properties.find( MyProperty( IsRemote ) );
            if( it1 != properties.end() )
                it1->setValue( uno::makeAny( isRemote ) );

            it1 = properties.find( MyProperty( IsRemoveable ) );
            if( it1 != properties.end() )
                it1->setValue( uno::makeAny( isRemoveable ) );

            it1 = properties.find( MyProperty( IsCompactDisc ) );
            if( it1 != properties.end() )
                it1->setValue( uno::makeAny( isCompactDisc ) );

            it1 = properties.find( MyProperty( IsFloppy ) );
            if( it1 != properties.end() )
                it1->setValue( uno::makeAny( isFloppy ) );
        }
        else
        {
            uno::Any aAny(false);
            it1 = properties.find( MyProperty( IsRemote ) );
            if( it1 != properties.end() )
                it1->setValue( aAny );

            it1 = properties.find( MyProperty( IsRemoveable ) );
            if( it1 != properties.end() )
                it1->setValue( aAny );

            it1 = properties.find( MyProperty( IsCompactDisc ) );
            if( it1 != properties.end() )
                it1->setValue( aAny );

            it1 = properties.find( MyProperty( IsFloppy ) );
            if( it1 != properties.end() )
                it1->setValue( aAny );
        }
    }
    else
    {
        isDirectory = false;
    }

    it1 = properties.find( MyProperty( Size ) );
    if( it1 != properties.end() )
        it1->setValue( uno::makeAny( dirSize ) );

    it1 = properties.find( MyProperty( IsReadOnly ) );
    if( it1 != properties.end() )
    {
        if( aFileStatus.isValid( osl_FileStatus_Mask_Attributes ) )
        {
            sal_uInt64 Attr = aFileStatus.getAttributes();
            bool readonly = ( Attr & osl_File_Attribute_ReadOnly ) != 0;
            it1->setValue( uno::makeAny( readonly ) );
        }
    }

    it1 = properties.find( MyProperty( IsHidden ) );
    if( it1 != properties.end() )
    {
        if( aFileStatus.isValid( osl_FileStatus_Mask_Attributes ) )
        {
            sal_uInt64 Attr = aFileStatus.getAttributes();
            bool ishidden = ( Attr & osl_File_Attribute_Hidden ) != 0;
            it1->setValue( uno::makeAny( ishidden ) );
        }
    }

    it1 = properties.find( MyProperty( DateModified ) );
    if( it1 != properties.end() )
    {
        if( aFileStatus.isValid( osl_FileStatus_Mask_ModifyTime ) )
        {
            TimeValue temp = aFileStatus.getModifyTime();

            // Convert system time to local time (for EA)
            TimeValue myLocalTime;
            if (!osl_getLocalTimeFromSystemTime( &temp, &myLocalTime ))
            {
                SAL_WARN(
                    "ucb.ucp.file",
                    "cannot convert (" << temp.Seconds << ", " << temp.Nanosec
                        << ") to local time");
                myLocalTime = temp;
            }

            oslDateTime myDateTime;
            osl_getDateTimeFromTimeValue( &myLocalTime, &myDateTime );
            util::DateTime aDateTime;

            aDateTime.NanoSeconds = myDateTime.NanoSeconds;
            aDateTime.Seconds = myDateTime.Seconds;
            aDateTime.Minutes = myDateTime.Minutes;
            aDateTime.Hours = myDateTime.Hours;
            aDateTime.Day = myDateTime.Day;
            aDateTime.Month = myDateTime.Month;
            aDateTime.Year = myDateTime.Year;
            it1->setValue( uno::makeAny( aDateTime ) );
        }
    }

    it1 = properties.find( MyProperty( CreatableContentsInfo ) );
    if( it1 != properties.end() )
        it1->setValue( uno::makeAny(
            isDirectory || !aFileStatus.isValid( osl_FileStatus_Mask_Type )
                ? queryCreatableContentsInfo()
                : uno::Sequence< ucb::ContentInfo >() ) );
}


// Special optimized method for getting the properties of a
// directoryitem, which is returned by osl::DirectoryItem::getNextItem()


bool
TaskManager::getv(
    Notifier* pNotifier,
    const uno::Sequence< beans::Property >& properties,
    osl::DirectoryItem& aDirItem,
    OUString& aUnqPath,
    bool& aIsRegular,
    uno::Reference< sdbc::XRow > & row )
{
    uno::Sequence< uno::Any > seq( properties.getLength() );

    sal_Int32 n_Mask;
    getMaskFromProperties( n_Mask,properties );

    // Always retrieve the type and the target URL because item might be a link
    osl::FileStatus aFileStatus( n_Mask |
                                 osl_FileStatus_Mask_FileURL |
                                 osl_FileStatus_Mask_Type |
                                 osl_FileStatus_Mask_LinkTargetURL );

    osl::FileBase::RC aRes = aDirItem.getFileStatus( aFileStatus );
    if ( aRes != osl::FileBase::E_None )
    {
        SAL_WARN(
            "ucb.ucp.file",
            "osl::DirectoryItem::getFileStatus failed with " << +aRes);
        return false;
    }

    aUnqPath = aFileStatus.getFileURL();

    // If the directory item type is a link retrieve the type of the target

    if ( aFileStatus.getFileType() == osl::FileStatus::Link )
    {
        // Assume failure
        aIsRegular = false;
        osl::DirectoryItem aTargetItem;
        osl::DirectoryItem::get( aFileStatus.getLinkTargetURL(), aTargetItem );
        if ( aTargetItem.is() )
        {
            osl::FileStatus aTargetStatus( osl_FileStatus_Mask_Type );

            if ( osl::FileBase::E_None == aTargetItem.getFileStatus( aTargetStatus ) )
                aIsRegular =
                    aTargetStatus.getFileType() == osl::FileStatus::Regular;
        }
    }
    else
        aIsRegular = aFileStatus.getFileType() == osl::FileStatus::Regular;

    registerNotifier( aUnqPath,pNotifier );
    insertDefaultProperties( aUnqPath );
    {
        osl::MutexGuard aGuard( m_aMutex );

        TaskManager::ContentMap::iterator it = m_aContent.find( aUnqPath );
        commit( it,aFileStatus );

        TaskManager::PropertySet::iterator it1;
        PropertySet& propset = *(it->second.properties);

        for( sal_Int32 i = 0; i < seq.getLength(); ++i )
        {
            MyProperty readProp( properties[i].Name );
            it1 = propset.find( readProp );
            if( it1 == propset.end() )
                seq[i] = uno::Any();
            else
                seq[i] = it1->getValue();
        }
    }
    deregisterNotifier( aUnqPath,pNotifier );

    XRow_impl* p = new XRow_impl( this,seq );
    row = uno::Reference< sdbc::XRow >( p );
    return true;
}


// EventListener


std::vector< std::unique_ptr< ContentEventNotifier > >
TaskManager::getContentEventListeners( const OUString& aName )
{
    std::vector< std::unique_ptr<ContentEventNotifier> > listeners;
    {
        osl::MutexGuard aGuard( m_aMutex );
        TaskManager::ContentMap::iterator it = m_aContent.find( aName );
        if( it != m_aContent.end() && it->second.notifier )
        {
            std::vector<Notifier*>& listOfNotifiers = *( it->second.notifier );
            for (auto const& pointer : listOfNotifiers)
            {
                std::unique_ptr<ContentEventNotifier> notifier = pointer->cCEL();
                if( notifier )
                    listeners.push_back( std::move(notifier) );
            }
        }
    }
    return listeners;
}


std::vector< std::unique_ptr<ContentEventNotifier> >
TaskManager::getContentDeletedEventListeners( const OUString& aName )
{
    std::vector< std::unique_ptr< ContentEventNotifier > > listeners;
    {
        osl::MutexGuard aGuard( m_aMutex );
        TaskManager::ContentMap::iterator it = m_aContent.find( aName );
        if( it != m_aContent.end() && it->second.notifier )
        {
            std::vector<Notifier*>& listOfNotifiers = *( it->second.notifier );
            for (auto const& pointer : listOfNotifiers)
            {
                std::unique_ptr<ContentEventNotifier> notifier = pointer->cDEL();
                if( notifier )
                    listeners.push_back( std::move(notifier) );
            }
        }
    }
    return listeners;
}

void TaskManager::notifyInsert(const std::vector<std::unique_ptr<ContentEventNotifier>>& listeners,
                               const OUString& aChildName)
{
    for (const auto & l : listeners )
    {
        l->notifyChildInserted( aChildName );
    }
}

void TaskManager::notifyContentDeleted(
    const std::vector<std::unique_ptr<ContentEventNotifier>>& listeners)
{
    for( auto const & l : listeners )
    {
        l->notifyDeleted();
    }
}

void TaskManager::notifyContentRemoved(
    const std::vector<std::unique_ptr<ContentEventNotifier>>& listeners, const OUString& aChildName)
{
    for( auto const & l : listeners )
    {
        l->notifyRemoved( aChildName );
    }
}


std::vector< std::unique_ptr< PropertySetInfoChangeNotifier > >
TaskManager::getPropertySetListeners( const OUString& aName )
{
    std::vector< std::unique_ptr< PropertySetInfoChangeNotifier > > listeners;
    {
        osl::MutexGuard aGuard( m_aMutex );
        TaskManager::ContentMap::iterator it = m_aContent.find( aName );
        if( it != m_aContent.end() && it->second.notifier )
        {
            std::vector<Notifier*>& listOfNotifiers = *( it->second.notifier );
            for (auto const& pointer : listOfNotifiers)
            {
                std::unique_ptr<PropertySetInfoChangeNotifier> notifier = pointer->cPSL();
                if( notifier )
                    listeners.push_back( std::move(notifier) );
            }
        }
    }
    return listeners;
}

void TaskManager::notifyPropertyAdded(
    const std::vector<std::unique_ptr<PropertySetInfoChangeNotifier>>& listeners,
    const OUString& aPropertyName)
{
    for( auto const & l : listeners )
    {
        l->notifyPropertyAdded( aPropertyName );
    }
}

void TaskManager::notifyPropertyRemoved(
    const std::vector<std::unique_ptr<PropertySetInfoChangeNotifier>>& listeners,
    const OUString& aPropertyName)
{
    for( auto const & l : listeners )
    {
        l->notifyPropertyRemoved( aPropertyName );
    }
}


std::vector< std::unique_ptr< ContentEventNotifier > >
TaskManager::getContentExchangedEventListeners( const OUString& aOldPrefix,
                                          const OUString& aNewPrefix,
                                          bool withChildren )
{
    std::vector< std::unique_ptr< ContentEventNotifier > > aVector;

    sal_Int32 count;
    OUString aOldName;
    OUString aNewName;
    std::vector< OUString > oldChildList;

    {
        osl::MutexGuard aGuard( m_aMutex );

        if( ! withChildren )
        {
            aOldName = aOldPrefix;
            aNewName = aNewPrefix;
            count = 1;
        }
        else
        {
            for (auto const& content : m_aContent)
            {
                if( isChild( aOldPrefix,content.first ) )
                {
                    oldChildList.push_back( content.first );
                }
            }
            count = oldChildList.size();
        }


        for( sal_Int32 j = 0; j < count; ++j )
        {
            if( withChildren )
            {
                aOldName = oldChildList[j];
                aNewName = newName( aNewPrefix,aOldPrefix,aOldName );
            }

            TaskManager::ContentMap::iterator itold = m_aContent.find( aOldName );
            if( itold != m_aContent.end() )
            {
                TaskManager::ContentMap::iterator itnew = m_aContent.emplace(
                    aNewName,UnqPathData() ).first;

                // copy Ownership also
                itnew->second.properties = std::move(itold->second.properties);

                // copy existing list
                std::unique_ptr<std::vector< Notifier* >> copyList = std::move(itnew->second.notifier);
                itnew->second.notifier = std::move(itold->second.notifier);

                m_aContent.erase( itold );

                if( itnew != m_aContent.end() && itnew->second.notifier )
                {
                    std::vector<Notifier*>& listOfNotifiers = *( itnew->second.notifier );
                    for (auto const& pointer : listOfNotifiers)
                    {
                        std::unique_ptr<ContentEventNotifier> notifier = pointer->cEXC( aNewName );
                        if( notifier )
                            aVector.push_back( std::move(notifier) );
                    }
                }

                // Merge with preexisting notifiers
                // However, these may be in status BaseContent::Deleted
                if( copyList != nullptr )
                {
                    for( const auto& rCopyPtr : *copyList )
                    {
                        itnew->second.notifier->push_back( rCopyPtr );
                    }
                }
            }
        }
    }

    return aVector;
}

void TaskManager::notifyContentExchanged(
    const std::vector<std::unique_ptr<ContentEventNotifier>>& listeners_vec)
{
    for( auto & l : listeners_vec)
    {
        l->notifyExchanged();
    }
}


std::vector< std::unique_ptr<PropertyChangeNotifier> >
TaskManager::getPropertyChangeNotifier( const OUString& aName )
{
    std::vector< std::unique_ptr<PropertyChangeNotifier> > listeners;
    {
        osl::MutexGuard aGuard( m_aMutex );
        TaskManager::ContentMap::iterator it = m_aContent.find( aName );
        if( it != m_aContent.end() && it->second.notifier )
        {
            std::vector<Notifier*>& listOfNotifiers = *( it->second.notifier );
            for (auto const& pointer : listOfNotifiers)
            {
                std::unique_ptr<PropertyChangeNotifier> notifier = pointer->cPCL();
                if( notifier )
                    listeners.push_back( std::move(notifier) );
            }
        }
    }
    return listeners;
}

void TaskManager::notifyPropertyChanges(
    const std::vector<std::unique_ptr<PropertyChangeNotifier>>& listeners,
    const uno::Sequence<beans::PropertyChangeEvent>& seqChanged)
{
    for( auto const & l : listeners )
    {
        l->notifyPropertyChanged( seqChanged );
    }
}


/********************************************************************************/
/*                       remove persistent propertyset                          */
/********************************************************************************/

void
TaskManager::erasePersistentSet( const OUString& aUnqPath,
                           bool withChildren )
{
    if( ! m_xFileRegistry.is() )
    {
        OSL_ASSERT( m_xFileRegistry.is() );
        return;
    }

    uno::Sequence< OUString > seqNames;

    if( withChildren )
    {
        uno::Reference< container::XNameAccess > xName( m_xFileRegistry,uno::UNO_QUERY );
        seqNames = xName->getElementNames();
    }

    sal_Int32 count = withChildren ? seqNames.getLength() : 1;

    OUString
        old_Name = aUnqPath;

    for( sal_Int32 j = 0; j < count; ++j )
    {
        if( withChildren  && ! ( isChild( old_Name,seqNames[j] ) ) )
            continue;

        if( withChildren )
        {
            old_Name = seqNames[j];
        }

        {
            // Release possible references
            osl::MutexGuard aGuard( m_aMutex );
            ContentMap::iterator it = m_aContent.find( old_Name );
            if( it != m_aContent.end() )
            {
                it->second.xS = nullptr;
                it->second.xC = nullptr;
                it->second.xA = nullptr;

                it->second.properties.reset();
            }
        }

        if( m_xFileRegistry.is() )
            m_xFileRegistry->removePropertySet( old_Name );
    }
}


/********************************************************************************/
/*                       copy persistent propertyset                            */
/*                       from srcUnqPath to dstUnqPath                          */
/********************************************************************************/


void
TaskManager::copyPersistentSet( const OUString& srcUnqPath,
                          const OUString& dstUnqPath,
                          bool withChildren )
{
    if( ! m_xFileRegistry.is() )
    {
        OSL_ASSERT( m_xFileRegistry.is() );
        return;
    }

    uno::Sequence< OUString > seqNames;

    if( withChildren )
    {
        uno::Reference< container::XNameAccess > xName( m_xFileRegistry,uno::UNO_QUERY );
        seqNames = xName->getElementNames();
    }

    sal_Int32 count = withChildren ? seqNames.getLength() : 1;

    OUString
        old_Name = srcUnqPath,
        new_Name = dstUnqPath;

    for( sal_Int32 j = 0; j < count; ++j )
    {
        if( withChildren  && ! ( isChild( srcUnqPath,seqNames[j] ) ) )
            continue;

        if( withChildren )
        {
            old_Name = seqNames[j];
            new_Name = newName( dstUnqPath,srcUnqPath,old_Name );
        }

        uno::Reference< XPersistentPropertySet > x_src;

        if( m_xFileRegistry.is() )
        {
            x_src = m_xFileRegistry->openPropertySet( old_Name,false );
            m_xFileRegistry->removePropertySet( new_Name );
        }

        if( x_src.is() )
        {
            uno::Sequence< beans::Property > seqProperty =
                x_src->getPropertySetInfo()->getProperties();

            if( seqProperty.getLength() )
            {
                uno::Reference< XPersistentPropertySet >
                    x_dstS = m_xFileRegistry->openPropertySet( new_Name,true );
                uno::Reference< beans::XPropertyContainer >
                    x_dstC( x_dstS,uno::UNO_QUERY );

                for( sal_Int32 i = 0; i < seqProperty.getLength(); ++i )
                {
                    x_dstC->addProperty( seqProperty[i].Name,
                                         seqProperty[i].Attributes,
                                         x_src->getPropertyValue( seqProperty[i].Name ) );
                }
            }
        }
    }         // end for( sal_Int...
}

uno::Sequence< ucb::ContentInfo > TaskManager::queryCreatableContentsInfo()
{
    uno::Sequence< ucb::ContentInfo > seq(2);

    // file
    seq[0].Type       = FileContentType;
    seq[0].Attributes = ucb::ContentInfoAttribute::INSERT_WITH_INPUTSTREAM
                        | ucb::ContentInfoAttribute::KIND_DOCUMENT;

    uno::Sequence< beans::Property > props( 1 );
    props[0] = beans::Property(
        "Title",
        -1,
        cppu::UnoType<OUString>::get(),
        beans::PropertyAttribute::MAYBEVOID
        | beans::PropertyAttribute::BOUND );
    seq[0].Properties = props;

    // folder
    seq[1].Type       = FolderContentType;
    seq[1].Attributes = ucb::ContentInfoAttribute::KIND_FOLDER;
    seq[1].Properties = props;
    return seq;
}

/*******************************************************************************/
/*                                                                             */
/*                 some miscellaneous static functions                        */
/*                                                                             */
/*******************************************************************************/

void
TaskManager::getScheme( OUString& Scheme )
{
  Scheme = "file";
}

OUString
TaskManager::getImplementationName_static()
{
  return OUString("com.sun.star.comp.ucb.FileProvider");
}


uno::Sequence< OUString >
TaskManager::getSupportedServiceNames_static()
{
  OUString Supported("com.sun.star.ucb.FileContentProvider");
  css::uno::Sequence< OUString > Seq( &Supported,1 );
  return Seq;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
