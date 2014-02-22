/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include "dumpfilter.hxx"

#include <wrtsh.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <docsh.hxx>
#include <rootfrm.hxx>
#include <unotxdoc.hxx>

#include <unotools/mediadescriptor.hxx>

#include <libxml/xmlwriter.h>

using namespace ::com::sun::star;

OUString SAL_CALL LayoutDumpFilter_getImplementationName() throw( uno::RuntimeException )
{
    return OUString( "com.sun.star.comp.Writer.LayoutDump" );
}

uno::Sequence< OUString > SAL_CALL LayoutDumpFilter_getSupportedServiceNames() throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aSeq( 1 );
    aSeq[0] = "com.sun.star.document.ExportFilter";
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL LayoutDumpFilter_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & )
{
    return static_cast< cppu::OWeakObject* >( new sw::LayoutDumpFilter( ) );
}

namespace
{
    int writeCallback( void* pContext, const char* sBuffer, int nLen )
    {
        int written = nLen;

        
        try
        {
            uno::XInterface* pObj = ( uno::XInterface* )pContext;
            uno::Reference< io::XOutputStream > xOut( pObj, uno::UNO_QUERY_THROW );

            
            uno::Sequence< sal_Int8 > seq( nLen );
            strncpy( ( char * ) seq.getArray() , sBuffer, nLen );
            xOut->writeBytes( seq );
        }
        catch (const uno::Exception&)
        {
            written = -1;
        }

        return written;
    }

    int closeCallback( void* pContext )
    {
        int result = 0;
        try
        {
            uno::XInterface* pObj = ( uno::XInterface* )pContext;
            uno::Reference< io::XOutputStream > xOut( pObj, uno::UNO_QUERY_THROW );
            xOut->closeOutput( );
        }
        catch (const uno::Exception&)
        {
            result = -1;
        }
        return result;
    }
}

namespace sw
{

    LayoutDumpFilter::LayoutDumpFilter( )
    {
    }

    LayoutDumpFilter::~LayoutDumpFilter( )
    {
    }

    
    sal_Bool LayoutDumpFilter::filter( const uno::Sequence< beans::PropertyValue >& aDescriptor )
        throw (uno::RuntimeException)
    {
        sal_Bool bRet = sal_False;

        utl::MediaDescriptor aMediaDesc = aDescriptor;

        
        uno::Reference< io::XOutputStream > xOut = aMediaDesc.getUnpackedValueOrDefault(
                utl::MediaDescriptor::PROP_OUTPUTSTREAM(),
                uno::Reference< io::XOutputStream >() );

        
        uno::Reference< lang::XUnoTunnel > xDocTunnel( m_xSrcDoc, uno::UNO_QUERY );
        SwXTextDocument* pXDoc = UnoTunnelGetImplementation< SwXTextDocument >( xDocTunnel );
        if ( pXDoc )
        {
            SwRootFrm* pLayout = pXDoc->GetDocShell()->GetWrtShell()->GetLayout();

            
            
            pXDoc->GetDocShell()->GetWrtShell()->StartAction();
            Rectangle aRect( 0, 0, 26000, 21000 );
            pXDoc->GetDocShell()->SetVisArea( aRect );
            pLayout->InvalidateAllCntnt( );
            pXDoc->GetDocShell()->GetWrtShell()->EndAction();

            
            xmlOutputBufferPtr outBuffer = xmlOutputBufferCreateIO(
                    writeCallback, closeCallback, ( void* ) xOut.get(), NULL );

            xmlTextWriterPtr writer = xmlNewTextWriter( outBuffer );
            xmlTextWriterSetIndent(writer, 1);
            xmlTextWriterStartDocument( writer, NULL, NULL, NULL );

            
            pLayout->dumpAsXml( writer );

            xmlTextWriterEndDocument( writer );
            xmlFreeTextWriter( writer );

            bRet = sal_True;
        }

        return bRet;
    }

    void LayoutDumpFilter::cancel(  ) throw (uno::RuntimeException)
    {
    }

    
    void LayoutDumpFilter::setSourceDocument( const uno::Reference< lang::XComponent >& xDoc )
        throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        m_xSrcDoc = xDoc;
    }

    
    void LayoutDumpFilter::initialize( const uno::Sequence< uno::Any >& )
        throw (uno::Exception, uno::RuntimeException)
    {
    }

    
    OUString LayoutDumpFilter::getImplementationName(  )
        throw (uno::RuntimeException)
    {
        return LayoutDumpFilter_getImplementationName();
    }

    sal_Bool LayoutDumpFilter::supportsService( const OUString& rServiceName )
        throw (uno::RuntimeException)
    {
        return cppu::supportsService(this, rServiceName);
    }

    uno::Sequence< OUString > LayoutDumpFilter::getSupportedServiceNames()
        throw (uno::RuntimeException)
    {
        return LayoutDumpFilter_getSupportedServiceNames();
    }

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
