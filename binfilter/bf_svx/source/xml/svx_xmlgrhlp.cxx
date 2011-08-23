/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/tempfile.hxx>
#include <cppuhelper/implbase1.hxx>	// helper for implementations
#include <sal/macros.h>

#include "impgrf.hxx"
#include "xmlgrhlp.hxx"
namespace binfilter {

// -----------
// - Defines -
// -----------

using namespace rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;

#define XML_GRAPHICSTORAGE_NAME		"Pictures"
#define XML_PACKAGE_URL_BASE		"vnd.sun.star.Package:"
#define XML_GRAPHICOBJECT_URL_BASE	"vnd.sun.star.GraphicObject:"

// ---------------------------
// - SvXMLGraphicInputStream -
// ---------------------------

class SvXMLGraphicInputStream : public::cppu::WeakImplHelper1< XInputStream >
{
private:
    
    virtual sal_Int32	SAL_CALL	readBytes( Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead) throw(NotConnectedException, BufferSizeExceededException, RuntimeException);
    virtual sal_Int32	SAL_CALL	readSomeBytes(Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead) throw(NotConnectedException, BufferSizeExceededException, RuntimeException);
    virtual void		SAL_CALL	skipBytes(sal_Int32 nBytesToSkip) throw(NotConnectedException, BufferSizeExceededException, RuntimeException);
    virtual sal_Int32	SAL_CALL	available() throw(NotConnectedException, RuntimeException);
    virtual void		SAL_CALL	closeInput() throw(NotConnectedException, RuntimeException);

private:

    ::utl::TempFile                 maTmp;
    Reference< XInputStream >       mxStmWrapper;

                                    // not available
                                    SvXMLGraphicInputStream();
                                    SvXMLGraphicInputStream( const SvXMLGraphicInputStream& );
    SvXMLGraphicInputStream&        operator==( SvXMLGraphicInputStream& );
                                            
public:                                     
                                        
                                    SvXMLGraphicInputStream( const OUString& rGraphicId );
    virtual                         ~SvXMLGraphicInputStream();

    sal_Bool                        Exists() const { return mxStmWrapper.is(); }
};                                      

// -----------------------------------------------------------------------------

/*N*/ SvXMLGraphicInputStream::SvXMLGraphicInputStream( const OUString& rGraphicId )
/*N*/ {
/*N*/ 	String			aGraphicId( rGraphicId );
/*N*/ 	BfGraphicObject aGrfObject( ByteString( aGraphicId, RTL_TEXTENCODING_ASCII_US ) );
/*N*/ 
/*N*/     maTmp.EnableKillingFile();
/*N*/ 
/*N*/ 	if( aGrfObject.GetType() != GRAPHIC_NONE )
/*N*/ 	{
/*N*/         SvStream* pStm = ::utl::UcbStreamHelper::CreateStream( maTmp.GetURL(), STREAM_WRITE | STREAM_TRUNC );
/*N*/ 
/*N*/         if( pStm )
/*N*/         {
/*N*/ 			Graphic			aGraphic( (Graphic&) aGrfObject.GetGraphic() );
/*N*/ 			const GfxLink	aGfxLink( aGraphic.GetLink() );
/*N*/             sal_Bool        bRet = sal_False;
/*N*/ 
/*N*/ 			if( aGfxLink.GetDataSize() )
/*N*/             {
/*N*/ 				pStm->Write( aGfxLink.GetData(), aGfxLink.GetDataSize() );
/*N*/ 			    bRet = ( pStm->GetError() == 0 );
/*N*/             }
/*N*/ 			else
/*N*/ 			{
/*N*/ 				if( aGraphic.GetType() == GRAPHIC_BITMAP )
/*N*/ 				{
/*N*/ 					GraphicFilter*  pFilter = GetGrfFilter();
/*N*/                     String          aFormat;
/*N*/ 
/*N*/ 					if( aGraphic.IsAnimated() )
/*N*/ 						aFormat = String( RTL_CONSTASCII_USTRINGPARAM( "gif" ) );
/*N*/ 					else
/*N*/ 						aFormat = String( RTL_CONSTASCII_USTRINGPARAM( "png" ) );
/*N*/ 
/*N*/ 					bRet = ( pFilter->ExportGraphic( aGraphic, String(), *pStm, pFilter->GetExportFormatNumberForShortName( aFormat ) ) == 0 );
/*N*/ 				}
/*N*/ 				else if( aGraphic.GetType() == GRAPHIC_GDIMETAFILE )
/*N*/ 				{
/*N*/ 					( (GDIMetaFile&) aGraphic.GetGDIMetaFile() ).Write( *pStm );
/*N*/     			    bRet = ( pStm->GetError() == 0 );
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/             if( bRet )
/*N*/             {
/*N*/                 pStm->Seek( 0 );
/*N*/                 mxStmWrapper = new ::utl::OInputStreamWrapper( pStm, sal_True );
/*N*/             }
/*N*/             else
/*N*/                 delete pStm;
/*N*/         }
/*N*/     }
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ SvXMLGraphicInputStream::~SvXMLGraphicInputStream()
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------------
                                        
/*N*/ sal_Int32 SAL_CALL SvXMLGraphicInputStream::readBytes( Sequence< sal_Int8 >& rData, sal_Int32 nBytesToRead ) 
/*N*/     throw( NotConnectedException, BufferSizeExceededException, RuntimeException )
/*N*/ {
/*N*/     if( !mxStmWrapper.is() )
/*N*/         throw NotConnectedException();
/*N*/ 
/*N*/     return mxStmWrapper->readBytes( rData, nBytesToRead );
/*N*/ }

// -----------------------------------------------------------------------------

/*?*/ sal_Int32 SAL_CALL SvXMLGraphicInputStream::readSomeBytes( Sequence< sal_Int8 >& rData, sal_Int32 nMaxBytesToRead )
/*?*/     throw( NotConnectedException, BufferSizeExceededException, RuntimeException )
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001     if( !mxStmWrapper.is() )
/*?*/ }

// -----------------------------------------------------------------------------

/*?*/ void SAL_CALL SvXMLGraphicInputStream::skipBytes( sal_Int32 nBytesToSkip ) 
/*?*/     throw( NotConnectedException, BufferSizeExceededException, RuntimeException )
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); //STRIP001     if( !mxStmWrapper.is() )
/*?*/ }

// -----------------------------------------------------------------------------

/*?*/ sal_Int32 SAL_CALL SvXMLGraphicInputStream::available() throw( NotConnectedException, RuntimeException )
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001     if( !mxStmWrapper.is() )
/*?*/ }

// -----------------------------------------------------------------------------

/*?*/ void SAL_CALL SvXMLGraphicInputStream::closeInput() throw( NotConnectedException, RuntimeException )
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); //STRIP001     if( !mxStmWrapper.is() )
/*?*/ }

// ----------------------------
// - SvXMLGraphicOutputStream -
// ----------------------------

class SvXMLGraphicOutputStream : public::cppu::WeakImplHelper1< XOutputStream >
{
private:
    
    // XOutputStream
    virtual void SAL_CALL           writeBytes( const Sequence< sal_Int8 >& rData ) throw( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException );
    virtual void SAL_CALL           flush() throw( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException );
    virtual void SAL_CALL           closeOutput() throw( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException );

private:

    ::utl::TempFile*                mpTmp;
    SvStream*                       mpOStm;
    Reference< XOutputStream >      mxStmWrapper;
    BfGraphicObject maGrfObj;
    sal_Bool                        mbClosed;

                                    // not available
                                    SvXMLGraphicOutputStream( const SvXMLGraphicOutputStream& );
    SvXMLGraphicOutputStream&       operator==( SvXMLGraphicOutputStream& );
                                            
public:                                     
                                        
                                    SvXMLGraphicOutputStream();
    virtual                         ~SvXMLGraphicOutputStream();

    sal_Bool                        Exists() const { return mxStmWrapper.is(); }
    const BfGraphicObject&            GetGraphicObject();
};                                      

// -----------------------------------------------------------------------------

/*N*/ SvXMLGraphicOutputStream::SvXMLGraphicOutputStream() :
/*N*/     mpTmp( new ::utl::TempFile ),
/*N*/     mbClosed( sal_False )
/*N*/ {
/*N*/     mpTmp->EnableKillingFile();
/*N*/ 
/*N*/     mpOStm = ::utl::UcbStreamHelper::CreateStream( mpTmp->GetURL(), STREAM_WRITE | STREAM_TRUNC );
/*N*/ 
/*N*/     if( mpOStm )
/*N*/         mxStmWrapper = new ::utl::OOutputStreamWrapper( *mpOStm );
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ SvXMLGraphicOutputStream::~SvXMLGraphicOutputStream()
/*N*/ {
/*N*/     delete mpTmp;
/*N*/     delete mpOStm;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SAL_CALL SvXMLGraphicOutputStream::writeBytes( const Sequence< sal_Int8 >& rData ) 
/*N*/     throw( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException )
/*N*/ {
/*N*/     if( !mxStmWrapper.is() )
/*N*/         throw NotConnectedException() ;
/*N*/ 
/*N*/     mxStmWrapper->writeBytes( rData );
/*N*/ }

// -----------------------------------------------------------------------------

/*?*/ void SAL_CALL SvXMLGraphicOutputStream::flush() 
/*?*/     throw( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException )
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); //STRIP001 //STRIP001     if( !mxStmWrapper.is() )
/*?*/ }

// -----------------------------------------------------------------------------

/*N*/ void SAL_CALL SvXMLGraphicOutputStream::closeOutput() 
/*N*/     throw( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException )
/*N*/ {
/*N*/     if( !mxStmWrapper.is() )
/*N*/         throw NotConnectedException() ;
/*N*/ 
/*N*/     mxStmWrapper->closeOutput();
/*N*/     mxStmWrapper = Reference< XOutputStream >();
/*N*/ 
/*N*/     mbClosed = sal_True;
/*N*/ }

// ------------------------------------------------------------------------------

/*N*/ const BfGraphicObject& SvXMLGraphicOutputStream::GetGraphicObject()
/*N*/ {
/*N*/     if( mbClosed && ( maGrfObj.GetType() == GRAPHIC_NONE ) && mpOStm )
/*N*/     {
/*N*/     	Graphic aGraphic;
/*N*/ 
/*N*/         mpOStm->Seek( 0 );
/*N*/         GetGrfFilter()->ImportGraphic( aGraphic, String(), *mpOStm );
/*N*/         
/*N*/         maGrfObj = BfGraphicObject(aGraphic);
/*N*/ 
/*N*/         if( maGrfObj.GetType() != GRAPHIC_NONE )
/*N*/         {
/*N*/             delete mpOStm, mpOStm = NULL;
/*N*/             delete mpTmp, mpTmp = NULL;
/*N*/         }
/*N*/     }
/*N*/ 
/*N*/     return maGrfObj;
/*N*/ }

// ----------------------
// - SvXMLGraphicHelper -
// ----------------------

/*N*/ SvXMLGraphicHelper::SvXMLGraphicHelper( SvXMLGraphicHelperMode eCreateMode ) :
/*N*/ 	::cppu::WeakComponentImplHelper2< ::com::sun::star::document::XGraphicObjectResolver,
/*N*/                                       ::com::sun::star::document::XBinaryStreamResolver >( maMutex )
/*N*/ {
/*N*/ 	Init( NULL, eCreateMode, sal_False );
/*N*/ }

/*N*/ SvXMLGraphicHelper::SvXMLGraphicHelper() :
/*N*/ 	::cppu::WeakComponentImplHelper2< ::com::sun::star::document::XGraphicObjectResolver,
/*N*/                                       ::com::sun::star::document::XBinaryStreamResolver >( maMutex )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ SvXMLGraphicHelper::~SvXMLGraphicHelper()
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SAL_CALL SvXMLGraphicHelper::disposing()
/*N*/ {
/*N*/ 	Flush();
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ sal_Bool SvXMLGraphicHelper::ImplGetStreamNames( const ::rtl::OUString& rURLStr, 
/*N*/ 												 ::rtl::OUString& rPictureStorageName, 
/*N*/ 												 ::rtl::OUString& rPictureStreamName )
/*N*/ {
/*N*/ 	String		aURLStr( rURLStr );
/*N*/ 	sal_Bool	bRet = sal_False;
/*N*/ 
/*N*/ 	if( aURLStr.Len() )
/*N*/ 	{
/*N*/ 		aURLStr = aURLStr.GetToken( aURLStr.GetTokenCount( ':' ) - 1, ':' );
/*N*/ 		const sal_uInt32 nTokenCount = aURLStr.GetTokenCount( '/' );
/*N*/ 
/*N*/ 		if( 1 == nTokenCount )
/*N*/ 		{
/*N*/ 			rPictureStorageName = String( RTL_CONSTASCII_USTRINGPARAM( XML_GRAPHICSTORAGE_NAME ) );
/*N*/ 			rPictureStreamName = aURLStr;
/*N*/ 			bRet = sal_True; 
/*N*/ 		}
/*N*/ 		else if( 2 == nTokenCount )
/*N*/ 		{
/*N*/ 			rPictureStorageName = aURLStr.GetToken( 0, '/' );
/*N*/ 			
/*N*/ 			if( rPictureStorageName.getLength() && rPictureStorageName.getStr()[ 0 ] == '#' )
/*?*/ 				rPictureStorageName = rPictureStorageName.copy( 1 );
/*N*/ 
/*N*/ 			rPictureStreamName = aURLStr.GetToken( 1, '/' );
/*N*/ 			bRet = sal_True; 
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			DBG_ERROR( "SvXMLGraphicHelper::ImplInsertGraphicURL: invalid scheme" );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return bRet;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ SotStorageRef SvXMLGraphicHelper::ImplGetGraphicStorage( const ::rtl::OUString& rStorageName )
/*N*/ {
/*N*/ 	if( mpRootStorage && ( !mxGraphicStorage.Is() || ( rStorageName != maCurStorageName ) ) )
/*N*/ 	{
/*N*/ 		if( mxGraphicStorage.Is() && GRAPHICHELPER_MODE_WRITE == meCreateMode )
/*?*/ 			mxGraphicStorage->Commit();
/*N*/ 
/*N*/ 		mxGraphicStorage = mpRootStorage->OpenUCBStorage( maCurStorageName = rStorageName, STREAM_READ | STREAM_WRITE );
/*N*/ 	}
/*N*/ 
/*N*/ 	return mxGraphicStorage;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ SotStorageStreamRef SvXMLGraphicHelper::ImplGetGraphicStream( const ::rtl::OUString& rPictureStorageName,
/*N*/ 															  const ::rtl::OUString& rPictureStreamName,
/*N*/ 															  BOOL bTruncate )
/*N*/ {
/*N*/ 	SotStorageStreamRef	xStm;
/*N*/ 	SotStorageRef		xStorage( ImplGetGraphicStorage( rPictureStorageName ) ); 
/*N*/ 
/*N*/ 	if( xStorage.Is() )
/*N*/ 	{
/*N*/ 		xStm = xStorage->OpenSotStream( rPictureStreamName, 
/*N*/ 									    STREAM_READ | ( ( GRAPHICHELPER_MODE_WRITE == meCreateMode ) ? 
/*N*/                                         ( STREAM_WRITE | ( bTruncate ? STREAM_TRUNC : 0 ) ) : 0 ) );
/*N*/ 		
/*N*/         if( xStm.Is() && ( GRAPHICHELPER_MODE_WRITE == meCreateMode ) )
/*N*/ 		{
/*N*/ 			OUString aPropName( RTL_CONSTASCII_USTRINGPARAM("Encrypted") );
/*N*/ 			sal_Bool bTrue = sal_True;
/*N*/ 			uno::Any aAny;
/*N*/ 
/*N*/ 			aAny.setValue( &bTrue, ::getBooleanCppuType() );
/*N*/ 			xStm->SetProperty( aPropName, aAny );
/*N*/ 
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return xStm;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ String SvXMLGraphicHelper::ImplGetGraphicMimeType( const String& rFileName ) const
/*N*/ {
/*N*/     struct XMLGraphicMimeTypeMapper 
/*N*/     {
/*N*/ 	    const char*	pExt;
/*N*/ 	    const char*	pMimeType;
/*N*/     };
/*N*/     
/*N*/     static XMLGraphicMimeTypeMapper aMapper[] =
/*N*/     {
/*N*/         { "gif", "image/gif" },
/*N*/         { "png", "image/png" },
/*N*/         { "jpg", "image/jpeg" },
/*N*/         { "tif", "image/tiff" }
/*N*/     };
/*N*/ 
/*N*/     String aMimeType;
/*N*/ 
/*N*/     if( ( rFileName.Len() >= 4 ) && ( rFileName.GetChar( rFileName.Len() - 4 ) == '.' ) )
/*N*/     {
/*N*/         const ByteString aExt( rFileName.Copy( rFileName.Len() - 3 ), RTL_TEXTENCODING_ASCII_US );
/*N*/ 
/*N*/         for( long i = 0, nCount = SAL_N_ELEMENTS( aMapper ); ( i < nCount ) && !aMimeType.Len(); i++ )
/*N*/             if( aExt == aMapper[ i ].pExt )
/*N*/                 aMimeType = String( aMapper[ i ].pMimeType, RTL_TEXTENCODING_ASCII_US );
/*N*/     }
/*N*/ 
/*N*/     return aMimeType;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ Graphic SvXMLGraphicHelper::ImplReadGraphic( const ::rtl::OUString& rPictureStorageName, 
/*N*/ 											 const ::rtl::OUString& rPictureStreamName )
/*N*/ {
/*N*/ 	Graphic				aGraphic;
/*N*/ 	SotStorageStreamRef	xStm( ImplGetGraphicStream( rPictureStorageName, rPictureStreamName, FALSE ) );
/*N*/ 
/*N*/ 	if( xStm.Is() )
/*N*/ 		GetGrfFilter()->ImportGraphic( aGraphic, String(), *xStm );
/*N*/ 
/*N*/ 	return aGraphic;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ sal_Bool SvXMLGraphicHelper::ImplWriteGraphic( const ::rtl::OUString& rPictureStorageName, 
/*N*/ 											   const ::rtl::OUString& rPictureStreamName,
/*N*/ 											   const ::rtl::OUString& rGraphicId )
/*N*/ {
/*N*/ 	String			aGraphicId( rGraphicId );
/*N*/ 	BfGraphicObject aGrfObject( ByteString( aGraphicId, RTL_TEXTENCODING_ASCII_US ) );
/*N*/ 	sal_Bool		bRet = sal_False;
/*N*/ 
/*N*/ 	if( aGrfObject.GetType() != GRAPHIC_NONE )
/*N*/ 	{
/*N*/ 		SotStorageStreamRef xStm( ImplGetGraphicStream( rPictureStorageName, rPictureStreamName, TRUE ) );
/*N*/ 
/*N*/ 		if( xStm.Is() )
/*N*/ 		{
/*N*/ 			Graphic			aGraphic( (Graphic&) aGrfObject.GetGraphic() );
/*N*/ 			const GfxLink	aGfxLink( aGraphic.GetLink() );
/*N*/             const OUString  aMimeType( ImplGetGraphicMimeType( rPictureStreamName ) );
/*N*/             uno::Any        aAny;
/*N*/ 
/*N*/             // set stream properties (MediaType/Compression)
/*N*/             if( aMimeType.getLength() )
/*N*/             {
/*N*/ 	            aAny <<= aMimeType;
/*N*/ 	            xStm->SetProperty( String( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) ), aAny );
/*N*/             }
/*N*/ 
/*N*/             const sal_Bool bCompressed = ( ( 0 == aMimeType.getLength() ) || ( aMimeType == OUString::createFromAscii( "image/tiff" ) ) );
/*N*/             aAny <<= bCompressed;
/*N*/             xStm->SetProperty( String( RTL_CONSTASCII_USTRINGPARAM( "Compressed" ) ), aAny );
/*N*/ 
/*N*/ 			if( aGfxLink.GetDataSize() )
/*N*/ 				xStm->Write( aGfxLink.GetData(), aGfxLink.GetDataSize() );
/*N*/ 			else
/*N*/ 			{
/*N*/ 				if( aGraphic.GetType() == GRAPHIC_BITMAP )
/*N*/ 				{
/*?*/ 					GraphicFilter*  pFilter = GetGrfFilter();
/*?*/                     String          aFormat;
/*?*/ 
/*?*/ 					if( aGraphic.IsAnimated() )
/*?*/ 						aFormat = String( RTL_CONSTASCII_USTRINGPARAM( "gif" ) );
/*?*/ 					else
/*?*/ 						aFormat = String( RTL_CONSTASCII_USTRINGPARAM( "png" ) );
/*?*/ 
/*?*/ 					bRet = ( pFilter->ExportGraphic( aGraphic, String(), *xStm,
/*N*/ 													 pFilter->GetExportFormatNumberForShortName( aFormat ) ) == 0 );
/*N*/ 				}
/*N*/ 				else if( aGraphic.GetType() == GRAPHIC_GDIMETAFILE )
/*N*/ 				{
/*N*/ 					( (GDIMetaFile&) aGraphic.GetGDIMetaFile() ).Write( *xStm );
/*N*/ 					bRet = ( xStm->GetError() == 0 );
/*N*/ 				}
/*N*/ 			}
/*N*/ 			
/*N*/ 			xStm->Commit();
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return bRet;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SvXMLGraphicHelper::ImplInsertGraphicURL( const ::rtl::OUString& rURLStr, sal_uInt32 nInsertPos )
/*N*/ {
/*N*/ 	::rtl::OUString	aPictureStorageName, aPictureStreamName;
/*N*/ 
/*N*/ 	if( ( maURLSet.find( rURLStr ) != maURLSet.end() ) )
/*N*/ 	{
/*N*/ 		URLPairVector::iterator aIter( maGrfURLs.begin() ), aEnd( maGrfURLs.end() );
/*N*/ 
/*N*/ 		while( aIter != aEnd )
/*N*/ 		{
/*N*/ 			if( rURLStr == (*aIter).first )
/*N*/ 			{
/*N*/ 				maGrfURLs[ nInsertPos ].second = (*aIter).second;
/*N*/ 				aIter = aEnd;
/*N*/ 			}
/*N*/ 			else
/*N*/ 				aIter++;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if( ImplGetStreamNames( rURLStr, aPictureStorageName, aPictureStreamName ) )
/*N*/ 	{
/*N*/ 		URLPair& rURLPair = maGrfURLs[ nInsertPos ];
/*N*/ 
/*N*/ 		if( GRAPHICHELPER_MODE_READ == meCreateMode )
/*N*/ 		{
/*N*/ 			const BfGraphicObject aObj( ImplReadGraphic( aPictureStorageName, aPictureStreamName ) );
/*N*/ 
/*N*/ 			if( aObj.GetType() != GRAPHIC_NONE )
/*N*/ 			{
/*N*/ 				const static ::rtl::OUString aBaseURL( RTL_CONSTASCII_USTRINGPARAM( XML_GRAPHICOBJECT_URL_BASE ) );
/*N*/ 
/*N*/ 				maGrfObjs.push_back( aObj );
/*N*/ 				rURLPair.second = aBaseURL;
/*N*/ 				rURLPair.second += String( aObj.GetUniqueID().GetBuffer(), RTL_TEXTENCODING_ASCII_US );
/*N*/ 			}
/*N*/ 			else
/*?*/ 				rURLPair.second = String();
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			const String		aGraphicObjectId( aPictureStreamName );
/*N*/ 			const BfGraphicObject aGrfObject( ByteString( aGraphicObjectId, RTL_TEXTENCODING_ASCII_US ) );
/*N*/ 
/*N*/ 			if( aGrfObject.GetType() != GRAPHIC_NONE )
/*N*/ 			{
/*N*/ 				String			aStreamName( aGraphicObjectId );
/*N*/ 				Graphic			aGraphic( (Graphic&) aGrfObject.GetGraphic() );
/*N*/ 				const GfxLink	aGfxLink( aGraphic.GetLink() );
/*N*/ 
/*N*/ 				if( aGfxLink.GetDataSize() )
/*N*/ 				{
/*N*/ 					switch( aGfxLink.GetType() )
/*N*/ 					{
/*N*/ 						case( GFX_LINK_TYPE_EPS_BUFFER ): aStreamName += String( RTL_CONSTASCII_USTRINGPARAM( ".eps" ) ); break;
/*N*/ 						case( GFX_LINK_TYPE_NATIVE_GIF ): aStreamName += String( RTL_CONSTASCII_USTRINGPARAM( ".gif" ) ); break;
/*N*/ 						case( GFX_LINK_TYPE_NATIVE_JPG ): aStreamName += String( RTL_CONSTASCII_USTRINGPARAM( ".jpg" ) ); break;
/*N*/ 						case( GFX_LINK_TYPE_NATIVE_PNG ): aStreamName += String( RTL_CONSTASCII_USTRINGPARAM( ".png" ) ); break;
/*N*/ 						case( GFX_LINK_TYPE_NATIVE_TIF ): aStreamName += String( RTL_CONSTASCII_USTRINGPARAM( ".tif" ) ); break;
/*N*/ 						case( GFX_LINK_TYPE_NATIVE_WMF ): aStreamName += String( RTL_CONSTASCII_USTRINGPARAM( ".wmf" ) ); break;
/*N*/ 						case( GFX_LINK_TYPE_NATIVE_MET ): aStreamName += String( RTL_CONSTASCII_USTRINGPARAM( ".met" ) ); break;
/*N*/ 						case( GFX_LINK_TYPE_NATIVE_PCT ): aStreamName += String( RTL_CONSTASCII_USTRINGPARAM( ".pct" ) ); break;
/*N*/ 
/*N*/ 						default:
/*N*/ 							aStreamName += String( RTL_CONSTASCII_USTRINGPARAM( ".grf" ) );
/*N*/ 						break;
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					if( aGrfObject.GetType() == GRAPHIC_BITMAP )
/*N*/ 					{
/*N*/ 						if( aGrfObject.IsAnimated() )
/*N*/ 							aStreamName += String( RTL_CONSTASCII_USTRINGPARAM( ".gif" ) );
/*N*/ 						else
/*N*/ 							aStreamName += String( RTL_CONSTASCII_USTRINGPARAM( ".png" ) );
/*N*/ 					}
/*N*/ 					else if( aGrfObject.GetType() == GRAPHIC_GDIMETAFILE )
/*N*/ 						aStreamName += String( RTL_CONSTASCII_USTRINGPARAM( ".svm" ) );
/*N*/ 				}
/*N*/ 
/*N*/ 				if( mbDirect && aStreamName.Len() )
/*N*/ 					ImplWriteGraphic( aPictureStorageName, aStreamName, aGraphicObjectId );
/*N*/ 
/*N*/ 				rURLPair.second = String( RTL_CONSTASCII_USTRINGPARAM( "#Pictures/" ) );
/*N*/ 				rURLPair.second += aStreamName;
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		maURLSet.insert( rURLStr );
/*N*/ 	}
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SvXMLGraphicHelper::Init( SotStorage* pXMLStorage, 
/*N*/ 							   SvXMLGraphicHelperMode eCreateMode,
/*N*/ 							   BOOL bDirect )
/*N*/ {
/*N*/ 	mpRootStorage = pXMLStorage;
/*N*/ 	meCreateMode = eCreateMode;
/*N*/ 	mbDirect = ( ( GRAPHICHELPER_MODE_READ == meCreateMode ) ? bDirect : sal_True );
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ SvXMLGraphicHelper* SvXMLGraphicHelper::Create( SotStorage& rXMLStorage, 
/*N*/ 												SvXMLGraphicHelperMode eCreateMode,
/*N*/ 												BOOL bDirect )
/*N*/ {
/*N*/ 	SvXMLGraphicHelper* pThis = new SvXMLGraphicHelper;
/*N*/ 
/*N*/ 	pThis->acquire();
/*N*/ 	pThis->Init( &rXMLStorage, eCreateMode, bDirect );
/*N*/ 
/*N*/ 	return pThis;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ SvXMLGraphicHelper*	SvXMLGraphicHelper::Create( SvXMLGraphicHelperMode eCreateMode )
/*N*/ {
/*N*/ 	SvXMLGraphicHelper* pThis = new SvXMLGraphicHelper;
/*N*/ 
/*N*/ 	pThis->acquire();
/*N*/ 	pThis->Init( NULL, eCreateMode, sal_False );
/*N*/ 
/*N*/ 	return pThis;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SvXMLGraphicHelper::Destroy( SvXMLGraphicHelper* pSvXMLGraphicHelper )
/*N*/ {
/*N*/ 	if( pSvXMLGraphicHelper )
/*N*/ 	{
/*N*/ 		pSvXMLGraphicHelper->dispose();
/*N*/ 		pSvXMLGraphicHelper->release();
/*N*/ 	}
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SvXMLGraphicHelper::Flush()
/*N*/ {
/*N*/ 	if( ( GRAPHICHELPER_MODE_WRITE == meCreateMode ) && !mbDirect )
/*N*/ 	{
/*?*/ 		::rtl::OUString		aPictureStorageName, aPictureStreamName;
/*?*/ 		URLSet::iterator	aSetIter( maURLSet.begin() ), aSetEnd( maURLSet.end() );
/*?*/ 
/*?*/ 		while( aSetIter != aSetEnd )
/*?*/ 		{
/*?*/ 			URLPairVector::iterator aPairIter( maGrfURLs.begin() ), aPairEnd( maGrfURLs.end() );
/*?*/ 
/*?*/ 			while( aPairIter != aPairEnd )
/*?*/ 			{
/*?*/ 				if( *aSetIter == (*aPairIter).first )
/*?*/ 				{
/*?*/ 					if( ImplGetStreamNames( (*aPairIter).second, aPictureStorageName, aPictureStreamName ) )
/*?*/ 					{
/*?*/ 						DBG_ASSERT( String( aPictureStreamName ).GetTokenCount( '.' ) == 2, "invalid URL" );
/*?*/ 						ImplWriteGraphic( aPictureStorageName, aPictureStreamName, String( aPictureStreamName ).GetToken( 0, '.' ) );
/*?*/ 					}
/*?*/ 					
/*?*/ 					aPairIter = aPairEnd;
/*?*/ 				}
/*?*/ 				else
/*?*/ 					aPairIter++;
/*?*/ 			}
/*?*/ 
/*?*/ 			aSetIter++;
/*?*/ 		}
/*?*/ 
/*?*/ 		mbDirect = sal_True;
/*N*/ 	}
/*N*/ 	if( GRAPHICHELPER_MODE_WRITE == meCreateMode )
/*N*/ 	{
/*N*/ 		SotStorageRef xStorage = ImplGetGraphicStorage( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(XML_GRAPHICSTORAGE_NAME) ) );
/*N*/ 
/*N*/ 		if( xStorage.Is() )
/*N*/ 			xStorage->Commit();
/*N*/ 	}
/*N*/ 
/*N*/ }

// -----------------------------------------------------------------------------

// XGraphicObjectResolver
/*N*/ OUString SAL_CALL SvXMLGraphicHelper::resolveGraphicObjectURL( const OUString& aURL )
/*N*/ 	throw(uno::RuntimeException)
/*N*/ {
/*N*/ 	::osl::MutexGuard   aGuard( maMutex );
/*N*/ 	const sal_Int32     nIndex = maGrfURLs.size();
/*N*/ 	
/*N*/     maGrfURLs.push_back( ::std::make_pair( aURL, ::rtl::OUString() ) );
/*N*/ 	ImplInsertGraphicURL( aURL, nIndex );
/*N*/ 	
/*N*/     return maGrfURLs[ nIndex ].second;
/*N*/ }

// -----------------------------------------------------------------------------

// XBinaryStreamResolver
/*N*/ Reference< XInputStream > SAL_CALL SvXMLGraphicHelper::getInputStream( const OUString& rURL ) 
/*N*/     throw( RuntimeException )
/*N*/ {
/*N*/     Reference< XInputStream >   xRet;
/*N*/     OUString                    aPictureStorageName, aGraphicId;
/*N*/ 
/*N*/ 
/*N*/ 	if( ( GRAPHICHELPER_MODE_WRITE == meCreateMode ) && 
/*N*/         ImplGetStreamNames( rURL, aPictureStorageName, aGraphicId ) )
/*N*/     {
/*N*/         SvXMLGraphicInputStream* pInputStream = new SvXMLGraphicInputStream( aGraphicId );
/*N*/ 
/*N*/         if( pInputStream->Exists() )
/*N*/             xRet = pInputStream;
/*N*/         else
/*N*/             delete pInputStream;
/*N*/     }
/*N*/ 
/*N*/     return xRet;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ Reference< XOutputStream > SAL_CALL SvXMLGraphicHelper::createOutputStream() 
/*N*/     throw( RuntimeException )
/*N*/ {
/*N*/     Reference< XOutputStream > xRet;
/*N*/ 
/*N*/ 	if( GRAPHICHELPER_MODE_READ == meCreateMode )
/*N*/     {
/*N*/         SvXMLGraphicOutputStream* pOutputStream = new SvXMLGraphicOutputStream;
/*N*/ 
/*N*/         if( pOutputStream->Exists() )
/*N*/             maGrfStms.push_back( xRet = pOutputStream );
/*N*/         else
/*?*/             delete pOutputStream;
/*N*/     }
/*N*/ 
/*N*/     return xRet;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ OUString SAL_CALL SvXMLGraphicHelper::resolveOutputStream( const Reference< XOutputStream >& rxBinaryStream )
/*N*/     throw( RuntimeException )
/*N*/ {
/*N*/     OUString aRet;
/*N*/ 
/*N*/ 	if( ( GRAPHICHELPER_MODE_READ == meCreateMode ) && rxBinaryStream.is() )
/*N*/     {
/*N*/         if( ::std::find( maGrfStms.begin(), maGrfStms.end(), rxBinaryStream ) != maGrfStms.end() )
/*N*/         {
/*N*/             SvXMLGraphicOutputStream* pOStm = static_cast< SvXMLGraphicOutputStream* >( rxBinaryStream.get() );
/*N*/ 
/*N*/             if( pOStm )
/*N*/             {
/*N*/                 const BfGraphicObject&    rGrfObj = pOStm->GetGraphicObject();
/*N*/                 const OUString          aId( OUString::createFromAscii( rGrfObj.GetUniqueID().GetBuffer() ) );
/*N*/ 
/*N*/                 if( aId.getLength() )
/*N*/                 {
/*N*/                     aRet = OUString::createFromAscii( XML_GRAPHICOBJECT_URL_BASE );
/*N*/                     aRet += aId;
/*N*/                 }
/*N*/             }
/*N*/         }
/*N*/     }
/*N*/ 
/*N*/     return aRet;
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
