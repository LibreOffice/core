/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <dumpfilter.hxx>

#include <wrtsh.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <docsh.hxx>
#include <rootfrm.hxx>
#include <unotxdoc.hxx>

#include <comphelper/servicehelper.hxx>
#include <unotools/mediadescriptor.hxx>

#include <libxml/xmlwriter.h>

using namespace ::com::sun::star;

namespace
{
    int writeCallback( void* pContext, const char* sBuffer, int nLen )
    {
        int written = nLen;

        // Actually write bytes to XOutputSream
        try
        {
            uno::XInterface* pObj = static_cast<uno::XInterface*>(pContext);
            uno::Reference< io::XOutputStream > xOut( pObj, uno::UNO_QUERY_THROW );

            // Don't output the terminating \0 to the xml or the file will be invalid
            uno::Sequence< sal_Int8 > seq( nLen );
            strncpy( reinterpret_cast<char *>(seq.getArray()), sBuffer, nLen );
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
            uno::XInterface* pObj = static_cast<uno::XInterface*>(pContext);
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

    // XFilter
    sal_Bool LayoutDumpFilter::filter( const uno::Sequence< beans::PropertyValue >& aDescriptor )
    {
        bool bRet = false;

        utl::MediaDescriptor aMediaDesc = aDescriptor;

        // Get the output stream
        uno::Reference< io::XOutputStream > xOut = aMediaDesc.getUnpackedValueOrDefault(
                utl::MediaDescriptor::PROP_OUTPUTSTREAM,
                uno::Reference< io::XOutputStream >() );

        // Actually get the SwRootFrame to call dumpAsXml
        auto pXDoc = comphelper::getFromUnoTunnel<SwXTextDocument>(m_xSrcDoc);
        if ( pXDoc )
        {
            SwRootFrame* pLayout = pXDoc->GetDocShell()->GetWrtShell()->GetLayout();

            // Get sure that the whole layout is processed: set a visible area
            // even though there isn't any need of it
            pXDoc->GetDocShell()->GetWrtShell()->StartAction();
            tools::Rectangle aRect( 0, 0, 26000, 21000 );
            pXDoc->GetDocShell()->SetVisArea( aRect );
            pLayout->InvalidateAllContent( SwInvalidateFlags::Size );
            pXDoc->GetDocShell()->GetWrtShell()->EndAction();

            // Dump the layout XML into the XOutputStream
            xmlOutputBufferPtr outBuffer = xmlOutputBufferCreateIO(
                    writeCallback, closeCallback, static_cast<void*>(xOut.get()), nullptr );

            xmlTextWriterPtr writer = xmlNewTextWriter( outBuffer );
            xmlTextWriterSetIndent(writer, 1);
            (void)xmlTextWriterStartDocument( writer, nullptr, nullptr, nullptr );

            // TODO This doesn't export the whole XML file, whereas dumpAsXML() does it nicely
            pLayout->dumpAsXml( writer );

            (void)xmlTextWriterEndDocument( writer );
            xmlFreeTextWriter( writer );

            bRet = true;
        }

        return bRet;
    }

    void LayoutDumpFilter::cancel(  )
    {
    }

    // XExporter
    void LayoutDumpFilter::setSourceDocument( const uno::Reference< lang::XComponent >& xDoc )
    {
        m_xSrcDoc = xDoc;
    }

    // XInitialization
    void LayoutDumpFilter::initialize( const uno::Sequence< uno::Any >& )
    {
    }

    // XServiceInfo
    OUString LayoutDumpFilter::getImplementationName(  )
    {
        return u"com.sun.star.comp.Writer.LayoutDump"_ustr;
    }

    sal_Bool LayoutDumpFilter::supportsService( const OUString& rServiceName )
    {
        return cppu::supportsService(this, rServiceName);
    }

    uno::Sequence< OUString > LayoutDumpFilter::getSupportedServiceNames()
    {
        return { u"com.sun.star.document.ExportFilter"_ustr };
    }

} // Namespace sw


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Writer_LayoutDump_get_implementation(css::uno::XComponentContext*,
                                css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new sw::LayoutDumpFilter());
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
