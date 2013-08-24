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

#include <sys/stat.h>

#include <stack>
#include "osl/diagnose.h"
#include <rtl/uri.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/time.h>
#include <osl/file.hxx>
#include <com/sun/star/lang/IllegalAccessException.hpp>
#include <com/sun/star/beans/IllegalTypeException.hpp>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/ucb/Store.hpp>
#include <com/sun/star/ucb/XContentIdentifier.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/ucb/OpenCommandArgument.hpp>
#include <com/sun/star/ucb/XPropertySetRegistryFactory.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/ContentInfoAttribute.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#include <rtl/string.hxx>
#include "filerror.hxx"
#include "filglob.hxx"
#include "filcmd.hxx"
#include "filinpstr.hxx"
#include "filstr.hxx"
#include "filrset.hxx"
#include "filrow.hxx"
#include "filprp.hxx"
#include "filid.hxx"
#include "shell.hxx"
#include "prov.hxx"
#include "bc.hxx"


using namespace fileaccess;
using namespace com::sun::star;
using namespace com::sun::star::ucb;


shell::UnqPathData::UnqPathData()
    : properties( 0 ),
      notifier( 0 ),
      xS( 0 ),
      xC( 0 ),
      xA( 0 )
{
    // empty
}


shell::UnqPathData::UnqPathData( const UnqPathData& a )
    : properties( a.properties ),
      notifier( a.notifier ),
      xS( a.xS ),
      xC( a.xC ),
      xA( a.xA )
{
}


shell::UnqPathData& shell::UnqPathData::operator=( UnqPathData& a )
{
    properties = a.properties;
    notifier = a.notifier;
    xS = a.xS;
    xC = a.xC;
    xA = a.xA;
    a.properties = 0;
    a.notifier = 0;
    a.xS = 0;
    a.xC = 0;
    a.xA = 0;
    return *this;
}

shell::UnqPathData::~UnqPathData()
{
    if( properties )
        delete properties;
    if( notifier )
        delete notifier;
}



////////////////////////////////////////////////////////////////////////////////////////





shell::MyProperty::MyProperty( const OUString&                         __PropertyName )
    : PropertyName( __PropertyName )
{
    // empty
}


shell::MyProperty::MyProperty( const sal_Bool&                              __isNative,
                               const OUString&                         __PropertyName,
                               const sal_Int32&                             __Handle,
                               const com::sun::star::uno::Type&              __Typ,
                               const com::sun::star::uno::Any&              __Value,
                               const com::sun::star::beans::PropertyState&  __State,
                               const sal_Int16&                             __Attributes )
    : PropertyName( __PropertyName ),
      Handle( __Handle ),
      isNative( __isNative ),
      Typ( __Typ ),
      Value( __Value ),
      State( __State ),
      Attributes( __Attributes )
{
    // empty
}

shell::MyProperty::~MyProperty()
{
    // empty for now
}


#include "filinl.hxx"


shell::shell( const uno::Reference< uno::XComponentContext >& rxContext,
              FileProvider* pProvider, sal_Bool bWithConfig )
    : TaskManager(),
      m_bWithConfig( bWithConfig ),
      m_pProvider( pProvider ),
      m_xContext( rxContext ),
      Title( "Title" ),
      CasePreservingURL( "CasePreservingURL" ),
      IsDocument( "IsDocument" ),
      IsFolder( "IsFolder" ),
      DateModified( "DateModified" ),
      Size( "Size" ),
      IsVolume( "IsVolume" ),
      IsRemoveable( "IsRemoveable" ),
      IsRemote( "IsRemote" ),
      IsCompactDisc( "IsCompactDisc" ),
      IsFloppy( "IsFloppy" ),
      IsHidden( "IsHidden" ),
      ContentType( "ContentType" ),
      IsReadOnly( "IsReadOnly" ),
      CreatableContentsInfo( "CreatableContentsInfo" ),
      FolderContentType( "application/vnd.sun.staroffice.fsys-folder" ),
      FileContentType( "application/vnd.sun.staroffice.fsys-file" ),
      m_sCommandInfo( 9 )
{
    // Title
    m_aDefaultProperties.insert( MyProperty( true,
                                             Title,
                                             -1 ,
                                             getCppuType( static_cast< OUString* >( 0 ) ),
                                             uno::Any(),
                                             beans::PropertyState_DEFAULT_VALUE,
                                             beans::PropertyAttribute::MAYBEVOID
                                             | beans::PropertyAttribute::BOUND ) );

    // CasePreservingURL
    m_aDefaultProperties.insert(
        MyProperty( true,
                    CasePreservingURL,
                    -1 ,
                    getCppuType( static_cast< OUString* >( 0 ) ),
                    uno::Any(),
                    beans::PropertyState_DEFAULT_VALUE,
                    beans::PropertyAttribute::MAYBEVOID
                    | beans::PropertyAttribute::BOUND
                    | beans::PropertyAttribute::READONLY ) );


    // IsFolder
    m_aDefaultProperties.insert( MyProperty( true,
                                             IsFolder,
                                             -1 ,
                                             getCppuType( static_cast< sal_Bool* >( 0 ) ),
                                             uno::Any(),
                                             beans::PropertyState_DEFAULT_VALUE,
                                             beans::PropertyAttribute::MAYBEVOID
                                             | beans::PropertyAttribute::BOUND
                                             | beans::PropertyAttribute::READONLY ) );


    // IsDocument
    m_aDefaultProperties.insert( MyProperty( true,
                                             IsDocument,
                                             -1 ,
                                             getCppuType( static_cast< sal_Bool* >( 0 ) ),
                                             uno::Any(),
                                             beans::PropertyState_DEFAULT_VALUE,
                                             beans::PropertyAttribute::MAYBEVOID
                                             | beans::PropertyAttribute::BOUND
                                             | beans::PropertyAttribute::READONLY ) );

    // Removable
    m_aDefaultProperties.insert( MyProperty( true,
                                             IsVolume,
                                             -1 ,
                                             getCppuType( static_cast< sal_Bool* >( 0 ) ),
                                             uno::Any(),
                                             beans::PropertyState_DEFAULT_VALUE,
                                             beans::PropertyAttribute::MAYBEVOID
                                             | beans::PropertyAttribute::BOUND
                                             | beans::PropertyAttribute::READONLY ) );


    // Removable
    m_aDefaultProperties.insert( MyProperty( true,
                                             IsRemoveable,
                                             -1 ,
                                             getCppuType( static_cast< sal_Bool* >( 0 ) ),
                                             uno::Any(),
                                             beans::PropertyState_DEFAULT_VALUE,
                                             beans::PropertyAttribute::MAYBEVOID
                                             | beans::PropertyAttribute::BOUND
                                             | beans::PropertyAttribute::READONLY ) );

    // Remote
    m_aDefaultProperties.insert( MyProperty( true,
                                             IsRemote,
                                             -1 ,
                                             getCppuType( static_cast< sal_Bool* >( 0 ) ),
                                             uno::Any(),
                                             beans::PropertyState_DEFAULT_VALUE,
                                             beans::PropertyAttribute::MAYBEVOID
                                             | beans::PropertyAttribute::BOUND
                                             | beans::PropertyAttribute::READONLY ) );

    // CompactDisc
    m_aDefaultProperties.insert( MyProperty( true,
                                             IsCompactDisc,
                                             -1 ,
                                             getCppuType( static_cast< sal_Bool* >( 0 ) ),
                                             uno::Any(),
                                             beans::PropertyState_DEFAULT_VALUE,
                                             beans::PropertyAttribute::MAYBEVOID
                                             | beans::PropertyAttribute::BOUND
                                             | beans::PropertyAttribute::READONLY ) );

    // Floppy
    m_aDefaultProperties.insert( MyProperty( true,
                                             IsFloppy,
                                             -1 ,
                                             getCppuType( static_cast< sal_Bool* >( 0 ) ),
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
            getCppuType( static_cast< sal_Bool* >( 0 ) ),
            uno::Any(),
            beans::PropertyState_DEFAULT_VALUE,
            beans::PropertyAttribute::MAYBEVOID
            | beans::PropertyAttribute::BOUND
#if defined( WNT )
        ));
#else
    | beans::PropertyAttribute::READONLY)); // under unix/linux only readable
#endif


    // ContentType
    uno::Any aAny;
    aAny <<= OUString();
    m_aDefaultProperties.insert( MyProperty( false,
                                             ContentType,
                                             -1 ,
                                             getCppuType( static_cast< OUString* >( 0 ) ),
                                             aAny,
                                             beans::PropertyState_DEFAULT_VALUE,
                                             beans::PropertyAttribute::MAYBEVOID
                                             | beans::PropertyAttribute::BOUND
                                             | beans::PropertyAttribute::READONLY ) );


    // DateModified
    m_aDefaultProperties.insert( MyProperty( true,
                                             DateModified,
                                             -1 ,
                                             getCppuType( static_cast< util::DateTime* >( 0 ) ),
                                             uno::Any(),
                                             beans::PropertyState_DEFAULT_VALUE,
                                             beans::PropertyAttribute::MAYBEVOID
                                             | beans::PropertyAttribute::BOUND ) );

    // Size
    m_aDefaultProperties.insert( MyProperty( true,
                                             Size,
                                             -1,
                                             getCppuType( static_cast< sal_Int64* >( 0 ) ),
                                             uno::Any(),
                                             beans::PropertyState_DEFAULT_VALUE,
                                             beans::PropertyAttribute::MAYBEVOID
                                             | beans::PropertyAttribute::BOUND ) );

    // IsReadOnly
    m_aDefaultProperties.insert( MyProperty( true,
                                             IsReadOnly,
                                             -1 ,
                                             getCppuType( static_cast< sal_Bool* >( 0 ) ),
                                             uno::Any(),
                                             beans::PropertyState_DEFAULT_VALUE,
                                             beans::PropertyAttribute::MAYBEVOID
                                             | beans::PropertyAttribute::BOUND ) );


    // CreatableContentsInfo
    m_aDefaultProperties.insert( MyProperty( true,
                                             CreatableContentsInfo,
                                             -1 ,
                                             getCppuType( static_cast< const uno::Sequence< ucb::ContentInfo > * >( 0 ) ),
                                             uno::Any(),
                                             beans::PropertyState_DEFAULT_VALUE,
                                             beans::PropertyAttribute::MAYBEVOID
                                             | beans::PropertyAttribute::BOUND
                                             | beans::PropertyAttribute::READONLY ) );

    // Commands
    m_sCommandInfo[0].Name = OUString("getCommandInfo");
    m_sCommandInfo[0].Handle = -1;
    m_sCommandInfo[0].ArgType = getCppuVoidType();

    m_sCommandInfo[1].Name = OUString("getPropertySetInfo");
    m_sCommandInfo[1].Handle = -1;
    m_sCommandInfo[1].ArgType = getCppuVoidType();

    m_sCommandInfo[2].Name = OUString("getPropertyValues");
    m_sCommandInfo[2].Handle = -1;
    m_sCommandInfo[2].ArgType = getCppuType( static_cast< uno::Sequence< beans::Property >* >( 0 ) );

    m_sCommandInfo[3].Name = OUString("setPropertyValues");
    m_sCommandInfo[3].Handle = -1;
    m_sCommandInfo[3].ArgType = getCppuType( static_cast< uno::Sequence< beans::PropertyValue >* >( 0 ) );

    m_sCommandInfo[4].Name = OUString("open");
    m_sCommandInfo[4].Handle = -1;
    m_sCommandInfo[4].ArgType = getCppuType( static_cast< OpenCommandArgument* >( 0 ) );

    m_sCommandInfo[5].Name = OUString("transfer");
    m_sCommandInfo[5].Handle = -1;
    m_sCommandInfo[5].ArgType = getCppuType( static_cast< TransferInfo* >( 0 ) );

    m_sCommandInfo[6].Name = OUString("delete");
    m_sCommandInfo[6].Handle = -1;
    m_sCommandInfo[6].ArgType = getCppuType( static_cast< sal_Bool* >( 0 ) );

    m_sCommandInfo[7].Name = OUString("insert");
    m_sCommandInfo[7].Handle = -1;
    m_sCommandInfo[7].ArgType = getCppuType( static_cast< InsertCommandArgument* > ( 0 ) );

    m_sCommandInfo[7].Name = OUString("createNewContent");
    m_sCommandInfo[7].Handle = -1;
    m_sCommandInfo[7].ArgType = getCppuType( static_cast< ucb::ContentInfo * > ( 0 ) );

    if(m_bWithConfig)
    {
        uno::Reference< XPropertySetRegistryFactory > xRegFac = ucb::Store::create( m_xContext );
        // Open/create a registry
        m_xFileRegistry = xRegFac->createPropertySetRegistry( OUString() );
    }
}


shell::~shell()
{
}


/*********************************************************************************/
/*                                                                               */
/*                     de/registerNotifier-Implementation                        */
/*                                                                               */
/*********************************************************************************/

//
//  This two methods register and deregister a change listener for the content belonging
//  to URL aUnqPath
//

void SAL_CALL
shell::registerNotifier( const OUString& aUnqPath, Notifier* pNotifier )
{
    osl::MutexGuard aGuard( m_aMutex );

    ContentMap::iterator it =
        m_aContent.insert( ContentMap::value_type( aUnqPath,UnqPathData() ) ).first;

    if( ! it->second.notifier )
        it->second.notifier = new NotifierList();

    std::list< Notifier* >& nlist = *( it->second.notifier );

    std::list<Notifier*>::iterator it1 = nlist.begin();
    while( it1 != nlist.end() )               // Every "Notifier" only once
    {
        if( *it1 == pNotifier ) return;
        ++it1;
    }
    nlist.push_back( pNotifier );
}



void SAL_CALL
shell::deregisterNotifier( const OUString& aUnqPath,Notifier* pNotifier )
{
    osl::MutexGuard aGuard( m_aMutex );

    ContentMap::iterator it = m_aContent.find( aUnqPath );
    if( it == m_aContent.end() )
        return;

    it->second.notifier->remove( pNotifier );

    if( ! it->second.notifier->size() )
        m_aContent.erase( it );
}



/*********************************************************************************/
/*                                                                               */
/*                     de/associate-Implementation                               */
/*                                                                               */
/*********************************************************************************/
//
//  Used to associate and deassociate a new property with
//  the content belonging to URL UnqPath.
//  The default value and the attributes are input
//

void SAL_CALL
shell::associate( const OUString& aUnqPath,
                  const OUString& PropertyName,
                  const uno::Any& DefaultValue,
                  const sal_Int16 Attributes )
    throw( beans::PropertyExistException,
           beans::IllegalTypeException,
           uno::RuntimeException )
{
    MyProperty newProperty( false,
                            PropertyName,
                            -1,
                            DefaultValue.getValueType(),
                            DefaultValue,
                            beans::PropertyState_DEFAULT_VALUE,
                            Attributes );

    shell::PropertySet::iterator it1 = m_aDefaultProperties.find( newProperty );
    if( it1 != m_aDefaultProperties.end() )
        throw beans::PropertyExistException( OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >() );

    {
        osl::MutexGuard aGuard( m_aMutex );

        ContentMap::iterator it = m_aContent.insert( ContentMap::value_type( aUnqPath,UnqPathData() ) ).first;

        // Load the XPersistentPropertySetInfo and create it, if it does not exist
        load( it,true );

        PropertySet& properties = *(it->second.properties);
        it1 = properties.find( newProperty );
        if( it1 != properties.end() )
            throw beans::PropertyExistException( OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >() );

        // Property does not exist
        properties.insert( newProperty );
        it->second.xC->addProperty( PropertyName,Attributes,DefaultValue );
    }
    notifyPropertyAdded( getPropertySetListeners( aUnqPath ), PropertyName );
}




void SAL_CALL
shell::deassociate( const OUString& aUnqPath,
            const OUString& PropertyName )
  throw( beans::UnknownPropertyException,
     beans::NotRemoveableException,
     uno::RuntimeException )
{
    MyProperty oldProperty( PropertyName );

    shell::PropertySet::iterator it1 = m_aDefaultProperties.find( oldProperty );
    if( it1 != m_aDefaultProperties.end() )
        throw beans::NotRemoveableException( OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >() );

    osl::MutexGuard aGuard( m_aMutex );

    ContentMap::iterator it = m_aContent.insert( ContentMap::value_type( aUnqPath,UnqPathData() ) ).first;

    load( it,false );

    PropertySet& properties = *(it->second.properties);

    it1 = properties.find( oldProperty );
    if( it1 == properties.end() )
        throw beans::UnknownPropertyException( OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >() );

    properties.erase( it1 );

    if( it->second.xC.is() )
        it->second.xC->removeProperty( PropertyName );

    if( properties.size() == 9 )
    {
        MyProperty ContentTProperty( ContentType );

        if( properties.find( ContentTProperty )->getState() == beans::PropertyState_DEFAULT_VALUE )
        {
            it->second.xS = 0;
            it->second.xC = 0;
            it->second.xA = 0;
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
//
//  Given an xOutputStream, this method writes the content of the file belonging to
//  URL aUnqPath into the XOutputStream
//


void SAL_CALL shell::page( sal_Int32 CommandId,
                           const OUString& aUnqPath,
                           const uno::Reference< io::XOutputStream >& xOutputStream )
    throw()
{
    uno::Reference< XContentProvider > xProvider( m_pProvider );
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
        err = aFile.read( (void*) BFF,bfz,nrc );
        if(  err == osl::FileBase::E_None )
        {
            uno::Sequence< sal_Int8 > seq( BFF, (sal_uInt32)nrc );
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
//
//  Given a file URL aUnqPath, this methods returns a XInputStream which reads from the open file.
//


uno::Reference< io::XInputStream > SAL_CALL
shell::open( sal_Int32 CommandId,
             const OUString& aUnqPath,
             sal_Bool bLock )
    throw()
{
    XInputStream_impl* xInputStream = new XInputStream_impl( this, aUnqPath, bLock ); // from filinpstr.hxx

    sal_Int32 ErrorCode = xInputStream->CtorSuccess();

    if( ErrorCode != TASKHANDLER_NO_ERROR )
    {
        installError( CommandId,
                      ErrorCode,
                      xInputStream->getMinorError() );

        delete xInputStream;
        xInputStream = 0;
    }

    return uno::Reference< io::XInputStream >( xInputStream );
}




/*********************************************************************************/
/*                                                                               */
/*                     open for read/write access-Implementation                 */
/*                                                                               */
/*********************************************************************************/
//
//  Given a file URL aUnqPath, this methods returns a XStream which can be used
//  to read and write from/to the file.
//


uno::Reference< io::XStream > SAL_CALL
shell::open_rw( sal_Int32 CommandId,
                const OUString& aUnqPath,
                sal_Bool bLock )
    throw()
{
    XStream_impl* xStream = new XStream_impl( this, aUnqPath, bLock );  // from filstr.hxx

    sal_Int32 ErrorCode = xStream->CtorSuccess();

    if( ErrorCode != TASKHANDLER_NO_ERROR )
    {
        installError( CommandId,
                      ErrorCode,
                      xStream->getMinorError() );

        delete xStream;
        xStream = 0;
    }
    return uno::Reference< io::XStream >( xStream );
}



/*********************************************************************************/
/*                                                                               */
/*                       ls-Implementation                                       */
/*                                                                               */
/*********************************************************************************/
//
//  This method returns the result set containing the children of the directory belonging
//  to file URL aUnqPath
//


uno::Reference< XDynamicResultSet > SAL_CALL
shell::ls( sal_Int32 CommandId,
           const OUString& aUnqPath,
           const sal_Int32 OpenMode,
           const uno::Sequence< beans::Property >& seq,
           const uno::Sequence< NumberedSortingInfo >& seqSort )
    throw()
{
    XResultSet_impl* p = new XResultSet_impl( this,aUnqPath,OpenMode,seq,seqSort );

    sal_Int32 ErrorCode = p->CtorSuccess();

    if( ErrorCode != TASKHANDLER_NO_ERROR )
    {
        installError( CommandId,
                      ErrorCode,
                      p->getMinorError() );

        delete p;
        p = 0;
    }

    return uno::Reference< XDynamicResultSet > ( p );
}




/*********************************************************************************/
/*                                                                               */
/*                          info_c implementation                                */
/*                                                                               */
/*********************************************************************************/
// Info for commands

uno::Reference< XCommandInfo > SAL_CALL
shell::info_c()
    throw()
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

uno::Reference< beans::XPropertySetInfo > SAL_CALL
shell::info_p( const OUString& aUnqPath )
    throw()
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
//
//  Sets the values of the properties belonging to fileURL aUnqPath
//


uno::Sequence< uno::Any > SAL_CALL
shell::setv( const OUString& aUnqPath,
             const uno::Sequence< beans::PropertyValue >& values )
    throw()
{
    osl::MutexGuard aGuard( m_aMutex );

    sal_Int32 propChanged = 0;
    uno::Sequence< uno::Any > ret( values.getLength() );
    uno::Sequence< beans::PropertyChangeEvent > seqChanged( values.getLength() );

    shell::ContentMap::iterator it = m_aContent.find( aUnqPath );
    PropertySet& properties = *( it->second.properties );
    shell::PropertySet::iterator it1;
    uno::Any aAny;

    for( sal_Int32 i = 0; i < values.getLength(); ++i )
    {
        MyProperty toset( values[i].Name );
        it1 = properties.find( toset );
        if( it1 == properties.end() )
        {
            ret[i] <<= beans::UnknownPropertyException( OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >() );
            continue;
        }

        aAny = it1->getValue();
        if( aAny == values[i].Value )
            continue;  // nothing needs to be changed

        if( it1->getAttributes() & beans::PropertyAttribute::READONLY )
        {
            ret[i] <<= lang::IllegalAccessException( OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >() );
            continue;
        }

        seqChanged[ propChanged   ].PropertyName = values[i].Name;
        seqChanged[ propChanged   ].PropertyHandle   = -1;
        seqChanged[ propChanged   ].Further   = false;
        seqChanged[ propChanged   ].OldValue <<= aAny;
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
                        uno::Sequence< uno::Any > names( 1 );
                        ret[0] <<= beans::PropertyValue(
                            OUString("Uri"), -1,
                            uno::makeAny(aUnqPath),
                            beans::PropertyState_DIRECT_VALUE);
                        IOErrorCode ioError(IOErrorCode_GENERAL);
                        ret[i] <<= InteractiveAugmentedIOException(
                            OUString(),
                            0,
                            task::InteractionClassification_ERROR,
                            ioError,
                            names );
                    }
                }
                else
                    ret[i] <<= beans::IllegalTypeException( OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >() );
            }
            else if(values[i].Name == IsReadOnly ||
                    values[i].Name == IsHidden)
            {
                sal_Bool value = sal_False;
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
                        uno::Sequence< uno::Any > names( 1 );
                        names[0] <<= beans::PropertyValue(
                            OUString("Uri"), -1,
                            uno::makeAny(aUnqPath),
                            beans::PropertyState_DIRECT_VALUE);
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
                            0,
                            task::InteractionClassification_ERROR,
                            ioError,
                            names );
                    }
                }
                else
                    ret[i] <<= beans::IllegalTypeException( OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >() );
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
//
//  Reads the values of the properties belonging to fileURL aUnqPath;
//  Returns an XRow object containing the values in the requested order.
//


uno::Reference< sdbc::XRow > SAL_CALL
shell::getv( sal_Int32 CommandId,
             const OUString& aUnqPath,
             const uno::Sequence< beans::Property >& properties )
    throw()
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

        shell::ContentMap::iterator it = m_aContent.find( aUnqPath );
        commit( it,aFileStatus );

        shell::PropertySet::iterator it1;
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
//
//  Moves the content belonging to fileURL srcUnqPath to fileURL dstUnqPath.
//

void SAL_CALL
shell::move( sal_Int32 CommandId,
             const OUString srcUnqPath,
             const OUString dstUnqPathIn,
             const sal_Int32 NameClash )
    throw()
{
    // --> #i88446# Method notifyContentExchanged( getContentExchangedEventListeners( srcUnqPath,dstUnqPath,!isDocument ) ); crashes if
    // srcUnqPath and dstUnqPathIn are equal
    if( srcUnqPath == dstUnqPathIn )
        return;
    //
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

                    OUString aPostFix( "_" );
                    aPostFix += OUString::number( ++nTry );

                    newDstUnqPath = newDstUnqPath.replaceAt( nPos, 0, aPostFix );

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
    sal_Bool isDocument = ( aStatus.getFileType() == osl::FileStatus::Regular );


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
//
//  Copies the content belonging to fileURL srcUnqPath to fileURL dstUnqPath ( files and directories )
//

namespace {

bool getType(
    TaskManager & task, sal_Int32 id, OUString const & fileUrl,
    osl::DirectoryItem * item, osl::FileStatus::Type * type)
{
    OSL_ASSERT(item != 0 && type != 0);
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

void SAL_CALL
shell::copy(
    sal_Int32 CommandId,
    const OUString srcUnqPath,
    const OUString dstUnqPathIn,
    sal_Int32 NameClash )
    throw()
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

    sal_Bool isDocument
        = type != osl::FileStatus::Directory && type != osl::FileStatus::Volume;
    sal_Int32 IsWhat = isDocument ? -1 : 1;

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
            remove( CommandId, dstUnqPath, IsWhat, sal_False );

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
            OUString newDstUnqPath;
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

                    OUString aPostFix( "_" );
                    aPostFix += OUString::number( ++nTry );

                    newDstUnqPath = newDstUnqPath.replaceAt( nPos, 0, aPostFix );

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
//
//  Deletes the content belonging to fileURL aUnqPath( recursively in case of directory )
//  Return: success of operation
//


sal_Bool SAL_CALL
shell::remove( sal_Int32 CommandId,
               const OUString& aUnqPath,
               sal_Int32 IsWhat,
               sal_Bool  MustExist )
    throw()
{
    sal_Int32 nMask = osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileURL;

    osl::DirectoryItem aItem;
    osl::FileStatus aStatus( nMask );
    osl::FileBase::RC nError;

    if( IsWhat == 0 ) // Determine whether we are removing a directory or a file
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
            return sal_False;
        }

        if( aStatus.getFileType() == osl::FileStatus::Regular ||
            aStatus.getFileType() == osl::FileStatus::Link )
            IsWhat = -1;  // RemoveFile
        else if(  aStatus.getFileType() == osl::FileStatus::Directory ||
                  aStatus.getFileType() == osl::FileStatus::Volume )
            IsWhat = +1;  // RemoveDirectory
    }


    if( IsWhat == -1 )    // Removing a file
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
    else if( IsWhat == +1 )    // Removing a directory
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

        sal_Bool whileSuccess = sal_True;
        sal_Int32 recurse = 0;
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
                whileSuccess = sal_False;
                break;
            }

            if( aStatus.getFileType() == osl::FileStatus::Regular ||
                aStatus.getFileType() == osl::FileStatus::Link )
                recurse = -1;
            else if( aStatus.getFileType() == osl::FileStatus::Directory ||
                     aStatus.getFileType() == osl::FileStatus::Volume )
                recurse = +1;

            name = aStatus.getFileURL();
            whileSuccess = remove(
                CommandId, name, recurse, MustExist );
            if( !whileSuccess )
                break;

            nError = aDirectory.getNextItem( aItem );
        }

        aDirectory.close();

        if( ! whileSuccess )
            return sal_False;     // error code is installed

        if( nError != osl::FileBase::E_NOENT )
        {
            installError( CommandId,
                          TASKHANDLING_DIRECTORYEXHAUSTED_FOR_REMOVE,
                          nError );
            return sal_False;
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
        return sal_False;
    }

    return sal_True;
}


/********************************************************************************/
/*                                                                              */
/*                         mkdir-implementation                                 */
/*                                                                              */
/********************************************************************************/
//
//  Creates new directory with given URL, recursively if necessary
//  Return:: success of operation
//

sal_Bool SAL_CALL
shell::mkdir( sal_Int32 CommandId,
              const OUString& rUnqPath,
              sal_Bool OverWrite )
    throw()
{
    OUString aUnqPath;

    // remove trailing slash
    if ( rUnqPath[ rUnqPath.getLength() - 1 ] == sal_Unicode( '/' ) )
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
                return sal_False;
            }
            else
                return sal_True;
        }
        case osl::FileBase::E_INVAL:
        {
            installError(CommandId,
                         TASKHANDLING_INVALID_NAME_MKDIR);
            return sal_False;
        }
        case osl::FileBase::E_None:
        {
            OUString aPrtPath = getParentName( aUnqPath );
            notifyInsert( getContentEventListeners( aPrtPath ),aUnqPath );
            return sal_True;
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
//
//  Creates new file with given URL.
//  The content of aInputStream becomes the content of the file
//  Return:: success of operation
//

sal_Bool SAL_CALL
shell::mkfil( sal_Int32 CommandId,
              const OUString& aUnqPath,
              sal_Bool Overwrite,
              const uno::Reference< io::XInputStream >& aInputStream )
    throw()
{
    // return value unimportant
    sal_Bool bSuccess = write( CommandId,
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
//
//  writes to the file with given URL.
//  The content of aInputStream becomes the content of the file
//  Return:: success of operation
//

sal_Bool SAL_CALL
shell::write( sal_Int32 CommandId,
              const OUString& aUnqPath,
              sal_Bool OverWrite,
              const uno::Reference< io::XInputStream >& aInputStream )
    throw()
{
    if( ! aInputStream.is() )
    {
        installError( CommandId,
                      TASKHANDLING_INPUTSTREAM_FOR_WRITE );
        return sal_False;
    }

    // Create parent path, if necessary.
    if ( ! ensuredir( CommandId,
                      getParentName( aUnqPath ),
                      TASKHANDLING_ENSUREDIR_FOR_WRITE ) )
        return sal_False;

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
                return sal_False;
            }

            // the existing file was just opened and should be overwritten now,
            // truncate it first

            err = aFile.setSize( 0 );
            if( err != osl::FileBase::E_None  )
            {
                installError( CommandId,
                              TASKHANDLING_FILESIZE_FOR_WRITE,
                              err );
                return sal_False;
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
            return sal_False;
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
            return sal_False;
        }
    }

    sal_Bool bSuccess = sal_True;

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
            bSuccess = sal_False;
            break;
        }
        catch( const io::BufferSizeExceededException& )
        {
            installError( CommandId,
                          TASKHANDLING_BUFFERSIZEEXCEEDED_FOR_WRITE );
            bSuccess = sal_False;
            break;
        }
        catch( const io::IOException& )
        {
            installError( CommandId,
                          TASKHANDLING_IOEXCEPTION_FOR_WRITE );
            bSuccess = sal_False;
            break;
        }

        if( nReadBytes )
        {
            const sal_Int8* p = seq.getConstArray();

            err = aFile.write( ((void*)(p)),
                               sal_uInt64( nReadBytes ),
                               nWrittenBytes );

            if( err != osl::FileBase::E_None )
            {
                installError( CommandId,
                              TASKHANDLING_FILEIOERROR_FOR_WRITE,
                              err );
                bSuccess = sal_False;
                break;
            }
            else if( nWrittenBytes != sal_uInt64( nReadBytes ) )
            {
                installError( CommandId,
                              TASKHANDLING_FILEIOERROR_FOR_NO_SPACE );
                bSuccess = sal_False;
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
        bSuccess = sal_False;
    }

    return bSuccess;
}



/*********************************************************************************/
/*                                                                               */
/*                 insertDefaultProperties-Implementation                        */
/*                                                                               */
/*********************************************************************************/


void SAL_CALL shell::insertDefaultProperties( const OUString& aUnqPath )
{
    osl::MutexGuard aGuard( m_aMutex );

    ContentMap::iterator it =
        m_aContent.insert( ContentMap::value_type( aUnqPath,UnqPathData() ) ).first;

    load( it,false );

    MyProperty ContentTProperty( ContentType );

    PropertySet& properties = *(it->second.properties);
    sal_Bool ContentNotDefau = properties.find( ContentTProperty ) != properties.end();

    shell::PropertySet::iterator it1 = m_aDefaultProperties.begin();
    while( it1 != m_aDefaultProperties.end() )
    {
        if( ContentNotDefau && it1->getPropertyName() == ContentType )
        {
            // No insertion
        }
        else
            properties.insert( *it1 );
        ++it1;
    }
}




/******************************************************************************/
/*                                                                            */
/*                          mapping of file urls                              */
/*                          to uncpath and vice versa                         */
/*                                                                            */
/******************************************************************************/


sal_Bool SAL_CALL shell::getUnqFromUrl( const OUString& Url,OUString& Unq )
{
    if ( Url == "file:///" || Url == "file://localhost/" || Url == "file://127.0.0.1/" )
    {
        Unq = OUString("file:///");
        return false;
    }

    sal_Bool err = osl::FileBase::E_None != osl::FileBase::getSystemPathFromFileURL( Url,Unq );

    Unq = Url;

    sal_Int32 l = Unq.getLength()-1;
    if( ! err && Unq.getStr()[ l ] == '/' &&
        Unq.indexOf( '/', RTL_CONSTASCII_LENGTH("//") ) < l )
        Unq = Unq.copy(0, Unq.getLength() - 1);

    return err;
}



sal_Bool SAL_CALL shell::getUrlFromUnq( const OUString& Unq,OUString& Url )
{
    sal_Bool err = osl::FileBase::E_None != osl::FileBase::getSystemPathFromFileURL( Unq,Url );

    Url = Unq;

    return err;
}



// Helper function for public copy

osl::FileBase::RC SAL_CALL
shell::copy_recursive( const OUString& srcUnqPath,
                       const OUString& dstUnqPath,
                       sal_Int32 TypeToCopy,
                       sal_Bool testExistBeforeCopy )
    throw()
{
    osl::FileBase::RC err = osl::FileBase::E_None;

    if( TypeToCopy == -1 ) // Document
    {
        err = osl_File_copy( srcUnqPath,dstUnqPath,testExistBeforeCopy );
    }
    else if( TypeToCopy == +1 ) // Folder
    {
        osl::Directory aDir( srcUnqPath );
        aDir.open();

        err = osl::Directory::create( dstUnqPath );
        osl::FileBase::RC next = err;
        if( err == osl::FileBase::E_None )
        {
            sal_Int32 n_Mask = osl_FileStatus_Mask_FileURL | osl_FileStatus_Mask_FileName | osl_FileStatus_Mask_Type;

            osl::DirectoryItem aDirItem;

            while( err == osl::FileBase::E_None && ( next = aDir.getNextItem( aDirItem ) ) == osl::FileBase::E_None )
            {
                sal_Bool IsDoc = false;
                osl::FileStatus aFileStatus( n_Mask );
                aDirItem.getFileStatus( aFileStatus );
                if( aFileStatus.isValid( osl_FileStatus_Mask_Type ) )
                    IsDoc = aFileStatus.getFileType() == osl::FileStatus::Regular;

                // Getting the information for the next recursive copy
                sal_Int32 newTypeToCopy = IsDoc ? -1 : +1;

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

                if( newDstUnqPath.lastIndexOf( sal_Unicode('/') ) != newDstUnqPath.getLength()-1 )
                    newDstUnqPath += OUString("/");

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


sal_Bool SAL_CALL shell::ensuredir( sal_Int32 CommandId,
                                    const OUString& rUnqPath,
                                    sal_Int32 errorCode )
    throw()
{
    OUString aPath;

    if ( rUnqPath.isEmpty() )
        return sal_False;

    if ( rUnqPath[ rUnqPath.getLength() - 1 ] == sal_Unicode( '/' ) )
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
    // in this OS X specific code block.)

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
        return sal_True;

    nError = osl::Directory::create( aPath );

    if( nError == osl::File::E_None )
        notifyInsert( getContentEventListeners( getParentName( aPath ) ),aPath );

    sal_Bool  bSuccess = ( nError == osl::File::E_None || nError == osl::FileBase::E_EXIST );

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




//
//  Given a sequence of properties seq, this method determines the mask
//  used to instantiate a osl::FileStatus, so that a call to
//  osl::DirectoryItem::getFileStatus fills the required fields.
//


void SAL_CALL
shell::getMaskFromProperties(
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
//
//  Load the properties from configuration, if create == true create them.
//  The Properties are stored under the url belonging to it->first.
//

void SAL_CALL
shell::load( const ContentMap::iterator& it, sal_Bool create )
{
    if( ! it->second.properties )
        it->second.properties = new PropertySet;

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


void SAL_CALL
shell::commit( const shell::ContentMap::iterator& it,
               const osl::FileStatus& aFileStatus )
{
    uno::Any aAny;
    uno::Any emptyAny;
    shell::PropertySet::iterator it1;

    if( it->second.properties == 0 )
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
            aAny <<= aFileStatus.getFileName();
            it1->setValue( aAny );
        }
    }

    it1 = properties.find( MyProperty( CasePreservingURL ) );
    if( it1 != properties.end() )
    {
        if( aFileStatus.isValid( osl_FileStatus_Mask_FileURL ) )
        {
            aAny <<= aFileStatus.getFileURL();
            it1->setValue( aAny );
        }
    }


    sal_Bool isDirectory,isFile,isVolume,isRemoveable,isRemote,isFloppy,isCompactDisc;

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
                // extremly ugly, but otherwise default construction
                // of aDirItem and aFileStatus2
                // before the preciding if
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
            sal_Bool dummy = false;
            aAny <<= dummy;
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
        isDirectory = sal_False;
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
            sal_Bool readonly = ( Attr & osl_File_Attribute_ReadOnly ) != 0;
            it1->setValue( uno::makeAny( readonly ) );
        }
    }

    it1 = properties.find( MyProperty( IsHidden ) );
    if( it1 != properties.end() )
    {
        if( aFileStatus.isValid( osl_FileStatus_Mask_Attributes ) )
        {
            sal_uInt64 Attr = aFileStatus.getAttributes();
            sal_Bool ishidden = ( Attr & osl_File_Attribute_Hidden ) != 0;
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
            osl_getLocalTimeFromSystemTime( &temp, &myLocalTime );

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


uno::Reference< sdbc::XRow > SAL_CALL
shell::getv(
    Notifier* pNotifier,
    const uno::Sequence< beans::Property >& properties,
    osl::DirectoryItem& aDirItem,
    OUString& aUnqPath,
    sal_Bool& aIsRegular )
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
    if ( aRes == osl::FileBase::E_None )
    {
        aUnqPath = aFileStatus.getFileURL();

        // If the directory item type is a link retrieve the type of the target

        if ( aFileStatus.getFileType() == osl::FileStatus::Link )
        {
            // Assume failure
            aIsRegular = false;
            osl::FileBase::RC result = osl::FileBase::E_INVAL;
            osl::DirectoryItem aTargetItem;
            osl::DirectoryItem::get( aFileStatus.getLinkTargetURL(), aTargetItem );
            if ( aTargetItem.is() )
            {
                osl::FileStatus aTargetStatus( osl_FileStatus_Mask_Type );

                if ( osl::FileBase::E_None ==
                     ( result = aTargetItem.getFileStatus( aTargetStatus ) ) )
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

            shell::ContentMap::iterator it = m_aContent.find( aUnqPath );
            commit( it,aFileStatus );

            shell::PropertySet::iterator it1;
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
    }
    XRow_impl* p = new XRow_impl( this,seq );
    return uno::Reference< sdbc::XRow >( p );
}






// EventListener


std::list< ContentEventNotifier* >* SAL_CALL
shell::getContentEventListeners( const OUString& aName )
{
    std::list< ContentEventNotifier* >* p = new std::list< ContentEventNotifier* >;
    std::list< ContentEventNotifier* >& listeners = *p;
    {
        osl::MutexGuard aGuard( m_aMutex );
        shell::ContentMap::iterator it = m_aContent.find( aName );
        if( it != m_aContent.end() && it->second.notifier )
        {
            std::list<Notifier*>& listOfNotifiers = *( it->second.notifier );
            std::list<Notifier*>::iterator it1 = listOfNotifiers.begin();
            while( it1 != listOfNotifiers.end() )
            {
                Notifier* pointer = *it1;
                ContentEventNotifier* notifier = pointer->cCEL();
                if( notifier )
                    listeners.push_back( notifier );
                ++it1;
            }
        }
    }
    return p;
}



std::list< ContentEventNotifier* >* SAL_CALL
shell::getContentDeletedEventListeners( const OUString& aName )
{
    std::list< ContentEventNotifier* >* p = new std::list< ContentEventNotifier* >;
    std::list< ContentEventNotifier* >& listeners = *p;
    {
        osl::MutexGuard aGuard( m_aMutex );
        shell::ContentMap::iterator it = m_aContent.find( aName );
        if( it != m_aContent.end() && it->second.notifier )
        {
            std::list<Notifier*>& listOfNotifiers = *( it->second.notifier );
            std::list<Notifier*>::iterator it1 = listOfNotifiers.begin();
            while( it1 != listOfNotifiers.end() )
            {
                Notifier* pointer = *it1;
                ContentEventNotifier* notifier = pointer->cDEL();
                if( notifier )
                    listeners.push_back( notifier );
                ++it1;
            }
        }
    }
    return p;
}


void SAL_CALL
shell::notifyInsert( std::list< ContentEventNotifier* >* listeners,const OUString& aChildName )
{
    std::list< ContentEventNotifier* >::iterator it = listeners->begin();
    while( it != listeners->end() )
    {
        (*it)->notifyChildInserted( aChildName );
        delete (*it);
        ++it;
    }
    delete listeners;
}


void SAL_CALL
shell::notifyContentDeleted( std::list< ContentEventNotifier* >* listeners )
{
    std::list< ContentEventNotifier* >::iterator it = listeners->begin();
    while( it != listeners->end() )
    {
        (*it)->notifyDeleted();
        delete (*it);
        ++it;
    }
    delete listeners;
}


void SAL_CALL
shell::notifyContentRemoved( std::list< ContentEventNotifier* >* listeners,
                             const OUString& aChildName )
{
    std::list< ContentEventNotifier* >::iterator it = listeners->begin();
    while( it != listeners->end() )
    {
        (*it)->notifyRemoved( aChildName );
        delete (*it);
        ++it;
    }
    delete listeners;
}




std::list< PropertySetInfoChangeNotifier* >* SAL_CALL
shell::getPropertySetListeners( const OUString& aName )
{
    std::list< PropertySetInfoChangeNotifier* >* p = new std::list< PropertySetInfoChangeNotifier* >;
    std::list< PropertySetInfoChangeNotifier* >& listeners = *p;
    {
        osl::MutexGuard aGuard( m_aMutex );
        shell::ContentMap::iterator it = m_aContent.find( aName );
        if( it != m_aContent.end() && it->second.notifier )
        {
            std::list<Notifier*>& listOfNotifiers = *( it->second.notifier );
            std::list<Notifier*>::iterator it1 = listOfNotifiers.begin();
            while( it1 != listOfNotifiers.end() )
            {
                Notifier* pointer = *it1;
                PropertySetInfoChangeNotifier* notifier = pointer->cPSL();
                if( notifier )
                    listeners.push_back( notifier );
                ++it1;
            }
        }
    }
    return p;
}


void SAL_CALL
shell::notifyPropertyAdded( std::list< PropertySetInfoChangeNotifier* >* listeners,
                            const OUString& aPropertyName )
{
    std::list< PropertySetInfoChangeNotifier* >::iterator it = listeners->begin();
    while( it != listeners->end() )
    {
        (*it)->notifyPropertyAdded( aPropertyName );
        delete (*it);
        ++it;
    }
    delete listeners;
}


void SAL_CALL
shell::notifyPropertyRemoved( std::list< PropertySetInfoChangeNotifier* >* listeners,
                              const OUString& aPropertyName )
{
    std::list< PropertySetInfoChangeNotifier* >::iterator it = listeners->begin();
    while( it != listeners->end() )
    {
        (*it)->notifyPropertyRemoved( aPropertyName );
        delete (*it);
        ++it;
    }
    delete listeners;
}



std::vector< std::list< ContentEventNotifier* >* >* SAL_CALL
shell::getContentExchangedEventListeners( const OUString aOldPrefix,
                                          const OUString aNewPrefix,
                                          sal_Bool withChildren )
{

    std::vector< std::list< ContentEventNotifier* >* >* aVectorOnHeap =
        new std::vector< std::list< ContentEventNotifier* >* >;
    std::vector< std::list< ContentEventNotifier* >* >&  aVector = *aVectorOnHeap;

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
            ContentMap::iterator itnames = m_aContent.begin();
            while( itnames != m_aContent.end() )
            {
                if( isChild( aOldPrefix,itnames->first ) )
                {
                    oldChildList.push_back( itnames->first );
                }
                ++itnames;
            }
            count = oldChildList.size();
        }


        for( sal_Int32 j = 0; j < count; ++j )
        {
            std::list< ContentEventNotifier* >* p = new std::list< ContentEventNotifier* >;
            std::list< ContentEventNotifier* >& listeners = *p;

            if( withChildren )
            {
                aOldName = oldChildList[j];
                aNewName = newName( aNewPrefix,aOldPrefix,aOldName );
            }

            shell::ContentMap::iterator itold = m_aContent.find( aOldName );
            if( itold != m_aContent.end() )
            {
                shell::ContentMap::iterator itnew = m_aContent.insert(
                    ContentMap::value_type( aNewName,UnqPathData() ) ).first;

                // copy Ownership also
                delete itnew->second.properties;
                itnew->second.properties = itold->second.properties;
                itold->second.properties = 0;

                // copy existing list
                std::list< Notifier* >* copyList = itnew->second.notifier;
                itnew->second.notifier = itold->second.notifier;
                itold->second.notifier = 0;

                m_aContent.erase( itold );

                if( itnew != m_aContent.end() && itnew->second.notifier )
                {
                    std::list<Notifier*>& listOfNotifiers = *( itnew->second.notifier );
                    std::list<Notifier*>::iterator it1 = listOfNotifiers.begin();
                    while( it1 != listOfNotifiers.end() )
                    {
                        Notifier* pointer = *it1;
                        ContentEventNotifier* notifier = pointer->cEXC( aNewName );
                        if( notifier )
                            listeners.push_back( notifier );
                        ++it1;
                    }
                }

                // Merge with preexisting notifiers
                // However, these may be in status BaseContent::Deleted
                if( copyList != 0 )
                {
                    std::list< Notifier* >::iterator copyIt = copyList->begin();
                    while( copyIt != copyList->end() )
                    {
                        itnew->second.notifier->push_back( *copyIt );
                        ++copyIt;
                    }
                }
                delete copyList;
            }
            aVector.push_back( p );
        }
    }

    return aVectorOnHeap;
}



void SAL_CALL
shell::notifyContentExchanged( std::vector< std::list< ContentEventNotifier* >* >* listeners_vec )
{
    std::list< ContentEventNotifier* >* listeners;
    for( sal_uInt32 i = 0; i < listeners_vec->size(); ++i )
    {
        listeners = (*listeners_vec)[i];
        std::list< ContentEventNotifier* >::iterator it = listeners->begin();
        while( it != listeners->end() )
        {
            (*it)->notifyExchanged();
            delete (*it);
            ++it;
        }
        delete listeners;
    }
    delete listeners_vec;
}



std::list< PropertyChangeNotifier* >* SAL_CALL
shell::getPropertyChangeNotifier( const OUString& aName )
{
    std::list< PropertyChangeNotifier* >* p = new std::list< PropertyChangeNotifier* >;
    std::list< PropertyChangeNotifier* >& listeners = *p;
    {
        osl::MutexGuard aGuard( m_aMutex );
        shell::ContentMap::iterator it = m_aContent.find( aName );
        if( it != m_aContent.end() && it->second.notifier )
        {
            std::list<Notifier*>& listOfNotifiers = *( it->second.notifier );
            std::list<Notifier*>::iterator it1 = listOfNotifiers.begin();
            while( it1 != listOfNotifiers.end() )
            {
                Notifier* pointer = *it1;
                PropertyChangeNotifier* notifier = pointer->cPCL();
                if( notifier )
                    listeners.push_back( notifier );
                ++it1;
            }
        }
    }
    return p;
}


void SAL_CALL shell::notifyPropertyChanges( std::list< PropertyChangeNotifier* >* listeners,
                                            const uno::Sequence< beans::PropertyChangeEvent >& seqChanged )
{
    std::list< PropertyChangeNotifier* >::iterator it = listeners->begin();
    while( it != listeners->end() )
    {
        (*it)->notifyPropertyChanged( seqChanged );
        delete (*it);
        ++it;
    }
    delete listeners;
}




/********************************************************************************/
/*                       remove persistent propertyset                          */
/********************************************************************************/

void SAL_CALL
shell::erasePersistentSet( const OUString& aUnqPath,
                           sal_Bool withChildren )
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
                it->second.xS = 0;
                it->second.xC = 0;
                it->second.xA = 0;

                delete it->second.properties;
                it->second.properties = 0;
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


void SAL_CALL
shell::copyPersistentSet( const OUString& srcUnqPath,
                          const OUString& dstUnqPath,
                          sal_Bool withChildren )
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

uno::Sequence< ucb::ContentInfo > shell::queryCreatableContentsInfo()
{
    uno::Sequence< ucb::ContentInfo > seq(2);

    // file
    seq[0].Type       = FileContentType;
    seq[0].Attributes = ucb::ContentInfoAttribute::INSERT_WITH_INPUTSTREAM
                        | ucb::ContentInfoAttribute::KIND_DOCUMENT;

    uno::Sequence< beans::Property > props( 1 );
    props[0] = beans::Property(
        OUString("Title"),
        -1,
        getCppuType( static_cast< OUString* >( 0 ) ),
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
/*                 some misceancellous static functions                        */
/*                                                                             */
/*******************************************************************************/

void SAL_CALL
shell::getScheme( OUString& Scheme )
{
  Scheme = OUString("file");
}

OUString SAL_CALL
shell::getImplementationName_static( void )
{
  return OUString("com.sun.star.comp.ucb.FileProvider");
}


uno::Sequence< OUString > SAL_CALL
shell::getSupportedServiceNames_static( void )
{
  OUString Supported("com.sun.star.ucb.FileContentProvider");
  com::sun::star::uno::Sequence< OUString > Seq( &Supported,1 );
  return Seq;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
