/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "WPXSvStream.hxx"
#include <tools/stream.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <limits>
#include <vector>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;

namespace
{
static void splitPath( std::vector<OUString> &rElems, const OUString &rPath )
{
    for (sal_Int32 i = 0; i >= 0;)
        rElems.push_back( rPath.getToken( 0, '/', i ) );
}

} // anonymous namespace

typedef struct
{
    SotStorageRef ref;
} SotStorageRefWrapper;

typedef struct
{
    SotStorageStreamRef ref;
} SotStorageStreamRefWrapper;

class WPXSvInputStreamImpl : public WPXInputStream
{
public :
    WPXSvInputStreamImpl( ::com::sun::star::uno::Reference<
                      ::com::sun::star::io::XInputStream > xStream );
    ~WPXSvInputStreamImpl();

    bool isOLEStream();
    WPXInputStream * getDocumentOLEStream(const char *name);

    const unsigned char *read(unsigned long numBytes, unsigned long &numBytesRead);
    int seek(long offset, WPX_SEEK_TYPE seekType);
    long tell();
    bool atEOS();
private:
    ::std::vector< SotStorageRefWrapper > mxChildrenStorages;
    ::std::vector< SotStorageStreamRefWrapper > mxChildrenStreams;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::io::XInputStream > mxStream;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::io::XSeekable > mxSeekable;
    ::com::sun::star::uno::Sequence< sal_Int8 > maData;
    sal_Int64 mnLength;
};

WPXSvInputStreamImpl::WPXSvInputStreamImpl( Reference< XInputStream > xStream ) :
    WPXInputStream(),
    mxChildrenStorages(),
    mxChildrenStreams(),
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
                SAL_WARN("writerperfect", "mnLength = mxSeekable->getLength() threw exception");
                mnLength = 0;
            }
        }
    }
}

WPXSvInputStreamImpl::~WPXSvInputStreamImpl()
{
}

const unsigned char *WPXSvInputStreamImpl::read(unsigned long numBytes, unsigned long &numBytesRead)
{
    numBytesRead = 0;

    if (numBytes == 0 || atEOS())
        return 0;

    numBytesRead = mxStream->readSomeBytes (maData, numBytes);
    if (numBytesRead == 0)
        return 0;

    return (const unsigned char *)maData.getConstArray();
}

long WPXSvInputStreamImpl::tell()
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

int WPXSvInputStreamImpl::seek(long offset, WPX_SEEK_TYPE seekType)
{
    if ((mnLength == 0) || !mxStream.is() || !mxSeekable.is())
        return -1;

    sal_Int64 tmpPosition = mxSeekable->getPosition();
    if ((tmpPosition < 0) || (tmpPosition > (std::numeric_limits<long>::max)()))
        return -1;

    sal_Int64 tmpOffset = offset;
    if (seekType == WPX_SEEK_CUR)
        tmpOffset += tmpPosition;
    if (seekType == WPX_SEEK_END)
        tmpOffset += mnLength;

    int retVal = 0;
    if (tmpOffset < 0)
    {
        tmpOffset = 0;
        retVal = -1;
    }
    if (tmpOffset > mnLength)
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
        SAL_WARN("writerperfect", "mxSeekable->seek(offset) threw exception");
        return -1;
    }
}

bool WPXSvInputStreamImpl::atEOS()
{
    if ((mnLength == 0) || !mxStream.is() || !mxSeekable.is())
        return true;
    return (mxSeekable->getPosition() >= mnLength);
}

bool WPXSvInputStreamImpl::isOLEStream()
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

WPXInputStream *WPXSvInputStreamImpl::getDocumentOLEStream(const char *name)
{
    if (!name)
        return 0;
    OUString rPath(name,strlen(name),RTL_TEXTENCODING_UTF8);
    std::vector<OUString> aElems;
    splitPath( aElems, rPath );

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

    SotStorageRefWrapper storageRefWrapper;
    storageRefWrapper.ref = new SotStorage( pStream, sal_True );
    mxChildrenStorages.push_back( storageRefWrapper );

    unsigned i = 0;
    while (i < aElems.size())
    {
        if( mxChildrenStorages.back().ref->IsStream(aElems[i]))
            break;
        else if (mxChildrenStorages.back().ref->IsStorage(aElems[i]))
        {
            SotStorageRef tmpParent(mxChildrenStorages.back().ref);
            storageRefWrapper.ref = tmpParent->OpenSotStorage(aElems[i++], STREAM_STD_READ);
            mxChildrenStorages.push_back(storageRefWrapper);
        }
        else
            // should not happen
            return 0;
    }

    // For the while don't return stream in this situation.
    // Later, given how libcdr's zip stream implementation behaves,
    // return the first stream in the storage if there is one.
    if (i >= aElems.size())
        return 0;

    SotStorageStreamRefWrapper storageStreamRefWrapper;
    storageStreamRefWrapper.ref = mxChildrenStorages.back().ref->OpenSotStream( aElems[i], STREAM_STD_READ );
    mxChildrenStreams.push_back( storageStreamRefWrapper );

    mxSeekable->seek(tmpPosition);

    if ( !mxChildrenStreams.back().ref.Is() || mxChildrenStreams.back().ref->GetError() )
    {
        mxSeekable->seek(tmpPosition);
        return 0;
    }

    Reference < XInputStream > xContents(new utl::OSeekableInputStreamWrapper( mxChildrenStreams.back().ref ));
    mxSeekable->seek(tmpPosition);
    if (xContents.is())
        return new WPXSvInputStream( xContents );
    else
        return 0;
}


WPXSvInputStream::WPXSvInputStream( Reference< XInputStream > xStream ) :
    mpImpl(new WPXSvInputStreamImpl(xStream))
{
}

WPXSvInputStream::~WPXSvInputStream()
{
   delete mpImpl;
}

const unsigned char *WPXSvInputStream::read(unsigned long numBytes, unsigned long &numBytesRead)
{
    return mpImpl->read(numBytes, numBytesRead);
}

long WPXSvInputStream::tell()
{
    return mpImpl->tell();
}

int WPXSvInputStream::seek(long offset, WPX_SEEK_TYPE seekType)
{
    return mpImpl->seek(offset, seekType);
}

bool WPXSvInputStream::atEOS()
{
    return mpImpl->atEOS();
}

bool WPXSvInputStream::isOLEStream()
{
    return mpImpl->isOLEStream();
}

WPXInputStream *WPXSvInputStream::getDocumentOLEStream(const char *name)
{
    return mpImpl->getDocumentOLEStream(name);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
