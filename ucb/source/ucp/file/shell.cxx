 /*************************************************************************
 *
 *  $RCSfile: shell.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: abi $ $Date: 2001-02-19 10:17:05 $
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
#ifndef _VOS_DIAGNOSE_H_
#include <vos/diagnose.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _COM_SUN_STAR_UCB_INSERTCOMMANDARGUMENT_HPP_
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_NAMECLASH_HPP_
#include <com/sun/star/ucb/NameClash.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTIDENTIFIER_HPP_
#include <com/sun/star/ucb/XContentIdentifier.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTACCESS_
#include <com/sun/star/ucb/XContentAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENCOMMANDARGUMENT_HPP_
#include <com/sun/star/ucb/OpenCommandArgument.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XPROPERTYSETREGISTRYFACTORY_HPP_
#include <com/sun/star/ucb/XPropertySetRegistryFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_TRANSFERINFO_HPP_
#include <com/sun/star/ucb/TransferInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYCHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTIESCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#endif

#ifndef _FILRSET_HXX_
#include "filrset.hxx"
#endif
#ifndef _FILROW_HXX_
#include "filrow.hxx"
#endif
#ifndef _FILPRP_HXX_
#include "filprp.hxx"
#endif
#ifndef _FILID_HXX_
#include "filid.hxx"
#endif
#ifndef _SHELL_HXX_
#include "shell.hxx"
#endif
#ifndef _PROV_HXX_
#include "prov.hxx"
#endif
#ifndef _BC_HXX_
#include "bc.hxx"
#endif

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





shell::MyProperty::MyProperty( const rtl::OUString&                         __PropertyName )
    : PropertyName( __PropertyName )
{
    // empty
}


shell::MyProperty::MyProperty( const sal_Bool&                              __isNative,
                               const rtl::OUString&                         __PropertyName,
                               const sal_Int32&                             __Handle,
                               const com::sun::star::uno::Type&              __Typ,
                               const com::sun::star::uno::Any&              __Value,
                               const com::sun::star::beans::PropertyState&  __State,
                               const sal_Int16&                             __Attributes )
    : isNative( __isNative ),
      PropertyName( __PropertyName ),
      Handle( __Handle ),
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


#ifndef _FILINL_HXX_
#include "filinl.hxx"
#endif


shell::shell( const uno::Reference< lang::XMultiServiceFactory >& xMultiServiceFactory,
              FileProvider* pProvider )
    : TaskManager(),
      m_xMultiServiceFactory( xMultiServiceFactory ),
      m_pProvider( pProvider ),
      m_sCommandInfo( 8 ),
      m_bFaked( false ),
      Title( rtl::OUString::createFromAscii( "Title" ) ),
      IsDocument( rtl::OUString::createFromAscii( "IsDocument" ) ),
      IsFolder( rtl::OUString::createFromAscii( "IsFolder" ) ),
      DateCreated( rtl::OUString::createFromAscii( "DateCreated" ) ),
      DateModified( rtl::OUString::createFromAscii( "DateModified" ) ),
      Size( rtl::OUString::createFromAscii( "Size" ) ),
      FolderCount( rtl::OUString::createFromAscii( "FolderCount" ) ),
      DocumentCount( rtl::OUString::createFromAscii( "DocumentCount" ) ),
      ContentType( rtl::OUString::createFromAscii( "ContentType" ) ),
      IsReadOnly( rtl::OUString::createFromAscii( "IsReadOnly" ) ),
      FolderContentType( rtl::OUString::createFromAscii( "application/vnd.sun.staroffice.fsys-folder" ) ),
      FileContentType( rtl::OUString::createFromAscii( "application/vnd.sun.staroffice.fsys-file" ) )
{
    // Title
    m_aDefaultProperties.insert( MyProperty( true,
                                             Title,
                                             -1 ,
                                             getCppuType( static_cast< rtl::OUString* >( 0 ) ),
                                             uno::Any(),
                                             beans::PropertyState_DEFAULT_VALUE,
                                             beans::PropertyAttribute::MAYBEVOID
                                             | beans::PropertyAttribute::BOUND ) );


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


    // ContentType
    uno::Any aAny;
    aAny <<= rtl::OUString();
    m_aDefaultProperties.insert( MyProperty( false,
                                             ContentType,
                                             -1 ,
                                             getCppuType( static_cast< rtl::OUString* >( 0 ) ),
                                             aAny,
                                             beans::PropertyState_DEFAULT_VALUE,
                                             beans::PropertyAttribute::MAYBEVOID
                                             | beans::PropertyAttribute::BOUND
                                             | beans::PropertyAttribute::READONLY ) );


    // DateCreated
    m_aDefaultProperties.insert( MyProperty( true,
                                             DateCreated,
                                             -1,
                                             getCppuType( static_cast< util::DateTime* >( 0 ) ),
                                             uno::Any(),
                                             beans::PropertyState_DEFAULT_VALUE,
                                             beans::PropertyAttribute::MAYBEVOID
                                             | beans::PropertyAttribute::BOUND ) );


    // DateModified
    m_aDefaultProperties.insert( MyProperty( true,
                                             DateModified,
                                             -1 ,
                                             getCppuType( static_cast< util::DateTime* >( 0 ) ),
                                             uno::Any(),
                                             beans::PropertyState_DEFAULT_VALUE,
                                             beans::PropertyAttribute::MAYBEVOID
                                             | beans::PropertyAttribute::BOUND ) );


    // DocumentCount
    m_aDefaultProperties.insert( MyProperty( true,
                                             DocumentCount,
                                             -1,
                                             getCppuType( static_cast< sal_Int32* >( 0 ) ),
                                             uno::Any(),
                                             beans::PropertyState_DEFAULT_VALUE,
                                             beans::PropertyAttribute::MAYBEVOID
                                             | beans::PropertyAttribute::BOUND
                                             | beans::PropertyAttribute::READONLY ) );

    // FolderCount
    m_aDefaultProperties.insert( MyProperty( true,
                                             FolderCount,
                                             -1,
                                             getCppuType( static_cast< sal_Int32* >( 0 ) ),
                                             uno::Any(),
                                             beans::PropertyState_DEFAULT_VALUE,
                                             beans::PropertyAttribute::MAYBEVOID
                                             | beans::PropertyAttribute::BOUND
                                             | beans::PropertyAttribute::READONLY ) );

    // Size
    m_aDefaultProperties.insert( MyProperty( true,
                                             Size,
                                             -1,
                                             getCppuType( static_cast< sal_Int64* >( 0 ) ),
                                             uno::Any(),
                                             beans::PropertyState_DEFAULT_VALUE,
                                             beans::PropertyAttribute::MAYBEVOID
                                             | beans::PropertyAttribute::BOUND ) );

    m_aDefaultProperties.insert( MyProperty( true,
                                             IsReadOnly,
                                             -1 ,
                                             getCppuType( static_cast< sal_Bool* >( 0 ) ),
                                             uno::Any(),
                                             beans::PropertyState_DEFAULT_VALUE,
                                             beans::PropertyAttribute::MAYBEVOID
                                             | beans::PropertyAttribute::BOUND ) );


    // Commands
    m_sCommandInfo[0].Name = rtl::OUString::createFromAscii( "getCommandInfo" );
    m_sCommandInfo[0].Handle = -1;
    m_sCommandInfo[0].ArgType = getCppuVoidType();

    m_sCommandInfo[1].Name = rtl::OUString::createFromAscii( "getPropertySetInfo" );
    m_sCommandInfo[1].Handle = -1;
    m_sCommandInfo[1].ArgType = getCppuVoidType();

    m_sCommandInfo[2].Name = rtl::OUString::createFromAscii( "getPropertyValues" );
    m_sCommandInfo[2].Handle = -1;
    m_sCommandInfo[2].ArgType = getCppuType( static_cast< uno::Sequence< beans::Property >* >( 0 ) );

    m_sCommandInfo[3].Name = rtl::OUString::createFromAscii( "setPropertyValues" );
    m_sCommandInfo[3].Handle = -1;
    m_sCommandInfo[3].ArgType = getCppuType( static_cast< uno::Sequence< beans::PropertyValue >* >( 0 ) );

    m_sCommandInfo[4].Name = rtl::OUString::createFromAscii( "open" );
    m_sCommandInfo[4].Handle = -1;
    m_sCommandInfo[4].ArgType = getCppuType( static_cast< OpenCommandArgument* >( 0 ) );

    m_sCommandInfo[5].Name = rtl::OUString::createFromAscii( "transfer" );
    m_sCommandInfo[5].Handle = -1;
    m_sCommandInfo[5].ArgType = getCppuType( static_cast< TransferInfo* >( 0 ) );

    m_sCommandInfo[6].Name = rtl::OUString::createFromAscii( "delete" );
    m_sCommandInfo[6].Handle = -1;
    m_sCommandInfo[6].ArgType = getCppuType( static_cast< sal_Bool* >( 0 ) );

    m_sCommandInfo[7].Name = rtl::OUString::createFromAscii( "insert" );
    m_sCommandInfo[7].Handle = -1;
    m_sCommandInfo[7].ArgType = getCppuType( static_cast< InsertCommandArgument* > ( 0 ) );

    rtl::OUString Store = rtl::OUString::createFromAscii( "com.sun.star.ucb.Store" );
    uno::Reference< XPropertySetRegistryFactory > xRegFac( m_xMultiServiceFactory->createInstance( Store ),
                                                           uno::UNO_QUERY );
    if ( xRegFac.is() )
    {
        // Open/create a registry
        m_xFileRegistry = xRegFac->createPropertySetRegistry( rtl::OUString() );
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

void SAL_CALL
shell::registerNotifier( const rtl::OUString& aUnqPath, Notifier* pNotifier )
{
    vos::OGuard aGuard( m_aMutex );

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
shell::deregisterNotifier( const rtl::OUString& aUnqPath,Notifier* pNotifier )
{
    vos::OGuard aGuard( m_aMutex );

    ContentMap::iterator it = m_aContent.find( aUnqPath );
    if( it == m_aContent.end() )
        return;

    it->second.notifier->remove( pNotifier );

    if( ! it->second.notifier->size() )
        m_aContent.erase( it );
}


/*********************************************************************************/
/*                                                                               */
/*                     load-Implementation                                       */
/*                                                                               */
/*********************************************************************************/

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
/*                     de/associate-Implementation                               */
/*                                                                               */
/*********************************************************************************/


void SAL_CALL
shell::associate( const rtl::OUString& aUnqPath,
                  const rtl::OUString& PropertyName,
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
        throw beans::PropertyExistException();

    {
        vos::OGuard aGuard( m_aMutex );

        ContentMap::iterator it = m_aContent.insert( ContentMap::value_type( aUnqPath,UnqPathData() ) ).first;

        // Load the XPersistentPropertySetInfo and create it, if it does not exist
        load( it,true );

        PropertySet& properties = *(it->second.properties);
        it1 = properties.find( newProperty );
        if( it1 != properties.end() )
            throw beans::PropertyExistException();

        // Property does not exist
        properties.insert( newProperty );
        it->second.xC->addProperty( PropertyName,Attributes,DefaultValue );
    }
    notifyPropertyAdded( getPropertySetListeners( aUnqPath ), PropertyName );
}




void SAL_CALL
shell::deassociate( const rtl::OUString& aUnqPath,
            const rtl::OUString& PropertyName )
  throw( beans::UnknownPropertyException,
     beans::NotRemoveableException,
     uno::RuntimeException )
{
    MyProperty oldProperty( PropertyName );

    shell::PropertySet::iterator it1 = m_aDefaultProperties.find( oldProperty );
    if( it1 != m_aDefaultProperties.end() )
        throw beans::NotRemoveableException();

    vos::OGuard aGuard( m_aMutex );

    ContentMap::iterator it = m_aContent.insert( ContentMap::value_type( aUnqPath,UnqPathData() ) ).first;

    load( it,false );

    PropertySet& properties = *(it->second.properties);

    it1 = properties.find( oldProperty );
    if( it1 == properties.end() )
        throw beans::UnknownPropertyException();

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


void SAL_CALL shell::page( sal_Int32 CommandId,
                           const rtl::OUString& aUnqPath,
                           const uno::Reference< io::XOutputStream >& xOutputStream )
    throw( CommandAbortedException )
{
    uno::Reference< XContentProvider > xProvider( m_pProvider );
    osl::File aFile( aUnqPath );
    osl::FileBase::RC err = aFile.open( OpenFlag_Read );

    if( err != osl::FileBase::E_None )
    {
        aFile.close();
        return;
    }

    const sal_uInt64 bfz = 4*1024;
    sal_Int8 BFF[bfz];
    sal_uInt64 nrc;  // Retrieved number of Bytes;
    sal_Bool no_err;

    do
    {
        no_err = aFile.read( (void*) BFF,bfz,nrc ) == osl::FileBase::E_None;
        if( no_err )
        {
            uno::Sequence< sal_Int8 > seq( BFF,nrc );
            xOutputStream->writeBytes( seq );
        }
        else
            break;
    } while( nrc == bfz );

    aFile.close();

    if( no_err )
        xOutputStream->closeOutput();
    else
        throw CommandAbortedException();
}


/*********************************************************************************/
/*                                                                               */
/*                     open-Implementation                                       */
/*                                                                               */
/*********************************************************************************/



class XInputStream_impl
    : public cppu::OWeakObject,
      public io::XInputStream,
      public io::XSeekable
{
public:
    XInputStream_impl( shell* pMyShell,const rtl::OUString& aUncPath );
    virtual ~XInputStream_impl();

    sal_Bool SAL_CALL CtorSuccess();

    virtual uno::Any SAL_CALL
    queryInterface(
        const uno::Type& rType )
        throw( uno::RuntimeException)
    {
        uno::Any aRet = cppu::queryInterface( rType,
                                              SAL_STATIC_CAST( io::XInputStream*,this ),
                                              SAL_STATIC_CAST( io::XSeekable*,this ) );
        return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
    }

    virtual void SAL_CALL
    acquire(
        void )
        throw( uno::RuntimeException)
    {
        OWeakObject::acquire();
    }

    virtual void SAL_CALL
    release(
        void )
        throw( uno::RuntimeException )
    {
        OWeakObject::release();
    }

    virtual sal_Int32 SAL_CALL
    readBytes(
        uno::Sequence< sal_Int8 >& aData,
        sal_Int32 nBytesToRead )
        throw( io::NotConnectedException,
               io::BufferSizeExceededException,
               io::IOException,
               uno::RuntimeException);

    virtual sal_Int32 SAL_CALL
    readSomeBytes(
        uno::Sequence< sal_Int8 >& aData,
        sal_Int32 nMaxBytesToRead )
        throw( io::NotConnectedException,
               io::BufferSizeExceededException,
               io::IOException,
               uno::RuntimeException);

    virtual void SAL_CALL
    skipBytes(
        sal_Int32 nBytesToSkip )
        throw( io::NotConnectedException,
               io::BufferSizeExceededException,
               io::IOException,
               uno::RuntimeException );

    virtual sal_Int32 SAL_CALL
    available(
        void )
        throw( io::NotConnectedException,
               io::IOException,
               uno::RuntimeException );

    virtual void SAL_CALL
    closeInput(
        void )
        throw( io::NotConnectedException,
               io::IOException,
               uno::RuntimeException );

    virtual void SAL_CALL
    seek(
        sal_Int64 location )
        throw( lang::IllegalArgumentException,
               io::IOException,
               uno::RuntimeException );

    virtual sal_Int64 SAL_CALL
    getPosition(
        void )
        throw( io::IOException,
               uno::RuntimeException );

    virtual sal_Int64 SAL_CALL
    getLength(
        void )
        throw( io::IOException,
               uno::RuntimeException );

private:
    shell*       m_pMyShell;
    uno::Reference< XContentProvider > m_xProvider;
    sal_Bool     m_nIsOpen;
    osl::File    m_aFile;
};



uno::Reference< io::XInputStream > SAL_CALL
shell::open( sal_Int32 CommandId,
             const rtl::OUString& aUnqPath )
{
  XInputStream_impl* xInputStream = new XInputStream_impl( this,aUnqPath );

  if( ! xInputStream->CtorSuccess() ) {
    delete xInputStream;
    xInputStream = 0;
  }
  return uno::Reference< io::XInputStream >( xInputStream );
}


XInputStream_impl::XInputStream_impl( shell* pMyShell,const rtl::OUString& aUncPath )
    : m_pMyShell( pMyShell ),
      m_aFile( aUncPath ),
      m_xProvider( pMyShell->m_pProvider )
{
    if( m_aFile.open( OpenFlag_Read ) != osl::FileBase::E_None )
    {
        m_nIsOpen = false;
        m_aFile.close();
    }
    else
        m_nIsOpen = true;
}


XInputStream_impl::~XInputStream_impl()
{
  closeInput();
}

sal_Bool SAL_CALL XInputStream_impl::CtorSuccess()
{
  return m_nIsOpen;
};


sal_Int32 SAL_CALL
XInputStream_impl::readBytes(
                 uno::Sequence< sal_Int8 >& aData,
                 sal_Int32 nBytesToRead )
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException)
{
    if( ! m_nIsOpen ) throw io::IOException();

    sal_Int8 * buffer;
    try
    {
        buffer = new sal_Int8[nBytesToRead];
    }
    catch( std::bad_alloc )
    {
        if( m_nIsOpen ) m_aFile.close();
        throw io::BufferSizeExceededException();
    }

    sal_uInt64 nrc;
    m_aFile.read( (void* )buffer,sal_uInt64(nBytesToRead),nrc );

    aData = uno::Sequence< sal_Int8 > ( buffer,nrc );
    delete[] buffer;
    return ( sal_Int32 ) nrc;
}

sal_Int32 SAL_CALL
XInputStream_impl::readSomeBytes(
    uno::Sequence< sal_Int8 >& aData,
    sal_Int32 nMaxBytesToRead )
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException)
{
    return readBytes( aData,nMaxBytesToRead );
}


void SAL_CALL
XInputStream_impl::skipBytes(
    sal_Int32 nBytesToSkip )
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException)
{
    m_aFile.setPos( osl_Pos_Current, sal_uInt64( nBytesToSkip ) );
}


sal_Int32 SAL_CALL
XInputStream_impl::available(
    void )
    throw( io::NotConnectedException,
           io::IOException,
           uno::RuntimeException)
{
    return 0;
}


void SAL_CALL
XInputStream_impl::closeInput(
    void )
    throw( io::NotConnectedException,
           io::IOException,
           uno::RuntimeException )
{
    if( m_nIsOpen )
    {
        osl::FileBase::RC err = m_aFile.close();
        if( err != osl::FileBase::E_None )
            throw io::IOException();
        m_nIsOpen = false;
    }
}


void SAL_CALL
XInputStream_impl::seek(
    sal_Int64 location )
    throw( lang::IllegalArgumentException,
           io::IOException,
           uno::RuntimeException )
{
    if( location < 0 )
        throw lang::IllegalArgumentException();
    if( osl::FileBase::E_None != m_aFile.setPos( Pos_Absolut, sal_uInt64( location ) ) )
        throw io::IOException();
}


sal_Int64 SAL_CALL
XInputStream_impl::getPosition(
    void )
    throw( io::IOException,
           uno::RuntimeException )
{
    sal_uInt64 uPos;
    if( osl::FileBase::E_None != m_aFile.getPos( uPos ) )
        throw io::IOException();
    return sal_Int64( uPos );
}

sal_Int64 SAL_CALL
XInputStream_impl::getLength(
    void )
    throw( io::IOException,
           uno::RuntimeException )
{
    osl::FileBase::RC   err;
    sal_uInt64          uCurrentPos, uEndPos;

    err = m_aFile.getPos( uCurrentPos );
    if( err != osl::FileBase::E_None )
        throw io::IOException();

    err = m_aFile.setPos( Pos_End, 0 );
    if( err != osl::FileBase::E_None )
        throw io::IOException();

    err = m_aFile.getPos( uEndPos );
    if( err != osl::FileBase::E_None )
        throw io::IOException();

    err = m_aFile.setPos( Pos_Absolut, uCurrentPos );
    if( err != osl::FileBase::E_None )
        throw io::IOException();
    else
        return sal_Int64( uEndPos );
}


/*********************************************************************************/
/*                                                                               */
/*                     open for read/write access-Implementation                 */
/*                                                                               */
/*********************************************************************************/



class XStream_impl
    : public cppu::OWeakObject,
      public io::XStream,
      public io::XSeekable
{
public:
    XStream_impl( shell* pMyShell,const rtl::OUString& aUncPath );
    sal_Bool SAL_CALL CtorSuccess();
    virtual ~XStream_impl();


    virtual uno::Any SAL_CALL
    queryInterface(
        const uno::Type& rType )
        throw( uno::RuntimeException)
    {
        uno::Any aRet = cppu::queryInterface( rType,
                                              SAL_STATIC_CAST( io::XStream*,this ),
                                              SAL_STATIC_CAST( io::XSeekable*,this ) );
        return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
    }

    virtual void SAL_CALL
    acquire(
        void )
        throw( uno::RuntimeException)
    {
        OWeakObject::acquire();
    }

    virtual void SAL_CALL
    release(
        void )
        throw( uno::RuntimeException )
    {
        OWeakObject::release();
    }

    // XStream

    class XInputStreamForStream
        : public cppu::OWeakObject,
          public io::XInputStream,
          public io::XSeekable
    {
    public:
        XInputStreamForStream( XStream_impl* xPtr )
            : m_xPtr( xPtr ),
              m_bOpen( true )
        {
            m_xPtr->acquire();
        }
        ~XInputStreamForStream()
        {
            m_xPtr->release();
        }

        uno::Any SAL_CALL
        queryInterface(
            const uno::Type& rType )
            throw( uno::RuntimeException)
        {
            uno::Any aRet = cppu::queryInterface( rType,
                                                  SAL_STATIC_CAST( io::XInputStream*,this ),
                                                  SAL_STATIC_CAST( io::XSeekable*,this ) );
            return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
        }


        void SAL_CALL
        acquire(
            void )
            throw( uno::RuntimeException)
        {
            OWeakObject::acquire();
        }

        void SAL_CALL
        release(
            void )
            throw( uno::RuntimeException )
        {
            OWeakObject::release();
        }

        sal_Int32 SAL_CALL
        readBytes(
            uno::Sequence< sal_Int8 >& aData,
            sal_Int32 nBytesToRead )
            throw( io::NotConnectedException,
                   io::BufferSizeExceededException,
                   io::IOException,
                   uno::RuntimeException)
        {
            if( m_bOpen )
                return m_xPtr->readBytes( aData,nBytesToRead );
            else
                throw io::IOException();
        }

        sal_Int32 SAL_CALL
        readSomeBytes(
            uno::Sequence< sal_Int8 >& aData,
            sal_Int32 nMaxBytesToRead )
            throw( io::NotConnectedException,
                   io::BufferSizeExceededException,
                   io::IOException,
                   uno::RuntimeException)
        {
            if( m_bOpen )
                return m_xPtr->readSomeBytes( aData,nMaxBytesToRead );
            else
                throw io::IOException();
        }

        void SAL_CALL
        skipBytes(
            sal_Int32 nBytesToSkip )
            throw( io::NotConnectedException,
                   io::BufferSizeExceededException,
                   io::IOException,
                   uno::RuntimeException )
        {
            if( m_bOpen )
                m_xPtr->skipBytes( nBytesToSkip );
            else
                throw io::IOException();
        }

        sal_Int32 SAL_CALL
        available(
            void )
            throw( io::NotConnectedException,
                   io::IOException,
                   uno::RuntimeException )
        {
            if( m_bOpen )
                return m_xPtr->available();
            else
                return 0;
        }

        void SAL_CALL
        closeInput(
            void )
            throw( io::NotConnectedException,
                   io::IOException,
                   uno::RuntimeException )
        {
            m_xPtr->closeInput();
            m_bOpen = false;
        }

        void SAL_CALL
        seek(
            sal_Int64 location )
            throw( lang::IllegalArgumentException,
                   io::IOException,
                   uno::RuntimeException )
        {
            if( m_bOpen )
                m_xPtr->seek( location );
            else
                throw io::IOException();
        }

        sal_Int64 SAL_CALL
        getPosition(
            void )
            throw( io::IOException,
                   uno::RuntimeException )
        {
            if( m_bOpen )
                return m_xPtr->getPosition();
            else
                throw io::IOException();
        }

        sal_Int64 SAL_CALL
        getLength(
            void )
            throw( io::IOException,
                   uno::RuntimeException )
        {
            if( m_bOpen )
                return m_xPtr->getLength();
            else
                throw io::IOException();
        }


    private:
        sal_Bool      m_bOpen;
        XStream_impl* m_xPtr;
    };


    virtual uno::Reference< io::XInputStream > SAL_CALL
    getInputStream(  )
        throw( uno::RuntimeException)
    {
        if( ! m_xInputStream.is() )
        {
            XInputStreamForStream* p = new XInputStreamForStream( this );
            m_xInputStream = uno::Reference< io::XInputStream >( p );
        }

        return m_xInputStream;
    }




    class XOutputStreamForStream
        : public cppu::OWeakObject,
          public io::XOutputStream,
          public io::XSeekable
    {
    public:
        XOutputStreamForStream( XStream_impl* xPtr )
            : m_xPtr( xPtr ),
              m_bOpen( true )
        {
            m_xPtr->acquire();
        }
        ~XOutputStreamForStream()
        {
            m_xPtr->release();
        }

        uno::Any SAL_CALL
        queryInterface(
            const uno::Type& rType )
            throw( uno::RuntimeException)
        {
            uno::Any aRet = cppu::queryInterface( rType,
                                                  SAL_STATIC_CAST( io::XOutputStream*,this ),
                                                  SAL_STATIC_CAST( io::XSeekable*,this ) );
            return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
        }


        void SAL_CALL
        acquire(
            void )
            throw( uno::RuntimeException)
        {
            OWeakObject::acquire();
        }

        void SAL_CALL
        release(
            void )
            throw( uno::RuntimeException )
        {
            OWeakObject::release();
        }


        void SAL_CALL
        seek(
            sal_Int64 location )
            throw( lang::IllegalArgumentException,
                   io::IOException,
                   uno::RuntimeException )
        {
            if( m_bOpen )
                m_xPtr->seek( location );
            else
                throw io::IOException();
        }

        sal_Int64 SAL_CALL
        getPosition(
            void )
            throw( io::IOException,
                   uno::RuntimeException )
        {
            if( m_bOpen )
                return m_xPtr->getPosition();
            else
                throw io::IOException();
        }

        sal_Int64 SAL_CALL
        getLength(
            void )
            throw( io::IOException,
                   uno::RuntimeException )
        {
            if( m_bOpen )
                return m_xPtr->getLength();
            else
                throw io::IOException();
        }


        void SAL_CALL
        writeBytes( const uno::Sequence< sal_Int8 >& aData )
            throw( io::NotConnectedException,
                   io::BufferSizeExceededException,
                   io::IOException,
                   uno::RuntimeException)
        {
            if( m_bOpen )
                m_xPtr->writeBytes( aData );
            else
                throw io::IOException();
        }

        void SAL_CALL
        flush(  )
            throw( io::NotConnectedException,
                   io::BufferSizeExceededException,
                   io::IOException,
                   uno::RuntimeException)
        {
            if( m_bOpen )
                m_xPtr->flush();
            else
                throw io::IOException();
        }

        void SAL_CALL
        closeOutput(  )
            throw( io::NotConnectedException,
                   io::BufferSizeExceededException,
                   io::IOException,
                   uno::RuntimeException)
        {
            m_xPtr->closeOutput();
            m_bOpen = false;
        }

    private:
        sal_Bool      m_bOpen;
        XStream_impl* m_xPtr;
    };




    virtual uno::Reference< io::XOutputStream > SAL_CALL
    getOutputStream(  )
        throw( uno::RuntimeException )
    {
        if( ! m_xOutputStream.is() )
        {
            XOutputStreamForStream* p = new XOutputStreamForStream( this );
            m_xOutputStream = uno::Reference< io::XOutputStream >( p );
        }

        return m_xOutputStream;
    }


    // Implementation methods

    sal_Int32 SAL_CALL
    readBytes(
        uno::Sequence< sal_Int8 >& aData,
        sal_Int32 nBytesToRead )
        throw( io::NotConnectedException,
               io::BufferSizeExceededException,
               io::IOException,
               uno::RuntimeException);

    sal_Int32 SAL_CALL
    readSomeBytes(
        uno::Sequence< sal_Int8 >& aData,
        sal_Int32 nMaxBytesToRead )
        throw( io::NotConnectedException,
               io::BufferSizeExceededException,
               io::IOException,
               uno::RuntimeException);

    void SAL_CALL
    writeBytes( const uno::Sequence< sal_Int8 >& aData )
        throw( io::NotConnectedException,
               io::BufferSizeExceededException,
               io::IOException,
               uno::RuntimeException);

    void SAL_CALL
    skipBytes(
        sal_Int32 nBytesToSkip )
        throw( io::NotConnectedException,
               io::BufferSizeExceededException,
               io::IOException,
               uno::RuntimeException );

    sal_Int32 SAL_CALL
    available(
        void )
        throw( io::NotConnectedException,
               io::IOException,
               uno::RuntimeException );

    void SAL_CALL
    flush()
        throw( io::NotConnectedException,
               io::BufferSizeExceededException,
               io::IOException,
               uno::RuntimeException);

    void SAL_CALL
    closeStream(
        void )
        throw( io::NotConnectedException,
               io::IOException,
               uno::RuntimeException );

    void SAL_CALL
    closeInput(
        void )
        throw( io::NotConnectedException,
               io::IOException,
               uno::RuntimeException );

    void SAL_CALL
    closeOutput(
        void )
        throw( io::NotConnectedException,
               io::IOException,
               uno::RuntimeException );

    void SAL_CALL
    seek(
        sal_Int64 location )
        throw( lang::IllegalArgumentException,
               io::IOException,
               uno::RuntimeException );

    sal_Int64 SAL_CALL
    getPosition(
        void )
        throw( io::IOException,
               uno::RuntimeException );

    sal_Int64 SAL_CALL
    getLength(
        void )
        throw( io::IOException,
               uno::RuntimeException );

private:
    sal_Bool                           m_bInputStreamClosed;
    uno::Reference< io::XInputStream > m_xInputStream;

    sal_Bool                            m_bOutputStreamClosed;
    uno::Reference< io::XOutputStream > m_xOutputStream;

    shell*       m_pMyShell;
    uno::Reference< XContentProvider > m_xProvider;
    sal_Bool     m_nIsOpen;
    osl::File    m_aFile;
};



uno::Reference< io::XStream > SAL_CALL
shell::open_rw( sal_Int32 CommandId,
                const rtl::OUString& aUnqPath )
{
    XStream_impl* xStream = new XStream_impl( this,aUnqPath );

    if( ! xStream->CtorSuccess() )
    {
        delete xStream;
        xStream = 0;
    }
    return uno::Reference< io::XStream >( xStream );
}



XStream_impl::XStream_impl( shell* pMyShell,const rtl::OUString& aUncPath )
    : m_pMyShell( pMyShell ),
      m_aFile( aUncPath ),
      m_xProvider( m_pMyShell->m_pProvider ),
      m_xInputStream( 0 ),
      m_xOutputStream( 0 ),
      m_bInputStreamClosed( false ),
      m_bOutputStreamClosed( false )
{
    if( m_aFile.open( OpenFlag_Read | OpenFlag_Write ) != osl::FileBase::E_None )
    {
        m_nIsOpen = false;
        m_aFile.close();
    }
    else
        m_nIsOpen = true;
}


XStream_impl::~XStream_impl()
{
    closeStream();
}

sal_Bool SAL_CALL XStream_impl::CtorSuccess()
{
    return m_nIsOpen;
}


sal_Int32 SAL_CALL
XStream_impl::readBytes(
    uno::Sequence< sal_Int8 >& aData,
    sal_Int32 nBytesToRead )
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException)
{
    if( ! m_nIsOpen )
        throw io::IOException();

    sal_Int8 * buffer;
    try
    {
        buffer = new sal_Int8[nBytesToRead];
    }
    catch( std::bad_alloc )
    {
        if( m_nIsOpen ) m_aFile.close();
        throw io::BufferSizeExceededException();
    }

    sal_uInt64 nrc;
    m_aFile.read( (void* )buffer,sal_uInt64(nBytesToRead),nrc );

    aData = uno::Sequence< sal_Int8 > ( buffer,nrc );
    delete[] buffer;
    return ( sal_Int32 ) nrc;
}


sal_Int32 SAL_CALL
XStream_impl::readSomeBytes(
    uno::Sequence< sal_Int8 >& aData,
    sal_Int32 nMaxBytesToRead )
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException)
{
    return readBytes( aData,nMaxBytesToRead );
}


void SAL_CALL
XStream_impl::skipBytes(
    sal_Int32 nBytesToSkip )
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException)
{
    m_aFile.setPos( osl_Pos_Current, sal_uInt64( nBytesToSkip ) );
}


sal_Int32 SAL_CALL
XStream_impl::available(
    void )
    throw( io::NotConnectedException,
           io::IOException,
           uno::RuntimeException)
{
    return 0;
}


void SAL_CALL
XStream_impl::writeBytes( const uno::Sequence< sal_Int8 >& aData )
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException)
{
    sal_Int32 length = aData.getLength();
    sal_uInt64 nWrittenBytes;
    if( length )
    {
        const sal_Int8* p = aData.getConstArray();
        m_aFile.write( ((void*)(p)),
                       sal_uInt64( length ),
                       nWrittenBytes );
        if( nWrittenBytes != length )
        {
            // DBG_ASSERT( "Write Operation not successful" );
        }
    }
}


void SAL_CALL
XStream_impl::closeStream(
    void )
    throw( io::NotConnectedException,
           io::IOException,
           uno::RuntimeException )
{
    if( m_nIsOpen )
    {
        osl::FileBase::RC err = m_aFile.close();
        if( err != osl::FileBase::E_None ) throw io::IOException();
        m_nIsOpen = false;
    }
}

void SAL_CALL
XStream_impl::closeInput(
    void )
    throw( io::NotConnectedException,
           io::IOException,
           uno::RuntimeException )
{
    m_bInputStreamClosed = true;
    // if( m_bInputStreamClosed && m_bOutputStreamClosed )
    closeStream();
}


void SAL_CALL
XStream_impl::closeOutput(
    void )
    throw( io::NotConnectedException,
           io::IOException,
           uno::RuntimeException )
{
    m_bOutputStreamClosed = true;
//  if( m_bInputStreamClosed && m_bOutputStreamClosed )
    closeStream();
}


void SAL_CALL
XStream_impl::seek(
    sal_Int64 location )
    throw( lang::IllegalArgumentException,
           io::IOException,
           uno::RuntimeException )
{
    if( location < 0 )
        throw lang::IllegalArgumentException();
    if( osl::FileBase::E_None != m_aFile.setPos( Pos_Absolut, sal_uInt64( location ) ) )
        throw io::IOException();
}


sal_Int64 SAL_CALL
XStream_impl::getPosition(
    void )
    throw( io::IOException,
           uno::RuntimeException )
{
    sal_uInt64 uPos;
    if( osl::FileBase::E_None != m_aFile.getPos( uPos ) )
        throw io::IOException();
    return sal_Int64( uPos );
}

sal_Int64 SAL_CALL
XStream_impl::getLength(
    void )
    throw( io::IOException,
           uno::RuntimeException )
{
    osl::FileBase::RC   err;
    sal_uInt64          uCurrentPos, uEndPos;

    err = m_aFile.getPos( uCurrentPos );
    if( err != osl::FileBase::E_None )
        throw io::IOException();

    err = m_aFile.setPos( Pos_End, 0 );
    if( err != osl::FileBase::E_None )
        throw io::IOException();

    err = m_aFile.getPos( uEndPos );
    if( err != osl::FileBase::E_None )
        throw io::IOException();

    err = m_aFile.setPos( Pos_Absolut, uCurrentPos );
    if( err != osl::FileBase::E_None )
        throw io::IOException();
    else
        return sal_Int64( uEndPos );

}


void SAL_CALL
XStream_impl::flush()
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException )
{
    return;
}


/*********************************************************************************/
/*                                                                               */
/*                          info_c implementation                                */
/*                                                                               */
/*********************************************************************************/


class fileaccess::XCommandInfo_impl
    : public cppu::OWeakObject,
      public XCommandInfo
{
public:

    XCommandInfo_impl( shell* pMyShell, const rtl::OUString& aUnqPath );
    virtual ~XCommandInfo_impl();

    // XInterface
    virtual uno::Any SAL_CALL
    queryInterface(
        const uno::Type& aType )
        throw( uno::RuntimeException);

    virtual void SAL_CALL
    acquire(
        void )
        throw( uno::RuntimeException);

    virtual void SAL_CALL
    release(
        void )
        throw( uno::RuntimeException);

    // XCommandInfo

    virtual uno::Sequence< CommandInfo > SAL_CALL
    getCommands(
        void )
        throw( uno::RuntimeException);

    virtual CommandInfo SAL_CALL
    getCommandInfoByName(
        const rtl::OUString& Name )
        throw( UnsupportedCommandException,
               uno::RuntimeException);

    virtual CommandInfo SAL_CALL
    getCommandInfoByHandle(
        sal_Int32 Handle )
        throw( UnsupportedCommandException,
               uno::RuntimeException );

    virtual sal_Bool SAL_CALL
    hasCommandByName(
        const rtl::OUString& Name )
        throw( uno::RuntimeException );

    virtual sal_Bool SAL_CALL
    hasCommandByHandle(
        sal_Int32 Handle )
        throw( uno::RuntimeException );
private:

    shell* m_pMyShell;
    uno::Reference< XContentProvider > m_xProvider;
};


uno::Reference< XCommandInfo > SAL_CALL
shell::info_c( sal_Int32 CommandId, const rtl::OUString& aUnqPath )
{
    XCommandInfo_impl* p = new XCommandInfo_impl( this,aUnqPath );
    return uno::Reference< XCommandInfo >( p );
}


XCommandInfo_impl::XCommandInfo_impl( shell* pMyShell,const rtl::OUString& aUnqPath )
    : m_pMyShell( pMyShell ),
      m_xProvider( pMyShell->m_pProvider )
{
}

XCommandInfo_impl::~XCommandInfo_impl()
{
}



void SAL_CALL
XCommandInfo_impl::acquire(
                 void )
  throw( uno::RuntimeException )
{
  OWeakObject::acquire();
}


void SAL_CALL
XCommandInfo_impl::release(
    void )
  throw( uno::RuntimeException )
{
    OWeakObject::release();
}


uno::Any SAL_CALL
XCommandInfo_impl::queryInterface(
                    const uno::Type& rType )
  throw( uno::RuntimeException )
{
    uno::Any aRet = cppu::queryInterface( rType,
                                          SAL_STATIC_CAST( XCommandInfo*,this) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


uno::Sequence< CommandInfo > SAL_CALL
XCommandInfo_impl::getCommands(
    void )
    throw( uno::RuntimeException )
{
    return m_pMyShell->m_sCommandInfo;
}


CommandInfo SAL_CALL
XCommandInfo_impl::getCommandInfoByName(
    const rtl::OUString& aName )
    throw( UnsupportedCommandException,
           uno::RuntimeException)
{
    for( sal_Int32 i = 0; i < m_pMyShell->m_sCommandInfo.getLength(); i++ )
        if( m_pMyShell->m_sCommandInfo[i].Name == aName )
            return m_pMyShell->m_sCommandInfo[i];

    throw UnsupportedCommandException();
}


CommandInfo SAL_CALL
XCommandInfo_impl::getCommandInfoByHandle(
    sal_Int32 Handle )
    throw( UnsupportedCommandException,
           uno::RuntimeException )
{
    for( sal_Int32 i = 0; i < m_pMyShell->m_sCommandInfo.getLength(); ++i )
        if( m_pMyShell->m_sCommandInfo[i].Handle == Handle )
            return m_pMyShell->m_sCommandInfo[i];

    throw UnsupportedCommandException();
}


sal_Bool SAL_CALL
XCommandInfo_impl::hasCommandByName(
    const rtl::OUString& aName )
    throw( uno::RuntimeException )
{
    for( sal_Int32 i = 0; i < m_pMyShell->m_sCommandInfo.getLength(); ++i )
        if( m_pMyShell->m_sCommandInfo[i].Name == aName )
            return true;

    return false;
}


sal_Bool SAL_CALL
XCommandInfo_impl::hasCommandByHandle(
    sal_Int32 Handle )
    throw( uno::RuntimeException )
{
    for( sal_Int32 i = 0; i < m_pMyShell->m_sCommandInfo.getLength(); ++i )
        if( m_pMyShell->m_sCommandInfo[i].Handle == Handle )
            return true;

    return false;
}


/*********************************************************************************/
/*                                                                               */
/*                     info_p-Implementation                                     */
/*                                                                               */
/*********************************************************************************/




uno::Reference< beans::XPropertySetInfo > SAL_CALL
shell::info_p( sal_Int32 CommandId,
               const rtl::OUString& aUnqPath )
{
    vos::OGuard aGuard( m_aMutex );
    XPropertySetInfo_impl* p = new XPropertySetInfo_impl( this,aUnqPath );
    return uno::Reference< beans::XPropertySetInfo >( p );
}






/*********************************************************************************/
/*                                                                               */
/*                     setv-Implementation                                       */
/*                                                                               */
/*********************************************************************************/

void SAL_CALL
shell::setv( sal_Int32 CommandId,
             const rtl::OUString& aUnqPath,
             const uno::Sequence< beans::PropertyValue >& values )
{
    vos::OGuard aGuard( m_aMutex );

    sal_Int32 propChanged = 0;
    uno::Sequence< beans::PropertyChangeEvent > seqChanged( values.getLength() );

    shell::ContentMap::iterator it = m_aContent.find( aUnqPath );
    PropertySet& properties = *( it->second.properties );
    shell::PropertySet::iterator it1;
    uno::Any aAny;

    for( sal_Int32 i = 0; i < values.getLength(); ++i )
    {
        MyProperty toset( values[i].Name );
        it1 = properties.find( toset );
        if( it1 == properties.end() ) continue;

        aAny = it1->getValue();
        if( aAny == values[i].Value ) continue;

        seqChanged[ propChanged   ].PropertyName = values[i].Name;
        seqChanged[ propChanged   ].PropertyHandle   = -1;
        seqChanged[ propChanged   ].Further   = false;
        seqChanged[ propChanged   ].OldValue <<= aAny;
        seqChanged[ propChanged++ ].NewValue = values[i].Value;

        it1->setValue( values[i].Value );

        if( ! it1->IsNative() )
        {
            // Also put logical properties into storage
            if( !it->second.xS.is() )
                load( it,true );

            // Special logic for ContentType
            if( ( values[i].Name == ContentType ) &&
                it1->getState() == beans::PropertyState_DEFAULT_VALUE )
            {
                it1->setState( beans::PropertyState_DIRECT_VALUE );
                it->second.xC->addProperty( values[i].Name,
                                            beans::PropertyAttribute::MAYBEVOID,
                                            values[i].Value );
            }

            it->second.xS->setPropertyValue( values[i].Name,values[i].Value );
        }
        else
        {
            // Setting of physical file properties
        }
    }

    if( propChanged )
    {
        seqChanged.realloc( propChanged );
        notifyPropertyChanges( getPropertyChangeNotifier( aUnqPath ),seqChanged );
    }
}


/*********************************************************************************/
/*                                                                               */
/*                     getv-Implementation                                       */
/*                                                                               */
/*********************************************************************************/








uno::Reference< sdbc::XRow > SAL_CALL
shell::getv( sal_Int32 CommandId,
         const rtl::OUString& aUnqPath,
         const uno::Sequence< beans::Property >& properties )
{
    uno::Sequence< uno::Any > seq( properties.getLength() );

    sal_Int32 n_Mask;
    getMaskFromProperties( n_Mask,properties );
    osl::FileStatus aFileStatus( n_Mask );

    osl::DirectoryItem aDirItem;
    osl::DirectoryItem::get( aUnqPath,aDirItem );
    aDirItem.getFileStatus( aFileStatus );

    {
        vos::OGuard aGuard( m_aMutex );

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




/*********************************************************************************/
/*                                                                               */
/*                       ls-Implementation                                       */
/*                                                                               */
/*********************************************************************************/







uno::Reference< XDynamicResultSet > SAL_CALL
shell::ls( sal_Int32 CommandId,
           const rtl::OUString& aUnqPath,
           const sal_Int32 OpenMode,
           const uno::Sequence< beans::Property >& seq,
           const uno::Sequence< NumberedSortingInfo >& seqSort )
{
    XResultSet_impl* p = new XResultSet_impl( this,aUnqPath,OpenMode,seq,seqSort );

    if( ! p->CtorSuccess() )
    {
        delete p; p = 0;
    }

    return uno::Reference< XDynamicResultSet > ( p );
}







/********************************************************************************/
/*                                                                              */
/*                         transfer-commandos                                   */
/*                                                                              */
/********************************************************************************/



// Returns true if dstUnqPath is a child from srcUnqPath or both are equal
sal_Bool SAL_CALL isChild( const rtl::OUString& srcUnqPath,
                           const rtl::OUString& dstUnqPath )
{
    static sal_Unicode slash = '/';
    // Simple lexical comparison
    sal_Int32 srcL = srcUnqPath.getLength();
    sal_Int32 dstL = dstUnqPath.getLength();

    return (
        ( srcUnqPath == dstUnqPath )
        ||
        ( ( dstL > srcL )
          &&
          ( srcUnqPath.compareTo( dstUnqPath, srcL ) == 0 )
          &&
          ( dstUnqPath[ srcL ] == slash ) )
    );
}


// Changes the prefix in name
rtl::OUString SAL_CALL newName(
    const rtl::OUString& aNewPrefix,
    const rtl::OUString& aOldPrefix,
    const rtl::OUString& old_Name )
{
    sal_Int32 srcL = aOldPrefix.getLength();

    rtl::OUString new_Name = old_Name.copy( srcL );
    new_Name = ( aNewPrefix + new_Name );
    return new_Name;
}


rtl::OUString SAL_CALL getTitle( const rtl::OUString& aPath )
{
    sal_Unicode slash = '/';
    sal_Int32 lastIndex = aPath.lastIndexOf( slash );
    return aPath.copy( lastIndex + 1 );
}


void SAL_CALL
shell::erasePersistentSet( const rtl::OUString& aUnqPath,
                           sal_Bool withChilds )
{
    if( ! m_xFileRegistry.is() )
    {
        VOS_ASSERT( m_xFileRegistry.is() );
    }

    uno::Sequence< rtl::OUString > seqNames;

    if( withChilds )
    {
        uno::Reference< container::XNameAccess > xName( m_xFileRegistry,uno::UNO_QUERY );
        seqNames = xName->getElementNames();
    }

    sal_Int32 count = withChilds ? seqNames.getLength() : 1;

    rtl::OUString
        old_Name = aUnqPath;

    for( sal_Int32 j = 0; j < count; ++j )
    {
        if( withChilds  && ! ( ::isChild( old_Name,seqNames[j] ) ) )
            continue;

        if( withChilds )
        {
            old_Name = seqNames[j];
        }

        {
            // Release possible references
            vos::OGuard aGuard( m_aMutex );
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



void SAL_CALL
shell::copyPersistentSet( const rtl::OUString& srcUnqPath,
                          const rtl::OUString& dstUnqPath,
                          sal_Bool withChilds )
{
    if( ! m_xFileRegistry.is() )
    {
        VOS_ASSERT( m_xFileRegistry.is() );
    }

    uno::Sequence< rtl::OUString > seqNames;

    if( withChilds )
    {
        uno::Reference< container::XNameAccess > xName( m_xFileRegistry,uno::UNO_QUERY );
        seqNames = xName->getElementNames();
    }

    sal_Int32 count = withChilds ? seqNames.getLength() : 1;

    rtl::OUString
        old_Name = srcUnqPath,
        new_Name = dstUnqPath;

    for( sal_Int32 j = 0; j < count; ++j )
    {
        if( withChilds  && ! ( ::isChild( srcUnqPath,seqNames[j] ) ) )
            continue;

        if( withChilds )
        {
            old_Name = seqNames[j];
            new_Name = ::newName( dstUnqPath,srcUnqPath,old_Name );
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


// Moves "srcUnqPath" to "dstUnqPath/NewTitle"
void SAL_CALL
shell::move( sal_Int32 CommandId,
             const rtl::OUString srcUnqPath,
             const rtl::OUString dstUnqPathIn,
             const sal_Int32 NameClash )
    throw( CommandAbortedException )
{
    rtl::OUString dstUnqPath( dstUnqPathIn );

    if( dstUnqPath == srcUnqPath )    // Nothing left to be done
        return;

    // Moving file or folder ?
    osl::DirectoryItem aItem;
    osl::FileBase::RC err = osl::DirectoryItem::get( srcUnqPath,aItem );
    if( err )
        throw CommandAbortedException();
    osl::FileStatus aStatus( FileStatusMask_Type );
    aItem.getFileStatus( aStatus );

    sal_Bool isDocument;
    if( aStatus.isValid( FileStatusMask_Type ) )
        isDocument = aStatus.getFileType() == osl::FileStatus::Regular;

    err = osl::File::move( srcUnqPath,dstUnqPath );   // Why, the hell, is this not all what has to be done?

    if( err == osl::FileBase::E_EXIST )
    {
        switch( NameClash )
        {
            case NameClash::KEEP:
                return;
                break;
            case NameClash::OVERWRITE:
            {
                sal_Int32 IsWhat = isDocument ? -1 : 1;
//              don't call shell::remove because that function will send a deleted hint
//              for dstUnqPath which isn't right. dstUnqPath will exist again after the
//              call to osl::File::move. call osl::File::remove() instead.
//              remove( CommandId,dstUnqPath,IsWhat );
                osl::File::remove( dstUnqPath );
                err = osl::File::move( srcUnqPath,dstUnqPath );
            }
            break;
            case NameClash::RENAME:
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
                rtl::OUString newDstUnqPath;

                do
                {
                    newDstUnqPath = dstUnqPath;

                    rtl::OUString aPostFix( rtl::OUString::createFromAscii( "_" ) );
                    aPostFix += rtl::OUString::valueOf( ++nTry );

                    newDstUnqPath = newDstUnqPath.replaceAt( nPos, 0, aPostFix );

                    err = osl::File::move( srcUnqPath,newDstUnqPath );
                }
                while ( ( err == osl::FileBase::E_EXIST ) && ( nTry < 10000 ) );

                if ( err )
                {
                    VOS_ENSURE( sal_False,
                                "shell::move - Unable to resolve name clash" );
                    throw CommandAbortedException();
                }
                else
                    dstUnqPath = newDstUnqPath;

                break;
            }
            case NameClash::ERROR:
                break;
            default:
                break;
        }
    }

    if( err )
        throw CommandAbortedException();


    copyPersistentSet( srcUnqPath,dstUnqPath, !isDocument );

    rtl::OUString aDstParent = getParentName( dstUnqPath );
    rtl::OUString aDstTitle  = getTitle( dstUnqPath );

    rtl::OUString aSrcParent = getParentName( srcUnqPath );
    rtl::OUString aSrcTitle  = getTitle( srcUnqPath );

    notifyInsert( getContentEventListeners( aDstParent ),dstUnqPath );
    if(  aDstParent != aSrcParent )
        notifyContentRemoved( getContentEventListeners( aSrcParent ),srcUnqPath );

    notifyContentExchanged( getContentExchangedEventListeners( srcUnqPath,dstUnqPath, !isDocument ) );

/*
  if( aSrcTitle != aDstTitle )
  {
  uno::Sequence< beans::PropertyChangeEvent > seq(1);
  seq[0].PropertyName = Title;
  seq[0].Further = false;
  seq[0].PropertyHandle = -1;
  seq[0].OldValue <<= aSrcTitle;
  seq[0].NewValue <<= aDstTitle;
  notifyPropertyChanges( getPropertyChangeNotifier( dstUnqPath ),seq );
  }
*/
    erasePersistentSet( srcUnqPath, !isDocument );
}


osl::FileBase::RC SAL_CALL
shell::copy_recursive( const rtl::OUString& srcUnqPath,
                       const rtl::OUString& dstUnqPath,
                       sal_Int32 TypeToCopy )
{
    osl::FileBase::RC err;

    if( TypeToCopy == -1 ) // Document
    {
        err = osl::File::copy( srcUnqPath,dstUnqPath );
    }
    else if( TypeToCopy == +1 ) // Folder
    {
        osl::Directory aDir( srcUnqPath );
        aDir.open();

        err = osl::Directory::create( dstUnqPath );
        osl::FileBase::RC next = err;
        if( err == osl::FileBase::E_None )
        {
            sal_Int32 n_Mask = FileStatusMask_FilePath | FileStatusMask_FileName | FileStatusMask_Type;

            osl::DirectoryItem aDirItem;

            while( ( next = aDir.getNextItem( aDirItem ) ) == osl::FileBase::E_None )
            {
                sal_Bool IsDocument;
                osl::FileStatus aFileStatus( n_Mask );
                aDirItem.getFileStatus( aFileStatus );
                if( aFileStatus.isValid( FileStatusMask_Type ) )
                    IsDocument = aFileStatus.getFileType() == osl::FileStatus::Regular;

                // Getting the information for the next recursive copy
                sal_Int32 newTypeToCopy = IsDocument ? -1 : +1;

                rtl::OUString newSrcUnqPath;
                if( aFileStatus.isValid( FileStatusMask_FilePath ) )
                    newSrcUnqPath = aFileStatus.getFilePath();

                rtl::OUString newDstUnqPath = dstUnqPath;
                rtl::OUString tit;
                if( aFileStatus.isValid( FileStatusMask_FileName ) )
                    tit = aFileStatus.getFileName();
                if( newDstUnqPath.lastIndexOf( sal_Unicode('/') ) != newDstUnqPath.getLength()-1 )
                    newDstUnqPath += rtl::OUString::createFromAscii( "/" );
                newDstUnqPath += tit;

                if ( newSrcUnqPath != dstUnqPath )
                    copy_recursive( newSrcUnqPath,newDstUnqPath,newTypeToCopy );
            }

            if( next != osl::FileBase::E_NOENT )
                err = osl::FileBase::E_INVAL;
        }
        aDir.close();
    }

    return err;
}


void SAL_CALL
shell::copy(
    sal_Int32 CommandId,
    const rtl::OUString srcUnqPath,
    const rtl::OUString dstUnqPathIn,
    sal_Int32 NameClash )
    throw( CommandAbortedException )
{
    rtl::OUString dstUnqPath( dstUnqPathIn );

    if( dstUnqPath == srcUnqPath )    // Nothing left to be done
        return;

    // Moving file or folder ?
    osl::DirectoryItem aItem;
    osl::FileBase::RC err = osl::DirectoryItem::get( srcUnqPath,aItem );
    if( err )
        throw CommandAbortedException();
    osl::FileStatus aStatus( FileStatusMask_Type );
    aItem.getFileStatus( aStatus );

    sal_Bool isDocument;
    if( aStatus.isValid( FileStatusMask_Type ) )
        isDocument = aStatus.getFileType() == osl::FileStatus::Regular;

    sal_Int32 IsWhat = isDocument ? -1 : 1;

    err = copy_recursive( srcUnqPath,dstUnqPath,IsWhat );

    if( err == osl::FileBase::E_EXIST )
    {
        switch( NameClash )
        {
            case NameClash::KEEP:
                return;
                break;
            case NameClash::OVERWRITE:
            {
                remove( CommandId,dstUnqPath,IsWhat );
                err = copy_recursive( srcUnqPath,dstUnqPath,IsWhat );
            }
            break;
            case NameClash::RENAME:
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
                rtl::OUString newDstUnqPath;

                do
                {
                    newDstUnqPath = dstUnqPath;

                    rtl::OUString aPostFix( rtl::OUString::createFromAscii( "_" ) );
                    aPostFix += rtl::OUString::valueOf( ++nTry );

                    newDstUnqPath = newDstUnqPath.replaceAt( nPos, 0, aPostFix );

                    err = copy_recursive( srcUnqPath, dstUnqPath, IsWhat );
                }
                while ( ( err == osl::FileBase::E_EXIST ) && ( nTry < 10000 ) );

                if ( err )
                {
                    VOS_ENSURE( sal_False,
                                "shell::copy - Unable to resolve name clash" );
                    throw CommandAbortedException();
                }
                else
                    dstUnqPath = newDstUnqPath;

                break;
            }
            case NameClash::ERROR:
                break;
            default:
                break;
        }
    }

    if( err )
        throw CommandAbortedException();

    copyPersistentSet( srcUnqPath,dstUnqPath, !isDocument );
    notifyInsert( getContentEventListeners( getParentName( dstUnqPath ) ),dstUnqPath );
}


// Recursive deleting

void SAL_CALL
shell::remove( sal_Int32 CommandId,
               const rtl::OUString& aUnqPath,
               sal_Int32 IsWhat )
{
    sal_Int32 nMask = FileStatusMask_Type | FileStatusMask_FilePath;
    osl::DirectoryItem aItem;
    osl::FileStatus aStatus( nMask );

    if( IsWhat == 0 )
    {
        osl::DirectoryItem::get( aUnqPath, aItem );
        aItem.getFileStatus( aStatus );
        if(  aStatus.isValid( nMask ) &&
             ( aStatus.getFileType() == osl::FileStatus::Regular ||
               aStatus.getFileType() == osl::FileStatus::Link ) )
            IsWhat = -1;
        else if( aStatus.isValid( nMask ) &&
                 ( aStatus.getFileType() == osl::FileStatus::Directory ||
                   aStatus.getFileType() == osl::FileStatus::Volume ) )
            IsWhat = +1;
    }


    if( IsWhat == -1 )
    {
        osl::File::remove( aUnqPath );

        notifyContentDeleted( getContentDeletedEventListeners(aUnqPath) );
        erasePersistentSet( aUnqPath );   // Removes from XPersistentPropertySet
    }
    else if( IsWhat == +1 )
    {
        sal_Int32 recurse;
        rtl::OUString name;

//          osl::Directory aFolder( aUnqPath );

        oslDirectory pDir=0;

        oslFileError tErr;
        oslDirectoryItem pItem=0;

        tErr = osl_openDirectory(aUnqPath.pData,&pDir);

//        fprintf(stderr,"RefCount == '%i'\n",aUnqPath.pData->refCount);
//      aFolder.open();

//        fprintf(stderr,"RefCount == '%i'\n",aUnqPath.pData->refCount);


//          osl::FileBase::RC rcError = aFolder.getNextItem( aItem );
//          while( osl::FileBase::E_None == rcError )
        tErr = osl_getNextDirectoryItem(pDir,&pItem,0);
        while ( tErr == osl_File_E_None )
        {
            oslFileStatus aStat;

            memset(&aStat,0,sizeof(aStat));
            rtl::OUString sFilePath;
            rtl::OUString sNativePath;

            aStat.uStructSize=sizeof(aStat);
            aStat.pstrFilePath=&sFilePath.pData;
            aStat.pstrNativePath=&sNativePath.pData;

//              aItem.getFileStatus( aStatus );

            osl_getFileStatus(pItem,&aStat,FileStatusMask_Type | FileStatusMask_FilePath);


//              if(  aStatus.isValid( nMask ) &&
//                   aStatus.getFileType() == osl::FileStatus::Regular ||
//                   aStatus.getFileType() == osl::FileStatus::Link )
//                  recurse = -1;
//              else if( aStatus.isValid( nMask ) &&
//                       ( aStatus.getFileType() == osl::FileStatus::Directory ||
//                         aStatus.getFileType() == osl::FileStatus::Volume ) )
//                  recurse = +1;

            if ( aStat.uValidFields & osl_FileStatus_Mask_Type && ( aStat.eType == osl_File_Type_Regular || aStat.eType == osl_File_Type_Link ) )
            {
                recurse = -1;
            }
            else if ( aStat.uValidFields & osl_FileStatus_Mask_Type && ( aStat.eType == osl_File_Type_Directory || aStat.eType == osl_File_Type_Volume ) )
            {
                recurse = +1;
            }

//              name = aStatus.getFilePath();
            name = rtl::OUString(*aStat.pstrFilePath);

            remove( CommandId,
                    name,
                    recurse );

//              rcError = aFolder.getNextItem( aItem );
            osl_releaseDirectoryItem(pItem);
            pItem=0;

            tErr = osl_getNextDirectoryItem(pDir,&pItem,0);
        }

//          aFolder.close();
        osl_closeDirectory(pDir);

        osl::FileBase::RC err = osl::Directory::remove( aUnqPath );
        if( ! err )
        {
            notifyContentDeleted( getContentDeletedEventListeners(aUnqPath) );
            erasePersistentSet( aUnqPath );
        }
    }
}



sal_Bool SAL_CALL
shell::ensuredir( const rtl::OUString& rUnqPath )
{
    rtl::OUString aUnqPath;
    if ( rUnqPath[ rUnqPath.getLength() - 1 ] == sal_Unicode( '/' ) )
        aUnqPath = rUnqPath.copy( 0, rUnqPath.getLength() - 1 );
    else
        aUnqPath = rUnqPath;

    sal_Int32 nPos = 3; // start after "//./"
    while ( nPos != - 1 )
    {
        nPos = aUnqPath.indexOf( '/', nPos + 1 );

        rtl::OUString aPath = ( nPos == -1 )
                            ? aUnqPath
                            : aUnqPath.copy( 0, nPos );

        osl::FileBase::RC nError = osl::Directory::create( aPath );

        if ( nError == osl::FileBase::E_None )
        {
            // created.
            rtl::OUString aPrtPath = getParentName( aPath );
            notifyInsert( getContentEventListeners( aPrtPath ),aPath );
        }
        else if ( nError != osl::FileBase::E_EXIST )
        {
            // Workaround for drives.
            if ( ( nError == osl::FileBase::E_ACCES ) &&
                 ( aPath.getLength() == 6 ) &&
                 ( aPath[ aPath.getLength() - 1 ] == sal_Unicode( ':' ) ) )
                continue;

            return false;
        }
    }

    return true;
}



sal_Bool SAL_CALL
shell::mkdir( sal_Int32 CommandId,
              const rtl::OUString& aUnqPath )
{
    return ensuredir( aUnqPath );
}



sal_Bool SAL_CALL
shell::mkfil( sal_Int32 CommandId,
              const rtl::OUString& aUnqPath,
              sal_Bool Overwrite,
              const uno::Reference< io::XInputStream >& aInputStream )
{

    // return value unimportant
    write( CommandId,
           aUnqPath,
           Overwrite,
           aInputStream );

    // Always notifications for an insert
    // Cannot give an error

    rtl::OUString aPrtPath = getParentName( aUnqPath );
    notifyInsert( getContentEventListeners( aPrtPath ),aUnqPath );

    return true;
}



sal_Bool SAL_CALL
shell::write( sal_Int32 CommandId,
              const rtl::OUString& aUnqPath,
              sal_Bool OverWrite,
              const uno::Reference< io::XInputStream >& aInputStream )
{

    osl::File aFile( aUnqPath );

    sal_Bool bSuccess;

    // Create parent path, if necessary.
    if ( !ensuredir( getParentName( aUnqPath ) ) )
        return false;

    if( OverWrite )
    {
        bSuccess = osl::FileBase::E_None == aFile.open( OpenFlag_Write | OpenFlag_Create );
        if( ! bSuccess )
            bSuccess = osl::FileBase::E_None == aFile.open( OpenFlag_Write );
    }
    else
    {
        bSuccess = osl::FileBase::E_None == aFile.open( OpenFlag_Write );
        if( bSuccess )
        {
            aFile.close();
            return true;
        }
        else
        {
            bSuccess = osl::FileBase::E_None == aFile.open( OpenFlag_Write | OpenFlag_Create );
        }
    }

    if( bSuccess && aInputStream.is() )
    {
        sal_uInt64 nTotalNumberOfBytes = 0;
        sal_uInt64 nWrittenBytes;
        sal_Int32 nReadBytes = 0, nRequestedBytes = 32768;
        uno::Sequence< sal_Int8 > seq( nRequestedBytes );
        do
        {
            try
            {
                nReadBytes = aInputStream->readBytes( seq,
                                                      nRequestedBytes );
            }
            catch( const io::NotConnectedException& e )
            {
                bSuccess = false;
            }
            catch( const io::BufferSizeExceededException& e )
            {
                bSuccess = false;
            }
            catch( const io::IOException& e )
            {
                bSuccess = false;
            }


            if( bSuccess && nReadBytes )
            {
                const sal_Int8* p = seq.getConstArray();
                aFile.write( ((void*)(p)),
                             sal_uInt64( nReadBytes ),
                             nWrittenBytes );
                nTotalNumberOfBytes += nWrittenBytes;
                if( nWrittenBytes != nReadBytes )
                {
                    // DBG_ASSERT( "Write Operation not successful" );
                }
            }
        } while( bSuccess && sal_uInt64( nReadBytes ) == nRequestedBytes );

        aFile.setSize( nTotalNumberOfBytes );
    }

//  else
//      bSuccess = false;


    aFile.close();

    return bSuccess;
}



/*********************************************************************************/
/*                                                                               */
/*                 InsertDefaultProperties-Implementation                        */
/*                                                                               */
/*********************************************************************************/


void SAL_CALL shell::InsertDefaultProperties( const rtl::OUString& aUnqPath )
{
  vos::OGuard aGuard( m_aMutex );

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


void SAL_CALL
shell::getScheme( rtl::OUString& Scheme )
{
  Scheme = rtl::OUString::createFromAscii( "file" );
}

rtl::OUString SAL_CALL
shell::getImplementationName_static( void )
{
  return rtl::OUString::createFromAscii( "FileProvider" );
}


uno::Sequence< rtl::OUString > SAL_CALL
shell::getSupportedServiceNames_static( void )
{
  rtl::OUString Supported = rtl::OUString::createFromAscii( "com.sun.star.ucb.FileContentProvider" ) ;
  com::sun::star::uno::Sequence< rtl::OUString > Seq( &Supported,1 );
  return Seq;
}


sal_Bool SAL_CALL shell::getUnqFromUrl( const rtl::OUString& Url,rtl::OUString& Unq )
{
    if( 0 == Url.compareToAscii( "file:///" ) ||
        0 == Url.compareToAscii( "file://localhost/" ) ||
        0 == Url.compareToAscii( "file://127.0.0.1/" ) )
    {
        Unq = rtl::OUString::createFromAscii( "//./" );
        return false;
    }

    sal_Bool err = osl::FileBase::E_None != osl::FileBase::getNormalizedPathFromFileURL( Url,Unq );

    sal_Int32 l = Unq.getLength()-1;
    if( ! err && Unq.getStr()[ l ] == '/' &&
        Unq.indexOf( '/', RTL_CONSTASCII_LENGTH("//") ) < l )
        Unq = Unq.copy(0, Unq.getLength() - 1);

    return err;
}

sal_Bool SAL_CALL shell::getUrlFromUnq( const rtl::OUString& Unq,rtl::OUString& Url )
{
    if( Unq.compareToAscii( "//./" ) == 0 )
    {
        Url = rtl::OUString::createFromAscii( "file:///" );
        return false;
    }

    rtl::OUString aUnq = Unq;
    if( aUnq[ aUnq.getLength()-1 ] == sal_Unicode( ':' ) && aUnq.getLength() == 6 )
    {
        aUnq += rtl::OUString::createFromAscii( "/" );
    }
    sal_Bool err = osl::FileBase::E_None != osl::FileBase::getFileURLFromNormalizedPath( aUnq,Url );
    return err;
}



// Privat methods


void SAL_CALL shell::getMaskFromProperties( sal_Int32& n_Mask, const uno::Sequence< beans::Property >& seq )
{
    n_Mask = FileStatusMask_FilePath;

    rtl::OUString PropertyName;
    for( sal_Int32 i = 0; i < seq.getLength(); ++i )
    {
        PropertyName = seq[i].Name;

        if( PropertyName == Title )
            n_Mask |= FileStatusMask_FileName;
        else if( PropertyName == IsDocument || PropertyName == IsFolder )
            n_Mask |= FileStatusMask_Type;
        else if( PropertyName == DateCreated )
            n_Mask |= FileStatusMask_CreationTime;
        else if( PropertyName == DateModified )
            n_Mask |= FileStatusMask_ModifyTime;
        else if( PropertyName == Size )
            n_Mask |= FileStatusMask_FileSize;
        else if( PropertyName == IsReadOnly )
            n_Mask |= FileStatusMask_Attributes;
//      else if( PropertyName == FolderCount )
//          ;
//      else if( PropertyName == DocumentCount )
//          ;
    }
}



void SAL_CALL
shell::commit( const shell::ContentMap::iterator& it,
               const osl::FileStatus& aFileStatus )
{
    uno::Any aAny;
    shell::PropertySet::iterator it1;

    if( it->second.properties == 0 )
    {
        rtl::OUString aPath = it->first;
        InsertDefaultProperties( aPath );
    }

    PropertySet& properties = *( it->second.properties );

    if( aFileStatus.isValid( FileStatusMask_FileName ) )
    {
        aAny <<= aFileStatus.getFileName();

        if( m_bFaked )
        {
            for( sal_Int32 i = 0; i < m_vecMountPoint.size(); ++i )
                if( it->first == m_vecMountPoint[i].m_aDirectory )
                    aAny <<= m_vecMountPoint[i].m_aTitle;
        }

        it1 = properties.find( MyProperty( Title ) );
        if( it1 != properties.end() )
        {
            it1->setValue( aAny );
        }
    }

    if( aFileStatus.isValid( FileStatusMask_Type ) )
    {
        sal_Bool dummy =
            osl::FileStatus::Volume == aFileStatus.getFileType()
            ||
            osl::FileStatus::Directory == aFileStatus.getFileType();

        aAny <<= dummy;
        it1 = properties.find( MyProperty( IsFolder ) );
        if( it1 != properties.end() )
        {
            it1->setValue( aAny );
        }

        dummy = osl::FileStatus::Regular == aFileStatus.getFileType();
        aAny <<= dummy;
        it1 = properties.find( MyProperty( IsDocument ) );
        if( it1 != properties.end() )
        {
            it1->setValue( aAny );
        }
    }

    if( m_bFaked && it->first.compareToAscii( "//./" ) == 0 )
    {
        sal_Bool dummy = true;

        aAny <<= dummy;
        it1 = properties.find( MyProperty( IsFolder ) );
        if( it1 != properties.end() )
        {
            it1->setValue( aAny );
        }

        dummy = false;
        aAny <<= dummy;
        it1 = properties.find( MyProperty( IsDocument ) );
        if( it1 != properties.end() )
        {
            it1->setValue( aAny );
        }
    }

    if( aFileStatus.isValid( FileStatusMask_FileSize ) )
    {
        aAny <<= sal_Int64( aFileStatus.getFileSize() );
        it1 = properties.find( MyProperty( Size ) );
        if( it1 != properties.end() )
        {
            it1->setValue( aAny );
        }
    }

    if( aFileStatus.isValid( FileStatusMask_Attributes ) )
    {
        sal_uInt64 Attr = aFileStatus.getAttributes();
        sal_Bool readonly = ( Attr & Attribute_ReadOnly ) != 0;
        aAny <<= readonly;
        it1 = properties.find( MyProperty( IsReadOnly ) );
        if( it1 != properties.end() )
        {
            it1->setValue( aAny );
        }
    }

    if( m_bFaked && it->first.compareToAscii( "//./" ) == 0 )
    {
        sal_Bool readonly = true;
        aAny <<= readonly;
        it1 = properties.find( MyProperty( IsReadOnly ) );
        if( it1 != properties.end() )
        {
            it1->setValue( aAny );
        }
    }

    if( aFileStatus.isValid( FileStatusMask_ModifyTime ) )
    {
        TimeValue temp = aFileStatus.getModifyTime();

        // Convert system time to local time (for EA)
        TimeValue   myLocalTime;
        osl_getLocalTimeFromSystemTime( &temp, &myLocalTime );

        oslDateTime myDateTime;
        osl_getDateTimeFromTimeValue( &myLocalTime, &myDateTime );
        util::DateTime aDateTime;

        aDateTime.HundredthSeconds = myDateTime.NanoSeconds / 10000000;
        aDateTime.Seconds = myDateTime.Seconds;
        aDateTime.Minutes = myDateTime.Minutes;
        aDateTime.Hours = myDateTime.Hours;
        aDateTime.Day = myDateTime.Day;
        aDateTime.Month = myDateTime.Month;
        aDateTime.Year = myDateTime.Year;
        aAny <<= aDateTime;
        it1 = properties.find( MyProperty( DateModified ) );
        if( it1 != properties.end() )
        {
            it1->setValue( aAny );
        }
    }

    if( aFileStatus.isValid( FileStatusMask_CreationTime ) )
    {
        TimeValue temp = aFileStatus.getCreationTime();

        // Convert system time to local time (for EA)
        TimeValue   myLocalTime;
        osl_getLocalTimeFromSystemTime( &temp, &myLocalTime );

        oslDateTime myDateTime;
        osl_getDateTimeFromTimeValue( &myLocalTime,&myDateTime );
        util::DateTime aDateTime;

        aDateTime.HundredthSeconds = myDateTime.NanoSeconds / 10000000;
        aDateTime.Seconds = myDateTime.Seconds;
        aDateTime.Minutes = myDateTime.Minutes;
        aDateTime.Hours = myDateTime.Hours;
        aDateTime.Day = myDateTime.Day;
        aDateTime.Month = myDateTime.Month;
        aDateTime.Year = myDateTime.Year;
        aAny <<= aDateTime;
        it1 = properties.find( MyProperty( DateCreated ) );
        if( it1 != properties.end() )
        {
            it1->setValue( aAny );
        }
    }
}




uno::Reference< sdbc::XRow > SAL_CALL
shell::getv(
    sal_Int32 CommandId,
    Notifier* pNotifier,
    const uno::Sequence< beans::Property >& properties,
    osl::DirectoryItem& aDirItem,
    rtl::OUString& aUnqPath,
    sal_Bool& aIsRegular )
{
    uno::Sequence< uno::Any > seq( properties.getLength() );

    sal_Int32 n_Mask;
    getMaskFromProperties( n_Mask,properties );
    n_Mask |= FileStatusMask_Type;

    osl::FileStatus aFileStatus( n_Mask );
    aDirItem.getFileStatus( aFileStatus );

    aUnqPath = aFileStatus.getFilePath();
    aIsRegular = aFileStatus.getFileType() == osl::FileStatus::Regular;

    registerNotifier( aUnqPath,pNotifier );
    InsertDefaultProperties( aUnqPath );
    {
        vos::OGuard aGuard( m_aMutex );
        sal_Bool changer = false;

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

    XRow_impl* p = new XRow_impl( this,seq );
    return uno::Reference< sdbc::XRow >( p );
}




rtl::OUString
shell::getParentName( const rtl::OUString& aFileName )
{
    sal_Int32 lastIndex = aFileName.lastIndexOf( sal_Unicode('/') );
    rtl::OUString aParent = aFileName.copy( 0,lastIndex );

    if( aParent[ aParent.getLength()-1] == sal_Unicode(':') && aParent.getLength() == 6 )
        aParent += rtl::OUString::createFromAscii( "/" );
    if( 0 == aParent.compareToAscii( "//." ) )
        aParent = rtl::OUString::createFromAscii( "//./" );


    return aParent;
}



// EventListener


std::list< ContentEventNotifier* >* SAL_CALL
shell::getContentEventListeners( const rtl::OUString& aName )
{
    std::list< ContentEventNotifier* >* p = new std::list< ContentEventNotifier* >;
    std::list< ContentEventNotifier* >& listeners = *p;
    {
        vos::OGuard aGuard( m_aMutex );
        shell::ContentMap::iterator it = m_aContent.find( aName );
        if( it != m_aContent.end() && it->second.notifier )
        {
            std::list<Notifier*>& listOfNotifiers = *( it->second.notifier );
            std::list<Notifier*>::iterator it1 = listOfNotifiers.begin();
            while( it1 != listOfNotifiers.end() )
            {
                Notifier* pointer = *it1;
                ContentEventNotifier* not = pointer->cCEL();
                if( not )
                    listeners.push_back( not );
                ++it1;
            }
        }
    }
    return p;
}



std::list< ContentEventNotifier* >* SAL_CALL
shell::getContentDeletedEventListeners( const rtl::OUString& aName )
{
    std::list< ContentEventNotifier* >* p = new std::list< ContentEventNotifier* >;
    std::list< ContentEventNotifier* >& listeners = *p;
    {
        vos::OGuard aGuard( m_aMutex );
        shell::ContentMap::iterator it = m_aContent.find( aName );
        if( it != m_aContent.end() && it->second.notifier )
        {
            std::list<Notifier*>& listOfNotifiers = *( it->second.notifier );
            std::list<Notifier*>::iterator it1 = listOfNotifiers.begin();
            while( it1 != listOfNotifiers.end() )
            {
                Notifier* pointer = *it1;
                ContentEventNotifier* not = pointer->cDEL();
                if( not )
                    listeners.push_back( not );
                ++it1;
            }
        }
    }
    return p;
}


void SAL_CALL
shell::notifyInsert( std::list< ContentEventNotifier* >* listeners,const rtl::OUString& aChildName )
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
                             const rtl::OUString& aChildName )
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
shell::getPropertySetListeners( const rtl::OUString& aName )
{
    std::list< PropertySetInfoChangeNotifier* >* p = new std::list< PropertySetInfoChangeNotifier* >;
    std::list< PropertySetInfoChangeNotifier* >& listeners = *p;
    {
        vos::OGuard aGuard( m_aMutex );
        shell::ContentMap::iterator it = m_aContent.find( aName );
        if( it != m_aContent.end() && it->second.notifier )
        {
            std::list<Notifier*>& listOfNotifiers = *( it->second.notifier );
            std::list<Notifier*>::iterator it1 = listOfNotifiers.begin();
            while( it1 != listOfNotifiers.end() )
            {
                Notifier* pointer = *it1;
                PropertySetInfoChangeNotifier* not = pointer->cPSL();
                if( not )
                    listeners.push_back( not );
                ++it1;
            }
        }
    }
    return p;
}


void SAL_CALL
shell::notifyPropertyAdded( std::list< PropertySetInfoChangeNotifier* >* listeners,
                            const rtl::OUString& aPropertyName )
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
                              const rtl::OUString& aPropertyName )
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
shell::getContentExchangedEventListeners( const rtl::OUString aOldPrefix,
                                          const rtl::OUString aNewPrefix,
                                          sal_Bool withChilds )
{

    std::vector< std::list< ContentEventNotifier* >* >* aVectorOnHeap =
        new std::vector< std::list< ContentEventNotifier* >* >;
    std::vector< std::list< ContentEventNotifier* >* >&  aVector = *aVectorOnHeap;

    sal_Int32 count;
    rtl::OUString aOldName;
    rtl::OUString aNewName;
    std::vector< rtl::OUString > oldChildList;

    {
        vos::OGuard aGuard( m_aMutex );

        if( ! withChilds )
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
                if( ::isChild( aOldPrefix,itnames->first ) )
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

            if( withChilds )
            {
                aOldName = oldChildList[j];
                aNewName = ::newName( aNewPrefix,aOldPrefix,aOldName );
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
                        ContentEventNotifier* not = pointer->cEXC( aNewName );
                        if( not )
                            listeners.push_back( not );
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
    for( sal_Int32 i = 0; i < listeners_vec->size(); ++i )
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
shell::getPropertyChangeNotifier( const rtl::OUString& aName )
{
    std::list< PropertyChangeNotifier* >* p = new std::list< PropertyChangeNotifier* >;
    std::list< PropertyChangeNotifier* >& listeners = *p;
    {
        vos::OGuard aGuard( m_aMutex );
        shell::ContentMap::iterator it = m_aContent.find( aName );
        if( it != m_aContent.end() && it->second.notifier )
        {
            std::list<Notifier*>& listOfNotifiers = *( it->second.notifier );
            std::list<Notifier*>::iterator it1 = listOfNotifiers.begin();
            while( it1 != listOfNotifiers.end() )
            {
                Notifier* pointer = *it1;
                PropertyChangeNotifier* not = pointer->cPCL();
                if( not )
                    listeners.push_back( not );
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



// Return value: not mounted

#ifdef UNX
extern "C" oslFileError osl_getRealPath(rtl_uString* strPath, rtl_uString** strRealPath);
#endif

sal_Bool SAL_CALL shell::checkMountPoint( const rtl::OUString&  aUnqPath,
                                          rtl::OUString&        aRedirectedPath )
{
    sal_Int32 numMp = m_vecMountPoint.size();

    if( ! numMp )
    {
        // No access restrictions
        aRedirectedPath = aUnqPath;
        return true;
    }


    if( aUnqPath.compareTo( rtl::OUString::createFromAscii( "//./" ) ) == 0 )
    {
        aRedirectedPath = aUnqPath;
        return true;
    }

    for( sal_Int32 j = 0; j < numMp; ++j )
    {
        rtl::OUString aAlias = m_vecMountPoint[j].m_aMountPoint;
        rtl::OUString aDir   = m_vecMountPoint[j].m_aDirectory;
        sal_Int32 nL = aAlias.getLength();

        if( aUnqPath.compareTo( aAlias,nL ) == 0 && ( aUnqPath.getLength() == nL || aUnqPath[nL] == '/' ) )
        {
            aRedirectedPath = aDir;
            aRedirectedPath += aUnqPath.copy( nL );
            return true;
        }
    }

    return false;
}



sal_Bool SAL_CALL shell::uncheckMountPoint( const rtl::OUString&  aUnqPath,
                                            rtl::OUString&        aRedirectedPath )
{
    sal_Int32 numMp = m_vecMountPoint.size();

    if( ! numMp )
    {
        // No access restrictions
        aRedirectedPath = aUnqPath;
        return true;
    }


    if( aUnqPath.compareTo( rtl::OUString::createFromAscii( "//./" ) ) == 0 )
    {
        aRedirectedPath = aUnqPath;
        return true;
    }


    for( sal_Int32 j = 0; j < numMp; ++j )
    {
        sal_Int32 nL = m_vecMountPoint[j].m_aDirectory.getLength();

#ifdef UNX
        rtl::OUString   aRealUnqPath;
        oslFileError    error = osl_File_E_None;

        if ( !aRealUnqPath.pData->length )
            error = osl_getRealPath( aUnqPath.pData, &aRealUnqPath.pData );

        rtl::OUString dir = m_vecMountPoint[j].m_aDirectory;

        if ( osl_File_E_None == error && aRealUnqPath.compareTo( dir,nL ) == 0 &&
             ( aRealUnqPath.getLength() == nL || aRealUnqPath[nL] == '/' ) )
        {
            aRedirectedPath = m_vecMountPoint[j].m_aMountPoint;
            aRedirectedPath += aRealUnqPath.copy( nL );
            return true;
        }
#else
        rtl::OUString dir = m_vecMountPoint[j].m_aDirectory;

        if( aUnqPath.compareTo( dir,nL ) == 0  &&
            ( aUnqPath.getLength() == nL || aUnqPath[nL] == '/' )
        )
        {
            aRedirectedPath = m_vecMountPoint[j].m_aMountPoint;
            aRedirectedPath += aUnqPath.copy( nL );
            return true;
        }
#endif
    }

    return false;
}


shell::MountPoint::MountPoint( const rtl::OUString& aMountPoint,
                               const rtl::OUString& aDirectory )
    : m_aMountPoint( aMountPoint ),
      m_aDirectory( aDirectory )
{
    rtl::OUString Title = aMountPoint;
    sal_Int32 lastIndex = Title.lastIndexOf( sal_Unicode( '/' ) );
    m_aTitle = Title.copy( lastIndex + 1 );
}
