/*************************************************************************
 *
 *  $RCSfile: xmlgrhlp.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: ka $
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

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_CVTGRF_HXX
#include <vcl/cvtgrf.hxx>
#endif
#ifndef _SV_GFXLINK_HXX
#include <vcl/gfxlink.hxx>
#endif
#ifndef _GRFMGR_HXX
#include <goodies/grfmgr.hxx>
#endif

#include "impgrf.hxx"
#include "xmlgrhlp.hxx"

// -----------
// - Defines -
// -----------

#define XML_GRAPHICSTORAGE_NAME "Pictures"

// ----------------------
// - SvXMLGraphicHelper -
// ----------------------

SvXMLGraphicHelper::SvXMLGraphicHelper() :
    ::cppu::WeakComponentImplHelper1< ::com::sun::star::container::XIndexContainer >( maMutex )
{
}

// -----------------------------------------------------------------------------

SvXMLGraphicHelper::~SvXMLGraphicHelper()
{
}

// -----------------------------------------------------------------------------

SvStorageRef SvXMLGraphicHelper::ImplGetGraphicStorage()
{
    static const String aGraphicStorageName( RTL_CONSTASCII_USTRINGPARAM( XML_GRAPHICSTORAGE_NAME ) );

    if( !mxGraphicStorage.Is() )
        mxGraphicStorage = mpRootStorage->OpenStorage( aGraphicStorageName, STREAM_READ | STREAM_WRITE );

    return mxGraphicStorage;
}

// -----------------------------------------------------------------------------

SvStorageStreamRef SvXMLGraphicHelper::ImplGetGraphicStream( const ::rtl::OUString& rURLStr, BOOL bTruncate )
{
    SvStorageStreamRef  xStm;
    SvStorageRef        xStorage( ImplGetGraphicStorage() );

    if( xStorage.Is() )
    {
        const String    aURLStr( rURLStr );
        const String    aId( aURLStr.GetToken( aURLStr.GetTokenCount( ':' ) - 1, ':' ) );

        xStm = xStorage->OpenStream( aId, STREAM_READ | STREAM_WRITE | ( bTruncate ? STREAM_TRUNC : 0 ) );
    }

    return xStm;
}

// -----------------------------------------------------------------------------

void SvXMLGraphicHelper::ImplWriteGraphic( const ::rtl::OUString& rURLStr )
{
    const String        aURLStr( rURLStr );
    const ByteString    aId( aURLStr.GetToken( aURLStr.GetTokenCount( ':' ) - 1, ':' ),
                             RTL_TEXTENCODING_ASCII_US );
    GraphicObject       aGrfObject( aId );

    if( aGrfObject.GetType() != GRAPHIC_NONE )
    {
        SvStorageStreamRef xStm( ImplGetGraphicStream( rURLStr, TRUE ) );

        if( xStm.Is() )
        {
            Graphic         aGraphic( (Graphic&) aGrfObject.GetGraphic() );
            const GfxLink   aGfxLink( aGraphic.GetLink() );

            if( aGfxLink.GetDataSize() )
                xStm->Write( aGfxLink.GetData(), aGfxLink.GetDataSize() );
            else
            {
                if( aGraphic.GetType() == GRAPHIC_BITMAP )
                {
                    GraphicFilter*  pFilter = GetGrfFilter();
                    String          aFormat;

                    if( aGraphic.IsAnimated() )
                        aFormat = String( RTL_CONSTASCII_USTRINGPARAM( "GIF" ) );
                    else
                        aFormat = String( RTL_CONSTASCII_USTRINGPARAM( "PNG" ) );

                    pFilter->ExportGraphic( aGraphic, String(), *xStm,
                                            pFilter->GetExportFormatNumberForShortName( aFormat ) );
                }
                else if( aGraphic.GetType() == GRAPHIC_GDIMETAFILE )
                    ( (GDIMetaFile&) aGraphic.GetGDIMetaFile() ).Write( *xStm );
            }
        }
    }
}

// -----------------------------------------------------------------------------

void SvXMLGraphicHelper::ImplInsertGraphicURL( const ::rtl::OUString& rURLStr )
{
    if( mbDirect && ( maSet.find( rURLStr ) == maSet.end() ) )
        ImplWriteGraphic( rURLStr );

    maSet.insert( rURLStr );
}

// -----------------------------------------------------------------------------

void SvXMLGraphicHelper::Init( SvStorage& rXMLStorage,
                               SvXMLGraphicHelperMode eCreateMode,
                               BOOL bDirect )
{
    mpRootStorage = &rXMLStorage;
    meCreateMode = eCreateMode;
    mbDirect = bDirect;
}

// -----------------------------------------------------------------------------

SvXMLGraphicHelper* SvXMLGraphicHelper::Create( SvStorage& rXMLStorage,
                                                SvXMLGraphicHelperMode eCreateMode,
                                                BOOL bDirect )
{
    SvXMLGraphicHelper* pThis = new SvXMLGraphicHelper;

    pThis->acquire();
    pThis->Init( rXMLStorage, eCreateMode, bDirect );

    return pThis;
}

// -----------------------------------------------------------------------------

void SvXMLGraphicHelper::Destroy( SvXMLGraphicHelper* pSvXMLGraphicHelper )
{
    if( pSvXMLGraphicHelper )
    {
        pSvXMLGraphicHelper->Flush();
        pSvXMLGraphicHelper->release();
    }
}

// -----------------------------------------------------------------------------

void SvXMLGraphicHelper::Flush()
{
    if( !mbDirect )
    {
        ::_STL::set< ::rtl::OUString >::iterator aIter( maSet.begin() ), aEnd( maSet.end() );

        while( aIter != aEnd )
            ImplWriteGraphic( *aIter++ );
    }
}

// -----------------------------------------------------------------------------

void SAL_CALL SvXMLGraphicHelper::insertByIndex( sal_Int32 nIndex, const ::com::sun::star::uno::Any& rElement )
    throw(  ::com::sun::star::lang::IllegalArgumentException,
            ::com::sun::star::lang::IndexOutOfBoundsException,
            ::com::sun::star::lang::WrappedTargetException,
            ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard   aGuard( maMutex );

    ::rtl::OUString aStr;

    rElement >>= aStr;

    if( aStr.getLength() )
    {
        if( nIndex == (sal_Int32) maVector.size() )
            maVector.push_back( aStr );
        else
        {
             if( nIndex > (sal_Int32) maVector.size() )
                maVector.resize( nIndex + 1 );

            maVector[ nIndex ] = aStr;
        }

        ImplInsertGraphicURL( aStr );
    }
}

// -----------------------------------------------------------------------------

void SAL_CALL SvXMLGraphicHelper::removeByIndex( sal_Int32 nIndex )
    throw(  ::com::sun::star::lang::IndexOutOfBoundsException,
            ::com::sun::star::lang::WrappedTargetException,
            ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( maMutex );

    DBG_ASSERT( nIndex < getCount(), "invalid Index" );

    maVector.erase( maVector.begin() + nIndex );
}

// -----------------------------------------------------------------------------

void SAL_CALL SvXMLGraphicHelper::replaceByIndex( sal_Int32 nIndex, const ::com::sun::star::uno::Any& rElement )
    throw(  ::com::sun::star::lang::IllegalArgumentException,
            ::com::sun::star::lang::IndexOutOfBoundsException,
            ::com::sun::star::lang::WrappedTargetException,
            ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( maMutex );

    DBG_ASSERT( nIndex < getCount(), "invalid Index" );

    ::rtl::OUString aStr;

    rElement >>= aStr;

    if( aStr.getLength() )
    {
        maVector[ nIndex ] = aStr;
        ImplInsertGraphicURL( aStr );
    }
}

// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL SvXMLGraphicHelper::getCount()
    throw(  ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( maMutex );
    return maVector.size();
}

// -----------------------------------------------------------------------------

::com::sun::star::uno::Any SAL_CALL SvXMLGraphicHelper::getByIndex( sal_Int32 nIndex )
    throw(  ::com::sun::star::lang::IndexOutOfBoundsException,
            ::com::sun::star::lang::WrappedTargetException,
            ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( maMutex );

    DBG_ASSERT( nIndex < getCount(), "invalid Index" );

    ::com::sun::star::uno::Any aAny;

    aAny <<= maVector[ nIndex ];

    return aAny;
}

// -----------------------------------------------------------------------------

::com::sun::star::uno::Type SAL_CALL SvXMLGraphicHelper::getElementType()
    throw(  ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( maMutex );
    return ::getCppuType( (const ::rtl::OUString*) 0 );
}

// -----------------------------------------------------------------------------

sal_Bool SAL_CALL SvXMLGraphicHelper::hasElements()
    throw(  ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( maMutex );
    return maVector.size() > 0;
}
