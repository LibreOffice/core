/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/*************************************************************************
 * @file
 * Circle object.
 ************************************************************************/
/*************************************************************************
 * Change History
 * 2005-1-17  create this file.
 ************************************************************************/
#include "lwpfilter.hxx"
#include "lwpresource.hxx"
//for sax stream
#include "xfilter/xfsaxstream.hxx"
#include "xfilter/xffilestream.hxx"
//for file parser
#include "lwp9reader.hxx"
#include "lwpsvstream.hxx"
//for container reset
#include "xfilter/xffontfactory.hxx"
#include "xfilter/xfstylemanager.hxx"

#include <osl/file.h>
#include <osl/file.hxx>
#include <vcl/svapp.hxx>
#include <xmloff/attrlist.hxx>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XText.hpp>

#include <tools/stream.hxx>
#include <sfx2/docfile.hxx>

using namespace ::cppu;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star;
using ::rtl::OUString;
using ::com::sun::star::uno::Sequence;

sal_Bool IsWordproFile( uno::Reference<XInputStream>& rInputStream);
sal_Bool IsWordproFile(rtl::OUString file);

LWPFilterReader::LWPFilterReader()
{
}

LWPFilterReader::~LWPFilterReader()
{
}

sal_Bool LWPFilterReader::filter( const Sequence< PropertyValue >& aDescriptor )
    throw( RuntimeException )
{
    uno::Reference< XInputStream> xInputStream;
    ::rtl::OUString	sURL;
    for( sal_Int32 i = 0; i < aDescriptor.getLength(); i++ )
    {
        if( aDescriptor[i].Name == OUString::createFromAscii( "InputStream" ) )
            aDescriptor[i].Value >>= xInputStream;
        if( aDescriptor[i].Name == OUString::createFromAscii( "URL" ) )
            aDescriptor[i].Value >>= sURL;
    }

    if ( !xInputStream.is() )
    {
        OSL_ASSERT( 0 );
        return sal_False;
    }

    OString sFileName;
    sFileName = ::rtl::OUStringToOString(sURL, RTL_TEXTENCODING_INFO_ASCII);

    SvFileStream inputStream( sURL, STREAM_READ );
    if ( inputStream.IsEof() || ( inputStream.GetError() != SVSTREAM_OK ) )
        return sal_False;


    return (ReadWordproFile( &inputStream ,m_DocumentHandler) == 0);
}

void LWPFilterReader::cancel() throw (com::sun::star::uno::RuntimeException)
{
}

uno::Reference< XInterface > LWPFilterImportFilter_CreateInstance(
    const uno::Reference< XMultiServiceFactory >& rSMgr ) throw( Exception )
{
    LWPFilterImportFilter *p = new LWPFilterImportFilter( rSMgr );

    return uno::Reference< XInterface > ( (OWeakObject* )p );
}

Sequence< OUString > LWPFilterImportFilter::getSupportedServiceNames_Static( void ) throw ()
{
    Sequence< OUString > aRet(1);
    aRet.getArray()[0] = LWPFilterImportFilter::getImplementationName_Static();
    return aRet;
}

LWPFilterImportFilter::LWPFilterImportFilter( const uno::Reference< XMultiServiceFactory >& xFact )
{
    //OUString sService = OUString( SwXMLImport_getImplementationName ); //STR_WRITER_IMPORTER_NAME

    try
    {
        uno::Reference< XDocumentHandler > xDoc( xFact->createInstance( OUString::createFromAscii( STR_WRITER_IMPORTER_NAME ) ), UNO_QUERY );

        LWPFilterReader *p = new LWPFilterReader;
        p->setDocumentHandler( xDoc );

        uno::Reference< XImporter > xImporter = uno::Reference< XImporter >( xDoc, UNO_QUERY );
        rImporter = xImporter;
        uno::Reference< XFilter > xFilter = uno::Reference< XFilter >( p );
        rFilter = xFilter;
    }
    catch( Exception & )
    {
       exit( 1 );
    }
}

LWPFilterImportFilter::~LWPFilterImportFilter()
{
}

sal_Bool LWPFilterImportFilter::filter( const Sequence< PropertyValue >& aDescriptor )
    throw( RuntimeException )
{
    sal_Bool ret =  rFilter->filter( aDescriptor );

    return ret;
}

void LWPFilterImportFilter::cancel() throw (::com::sun::star::uno::RuntimeException)
{
    rFilter->cancel();
}

void LWPFilterImportFilter::setTargetDocument( const uno::Reference< XComponent >& xDoc )
    throw( IllegalArgumentException, RuntimeException )
{
    rImporter->setTargetDocument( xDoc );
}

OUString LWPFilterImportFilter::getImplementationName_Static() throw()
{
    return OUString::createFromAscii( STR_IMPLEMENTATION_NAME );
}

OUString LWPFilterImportFilter::getImplementationName() throw()
{
    return OUString::createFromAscii( STR_IMPLEMENTATION_NAME );
}

sal_Bool LWPFilterImportFilter::supportsService( const OUString& ServiceName ) throw()
{
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString *pArray = aSNL.getConstArray();

    for ( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
    {
        if ( pArray[i] == ServiceName ) return sal_True;
    }

    return sal_False;
}

Sequence< OUString> LWPFilterImportFilter::getSupportedServiceNames( void ) throw()
{
    Sequence< OUString > seq(1);
    seq.getArray()[0] = OUString::createFromAscii( STR_SERVICE_NAME );
    return seq;
}


::rtl::OUString SAL_CALL LWPFilterImportFilter::detect( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
        throw (::com::sun::star::uno::RuntimeException)
{
    rtl::OUString	ret;
    rtl::OUString aTypeName;            // a name describing the type (from MediaDescriptor, usually from flat detection)
    // opening as template is done when a parameter tells to do so and a template filter can be detected
    // (otherwise no valid filter would be found) or if the detected filter is a template filter and
    // there is no parameter that forbids to open as template
    sal_Bool bOpenAsTemplate = sal_False;
    sal_Int32 nPropertyCount = aDescriptor.getLength();
     for( sal_Int32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        if( aDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("TypeName")) )
        {
            aDescriptor[nProperty].Value >>= aTypeName;
        }
        else if( aDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("AsTemplate")) )
        {
            bOpenAsTemplate = sal_True;
        }
    }

    for( sal_Int32 i = 0; i < aDescriptor.getLength(); i++ )
    {
        OUString strTemp;
        aDescriptor[i].Value >>= strTemp;
        if( aDescriptor[i].Name == OUString::createFromAscii( "InputStream" ) )
        {
            uno::Reference< XInputStream> rInputStream;
            aDescriptor[i].Value >>= rInputStream;
            //
            // TRANSFORM IMPLEMENTATION HERE!!!!!!
            // and call m_DocumentHandler's SAX mDochods
            //
            if( IsWordproFile(rInputStream) )
            {
                if( aTypeName == OUString(RTL_CONSTASCII_USTRINGPARAM("wordpro_template")) )
                {
                    if(!bOpenAsTemplate)
                    {
                        aDescriptor.realloc( nPropertyCount + 1 );
                        aDescriptor[nPropertyCount].Name = ::rtl::OUString::createFromAscii("AsTemplate");
                        aDescriptor[nPropertyCount].Value <<= sal_True;
                    }
                    return OUString::createFromAscii("wordpro_template");
                }
                else
                {
                    return OUString::createFromAscii("wordpro");
                }
            }
            return ret;
        }
        else if( aDescriptor[i].Name == OUString::createFromAscii( "URL" ) )
        {
                    OUString		sURL;
            OUString		sFileName;

                aDescriptor[i].Value >>= sURL;
            /*
            osl::FileBase::RC rc = osl::FileBase::getSystemPathFromFileURL( sURL, sFileName );
            if(rc != osl::FileBase::E_None)
            {
                SAXException except;
                except.Message = OUString( RTL_CONSTASCII_USTRINGPARAM( "GDocting system path from URL failed!"));
                throw except;
            }
            */
            //end with .lwp:
            if( IsWordproFile(sURL) )
            {
                if( aTypeName == OUString(RTL_CONSTASCII_USTRINGPARAM("wordpro_template")) )
                {
                    if(!bOpenAsTemplate)
                    {
                        aDescriptor.realloc( nPropertyCount + 1 );
                        aDescriptor[nPropertyCount].Name = ::rtl::OUString::createFromAscii("AsTemplate");
                        aDescriptor[nPropertyCount].Value <<= sal_True;
                    }
                    return OUString::createFromAscii("wordpro_template");
                }
                else
                {
                    return OUString::createFromAscii("wordpro");
                }
            }
            return ret;
        }
    }
    return ret;
}

 /**
 * @descr	decompressed small file
 * @param	pCompressed - real file stream
 * @param	pDecompressed - file decompressed, create inside, caller should delete it
 * @return	success - sal_True, fail - sal_False
 */
#include "bento.hxx"
using namespace OpenStormBento;
#include "explode.hxx"
 sal_Bool Decompress(SvStream *pCompressed, SvStream * & pDecompressed)
{
    pCompressed->Seek(0);
    pDecompressed = new SvMemoryStream(4096, 4096);
    unsigned char buffer[512];
    pCompressed->Read(buffer, 16);
    pDecompressed->Write(buffer, 16);

    LwpSvStream * pLwpStream = new LwpSvStream(pCompressed);
    LtcBenContainer* pBentoContainer;
    /*ULONG ulRet = */ BenOpenContainer(pLwpStream, &pBentoContainer);
    LtcUtBenValueStream * pWordProData = (LtcUtBenValueStream *)pBentoContainer->FindValueStreamWithPropertyName("WordProData");

    // decompressing
    Decompression decompress(pWordProData, pDecompressed);
    if (0!= decompress.explode())
    {
        delete pDecompressed;
        pDecompressed = NULL;
        delete pWordProData;
        delete pLwpStream;
        return sal_False;
    }

    sal_uInt32 nPos = pWordProData->GetSize();
    nPos += 0x10;

    pCompressed->Seek(nPos);
    while (sal_uInt32 iRead = pCompressed->Read(buffer, 512))
    {
        pDecompressed->Write(buffer, iRead);
    }

    delete pWordProData;
    delete pLwpStream;
    return sal_True;
}

 /**
 * @descr	Get LwpSvStream, if small file, both compressed/decompressed stream
 *                  Otherwise, only normal stream
 * @param	pStream - real file stream
 * @param	 LwpSvStream * , created inside, deleted outside
 * @param      sal_Bool, sal_True -
 */
 sal_Bool GetLwpSvStream(SvStream *pStream, LwpSvStream * & pLwpSvStream)
{
    SvStream * pDecompressed = NULL;

    sal_uInt32 nTag;
    pStream->Seek(0x10);
    pStream->Read(&nTag, sizeof(nTag));
    if (nTag != 0x3750574c) // "LWP7"
    {
        // small file, needs decompression
        if (!Decompress(pStream, pDecompressed))
        {
            pLwpSvStream = NULL;
            return sal_True;
        }
        pStream->Seek(0);
        pDecompressed->Seek(0);
    }

    pLwpSvStream = NULL;
    sal_Bool bCompressed = sal_False;
    if (pDecompressed)
    {
        LwpSvStream *pOriginalLwpSvStream = new LwpSvStream(pStream);
        pLwpSvStream  = new LwpSvStream(pDecompressed, pOriginalLwpSvStream);
        bCompressed = sal_True;
    }
    else
    {
        pLwpSvStream  = new LwpSvStream(pStream);
    }
    return bCompressed;
}
int ReadWordproFile(SvStream* pStream, uno::Reference<XDocumentHandler>& xHandler)
{
    try
    {
        LwpSvStream *pLwpSvStream = NULL;
        SvStream * pDecompressed = NULL;
        if ( GetLwpSvStream(pStream, pLwpSvStream) && pLwpSvStream)
        {
            pDecompressed = pLwpSvStream->GetStream();
        }
        if (!pLwpSvStream)
        {
            // nothing returned, fail when uncompressing
            return 1;
        }

        IXFStream *pStrm = new XFSaxStream(xHandler);
        Lwp9Reader reader(pLwpSvStream, pStrm);
        //Reset all static objects,because this function may be called many times.
        XFGlobalReset();
        reader.Read();

        // added by

        if (pDecompressed)
        {
            delete pDecompressed;
            LwpSvStream * pTemp = pLwpSvStream->GetCompressedStream();
            delete pTemp;
        }
        delete pLwpSvStream;
        // end added by

        delete pStrm;
        return 0;
    }
    catch (...)
    {
        return 1;
    }
    return 1;
}

void ErrorMsg(int /*iErrCode*/)
{

}

/**
 * @descr		Compare if pBuf equals with the first 16 bytes
 * @param	pBuf that contains the file data
 * @return		if equals with the Word Pro characteristic strings
 */
sal_Bool IsWordProStr(const sal_Int8 *pBuf)
{
    sal_Bool bRet = sal_True;
    const sal_Int8 pLotusLwp[] =
    {
        0x57, 0x6F, 0x72, 0x64,
        0x50, 0x72, 0x6F
    };
    for(size_t i=0; i<sizeof(pLotusLwp); ++i)
    {
        if( pBuf[i] != pLotusLwp[i] )
        {
            bRet = sal_False;
        }
    }
    return bRet;
}

sal_Bool IsWordproFile(rtl::OUString file)
{
    sal_Bool bRet = sal_False;
    SfxMedium aMedium( file, STREAM_STD_READ, FALSE);
    SvStream* pStm = aMedium.GetInStream();

    if(pStm)
    {
        sal_Int8 buf[16];
        bRet = sal_True;

        pStm->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
        pStm->Seek(STREAM_SEEK_TO_BEGIN);
        sal_Size nRead = pStm->Read(buf, sizeof(buf));
        if( nRead< sizeof(buf) )
            bRet = sal_False;
        else
            bRet = IsWordProStr(buf);
    }
    return bRet;
}


sal_Bool IsWordproFile( uno::Reference<XInputStream>& rInputStream)
{
    Sequence<sal_Int8> aData;
    sal_Bool bRet = sal_False;

    sal_Int32 nRead = rInputStream->readBytes(aData, 16);
    if( nRead != 16 )
    {
        bRet = sal_False;
    }
    else
    {
        const sal_Int8 *data = aData.getConstArray();
        bRet = IsWordProStr(data);
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
