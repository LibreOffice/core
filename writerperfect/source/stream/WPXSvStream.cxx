#include "WPXSvStream.h"
#include <tools/stream.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/ucbstreamhelper.hxx>

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_H_
#include <com/sun/star/io/XSeekable.hpp>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;

WPXSvInputStream::WPXSvInputStream( Reference< XInputStream > xStream ) :
        WPXInputStream(true),
        mxChildStorage(),
        mxChildStream(),
        mxStream(xStream),
        mnOffset(0)
{
    if (!xStream.is())
    {
        mnLength = 0;
    }
    else
    {
        Reference < XSeekable> xSeekable = Reference < XSeekable > (xStream, UNO_QUERY);
        if (!xSeekable.is())
            mnLength = 0;
        else
        {
            try
            {
                mnLength = xSeekable->getLength(); // exception
            }
            catch ( ... )
            {
                mnLength = 0;
            }
        }
    }
}

WPXSvInputStream::~WPXSvInputStream()
{
}

const uint8_t * WPXSvInputStream::read(size_t numBytes, size_t &numBytesRead)
{
    // FIXME: assume no short reads (?)
    sal_Int64 oldMnOffset = mnOffset;
    mnOffset += mxStream->readBytes (maData, numBytes);
    numBytesRead = mnOffset - oldMnOffset;
    return (const uint8_t *)maData.getConstArray();
}

int WPXSvInputStream::seek(long offset, WPX_SEEK_TYPE seekType)
{
    if (seekType == WPX_SEEK_CUR && offset >= 0)
    {
            if (mnOffset + offset <= mnLength)
            {
                mxStream->skipBytes (offset); // exception ?
                mnOffset += offset;
                return FALSE;
            }
            else
                return TRUE;
    }
    Reference < XSeekable> xSeekable = Reference < XSeekable >(mxStream, UNO_QUERY);

    if (!xSeekable.is())
            return TRUE;

    if (seekType == WPX_SEEK_CUR)
            mnOffset += offset;
    else
            mnOffset = offset;

    if (mnOffset > mnLength)
        return TRUE;

    xSeekable->seek(mnOffset); // FIXME: catch exception!

    return FALSE;
}

long WPXSvInputStream::tell()
{
    return mnOffset;
}

bool WPXSvInputStream::atEOS()
{
    return mnOffset >= mnLength;
}

bool WPXSvInputStream::isOLEStream()
{
    bool bAns;

    SvStream *pStream = utl::UcbStreamHelper::CreateStream( mxStream );
    bAns = pStream && SotStorage::IsOLEStorage( pStream );
    delete pStream;

    seek (0, WPX_SEEK_SET);

    return bAns;
}

WPXInputStream * WPXSvInputStream::getDocumentOLEStream()
{
    SvStream *pStream = utl::UcbStreamHelper::CreateStream( mxStream );

    mxChildStorage = new SotStorage( pStream, TRUE );

    mxChildStream = mxChildStorage->OpenSotStream(
            rtl::OUString::createFromAscii( "PerfectOffice_MAIN" ),
            STREAM_STD_READ );

    if ( !mxChildStream.Is() || mxChildStream->GetError() )
        return NULL;

    Reference < XInputStream > xContents = new utl::OSeekableInputStreamWrapper( mxChildStream );
    if (xContents.is())
        return new WPXSvInputStream( xContents );
    else
        return NULL;
}
