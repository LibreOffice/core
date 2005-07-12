/*************************************************************************
 *
 *  $RCSfile: olepersist.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-12 12:19:36 $
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

#include <oleembobj.hxx>

#ifndef _COM_SUN_STAR_EMBED_EMBEDSTATES_HPP_
#include <com/sun/star/embed/EmbedStates.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_EMBEDVERBS_HPP_
#include <com/sun/star/embed/EmbedVerbs.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ENTRYINITMODES_HPP_
#include <com/sun/star/embed/EntryInitModes.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XTRANSACTEDOBJECT_HPP_
#include <com/sun/star/embed/XTransactedObject.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_EMBEDUPDATEMODES_HPP_
#include <com/sun/star/embed/EmbedUpdateModes.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ASPECTS_HPP_
#include <com/sun/star/embed/Aspects.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XTRUNCATE_HPP_
#include <com/sun/star/io/XTruncate.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_XSIMPLEFILEACCESS_HPP_
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#endif


#include <olecomponent.hxx>
#include <closepreventer.hxx>


using namespace ::com::sun::star;

//------------------------------------------------------
// TODO: probably later those common functions should be moved
//       to a separate helper library.

//-------------------------------------------------------------------------
sal_Bool ClassIDsEqual( const uno::Sequence< sal_Int8 >& aClassID1, const uno::Sequence< sal_Int8 >& aClassID2 )
{
    // TODO/LATER: move this method and other methods like this to a standalone library
    if ( aClassID1.getLength() != aClassID2.getLength() )
        return sal_False;

    for ( sal_Int32 nInd = 0; nInd < aClassID1.getLength(); nInd++ )
        if ( aClassID1[nInd] != aClassID2[nInd] )
            return sal_False;

    return sal_True;
}

//-------------------------------------------------------------------------
const sal_Int32 n_ConstBufferSize = 32000;
void copyInputToOutput_Impl( const uno::Reference< io::XInputStream >& aIn,
                             const uno::Reference< io::XOutputStream >& aOut )
{
    sal_Int32 nRead;
    uno::Sequence < sal_Int8 > aSequence ( n_ConstBufferSize );

    do
    {
        nRead = aIn->readBytes ( aSequence, n_ConstBufferSize );
        if ( nRead < n_ConstBufferSize )
        {
            uno::Sequence < sal_Int8 > aTempBuf ( aSequence.getConstArray(), nRead );
            aOut->writeBytes ( aTempBuf );
        }
        else
            aOut->writeBytes ( aSequence );
    }
    while ( nRead == n_ConstBufferSize );
}

//-----------------------------------------------
sal_Bool KillFile_Impl( const ::rtl::OUString& aURL, const uno::Reference< lang::XMultiServiceFactory >& xFactory )
{
    if ( !xFactory.is() )
        return sal_False;

    sal_Bool bRet = sal_False;

    try
    {
        uno::Reference < ucb::XSimpleFileAccess > xAccess(
                xFactory->createInstance (
                        ::rtl::OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" ) ),
                uno::UNO_QUERY );

        if ( xAccess.is() )
        {
            xAccess->kill( aURL );
            bRet = sal_True;
        }
    }
    catch( uno::Exception& )
    {
    }

    return bRet;
}

//----------------------------------------------
::rtl::OUString GetNewTempFileURL_Impl( const uno::Reference< lang::XMultiServiceFactory >& xFactory )
{
    OSL_ENSURE( xFactory.is(), "No factory is provided!\n" );

    ::rtl::OUString aResult;

    uno::Reference < beans::XPropertySet > xTempFile(
            xFactory->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.io.TempFile" ) ),
            uno::UNO_QUERY );

    if ( !xTempFile.is() )
        throw uno::RuntimeException(); // TODO

    try {
        xTempFile->setPropertyValue( ::rtl::OUString::createFromAscii( "RemoveFile" ), uno::makeAny( sal_False ) );
        uno::Any aUrl = xTempFile->getPropertyValue( ::rtl::OUString::createFromAscii( "Uri" ) );
        aUrl >>= aResult;
    }
    catch ( uno::Exception& )
    {
    }

    if ( !aResult.getLength() )
        throw uno::RuntimeException(); // TODO: can not create tempfile

    return aResult;
}

//-----------------------------------------------
::rtl::OUString GetNewFilledTempFile_Impl( const uno::Reference< io::XInputStream >& xInStream,
                                      const uno::Reference< lang::XMultiServiceFactory >& xFactory )
        throw( io::IOException )
{
    OSL_ENSURE( xInStream.is() && xFactory.is(), "Wrong parameters are provided!\n" );

    ::rtl::OUString aResult = GetNewTempFileURL_Impl( xFactory );

    if ( aResult )
    {
        try {
            uno::Reference < ucb::XSimpleFileAccess > xTempAccess(
                            xFactory->createInstance (
                                    ::rtl::OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" ) ),
                            uno::UNO_QUERY );

            if ( !xTempAccess.is() )
                throw uno::RuntimeException(); // TODO:

            uno::Reference< io::XOutputStream > xTempOutStream = xTempAccess->openFileWrite( aResult );
            if ( xTempOutStream.is() )
            {
                // copy stream contents to the file
                copyInputToOutput_Impl( xInStream, xTempOutStream );
                xTempOutStream->closeOutput();
                xTempOutStream = uno::Reference< io::XOutputStream >();
            }
            else
                throw io::IOException(); // TODO:
        }
        catch( packages::WrongPasswordException& )
        {
               KillFile_Impl( aResult, xFactory );
            throw io::IOException(); //TODO:
        }
        catch( io::IOException& )
        {
               KillFile_Impl( aResult, xFactory );
            throw;
        }
        catch( uno::RuntimeException& )
        {
               KillFile_Impl( aResult, xFactory );
            throw;
        }
        catch( uno::Exception& )
        {
               KillFile_Impl( aResult, xFactory );
            aResult = ::rtl::OUString();
        }
    }

    return aResult;
}

//------------------------------------------------------
void SetStreamMediaType_Impl( const uno::Reference< io::XStream >& xStream, const ::rtl::OUString& aMediaType )
{
    uno::Reference< beans::XPropertySet > xPropSet( xStream, uno::UNO_QUERY );
    if ( !xPropSet.is() )
        throw uno::RuntimeException(); // TODO: all the storage streams must support XPropertySet

    xPropSet->setPropertyValue( ::rtl::OUString::createFromAscii( "MediaType" ), uno::makeAny( aMediaType ) );
}

//------------------------------------------------------
void LetCommonStoragePassBeUsed_Impl( const uno::Reference< io::XStream >& xStream )
{
    uno::Reference< beans::XPropertySet > xPropSet( xStream, uno::UNO_QUERY );
    if ( !xPropSet.is() )
        throw uno::RuntimeException(); // Only StorageStreams must be provided here, they must implement the interface

    xPropSet->setPropertyValue( ::rtl::OUString::createFromAscii( "UseCommonStoragePasswordEncryption" ),
                                uno::makeAny( (sal_Bool)sal_True ) );
}

//------------------------------------------------------
uno::Reference< io::XStream > TryToGetAcceptableFormat_Impl( const uno::Reference< io::XStream >& xStream,
                                                             const uno::Reference< lang::XMultiServiceFactory >& xFactory )
        throw ( uno::Exception )
{
    // TODO/LATER: Actually this should be done by a centralized component ( may be a graphical filter )
    if ( !xFactory.is() )
        throw uno::RuntimeException();

    uno::Reference< io::XInputStream > xInStream = xStream->getInputStream();
    if ( !xInStream.is() )
        throw uno::RuntimeException();

    uno::Reference< io::XSeekable > xSeek( xStream, uno::UNO_QUERY_THROW );
    xSeek->seek( 0 );

    uno::Sequence< sal_Int8 > aData( 8 );
    sal_Int32 nRead = xInStream->readBytes( aData, 8 );
    xSeek->seek( 0 );

    if ( ( nRead >= 2 && aData[0] == 'B' && aData[1] == 'M' )
      || ( nRead >= 4 && aData[0] == 1 && aData[1] == 0 && aData[2] == 9 && aData[3] == 0 ) )
    {
        // it should be a bitmap or a Metafile
        return xStream;
    }

    sal_uInt32 nHeaderOffset = 0;
    if ( ( nRead >= 8 && aData[0] == -1 && aData[1] == -1 && aData[2] == -1 && aData[3] == -1 )
      && ( aData[4] == 2 || aData[4] == 3 ) && aData[5] == 0 && aData[6] == 0 && aData[7] == 0 )
    {
        nHeaderOffset = 40;
    }
    else if ( nRead > 4 )
    {
        // check whether the first bytes represent the size
        sal_uInt32 nSize = 0;
        for ( sal_Int32 nInd = 3; nInd >= 0; nInd-- )
            nSize = ( nSize << 8 ) + (sal_uInt8)aData[nInd];

        if ( nSize == xSeek->getLength() - 4 )
            nHeaderOffset = 4;
    }

    if ( nHeaderOffset )
    {
        // this is either a bitmap or a metafile clipboard format, retrieve the pure stream
        uno::Reference < io::XStream > xResult(
            xFactory->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.io.TempFile" ) ),
            uno::UNO_QUERY_THROW );
        uno::Reference < io::XSeekable > xResultSeek( xResult, uno::UNO_QUERY_THROW );
        uno::Reference < io::XOutputStream > xResultOut = xResult->getOutputStream();
        uno::Reference < io::XInputStream > xResultIn = xResult->getInputStream();
        if ( !xResultOut.is() || !xResultIn.is() )
            throw uno::RuntimeException();

        xSeek->seek( nHeaderOffset ); // header size for these formats
        copyInputToOutput_Impl( xInStream, xResultOut );
        xResultOut->closeOutput();
        xResultSeek->seek( 0 );
        xSeek->seek( 0 );

        return xResult;
    }

    return uno::Reference< io::XStream >();
}

//------------------------------------------------------
void OleEmbeddedObject::InsertVisualCache_Impl( const uno::Reference< io::XStream >& xTargetStream,
                                                const uno::Reference< io::XStream >& xCachedVisualRepresentation )
        throw ( uno::Exception )
{
    OSL_ENSURE( xTargetStream.is() && xCachedVisualRepresentation.is(), "Invalid argumants!\n" );
    if ( !xTargetStream.is() || !xCachedVisualRepresentation.is() )
        throw uno::RuntimeException();

    uno::Sequence< uno::Any > aArgs( 1 );
    aArgs[0] <<= xTargetStream;
    uno::Reference< container::XNameContainer > xNameContainer(
            m_xFactory->createInstanceWithArguments(
                    ::rtl::OUString::createFromAscii( "com.sun.star.embed.OLESimpleStorage" ),
                    aArgs ),
            uno::UNO_QUERY );

    if ( !xNameContainer.is() )
        throw uno::RuntimeException();

    ::rtl::OUString aCacheName = ::rtl::OUString::createFromAscii( "\002OlePres000" );
    if ( xNameContainer->hasByName( aCacheName ) )
        xNameContainer->replaceByName( aCacheName, uno::makeAny( xCachedVisualRepresentation ) );
    else
        xNameContainer->insertByName( aCacheName, uno::makeAny( xCachedVisualRepresentation ) );

    uno::Reference< embed::XTransactedObject > xTransacted( xNameContainer, uno::UNO_QUERY );
    if ( !xTransacted.is() )
        throw uno::RuntimeException();

    xTransacted->commit();
}

//------------------------------------------------------
void OleEmbeddedObject::RemoveVisualCache_Impl( const uno::Reference< io::XStream >& xTargetStream )
        throw ( uno::Exception )
{
    OSL_ENSURE( xTargetStream.is(), "Invalid argumant!\n" );
    if ( !xTargetStream.is() )
        throw uno::RuntimeException();

    uno::Sequence< uno::Any > aArgs( 1 );
    aArgs[0] <<= xTargetStream;
    uno::Reference< container::XNameContainer > xNameContainer(
            m_xFactory->createInstanceWithArguments(
                    ::rtl::OUString::createFromAscii( "com.sun.star.embed.OLESimpleStorage" ),
                    aArgs ),
            uno::UNO_QUERY );

    if ( !xNameContainer.is() )
        throw uno::RuntimeException();

    for ( sal_uInt8 nInd = 0; nInd < 10; nInd++ )
    {
        ::rtl::OUString aStreamName = ::rtl::OUString::createFromAscii( "\002OlePres00" );
        aStreamName += ::rtl::OUString::valueOf( (sal_Int32)nInd );
        if ( xNameContainer->hasByName( aStreamName ) )
            xNameContainer->removeByName( aStreamName );
    }

    uno::Reference< embed::XTransactedObject > xTransacted( xNameContainer, uno::UNO_QUERY );
    if ( !xTransacted.is() )
        throw uno::RuntimeException();

    xTransacted->commit();
}

//------------------------------------------------------
uno::Reference< io::XStream > OleEmbeddedObject::TryToRetrieveCachedVisualRepresentation_Impl(
        const uno::Reference< io::XStream >& xStream )
    throw ()
{
    uno::Reference< io::XStream > xResult;

    if ( xStream.is() )
    {
        uno::Sequence< uno::Any > aArgs( 1 );
        aArgs[0] <<= xStream;
        uno::Reference< container::XNameContainer > xNameContainer(
                m_xFactory->createInstanceWithArguments(
                        ::rtl::OUString::createFromAscii( "com.sun.star.embed.OLESimpleStorage" ),
                        aArgs ),
                uno::UNO_QUERY );

        if ( xNameContainer.is() )
        {
            for ( sal_uInt8 nInd = 0; nInd < 10; nInd++ )
            {
                ::rtl::OUString aStreamName = ::rtl::OUString::createFromAscii( "\002OlePres00" );
                aStreamName += ::rtl::OUString::valueOf( (sal_Int32)nInd );
                uno::Reference< io::XStream > xCachedCopyStream;
                try
                {
                    if ( ( xNameContainer->getByName( aStreamName ) >>= xCachedCopyStream ) && xCachedCopyStream.is() )
                    {
                        xResult = TryToGetAcceptableFormat_Impl( xCachedCopyStream, m_xFactory );
                        if ( xResult.is() )
                            break;
                    }
                }
                catch( uno::Exception& )
                {}

                if ( nInd == 0 )
                {
                    // to be compatible with the old versions Ole10Native is checked after OlePress000
                    aStreamName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "\001Ole10Native" ) );
                    try
                    {
                        if ( ( xNameContainer->getByName( aStreamName ) >>= xCachedCopyStream ) && xCachedCopyStream.is() )
                        {
                            xResult = TryToGetAcceptableFormat_Impl( xCachedCopyStream, m_xFactory );
                            if ( xResult.is() )
                                break;
                        }
                    }
                    catch( uno::Exception& )
                    {}
                }
            }
        }
    }

    return xResult;
}

//------------------------------------------------------
void OleEmbeddedObject::SwitchOwnPersistence( const uno::Reference< embed::XStorage >& xNewParentStorage,
                                              const uno::Reference< io::XStream >& xNewObjectStream,
                                              const ::rtl::OUString& aNewName )
{
    if ( xNewParentStorage == m_xParentStorage && aNewName.equals( m_aEntryName ) )
    {
        OSL_ENSURE( xNewObjectStream == m_xObjectStream, "The streams must be the same!\n" );
        return;
    }

    try {
        uno::Reference< lang::XComponent > xComponent( m_xObjectStream, uno::UNO_QUERY );
        OSL_ENSURE( !m_xObjectStream.is() || xComponent.is(), "Wrong stream implementation!" );
        if ( xComponent.is() )
            xComponent->dispose();
    }
    catch ( uno::Exception& )
    {
    }

    m_xObjectStream = xNewObjectStream;
    m_xParentStorage = xNewParentStorage;
    m_aEntryName = aNewName;
}

//------------------------------------------------------
void OleEmbeddedObject::SwitchOwnPersistence( const uno::Reference< embed::XStorage >& xNewParentStorage,
                                              const ::rtl::OUString& aNewName )
{
    if ( xNewParentStorage == m_xParentStorage && aNewName.equals( m_aEntryName ) )
        return;

    sal_Int32 nStreamMode = m_bReadOnly ? embed::ElementModes::READ : embed::ElementModes::READWRITE;

    uno::Reference< io::XStream > xNewOwnStream = xNewParentStorage->openStreamElement( aNewName, nStreamMode );
    OSL_ENSURE( xNewOwnStream.is(), "The method can not return empty reference!" );

    SwitchOwnPersistence( xNewParentStorage, xNewOwnStream, aNewName );
}


//----------------------------------------------
sal_Bool OleEmbeddedObject::SaveObject_Impl()
{
    sal_Bool bResult = sal_False;

    if ( m_xClientSite.is() )
    {
        try
        {
            m_xClientSite->saveObject();
            bResult = sal_True;
        }
        catch( uno::Exception& )
        {
        }
    }

    return bResult;
}

//----------------------------------------------
sal_Bool OleEmbeddedObject::OnShowWindow_Impl( sal_Bool bShow )
{
    ::osl::ResettableMutexGuard aGuard( m_aMutex );

    sal_Bool bResult = sal_False;

    OSL_ENSURE( m_nObjectState != -1, "The object has no persistence!\n" );
    OSL_ENSURE( m_nObjectState != embed::EmbedStates::LOADED, "The object get OnShowWindow in loaded state!\n" );
    if ( m_nObjectState == -1 || m_nObjectState == embed::EmbedStates::LOADED )
        return sal_False;

    // the object is either activated or deactivated
    sal_Int32 nOldState = m_nObjectState;
    if ( bShow && m_nObjectState == embed::EmbedStates::RUNNING )
    {
        m_nObjectState = embed::EmbedStates::ACTIVE;
        aGuard.clear();
        StateChangeNotification_Impl( sal_False, nOldState, m_nObjectState );
    }
    else if ( !bShow && m_nObjectState == embed::EmbedStates::ACTIVE )
    {
        m_nObjectState = embed::EmbedStates::RUNNING;
        aGuard.clear();
        StateChangeNotification_Impl( sal_False, nOldState, m_nObjectState );
    }

    if ( m_xClientSite.is() )
    {
        try
        {
            m_xClientSite->visibilityChanged( bShow );
            bResult = sal_True;
        }
        catch( uno::Exception& )
        {
        }
    }

    return bResult;
}

void OleEmbeddedObject::OnViewChanged_Impl()
{
    if ( m_pOleComponent && m_nUpdateMode == embed::EmbedUpdateModes::ALWAYS_UPDATE )
    {
        MakeEventListenerNotification_Impl( ::rtl::OUString::createFromAscii( "OnVisAreaChanged" ) );
        MakeEventListenerNotification_Impl( ::rtl::OUString::createFromAscii( "OnVisAreaChanged" ) );
    }
}

//------------------------------------------------------
void OleEmbeddedObject::CreateOleComponent_Impl( OleComponent* pOleComponent )
{
#ifdef WNT
    if ( !m_pOleComponent )
    {
        m_pOleComponent = pOleComponent ? pOleComponent : new OleComponent( m_xFactory, this );
        m_pOleComponent->acquire(); // TODO: needs holder?

        if ( !m_xClosePreventer.is() )
            m_xClosePreventer = uno::Reference< util::XCloseListener >(
                                    static_cast< ::cppu::OWeakObject* >( new OClosePreventer ),
                                    uno::UNO_QUERY );

        m_pOleComponent->addCloseListener( m_xClosePreventer );
    }
    else
        OSL_ENSURE( sal_False, "Trying to recreate OLE component!\n" );
#endif
}

//------------------------------------------------------
void OleEmbeddedObject::CreateOleComponentAndLoad_Impl( OleComponent* pOleComponent )
{
#ifdef WNT
    if ( !m_pOleComponent )
    {
        if ( !m_xObjectStream.is() )
            throw uno::RuntimeException();

        CreateOleComponent_Impl( pOleComponent );
        // load object from the stream
        uno::Reference< io::XInputStream > xInStream = m_xObjectStream->getInputStream();
        if ( !xInStream.is() )
            throw io::IOException(); // TODO: access denied

        // after the loading the object can appear as a link
        // will be detected later by olecomponent
        m_pOleComponent->LoadEmbeddedObject( xInStream );
    }
#endif
}

//------------------------------------------------------
void OleEmbeddedObject::CreateOleComponentFromClipboard_Impl( OleComponent* pOleComponent )
{
#ifdef WNT
    if ( !m_pOleComponent )
    {
        if ( !m_xObjectStream.is() )
            throw uno::RuntimeException();

        CreateOleComponent_Impl( pOleComponent );

        // after the loading the object can appear as a link
        // will be detected later by olecomponent
        m_pOleComponent->CreateObjectFromClipboard();
    }
#endif
}

//------------------------------------------------------
uno::Reference< io::XOutputStream > OleEmbeddedObject::GetStreamForSaving()
{
    if ( !m_xObjectStream.is() )
        throw uno::RuntimeException(); //TODO:

    uno::Reference< io::XOutputStream > xOutStream = m_xObjectStream->getOutputStream();
    if ( !xOutStream.is() )
        throw io::IOException(); //TODO: access denied

    uno::Reference< io::XTruncate > xTruncate( xOutStream, uno::UNO_QUERY );
    if ( !xTruncate.is() )
        throw uno::RuntimeException(); //TODO:

    xTruncate->truncate();

    return xOutStream;
}

//------------------------------------------------------
void OleEmbeddedObject::StoreToLocation_Impl(
                            const uno::Reference< embed::XStorage >& xStorage,
                            const ::rtl::OUString& sEntName,
                            const uno::Sequence< beans::PropertyValue >& lArguments,
                            const uno::Sequence< beans::PropertyValue >& lObjArgs,
                            sal_Bool bSaveAs )
        throw ( uno::Exception )
{
    // TODO: use lObjArgs
    // TODO: exchange StoreVisualReplacement by SO file format version?

    if ( m_nObjectState == -1 )
    {
        // the object is still not loaded
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "Can't store object without persistence!\n" ),
                                        uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );
    }

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    ::rtl::OUString::createFromAscii( "The object waits for saveCompleted() call!\n" ),
                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    OSL_ENSURE( m_xParentStorage.is() && m_xObjectStream.is(), "The object has no valid persistence!\n" );

    sal_Bool bVisReplIsStored = sal_False;

    sal_Bool bStoreVis = m_bStoreVisRepl;
    uno::Reference< io::XStream > xCachedVisualRepresentation;
    for ( sal_Int32 nInd = 0; nInd < lObjArgs.getLength(); nInd++ )
    {
        if ( lObjArgs[nInd].Name.equalsAscii( "StoreVisualReplacement" ) )
            lObjArgs[nInd].Value >>= bStoreVis;
        if ( lObjArgs[nInd].Name.equalsAscii( "VisualReplacement" ) )
            lObjArgs[nInd].Value >>= xCachedVisualRepresentation;
    }

    // ignore visual representation provided from outside if it should not be stored
    if ( !bStoreVis )
        xCachedVisualRepresentation = uno::Reference< io::XStream >();

    if ( bStoreVis && !m_bVisReplInStream && !xCachedVisualRepresentation.is() )
        throw io::IOException(); // TODO: there is no cached visual representation and nothing is provided from outside

    uno::Reference< io::XStream > xTargetStream;

    sal_Bool bStoreLoaded = sal_False;
    if ( m_nObjectState == embed::EmbedStates::LOADED )
    {
        m_xParentStorage->copyElementTo( m_aEntryName, xStorage, sEntName );
        bVisReplIsStored = m_bVisReplInStream;
        bStoreLoaded = sal_True;
    }
#ifdef WNT
    else if ( m_pOleComponent )
    {
        xTargetStream =
                xStorage->openStreamElement( sEntName, embed::ElementModes::READWRITE );
        if ( !xTargetStream.is() )
            throw io::IOException(); //TODO: access denied

        SetStreamMediaType_Impl( xTargetStream, ::rtl::OUString::createFromAscii( "application/vnd.sun.star.oleobject" ) );
        uno::Reference< io::XOutputStream > xOutStream = xTargetStream->getOutputStream();
        if ( !xOutStream.is() )
            throw io::IOException(); //TODO: access denied

        m_pOleComponent->StoreObjectToStream( xOutStream );
        bVisReplIsStored = sal_True;

        if ( !xCachedVisualRepresentation.is() )
            xCachedVisualRepresentation = TryToRetrieveCachedVisualRepresentation_Impl( xTargetStream );
    }
#endif
    else
    {
        throw io::IOException(); // TODO
    }

    if ( !xTargetStream.is() )
    {
        xTargetStream =
            xStorage->openStreamElement( sEntName, embed::ElementModes::READWRITE );
        if ( !xTargetStream.is() )
            throw io::IOException(); //TODO: access denied
    }

    LetCommonStoragePassBeUsed_Impl( xTargetStream );

    if ( bStoreVis != bVisReplIsStored )
    {
        if ( bStoreVis )
            InsertVisualCache_Impl( xTargetStream, xCachedVisualRepresentation );
        else
        {
            // the removed representation could be cached by this method
            RemoveVisualCache_Impl( xTargetStream );
        }
    }

    if ( bSaveAs )
    {
        m_bWaitSaveCompleted = sal_True;
        m_xNewObjectStream = xTargetStream;
        m_xNewParentStorage = xStorage;
        m_aNewEntryName = sEntName;
        m_bNewVisReplInStream = bStoreVis;
        m_bStoreLoaded = bStoreLoaded;

        if ( xCachedVisualRepresentation.is() )
            m_xNewCachedVisRepl = xCachedVisualRepresentation;

        // TODO: register listeners for storages above, in case they are disposed
        //       an exception will be thrown on saveCompleted( true )
    }
    else
    {
        uno::Reference< lang::XComponent > xComp( xTargetStream, uno::UNO_QUERY );
        if ( xComp.is() )
        {
            try {
                xComp->dispose();
            } catch( uno::Exception& )
            {
            }
        }
    }
}

//------------------------------------------------------
void SAL_CALL OleEmbeddedObject::setPersistentEntry(
                    const uno::Reference< embed::XStorage >& xStorage,
                    const ::rtl::OUString& sEntName,
                    sal_Int32 nEntryConnectionMode,
                    const uno::Sequence< beans::PropertyValue >& lArguments,
                    const uno::Sequence< beans::PropertyValue >& lObjArgs )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
    // TODO: use lObjArgs

    // the type of the object must be already set
    // a kind of typedetection should be done in the factory;
    // the only exception is object initialized from a stream,
    // the class ID will be detected from the stream

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "No parent storage is provided!\n" ),
                                            uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                                            1 );

    if ( !sEntName.getLength() )
        throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "Empty element name is provided!\n" ),
                                            uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                                            2 );

    // May be LOADED should be forbidden here ???
    if ( ( m_nObjectState != -1 || nEntryConnectionMode == embed::EntryInitModes::NO_INIT )
      && ( m_nObjectState == -1 || nEntryConnectionMode != embed::EntryInitModes::NO_INIT ) )
    {
        // if the object is not loaded
        // it can not get persistant representation without initialization

        // if the object is loaded
        // it can switch persistant representation only without initialization

        throw embed::WrongStateException(
                    ::rtl::OUString::createFromAscii( "Can't change persistant representation of activated object!\n" ),
                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );
    }

    if ( m_bWaitSaveCompleted )
    {
        if ( nEntryConnectionMode == embed::EntryInitModes::NO_INIT )
            saveCompleted( ( m_xParentStorage != xStorage || !m_aEntryName.equals( sEntName ) ) );
        else
            throw embed::WrongStateException(
                        ::rtl::OUString::createFromAscii( "The object waits for saveCompleted() call!\n" ),
                        uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );
    }

    uno::Reference< container::XNameAccess > xNameAccess( xStorage, uno::UNO_QUERY );
    if ( !xNameAccess.is() )
        throw uno::RuntimeException(); //TODO

    // detect entry existence
    sal_Bool bElExists = xNameAccess->hasByName( sEntName );

    m_bReadOnly = sal_False;
    sal_Int32 nInd = 0;
    for ( nInd = 0; nInd < lArguments.getLength(); nInd++ )
        if ( lArguments[nInd].Name.equalsAscii( "ReadOnly" ) )
            lArguments[nInd].Value >>= m_bReadOnly;

    sal_Int32 nStorageMode = m_bReadOnly ? embed::ElementModes::READ : embed::ElementModes::READWRITE;

    SwitchOwnPersistence( xStorage, sEntName );

    for ( nInd = 0; nInd < lObjArgs.getLength(); nInd++ )
        if ( lObjArgs[nInd].Name.equalsAscii( "StoreVisualReplacement" ) )
            lObjArgs[nInd].Value >>= m_bStoreVisRepl;

#ifdef WNT
    if ( nEntryConnectionMode == embed::EntryInitModes::DEFAULT_INIT )
    {
        if ( m_bFromClipboard )
        {
            // the object should be initialized from clipboard
            // inpossibility to initialize the object means error here
            CreateOleComponentFromClipboard_Impl( NULL );
            m_aClassID = m_pOleComponent->GetCLSID(); // was not set during consruction
            m_pOleComponent->RunObject();
            m_nObjectState = embed::EmbedStates::RUNNING;
        }
        else if ( bElExists )
        {
            // load object from the stream
            // after the loading the object can appear as a link
            // will be detected by olecomponent
            try
            {
                CreateOleComponentAndLoad_Impl( NULL );
                m_aClassID = m_pOleComponent->GetCLSID(); // was not set during consruction
            }
            catch( uno::Exception& )
            {
                // TODO/LATER: detect classID of the object if possible
                // means that the object inprocess server could not be successfuly instantiated
                GetRidOfComponent();
            }

            m_nObjectState = embed::EmbedStates::LOADED;
        }
        else
        {
            // create a new object
            CreateOleComponent_Impl();
            m_pOleComponent->CreateNewEmbeddedObject( m_aClassID );
            m_pOleComponent->RunObject();
            m_nObjectState = embed::EmbedStates::RUNNING;
        }
    }
    else
    {
        if ( ( nStorageMode & embed::ElementModes::READWRITE ) != embed::ElementModes::READWRITE )
            throw io::IOException();

        if ( nEntryConnectionMode == embed::EntryInitModes::NO_INIT )
        {
            // the document just already changed its stream to store to;
            // the links to OLE documents switch their persistence in the same way
            // as normal embedded objects
        }
        else if ( nEntryConnectionMode == embed::EntryInitModes::TRUNCATE_INIT )
        {
            // create a new object, that will be stored in specified stream
            CreateOleComponent_Impl();

            m_pOleComponent->CreateNewEmbeddedObject( m_aClassID );
            m_pOleComponent->RunObject();
            m_nObjectState = embed::EmbedStates::RUNNING;
        }
        else if ( nEntryConnectionMode == embed::EntryInitModes::MEDIA_DESCRIPTOR_INIT )
        {
            // use URL ( may be content or stream later ) from MediaDescriptor to initialize object
            ::rtl::OUString aURL;
            for ( sal_Int32 nInd = 0; nInd < lArguments.getLength(); nInd++ )
                if ( lArguments[nInd].Name.equalsAscii( "URL" ) )
                    lArguments[nInd].Value >>= aURL;

            if ( !aURL.getLength() )
                throw lang::IllegalArgumentException(
                                    ::rtl::OUString::createFromAscii( "Empty URL is provided in the media descriptor!\n" ),
                                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                                    4 );

            CreateOleComponent_Impl();

            // TODO: the m_bIsLink value must be set already
            if ( !m_bIsLink )
                m_pOleComponent->CreateObjectFromFile( aURL );
            else
                m_pOleComponent->CreateLinkFromFile( aURL );

            m_pOleComponent->RunObject();
            m_aClassID = m_pOleComponent->GetCLSID(); // was not set during consruction

            m_nObjectState = embed::EmbedStates::RUNNING;
        }
        //else if ( nEntryConnectionMode == embed::EntryInitModes::TRANSFERABLE_INIT )
        //{
            //TODO:
        //}
        else
            throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "Wrong connection mode is provided!\n" ),
                                        uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                                        3 );
    }
#else
    // On unix the ole object can not do anything except storing itself somewere
    if ( nEntryConnectionMode == embed::EntryInitModes::DEFAULT_INIT && bElExists )
    {
        // TODO/LATER: detect classID of the object
        // can be a real problem for the links

        m_nObjectState = embed::EmbedStates::LOADED;
    }
    else if ( nEntryConnectionMode == embed::EntryInitModes::NO_INIT )
    {
        // do nothing, the object has already switched it's persistence
    }
    else
        throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "Wrong connection mode is provided!\n" ),
                                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                                    3 );

#endif
}

//------------------------------------------------------
void SAL_CALL OleEmbeddedObject::storeToEntry( const uno::Reference< embed::XStorage >& xStorage,
                            const ::rtl::OUString& sEntName,
                            const uno::Sequence< beans::PropertyValue >& lArguments,
                            const uno::Sequence< beans::PropertyValue >& lObjArgs )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    StoreToLocation_Impl( xStorage, sEntName, lArguments, lObjArgs, sal_False );

    // TODO: should the listener notification be done?
}

//------------------------------------------------------
void SAL_CALL OleEmbeddedObject::storeAsEntry( const uno::Reference< embed::XStorage >& xStorage,
                            const ::rtl::OUString& sEntName,
                            const uno::Sequence< beans::PropertyValue >& lArguments,
                            const uno::Sequence< beans::PropertyValue >& lObjArgs )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    StoreToLocation_Impl( xStorage, sEntName, lArguments, lObjArgs, sal_True );

    // TODO: should the listener notification be done here or in saveCompleted?
}

//------------------------------------------------------
void SAL_CALL OleEmbeddedObject::saveCompleted( sal_Bool bUseNew )
        throw ( embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::ResettableMutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
    {
        // the object is still not loaded
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "Can't store object without persistence!\n" ),
                                        uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );
    }

    // it is allowed to call saveCompleted( false ) for nonstored objects
    if ( !m_bWaitSaveCompleted && !bUseNew )
        return;

    OSL_ENSURE( m_bWaitSaveCompleted, "Unexpected saveCompleted() call!\n" );
    if ( !m_bWaitSaveCompleted )
        throw io::IOException(); // TODO: illegal call

    OSL_ENSURE( m_xNewObjectStream.is() && m_xNewParentStorage.is() , "Internal object information is broken!\n" );
    if ( !m_xNewObjectStream.is() || !m_xNewParentStorage.is() )
        throw uno::RuntimeException(); // TODO: broken internal information

    if ( bUseNew )
    {
        SwitchOwnPersistence( m_xNewParentStorage, m_xNewObjectStream, m_aNewEntryName );
        m_bStoreVisRepl = m_bNewVisReplInStream;
        if ( m_xNewCachedVisRepl.is() )
            m_xCachedVisualRepresentation = m_xNewCachedVisRepl;
    }
    else
    {
        // close remembered stream
        try {
            uno::Reference< lang::XComponent > xComponent( m_xNewObjectStream, uno::UNO_QUERY );
            OSL_ENSURE( xComponent.is(), "Wrong storage implementation!" );
            if ( xComponent.is() )
                xComponent->dispose();
        }
        catch ( uno::Exception& )
        {
        }
    }

    sal_Bool bStoreLoaded = m_bStoreLoaded;

    m_xNewObjectStream = uno::Reference< io::XStream >();
    m_xNewParentStorage = uno::Reference< embed::XStorage >();
    m_aNewEntryName = ::rtl::OUString();
    m_bWaitSaveCompleted = sal_False;
    m_bNewVisReplInStream = sal_False;
    m_xNewCachedVisRepl = uno::Reference< io::XStream >();
    m_bStoreLoaded = sal_False;

    aGuard.clear();
    if ( bUseNew )
    {
        MakeEventListenerNotification_Impl( ::rtl::OUString::createFromAscii( "OnSaveAsDone" ) );

        // the object can be changed only on windows
        // the notification should be done only if the object is not in loaded state
        if ( m_pOleComponent && m_nUpdateMode == embed::EmbedUpdateModes::ALWAYS_UPDATE && !bStoreLoaded )
            MakeEventListenerNotification_Impl( ::rtl::OUString::createFromAscii( "OnVisAreaChanged" ) );
    }
}

//------------------------------------------------------
sal_Bool SAL_CALL OleEmbeddedObject::hasEntry()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    ::rtl::OUString::createFromAscii( "The object waits for saveCompleted() call!\n" ),
                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    if ( m_xObjectStream.is() )
        return sal_True;

    return sal_False;
}

//------------------------------------------------------
::rtl::OUString SAL_CALL OleEmbeddedObject::getEntryName()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
    {
        // the object is still not loaded
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object persistence is not initialized!\n" ),
                                        uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );
    }

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    ::rtl::OUString::createFromAscii( "The object waits for saveCompleted() call!\n" ),
                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    return m_aEntryName;
}


//------------------------------------------------------
void SAL_CALL OleEmbeddedObject::storeOwn()
        throw ( embed::WrongStateException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
    // during switching from Activated to Running and from Running to Loaded states the object will
    // ask container to store the object, the container has to make decision
    // to do so or not

    ::osl::ResettableMutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
    {
        // the object is still not loaded
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "Can't store object without persistence!\n" ),
                                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );
    }

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    ::rtl::OUString::createFromAscii( "The object waits for saveCompleted() call!\n" ),
                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    if ( m_bReadOnly )
        throw io::IOException(); // TODO: access denied

    LetCommonStoragePassBeUsed_Impl( m_xObjectStream );

    sal_Bool bStoreLoaded = sal_True;

#ifdef WNT
    if ( m_nObjectState != embed::EmbedStates::LOADED && m_pOleComponent )
    {
        bStoreLoaded = sal_False;

        OSL_ENSURE( m_xParentStorage.is() && m_xObjectStream.is(), "The object has no valid persistence!\n" );

        if ( !m_xObjectStream.is() )
            throw io::IOException(); //TODO: access denied

        SetStreamMediaType_Impl( m_xObjectStream, ::rtl::OUString::createFromAscii( "application/vnd.sun.star.oleobject" ) );
        uno::Reference< io::XOutputStream > xOutStream = m_xObjectStream->getOutputStream();
        if ( !xOutStream.is() )
            throw io::IOException(); //TODO: access denied

        if ( m_bIsLink )
        {
            // just let the link store itself
            // in case visual repersentation must be stored also
            // the procedure should be the same as for embedded objects

            uno::Reference< io::XOutputStream > xOutStream = GetStreamForSaving();

            // should the component detect that it is a link???
            m_pOleComponent->StoreObjectToStream( xOutStream );
        }
        else
        {
            uno::Reference< io::XOutputStream > xOutStream = GetStreamForSaving();
            m_pOleComponent->StoreObjectToStream( xOutStream );
        }

        // the replacement is changed probably
        m_xCachedVisualRepresentation = uno::Reference< io::XStream >();
        m_bVisReplInStream = sal_True;
    }
#endif

    if ( m_bStoreVisRepl != m_bVisReplInStream )
    {
        if ( m_bStoreVisRepl )
        {
            // the m_xCachedVisualRepresentation must be set or it should be already stored
            if ( m_xCachedVisualRepresentation.is() )
                InsertVisualCache_Impl( m_xObjectStream, m_xCachedVisualRepresentation );
            else
            {
                m_xCachedVisualRepresentation = TryToRetrieveCachedVisualRepresentation_Impl( m_xObjectStream );
                OSL_ENSURE( m_xCachedVisualRepresentation.is(), "No representation is available!" );
            }
        }
        else
        {
            m_xCachedVisualRepresentation = TryToRetrieveCachedVisualRepresentation_Impl( m_xObjectStream );
            RemoveVisualCache_Impl( m_xObjectStream );
        }

        m_bVisReplInStream = m_bStoreVisRepl;
    }

    aGuard.clear();

    MakeEventListenerNotification_Impl( ::rtl::OUString::createFromAscii( "OnSaveDone" ) );

    // the object can be changed only on Windows
    // the notification should be done only if the object is not in loaded state
    if ( m_pOleComponent && m_nUpdateMode == embed::EmbedUpdateModes::ALWAYS_UPDATE && !bStoreLoaded )
        MakeEventListenerNotification_Impl( ::rtl::OUString::createFromAscii( "OnVisAreaChanged" ) );
}

//------------------------------------------------------
sal_Bool SAL_CALL OleEmbeddedObject::isReadonly()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
    {
        // the object is still not loaded
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object persistence is not initialized!\n" ),
                                        uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );
    }

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    ::rtl::OUString::createFromAscii( "The object waits for saveCompleted() call!\n" ),
                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    return m_bReadOnly;
}

//------------------------------------------------------
void SAL_CALL OleEmbeddedObject::reload(
                const uno::Sequence< beans::PropertyValue >& lArguments,
                const uno::Sequence< beans::PropertyValue >& lObjArgs )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
    // TODO: use lObjArgs

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
    {
        // the object is still not loaded
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object persistence is not initialized!\n" ),
                                        uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );
    }

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    ::rtl::OUString::createFromAscii( "The object waits for saveCompleted() call!\n" ),
                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    // TODO:
    // throw away current document
    // load new document from current storage
    // use meaningfull part of lArguments
}

//------------------------------------------------------
void SAL_CALL OleEmbeddedObject::breakLink( const uno::Reference< embed::XStorage >& xStorage,
                                                const ::rtl::OUString& sEntName )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "No parent storage is provided!\n" ),
                                            uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                                            1 );

    if ( !sEntName.getLength() )
        throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "Empty element name is provided!\n" ),
                                            uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                                            2 );

    // TODO: The object must be at least in Running state;
    if ( !m_bIsLink || m_nObjectState == -1 || !m_pOleComponent )
    {
        // it must be a linked initialized object
        throw embed::WrongStateException(
                    ::rtl::OUString::createFromAscii( "The object is not a valid linked object!\n" ),
                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );
    }

    if ( m_bReadOnly )
        throw io::IOException(); // TODO: Access denied

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    ::rtl::OUString::createFromAscii( "The object waits for saveCompleted() call!\n" ),
                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );


#ifdef WNT
    if ( m_pOleComponent )
    {
        // TODO: create an object based on the link
        OleComponent* pNewOleComponent = new OleComponent( m_xFactory, this );
        try {
            pNewOleComponent->InitEmbeddedCopyOfLink( m_pOleComponent );
        }
        catch ( uno::Exception& )
        {
            delete pNewOleComponent;
            throw;
        }

        try {
            GetRidOfComponent();
        }
        catch( uno::Exception& )
        {
            delete pNewOleComponent;
            throw;
        }

        CreateOleComponent_Impl( pNewOleComponent );

        if ( m_xParentStorage != xStorage || !m_aEntryName.equals( sEntName ) )
            SwitchOwnPersistence( xStorage, sEntName );

        if ( m_nObjectState != embed::EmbedStates::LOADED )
        {
            // TODO: should we activate the new object if the link was activated?

            sal_Int32 nTargetState = m_nObjectState;
            m_nObjectState = embed::EmbedStates::LOADED;

            if ( m_nObjectState == embed::EmbedStates::RUNNING )
                m_pOleComponent->RunObject(); // the object already was in running state, the server must be installed
            else // m_nObjectState == embed::EmbedStates::ACTIVE
            {
                m_pOleComponent->RunObject(); // the object already was in running state, the server must be installed
                m_pOleComponent->ExecuteVerb( embed::EmbedVerbs::MS_OLEVERB_OPEN );
            }

            m_nObjectState = nTargetState;
        }

        m_bIsLink = sal_False;
        m_aLinkURL = ::rtl::OUString();
    }
    else
#endif
    {
        throw io::IOException(); //TODO:
    }
}

//------------------------------------------------------
sal_Bool SAL_CALL  OleEmbeddedObject::isLink()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    return m_bIsLink;
}

//------------------------------------------------------
::rtl::OUString SAL_CALL OleEmbeddedObject::getLinkURL()
        throw ( embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    ::rtl::OUString::createFromAscii( "The object waits for saveCompleted() call!\n" ),
                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    if ( !m_bIsLink )
        throw embed::WrongStateException(
                    ::rtl::OUString::createFromAscii( "The object is not a link object!\n" ),
                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    // TODO: probably the link URL can be retrieved from OLE

    return m_aLinkURL;
}

