/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include <WW8StreamImpl.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>

#include <doctokLoggers.hxx>

namespace writerfilter {
namespace doctok
{
using namespace ::com::sun::star;

#ifdef DEBUG
TagLogger::Pointer_t debug_logger(TagLogger::getInstance("DEBUG"));
#endif

WW8Stream::~WW8Stream()
{
}

WW8StreamImpl::WW8StreamImpl(uno::Reference<uno::XComponentContext> rContext,
              uno::Reference<io::XInputStream> rStream)
: mrComponentContext(rContext), mrStream(rStream)
{
    xFactory = uno::Reference<lang::XMultiComponentFactory>
        (mrComponentContext->getServiceManager());

    uno::Sequence<uno::Any> aArgs( 1 );
    aArgs[0] <<= mrStream;

    xOLESimpleStorage = uno::Reference<container::XNameContainer>
        (xFactory->createInstanceWithArgumentsAndContext
         (::rtl::OUString::createFromAscii
          ("com.sun.star.embed.OLESimpleStorage"),
          aArgs, mrComponentContext ),
         uno::UNO_QUERY );

}

WW8StreamImpl::~WW8StreamImpl()
{
}

WW8Stream::Sequence WW8StreamImpl::get(sal_uInt32 nOffset,
                                       sal_uInt32 nCount) const
{
    uno::Sequence<sal_Int8> aSequence;

    if (nCount > 0)
    {
        uno::Reference< io::XSeekable > xSeek( mrStream, uno::UNO_QUERY_THROW );

        xSeek->seek(nOffset);

        sal_Int32 nRead = mrStream->readBytes(aSequence, nCount);

        Sequence aReturnSequence(const_cast<const sal_uInt8 *>
                                 (reinterpret_cast<sal_uInt8 *>
                                  (&(aSequence[0]))), nRead);

        return aReturnSequence;
    }

    return WW8Stream::Sequence();
}

WW8Stream::Pointer_t WW8StreamImpl::getSubStream(const ::rtl::OUString & sId)
{
    WW8Stream::Pointer_t pResult;

    try
    {
        if (xOLESimpleStorage.is())
        {
            if (xOLESimpleStorage->hasByName(sId))
            {
                uno::Reference<io::XStream> xNewStream;
                {
                    uno::Any aValue = xOLESimpleStorage->getByName(sId);
                    aValue >>= xNewStream;
                }

                if (xNewStream.is())
                {
                    WW8Stream::Pointer_t
                        pNew(new WW8StreamImpl(mrComponentContext,
                                               xNewStream->getInputStream()));

                    pResult = pNew;
                }
            }
        }
    }
    catch (...)
    {
        throw ExceptionNotFound("Stream not found");
    }


    return pResult;
}

string WW8StreamImpl::getSubStreamNames() const
{
    string sResult;

    if (xOLESimpleStorage.is())
    {
        uno::Sequence<rtl::OUString> aSeq = xOLESimpleStorage->getElementNames();

        for (sal_uInt32 n = 0;
             n < sal::static_int_cast<sal_uInt32>(aSeq.getLength()); ++n)
        {
            rtl::OUString aOUStr = aSeq[n];

            if (n > 0)
                sResult += ", ";

#if 0
            rtl::OString aOStr;
            aOUStr.convertToString(&aOStr, RTL_TEXTENCODING_ASCII_US,
                                    OUSTRING_TO_OSTRING_CVTFLAGS);


            sResult += aOStr.getStr();
#endif
            char sBuffer[256];
            for (sal_uInt32 j = 0;
                 j < sal::static_int_cast<sal_uInt32>(aOUStr.getLength()); ++j)
            {
                if (isprint(aOUStr[j]))
                {
                    sal_Unicode nC = aOUStr[j];

                    if (nC < 255)
                        sResult += sal::static_int_cast<char>(nC);
                    else
                        sResult += ".";
                }
                else
                {
                    snprintf(sBuffer, sizeof(sBuffer), "\\u%x", aOUStr[j]);
                    sResult += sBuffer;
                }
            }
        }
    }

    return sResult;
}

uno::Sequence<rtl::OUString> WW8StreamImpl::getSubStreamUNames() const
{
    return xOLESimpleStorage->getElementNames();
}

void WW8StreamImpl::dump(OutputWithDepth<string> & o) const
{
    o.addItem("<stream>");

    Sequence aSeq;
    sal_uInt32 nOffset = 0;
    sal_uInt32 nStep = 16;

    do
    {
        aSeq = get(nOffset, nStep);
        dumpLine(o, aSeq, nOffset, nStep);

        nOffset += nStep;
    }
    while (aSeq.getCount() == nStep);

    o.addItem("</stream>");
}

}}
