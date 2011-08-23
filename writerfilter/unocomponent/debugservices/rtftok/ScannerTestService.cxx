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

#include "ScannerTestService.hxx"
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <rtftok/RTFScanner.hxx>
#include <rtftok/RTFScannerHandler.hxx>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <ucbhelper/contentbroker.hxx>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <osl/process.h>
#include <rtl/string.hxx>
#include <hash_set>
#include <assert.h>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <comphelper/storagehelper.hxx>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/seqstream.hxx>

#include <ctype.h>

using namespace ::com::sun::star;

namespace writerfilter { namespace rtftok {

const sal_Char ScannerTestService::SERVICE_NAME[40] = "debugservices.rtftok.ScannerTestService";
const sal_Char ScannerTestService::IMPLEMENTATION_NAME[40] = "debugservices.rtftok.ScannerTestService";

struct ScannerTestServiceHelper
{
    size_t operator()(const rtl::OString &str) const
    {
        return str.hashCode();
    }
    bool operator()(const rtl::OString &str1, const rtl::OString &str2) const
    {
        return str1.compareTo(str2)==0;
    }
};

typedef ::std::hash_set< ::rtl::OString, ScannerTestServiceHelper, ScannerTestServiceHelper > ScannerTestServiceTokenMap;

class MyRtfScannerHandler : public writerfilter::rtftok::RTFScannerHandler
{
    ScannerTestServiceTokenMap destMap;
    ScannerTestServiceTokenMap ctrlMap;
    std::vector<unsigned char> binBuffer;
    int objDataLevel;
    int numOfOLEs;
    unsigned char hb;
    int numOfOLEChars;
    uno::Reference<lang::XMultiServiceFactory> xServiceFactory;
    uno::Reference<com::sun::star::ucb::XSimpleFileAccess> xFileAccess;
    uno::Reference<embed::XStorage> xStorage;

    void dest(char* token, char* /*value*/)
    {
        destMap.insert(rtl::OString(token));
//		printf("{\\*\\%s%s ", token, value);
        if (strcmp(token, "objdata")==0)
        {
            binBuffer.clear();
            objDataLevel=1;
            numOfOLEChars=0;
        }
    }
    void ctrl(char*token, char* /*value*/)
    {
        ctrlMap.insert(rtl::OString(token));
//		printf("\\%s%s ", token, value);
    }
    void lbrace(void)
    {
//		printf("{");
    }
    void rbrace(void)
    {
#ifndef LINUX
        unsigned char * binBufferStr = ((unsigned char*)&(*binBuffer.begin()));

        if (objDataLevel)
        {
            int o=0;
            unsigned int type=((unsigned int)binBuffer[o]) | ((unsigned int)binBuffer[o+1])<<8 | ((unsigned int)binBuffer[o+2])<<16 | ((unsigned int)binBuffer[o+3]<<24); o+=4;
            unsigned int recType=((unsigned int)binBuffer[o]) | ((unsigned int)binBuffer[o+1])<<8 | ((unsigned int)binBuffer[o+2])<<16 | ((unsigned int)binBuffer[o+3]<<24); o+=4;
            unsigned int strLen=((unsigned int)binBuffer[o]) | ((unsigned int)binBuffer[o+1])<<8 | ((unsigned int)binBuffer[o+2])<<16 | ((unsigned int)binBuffer[o+3]<<24); o+=4;
            unsigned char *str=binBufferStr+o;
            o+=strLen;
            o+=4; // dummy1
            o+=4; // dummy2
            unsigned int binLen=((unsigned int)binBuffer[o]) | ((unsigned int)binBuffer[o+1])<<8 | ((unsigned int)binBuffer[o+2])<<16 | ((unsigned int)binBuffer[o+3]<<24); o+=4;
            printf("OLE%i \"%s\" type=%i recType=%i binBuffer.size()=%u len=%u\n", numOfOLEs, str, type, recType, (unsigned int)(binBuffer.size()), o+binLen);
            //assert(binBuffer.size()==o+binLen);
            char buf[100];
            sprintf(buf, "ole%02i.ole", numOfOLEs);
/*			if 0{
            FILE *f=fopen(buf, "w+b");
            unsigned char *data=binBuffer.begin();
            fwrite(data+o, 1, binLen, f);
            fclose(f);
            }*/
/*
            rtl_uString *dir=NULL;
            osl_getProcessWorkingDir(&dir);
            rtl::OUString absFileUrl;
            rtl::OUString fileUrl=rtl::OUString::createFromAscii(buf);
            osl_getAbsoluteFileURL(dir, fileUrl.pData, &absFileUrl.pData);
            rtl_uString_release(dir);
*/
                comphelper::ByteSequence seq(binLen);
                unsigned char *data0=binBufferStr;
                memcpy(seq.getArray(), data0+o, binLen);
                uno::Reference<io::XInputStream> myStream=new comphelper::SequenceInputStream(seq);
//			uno::Reference<io::XStream> myStream=xFileAccess->openFileReadWrite(absFileUrl);
//			uno::Reference<io::XStream> myStream(new MyStreamImpl(binBuffer, o));
            uno::Sequence< uno::Any > aArgs0( 1 );
            aArgs0[0] <<= myStream;
            uno::Reference< container::XNameContainer > xNameContainer(
            xServiceFactory->createInstanceWithArguments(
                    ::rtl::OUString::createFromAscii("com.sun.star.embed.OLESimpleStorage" ),
                    aArgs0 ),
            uno::UNO_QUERY_THROW );
            try {
                printf("TRY\n");
        ::com::sun::star::uno::Sequence< ::rtl::OUString > names=xNameContainer->getElementNames();
                printf("OK\n");

            for(int i=0;i<names.getLength();i++)
            {
                rtl::OUString &name=names[i];
                wprintf(L"name=%s\n", name.getStr());
            }
            {
                uno::Reference< io::XStream > xContentStream = xStorage->openStreamElement(
                    rtl::OUString::createFromAscii(buf), embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE );
                uno::Reference<beans::XPropertySet> xContentStreamPropSet(xContentStream, uno::UNO_QUERY_THROW);
                xContentStreamPropSet->setPropertyValue(rtl::OUString::createFromAscii("MediaType"), uno::makeAny(rtl::OUString::createFromAscii("application/vnd.sun.star.oleobject")));
                uno::Reference<io::XOutputStream> myOutStream=xContentStream->getOutputStream();
                uno::Sequence< ::sal_Int8 > seq1(binLen);
                unsigned char *data1=binBufferStr;
                memcpy(seq1.getArray(), data1+o, binLen);
                myOutStream->writeBytes(seq1);
                myOutStream->closeOutput();
            }

            } catch(com::sun::star::uno::RuntimeException &)
            {
                printf("NOT OK\n");
                comphelper::ByteSequence seq2(4+binLen);
                //				memcpy(seq2.getArray(), &binLen, 4); assert(0); //TODO linux
                seq2[0]= sal::static_int_cast<sal_Int8>(binLen&0xFF);
                seq2[1]= sal::static_int_cast<sal_Int8>((binLen>>8)&0xFF);
                seq2[2]= sal::static_int_cast<sal_Int8>((binLen>>16)&0xFF);
                seq2[3]= sal::static_int_cast<sal_Int8>((binLen>>24)&0xFF);
                unsigned char *data2=binBufferStr;
                memcpy(seq2.getArray()+4, data2+o, binLen);
                uno::Reference<io::XInputStream> myInStream=new comphelper::SequenceInputStream(seq2);
                printf("SEQ OK\n");

                uno::Reference< io::XStream > xContentStream = xStorage->openStreamElement(
                    rtl::OUString::createFromAscii(buf), embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE );
                uno::Reference<beans::XPropertySet> xContentStreamPropSet(xContentStream, uno::UNO_QUERY_THROW);
                xContentStreamPropSet->setPropertyValue(rtl::OUString::createFromAscii("MediaType"), uno::makeAny(rtl::OUString::createFromAscii("application/vnd.sun.star.oleobject")));
                printf("CONTENT STREAM OK\n");

                uno::Sequence< uno::Any > aArgs1( 1 );
                aArgs1[0] <<= xContentStream;
                uno::Reference< container::XNameContainer > xNameContainer2(
                    xServiceFactory->createInstanceWithArguments(
                    ::rtl::OUString::createFromAscii("com.sun.star.embed.OLESimpleStorage" ),
                    aArgs1 ),
                    uno::UNO_QUERY_THROW );
                printf("OLE STORAGE OK\n");

                uno::Any anyStream;
                anyStream <<= myInStream;
                xNameContainer2->insertByName(rtl::OUString::createFromAscii("\1Ole10Native"), anyStream);
                printf("INSERT OK\n");

                uno::Reference<embed::XTransactedObject> xTransact(xNameContainer2, uno::UNO_QUERY);
                xTransact->commit();
            }
            objDataLevel--;
            numOfOLEs++;
        }
#endif
//		printf("}");
    }
    void addSpaces(int /*count*/)
    {
//		for(int i=0;i<count;i++)
//			printf(" ");

    }
    void addBinData(unsigned char /*data*/)
    {
//		printf("%02Xh", data);
    }
    void addChar(char ch)
    {
//		printf("%c", ch);
        if (objDataLevel)
        {
            if (numOfOLEChars%2==0)
            {
                char c=sal::static_int_cast<char>(toupper(ch));
                assert((c<='F' && c>='A') || (c<='9' && c>='0'));
                if(c>='A') hb=(unsigned char)(c-'A'+10); else hb=(unsigned char)(c-'0');
            }
            else
            {
                unsigned char lb;
                char c=sal::static_int_cast<char>(toupper(ch));
                assert((c<='F' && c>='A') || (c<='9' && c>='0'));
                if(c>='A') lb=(unsigned char)(c-'A'+10); else lb=(unsigned char)(c-'0');
                unsigned char r=(hb<<4)|lb;
                binBuffer.push_back(r);
            }
            numOfOLEChars++;
        }
    }
    void addCharU(sal_Unicode /*ch*/)
    {
//		printf("\\u%i ", ch);
    }
    void addHexChar(char* /*hexch*/)
    {
//		printf("\'%s ", hexch);
    }


public:
    MyRtfScannerHandler(uno::Reference<lang::XMultiServiceFactory> &xServiceFactory_, uno::Reference<com::sun::star::ucb::XSimpleFileAccess> &xFileAccess_, uno::Reference<embed::XStorage> &xStorage_) :
    objDataLevel(0), numOfOLEs(0),
    xServiceFactory(xServiceFactory_),
    xFileAccess(xFileAccess_),
    xStorage(xStorage_)
    {
    }

    virtual ~MyRtfScannerHandler() {}

    void dump()
    {
        printf("Destinations:\n");
        for(ScannerTestServiceTokenMap::iterator i=destMap.begin();i!=destMap.end();i++)
        {
            printf("  %s\n", i->getStr());
        }
        printf("Ctrls:\n");
        for(ScannerTestServiceTokenMap::iterator i=ctrlMap.begin();i!=ctrlMap.end();i++)
        {
            printf("  %s\n", i->getStr());
        }
    }
};

class RtfInputSourceImpl : public rtftok::RTFInputSource
{
private:
    uno::Reference< io::XInputStream > xInputStream;
    uno::Reference< io::XSeekable > xSeekable;
    uno::Reference< task::XStatusIndicator > xStatusIndicator;
    sal_Int64 bytesTotal;
    sal_Int64 bytesRead;
public:
    RtfInputSourceImpl(uno::Reference< io::XInputStream > &xInputStream_, uno::Reference< task::XStatusIndicator > &xStatusIndicator_) :
      xInputStream(xInputStream_),
      xStatusIndicator(xStatusIndicator_),
      bytesRead(0)
    {
        xSeekable=uno::Reference< io::XSeekable >(xInputStream, uno::UNO_QUERY);
        if (xSeekable.is())
            bytesTotal=xSeekable->getLength();
        if (xStatusIndicator.is() && xSeekable.is())
        {
            xStatusIndicator->start(::rtl::OUString::createFromAscii("Converting"), 100);
        }
    }

    virtual ~RtfInputSourceImpl() {}

    int read(void *buf, int maxlen)
    {
        uno::Sequence< sal_Int8 > buffer;
        int len=xInputStream->readSomeBytes(buffer,maxlen);
        if (len>0)
        {
            sal_Int8 *_buffer=buffer.getArray();
            memcpy(buf, _buffer, len);
            bytesRead+=len;
            if (xStatusIndicator.is())
            {
                if (xSeekable.is())
                {
                    xStatusIndicator->setValue((int)(bytesRead*100/bytesTotal));
                }
                else
                {
                    char buf1[100];
                    sprintf(buf1, "Converted %" SAL_PRIdINT64 " KB", bytesRead/1024);
                    xStatusIndicator->start(::rtl::OUString::createFromAscii(buf1), 0);
                }
            }
            return len;
        }
        else
        {
            if (xStatusIndicator.is())
            {
                xStatusIndicator->end();
            }
            return 0;
        }
    }
};

ScannerTestService::ScannerTestService(const uno::Reference< uno::XComponentContext > &xContext_) :
xContext( xContext_ )
{
}

sal_Int32 SAL_CALL ScannerTestService::run( const uno::Sequence< rtl::OUString >& aArguments ) throw (uno::RuntimeException)
{

  printf("TEST\n");

    uno::Sequence<uno::Any> aUcbInitSequence(2);
    aUcbInitSequence[0] <<= rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Local"));
    aUcbInitSequence[1] <<= rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Office"));
    uno::Reference<lang::XMultiServiceFactory> xServiceFactory(xContext->getServiceManager(), uno::UNO_QUERY_THROW);
  printf("A\n");
    uno::Reference<lang::XMultiComponentFactory> xFactory(xContext->getServiceManager(), uno::UNO_QUERY_THROW );
  printf("B\n");
    if (::ucbhelper::ContentBroker::initialize(xServiceFactory, aUcbInitSequence))
    {
  printf("C\n");
            rtl::OUString arg=aArguments[0];

            uno::Reference<com::sun::star::ucb::XSimpleFileAccess> xFileAccess(
            xFactory->createInstanceWithContext(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ucb.SimpleFileAccess")),
                xContext), uno::UNO_QUERY_THROW );

            rtl_uString *dir=NULL;
            osl_getProcessWorkingDir(&dir);
            rtl::OUString absFileUrl;
            osl_getAbsoluteFileURL(dir, arg.pData, &absFileUrl.pData);
            rtl_uString_release(dir);

            uno::Reference <lang::XSingleServiceFactory> xStorageFactory(
                xServiceFactory->createInstance (rtl::OUString::createFromAscii("com.sun.star.embed.StorageFactory")), uno::UNO_QUERY_THROW);

            rtl::OUString outFileUrl;
            {
            rtl_uString *dir1=NULL;
            osl_getProcessWorkingDir(&dir1);
            osl_getAbsoluteFileURL(dir1, aArguments[1].pData, &outFileUrl.pData);
            rtl_uString_release(dir1);
            }

            uno::Sequence< uno::Any > aArgs2( 2 );
            aArgs2[0] <<= outFileUrl;
            aArgs2[1] <<= embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE;
            uno::Reference<embed::XStorage> xStorage(xStorageFactory->createInstanceWithArguments(aArgs2), uno::UNO_QUERY_THROW);
            uno::Reference<beans::XPropertySet> xPropSet(xStorage, uno::UNO_QUERY_THROW);
            xPropSet->setPropertyValue(rtl::OUString::createFromAscii("MediaType"), uno::makeAny(rtl::OUString::createFromAscii("application/vnd.oasis.opendocument.text")));
            uno::Reference<io::XInputStream> xInputStream = xFileAccess->openFileRead(absFileUrl);
            uno::Reference< task::XStatusIndicator > xStatusIndicator;

        TimeValue t1; osl_getSystemTime(&t1);

            RtfInputSourceImpl rtfInputSource(xInputStream, xStatusIndicator);
            MyRtfScannerHandler eventHandler(xServiceFactory, xFileAccess, xStorage);
            writerfilter::rtftok::RTFScanner *rtfScanner=writerfilter::rtftok::RTFScanner::createRTFScanner(rtfInputSource, eventHandler);

            rtfScanner->yylex();
            delete rtfScanner;

        TimeValue t2; osl_getSystemTime(&t2);
        printf("time=%" SAL_PRIuUINT32 "s\n", t2.Seconds-t1.Seconds);

//			eventHandler.dump();
            uno::Reference<embed::XTransactedObject> xTransact(xStorage, uno::UNO_QUERY);
            xTransact->commit();


        ::ucbhelper::ContentBroker::deinitialize();
    }
    else
    {
        fprintf(stderr, "can't initialize UCB");
    }
    return 0;
}

::rtl::OUString ScannerTestService_getImplementationName ()
{
    return rtl::OUString::createFromAscii ( ScannerTestService::IMPLEMENTATION_NAME );
}

sal_Bool SAL_CALL ScannerTestService_supportsService( const ::rtl::OUString& ServiceName )
{
    return ServiceName.equals( rtl::OUString::createFromAscii( ScannerTestService::SERVICE_NAME ) );
}
uno::Sequence< rtl::OUString > SAL_CALL ScannerTestService_getSupportedServiceNames(  ) throw (uno::RuntimeException)
{
    uno::Sequence < rtl::OUString > aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] =  rtl::OUString::createFromAscii ( ScannerTestService::SERVICE_NAME );
    return aRet;
}

uno::Reference< uno::XInterface > SAL_CALL ScannerTestService_createInstance( const uno::Reference< uno::XComponentContext > & xContext) throw( uno::Exception )
{
    return (cppu::OWeakObject*) new ScannerTestService( xContext );
}

} } /* end namespace writerfilter::rtftok */
