/*************************************************************************
 *
 *  $RCSfile: hierarchycontent.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kso $ $Date: 2001-03-27 14:08:50 $
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
 *  Contributor(s): Kai Sommerfeld ( kso@sun.com )
 *
 *
 ************************************************************************/

/**************************************************************************
                                TODO
 **************************************************************************

 - optimize transfer command. "Move" should be implementable much more
   efficient!

 **************************************************************************

 - Root Folder vs. 'normal' Folder
     - root doesn't support command 'delete'
    - root doesn't support command 'insert'
    - root needs not created via XContentCreator - queryContent with root
      folder id ( HIERARCHY_ROOT_FOLDER_URL ) always returns a value != 0
    - root has no parent.

 *************************************************************************/

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_CONTENTINFOATTRIBUTE_HPP_
#include <com/sun/star/ucb/ContentInfoAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INSERTCOMMANDARGUMENT_HPP_
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVEBADTRANSFRERURLEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveBadTransferURLException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDINFO_HPP_
#include <com/sun/star/ucb/XCommandInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XPERSISTENTPROPERTYSET_HPP_
#include <com/sun/star/ucb/XPersistentPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_NAMECLASH_HPP_
#include <com/sun/star/ucb/NameClash.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_TRANSFERINFO_HPP_
#include <com/sun/star/ucb/TransferInfo.hpp>
#endif
#ifndef _VOS_DIAGNOSE_HXX_
#include <vos/diagnose.hxx>
#endif
#ifndef _UCBHELPER_CONTENTIDENTIFIER_HXX
#include <ucbhelper/contentidentifier.hxx>
#endif
#ifndef _UCBHELPER_PROPERTYVALUESET_HXX
#include <ucbhelper/propertyvalueset.hxx>
#endif

#ifndef _HIERARCHYCONTENT_HXX
#include "hierarchycontent.hxx"
#endif
#ifndef _HIERARCHYPROVIDER_HXX
#include "hierarchyprovider.hxx"
#endif
#ifndef _DYNAMICRESULTSET_HXX
#include "dynamicresultset.hxx"
#endif

using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace rtl;

using namespace hierarchy_ucp;

//=========================================================================
//=========================================================================
//
// HierarchyContent Implementation.
//
//=========================================================================
//=========================================================================

// static ( "virtual" ctor )
HierarchyContent* HierarchyContent::create(
                    const Reference< XMultiServiceFactory >& rxSMgr,
                    HierarchyContentProvider* pProvider,
                    const Reference< XContentIdentifier >& Identifier )
{
    // Fail, if content does not exist.
    HierarchyContentProperties aProps;
    if ( !loadData( rxSMgr, pProvider, Identifier, aProps ) )
        return 0;

    return new HierarchyContent( rxSMgr, pProvider, Identifier, aProps );
}

//=========================================================================
// static ( "virtual" ctor )
HierarchyContent* HierarchyContent::create(
                    const Reference< XMultiServiceFactory >& rxSMgr,
                    HierarchyContentProvider* pProvider,
                    const Reference< XContentIdentifier >& Identifier,
                    const ContentInfo& Info )
{
    if ( !Info.Type.getLength() )
        return 0;

    if ( ( Info.Type.compareToAscii( HIERARCHY_FOLDER_CONTENT_TYPE ) != 0 ) &&
         ( Info.Type.compareToAscii( HIERARCHY_LINK_CONTENT_TYPE ) != 0 ) )
        return 0;

#if 0
    // Fail, if content does exist.
    if ( hasData( rxSMgr, pProvider, Identifier ) )
        return 0;
#endif

    return new HierarchyContent( rxSMgr, pProvider, Identifier, Info );
}

//=========================================================================
HierarchyContent::HierarchyContent(
                        const Reference< XMultiServiceFactory >& rxSMgr,
                        HierarchyContentProvider* pProvider,
                        const Reference< XContentIdentifier >& Identifier,
                        const HierarchyContentProperties& rProps )
: ContentImplHelper( rxSMgr, pProvider, Identifier ),
  m_aProps( rProps ),
  m_eState( PERSISTENT ),
  m_pProvider( pProvider )
{
    setKind( Identifier );
}

//=========================================================================
HierarchyContent::HierarchyContent(
                        const Reference< XMultiServiceFactory >& rxSMgr,
                        HierarchyContentProvider* pProvider,
                        const Reference< XContentIdentifier >& Identifier,
                        const ContentInfo& Info )
: ContentImplHelper( rxSMgr, pProvider, Identifier, sal_False ),
  m_eState( TRANSIENT ),
  m_pProvider( pProvider )
{
    if ( Info.Type.compareToAscii( HIERARCHY_FOLDER_CONTENT_TYPE ) == 0 )
    {
        // New folder...
        m_aProps.aContentType = Info.Type;
//      m_aProps.aTitle       =
        m_aProps.bIsFolder    = sal_True;
        m_aProps.bIsDocument  = sal_False;
    }
    else
    {
        VOS_ENSURE(
            Info.Type.compareToAscii( HIERARCHY_LINK_CONTENT_TYPE ) == 0,
            "HierarchyContent::HierarchyContent - Wrong content info!" );

        // New link...
        m_aProps.aContentType = Info.Type;
//      m_aProps.aTitle       =
        m_aProps.bIsFolder    = sal_False;
        m_aProps.bIsDocument  = sal_True;
    }

    setKind( Identifier );
}

//=========================================================================
// virtual
HierarchyContent::~HierarchyContent()
{
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

// virtual
void SAL_CALL HierarchyContent::acquire()
    throw( RuntimeException )
{
    ContentImplHelper::acquire();
}

//=========================================================================
// virtual
void SAL_CALL HierarchyContent::release()
    throw( RuntimeException )
{
    ContentImplHelper::release();
}

//=========================================================================
// virtual
Any SAL_CALL HierarchyContent::queryInterface( const Type & rType )
    throw ( RuntimeException )
{
    Any aRet;

    if ( isFolder() )
      aRet = cppu::queryInterface( rType,
                        static_cast< XContentCreator * >( this ) );

     return aRet.hasValue() ? aRet : ContentImplHelper::queryInterface( rType );
}

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_COMMON_IMPL( HierarchyContent );

//=========================================================================
// virtual
Sequence< Type > SAL_CALL HierarchyContent::getTypes()
    throw( RuntimeException )
{
    static OTypeCollection* pCollection = NULL;

    if ( !pCollection )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
          if ( !pCollection )
          {
            if ( isFolder() )
            {
                  static OTypeCollection aCollection(
                    CPPU_TYPE_REF( XTypeProvider ),
                       CPPU_TYPE_REF( XServiceInfo ),
                       CPPU_TYPE_REF( XComponent ),
                       CPPU_TYPE_REF( XContent ),
                       CPPU_TYPE_REF( XCommandProcessor ),
                       CPPU_TYPE_REF( XPropertiesChangeNotifier ),
                       CPPU_TYPE_REF( XCommandInfoChangeNotifier ),
                       CPPU_TYPE_REF( XPropertyContainer ),
                       CPPU_TYPE_REF( XPropertySetInfoChangeNotifier ),
                       CPPU_TYPE_REF( XChild ),
                       CPPU_TYPE_REF( XContentCreator ) );  // !!
                  pCollection = &aCollection;
            }
            else
            {
                  static OTypeCollection aCollection(
                    CPPU_TYPE_REF( XTypeProvider ),
                       CPPU_TYPE_REF( XServiceInfo ),
                       CPPU_TYPE_REF( XComponent ),
                       CPPU_TYPE_REF( XContent ),
                       CPPU_TYPE_REF( XCommandProcessor ),
                       CPPU_TYPE_REF( XPropertiesChangeNotifier ),
                       CPPU_TYPE_REF( XCommandInfoChangeNotifier ),
                       CPPU_TYPE_REF( XPropertyContainer ),
                       CPPU_TYPE_REF( XPropertySetInfoChangeNotifier ),
                       CPPU_TYPE_REF( XChild ) );
                  pCollection = &aCollection;
            }
        }
    }

    return (*pCollection).getTypes();
}

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

// virtual
OUString SAL_CALL HierarchyContent::getImplementationName()
    throw( RuntimeException )
{
    return OUString::createFromAscii( "HierarchyContent" );
}

//=========================================================================
// virtual
Sequence< OUString > SAL_CALL HierarchyContent::getSupportedServiceNames()
    throw( RuntimeException )
{
    Sequence< OUString > aSNS( 1 );

    if ( m_eKind == LINK )
        aSNS.getArray()[ 0 ] = OUString::createFromAscii(
                                HIERARCHY_LINK_CONTENT_SERVICE_NAME );
    else if ( m_eKind == FOLDER )
        aSNS.getArray()[ 0 ] = OUString::createFromAscii(
                                HIERARCHY_FOLDER_CONTENT_SERVICE_NAME );
    else
        aSNS.getArray()[ 0 ] = OUString::createFromAscii(
                                HIERARCHY_ROOT_FOLDER_CONTENT_SERVICE_NAME );

    return aSNS;
}

//=========================================================================
//
// XContent methods.
//
//=========================================================================

// virtual
OUString SAL_CALL HierarchyContent::getContentType()
    throw( RuntimeException )
{
    return m_aProps.aContentType;
}

//=========================================================================
// virtual
Reference< XContentIdentifier > SAL_CALL HierarchyContent::getIdentifier()
    throw( RuntimeException )
{
    // Transient?
    if ( m_eState == TRANSIENT )
    {
        // Transient contents have no identifier.
        return Reference< XContentIdentifier >();
    }

    return ContentImplHelper::getIdentifier();
}

//=========================================================================
//
// XCommandProcessor methods.
//
//=========================================================================

// virtual
Any SAL_CALL HierarchyContent::execute( const Command& aCommand,
                                     sal_Int32 CommandId,
                                     const Reference<
                                        XCommandEnvironment >& Environment )
    throw( Exception, CommandAbortedException, RuntimeException )
{
    Any aRet;

    if ( aCommand.Name.compareToAscii( "getPropertyValues" ) == 0 )
    {
        //////////////////////////////////////////////////////////////////
        // getPropertyValues
        //////////////////////////////////////////////////////////////////

        Sequence< Property > Properties;
        if ( !( aCommand.Argument >>= Properties ) )
        {
            VOS_ENSURE( sal_False, "Wrong argument type!" );
            return Any();
        }

        aRet <<= getPropertyValues( Properties );
    }
    else if ( aCommand.Name.compareToAscii( "setPropertyValues" ) == 0 )
    {
        //////////////////////////////////////////////////////////////////
        // setPropertyValues
        //////////////////////////////////////////////////////////////////

        Sequence< PropertyValue > aProperties;
        if ( !( aCommand.Argument >>= aProperties ) )
        {
            VOS_ENSURE( sal_False, "Wrong argument type!" );
            return Any();
        }

        if ( !aProperties.getLength() )
        {
            VOS_ENSURE( sal_False, "No properties!" );
            return Any();
        }

        setPropertyValues( aProperties );
    }
    else if ( aCommand.Name.compareToAscii( "getPropertySetInfo" ) == 0 )
    {
        //////////////////////////////////////////////////////////////////
        // getPropertySetInfo
        //////////////////////////////////////////////////////////////////

        aRet <<= getPropertySetInfo( Environment );
    }
    else if ( aCommand.Name.compareToAscii( "getCommandInfo" ) == 0 )
    {
        //////////////////////////////////////////////////////////////////
        // getCommandInfo
        //////////////////////////////////////////////////////////////////

        aRet <<= getCommandInfo( Environment );
    }
    else if ( isFolder() && ( aCommand.Name.compareToAscii( "open" ) == 0 ) )
    {
        //////////////////////////////////////////////////////////////////
        // open command for a folder content
        //////////////////////////////////////////////////////////////////

        OpenCommandArgument2 aOpenCommand;
        if ( aCommand.Argument >>= aOpenCommand )
        {
            Reference< XDynamicResultSet > xSet
                = new DynamicResultSet( m_xSMgr, this, aOpenCommand );
            aRet <<= xSet;
        }
        else
        {
            VOS_ENSURE( sal_False,
                        "HierarchyContent::execute - invalid parameter!" );
            throw CommandAbortedException();
        }
    }
    else if ( aCommand.Name.compareToAscii( "insert" ) == 0 )
    {
        //////////////////////////////////////////////////////////////////
        // insert
        //  ( Not available at root folder and at persistent objects )
        //////////////////////////////////////////////////////////////////

        InsertCommandArgument aArg;
        if ( aCommand.Argument >>= aArg )
        {
            sal_Int32 nNameClash = aArg.ReplaceExisting
                                 ? NameClash::OVERWRITE
                                 : NameClash::ERROR;
            insert( nNameClash );
        }
        else
        {
            VOS_ENSURE( sal_False,
                        "HierarchyContent::execute - invalid parameter!" );
            throw CommandAbortedException();
        }
    }
    else if ( aCommand.Name.compareToAscii( "delete" ) == 0 )
    {
        //////////////////////////////////////////////////////////////////
        // delete
        //  ( Not available at root folder and at non-persistent objects )
        //////////////////////////////////////////////////////////////////

        sal_Bool bDeletePhysical = sal_False;
        aCommand.Argument >>= bDeletePhysical;
        destroy( bDeletePhysical );

        // Remove own and all children's persistent data.
        removeData();

        // Remove own and all children's Additional Core Properties.
        removeAdditionalPropertySet( sal_True );
    }
    else if ( aCommand.Name.compareToAscii( "transfer" ) == 0 )
    {
        //////////////////////////////////////////////////////////////////
        // transfer
        //  ( Not available at link objects )
        //////////////////////////////////////////////////////////////////

        TransferInfo aInfo;
        if ( aCommand.Argument >>= aInfo )
        {
            transfer( aInfo, Environment );
        }
        else
        {
            VOS_ENSURE( sal_False,
                        "HierarchyContent::execute - invalid parameter!" );
            throw CommandAbortedException();
        }
    }
    else
    {
        //////////////////////////////////////////////////////////////////
        // Unsupported command
        //////////////////////////////////////////////////////////////////

        VOS_ENSURE( sal_False,
                    "HierarchyContent::execute - unsupported command!" );
        throw CommandAbortedException();
    }

    return aRet;
}

//=========================================================================
// virtual
void SAL_CALL HierarchyContent::abort( sal_Int32 CommandId )
    throw( RuntimeException )
{
    // @@@ Generally, no action takes much time...
}

//=========================================================================
//
// XContentCreator methods.
//
//=========================================================================

// virtual
Sequence< ContentInfo > SAL_CALL
HierarchyContent::queryCreatableContentsInfo()
    throw( RuntimeException )
{
    if ( isFolder() )
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        Sequence< ContentInfo > aSeq( 2 );

        // Folder.
        aSeq.getArray()[ 0 ].Type
            = OUString::createFromAscii( HIERARCHY_FOLDER_CONTENT_TYPE );
        aSeq.getArray()[ 0 ].Attributes = ContentInfoAttribute::KIND_FOLDER;

        Sequence< Property > aFolderProps( 1 );
        aFolderProps.getArray()[ 0 ] = Property(
                        OUString::createFromAscii( "Title" ),
                        -1,
                        getCppuType( static_cast< const OUString * >( 0 ) ),
                        PropertyAttribute::BOUND );
        aSeq.getArray()[ 0 ].Properties = aFolderProps;

        // Link.
        aSeq.getArray()[ 1 ].Type
            = OUString::createFromAscii( HIERARCHY_LINK_CONTENT_TYPE );
        aSeq.getArray()[ 1 ].Attributes = ContentInfoAttribute::KIND_LINK;

        Sequence< Property > aLinkProps( 2 );
        aLinkProps.getArray()[ 0 ] = Property(
                        OUString::createFromAscii( "Title" ),
                        -1,
                        getCppuType( static_cast< const OUString * >( 0 ) ),
                        PropertyAttribute::BOUND );
         aLinkProps.getArray()[ 1 ] = Property(
                        OUString::createFromAscii( "TargetURL" ),
                        -1,
                        getCppuType( static_cast< const OUString * >( 0 ) ),
                        PropertyAttribute::BOUND );
        aSeq.getArray()[ 1 ].Properties = aLinkProps;

        return aSeq;
    }
    else
    {
        VOS_ENSURE( sal_False,
                    "queryCreatableContentsInfo called on non-folder object!" );

        return Sequence< ContentInfo >( 0 );
    }
}

//=========================================================================
// virtual
Reference< XContent > SAL_CALL HierarchyContent::createNewContent(
                                                const ContentInfo& Info )
    throw( RuntimeException )
{
    if ( isFolder() )
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        if ( !Info.Type.getLength() )
            return Reference< XContent >();

        if ( ( Info.Type.compareToAscii( HIERARCHY_FOLDER_CONTENT_TYPE ) != 0 )
              &&
              ( Info.Type.compareToAscii( HIERARCHY_LINK_CONTENT_TYPE ) != 0 ) )
            return Reference< XContent >();

        OUString aURL = m_xIdentifier->getContentIdentifier();

        VOS_ENSURE( aURL.getLength() > 0,
                    "HierarchyContent::createNewContent - empty identifier!" );

        if ( ( aURL.lastIndexOf( '/' ) + 1 ) != aURL.getLength() )
            aURL += OUString::createFromAscii( "/" );

        if ( Info.Type.compareToAscii( HIERARCHY_FOLDER_CONTENT_TYPE ) == 0 )
            aURL += OUString::createFromAscii( "New_Folder" );
        else
            aURL += OUString::createFromAscii( "New_Link" );

        Reference< XContentIdentifier > xId(
                        new ::ucb::ContentIdentifier( m_xSMgr, aURL ) );

        return create( m_xSMgr, m_pProvider, xId, Info );
    }
    else
    {
        VOS_ENSURE( sal_False,
                    "createNewContent called on non-folder object!" );
        return Reference< XContent >();
    }
}

//=========================================================================
// virtual
OUString HierarchyContent::getParentURL()
{
    OUString aURL = m_xIdentifier->getContentIdentifier();

    // Am I the root folder?
    if ( m_eKind == ROOT )
        return OUString();

    sal_Int32 nPos = aURL.lastIndexOf( '/' );

    if ( nPos == ( aURL.getLength() - 1 ) )
    {
        // Trailing slash found. Skip.
        nPos = aURL.lastIndexOf( '/', nPos );
    }

    if ( nPos != -1 )
    {
        OUString aParentURL = aURL.copy( 0, nPos );
        return aParentURL;
    }

    return OUString();
}

//=========================================================================
//static
sal_Bool HierarchyContent::hasData(
                        const Reference< XMultiServiceFactory >& rxSMgr,
                        HierarchyContentProvider* pProvider,
                        const Reference< XContentIdentifier >& Identifier )
{
    OUString aURL = Identifier->getContentIdentifier();

//  if ( aURL.compareToAscii( HIERARCHY_ROOT_FOLDER_URL,
//                            HIERARCHY_ROOT_FOLDER_URL_LENGTH ) != 0 )
//  {
//      // Illegal identifier!
//      return sal_False;
//  }

    // Am I the root folder?
    if ( aURL.getLength() == HIERARCHY_ROOT_FOLDER_URL_LENGTH )
    {
        // hasData must always return 'true' for root folder
        // even if no persistent data exist!!!
        return sal_True;
    }

    HierarchyEntry aEntry( rxSMgr, pProvider, aURL );
    HierarchyEntryData aData;

    return aEntry.hasData();
}

//=========================================================================
//static
sal_Bool HierarchyContent::loadData(
                        const Reference< XMultiServiceFactory >& rxSMgr,
                        HierarchyContentProvider* pProvider,
                        const Reference< XContentIdentifier >& Identifier,
                        HierarchyContentProperties& rProps )
{
    OUString aURL = Identifier->getContentIdentifier();

//  if ( aURL.compareToAscii( HIERARCHY_ROOT_FOLDER_URL,
//                            HIERARCHY_ROOT_FOLDER_URL_LENGTH ) != 0 )
//  {
//      // Illegal identifier!
//      return sal_False;
//  }

    // Am I the root folder?
    if ( aURL.getLength() == HIERARCHY_ROOT_FOLDER_URL_LENGTH )
    {
        // loadData must always return 'true' for root folder
        // even if no persistent data exist!!! --> Fill props!!!

        rProps.aContentType = OUString::createFromAscii(
                                            HIERARCHY_FOLDER_CONTENT_TYPE );
//      rProps.aTitle       = OUString();
//      rProps.aTargetURL   = OUString();
        rProps.bIsFolder    = sal_True;
        rProps.bIsDocument  = sal_False;
    }
    else
    {
        HierarchyEntry aEntry( rxSMgr, pProvider, aURL );
        if ( !aEntry.getData( rProps ) )
            return sal_False;

        if ( rProps.aTargetURL.getLength() > 0 )
        {
            rProps.aContentType = OUString::createFromAscii(
                                            HIERARCHY_LINK_CONTENT_TYPE );
            rProps.bIsFolder    = sal_False;
            rProps.bIsDocument  = sal_True;
        }
        else
        {
            rProps.aContentType = OUString::createFromAscii(
                                            HIERARCHY_FOLDER_CONTENT_TYPE );
            rProps.bIsFolder    = sal_True;
            rProps.bIsDocument  = sal_False;
        }
    }
    return sal_True;
}

//=========================================================================
sal_Bool HierarchyContent::storeData()
{
    HierarchyEntry aEntry(
            m_xSMgr, m_pProvider, m_xIdentifier->getContentIdentifier() );
    return aEntry.setData( m_aProps, sal_True );
}

//=========================================================================
sal_Bool HierarchyContent::renameData(
                            const Reference< XContentIdentifier >& xOldId,
                             const Reference< XContentIdentifier >& xNewId )
{
    HierarchyEntry aEntry(
            m_xSMgr, m_pProvider, xOldId->getContentIdentifier() );
    return aEntry.move( xNewId->getContentIdentifier(), m_aProps );
}

//=========================================================================
sal_Bool HierarchyContent::removeData()
{
    HierarchyEntry aEntry(
        m_xSMgr, m_pProvider, m_xIdentifier->getContentIdentifier() );
    return aEntry.remove();
}

//=========================================================================
void HierarchyContent::setKind(
                    const Reference< XContentIdentifier >& Identifier )
{
    if ( m_aProps.bIsFolder )
    {
        // Am I the root folder?
        if ( Identifier->getContentIdentifier().compareToAscii(
                                        HIERARCHY_ROOT_FOLDER_URL ) == 0 )
            m_eKind = ROOT;
        else
            m_eKind = FOLDER;
    }
    else
        m_eKind = LINK;
}

//=========================================================================
Reference< XContentIdentifier > HierarchyContent::getIdentifierFromTitle()
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    // Assemble new content identifier...

    OUString aURL( m_xIdentifier->getContentIdentifier() );
    sal_Int32 nPos = aURL.lastIndexOf( '/' );

    if ( nPos == ( aURL.getLength() - 1 ) )
    {
        // Trailing slash found. Skip.
        nPos = aURL.lastIndexOf( '/', nPos );
    }

    if ( nPos == -1 )
    {
        VOS_ENSURE( sal_False,
                    "HierarchyContent::getIdentifierFromTitle - Invalid URL!" );
        return Reference< XContentIdentifier >();
    }

    OUString aNewURL = aURL.copy( 0, nPos + 1 );
    aNewURL += HierarchyContentProvider::encodeSegment( m_aProps.aTitle );

    return Reference< XContentIdentifier >(
                           new ::ucb::ContentIdentifier( m_xSMgr, aNewURL ) );
}

//=========================================================================
void HierarchyContent::queryChildren( HierarchyContentRefList& rChildren )
{
    if ( ( m_eKind != FOLDER ) && ( m_eKind != ROOT ) )
        return;

    // Obtain a list with a snapshot of all currently instanciated contents
    // from provider and extract the contents which are direct children
    // of this content.

    ::ucb::ContentRefList aAllContents;
    m_xProvider->queryExistingContents( aAllContents );

    OUString aURL = m_xIdentifier->getContentIdentifier();
    sal_Int32 nPos = aURL.lastIndexOf( '/' );

    if ( nPos != ( aURL.getLength() - 1 ) )
    {
        // No trailing slash found. Append.
        aURL += OUString::createFromAscii( "/" );
    }

    sal_Int32 nLen = aURL.getLength();

    ::ucb::ContentRefList::const_iterator it  = aAllContents.begin();
    ::ucb::ContentRefList::const_iterator end = aAllContents.end();

    while ( it != end )
    {
        ::ucb::ContentImplHelperRef xChild = (*it);
        OUString aChildURL = xChild->getIdentifier()->getContentIdentifier();

        // Is aURL a prefix of aChildURL?
        if ( ( aChildURL.getLength() > nLen ) &&
             ( aChildURL.compareTo( aURL, nLen ) == 0 ) )
        {
            sal_Int32 nPos = nLen;
            nPos = aChildURL.indexOf( '/', nPos );

            if ( ( nPos == -1 ) ||
                 ( nPos == ( aChildURL.getLength() - 1 ) ) )
            {
                // No further slashes/ only a final slash. It's a child!
                rChildren.push_back(
                    HierarchyContentRef(
                        static_cast< HierarchyContent * >(
                            xChild.getBodyPtr() ) ) );
            }
        }
        ++it;
    }
}

//=========================================================================
sal_Bool HierarchyContent::exchangeIdentity(
                        const Reference< XContentIdentifier >& xNewId )
{
    if ( !xNewId.is() )
        return sal_False;

    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    Reference< XContent > xThis = this;

    // Already persistent?
    if ( m_eState != PERSISTENT )
    {
        VOS_ENSURE( sal_False,
                    "HierarchyContent::exchangeIdentity - Not persistent!" );
        return sal_False;
    }

    // Am I the root folder?
    if ( m_eKind == ROOT )
    {
        VOS_ENSURE( sal_False, "HierarchyContent::exchangeIdentity - "
                               "Not supported by root folder!" );
        return sal_False;
    }

    // Exchange own identitity.

    // Fail, if a content with given id already exists.
    if ( !hasData( xNewId ) )
    {
        OUString aOldURL = m_xIdentifier->getContentIdentifier();

        aGuard.clear();
        if ( exchange( xNewId ) )
        {
            if ( m_eKind == FOLDER )
            {
                // Process instanciated children...

                HierarchyContentRefList aChildren;
                queryChildren( aChildren );

                HierarchyContentRefList::const_iterator it  = aChildren.begin();
                HierarchyContentRefList::const_iterator end = aChildren.end();

                while ( it != end )
                {
                    HierarchyContentRef xChild = (*it);

                    // Create new content identifier for the child...
                    Reference< XContentIdentifier > xOldChildId
                                                    = xChild->getIdentifier();
                    OUString aOldChildURL = xOldChildId->getContentIdentifier();
                    OUString aNewChildURL
                        = aOldChildURL.replaceAt(
                                        0,
                                        aOldURL.getLength(),
                                        xNewId->getContentIdentifier() );
                    Reference< XContentIdentifier > xNewChildId
                        = new ::ucb::ContentIdentifier( m_xSMgr, aNewChildURL );

                    if ( !xChild->exchangeIdentity( xNewChildId ) )
                        return sal_False;

                    ++it;
                }
            }
            return sal_True;
        }
    }

    VOS_ENSURE( sal_False,
                "HierarchyContent::exchangeIdentity - "
                "Panic! Cannot exchange identity!" );
    return sal_False;
}

//=========================================================================
// static
Reference< XRow > HierarchyContent::getPropertyValues(
                const Reference< XMultiServiceFactory >& rSMgr,
                const Sequence< Property >& rProperties,
                const HierarchyContentProperties& rData,
                HierarchyContentProvider* pProvider,
                const OUString& rContentId )
{
    // Note: Empty sequence means "get values of all supported properties".

    vos::ORef< ::ucb::PropertyValueSet > xRow
                                = new ::ucb::PropertyValueSet( rSMgr );

    sal_Int32 nCount = rProperties.getLength();
    if ( nCount )
    {
        Reference< XPropertySet > xAdditionalPropSet;
        sal_Bool bTriedToGetAdditonalPropSet = sal_False;

        const Property* pProps = rProperties.getConstArray();
        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            const Property& rProp = pProps[ n ];

            // Process Core properties.

            if ( rProp.Name.compareToAscii( "ContentType" ) == 0 )
            {
                xRow->appendString ( rProp, rData.aContentType );
            }
            else if ( rProp.Name.compareToAscii( "Title" ) == 0 )
            {
                xRow->appendString ( rProp, rData.aTitle );
            }
            else if ( rProp.Name.compareToAscii( "IsDocument" ) == 0 )
            {
                xRow->appendBoolean( rProp, rData.bIsDocument );
            }
            else if ( rProp.Name.compareToAscii( "IsFolder" ) == 0 )
            {
                xRow->appendBoolean( rProp, rData.bIsFolder );
            }
            else if ( rProp.Name.compareToAscii( "TargetURL" ) == 0 )
            {
                // TargetURL is only supported by links.

                if ( rData.bIsDocument )
                    xRow->appendString( rProp, rData.aTargetURL );
                else
                    xRow->appendVoid( rProp );
            }

            else
            {
                // Not a Core Property! Maybe it's an Additional Core Property?!

                if ( !bTriedToGetAdditonalPropSet && !xAdditionalPropSet.is() )
                {
                    xAdditionalPropSet
                        = Reference< XPropertySet >(
                            pProvider->getAdditionalPropertySet( rContentId,
                                                                 sal_False ),
                            UNO_QUERY );
                    bTriedToGetAdditonalPropSet = sal_True;
                }

                if ( xAdditionalPropSet.is() )
                {
                    if ( !xRow->appendPropertySetValue(
                                                xAdditionalPropSet,
                                                rProp ) )
                    {
                        // Append empty entry.
                        xRow->appendVoid( rProp );
                    }
                }
                else
                {
                    // Append empty entry.
                    xRow->appendVoid( rProp );
                }
            }
        }
    }
    else
    {
        // Append all Core Properties.
        xRow->appendString (
            Property( OUString::createFromAscii( "ContentType" ),
                      -1,
                      getCppuType( static_cast< const OUString * >( 0 ) ),
                      PropertyAttribute::BOUND | PropertyAttribute::READONLY ),
            rData.aContentType );
        xRow->appendString (
            Property( OUString::createFromAscii( "Title" ),
                      -1,
                      getCppuType( static_cast< const OUString * >( 0 ) ),
                      PropertyAttribute::BOUND ),
            rData.aTitle );
        xRow->appendBoolean(
            Property( OUString::createFromAscii( "IsDocument" ),
                      -1,
                      getCppuBooleanType(),
                      PropertyAttribute::BOUND | PropertyAttribute::READONLY ),
            rData.bIsDocument );
        xRow->appendBoolean(
            Property( OUString::createFromAscii( "IsFolder" ),
                      -1,
                      getCppuBooleanType(),
                      PropertyAttribute::BOUND | PropertyAttribute::READONLY ),
            rData.bIsFolder );

        if ( rData.bIsDocument )
            xRow->appendString(
                Property( OUString::createFromAscii( "TargetURL" ),
                          -1,
                           getCppuType( static_cast< const OUString * >( 0 ) ),
                          PropertyAttribute::BOUND ),
                rData.aTargetURL );

        // Append all Additional Core Properties.

        Reference< XPropertySet > xSet(
            pProvider->getAdditionalPropertySet( rContentId, sal_False ),
            UNO_QUERY );
        xRow->appendPropertySet( xSet );
    }

    return Reference< XRow >( xRow.getBodyPtr() );
}

//=========================================================================
Reference< XRow > HierarchyContent::getPropertyValues(
                                const Sequence< Property >& rProperties )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    return getPropertyValues( m_xSMgr,
                              rProperties,
                              m_aProps,
                              m_pProvider,
                              m_xIdentifier->getContentIdentifier() );
}

//=========================================================================
void HierarchyContent::setPropertyValues(
                                const Sequence< PropertyValue >& rValues )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    Sequence< PropertyChangeEvent > aChanges( rValues.getLength() );
    sal_Int32 nChanged = 0;

    PropertyChangeEvent aEvent;
    aEvent.Source         = static_cast< OWeakObject * >( this );
    aEvent.Further        = sal_False;
//  aEvent.PropertyName   =
    aEvent.PropertyHandle = -1;
//  aEvent.OldValue       =
//  aEvent.NewValue       =

    const PropertyValue* pValues = rValues.getConstArray();
    sal_Int32 nCount = rValues.getLength();

    Reference< XPersistentPropertySet > xAdditionalPropSet;
    sal_Bool bTriedToGetAdditonalPropSet = sal_False;

    sal_Bool bExchange = sal_False;

    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const PropertyValue& rValue = pValues[ n ];

        if ( rValue.Name.compareToAscii( "ContentType" ) == 0 )
        {
            // Read-only property!
        }
        else if ( rValue.Name.compareToAscii( "IsDocument" ) == 0 )
        {
            // Read-only property!
        }
        else if ( rValue.Name.compareToAscii( "IsFolder" ) == 0 )
        {
            // Read-only property!
        }
        else if ( rValue.Name.compareToAscii( "Title" ) == 0 )
        {
            OUString aNewValue;
            if ( rValue.Value >>= aNewValue )
            {
                // No empty titles!
                if ( aNewValue.getLength() > 0 )
                {
                    if ( aNewValue != m_aProps.aTitle )
                    {
                        osl::Guard< osl::Mutex > aGuard( m_aMutex );

                        // modified title -> modified URL -> exchange !
                        if ( m_eState == PERSISTENT )
                            bExchange = sal_True;

                        aEvent.PropertyName = rValue.Name;
                        aEvent.OldValue     = makeAny( m_aProps.aTitle );
                        aEvent.NewValue     = makeAny( aNewValue );

                        aChanges.getArray()[ nChanged ] = aEvent;

                        m_aProps.aTitle = aNewValue;
                        m_aProps.aName
                            = HierarchyContentProvider::encodeSegment(
                                                                aNewValue );
                        nChanged++;
                    }
                }
            }
        }
        else if ( rValue.Name.compareToAscii( "TargetURL" ) == 0 )
        {
            // TargetURL is only supported by links.

            if ( m_eKind == LINK )
            {
                OUString aNewValue;
                if ( rValue.Value >>= aNewValue )
                {
                    // No empty target URL's!
                    if ( aNewValue.getLength() > 0 )
                    {
                        if ( aNewValue != m_aProps.aTargetURL )
                        {
                            osl::Guard< osl::Mutex > aGuard( m_aMutex );

                            aEvent.PropertyName = rValue.Name;
                            aEvent.OldValue = makeAny( m_aProps.aTargetURL );
                            aEvent.NewValue = makeAny( aNewValue );

                            aChanges.getArray()[ nChanged ] = aEvent;

                            m_aProps.aTargetURL = aNewValue;
                            nChanged++;
                        }
                    }
                }
            }
        }
        else
        {
            // Not a Core Property! Maybe it's an Additional Core Property?!

            if ( !bTriedToGetAdditonalPropSet && !xAdditionalPropSet.is() )
            {
                xAdditionalPropSet = getAdditionalPropertySet( sal_False );
                bTriedToGetAdditonalPropSet = sal_True;
            }

            if ( xAdditionalPropSet.is() )
            {
                try
                {
                    Any aOldValue = xAdditionalPropSet->getPropertyValue(
                                                                rValue.Name );
                    if ( aOldValue != rValue.Value )
                    {
                        xAdditionalPropSet->setPropertyValue(
                                                rValue.Name, rValue.Value );

                        aEvent.PropertyName = rValue.Name;
                        aEvent.OldValue     = aOldValue;
                        aEvent.NewValue     = rValue.Value;

                        aChanges.getArray()[ nChanged ] = aEvent;
                        nChanged++;
                    }
                }
                catch ( UnknownPropertyException )
                {
                }
                catch ( WrappedTargetException )
                {
                }
                catch ( PropertyVetoException )
                {
                }
                catch ( IllegalArgumentException )
                {
                }
            }
        }
    }

    // @@@ What, if exchange fails??? Rollback of Title prop? Old title is
    //     contained in aChanges...
    if ( bExchange )
    {
        Reference< XContentIdentifier > xOldId = m_xIdentifier;
        Reference< XContentIdentifier > xNewId = getIdentifierFromTitle();

        aGuard.clear();
        if ( exchangeIdentity( xNewId ) )
        {
            // Adapt persistent data.
            renameData( xOldId, xNewId );

            // Adapt Additional Core Properties.
            renameAdditionalPropertySet(
                            xOldId->getContentIdentifier(),
                            xNewId->getContentIdentifier(),
                            sal_True );
        }
    }

    if ( nChanged > 0 )
    {
        // Save changes, if content was already made persistent.
        if ( !bExchange && ( m_eState == PERSISTENT ) )
            storeData();

        aChanges.realloc( nChanged );
        notifyPropertiesChange( aChanges );
    }
}

//=========================================================================
void HierarchyContent::insert( sal_Int32 nNameClashResolve )
    throw( CommandAbortedException )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    // Transient?
    if ( m_eState != TRANSIENT )
    {
        VOS_ENSURE( sal_False, "HierarchyContent::insert - Not transient!" );
        throw CommandAbortedException();
    }

    // Am I the root folder?
    if ( m_eKind == ROOT )
    {
        VOS_ENSURE( sal_False, "HierarchyContent::insert - "
                               "Not supported by root folder!" );
        throw CommandAbortedException();
    }

    // Check, if all required properties were set.
    if ( m_aProps.aTitle.getLength() == 0 )
    {
        VOS_ENSURE( sal_False, "HierarchyContent::insert - No Title!" );
        throw CommandAbortedException();
    }

    // Assemble new content identifier...

    Reference< XContentIdentifier > xId = getIdentifierFromTitle();
    if ( !xId.is() )
        throw CommandAbortedException();

    if ( hasData( xId ) )
    {
        // Handle name clash...

        switch ( nNameClashResolve )
        {
            // fail.
            case NameClash::ERROR:
                throw CommandAbortedException();

            // replace existing object.
            case NameClash::OVERWRITE:
                break;

            // "invent" a new valid title.
            case NameClash::RENAME:
            {
                sal_Int32 nTry = 0;

                do
                {
                    OUString aNewId = xId->getContentIdentifier();
                    aNewId += OUString::createFromAscii( "_" );
                    aNewId += OUString::valueOf( ++nTry );
                    xId = new ::ucb::ContentIdentifier( m_xSMgr, aNewId );
                }
                while ( hasData( xId ) && ( nTry < 100000 ) );

                if ( nTry == 100000 )
                {
                    VOS_ENSURE( sal_False,
                                "HierarchyContent::insert - "
                                "Unable to resolve name clash" );
                    throw CommandAbortedException();
                }
                else
                {
                    m_aProps.aTitle += OUString::createFromAscii( "_" );
                    m_aProps.aTitle += OUString::valueOf( nTry );
                }
                break;
            }

            // keep existing sub-objects, transfer non-clashing sub-objects.
            case NameClash::KEEP:
                // @@@

            default:
                throw CommandAbortedException();
        }
    }

    m_xIdentifier = xId;

    if ( !storeData() )
        throw CommandAbortedException();

    m_eState = PERSISTENT;

    aGuard.clear();
    inserted();
}

//=========================================================================
void HierarchyContent::destroy( sal_Bool bDeletePhysical )
    throw( CommandAbortedException )
{
    // @@@ take care about bDeletePhysical -> trashcan support

    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    Reference< XContent > xThis = this;

    // Persistent?
    if ( m_eState != PERSISTENT )
    {
        VOS_ENSURE( sal_False, "HierarchyContent::destroy - Not persistent!" );
        throw CommandAbortedException();
    }

    // Am I the root folder?
    if ( m_eKind == ROOT )
    {
        VOS_ENSURE( sal_False, "HierarchyContent::destroy - "
                               "Not supported by root folder!" );
        throw CommandAbortedException();
    }

    m_eState = DEAD;

    aGuard.clear();
    deleted();

    if ( m_eKind == FOLDER )
    {
        // Process instanciated children...

        HierarchyContentRefList aChildren;
        queryChildren( aChildren );

        HierarchyContentRefList::const_iterator it  = aChildren.begin();
        HierarchyContentRefList::const_iterator end = aChildren.end();

        while ( it != end )
        {
            (*it)->destroy( bDeletePhysical );
            ++it;
        }
    }
}

//=========================================================================
void HierarchyContent::transfer( const TransferInfo& rInfo,
                                    const Reference< XCommandEnvironment > & xEnv )

    throw( CommandAbortedException )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    // Persistent?
    if ( m_eState != PERSISTENT )
    {
        VOS_ENSURE( sal_False, "HierarchyContent::transfer - Not persistent!" );
        throw CommandAbortedException();
    }

    if ( rInfo.SourceURL.getLength() == 0 )
        throw CommandAbortedException();

    // Is source a hierarchy content?
    if ( rInfo.SourceURL.compareToAscii(
            HIERARCHY_ROOT_FOLDER_URL, HIERARCHY_ROOT_FOLDER_URL_LENGTH ) != 0 )
        throw InteractiveBadTransferURLException();

    // Is source not a parent of me / not me?
    OUString aId = m_xIdentifier->getContentIdentifier();
    sal_Int32 nPos = aId.lastIndexOf( '/' );
    if ( nPos != ( aId.getLength() - 1 ) )
    {
        // No trailing slash found. Append.
        aId += OUString::createFromAscii( "/" );
    }

    if ( rInfo.SourceURL.getLength() <= aId.getLength() )
    {
        if ( aId.compareTo(
                rInfo.SourceURL, rInfo.SourceURL.getLength() ) == 0 )
            throw CommandAbortedException();
    }

    try
    {
        //////////////////////////////////////////////////////////////////
        // 0) Obtain content object for source.
        //////////////////////////////////////////////////////////////////

        Reference< XContentIdentifier > xId =
                    new ::ucb::ContentIdentifier( m_xSMgr, rInfo.SourceURL );

        // Note: The static cast is okay here, because its sure that
        //       m_xProvider is always the HierarchyContentProvider.
        vos::ORef< HierarchyContent > xSource
            = static_cast< HierarchyContent * >(
                m_xProvider->queryContent( xId ).get() );
        if ( !xSource.isValid() )
            throw CommandAbortedException();

        //////////////////////////////////////////////////////////////////
        // 1) Create new child content.
        //////////////////////////////////////////////////////////////////

        OUString aType = xSource->isFolder()
                       ? OUString::createFromAscii(
                               HIERARCHY_FOLDER_CONTENT_TYPE )
                       : OUString::createFromAscii(
                               HIERARCHY_LINK_CONTENT_TYPE );
        ContentInfo aInfo;
        aInfo.Type = aType;
        aInfo.Attributes = 0;

        // Note: The static cast is okay here, because its sure that
        //       createNewContent always creates a HierarchyContent.
        vos::ORef< HierarchyContent > xTarget
            = static_cast< HierarchyContent * >(
                createNewContent( aInfo ).get() );
        if ( !xTarget.isValid() )
            throw CommandAbortedException();

        //////////////////////////////////////////////////////////////////
        // 2) Copy data from source content to child content.
        //////////////////////////////////////////////////////////////////

        Sequence< Property > aProps
                        = xSource->getPropertySetInfo( xEnv )->getProperties();
        sal_Int32 nCount = aProps.getLength();

        if ( nCount )
        {
            sal_Bool bHadTitle = ( rInfo.NewTitle.getLength() == 0 );

            // Get all source values.
            Reference< XRow > xRow = xSource->getPropertyValues( aProps );

            Sequence< PropertyValue > aValues( nCount );
            PropertyValue* pValues = aValues.getArray();

            const Property* pProps = aProps.getConstArray();
            for ( sal_Int32 n = 0; n < nCount; ++n )
            {
                const Property& rProp  = pProps[ n ];
                PropertyValue&  rValue = pValues[ n ];

                rValue.Name   = rProp.Name;
                rValue.Handle = rProp.Handle;

                if ( !bHadTitle && rProp.Name.compareToAscii( "Title" ) == 0 )
                {
                    // Set new title instead of original.
                    bHadTitle = sal_True;
                    rValue.Value <<= rInfo.NewTitle;
                }
                else
                    rValue.Value
                        = xRow->getObject( n + 1, Reference< XNameAccess >() );

                rValue.State = PropertyState_DIRECT_VALUE;

                if ( rProp.Attributes & PropertyAttribute::REMOVABLE )
                {
                    // Add Additional Core Property.
                    try
                    {
                        xTarget->addProperty( rProp.Name,
                                              rProp.Attributes,
                                              rValue.Value );
                    }
                    catch ( PropertyExistException & )
                    {
                    }
                    catch ( IllegalTypeException & )
                    {
                    }
                    catch ( IllegalArgumentException & )
                    {
                    }
                }
            }

            // Set target values.
            xTarget->setPropertyValues( aValues );
        }

        //////////////////////////////////////////////////////////////////
        // 3) Commit (insert) child.
        //////////////////////////////////////////////////////////////////

        xTarget->insert( rInfo.NameClash );

        //////////////////////////////////////////////////////////////////
        // 4) Transfer (copy) children of source.
        //////////////////////////////////////////////////////////////////

        if ( xSource->isFolder() )
        {
            HierarchyEntry aFolder(
                    m_xSMgr, m_pProvider, xId->getContentIdentifier() );
            HierarchyEntry::iterator it;

            while ( aFolder.next( it ) )
            {
                const HierarchyEntryData& rResult = *it;

                OUString aChildId = xId->getContentIdentifier();
                if ( ( aChildId.lastIndexOf( '/' ) + 1 )
                                                != aChildId.getLength() )
                    aChildId += OUString::createFromAscii( "/" );

                aChildId += rResult.aName;

                Reference< XContentIdentifier > xChildId
                    = new ::ucb::ContentIdentifier( m_xSMgr, aChildId );

                vos::ORef< HierarchyContent > xChild
                    = static_cast< HierarchyContent * >(
                            m_xProvider->queryContent( xChildId ).get() );

                TransferInfo aInfo;
                aInfo.MoveData  = sal_False;
                aInfo.NewTitle  = OUString();
                aInfo.SourceURL = aChildId;
                aInfo.NameClash = rInfo.NameClash;

                // Transfer child to target.
                xTarget->transfer( aInfo, xEnv );
            }
        }

        //////////////////////////////////////////////////////////////////
        // 5) Destroy source ( when moving only ) .
        //////////////////////////////////////////////////////////////////

        if ( rInfo.MoveData )
        {
            xSource->destroy( sal_True );

            // Remove all persistent data of source and its children.
            xSource->removeData();

            // Remove own and all children's Additional Core Properties.
            xSource->removeAdditionalPropertySet( sal_True );
        }
    }
    catch ( IllegalIdentifierException & )
    {
        // queryContent
        VOS_ENSURE( sal_False, "HierarchyContent::transfer - "
                               "Caught IllegalIdentifierException!" );
        throw CommandAbortedException();
    }
}

