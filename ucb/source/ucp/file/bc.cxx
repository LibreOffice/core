/*************************************************************************
 *
 *  $RCSfile: bc.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hro $ $Date: 2001-04-03 12:06:25 $
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
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _VOS_DIAGNOSE_HXX_
#include <vos/diagnose.hxx>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENMODE_HPP_
#include <com/sun/star/ucb/OpenMode.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XPROGRESSHANDLER_HPP_
#include <com/sun/star/ucb/XProgressHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASTREAMER_HPP_
#include <com/sun/star/io/XActiveDataStreamer.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_NUMBEREDSORTINGINFO_HPP_
#include <com/sun/star/ucb/NumberedSortingInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYCHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSETINFOCHANGE_HPP_
#include <com/sun/star/beans/PropertySetInfoChange.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_CONTENTACTION_HPP_
#include <com/sun/star/ucb/ContentAction.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_NAMECLASH_HPP_
#include <com/sun/star/ucb/NameClash.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_CONTENTINFOATTRIBUTE_HPP_
#include <com/sun/star/ucb/ContentInfoAttribute.hpp>
#endif

#ifndef _FILGLOB_HXX_
#include "filglob.hxx"
#endif
#ifndef _FILID_HXX_
#include "filid.hxx"
#endif
#ifndef _FILROW_HXX_
#include "filrow.hxx"
#endif
#ifndef _BC_HXX_
#include "bc.hxx"
#endif
#ifndef _PROV_HXX_
#include "prov.hxx"
#endif

using namespace fileaccess;
using namespace com::sun::star;
using namespace com::sun::star::ucb;

// PropertyListeners


typedef cppu::OMultiTypeInterfaceContainerHelperVar< rtl::OUString,hashOUString,equalOUString >
PropertyListeners_impl;

class fileaccess::PropertyListeners
    : public PropertyListeners_impl
{
public:
    PropertyListeners( ::osl::Mutex& aMutex )
        : PropertyListeners_impl( aMutex )
    {
    }
};


/****************************************************************************************/
/*                                                                                      */
/*                    BaseContent                                                       */
/*                                                                                      */
/****************************************************************************************/

////////////////////////////////////////////////////////////////////////////////
// Private Constructor for just inserted Contents

BaseContent::BaseContent( shell* pMyShell,
                          const rtl::OUString& parentName,
                          sal_Bool bFolder )
    : m_pMyShell( pMyShell ),
      m_xContentIdentifier( 0 ),
      m_pDisposeEventListeners( 0 ),
      m_pContentEventListeners( 0 ),
      m_pPropertySetInfoChangeListeners( 0 ),
      m_pPropertyListener( 0 ),
      m_aUncPath( parentName ),
      m_nState( JustInserted ),
      m_bFolder( bFolder )
{
    m_pMyShell->m_pProvider->acquire();
    // No registering, since we have no name
}


////////////////////////////////////////////////////////////////////////////////
// Constructor for full featured Contents

BaseContent::BaseContent( shell* pMyShell,
                          const uno::Reference< XContentIdentifier >& xContentIdentifier,
                          const rtl::OUString& aUncPath )
    : m_pMyShell( pMyShell ),
      m_xContentIdentifier( xContentIdentifier ),
      m_pDisposeEventListeners( 0 ),
      m_pContentEventListeners( 0 ),
      m_pPropertySetInfoChangeListeners( 0 ),
      m_pPropertyListener( 0 ),
      m_aUncPath( aUncPath ),
      m_nState( FullFeatured ),
      m_bFolder( false )
{
    m_pMyShell->m_pProvider->acquire();
    m_pMyShell->registerNotifier( m_aUncPath,this );
    m_pMyShell->InsertDefaultProperties( m_aUncPath );
}


BaseContent::~BaseContent( )
{
    if( ( m_nState & FullFeatured ) || ( m_nState & Deleted ) )
    {
        m_pMyShell->deregisterNotifier( m_aUncPath,this );
    }
    m_pMyShell->m_pProvider->release();

    if( m_pDisposeEventListeners )
        delete m_pDisposeEventListeners;
    if( m_pContentEventListeners )
        delete m_pContentEventListeners;
    if( m_pPropertyListener )
        delete m_pPropertyListener;
    if( m_pPropertySetInfoChangeListeners )
        delete m_pPropertySetInfoChangeListeners;
}


//////////////////////////////////////////////////////////////////////////
// XInterface
//////////////////////////////////////////////////////////////////////////

void SAL_CALL
BaseContent::acquire( void )
    throw( uno::RuntimeException )
{
    OWeakObject::acquire();
}


void SAL_CALL
BaseContent::release( void )
    throw( uno::RuntimeException )
{
    OWeakObject::release();
}


uno::Any SAL_CALL
BaseContent::queryInterface( const uno::Type& rType )
    throw( uno::RuntimeException )
{
    uno::Any aRet = cppu::queryInterface( rType,
                                          SAL_STATIC_CAST( lang::XComponent*, this ),
                                          SAL_STATIC_CAST( lang::XServiceInfo*, this ),
                                          SAL_STATIC_CAST( XCommandProcessor*, this ),
                                          SAL_STATIC_CAST( container::XChild*, this ),
                                          SAL_STATIC_CAST( beans::XPropertiesChangeNotifier*, this ),
                                          SAL_STATIC_CAST( beans::XPropertyContainer*, this ),
                                          SAL_STATIC_CAST( XContentCreator*,this ),
                                          SAL_STATIC_CAST( beans::XPropertySetInfoChangeNotifier*, this ),
                                          SAL_STATIC_CAST( XContent*,this) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}




//////////////////////////////////////////////////////////////////////////////////////////
// XComponent
////////////////////////////////////////////////////////////////////////////////////////

void SAL_CALL
BaseContent::addEventListener( const uno::Reference< lang::XEventListener >& Listener )
    throw( uno::RuntimeException )
{
    vos::OGuard aGuard( m_aMutex );

    if ( ! m_pDisposeEventListeners )
        m_pDisposeEventListeners =
            new cppu::OInterfaceContainerHelper( m_aEventListenerMutex );

    m_pDisposeEventListeners->addInterface( Listener );
}


void SAL_CALL
BaseContent::removeEventListener( const uno::Reference< lang::XEventListener >& Listener )
    throw( uno::RuntimeException )
{
    vos::OGuard aGuard( m_aMutex );

    if ( m_pDisposeEventListeners )
        m_pDisposeEventListeners->removeInterface( Listener );
}


void SAL_CALL
BaseContent::dispose()
    throw( uno::RuntimeException )
{
    vos::OGuard aGuard( m_aMutex );
    lang::EventObject aEvt;
    aEvt.Source = static_cast< XContent* >( this );

    if ( m_pDisposeEventListeners && m_pDisposeEventListeners->getLength() )
        m_pDisposeEventListeners->disposeAndClear( aEvt );

    if ( m_pContentEventListeners && m_pContentEventListeners->getLength() )
        m_pContentEventListeners->disposeAndClear( aEvt );

    if( m_pPropertyListener )
        m_pPropertyListener->disposeAndClear( aEvt );

    if( m_pPropertySetInfoChangeListeners )
        m_pPropertySetInfoChangeListeners->disposeAndClear( aEvt );

}





rtl::OUString SAL_CALL
BaseContent::getImplementationName()
    throw( uno::RuntimeException)
{
    return rtl::OUString::createFromAscii( "FileContent" );
}



sal_Bool SAL_CALL
BaseContent::supportsService( const rtl::OUString& ServiceName )
    throw( uno::RuntimeException)
{
    if( ServiceName.compareToAscii( "com.sun.star.ucb.FileContent" ) == 0 )
        return true;
    else
        return false;
}



uno::Sequence< rtl::OUString > SAL_CALL
BaseContent::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    uno::Sequence< rtl::OUString > ret( 1 );
    ret[0] = rtl::OUString::createFromAscii( "com.sun.star.ucb.FileContent" );
    return ret;
}



//////////////////////////////////////////////////////////////////////////////////////////
//  XCommandProcessor
//////////////////////////////////////////////////////////////////////////////////////////

sal_Int32 SAL_CALL
BaseContent::createCommandIdentifier( void )
    throw( uno::RuntimeException )
{
    return m_pMyShell->getCommandId();
}


void SAL_CALL
BaseContent::abort( sal_Int32 CommandId )
    throw( uno::RuntimeException )
{
    m_pMyShell->abort( CommandId );
}


uno::Any SAL_CALL
BaseContent::execute( const Command& aCommand,
                      sal_Int32 CommandId,
                      const uno::Reference< XCommandEnvironment >& Environment )
    throw( uno::Exception,
           CommandAbortedException,
           uno::RuntimeException )
{
    if( CommandId )
    {
        uno::Reference< task::XInteractionHandler > xIH( 0 );
        uno::Reference< ucb::XProgressHandler > xPH( 0 );

        if( Environment.is() )
        {
            xIH = Environment->getInteractionHandler();
            xPH = Environment->getProgressHandler();
        }
        m_pMyShell->startTask( CommandId,
                               xIH,
                               xPH );
    }

    uno::Any aAny;
    sal_Bool success = true;   // Hope the best

    try
    {
        if( ! aCommand.Name.compareToAscii( "getPropertySetInfo" ) )
        {
            aAny <<= getPropertySetInfo( CommandId );
        }
        else if( ! aCommand.Name.compareToAscii( "getCommandInfo" ) )
        {
            aAny <<= getCommandInfo( CommandId );
        }
        else if( ! aCommand.Name.compareToAscii( "setPropertyValues" ) )
        {
            uno::Sequence< beans::PropertyValue > sPropertyValues;

            if( ! ( aCommand.Argument >>= sPropertyValues ) )
                throw CommandAbortedException();

            setPropertyValues( CommandId,sPropertyValues );
            aAny <<= getCppuVoidType();
        }
        else if( ! aCommand.Name.compareToAscii( "getPropertyValues" ) )
        {
            uno::Sequence< beans::Property > ListOfRequestedProperties;

            if( ! ( aCommand.Argument >>= ListOfRequestedProperties ) )
                throw CommandAbortedException();

            aAny <<= getPropertyValues( CommandId,
                                        ListOfRequestedProperties );
        }
        else if( ! aCommand.Name.compareToAscii( "open" ) )
        {
            OpenCommandArgument2 aOpenArgument;
            OpenCommandArgument  aFalseCommandArgument;
            if( ! ( aCommand.Argument >>= aOpenArgument ) )
            {
                if( ! ( aCommand.Argument >>= aFalseCommandArgument ) )
                    throw CommandAbortedException();

                if ( ( aFalseCommandArgument.Mode
                            == OpenMode::DOCUMENT_SHARE_DENY_NONE ) ||
                     ( aFalseCommandArgument.Mode
                             == OpenMode::DOCUMENT_SHARE_DENY_WRITE ) )
                {
                    // Currently unsupported.
                      throw CommandAbortedException();
                }

                aOpenArgument.Mode = aFalseCommandArgument.Mode;
                aOpenArgument.Priority = aFalseCommandArgument.Priority;
                aOpenArgument.Sink = aFalseCommandArgument.Sink;
                aOpenArgument.Properties = aFalseCommandArgument.Properties;
                aOpenArgument.SortingInfo = uno::Sequence< NumberedSortingInfo >( 0 );
            }

            uno::Reference< XDynamicResultSet > result = open( CommandId,aOpenArgument );
            if( result.is() )
            {
                aAny <<= result;
            }
            else
            {
                aAny <<= getCppuVoidType();
            }
        }
        else if( ! aCommand.Name.compareToAscii( "delete" ) )
        {
            sal_Bool aDeleteArgument;
            if( ! ( aCommand.Argument >>= aDeleteArgument ) )
                throw CommandAbortedException();
            deleteContent( CommandId,
                           aDeleteArgument );
            aAny <<= getCppuVoidType();
        }
        else if( ! aCommand.Name.compareToAscii( "transfer" ) )
        {
            TransferInfo aTransferInfo;
            if( ! ( aCommand.Argument >>= aTransferInfo ) )
                throw CommandAbortedException();
            transfer( CommandId,
                      aTransferInfo );
            aAny <<= getCppuVoidType();
        }
        else if( ! aCommand.Name.compareToAscii( "insert" ) )
        {
            InsertCommandArgument aInsertArgument;
            if( ! ( aCommand.Argument >>= aInsertArgument ) )
                throw CommandAbortedException();

            insert( CommandId,aInsertArgument );
            aAny <<= getCppuVoidType();
        }
        else if( ! aCommand.Name.compareToAscii( "update" ) )
        {
            aAny <<= getCppuVoidType();
        }
        else
        {
            success = false;
        }

        if( CommandId )
            m_pMyShell->endTask( CommandId );

        if( ! success )
            throw CommandAbortedException();

        return aAny;
    }
    catch( ... )
    {
        if( CommandId )
            m_pMyShell->endTask( CommandId );
        throw;
    }
}





void SAL_CALL
BaseContent::addPropertiesChangeListener(
    const uno::Sequence< rtl::OUString >& PropertyNames,
    const uno::Reference< beans::XPropertiesChangeListener >& Listener )
    throw( uno::RuntimeException )
{
    if( ! Listener.is() )
        return;

    vos::OGuard aGuard( m_aMutex );

    if( ! m_pPropertyListener )
        m_pPropertyListener = new PropertyListeners( m_aEventListenerMutex );


    if( PropertyNames.getLength() == 0 )
        m_pPropertyListener->addInterface( rtl::OUString(),Listener );
    else
    {
        uno::Reference< beans::XPropertySetInfo > xProp = m_pMyShell->info_p( -1,m_aUncPath );
        for( sal_Int32 i = 0; i < PropertyNames.getLength(); ++i )
            if( xProp->hasPropertyByName( PropertyNames[i] ) )
                m_pPropertyListener->addInterface( PropertyNames[i],Listener );
    }
}


void SAL_CALL
BaseContent::removePropertiesChangeListener( const uno::Sequence< rtl::OUString >& PropertyNames,
                                             const uno::Reference< beans::XPropertiesChangeListener >& Listener )
    throw( com::sun::star::uno::RuntimeException )
{
    if( ! Listener.is() || ! m_pPropertyListener )
        return;

    vos::OGuard aGuard( m_aMutex );

    for( sal_Int32 i = 0; i < PropertyNames.getLength(); ++i )
        m_pPropertyListener->removeInterface( PropertyNames[i],Listener );

    m_pPropertyListener->removeInterface( rtl::OUString(), Listener );
}


/////////////////////////////////////////////////////////////////////////////////////////
// XContent
/////////////////////////////////////////////////////////////////////////////////////////

uno::Reference< ucb::XContentIdentifier > SAL_CALL
BaseContent::getIdentifier()
    throw( uno::RuntimeException )
{
    return m_xContentIdentifier;
}


rtl::OUString SAL_CALL
BaseContent::getContentType()
    throw( uno::RuntimeException )
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
                uno::Sequence< beans::Property > seq(1);
                seq[0] = beans::Property( rtl::OUString::createFromAscii("IsDocument"),
                                                -1,
                                                getCppuType( static_cast< sal_Bool* >(0) ),
                                                0 );
                uno::Reference< sdbc::XRow > xRow = getPropertyValues( -1,seq );
                sal_Bool IsDocument = xRow->getBoolean( 1 );

                if ( !xRow->wasNull() )
                {
                    if ( IsDocument )
                        return m_pMyShell->FileContentType;
                    else
                        return m_pMyShell->FolderContentType;
                }
                else
                {
                    VOS_ENSURE( false,
                                "BaseContent::getContentType - Property value was null!" );
                }
            }
            catch ( sdbc::SQLException const & )
            {
                VOS_ENSURE( false,
                            "BaseContent::getContentType - Caught SQLException!" );
            }
        }
    }

    return rtl::OUString();
}



void SAL_CALL
BaseContent::addContentEventListener(
    const uno::Reference< XContentEventListener >& Listener )
    throw( uno::RuntimeException )
{
    vos::OGuard aGuard( m_aMutex );

    if ( ! m_pContentEventListeners )
        m_pContentEventListeners =
            new cppu::OInterfaceContainerHelper( m_aEventListenerMutex );


    m_pContentEventListeners->addInterface( Listener );
}


void SAL_CALL
BaseContent::removeContentEventListener(
    const uno::Reference< XContentEventListener >& Listener )
    throw( uno::RuntimeException )
{
    vos::OGuard aGuard( m_aMutex );

    if ( m_pContentEventListeners )
        m_pContentEventListeners->removeInterface( Listener );
}



////////////////////////////////////////////////////////////////////////////////
// XPropertyContainer
////////////////////////////////////////////////////////////////////////////////


void SAL_CALL
BaseContent::addProperty(
    const rtl::OUString& Name,
    sal_Int16 Attributes,
    const uno::Any& DefaultValue )
    throw( beans::PropertyExistException,
           beans::IllegalTypeException,
           lang::IllegalArgumentException,
           uno::RuntimeException)
{
    if( ( m_nState & JustInserted ) || ( m_nState & Deleted ) || Name == rtl::OUString() )
    {
        throw lang::IllegalArgumentException();
    }

    m_pMyShell->associate( m_aUncPath,Name,DefaultValue,Attributes );
}


void SAL_CALL
BaseContent::removeProperty(
    const rtl::OUString& Name )
    throw( beans::UnknownPropertyException,
           beans::NotRemoveableException,
           uno::RuntimeException)
{

    if( m_nState & Deleted )
        throw beans::UnknownPropertyException();

    m_pMyShell->deassociate( m_aUncPath, Name );
}

////////////////////////////////////////////////////////////////////////////////
// XContentCreator
////////////////////////////////////////////////////////////////////////////////

uno::Sequence< ContentInfo > SAL_CALL
BaseContent::queryCreatableContentsInfo(
    void )
    throw( uno::RuntimeException )
{
    uno::Sequence< ContentInfo > seq(2);

    // file
    seq[0].Type       = m_pMyShell->FileContentType;
    seq[0].Attributes = ContentInfoAttribute::INSERT_WITH_INPUTSTREAM
                        | ContentInfoAttribute::KIND_DOCUMENT;

    uno::Sequence< beans::Property > props( 1 );
    props[0] = beans::Property(
                        rtl::OUString::createFromAscii( "Title" ),
                        -1,
                        getCppuType( static_cast< rtl::OUString* >( 0 ) ),
                        beans::PropertyAttribute::MAYBEVOID
                        | beans::PropertyAttribute::BOUND );
    seq[0].Properties = props;

    // folder
    seq[1].Type       = m_pMyShell->FolderContentType;
    seq[1].Attributes = ContentInfoAttribute::KIND_FOLDER;
    seq[1].Properties = props;
    return seq;
}


uno::Reference< XContent > SAL_CALL
BaseContent::createNewContent(
    const ContentInfo& Info )
    throw( uno::RuntimeException )
{
    // Check type.
    if ( !Info.Type.getLength() )
        return uno::Reference< XContent >();

    sal_Bool bFolder
        = ( Info.Type.compareTo( m_pMyShell->FolderContentType ) == 0 );
    if ( !bFolder )
    {
        if ( Info.Type.compareTo( m_pMyShell->FileContentType ) != 0 )
        {
            // Neither folder nor file to create!
            return uno::Reference< XContent >();
        }
    }

    // Who am I ?
    sal_Bool IsDocument = false;

    try
    {
        uno::Sequence< beans::Property > seq(1);
        seq[0] = beans::Property( rtl::OUString::createFromAscii("IsDocument"),
                                    -1,
                                    getCppuType( static_cast< sal_Bool* >(0) ),
                                    0 );
        uno::Reference< sdbc::XRow > xRow = getPropertyValues( -1,seq );
        IsDocument = xRow->getBoolean( 1 );

        if ( xRow->wasNull() )
        {
            VOS_ENSURE( false,
                        "BaseContent::createNewContent - Property value was null!" );
            return uno::Reference< XContent >();
        }
    }
    catch ( sdbc::SQLException const & )
    {
        VOS_ENSURE( false,
                    "BaseContent::createNewContent - Caught SQLException!" );
        return uno::Reference< XContent >();
    }

    rtl::OUString dstUncPath;

    if( IsDocument )
    {
        // KSO: Why is a document a XContentCreator? This is quite unusual.
        dstUncPath = m_pMyShell->getParentName( m_aUncPath );
    }
    else
        dstUncPath = m_aUncPath;

    BaseContent* p = new BaseContent( m_pMyShell, dstUncPath, bFolder );
    return uno::Reference< XContent >( p );
}


////////////////////////////////////////////////////////////////////////////////
// XPropertySetInfoChangeNotifier
////////////////////////////////////////////////////////////////////////////////


void SAL_CALL
BaseContent::addPropertySetInfoChangeListener(
    const uno::Reference< beans::XPropertySetInfoChangeListener >& Listener )
    throw( uno::RuntimeException )
{
    vos::OGuard aGuard( m_aMutex );
    if( ! m_pPropertySetInfoChangeListeners )
        m_pPropertySetInfoChangeListeners = new cppu::OInterfaceContainerHelper( m_aEventListenerMutex );

    m_pPropertySetInfoChangeListeners->addInterface( Listener );
}


void SAL_CALL
BaseContent::removePropertySetInfoChangeListener(
    const uno::Reference< beans::XPropertySetInfoChangeListener >& Listener )
    throw( uno::RuntimeException )
{
    vos::OGuard aGuard( m_aMutex );

    if( m_pPropertySetInfoChangeListeners )
        m_pPropertySetInfoChangeListeners->removeInterface( Listener );
}


////////////////////////////////////////////////////////////////////////////////
// XChild
////////////////////////////////////////////////////////////////////////////////

uno::Reference< uno::XInterface > SAL_CALL
BaseContent::getParent(
    void )
    throw( uno::RuntimeException )
{
    rtl::OUString ParentUnq = m_pMyShell->getParentName( m_aUncPath );
    rtl::OUString ParentUrl;


    sal_Bool err = m_pMyShell->getUrlFromUnq( ParentUnq, ParentUrl );
    if( err )
        return uno::Reference< uno::XInterface >( 0 );

    FileContentIdentifier* p = new FileContentIdentifier( m_pMyShell,ParentUnq );
    uno::Reference< XContentIdentifier > Identifier( p );

    try
    {
        uno::Reference< XContent > content = m_pMyShell->m_pProvider->queryContent( Identifier );
        return content;
    }
    catch( IllegalIdentifierException )
    {
        return uno::Reference< uno::XInterface >();
    }
}


void SAL_CALL
BaseContent::setParent(
    const uno::Reference< uno::XInterface >& Parent )
    throw( lang::NoSupportException,
           uno::RuntimeException)
{
    throw lang::NoSupportException();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Private Methods
//////////////////////////////////////////////////////////////////////////////////////////


uno::Reference< XCommandInfo > SAL_CALL
BaseContent::getCommandInfo(
    sal_Int32 nMyCommandIdentifier )
    throw( uno::RuntimeException )
{
    if( m_nState & Deleted )
        return uno::Reference< XCommandInfo >();

    return m_pMyShell->info_c( nMyCommandIdentifier,m_aUncPath );
}


uno::Reference< beans::XPropertySetInfo > SAL_CALL
BaseContent::getPropertySetInfo(
    sal_Int32 nMyCommandIdentifier )
    throw( uno::RuntimeException )
{
    if( m_nState & Deleted )
        return uno::Reference< beans::XPropertySetInfo >();

    return m_pMyShell->info_p( nMyCommandIdentifier,m_aUncPath );
}




uno::Reference< sdbc::XRow > SAL_CALL
BaseContent::getPropertyValues(
    sal_Int32 nMyCommandIdentifier,
    const uno::Sequence< beans::Property >& PropertySet )
    throw( uno::RuntimeException )
{
    sal_Int32 nProps = PropertySet.getLength();
    if ( !nProps )
        return uno::Reference< sdbc::XRow >();

    if( m_nState & Deleted )
    {
        uno::Sequence< uno::Any > aValues( nProps );
        return uno::Reference< sdbc::XRow >( new XRow_impl( m_pMyShell, aValues ) );
    }

    if( m_nState & JustInserted )
    {
        uno::Sequence< uno::Any > aValues( nProps );
        uno::Any* pValues = aValues.getArray();

        const beans::Property* pProps = PropertySet.getConstArray();

        for ( sal_Int32 n = 0; n < nProps; ++n )
        {
            const beans::Property& rProp = pProps[ n ];
            uno::Any& rValue = pValues[ n ];

            if ( rProp.Name.compareToAscii( "ContentType" ) == 0 )
            {
                rValue <<= m_bFolder ? m_pMyShell->FolderContentType
                                     : m_pMyShell->FileContentType;
            }
            else if ( rProp.Name.compareToAscii( "IsFolder" ) == 0 )
            {
                rValue <<= m_bFolder;
            }
            else if ( rProp.Name.compareToAscii( "IsDocument" ) == 0 )
            {
                rValue <<= sal_Bool( !m_bFolder );
            }
//          else
//              rValue = uno::Any();
        }

        return uno::Reference< sdbc::XRow >(
                                    new XRow_impl( m_pMyShell, aValues ) );
    }

    return m_pMyShell->getv( nMyCommandIdentifier,
                             m_aUncPath,
                             PropertySet );
}


void SAL_CALL
BaseContent::setPropertyValues(
    sal_Int32 nMyCommandIdentifier,
    const uno::Sequence< beans::PropertyValue >& Values )
    throw( uno::RuntimeException )
{
    if( m_nState & Deleted )
    {
        return;
    }

    rtl::OUString Title = rtl::OUString::createFromAscii( "Title" );
    sal_Unicode slash = '/';


    // Special handling for files which have to be inserted
    if( m_nState & JustInserted )
    {
        for( sal_Int32 i = 0; i < Values.getLength(); ++i )
        {

            if( Values[i].Name == Title && ! ( m_nState & NameForInsertionSet ) )
            {
                rtl::OUString NewTitle;
                if( Values[i].Value >>= NewTitle )
                {
                    if( m_aUncPath.lastIndexOf( sal_Unicode('/') ) != m_aUncPath.getLength() - 1 )
                        m_aUncPath += rtl::OUString::createFromAscii("/");

                    m_aUncPath += NewTitle;
                    m_nState |= NameForInsertionSet;
                }
            }
        }
    }
    else
    {
        m_pMyShell->setv( nMyCommandIdentifier,      // Does not handle Title
                          m_aUncPath,
                          Values );



        // Special handling Title: Setting Title is equivalent to a renaming of the underlying file
        for( sal_Int32 i = 0; i < Values.getLength(); ++i )
        {
            if( Values[i].Name == Title )
            {
                rtl::OUString NewTitle;
                if( Values[i].Value >>= NewTitle  )
                {
                    rtl::OUString aDstName = m_pMyShell->getParentName( m_aUncPath );
                    if( aDstName.lastIndexOf( sal_Unicode('/') ) != aDstName.getLength() - 1 )
                        aDstName += rtl::OUString::createFromAscii("/");

                    aDstName += NewTitle;

                    try
                    {
                        m_pMyShell->move( nMyCommandIdentifier,     // move notifies the childs also ;
                                          m_aUncPath,
                                          aDstName,
                                          NameClash::KEEP );
                    }
                    catch( const CommandAbortedException& )
                    {
                    }
                }
                // NameChanges come back trough a ContentEvent
                //
                break;
            }
        }
    }
}



uno::Reference< XDynamicResultSet > SAL_CALL
BaseContent::open(
    sal_Int32 nMyCommandIdentifier,
    const OpenCommandArgument2& aCommandArgument )
    throw( CommandAbortedException )
{
    if( ( m_nState & Deleted ) || ( m_nState & JustInserted ) )
        return uno::Reference< XDynamicResultSet >();


    uno::Reference< io::XOutputStream > outputStream( aCommandArgument.Sink,uno::UNO_QUERY );
    if( outputStream.is() )
    {
        m_pMyShell->page( nMyCommandIdentifier,
                          m_aUncPath,
                          outputStream );
    }

    uno::Reference< io::XActiveDataSink > activeDataSink( aCommandArgument.Sink,uno::UNO_QUERY );
    if( activeDataSink.is() )
    {
        activeDataSink->setInputStream( m_pMyShell->open( nMyCommandIdentifier,
                                                          m_aUncPath ) );
    }


    uno::Reference< io::XActiveDataStreamer > activeDataStreamer( aCommandArgument.Sink,uno::UNO_QUERY );
    if( activeDataStreamer.is() )
    {
        activeDataStreamer->setStream( m_pMyShell->open_rw( nMyCommandIdentifier,
                                                            m_aUncPath ) );
    }

    return m_pMyShell->ls( nMyCommandIdentifier,
                           m_aUncPath,
                           aCommandArgument.Mode,
                           aCommandArgument.Properties,
                           aCommandArgument.SortingInfo );
}


void SAL_CALL
BaseContent::deleteContent( sal_Int32 nMyCommandIdentifier,
                            sal_Bool bDeleteArgument )
{
    if( m_nState & Deleted )
        return;
    m_pMyShell->remove( nMyCommandIdentifier,
                        m_aUncPath );

    vos::OGuard aGuard( m_aMutex );
    m_nState |= Deleted;
}



void SAL_CALL
BaseContent::transfer( sal_Int32 nMyCommandIdentifier,
                       const TransferInfo& aTransferInfo )
    throw( CommandAbortedException,InteractiveBadTransferURLException )
{

    if( m_nState & Deleted )
        return;

    // No write access to route
    if( m_pMyShell->m_bFaked && m_aUncPath.compareToAscii( "//./" ) == 0 )
        throw CommandAbortedException();


    rtl::OUString scheme = aTransferInfo.SourceURL.copy( 0,5 );
    if( scheme.compareToAscii( "file:" ) != 0 )
        throw InteractiveBadTransferURLException();


    sal_Unicode slash = '/';
    rtl::OUString srcUnc;
    sal_Bool err = m_pMyShell->getUnqFromUrl( aTransferInfo.SourceURL,srcUnc );
    if( err )
        throw CommandAbortedException();

    rtl::OUString srcUncPath;
    sal_Bool mounted = m_pMyShell->checkMountPoint( srcUnc,srcUncPath );

    if( ! mounted )
        throw CommandAbortedException();

    rtl::OUString NewTitle;
    if( aTransferInfo.NewTitle.getLength() )
        NewTitle = aTransferInfo.NewTitle;
    else
    {
        sal_Int32 lastSlash = srcUncPath.lastIndexOf( slash );
        NewTitle = srcUncPath.copy( lastSlash+1 );
    }


    // Who am I ?
    uno::Sequence< beans::Property > seq(1);
    seq[0] = beans::Property( rtl::OUString::createFromAscii("IsDocument"),
                              -1,
                              getCppuType( static_cast< sal_Bool* >(0) ),
                              0 );
    uno::Reference< sdbc::XRow > xRow = getPropertyValues( nMyCommandIdentifier,seq );
    sal_Bool IsDocument = xRow->getBoolean( 1 );
    if( xRow->wasNull() )
        throw CommandAbortedException();


    rtl::OUString dstUncPath;

    if( IsDocument ) {
        sal_Int32 lastSlash = m_aUncPath.lastIndexOf( slash );
        dstUncPath = m_aUncPath.copy(0,lastSlash );
    }
    else
        dstUncPath = m_aUncPath;

    dstUncPath += ( rtl::OUString::createFromAscii( "/" ) + NewTitle );

    sal_Int32 NameClash = aTransferInfo.NameClash;

    if( aTransferInfo.MoveData )
        m_pMyShell->move( nMyCommandIdentifier,srcUncPath,dstUncPath,NameClash );
    else
        m_pMyShell->copy( nMyCommandIdentifier,srcUncPath,dstUncPath,NameClash );
}


void SAL_CALL
BaseContent::write( sal_Int32 nMyCommandIdentifier,
                    sal_Bool OverWrite,
                    const uno::Reference< io::XInputStream >& aInputStream )
    throw( CommandAbortedException )
{
    sal_Bool err = ! m_pMyShell->write( nMyCommandIdentifier,
                                        m_aUncPath,
                                        OverWrite,
                                        aInputStream );
    if( err )
        throw CommandAbortedException() ;
}




void SAL_CALL BaseContent::insert( sal_Int32 nMyCommandIdentifier,
                                   const InsertCommandArgument& aInsertArgument )
    throw( CommandAbortedException )
{
    if( m_nState & FullFeatured )
    {
        write( nMyCommandIdentifier,
               aInsertArgument.ReplaceExisting,
               aInsertArgument.Data );
        return;
    }

    if( ! ( m_nState & JustInserted ) )
        return;

    // Inserts the content, which has the flag m_bIsFresh

    sal_Bool success = ( m_nState & NameForInsertionSet );
    if( success )
    {
        // Who am I ?
        sal_Bool bDocument = false;

        try
        {
            uno::Sequence< beans::Property > seq(1);
            seq[0] = beans::Property( rtl::OUString::createFromAscii("IsDocument"),
                                        -1,
                                        getCppuType( static_cast< sal_Bool* >(0) ),
                                        0 );
            uno::Reference< sdbc::XRow > xRow = getPropertyValues( -1,seq );
            bDocument = xRow->getBoolean( 1 );

            success = !xRow->wasNull();

            VOS_ENSURE( success,
                        "BaseContent::insert - Property value was null!" );
        }
        catch ( sdbc::SQLException const & )
        {
            VOS_ENSURE( false,
                        "BaseContent::insert - Caught SQLException!" );
            success = false;
        }

        if ( success )
        {
            if( bDocument )
            {
                success = m_pMyShell->mkfil( nMyCommandIdentifier,
                                             m_aUncPath,
                                             aInsertArgument.ReplaceExisting,
                                             aInsertArgument.Data );
            }
            else
            {
                success = m_pMyShell->mkdir( nMyCommandIdentifier,
                                             m_aUncPath );
            }
        }
    }

    if( ! success )
    {
        throw CommandAbortedException();
    }

    FileContentIdentifier* p = new FileContentIdentifier( m_pMyShell,m_aUncPath );
    m_xContentIdentifier = uno::Reference< XContentIdentifier >( p );

    m_pMyShell->registerNotifier( m_aUncPath,this );
    m_pMyShell->InsertDefaultProperties( m_aUncPath );

    vos::OGuard aGuard( m_aMutex );
    m_nState = FullFeatured;
}



ContentEventNotifier*
BaseContent::cDEL( void )
{
    vos::OGuard aGuard( m_aMutex );

    m_nState |= Deleted;

    ContentEventNotifier* p;
    if( m_pContentEventListeners )
        p = new ContentEventNotifier( m_pMyShell,
                                      this,
                                      m_xContentIdentifier,
                                      m_pContentEventListeners->getElements() );
    else
        p = 0;

    return p;
}


ContentEventNotifier*
BaseContent::cEXC( const rtl::OUString aNewName )
{
    vos::OGuard aGuard( m_aMutex );

    uno::Reference< XContentIdentifier > xOldRef = m_xContentIdentifier;
    m_aUncPath = aNewName;
    FileContentIdentifier* pp = new FileContentIdentifier( m_pMyShell,aNewName );
    m_xContentIdentifier = uno::Reference< XContentIdentifier >( pp );

    ContentEventNotifier* p = 0;
    if( m_pContentEventListeners )
        p = new ContentEventNotifier( m_pMyShell,
                                      this,
                                      m_xContentIdentifier,
                                      xOldRef,
                                      m_pContentEventListeners->getElements() );

    return p;
}


ContentEventNotifier*
BaseContent::cCEL( void )
{
    vos::OGuard aGuard( m_aMutex );
    ContentEventNotifier* p = 0;
    if( m_pContentEventListeners )
        p = new ContentEventNotifier( m_pMyShell,
                                      this,
                                      m_xContentIdentifier,
                                      m_pContentEventListeners->getElements() );

    return p;
}

PropertySetInfoChangeNotifier*
BaseContent::cPSL( void )
{
    vos::OGuard aGuard( m_aMutex );
    PropertySetInfoChangeNotifier* p = 0;
    if( m_pPropertySetInfoChangeListeners  )
        p = new PropertySetInfoChangeNotifier( m_pMyShell,
                                               this,
                                               m_xContentIdentifier,
                                               m_pPropertySetInfoChangeListeners->getElements() );

    return p;
}



PropertyChangeNotifier*
BaseContent::cPCL( void )
{
    vos::OGuard aGuard( m_aMutex );

    uno::Sequence< rtl::OUString > seqNames;

    if( m_pPropertyListener )
        seqNames = m_pPropertyListener->getContainedTypes();

    PropertyChangeNotifier* p = 0;

    sal_Int32 length = seqNames.getLength();

    if( length )
    {
        ListenerMap* listener = new ListenerMap();
        for( sal_Int32 i = 0; i < length; ++i )
        {
            (*listener)[seqNames[i]] = m_pPropertyListener->getContainer( seqNames[i] )->getElements();
        }

        p = new PropertyChangeNotifier( m_pMyShell,
                                        this,
                                        m_xContentIdentifier,
                                        listener );
    }

    return p;
}


rtl::OUString BaseContent::getKey( void )
{
    return m_aUncPath;
}
