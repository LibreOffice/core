/*************************************************************************
 *
 *  $RCSfile: olecomponent.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mav $ $Date: 2003-11-17 16:19:24 $
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

#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_DOUBLEINITIALIZATIONEXCEPTION_HPP_
#include <com/sun/star/frame/DoubleInitializationException.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_WRONGSTATEEXCEPTION_HPP_
#include <com/sun/star/embed/WrongStateException.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_XSIMPLEFILEACCESS_HPP_
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif


#include <platform.h>

#include <cppuhelper/interfacecontainer.h>
#include <osl/file.hxx>

#include "olecomponent.hxx"
#include "olewrapclient.hxx"
#include "advisesink.hxx"
#include "oleembobj.hxx"


using namespace ::com::sun::star;

#define     MAX_ENUM_ELE     20
const sal_Int32 n_ConstBufferSize = 32000;

STDAPI StarObject_SwitchDisplayAspect(IUnknown *pObj, LPDWORD pdwCurAspect
    , DWORD dwNewAspect, HGLOBAL hMetaPict, BOOL fDeleteOld
    , BOOL fViewAdvise, IAdviseSink *pSink, BOOL *pfMustUpdate)
{
    // TODO: implement the switching
    return S_OK;
}


sal_Bool ConvertDataForFlavor( const STGMEDIUM& aMedium, const datatransfer::DataFlavor& aFlavor, uno::Any& aResult )
{
    // TODO: try to convert data from Medium format to specified Flavor format
    return sal_False;
}

//----------------------------------------------
sal_Bool GraphicalFlavor( const datatransfer::DataFlavor& aFlavor )
{
    // TODO: just check that the requested flavour is one of supported graphical flavours
    return sal_False;
}

//-----------------------------------------------
// TODO: probably later such a common function can be moved
//       to a separate helper library.
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
sal_Bool KillFile( const ::rtl::OUString& aURL, const uno::Reference< lang::XMultiServiceFactory >& xFactory )
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
::rtl::OUString GetNewTempFileURL( const uno::Reference< lang::XMultiServiceFactory >& xFactory )
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
::rtl::OUString GetNewFilledTempFile( const uno::Reference< io::XInputStream >& xInStream,
                                      const uno::Reference< lang::XMultiServiceFactory >& xFactory )
        throw( io::IOException )
{
    OSL_ENSURE( xInStream.is() && xFactory.is(), "Wrong parameters are provided!\n" );

    ::rtl::OUString aResult = GetNewTempFileURL( xFactory );

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
               KillFile( aResult, xFactory );
            throw io::IOException(); //TODO:
        }
        catch( io::IOException& )
        {
               KillFile( aResult, xFactory );
            throw;
        }
        catch( uno::RuntimeException& )
        {
               KillFile( aResult, xFactory );
            throw;
        }
        catch( uno::Exception& )
        {
               KillFile( aResult, xFactory );
            aResult = ::rtl::OUString();
        }
    }

    return aResult;
}

//----------------------------------------------
sal_Bool GetClassIDFromSequence( uno::Sequence< sal_Int8 > aSeq, CLSID& aResult )
{
    if ( aSeq.getLength() == 16 )
    {
        aResult.Data1 = ( ( ( ( ( (sal_uInt8)aSeq[0] << 8 ) + (sal_uInt8)aSeq[1] ) << 8 ) + (sal_uInt8)aSeq[2] ) << 8 ) + (sal_uInt8)aSeq[3];
        aResult.Data2 = ( (sal_uInt8)aSeq[4] << 8 ) + (sal_uInt8)aSeq[5];
        aResult.Data3 = ( (sal_uInt8)aSeq[6] << 8 ) + (sal_uInt8)aSeq[7];
        for( int nInd = 0; nInd < 8; nInd++ )
            aResult.Data4[nInd] = (sal_uInt8)aSeq[nInd+8];

        return sal_True;
    }

    return sal_False;
}

//----------------------------------------------
::rtl::OUString WinAccToVcl( const sal_Unicode* pStr )
{
    ::rtl::OUString aResult;

    if( pStr )
    {
        while ( *pStr )
        {
            if ( *pStr == '&' )
            {
                aResult += ::rtl::OUString::createFromAscii( "~" );
                while( *(++pStr) == '&' );
            }
            else
            {
                aResult += ::rtl::OUString( pStr, 1 );
                pStr++;
            }
        }
    }

    return aResult;
}

//----------------------------------------------
OleComponent::OleComponent( const uno::Reference< lang::XMultiServiceFactory >& xFactory, OleEmbeddedObject* pUnoOleObject )
: m_pInterfaceContainer( NULL )
, m_bDisposed( sal_False )
, m_xFactory( xFactory )
, m_pOleWrapClientSite( NULL )
, m_pImplAdviseSink( NULL )
, m_pUnoOleObject( pUnoOleObject )
, m_nMSAspect( 0 )
, m_nOLEMiscFlags( 0 )
, m_nAdvConn( 0 )
, m_nSupportedFormat( 0 )
, m_nSupportedMedium( 0 )
, m_bOleInitialized( sal_False )
{
    OSL_ENSURE( m_pUnoOleObject, "No owner object is provided!" );

    HRESULT hr = OleInitialize( NULL );
    OSL_ENSURE( hr == S_OK || hr == S_FALSE, "The ole can not be successfuly initialized\n" );
    if ( hr == S_OK || hr == S_FALSE )
        m_bOleInitialized = sal_True;

    m_pOleWrapClientSite = new OleWrapperClientSite( (OleComponent*)this );
    m_pOleWrapClientSite->AddRef();

    m_pImplAdviseSink = new OleWrapperAdviseSink( (OleComponent*)this );
    m_pImplAdviseSink->AddRef();
}

//----------------------------------------------
OleComponent::~OleComponent()
{
    OSL_ENSURE( !m_pOleWrapClientSite && !m_pImplAdviseSink && !m_pInterfaceContainer && !m_bOleInitialized,
                "The object was not closed successfully! DISASTER is possible!" );

    if ( m_pOleWrapClientSite || m_pImplAdviseSink || m_pInterfaceContainer || m_bOleInitialized )
    {
        m_refCount++;
        try {
            Dispose();
        } catch( uno::Exception& ) {}
    }
}

//----------------------------------------------
void OleComponent::Dispose()
{
    if ( m_pOleWrapClientSite )
    {
        m_pOleWrapClientSite->disconnectOleComponent();
        m_pOleWrapClientSite->Release();
        m_pOleWrapClientSite = NULL;
    }

    if ( m_pImplAdviseSink )
    {
        m_pImplAdviseSink->disconnectOleComponent();
        m_pImplAdviseSink->Release();
        m_pImplAdviseSink = NULL;
    }

    if ( m_pInterfaceContainer )
    {
        lang::EventObject aEvent( (embed::XEmbeddedObject*)this );
        m_pInterfaceContainer->disposeAndClear( aEvent );

        delete m_pInterfaceContainer;
        m_pInterfaceContainer = NULL;
    }

    if ( m_bOleInitialized )
    {
        OleUninitialize();
        m_bOleInitialized = sal_False;
    }

    m_bDisposed = sal_True;
}

//----------------------------------------------
void OleComponent::disconnectEmbeddedObject()
{
    // must not be called from destructor of UNO OLE object!!!
    osl::MutexGuard aGuard( m_aMutex );
    m_pUnoOleObject = NULL;
}

//----------------------------------------------
CComPtr< IStorage > OleComponent::CreateIStorageOnXInputStream_Impl( const uno::Reference< io::XInputStream >& xInStream )
{
    // TODO: in future a global memory should be used instead of file.

    OSL_ENSURE( !m_aTempURL.getLength(), "The object already has temporary representation!\n" );

    // write the stream to the temporary file
    m_aTempURL = GetNewFilledTempFile( xInStream, m_xFactory );
    if ( !m_aTempURL.getLength() )
        throw uno::RuntimeException(); // TODO

    // open an IStorage based on the temporary file
    ::rtl::OUString aTempFilePath;
    if ( ::osl::FileBase::getSystemPathFromFileURL( m_aTempURL, aTempFilePath ) != ::osl::FileBase::E_None )
        throw uno::RuntimeException(); // TODO: something dangerous happend

    HRESULT hr = StgCreateDocfile( aTempFilePath, STGM_READWRITE | STGM_DELETEONRELEASE, 0, &m_pIStorage );
    if ( FAILED( hr ) || !m_pIStorage )
        throw io::IOException(); // TODO: transport error code?

    return m_pIStorage;
}

//----------------------------------------------
CComPtr< IStorage > OleComponent::CreateNewIStorage_Impl()
{
    // TODO: in future a global memory should be used instead of file.

    OSL_ENSURE( !m_aTempURL.getLength(), "The object already has temporary representation!\n" );

    // write the stream to the temporary file
    m_aTempURL = GetNewTempFileURL( m_xFactory );
    if ( !m_aTempURL.getLength() )
        throw uno::RuntimeException(); // TODO

    // open an IStorage based on the temporary file
    ::rtl::OUString aTempFilePath;
    if ( ::osl::FileBase::getSystemPathFromFileURL( m_aTempURL, aTempFilePath ) != ::osl::FileBase::E_None )
        throw uno::RuntimeException(); // TODO: something dangerous happend

    HRESULT hr = StgCreateDocfile( aTempFilePath, STGM_CREATE | STGM_READWRITE | STGM_TRANSACTED | STGM_DELETEONRELEASE, 0, &m_pIStorage );
    if ( FAILED( hr ) || !m_pIStorage )
        throw io::IOException(); // TODO: transport error code?

    return m_pIStorage;
}

//----------------------------------------------
void OleComponent::RetrieveObjectDataFlavors_Impl()
{
    if ( !m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    if ( !m_aDataFlavors.getLength() )
    {
        CComPtr< IDataObject > pDataObject;
        HRESULT hr = m_pObj->QueryInterface( IID_IDataObject, (void**)&pDataObject );
        if ( FAILED( hr ) || !pDataObject )
            throw io::IOException(); // TODO: transport error code

        CComPtr< IEnumFORMATETC > pFormatEnum;
        hr = pDataObject->EnumFormatEtc( DATADIR_GET, &pFormatEnum );
        if ( FAILED( hr ) || !pFormatEnum )
            throw io::IOException(); // TODO: transport error code

        FORMATETC pElem[ MAX_ENUM_ELE ];
        ULONG nNum = 0;

        do
        {
            HRESULT hr = pFormatEnum->Next( MAX_ENUM_ELE, pElem, &nNum );
            if( hr == S_OK || hr == S_FALSE )
            {
                for( sal_uInt32 nInd = 0; nInd < nNum; nInd++ )
                {
                    //TODO: add OOo format reachable from specified OLE format to the list
                }
            }
            else
                break;
        }
        while( nNum == MAX_ENUM_ELE );
    }
}

//----------------------------------------------
sal_Bool OleComponent::InitializeObject_Impl( sal_uInt32 nIconHandle )
// There will be no static objects!
{
    if ( !m_pObj )
        return sal_False;

    // the linked object will be detected here
    CComPtr< IOleLink > pOleLink;
    HRESULT hr = m_pObj->QueryInterface( IID_IOleLink, (void**)&pOleLink );
    m_pUnoOleObject->SetObjectIsLink_Impl( pOleLink != NULL );


    hr = m_pObj->QueryInterface( IID_IViewObject2, (void**)&m_pViewObject2 );
    if ( FAILED( hr ) || !m_pViewObject2 )
        return sal_False;

    m_pViewObject2->SetAdvise( m_nMSAspect, 0, m_pImplAdviseSink );

    // register cache in case there is no ( Visio 2000 workaround )
    IOleCache* pIOleCache = NULL;
    if ( SUCCEEDED( m_pObj->QueryInterface( IID_IOleCache, (void**)&pIOleCache ) ) && pIOleCache )
    {
        IEnumSTATDATA* pEnumSD = NULL;
        HRESULT hr = pIOleCache->EnumCache( &pEnumSD );

        sal_Bool bRegister = sal_True;
        if ( SUCCEEDED( hr ) && pEnumSD )
        {
            pEnumSD->Reset();
            STATDATA aSD;
            DWORD nNum;
            while( SUCCEEDED( pEnumSD->Next( 1, &aSD, &nNum ) ) && nNum == 1 )
            {
                if ( aSD.formatetc.cfFormat == 0 )
                {
                    bRegister = sal_False;
                    break;
                }
            }
        }

        if ( bRegister )
        {
            DWORD nConn;
            FORMATETC aFormat = { 0, 0, DVASPECT_CONTENT, -1, TYMED_MFPICT };
            hr = pIOleCache->Cache( &aFormat, ADVFCACHE_ONSAVE, &nConn );
        }

        pIOleCache->Release();
        pIOleCache = NULL;
    }

    hr = m_pObj->QueryInterface( IID_IOleObject, (void**)&m_pOleObject );
    if ( FAILED( hr ) || !m_pOleObject )
        return sal_False; // Static objects are not supported, they should be inserted as graphics

    m_pOleObject->GetMiscStatus( m_nMSAspect, (DWORD*)&m_nOLEMiscFlags );
    // TODO: use other misc flags also
    // the object should have drawable aspect even in case it supports only iconic representation
    // if ( m_nOLEMiscFlags & OLEMISC_ONLYICONIC )
    //  m_nMSAspect = DVASPECT_ICON;

    m_pOleObject->SetClientSite( m_pOleWrapClientSite );
    m_pOleObject->Advise( m_pImplAdviseSink, (DWORD*)&m_nAdvConn );

    OleSetContainedObject( m_pOleObject, TRUE );

    if ( m_nMSAspect & DVASPECT_ICON )
    {
        // TODO: the icon provided from dialog must be stored
        //       if there is no icon, it should be retrieved
        DWORD nOldAspect = DVASPECT_CONTENT;
        IAdviseSink *pSink;

        pSink = ( nIconHandle == NULL ) ? NULL : m_pImplAdviseSink;

        StarObject_SwitchDisplayAspect( m_pOleObject,
                                        &nOldAspect,
                                        DVASPECT_ICON,
                                        (HGLOBAL)(UINT)nIconHandle,
                                        FALSE,
                                        nIconHandle != NULL,
                                        pSink,
                                        NULL );
    }

    return sal_True;
}

//----------------------------------------------
void OleComponent::LoadEmbeddedObject( const uno::Reference< io::XInputStream >& xInStream, sal_Int64 nAspect )
{
    if ( !xInStream.is() )
        throw lang::IllegalArgumentException(); // TODO

    if ( m_pIStorage || m_aTempURL.getLength() )
        throw frame::DoubleInitializationException(); // TODO the object is already initialized

    m_nMSAspect = (DWORD)nAspect; // first 32 bits are for MS aspects

    m_pIStorage = CreateIStorageOnXInputStream_Impl( xInStream );
    if ( !m_pIStorage )
        throw uno::RuntimeException(); // TODO:

    HRESULT hr = OleLoad( m_pIStorage, IID_IUnknown, NULL, (void**)&m_pObj );
    if ( FAILED( hr ) || !m_pObj )
        throw uno::RuntimeException();

    if ( !InitializeObject_Impl( NULL ) )
        throw uno::RuntimeException(); // TODO
}

//----------------------------------------------
void OleComponent::CreateNewEmbeddedObject( const uno::Sequence< sal_Int8 >& aSeqCLSID,
                                            sal_Int64 nAspect,
                                            sal_uInt32 nIconHandle )
{
    CLSID aClsID;

    if ( !GetClassIDFromSequence( aSeqCLSID, aClsID ) )
        throw lang::IllegalArgumentException(); // TODO

    if ( m_pIStorage || m_aTempURL.getLength() )
        throw frame::DoubleInitializationException(); // the object is already initialized

    m_pIStorage = CreateNewIStorage_Impl();
    if ( !m_pIStorage )
        throw uno::RuntimeException(); // TODO

    // FORMATETC aFormat = { CF_METAFILEPICT, NULL, nAspect, -1, TYMED_MFPICT }; // for OLE..._DRAW should be NULL
    m_nMSAspect = (DWORD)nAspect; // first 32 bits are for MS aspects

    HRESULT hr = OleCreate( aClsID,
                            IID_IUnknown,
                            OLERENDER_DRAW, // OLERENDER_FORMAT
                            NULL,           // &aFormat,
                            NULL,
                            m_pIStorage,
                            (void**)&m_pObj );

    if ( FAILED( hr ) || !m_pObj )
        throw uno::RuntimeException(); // TODO

    if ( !InitializeObject_Impl( nIconHandle ) )
        throw uno::RuntimeException(); // TODO

    // TODO: getExtent???
}

//----------------------------------------------
void OleComponent::CreateObjectFromData( const uno::Reference< datatransfer::XTransferable >& xTransfer,
                                         sal_Int64 nAspect )
// Static objects are not supported, they should be inserted as graphics
{
    // TODO: May be this call is useless since there are no static objects
    //       and nonstatic objects will be created based on OLEstorage ( stream ).
    //       ???

    // OleQueryCreateFromData...
}

//----------------------------------------------
void OleComponent::CreateObjectFromFile( const ::rtl::OUString& aFileName, sal_Int64 nAspect, sal_uInt32 nIconHandle )
{
    // TODO:
}

//----------------------------------------------
void OleComponent::CreateLinkFromFile( const ::rtl::OUString& aFileName, sal_Int64 nAspect, sal_uInt32 nIconHandle )
{
    // TODO:
}

//----------------------------------------------
void OleComponent::RunObject()
{
    if ( !m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    if ( !OleIsRunning( m_pOleObject ) )
    {
        HRESULT hr = OleRun( m_pObj );
        if ( FAILED( hr ) )
            throw io::IOException();
    }
}

//----------------------------------------------
void OleComponent::CloseObject()
{
    if ( !m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    if ( OleIsRunning( m_pOleObject ) )
        m_pOleObject->Close( OLECLOSE_NOSAVE ); // must be saved before
}

//----------------------------------------------
uno::Sequence< embed::VerbDescr > OleComponent::GetVerbList()
{
    if ( !m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    if( !m_aVerbList.getLength() )
    {
        CComPtr< IEnumOLEVERB > pEnum;
        if( SUCCEEDED( m_pOleObject->EnumVerbs( &pEnum ) ) )
        {
            OLEVERB     szEle[ MAX_ENUM_ELE ];
            ULONG       nNum = 0;
            sal_Int32   nSeqSize = 0;

            do
            {
                HRESULT hr = pEnum->Next( MAX_ENUM_ELE, szEle, &nNum );
                if( hr == S_OK || hr == S_FALSE )
                {
                    m_aVerbList.realloc( nSeqSize += nNum );
                    for( sal_uInt32 nInd = 0; nInd < nNum; nInd++ )
                    {
                        m_aVerbList[nSeqSize-nNum+nInd].VerbID = szEle[ nInd ].lVerb;
                        m_aVerbList[nSeqSize-nNum+nInd].VerbName = WinAccToVcl( szEle[ nInd ].lpszVerbName );
                        m_aVerbList[nSeqSize-nNum+nInd].VerbFlags = szEle[ nInd ].fuFlags;
                        m_aVerbList[nSeqSize-nNum+nInd].VerbAttributes = szEle[ nInd ].grfAttribs;
                    }
                }
                else
                    break;
            }
            while( nNum == MAX_ENUM_ELE );
        }
    }

    return m_aVerbList;
}

//----------------------------------------------
void OleComponent::ExecuteVerb( sal_Int32 nVerbID )
{
    if ( !m_pOleObject )
        throw embed::WrongStateException(); // TODO

    HRESULT hr = OleRun( m_pOleObject );
    if ( FAILED( hr ) )
        throw io::IOException(); // TODO: a specific exception that transport error code can be thrown here

    // TODO: probably extents should be set here and stored in aRect
    // TODO: probably the parent window also should be set
    hr = m_pOleObject->DoVerb( nVerbID, NULL, m_pOleWrapClientSite, 0, NULL, NULL );

    if ( FAILED( hr ) )
        throw io::IOException(); // TODO
}

//----------------------------------------------
void OleComponent::SetHostName( const ::rtl::OUString& aContName,
                                const ::rtl::OUString& aEmbDocName )
{
    if ( !m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    // TODO: use aContName and aEmbDocName in m_pOleObject->SetHostNames()
}

//----------------------------------------------
void OleComponent::SetExtent( const awt::Size& aVisAreaSize, sal_Int64 nAspect )
{
    if ( !m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    DWORD nMSAspect = (DWORD)nAspect; // first 32 bits are for MS aspects

    SIZEL aSize = { aVisAreaSize.Width, aVisAreaSize.Height };
    HRESULT hr = m_pOleObject->SetExtent( nMSAspect, &aSize );

    if ( FAILED( hr ) )
        throw io::IOException(); // TODO
}

//----------------------------------------------
awt::Size OleComponent::GetExtent( sal_Int64 nAspect )
{
    if ( !m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    DWORD nMSAspect = (DWORD)nAspect; // first 32 bits are for MS aspects
    SIZEL aSize;
    HRESULT hr = m_pOleObject->GetExtent( nMSAspect, &aSize );

    if ( FAILED( hr ) )
        throw io::IOException(); // TODO

    return awt::Size( aSize.cx, aSize.cy );
}

//----------------------------------------------
sal_Int64 OleComponent::GetMiscStatus()
{
    if ( !m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    return (sal_Int64)m_nOLEMiscFlags; // first 32 bits are for MS flags
}

//----------------------------------------------
sal_Int64 OleComponent::GetViewAspect()
{
    if ( !m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    return (sal_Int64)m_nMSAspect; // first 32 bits are for MS aspects
}

//----------------------------------------------
void OleComponent::StoreObjectToStream( uno::Reference< io::XOutputStream > xOutStream, sal_Bool bStoreVisReplace )
{
    if ( !m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    CComPtr< IPersistStorage > pPersistStorage;
    HRESULT hr = m_pObj->QueryInterface(IID_IPersistStorage, (void**)&pPersistStorage);
    if ( FAILED( hr ) || !pPersistStorage )
        throw io::IOException(); // TODO

    hr = OleSave(pPersistStorage, m_pIStorage, TRUE);
    if ( FAILED( hr ) )
        throw io::IOException(); // TODO

    pPersistStorage->SaveCompleted(NULL);
    m_pIStorage->Commit( STGC_DEFAULT );

    // now all the changes should be in temporary location

    // open temporary file for reading
    uno::Reference < ucb::XSimpleFileAccess > xTempAccess(
                    m_xFactory->createInstance (
                            ::rtl::OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" ) ),
                    uno::UNO_QUERY );

    if ( !xTempAccess.is() )
        throw uno::RuntimeException(); // TODO:

    uno::Reference< io::XInputStream > xTempInStream = xTempAccess->openFileRead( m_aTempURL );
    OSL_ENSURE( xTempInStream.is(), "The object's temporary file can not be reopened for reading!\n" );

    // TODO: use bStoreVisReplace

    if ( xTempInStream.is() )
    {
        // write all the contents to XOutStream
        copyInputToOutput_Impl( xTempInStream, xOutStream );
    }
    else
        throw io::IOException(); // TODO:

    // TODO: should the view replacement be in the stream ???
    //       probably it must be specified on storing
}

//----------------------------------------------
sal_Bool OleComponent::SaveObject_Impl()
{
    sal_Bool bResult = sal_False;
    OleEmbeddedObject* pLockObject = NULL;

    {
        osl::MutexGuard aGuard( m_aMutex );
        if ( m_pUnoOleObject )
        {
            pLockObject = m_pUnoOleObject;
            pLockObject->acquire();
        }
    }

    if ( pLockObject )
    {
        bResult = pLockObject->SaveObject_Impl();
        pLockObject->release();
    }

    return bResult;
}

//----------------------------------------------
sal_Bool OleComponent::OnShowWindow_Impl( sal_Bool bShow )
{
    sal_Bool bResult = sal_False;
    OleEmbeddedObject* pLockObject = NULL;

    {
        osl::MutexGuard aGuard( m_aMutex );
        if ( m_pUnoOleObject )
        {
            pLockObject = m_pUnoOleObject;
            pLockObject->acquire();
        }
    }

    if ( pLockObject )
    {
        bResult = m_pUnoOleObject->OnShowWindow_Impl( bShow );
        pLockObject->release();
    }

    return bResult;
}

//----------------------------------------------
void OleComponent::OnViewChange_Impl( DWORD dwAspect )
{
    // TODO: make a notification ?
    // TODO: check if it is enough or may be saving notifications are required for Visio2000
}

//----------------------------------------------
sal_Bool OleComponent::GetGraphicalCache_Impl( const datatransfer::DataFlavor& aFlavor, uno::Any& aResult )
{
    sal_Bool bOk = sal_False;
    if ( m_pIStorage )
    {
        // try to retrieve cached representation
        // TODO: in future it must be converted to requested format
        for ( sal_uInt8 nInd = 0; nInd < 10; nInd++ )
        {
            CComPtr< IStream > pGrStream;
            ::rtl::OUString aStreamName = ::rtl::OUString::createFromAscii( "\002OlePres00" );
            aStreamName += ::rtl::OUString::valueOf( nInd );
            HRESULT hr = m_pIStorage->OpenStream( aStreamName.getStr(),
                                                    NULL,
                                                    STGM_READ,
                                                    NULL,
                                                    &pGrStream );
            if ( FAILED( hr ) || !pGrStream )
                break;

            // TODO: check that the format is acceptable
            //       if so - break

            // create XInputStream ( for now SvStream ) with graphical representation
            // try to generate a Metafile or Bitmap from it
            // convert the result representation to requested format
            // if ( succeeded ) (bOk = sal_True), break;
        }
    }

    return bOk;
}

// XCloseable
//----------------------------------------------
void SAL_CALL OleComponent::close( sal_Bool bDeliverOwnership )
    throw ( util::CloseVetoException,
            uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    uno::Reference< uno::XInterface > xSelfHold( static_cast< ::cppu::OWeakObject* >( this ) );
    lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >( this ) );

    if ( m_pInterfaceContainer )
    {
        ::cppu::OInterfaceContainerHelper* pContainer =
            m_pInterfaceContainer->getContainer( ::getCppuType( ( const uno::Reference< util::XCloseListener >*) NULL ) );
        if ( pContainer != NULL )
        {
            ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
            while (pIterator.hasMoreElements())
            {
                try
                {
                    ((util::XCloseListener*)pIterator.next())->queryClosing( aSource, bDeliverOwnership );
                }
                catch( uno::RuntimeException& )
                {
                    pIterator.remove();
                }
            }
        }

        pContainer = m_pInterfaceContainer->getContainer(
                                    ::getCppuType( ( const uno::Reference< util::XCloseListener >*) NULL ) );
        if ( pContainer != NULL )
        {
            ::cppu::OInterfaceIteratorHelper pCloseIterator(*pContainer);
            while (pCloseIterator.hasMoreElements())
            {
                try
                {
                    ((util::XCloseListener*)pCloseIterator.next())->notifyClosing( aSource );
                }
                catch( uno::RuntimeException& )
                {
                    pCloseIterator.remove();
                }
            }
        }
    }

    Dispose();
}

//----------------------------------------------
void SAL_CALL OleComponent::addCloseListener( const uno::Reference< util::XCloseListener >& xListener )
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_pInterfaceContainer )
        m_pInterfaceContainer = new ::cppu::OMultiTypeInterfaceContainerHelper( m_aMutex );

    m_pInterfaceContainer->addInterface( ::getCppuType( (const uno::Reference< util::XCloseListener >*)0 ), xListener );
}

//----------------------------------------------
void SAL_CALL OleComponent::removeCloseListener( const uno::Reference< util::XCloseListener >& xListener )
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( ::getCppuType( (const uno::Reference< util::XCloseListener >*)0 ),
                                                xListener );
}

// XTransferable
//----------------------------------------------
uno::Any SAL_CALL OleComponent::getTransferData( const datatransfer::DataFlavor& aFlavor )
    throw ( datatransfer::UnsupportedFlavorException,
            io::IOException,
            uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    uno::Any aResult;
    sal_Bool bSupportedFlavor = sal_False;

    if ( GraphicalFlavor( aFlavor ) )
    {
        // first check if there is a cached representation
        if ( m_pIStorage )
        {
            // try to retrieve cached representation
            bSupportedFlavor = GetGraphicalCache_Impl( aFlavor, aResult );
        }

        if ( !bSupportedFlavor )
        {
            // TODO: retrieve MS METAFILE or bitmap and try to convert it to required format
            CComPtr< IDataObject > pDataObject;
            HRESULT hr = m_pObj->QueryInterface( IID_IDataObject, (void**)&pDataObject );
            if ( FAILED( hr ) || !pDataObject )
                throw io::IOException(); // TODO: transport error code

            if ( m_nSupportedFormat )
            {
                FORMATETC aFormatEtc = { m_nSupportedFormat, NULL, m_nMSAspect, -1, m_nSupportedMedium };
                STGMEDIUM aMedium;
                hr = pDataObject->GetData( &aFormatEtc, &aMedium );
                if ( SUCCEEDED( hr ) )
                    bSupportedFlavor = ConvertDataForFlavor( aMedium, aFlavor, aResult );
            }
            else
            {
                // the supported format of the application is still not found, find one
                sal_Int16 nFormats[3] = { CF_ENHMETAFILE, CF_METAFILEPICT, CF_BITMAP };
                sal_Int32 nMediums[3] = { TYMED_ENHMF, TYMED_MFPICT, TYMED_GDI };

                for ( sal_Int32 nInd = 0; nInd < 3; nInd++ )
                {
                    FORMATETC aFormatEtc = { nFormats[nInd], NULL, m_nMSAspect, -1, nMediums[nInd] };
                    STGMEDIUM aMedium;
                    hr = pDataObject->GetData( &aFormatEtc, &aMedium );
                    if ( SUCCEEDED( hr ) )
                    {
                        m_nSupportedFormat = nFormats[nInd];
                        m_nSupportedMedium = nMediums[nInd];

                        bSupportedFlavor = ConvertDataForFlavor( aMedium, aFlavor, aResult );
                        break;
                    }
                }
            }

            if ( !bSupportedFlavor )
            {
                // TODO: implement workaround for stampit
            }
        }
    }
    else
    {
        // TODO: allow to retrieve stream-representation of the object persistence
    }

    if ( !bSupportedFlavor )
        throw datatransfer::UnsupportedFlavorException();

    return aResult;
}

//----------------------------------------------
uno::Sequence< datatransfer::DataFlavor > SAL_CALL OleComponent::getTransferDataFlavors()
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    RetrieveObjectDataFlavors_Impl();

    return m_aDataFlavors;
}

//----------------------------------------------
sal_Bool SAL_CALL OleComponent::isDataFlavorSupported( const datatransfer::DataFlavor& aFlavor )
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    if ( !m_aDataFlavors.getLength() )
    {
        // TODO: detect format, the object writes representation in, and return reachable from it formats
        RetrieveObjectDataFlavors_Impl();
    }

    for ( sal_Int32 nInd = 0; nInd < m_aDataFlavors.getLength(); nInd++ )
        if ( m_aDataFlavors[nInd].MimeType.equals( aFlavor.MimeType ) && m_aDataFlavors[nInd].DataType == aFlavor.DataType )
            return sal_True;

    return sal_False;
}

