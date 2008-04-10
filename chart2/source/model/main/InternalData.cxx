/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: InternalData.cxx,v $
 * $Revision: 1.3 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "InternalData.hxx"
#include "macros.hxx"
#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/embed/XEmbedObjectCreator.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <sot/clsids.hxx>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace
{
/** convert a class-id macro into a byte-sequence
    call e.g. lcl_GetSequenceClassID( SO3_SC_CLASSID_60 )
 */
Sequence< sal_Int8 > lcl_GetSequenceClassID( sal_uInt32 n1, sal_uInt16 n2, sal_uInt16 n3,
                                             sal_uInt8 b8, sal_uInt8 b9, sal_uInt8 b10, sal_uInt8 b11,
                                             sal_uInt8 b12, sal_uInt8 b13, sal_uInt8 b14, sal_uInt8 b15 )
{
    Sequence< sal_Int8 > aResult( 16 );
    aResult[0] = static_cast<sal_Int8>(n1 >> 24);
    aResult[1] = static_cast<sal_Int8>(( n1 << 8 ) >> 24);
    aResult[2] = static_cast<sal_Int8>(( n1 << 16 ) >> 24);
    aResult[3] = static_cast<sal_Int8>(( n1 << 24 ) >> 24);
    aResult[4] = n2 >> 8;
    aResult[5] = ( n2 << 8 ) >> 8;
    aResult[6] = n3 >> 8;
    aResult[7] = ( n3 << 8 ) >> 8;
    aResult[8] = b8;
    aResult[9] = b9;
    aResult[10] = b10;
    aResult[11] = b11;
    aResult[12] = b12;
    aResult[13] = b13;
    aResult[14] = b14;
    aResult[15] = b15;

    return aResult;
}

Reference< embed::XStorage > lcl_CreateTempStorage(
    const Reference< lang::XMultiServiceFactory > & rFactory )
{
    Reference< embed::XStorage > xResult;

    try
    {
        Reference< lang::XSingleServiceFactory > xStorageFact(
            rFactory->createInstance( C2U( "com.sun.star.embed.StorageFactory" )),
            uno::UNO_QUERY_THROW );
        xResult.set( xStorageFact->createInstance(), uno::UNO_QUERY_THROW );
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return xResult;
}

}  // anonymous namespace



namespace chart
{

//explicit
InternalData::InternalData(
    const Reference< uno::XComponentContext > & xContext,
    const Reference< embed::XStorage > & xParentStorage ) :
        m_aDataStorageName( RTL_CONSTASCII_USTRINGPARAM("ChartData")),
        m_xContext( xContext ),
        m_xParentStorage( xParentStorage )
{}

InternalData::~InternalData()
{
}

void SAL_CALL InternalData::saveObject()
    throw (embed::ObjectSaveVetoException,
           uno::Exception,
           uno::RuntimeException)
{
    if( m_xInternalData.is())
    {
        try
        {
            Reference< embed::XEmbedPersist > xPersist( m_xInternalData, uno::UNO_QUERY_THROW );
            xPersist->storeOwn();
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
}

void SAL_CALL InternalData::visibilityChanged( sal_Bool bVisible )
    throw (embed::WrongStateException,
           uno::RuntimeException)
{
    // ignore (this is for swapping OLE objects?)
}

Reference< util::XCloseable > SAL_CALL InternalData::getComponent()
    throw (uno::RuntimeException)
{
    return Reference< util::XCloseable >( m_xInternalData, uno::UNO_QUERY );
}

// ____ XCloseListener ____
void SAL_CALL InternalData::queryClosing( const lang::EventObject& Source, ::sal_Bool GetsOwnership )
    throw (util::CloseVetoException,
           uno::RuntimeException)
{
    // empty
}

void SAL_CALL InternalData::notifyClosing( const lang::EventObject& Source )
    throw (uno::RuntimeException)
{
    try
    {
        Reference< util::XCloseable > xCloseable( m_xInternalData, uno::UNO_QUERY );
        if( xCloseable.is())
            xCloseable->close( /* DeliverOwnership */ sal_False );

        Reference< lang::XComponent > xComp( m_xInternalData, uno::UNO_QUERY );
        if( xComp.is())
            xComp->dispose();
        m_xInternalData = 0;
    }
    catch( const util::CloseVetoException & )
    {
        throw;
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

    // ____ XEventListener ____
void SAL_CALL InternalData::disposing(
    const lang::EventObject& Source )
    throw (uno::RuntimeException)
{
    // empty
}

Reference< chart2::data::XDataProvider > InternalData::createEmbeddedObject() throw()
{
    OSL_ASSERT( m_xContext.is());
    Reference< chart2::data::XDataProvider > xResult;

    try
    {
        if( ! m_xInternalData.is() && m_xContext.is())
        {
            Reference< lang::XMultiServiceFactory > xFactory(
                m_xContext->getServiceManager(), uno::UNO_QUERY_THROW );
            Reference< embed::XEmbedObjectCreator > xCreator(
                xFactory->createInstance(
                    C2U( "com.sun.star.embed.EmbeddedObjectCreator")), uno::UNO_QUERY_THROW );

            Reference< embed::XStorage > xStorage( m_xParentStorage );
            if( !xStorage.is())
            {
                // as we don't store the spreadsheet as substorage in the final
                // document, it is ok, or maybe even better to create a
                // temporary storage
//                 OSL_ENSURE( false, "Using temporary storage for chart data!" );
                xStorage.set( lcl_CreateTempStorage( xFactory ));
            }

            m_xInternalData.set(
                xCreator->createInstanceInitNew(
                    lcl_GetSequenceClassID( SO3_SC_CLASSID_60 ),
                    C2U( "ChartDataEditor" ),
                    xStorage,
                    m_aDataStorageName,
                    Sequence< beans::PropertyValue >() ), uno::UNO_QUERY_THROW );

            m_xInternalData->setClientSite( this );

            xFactory.set( m_xInternalData->getComponent(), uno::UNO_QUERY_THROW );
            xResult.set(
                xFactory->createInstance( C2U( "com.sun.star.chart2.data.DataProvider" )), uno::UNO_QUERY_THROW );
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return xResult;
}

void InternalData::removeEmbeddedObject() throw()
{
    if( m_xParentStorage.is())
    {
        try
        {
            m_xParentStorage->removeElement( m_aDataStorageName );
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
}

Reference< embed::XEmbeddedObject > InternalData::getEmbeddedObject() const throw()
{
    return m_xInternalData;
}

} //  namespace chart
