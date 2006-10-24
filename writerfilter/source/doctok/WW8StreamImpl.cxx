#include <WW8StreamImpl.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>

namespace doctok
{
using namespace ::com::sun::star;

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
    }

    if (pResult.get() == NULL)
        throw ExceptionNotFound("Stream not found");

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

}
