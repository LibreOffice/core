/*************************************************************************
 *
 *  $RCSfile: FileAccess.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-22 12:51:43 $
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <osl/mutex.hxx>
#include <osl/diagnose.h>

#include <uno/mapping.hxx>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase1.hxx>

#include <tools/ref.hxx>
#include <tools/urlobj.hxx>
#include <ucbhelper/content.hxx>
using namespace ::ucb;


#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataStreamer.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/beans/Property.hpp>

//#include <com/sun/star/lang/IllegalArgumentException.hpp>
//#include <com/sun/star/connection/XConnector.hpp>

//#include "connector.hxx"

#define IMPLEMENTATION_NAME "com.sun.star.comp.ucb.SimpleFileAccess"
#define SERVICE_NAME "com.sun.star.ucb.SimpleFileAccess"

using namespace ::osl;
using namespace ::rtl;
using namespace ::cppu;
//using namespace ::vos;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::registry;

namespace io_FileAccess
{


//===========================================================================
// Implementation XSimpleFileAccess

typedef WeakImplHelper1< XSimpleFileAccess > FileAccessHelper;
class OCommandEnvironment;

class OFileAccess : public FileAccessHelper
{
    Reference< XCommandEnvironment > mxEnvironment;
    OCommandEnvironment* mpEnvironment;

    void transferImpl( const OUString& rSource, const OUString& rDest, sal_Bool bMoveData )
        throw(CommandAbortedException, Exception, RuntimeException);

public:
    OFileAccess() : mpEnvironment( NULL ) {}

    // Methods
    virtual void SAL_CALL copy( const ::rtl::OUString& SourceURL, const ::rtl::OUString& DestURL ) throw(::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL move( const ::rtl::OUString& SourceURL, const ::rtl::OUString& DestURL ) throw(::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL kill( const ::rtl::OUString& FileURL ) throw(::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isFolder( const ::rtl::OUString& FileURL ) throw(::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isReadOnly( const ::rtl::OUString& FileURL ) throw(::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setReadOnly( const ::rtl::OUString& FileURL, sal_Bool bReadOnly ) throw(::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL createFolder( const ::rtl::OUString& NewFolderURL ) throw(::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getSize( const ::rtl::OUString& FileURL ) throw(::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getContentType( const ::rtl::OUString& FileURL ) throw(::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::util::DateTime SAL_CALL getDateTimeModified( const ::rtl::OUString& FileURL ) throw(::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getFolderContents( const ::rtl::OUString& FolderURL, sal_Bool bIncludeFolders ) throw(::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL exists( const ::rtl::OUString& FileURL ) throw(::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL openFileRead( const ::rtl::OUString& FileURL ) throw(::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > SAL_CALL openFileWrite( const ::rtl::OUString& FileURL ) throw(::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > SAL_CALL openFileReadWrite( const ::rtl::OUString& FileURL ) throw(::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setInteractionHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler ) throw(::com::sun::star::uno::RuntimeException);

};


//===========================================================================
// Implementation XActiveDataSink

typedef WeakImplHelper1< XActiveDataSink > ActiveDataSinkHelper;

class OActiveDataSink : public ActiveDataSinkHelper
{
    Reference< XInputStream > mxStream;

public:

    // Methods
    virtual void SAL_CALL setInputStream( const Reference< XInputStream >& aStream )
        throw(RuntimeException);
    virtual Reference< XInputStream > SAL_CALL getInputStream(  )
        throw(RuntimeException);
};

void OActiveDataSink::setInputStream( const Reference< XInputStream >& aStream )
    throw(RuntimeException)
{
    mxStream = aStream;
}

Reference< XInputStream > OActiveDataSink::getInputStream()
    throw(RuntimeException)
{
    return mxStream;
}


//===========================================================================
// Implementation XActiveDataSource

typedef WeakImplHelper1< XActiveDataSource > ActiveDataSourceHelper;

class OActiveDataSource : public ActiveDataSourceHelper
{
    Reference< XOutputStream > mxStream;

public:

    // Methods
    virtual void SAL_CALL setOutputStream( const Reference< XOutputStream >& aStream )
        throw(RuntimeException);
    virtual Reference< XOutputStream > SAL_CALL getOutputStream()
        throw(RuntimeException);
};

void OActiveDataSource::setOutputStream( const Reference< XOutputStream >& aStream )
    throw(RuntimeException)
{
    mxStream = aStream;
}

Reference< XOutputStream > OActiveDataSource::getOutputStream()
    throw(RuntimeException)
{
    return mxStream;
}


//===========================================================================
// Implementation XActiveDataStreamer

typedef WeakImplHelper1< XActiveDataStreamer > ActiveDataStreamerHelper;

class OActiveDataStreamer : public ActiveDataStreamerHelper
{
    Reference< XStream > mxStream;

public:

    // Methods
    virtual void SAL_CALL setStream( const Reference< XStream >& aStream )
        throw(RuntimeException);
    virtual Reference< XStream > SAL_CALL getStream()
        throw(RuntimeException);
};

void OActiveDataStreamer::setStream( const Reference< XStream >& aStream )
    throw(RuntimeException)
{
    mxStream = aStream;
}

Reference< XStream > OActiveDataStreamer::getStream()
    throw(RuntimeException)
{
    return mxStream;
}



//===========================================================================
// Implementation XCommandEnvironment

typedef WeakImplHelper1< XCommandEnvironment > CommandEnvironmentHelper;

class OCommandEnvironment : public CommandEnvironmentHelper
{
    Reference< XInteractionHandler > mxInteraction;

public:
    void setHandler( Reference< XInteractionHandler > xInteraction_ )
    {
        mxInteraction = xInteraction_;
    }

    // Methods
    virtual Reference< XInteractionHandler > SAL_CALL getInteractionHandler()
        throw(RuntimeException);
    virtual Reference< XProgressHandler > SAL_CALL getProgressHandler()
        throw(RuntimeException);
};

Reference< XInteractionHandler > OCommandEnvironment::getInteractionHandler()
    throw(RuntimeException)
{
    return mxInteraction;
}

Reference< XProgressHandler > OCommandEnvironment::getProgressHandler()
    throw(RuntimeException)
{
    Reference< XProgressHandler > xRet;
    return xRet;
}


//===========================================================================

void OFileAccess::transferImpl( const OUString& rSource, const OUString& rDest, sal_Bool bMoveData )
    throw(CommandAbortedException, Exception, RuntimeException)
{
    // SfxContentHelper::Transfer_Impl
    INetURLObject aSourceObj( rSource, INET_PROT_FILE );
    INetURLObject aDestObj( rDest, INET_PROT_FILE );
    String aName = aDestObj.getName( INetURLObject::LAST_SEGMENT, true,
        INetURLObject::DECODE_WITH_CHARSET );
    aDestObj.removeSegment();
    aDestObj.setFinalSlash();

    Content aDestPath( aDestObj.GetMainURL(), mxEnvironment );
    aDestPath.executeCommand( OUString::createFromAscii( "transfer" ), makeAny(
        ::com::sun::star::ucb::TransferInfo( bMoveData, aSourceObj.GetMainURL(), aName,
                                             ::com::sun::star::ucb::NameClash::OVERWRITE ) ) );
}

void OFileAccess::copy( const OUString& SourceURL, const OUString& DestURL )
    throw(CommandAbortedException, Exception, RuntimeException)
{
    transferImpl( SourceURL, DestURL, sal_False );
}

void OFileAccess::move( const OUString& SourceURL, const OUString& DestURL )
    throw(CommandAbortedException, Exception, RuntimeException)
{
    transferImpl( SourceURL, DestURL, sal_True );
}

void OFileAccess::kill( const OUString& FileURL )
    throw(CommandAbortedException, Exception, RuntimeException)
{
    // SfxContentHelper::Kill
    INetURLObject aDeleteObj( FileURL, INET_PROT_FILE );
    Content aCnt( aDeleteObj.GetMainURL(), mxEnvironment );
    aCnt.executeCommand( OUString::createFromAscii( "delete" ), makeAny( sal_Bool( sal_True ) ) );
}

sal_Bool OFileAccess::isFolder( const OUString& FileURL )
    throw(CommandAbortedException, Exception, RuntimeException)
{
    sal_Bool bRet = sal_False;
    try
    {
        INetURLObject aURLObj( FileURL, INET_PROT_FILE );
        Content aCnt( aURLObj.GetMainURL(), mxEnvironment );
        bRet = aCnt.isFolder();
    }
    catch (CommandAbortedException &) {}
    catch (RuntimeException &) {}
    catch (Exception &) {}
    return bRet;
}

sal_Bool OFileAccess::isReadOnly( const OUString& FileURL )
    throw(CommandAbortedException, Exception, RuntimeException)
{
    INetURLObject aURLObj( FileURL, INET_PROT_FILE );
    Content aCnt( aURLObj.GetMainURL(), mxEnvironment );
    Any aRetAny = aCnt.getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsReadOnly" ) ) );
    sal_Bool bRet = sal_False;
    aRetAny >>= bRet;
    return bRet;
}

void OFileAccess::setReadOnly( const OUString& FileURL, sal_Bool bReadOnly )
    throw(CommandAbortedException, Exception, RuntimeException)
{
    INetURLObject aURLObj( FileURL, INET_PROT_FILE );
    Content aCnt( aURLObj.GetMainURL(), mxEnvironment );
    Any aAny;
    aAny <<= bReadOnly;
    aCnt.setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsReadOnly" ) ), aAny );
}

void OFileAccess::createFolder( const OUString& NewFolderURL )
    throw(CommandAbortedException, Exception, RuntimeException)
{
    // Does the folder already exist?
    if( !NewFolderURL.getLength() || isFolder( NewFolderURL ) )
        return;

    // SfxContentHelper::MakeFolder
    INetURLObject aURL( NewFolderURL, INET_PROT_FILE );
    String aNewFolderURLStr = aURL.GetMainURL();
    String aTitle = aURL.getName();
    aURL.removeSegment();

    // Does the base folder exist? Otherwise create it first
    String aBaseFolderURLStr = aURL.GetMainURL();
    if( !isFolder( aBaseFolderURLStr ) )
    {
        createFolder( aBaseFolderURLStr );
    }

    Sequence<OUString> aNames(2);
    OUString* pNames = aNames.getArray();
    pNames[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) );
    pNames[1] = OUString( RTL_CONSTASCII_USTRINGPARAM( "IsFolder" ) );
    Sequence<Any> aValues(2);
    Any* pValues = aValues.getArray();
    pValues[0] = makeAny( OUString( aTitle ) );
    pValues[1] = makeAny( sal_Bool( sal_True ) );
    Reference< XCommandEnvironment > aCmdEnv;

    Content aCnt( aURL.GetMainURL(), aCmdEnv );
    Content aNewFolder( aNewFolderURLStr, aCmdEnv );
    OUString aType( RTL_CONSTASCII_USTRINGPARAM( "application/vnd.sun.staroffice.fsys-folder" ) );
    aCnt.insertNewContent( aType, aNames, aValues, aNewFolder );
}

sal_Int32 OFileAccess::getSize( const OUString& FileURL )
    throw(CommandAbortedException, Exception, RuntimeException)
{
    // SfxContentHelper::GetSize
    sal_Int32 nSize = 0;
    sal_Int64 nTemp = 0;
    INetURLObject aObj( FileURL, INET_PROT_FILE );
    Content aCnt( aObj.GetMainURL(), mxEnvironment );
    aCnt.getPropertyValue( OUString::createFromAscii( "Size" ) ) >>= nTemp;
    nSize = (sal_Int32)nTemp;
    return nSize;
}

OUString OFileAccess::getContentType( const OUString& FileURL )
    throw(CommandAbortedException, Exception, RuntimeException)
{
    INetURLObject aObj( FileURL, INET_PROT_FILE );
    Content aCnt( aObj.GetMainURL(), mxEnvironment );

    Reference< XContent > xContent = aCnt.get();
    OUString aTypeStr = xContent->getContentType();
    return aTypeStr;
}

DateTime OFileAccess::getDateTimeModified( const OUString& FileURL )
    throw(CommandAbortedException, Exception, RuntimeException)
{
    INetURLObject aFileObj( FileURL, INET_PROT_FILE );
    DateTime aDateTime;

    Reference< XCommandEnvironment > aCmdEnv;
    Content aYoung( aFileObj.GetMainURL(), aCmdEnv );
    aYoung.getPropertyValue( OUString::createFromAscii( "DateModified" ) ) >>= aDateTime;
    return aDateTime;
}


DECLARE_LIST( StringList_Impl, OUString* )

Sequence< OUString > OFileAccess::getFolderContents( const OUString& FolderURL, sal_Bool bIncludeFolders )
    throw(CommandAbortedException, Exception, RuntimeException)
{
    // SfxContentHelper::GetFolderContents

    StringList_Impl* pFiles = NULL;
    INetURLObject aFolderObj( FolderURL, INET_PROT_FILE );

    Content aCnt( aFolderObj.GetMainURL(), mxEnvironment );
    Reference< XResultSet > xResultSet;
    Sequence< OUString > aProps(0);
    //Sequence< OUString > aProps(1);
    //OUString* pProps = aProps.getArray();
    //pProps[0] == OUString::createFromAscii( "Url" );

    ResultSetInclude eInclude = bIncludeFolders ? INCLUDE_FOLDERS_AND_DOCUMENTS : INCLUDE_DOCUMENTS_ONLY;
    xResultSet = aCnt.createCursor( aProps, eInclude );

    if ( xResultSet.is() )
    {
        pFiles = new StringList_Impl;
        Reference< com::sun::star::ucb::XContentAccess > xContentAccess( xResultSet, UNO_QUERY );

        while ( xResultSet->next() )
        {
            OUString aId = xContentAccess->queryContentIdentifierString();
            OUString* pFile = new OUString( aId );
            pFiles->Insert( pFile, LIST_APPEND );
        }
    }

    if ( pFiles )
    {
        USHORT nCount = pFiles->Count();
        Sequence < OUString > aRet( nCount );
        OUString* pRet = aRet.getArray();
        for ( USHORT i = 0; i < nCount; ++i )
        {
            OUString* pFile = pFiles->GetObject(i);
            pRet[i] = *( pFile );
            delete pFile;
        }
        delete pFiles;
        return aRet;
    }
    else
        return Sequence < OUString > ();
}

sal_Bool OFileAccess::exists( const OUString& FileURL )
    throw(CommandAbortedException, Exception, RuntimeException)
{
    sal_Bool bRet = sal_False;
    try
    {
        bRet = isFolder( FileURL );
        if( !bRet )
        {
            Reference< XInputStream > xStream = openFileRead( FileURL );
            bRet = xStream.is();
            if( bRet )
                xStream->closeInput();
        }
    }
    catch (CommandAbortedException &) {}
    catch (RuntimeException &) {}
    catch (Exception &) {}
    return bRet;
}

Reference< XInputStream > OFileAccess::openFileRead( const OUString& FileURL )
    throw(CommandAbortedException, Exception, RuntimeException)
{
    Reference< XInputStream > xRet;
    INetURLObject aObj( FileURL, INET_PROT_FILE );
    Content aCnt( aObj.GetMainURL(), mxEnvironment );

    Reference< XActiveDataSink > xSink = (XActiveDataSink*)(new OActiveDataSink());
    sal_Bool bRet = aCnt.openStream( xSink );
    if( bRet )
        xRet = xSink->getInputStream();
    return xRet;
}

Reference< XOutputStream > OFileAccess::openFileWrite( const OUString& FileURL )
    throw(CommandAbortedException, Exception, RuntimeException)
{
    Reference< XOutputStream > xRet;
    Reference< XStream > xStream = OFileAccess::openFileReadWrite( FileURL );
    if( xStream.is() )
        xRet = xStream->getOutputStream();
    return xRet;
}

Reference< XStream > OFileAccess::openFileReadWrite( const OUString& FileURL )
    throw(CommandAbortedException, Exception, RuntimeException)
{
    InsertCommandArgument aInsertArg;
    aInsertArg.Data = Reference< XInputStream >();
    aInsertArg.ReplaceExisting = sal_True;

    INetURLObject aFileObj( FileURL, INET_PROT_FILE );
    Content aCnt( aFileObj.GetMainURL(), mxEnvironment );
    Any aCmdArg;
    aCmdArg <<= aInsertArg;
    aCnt.executeCommand( OUString::createFromAscii( "insert" ), aCmdArg );


    Reference< XActiveDataStreamer > xSink = (XActiveDataStreamer*)new OActiveDataStreamer();
    Reference< XInterface > xSinkIface = Reference< XInterface >::query( xSink );

    OpenCommandArgument2 aArg;
    aArg.Mode       = OpenMode::DOCUMENT;
    aArg.Priority   = 0; // unused
    aArg.Sink       = xSink;
    aArg.Properties = Sequence< Property >( 0 ); // unused
    aCmdArg <<= aArg;
    aCnt.executeCommand( OUString::createFromAscii( "open" ), aCmdArg );

    Reference< XStream > xRet = xSink->getStream();
    return xRet;
}

void OFileAccess::setInteractionHandler( const Reference< XInteractionHandler >& Handler )
    throw(RuntimeException)
{
    if( !mpEnvironment )
    {
        mpEnvironment = new OCommandEnvironment();
        mxEnvironment = (XCommandEnvironment*)mpEnvironment;
    }
    mpEnvironment->setHandler( Handler );
}


Reference< XInterface > SAL_CALL FileAccess_CreateInstance( const Reference< XMultiServiceFactory > &)
{
    return Reference < XInterface >( ( OWeakObject * ) new OFileAccess );
}


Sequence< OUString > FileAccess_getSupportedServiceNames()
{
    static Sequence < OUString > *pNames = 0;
    if( ! pNames )
    {
        MutexGuard guard( Mutex::getGlobalMutex() );
        if( !pNames )
        {
            static Sequence< OUString > seqNames(1);
            seqNames.getArray()[0] = OUString::createFromAscii( SERVICE_NAME );
            pNames = &seqNames;
        }
    }
    return *pNames;
}


}





//==================================================================================================
// Component exports

extern "C"
{
//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                    OUString::createFromAscii("/" IMPLEMENTATION_NAME "/UNO/SERVICES" ) ) );

            const Sequence< OUString > & rSNL = io_FileAccess::FileAccess_getSupportedServiceNames();
            const OUString * pArray = rSNL.getConstArray();
            for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;

    if (pServiceManager && rtl_str_compare( pImplName, IMPLEMENTATION_NAME ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString::createFromAscii( pImplName ),
            io_FileAccess::FileAccess_CreateInstance,
            io_FileAccess::FileAccess_getSupportedServiceNames() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}


