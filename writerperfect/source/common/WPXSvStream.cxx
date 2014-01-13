/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "WPXSvStream.hxx"

#include <rtl/string.hxx>
#include <tools/stream.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include <limits>
#include <vector>

#include <boost/scoped_ptr.hpp>
#include <boost/unordered_map.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;

namespace
{

class PositionHolder
{
    // disable copying
    PositionHolder(const PositionHolder &);
    PositionHolder &operator=(const PositionHolder &);

public:
    explicit PositionHolder(const Reference<XSeekable> &rxSeekable);
    ~PositionHolder();

private:
    const Reference<XSeekable> mxSeekable;
    const sal_uInt64 mnPosition;
};

PositionHolder::PositionHolder(const Reference<XSeekable> &rxSeekable)
    : mxSeekable(rxSeekable)
    , mnPosition(rxSeekable->getPosition())
{
}

PositionHolder::~PositionHolder() try
{
    mxSeekable->seek(mnPosition);
}
catch (...)
{
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

namespace
{

const rtl::OUString concatPath(const rtl::OUString &lhs, const rtl::OUString &rhs)
{
    if (lhs.isEmpty())
        return rhs;
    return lhs + "/" + rhs;
}

struct StreamData
{
    explicit StreamData(const rtl::OString &rName);

    SotStorageStreamRefWrapper stream;
    rtl::OString name;
};

typedef boost::unordered_map<rtl::OUString, std::size_t, rtl::OUStringHash> NameMap_t;
typedef boost::unordered_map<rtl::OUString, SotStorageRefWrapper, rtl::OUStringHash> OLEStorageMap_t;

struct OLEStorageImpl
{
    OLEStorageImpl();

    void initialize(SvStream *pStream);

    SotStorageStreamRef getStream(const rtl::OUString &rPath);
    SotStorageStreamRef getStream(std::size_t nId);

private:
    void traverse(const SotStorageRef &rStorage, const rtl::OUString &rPath);

    SotStorageStreamRef createStream(const rtl::OUString &rPath);

public:
    boost::scoped_ptr<SvStream> mpStream;
    SotStorageRefWrapper mxRootStorage;
    OLEStorageMap_t maStorageMap;
    ::std::vector< StreamData > maStreams;
    NameMap_t maNameMap;
    bool mbInitialized;
};

StreamData::StreamData(const rtl::OString &rName)
    : stream()
    , name(rName)
{
}

OLEStorageImpl::OLEStorageImpl()
    : mpStream()
    , mxRootStorage()
    , maStorageMap()
    , maStreams()
    , maNameMap()
    , mbInitialized(false)
{
}

void OLEStorageImpl::initialize(SvStream *const pStream)
{
    if (!pStream)
        return;

    mpStream.reset(pStream);
    mxRootStorage.ref = new SotStorage( pStream, sal_True );

    traverse(mxRootStorage.ref, "");

    mbInitialized = true;
}

SotStorageStreamRef OLEStorageImpl::getStream(const rtl::OUString &rPath)
{
    NameMap_t::iterator aIt = maNameMap.find(rPath);

    // For the while don't return stream in this situation.
    // Later, given how libcdr's zip stream implementation behaves,
    // return the first stream in the storage if there is one.
    if (maNameMap.end() == aIt)
        return SotStorageStreamRef();

    if (!maStreams[aIt->second].stream.ref.Is())
        maStreams[aIt->second].stream.ref = createStream(rPath);

    return maStreams[aIt->second].stream.ref;
}

SotStorageStreamRef OLEStorageImpl::getStream(const std::size_t nId)
{
    if (!maStreams[nId].stream.ref.Is())
        maStreams[nId].stream.ref = createStream(rtl::OStringToOUString(maStreams[nId].name, RTL_TEXTENCODING_UTF8));

    return maStreams[nId].stream.ref;
}

void OLEStorageImpl::traverse(const SotStorageRef &rStorage, const rtl::OUString &rPath)
{
    SvStorageInfoList infos;

    rStorage->FillInfoList(&infos);

    for (SvStorageInfoList::const_iterator aIt = infos.begin(); infos.end() != aIt; ++aIt)
    {
        if (aIt->IsStream())
        {
            maStreams.push_back(StreamData(rtl::OUStringToOString(aIt->GetName(), RTL_TEXTENCODING_UTF8)));
            maNameMap[concatPath(rPath, aIt->GetName())] = maStreams.size() - 1;
        }
        else if (aIt->IsStorage())
        {
            const rtl::OUString aPath = concatPath(rPath, aIt->GetName());
            SotStorageRefWrapper xStorage;
            xStorage.ref = rStorage->OpenSotStorage(aIt->GetName(), STREAM_STD_READ);
            maStorageMap[aPath] = xStorage;

            // deep-first traversal
            traverse(xStorage.ref, aPath);
        }
        else
        {
            assert(0);
        }
    }
}

SotStorageStreamRef OLEStorageImpl::createStream(const rtl::OUString &rPath)
{
    const sal_Int32 nDelim = rPath.lastIndexOf(sal_Unicode('/'));

    if (-1 == nDelim)
        return mxRootStorage.ref->OpenSotStream(rPath, STREAM_STD_READ);

    const rtl::OUString aDir = rPath.copy(0, nDelim);
    const rtl::OUString aName = rPath.copy(nDelim + 1);

    const OLEStorageMap_t::const_iterator aIt = maStorageMap.find(aDir);

    // We can only get there for paths that are present in the OLE.
    // Which means the storage must exist.
    assert(maStorageMap.end() != aIt);

    return aIt->second.ref->OpenSotStream(aName, STREAM_STD_READ);
}

}

class WPXSvInputStreamImpl
{
public :
    WPXSvInputStreamImpl( ::com::sun::star::uno::Reference<
                      ::com::sun::star::io::XInputStream > xStream );
    ~WPXSvInputStreamImpl();

    bool isStructured();
    unsigned subStreamCount();
    const char * subStreamName(unsigned id);
    bool existsSubStream(const char *name);
    WPXInputStream * getSubStreamByName(const char *name);
    WPXInputStream * getSubStreamById(unsigned id);

    const unsigned char *read(unsigned long numBytes, unsigned long &numBytesRead);
    int seek(long offset);
    long tell();
    bool isEnd();

    void invalidateReadBuffer();

private:
    bool isOLE();
    void ensureOLEIsInitialized();

    WPXInputStream *createWPXStream(const SotStorageStreamRef &rxStorage);

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > mxStream;
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XSeekable > mxSeekable;
    ::com::sun::star::uno::Sequence< sal_Int8 > maData;
    boost::scoped_ptr< OLEStorageImpl > mpOLEStorage;
    bool mbCheckedOLE;
public:
    sal_Int64 mnLength;
    unsigned char *mpReadBuffer;
    unsigned long mnReadBufferLength;
    unsigned long mnReadBufferPos;
};

WPXSvInputStreamImpl::WPXSvInputStreamImpl( Reference< XInputStream > xStream ) :
    mxStream(xStream),
    mxSeekable(xStream, UNO_QUERY),
    maData(0),
    mnLength(0),
    mpReadBuffer(0),
    mnReadBufferLength(0),
    mnReadBufferPos(0)
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
    if (mpReadBuffer)
        delete [] mpReadBuffer;
}

const unsigned char *WPXSvInputStreamImpl::read(unsigned long numBytes, unsigned long &numBytesRead)
{
    numBytesRead = 0;

    if (numBytes == 0 || isEnd())
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

int WPXSvInputStreamImpl::seek(long offset)
{
    if ((mnLength == 0) || !mxStream.is() || !mxSeekable.is())
        return -1;

    sal_Int64 tmpPosition = mxSeekable->getPosition();
    if ((tmpPosition < 0) || (tmpPosition > (std::numeric_limits<long>::max)()))
        return -1;

    try
    {
        mxSeekable->seek(offset);
        return 0;
    }
    catch (...)
    {
        SAL_WARN("writerperfect", "mxSeekable->seek(offset) threw exception");
        return -1;
    }
}

bool WPXSvInputStreamImpl::isEnd()
{
    if ((mnLength == 0) || !mxStream.is() || !mxSeekable.is())
        return true;
    return (mxSeekable->getPosition() >= mnLength);
}

bool WPXSvInputStreamImpl::isStructured()
{
    if ((mnLength == 0) || !mxStream.is() || !mxSeekable.is())
        return false;

    PositionHolder pos(mxSeekable);
    mxSeekable->seek(0);

    return isOLE();
}

unsigned WPXSvInputStreamImpl::subStreamCount()
{
    if ((mnLength == 0) || !mxStream.is() || !mxSeekable.is())
        return 0;

    PositionHolder pos(mxSeekable);
    mxSeekable->seek(0);

    if (isOLE())
    {
        ensureOLEIsInitialized();

        return mpOLEStorage->maStreams.size();
    }

    return 0;
}

const char *WPXSvInputStreamImpl::subStreamName(const unsigned id)
{
    if ((mnLength == 0) || !mxStream.is() || !mxSeekable.is())
        return 0;

    PositionHolder pos(mxSeekable);
    mxSeekable->seek(0);

    if (isOLE())
    {
        ensureOLEIsInitialized();

        if (mpOLEStorage->maStreams.size() <= id)
            return 0;

        return mpOLEStorage->maStreams[id].name.getStr();
    }

    return 0;
}

bool WPXSvInputStreamImpl::existsSubStream(const char *const name)
{
    if (!name)
        return 0;

    if ((mnLength == 0) || !mxStream.is() || !mxSeekable.is())
        return false;

    PositionHolder pos(mxSeekable);
    mxSeekable->seek(0);

    if (isOLE())
    {
        ensureOLEIsInitialized();

        const rtl::OUString aName(rtl::OStringToOUString(rtl::OString(name), RTL_TEXTENCODING_UTF8));
        return mpOLEStorage->maNameMap.end() != mpOLEStorage->maNameMap.find(aName);
    }

    return false;
}

WPXInputStream *WPXSvInputStreamImpl::getSubStreamByName(const char *const name)
{
    if (!name)
        return 0;

    if ((mnLength == 0) || !mxStream.is() || !mxSeekable.is())
        return 0;

    PositionHolder pos(mxSeekable);
    mxSeekable->seek(0);

    if (isOLE())
    {
        ensureOLEIsInitialized();

        const rtl::OUString aName(rtl::OStringToOUString(rtl::OString(name), RTL_TEXTENCODING_UTF8));
        return createWPXStream(mpOLEStorage->getStream(aName));
    }

    return 0;
}

WPXInputStream *WPXSvInputStreamImpl::getSubStreamById(const unsigned id)
{
    if ((mnLength == 0) || !mxStream.is() || !mxSeekable.is())
        return 0;

    PositionHolder pos(mxSeekable);
    mxSeekable->seek(0);

    if (isOLE())
    {
        ensureOLEIsInitialized();

        if (mpOLEStorage->maStreams.size() <= id)
            return 0;

        return createWPXStream(mpOLEStorage->getStream(id));
    }

    return 0;
}

void WPXSvInputStreamImpl::invalidateReadBuffer()
{
    if (mpReadBuffer)
    {
        seek((long) tell() + (long)mnReadBufferPos - (long)mnReadBufferLength);
        delete [] mpReadBuffer;
        mpReadBuffer = 0;
        mnReadBufferPos = 0;
        mnReadBufferLength = 0;
    }
}

WPXInputStream *WPXSvInputStreamImpl::createWPXStream(const SotStorageStreamRef &rxStorage)
{
    Reference < XInputStream > xContents(new utl::OSeekableInputStreamWrapper( rxStorage ));
    if (xContents.is())
        return new WPXSvInputStream( xContents );
    else
        return 0;
}

bool WPXSvInputStreamImpl::isOLE()
{
    if (!mbCheckedOLE)
    {
        assert(0 == mxSeekable->getPosition());

        boost::scoped_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream( mxStream ));
        if (pStream && SotStorage::IsOLEStorage(pStream.get()))
            mpOLEStorage.reset(new OLEStorageImpl());

        mbCheckedOLE = true;
    }

    return bool(mpOLEStorage);
}

void WPXSvInputStreamImpl::ensureOLEIsInitialized()
{
    assert(mpOLEStorage);

    if (!mpOLEStorage->mbInitialized)
        mpOLEStorage->initialize(utl::UcbStreamHelper::CreateStream( mxStream ));
}

WPXSvInputStream::WPXSvInputStream( Reference< XInputStream > xStream ) :
    mpImpl(new WPXSvInputStreamImpl(xStream))
{
}

WPXSvInputStream::~WPXSvInputStream()
{
   if (mpImpl)
       delete mpImpl;
}

#define BUFFER_MAX 65536

const unsigned char *WPXSvInputStream::read(unsigned long numBytes, unsigned long &numBytesRead)
{
    numBytesRead = 0;

    if (numBytes == 0 || numBytes > (std::numeric_limits<unsigned long>::max)()/2)
        return 0;

    if (mpImpl->mpReadBuffer)
    {
        if ((mpImpl->mnReadBufferPos + numBytes > mpImpl->mnReadBufferPos) && (mpImpl->mnReadBufferPos + numBytes <= mpImpl->mnReadBufferLength))
        {
            const unsigned char *pTmp = mpImpl->mpReadBuffer + mpImpl->mnReadBufferPos;
            mpImpl->mnReadBufferPos += numBytes;
            numBytesRead = numBytes;
            return pTmp;
        }

        mpImpl->invalidateReadBuffer();
    }

    unsigned long curpos = (unsigned long) mpImpl->tell();
    if (curpos == (unsigned long)-1)  // returned ERROR
        return 0;

    if ((curpos + numBytes < curpos) /*overflow*/ ||
            (curpos + numBytes >= (sal_uInt64)mpImpl->mnLength))  /*reading more than available*/
    {
        numBytes = mpImpl->mnLength - curpos;
    }

    if (numBytes < BUFFER_MAX)
    {
        if (BUFFER_MAX < mpImpl->mnLength - curpos)
            mpImpl->mnReadBufferLength = BUFFER_MAX;
        else /* BUFFER_MAX >= mpImpl->mnLength - curpos */
            mpImpl->mnReadBufferLength = mpImpl->mnLength - curpos;
    }
    else
        return mpImpl->read(numBytes, numBytesRead);

    mpImpl->mpReadBuffer = new unsigned char[mpImpl->mnReadBufferLength];
    unsigned long tmpNumBytes(0);
    const unsigned char *pTmp = mpImpl->read(mpImpl->mnReadBufferLength, tmpNumBytes);
    if (tmpNumBytes != mpImpl->mnReadBufferLength)
        mpImpl->mnReadBufferLength = tmpNumBytes;

    mpImpl->mnReadBufferPos = 0;
    if (!mpImpl->mnReadBufferLength)
        return 0;

    numBytesRead = numBytes;

    mpImpl->mnReadBufferPos += numBytesRead;
    memcpy(mpImpl->mpReadBuffer, pTmp, mpImpl->mnReadBufferLength);
    return const_cast<const unsigned char *>(mpImpl->mpReadBuffer);
}

long WPXSvInputStream::tell()
{
    long retVal = mpImpl->tell();
    return retVal - (long)mpImpl->mnReadBufferLength + (long)mpImpl->mnReadBufferPos;
}

int WPXSvInputStream::seek(long offset, WPX_SEEK_TYPE seekType)
{
    sal_Int64 tmpOffset = offset;
    if (seekType == WPX_SEEK_CUR)
        tmpOffset += tell();
    if (seekType == WPX_SEEK_END)
        tmpOffset += mpImpl->mnLength;

    int retVal = 0;
    if (tmpOffset < 0)
    {
        tmpOffset = 0;
        retVal = -1;
    }
    if (tmpOffset > mpImpl->mnLength)
    {
        tmpOffset = mpImpl->mnLength;
        retVal = -1;
    }

    if (tmpOffset < mpImpl->tell() && (unsigned long)tmpOffset >= (unsigned long)mpImpl->tell() - mpImpl->mnReadBufferLength)
    {
        mpImpl->mnReadBufferPos = (unsigned long)(tmpOffset + (long) mpImpl->mnReadBufferLength - (long) mpImpl->tell());
        return retVal;
    }

    mpImpl->invalidateReadBuffer();

    if (mpImpl->seek(tmpOffset))
        return -1;
    return retVal;
}

bool WPXSvInputStream::isEnd()
{
    return mpImpl->isEnd() && mpImpl->mnReadBufferPos == mpImpl->mnReadBufferLength;
}

bool WPXSvInputStream::isStructured()
{
    mpImpl->invalidateReadBuffer();
    return mpImpl->isStructured();
}

unsigned WPXSvInputStream::subStreamCount()
{
    mpImpl->invalidateReadBuffer();
    return mpImpl->subStreamCount();
}

const char *WPXSvInputStream::subStreamName(const unsigned id)
{
    mpImpl->invalidateReadBuffer();
    return mpImpl->subStreamName(id);
}

bool WPXSvInputStream::existsSubStream(const char *const name)
{
    mpImpl->invalidateReadBuffer();
    return mpImpl->existsSubStream(name);
}

WPXInputStream *WPXSvInputStream::getSubStreamByName(const char *name)
{
    mpImpl->invalidateReadBuffer();
    return mpImpl->getSubStreamByName(name);
}

WPXInputStream *WPXSvInputStream::getSubStreamById(const unsigned id)
{
    mpImpl->invalidateReadBuffer();
    return mpImpl->getSubStreamById(id);
}

bool WPXSvInputStream::atEOS()
{
    return isEnd();
}

bool WPXSvInputStream::isOLEStream()
{
    return isStructured();
}

WPXInputStream *WPXSvInputStream::getDocumentOLEStream(const char *name)
{
    return getSubStreamByName(name);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
