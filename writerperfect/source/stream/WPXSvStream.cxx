#include "WPXSvStream.h"
#include "libwpd_internal.h"

#include <sot/storage.hxx>
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
        mxStream(xStream),
        mnOffset(0)
{
    Reference < XSeekable> xSeekable = Reference < XSeekable > (xStream, UNO_QUERY);
    if (!xSeekable.is())
        mnLength = 0;
    else
        mnLength = xSeekable->getLength(); // exception
}

WPXSvInputStream::~WPXSvInputStream()
{
}

const uint8_t * WPXSvInputStream::read(size_t numBytes)
{
    // FIXME: assume no short reads (?)
    mnOffset += mxStream->readBytes (maData, numBytes);
    return (const uint8_t *)maData.getConstArray();
}

int WPXSvInputStream::seek(long offset, WPX_SEEK_TYPE seekType)
{
    if (seekType == WPX_SEEK_CUR && offset >= 0)
    {
            mxStream->skipBytes (offset); // exception ?
            mnOffset += offset;
            return FALSE;
    }

    Reference < XSeekable> xSeekable = Reference < XSeekable >(mxStream, UNO_QUERY);

    if (!xSeekable.is())
            return TRUE;

    if (seekType == WPX_SEEK_CUR)
            mnOffset += offset;
    else
            mnOffset = offset;

    xSeekable->seek(mnOffset); // FIXME: catch exception!

    return FALSE;
}

long WPXSvInputStream::tell()
{
    return mnOffset;
}

bool WPXSvInputStream::atEOS()
{
    return mnOffset < mnLength;
}

bool WPXSvInputStream::isOLEStream()
{
    bool bAns;

    SvStream *pStream = utl::UcbStreamHelper::CreateStream( mxStream );
    bAns = pStream && SotStorage::IsOLEStorage( pStream );
    delete pStream;

    return bAns;
}

WPXInputStream * WPXSvInputStream::getDocumentOLEStream()
{
    SvStream *pStream = utl::UcbStreamHelper::CreateStream( mxStream );

    SotStorage *pStorage = new SotStorage( pStream, TRUE );

    fprintf (stderr, "Dodgy\n");
    SvStream *pContents = pStorage->OpenSotStream(
            rtl::OUString::createFromAscii( "PerfectOffice_MAIN" ),
            STREAM_STD_READ );

    Reference < XInputStream > xContents = new utl::OInputStreamWrapper( pStream );
    if (xContents.is())
        return new WPXSvInputStream( xContents );
    else {
        fprintf( stderr, "Badly wrong\n" );
        return NULL;
    }
    // FIXME: we leak pStorage - investigate protected destructor there.
}
