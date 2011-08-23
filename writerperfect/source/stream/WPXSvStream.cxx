#include "WPXSvStream.h"
#include "filter/FilterInternal.hxx"
#include <tools/stream.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <limits>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;

WPXSvInputStream::WPXSvInputStream( Reference< XInputStream > xStream ) :
        WPSInputStream(),
        mxChildStorage(),
        mxChildStream(),
        mxStream(xStream),
        mxSeekable(xStream, UNO_QUERY),
        maData(0)
{
    if (!xStream.is() || !mxStream.is())
        mnLength = 0;
    else
    {
        if (!mxSeekable.is())
            mnLength = 0;
        else
        {
            try
            {
                mnLength = mxSeekable->getLength();
            }
            catch ( ... )
            {
                WRITER_DEBUG_MSG(("mnLength = mxSeekable->getLength() threw exception\n"));
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
    numBytesRead = 0;

    if (numBytes == 0 || atEOS())
        return 0;

    numBytesRead = mxStream->readSomeBytes (maData, numBytes);
    if (numBytesRead == 0)
        return 0;

    return (const uint8_t *)maData.getConstArray();
}

long WPXSvInputStream::tell()
{
    if ((mnLength == 0) || !mxStream.is() || !mxSeekable.is())
        return -1L;
    else
    {
        sal_Int64 tmpPosition = mxSeekable->getPosition();
        if ((tmpPosition < 0) || (tmpPosition > (std::numeric_limits<long>::max)()))
            return -1L;
        return (long)tmpPosition;
    }
}

int WPXSvInputStream::seek(long offset, WPX_SEEK_TYPE seekType)
{
    if ((mnLength == 0) || !mxStream.is() || !mxSeekable.is())
        return -1;

    sal_Int64 tmpPosition = mxSeekable->getPosition();
    if ((tmpPosition < 0) || (tmpPosition > (std::numeric_limits<long>::max)()))
        return -1;

    sal_Int64 tmpOffset = offset;
    if (seekType == WPX_SEEK_CUR)
        tmpOffset += tmpPosition;

    int retVal = 0;
    if (tmpOffset < 0)
    {	
        tmpOffset = 0;
        retVal = -1;
    }
    if (offset > mnLength)
    {
        tmpOffset = mnLength;
        retVal = -1;
    }

    try
    {
        mxSeekable->seek(tmpOffset);
        return retVal;
    }
    catch (...)
    {
        WRITER_DEBUG_MSG(("mxSeekable->seek(offset) threw exception\n"));
        return -1;
    }
}

bool WPXSvInputStream::atEOS()
{
    if ((mnLength == 0) || !mxStream.is() || !mxSeekable.is())
        return true;
    return (mxSeekable->getPosition() >= mnLength);
}

bool WPXSvInputStream::isOLEStream()
{
    if ((mnLength == 0) || !mxStream.is() || !mxSeekable.is())
        return false;

    sal_Int64 tmpPosition = mxSeekable->getPosition();
    mxSeekable->seek(0);

    SvStream *pStream = utl::UcbStreamHelper::CreateStream( mxStream );
    bool bAns = pStream && SotStorage::IsOLEStorage( pStream );
    if (pStream)
        delete pStream;

    mxSeekable->seek(tmpPosition);

    return bAns;
}

WPXInputStream * WPXSvInputStream::getDocumentOLEStream(const char * name)
{
    if ((mnLength == 0) || !mxStream.is() || !mxSeekable.is())
        return 0;

    sal_Int64 tmpPosition = mxSeekable->getPosition();
    mxSeekable->seek(0);

    SvStream *pStream = utl::UcbStreamHelper::CreateStream( mxStream );

    if (!pStream || !SotStorage::IsOLEStorage( pStream ))
    {
        mxSeekable->seek(tmpPosition);
        return 0;
    }

    mxChildStorage = new SotStorage( pStream, TRUE );

    mxChildStream = mxChildStorage->OpenSotStream(
            rtl::OUString::createFromAscii( name ),
            STREAM_STD_READ );

    mxSeekable->seek(tmpPosition);

    if ( !mxChildStream.Is() || mxChildStream->GetError() )
    {
        mxSeekable->seek(tmpPosition);
        return 0;
    }

    Reference < XInputStream > xContents(new utl::OSeekableInputStreamWrapper( mxChildStream ));
    mxSeekable->seek(tmpPosition);
    if (xContents.is())
        return new WPXSvInputStream( xContents );
    else
        return 0;
}

WPXInputStream * WPXSvInputStream::getDocumentOLEStream()
{
    return getDocumentOLEStream( "PerfectOffice_MAIN" );
}
