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
#include "lwpfilter.hxx"
#include "lwpresource.hxx"
//for sax stream
#include "xfilter/xfsaxstream.hxx"
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

#include <cppuhelper/supportsservice.hxx>

#include <tools/stream.hxx>
#include <sfx2/docfile.hxx>

#include <boost/scoped_ptr.hpp>

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
using ::com::sun::star::uno::Sequence;

sal_Bool IsWordproFile( uno::Reference<XInputStream>& rInputStream);
sal_Bool IsWordproFile(OUString file);

LWPFilterReader::LWPFilterReader()
{
}

LWPFilterReader::~LWPFilterReader()
{
}

sal_Bool LWPFilterReader::filter( const Sequence< PropertyValue >& aDescriptor )
    throw( RuntimeException, std::exception )
{
    OUString sURL;
    for( sal_Int32 i = 0; i < aDescriptor.getLength(); i++ )
    {
        //Note we should attempt to use "InputStream" if it exists first!
        if ( aDescriptor[i].Name == "URL" )
            aDescriptor[i].Value >>= sURL;
    }

    SvFileStream inputStream( sURL, STREAM_READ );
    if ( inputStream.IsEof() || ( inputStream.GetError() != SVSTREAM_OK ) )
        return sal_False;

    return (ReadWordproFile(inputStream, m_DocumentHandler) == 0);
}

void LWPFilterReader::cancel() throw (com::sun::star::uno::RuntimeException, std::exception)
{
}

uno::Reference< XInterface > LWPFilterImportFilter_CreateInstance(
    const uno::Reference< XMultiServiceFactory >& rSMgr ) throw( Exception )
{
    LWPFilterImportFilter *p = new LWPFilterImportFilter( rSMgr );

    return uno::Reference< XInterface > ( (OWeakObject* )p );
}

LWPFilterImportFilter::LWPFilterImportFilter( const uno::Reference< XMultiServiceFactory >& xFact )
{
    try
    {
        uno::Reference< XDocumentHandler > xDoc( xFact->createInstance( OUString( STR_WRITER_IMPORTER_NAME ) ), UNO_QUERY );

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
    throw( RuntimeException, std::exception )
{
    sal_Bool ret =  rFilter->filter( aDescriptor );

    return ret;
}

void LWPFilterImportFilter::cancel() throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    rFilter->cancel();
}

void LWPFilterImportFilter::setTargetDocument( const uno::Reference< XComponent >& xDoc )
    throw( IllegalArgumentException, RuntimeException, std::exception )
{
    rImporter->setTargetDocument( xDoc );
}

OUString LWPFilterImportFilter::getImplementationName() throw(std::exception)
{
    return OUString( STR_IMPLEMENTATION_NAME );
}

sal_Bool LWPFilterImportFilter::supportsService( const OUString& ServiceName ) throw(std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString> LWPFilterImportFilter::getSupportedServiceNames( void ) throw(std::exception)
{
    Sequence< OUString > seq(1);
    seq[0] = STR_SERVICE_NAME;
    return seq;
}

OUString SAL_CALL LWPFilterImportFilter::detect( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
        throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    OUString   ret;
    OUString aTypeName;            // a name describing the type (from MediaDescriptor, usually from flat detection)
    // opening as template is done when a parameter tells to do so and a template filter can be detected
    // (otherwise no valid filter would be found) or if the detected filter is a template filter and
    // there is no parameter that forbids to open as template
    sal_Bool bOpenAsTemplate = sal_False;
    sal_Int32 nPropertyCount = aDescriptor.getLength();
     for( sal_Int32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        if ( aDescriptor[nProperty].Name == "TypeName" )
        {
            aDescriptor[nProperty].Value >>= aTypeName;
        }
        else if ( aDescriptor[nProperty].Name == "AsTemplate" )
        {
            bOpenAsTemplate = sal_True;
        }
    }

    for( sal_Int32 i = 0; i < aDescriptor.getLength(); i++ )
    {
        OUString strTemp;
        aDescriptor[i].Value >>= strTemp;
        if ( aDescriptor[i].Name == "InputStream" )
        {
            uno::Reference< XInputStream> rInputStream;
            aDescriptor[i].Value >>= rInputStream;

            // TODO TRANSFORM IMPLEMENTATION HERE!!!!!!
            // and call m_DocumentHandler's SAX mDochods

            if( IsWordproFile(rInputStream) )
            {
                if ( aTypeName == "wordpro_template" )
                {
                    if(!bOpenAsTemplate)
                    {
                        aDescriptor.realloc( nPropertyCount + 1 );
                        aDescriptor[nPropertyCount].Name = "AsTemplate";
                        aDescriptor[nPropertyCount].Value <<= sal_True;
                    }
                    return OUString("wordpro_template");
                }
                else
                {
                    return OUString("wordpro");
                }
            }
            return ret;
        }
        else if ( aDescriptor[i].Name == "URL" )
        {
                    OUString        sURL;

                aDescriptor[i].Value >>= sURL;
            /*
            osl::FileBase::RC rc = osl::FileBase::getSystemPathFromFileURL( sURL, sFileName );
            if(rc != osl::FileBase::E_None)
            {
                SAXException except;
                except.Message = "GDocting system path from URL failed!";
                throw except;
            }
            */
            //end with .lwp:
            if( IsWordproFile(sURL) )
            {
                if ( aTypeName == "wordpro_template" )
                {
                    if(!bOpenAsTemplate)
                    {
                        aDescriptor.realloc( nPropertyCount + 1 );
                        aDescriptor[nPropertyCount].Name = "AsTemplate";
                        aDescriptor[nPropertyCount].Value <<= sal_True;
                    }
                    return OUString("wordpro_template");
                }
                else
                {
                    return OUString("wordpro");
                }
            }
            return ret;
        }
    }
    return ret;
}

 /**
 * @descr   decompressed small file
 * @param   pCompressed - real file stream
 * @param   pDecompressed - file decompressed, create inside, caller should delete it
 * @return  success - sal_True, fail - sal_False
 */
#include "bento.hxx"
using namespace OpenStormBento;
#include "explode.hxx"
 sal_Bool Decompress(SvStream *pCompressed, SvStream * & pOutDecompressed)
{
    pCompressed->Seek(0);
    std::auto_ptr<SvStream> aDecompressed(new SvMemoryStream(4096, 4096));
    unsigned char buffer[512];
    pCompressed->Read(buffer, 16);
    aDecompressed->Write(buffer, 16);

    boost::scoped_ptr<LwpSvStream> aLwpStream(new LwpSvStream(pCompressed));
    LtcBenContainer* pBentoContainer;
    sal_uLong ulRet = BenOpenContainer(aLwpStream.get(), &pBentoContainer);
    if (ulRet != BenErr_OK)
        return sal_False;

    boost::scoped_ptr<LtcUtBenValueStream> aWordProData((LtcUtBenValueStream *)pBentoContainer->FindValueStreamWithPropertyName("WordProData"));

    if (!aWordProData.get())
        return sal_False;

    // decompressing
    Decompression decompress(aWordProData.get(), aDecompressed.get());
    if (0!= decompress.explode())
        return sal_False;

    sal_uInt32 nPos = aWordProData->GetSize();
    nPos += 0x10;

    pCompressed->Seek(nPos);
    while (sal_uInt32 iRead = pCompressed->Read(buffer, 512))
        aDecompressed->Write(buffer, iRead);

    //transfer ownership of aDecompressed's ptr
    pOutDecompressed = aDecompressed.release();
    return sal_True;
}

 /**
 * @descr   Get LwpSvStream, if small file, both compressed/decompressed stream
 *                  Otherwise, only normal stream
 * @param   pStream - real file stream
 * @param    LwpSvStream * , created inside, deleted outside
 * @param      sal_Bool, sal_True -
 */
 sal_Bool GetLwpSvStream(SvStream *pStream, LwpSvStream * & pLwpSvStream)
{
    SvStream * pDecompressed = NULL;

    sal_uInt32 nTag;
    pStream->Seek(0x10);
    pStream->ReadUInt32( nTag );
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
int ReadWordproFile(SvStream &rStream, uno::Reference<XDocumentHandler>& xHandler)
{
    try
    {
        LwpSvStream *pRawLwpSvStream = NULL;
        boost::scoped_ptr<LwpSvStream> aLwpSvStream;
        boost::scoped_ptr<LwpSvStream> aCompressedLwpSvStream;
        boost::scoped_ptr<SvStream> aDecompressed;
        if (GetLwpSvStream(&rStream, pRawLwpSvStream) && pRawLwpSvStream)
        {
            SvStream *pDecompressed = pRawLwpSvStream->GetStream();
            if (pDecompressed)
            {
                aDecompressed.reset(pDecompressed);
                aCompressedLwpSvStream.reset(pRawLwpSvStream->GetCompressedStream());
            }
        }

        if (!pRawLwpSvStream)
        {
            // nothing returned, fail when uncompressing
            return 1;
        }

        aLwpSvStream.reset(pRawLwpSvStream);

        boost::scoped_ptr<IXFStream> pStrm(new XFSaxStream(xHandler));
        Lwp9Reader reader(aLwpSvStream.get(), pStrm.get());
        //Reset all static objects,because this function may be called many times.
        XFGlobalReset();
        reader.Read();

        return 0;
    }
    catch (...)
    {
        return 1;
    }
}

void ErrorMsg(int /*iErrCode*/)
{

}

/**
 * @descr       Compare if pBuf equals with the first 16 bytes
 * @param   pBuf that contains the file data
 * @return      if equals with the Word Pro characteristic strings
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

sal_Bool IsWordproFile(OUString file)
{
    sal_Bool bRet = sal_False;
    SfxMedium aMedium( file, STREAM_STD_READ);
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
