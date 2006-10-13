/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: graphconvert.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-13 11:30:34 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_embeddedobj.hxx"

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_EMBED_ASPECTS_HPP_
#include <com/sun/star/embed/Aspects.hpp>
#endif

#include "mtnotification.hxx"
#include "oleembobj.hxx"

// TODO: when conversion service is ready this headers should disappear
#include <svtools/filter.hxx>
#include <vcl/graph.hxx>

#include <tools/link.hxx>
#include <vcl/svapp.hxx>
#include <vos/mutex.hxx>


using namespace ::com::sun::star;


sal_Bool ConvertBufferToFormat( void* pBuf,
                                sal_uInt32 nBufSize,
                                const ::rtl::OUString& aFormatShortName,
                                uno::Any& aResult )
{
    // produces sequence with data in requested format and returns it in aResult
    if ( pBuf )
    {
        SvMemoryStream aBufStream( pBuf, nBufSize, STREAM_READ );
        aBufStream.ObjectOwnsMemory( sal_False );

        Graphic aGraph;
        GraphicFilter aGrFilter( sal_True );
        if ( aGrFilter.ImportGraphic( aGraph, String(), aBufStream ) == ERRCODE_NONE )
        {
            sal_uInt16 nFormat = aGrFilter.GetExportFormatNumberForShortName( aFormatShortName );

            if ( nFormat != GRFILTER_FORMAT_DONTKNOW )
            {
                SvMemoryStream aNewStream( 65535, 65535 );
                if ( aGrFilter.ExportGraphic( aGraph, String(), aNewStream, nFormat ) == ERRCODE_NONE )
                {
                    /*
                    {
                        aNewStream.Seek( 0 );
                        SvFileStream aFile( String::CreateFromAscii( "file:///d:/test.png" ), STREAM_STD_READWRITE);
                        aFile.SetStreamSize( 0 );
                        aNewStream >> aFile;
                    }
                    */

                    aResult <<= uno::Sequence< sal_Int8 >(
                                                    reinterpret_cast< const sal_Int8* >( aNewStream.GetData() ),
                                                    aNewStream.Seek( STREAM_SEEK_TO_END ) );
                    return sal_True;
                }
            }
        }
    }

    return sal_False;
}

// =====================================================================
// MainThreadNotificationRequest
// =====================================================================

MainThreadNotificationRequest::MainThreadNotificationRequest( OleEmbeddedObject* pObj, sal_uInt32 nAspect )
: m_pObject( pObj )
, m_xObject( static_cast< embed::XEmbeddedObject* >( pObj ) )
, m_nAspect( nAspect )
{}

void MainThreadNotificationRequest::mainThreadWorkerStart( MainThreadNotificationRequest* pMTRequest )
{
    if ( Application::GetMainThreadIdentifier() == osl_getThreadIdentifier( NULL ) )
    {
        // this is the main thread
        worker( pMTRequest, pMTRequest );
    }
    else
        Application::PostUserEvent( STATIC_LINK( NULL, MainThreadNotificationRequest, worker ), pMTRequest );
}

IMPL_STATIC_LINK_NOINSTANCE( MainThreadNotificationRequest, worker, MainThreadNotificationRequest*, pMTRequest )
{
    if ( pMTRequest )
    {
        if ( pMTRequest->m_pObject )
        {
            try
            {
                uno::Reference< uno::XInterface > xLock = pMTRequest->m_xObject.get();
                if ( xLock.is() )
                {
                    // this is the main thread, the solar mutex must be locked
                    ::vos::OGuard aGuard( Application::GetSolarMutex() );
                    if ( pMTRequest->m_nAspect == embed::Aspects::MSOLE_CONTENT )
                        pMTRequest->m_pObject->OnViewChanged_Impl();
                    else if ( pMTRequest->m_nAspect == embed::Aspects::MSOLE_ICON )
                        pMTRequest->m_pObject->OnIconChanged_Impl();
                }
            }
            catch( uno::Exception& )
            {
                // ignore all the errors
            }
        }

        delete pMTRequest;
    }

    return 0;
}

// =====================================================================

