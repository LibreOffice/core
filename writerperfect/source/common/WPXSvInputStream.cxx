/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <WPXSvInputStream.hxx>

#include <com/sun/star/packages/zip/XZipFileAccess2.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/seekableinput.hxx>
#include <o3tl/safeint.hxx>
#include <rtl/string.hxx>
#include <sal/log.hxx>

#include <unotools/streamwrap.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include <climits>
#include <limits>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace writerperfect
{
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;

namespace container = com::sun::star::container;
namespace packages = com::sun::star::packages;

namespace
{
class PositionHolder
{
public:
    explicit PositionHolder(const Reference<XSeekable>& rxSeekable);
    ~PositionHolder();
    PositionHolder(const PositionHolder&) = delete;
    PositionHolder& operator=(const PositionHolder&) = delete;

private:
    const Reference<XSeekable> mxSeekable;
    const sal_uInt64 mnPosition;
};
} // anonymous namespace

PositionHolder::PositionHolder(const Reference<XSeekable>& rxSeekable)
    : mxSeekable(rxSeekable)
    , mnPosition(rxSeekable->getPosition())
{
}

PositionHolder::~PositionHolder()
{
    try
    {
        mxSeekable->seek(mnPosition);
    }
    catch (...)
    {
    }
}

static OUString lcl_normalizeSubStreamPath(const OUString& rPath)
{
    // accept paths which begin by '/'
    // TODO: maybe this should do a full normalization
    if (rPath.startsWith("/") && rPath.getLength() >= 2)
        return rPath.copy(1);
    return rPath;
}

static OUString concatPath(std::u16string_view lhs, const OUString& rhs)
{
    if (lhs.empty())
        return rhs;
    return OUString::Concat(lhs) + "/" + rhs;
}

namespace
{
struct OLEStreamData
{
    OLEStreamData(OString aName, OString rvngName)
        : name(std::move(aName))
        , RVNGname(std::move(rvngName))
    {
    }

    rtl::Reference<SotStorageStream> stream;

    /** Name of the stream.
      *
      * This is not @c OUString, because we need to be able to
      * produce const char* from it.
      */
    OString name;
    /** librevenge name of the stream.
      *
      * This is not @c OUString, because we need to be able to
      * produce const char* from it.
      */
    OString RVNGname;
};
} // anonymous namespace

typedef std::unordered_map<OUString, std::size_t> NameMap_t;
typedef std::unordered_map<OUString, rtl::Reference<SotStorage>> OLEStorageMap_t;

/** Representation of an OLE2 storage.
  *
  * This class tries to bring a bit of sanity to use of SotStorage with
  * respect to the needs of @c librevenge::RVNGInputStream API. It
  * holds all nested storages for the whole lifetime (more precisely,
  * since initialization, which is performed by calling @c
  * initialize()), thus ensuring that no created stream is destroyed
  * just because its parent storage went out of scope. It also holds a
  * bidirectional map of stream names to their indexes (index of a
  * stream is determined by deep-first traversal), which is also
  * populated during initialization (member variables @c maStreams and
  * @c maNameMap).
  *
  * Streams are created on demand (and saved, for the same reason as
  * storages).
  */
struct OLEStorageImpl
{
    OLEStorageImpl();

    void initialize(std::unique_ptr<SvStream> pStream);

    rtl::Reference<SotStorageStream> getStream(const OUString& rPath);
    rtl::Reference<SotStorageStream> const& getStream(std::size_t nId);

private:
    void traverse(const rtl::Reference<SotStorage>& rStorage, std::u16string_view rPath);

    rtl::Reference<SotStorageStream> createStream(const OUString& rPath);

public:
    rtl::Reference<SotStorage> mxRootStorage; //< root storage of the OLE2
    OLEStorageMap_t maStorageMap; //< map of all sub storages by name
    ::std::vector<OLEStreamData> maStreams; //< list of streams and their names
    NameMap_t maNameMap; //< map of stream names to indexes (into @c maStreams)
    bool mbInitialized;
};

OLEStorageImpl::OLEStorageImpl()
    : mbInitialized(false)
{
}

void OLEStorageImpl::initialize(std::unique_ptr<SvStream> pStream)
{
    if (!pStream)
        return;

    mxRootStorage = new SotStorage(pStream.release(), true);

    traverse(mxRootStorage, u"");

    mbInitialized = true;
}

rtl::Reference<SotStorageStream> OLEStorageImpl::getStream(const OUString& rPath)
{
    const OUString aPath(lcl_normalizeSubStreamPath(rPath));
    NameMap_t::iterator aIt = maNameMap.find(aPath);

    // For the while don't return stream in this situation.
    // Later, given how libcdr's zip stream implementation behaves,
    // return the first stream in the storage if there is one.
    if (maNameMap.end() == aIt)
        return rtl::Reference<SotStorageStream>();

    if (!maStreams[aIt->second].stream.is())
        maStreams[aIt->second].stream
            = createStream(OStringToOUString(maStreams[aIt->second].name, RTL_TEXTENCODING_UTF8));

    return maStreams[aIt->second].stream;
}

rtl::Reference<SotStorageStream> const& OLEStorageImpl::getStream(const std::size_t nId)
{
    if (!maStreams[nId].stream.is())
        maStreams[nId].stream
            = createStream(OStringToOUString(maStreams[nId].name, RTL_TEXTENCODING_UTF8));

    return maStreams[nId].stream;
}

void OLEStorageImpl::traverse(const rtl::Reference<SotStorage>& rStorage, std::u16string_view rPath)
{
    SvStorageInfoList infos;

    rStorage->FillInfoList(&infos);

    for (const auto& info : infos)
    {
        if (info.IsStream())
        {
            OUString baseName = info.GetName(), rvngName = baseName;
            // librevenge::RVNGOLEStream ignores the first character when is a control code, so ...
            if (!rvngName.isEmpty() && rvngName.toChar() < 32)
                rvngName = rvngName.copy(1);
            maStreams.emplace_back(
                OUStringToOString(concatPath(rPath, baseName), RTL_TEXTENCODING_UTF8),
                OUStringToOString(concatPath(rPath, rvngName), RTL_TEXTENCODING_UTF8));
            maNameMap[concatPath(rPath, rvngName)] = maStreams.size() - 1;
        }
        else if (info.IsStorage())
        {
            const OUString aPath = concatPath(rPath, info.GetName());
            rtl::Reference<SotStorage> aStorage
                = rStorage->OpenSotStorage(info.GetName(), StreamMode::STD_READ);
            maStorageMap[aPath] = aStorage;

            // deep-first traversal
            traverse(aStorage, aPath);
        }
        else
        {
            SAL_WARN("writerperfect",
                     "OLEStorageImpl::traverse: invalid storage entry, neither stream nor file");
        }
    }
}

rtl::Reference<SotStorageStream> OLEStorageImpl::createStream(const OUString& rPath)
{
    const sal_Int32 nDelim = rPath.lastIndexOf(u'/');

    if (-1 == nDelim)
        return mxRootStorage->OpenSotStream(rPath, StreamMode::STD_READ);

    const OUString aDir = rPath.copy(0, nDelim);
    const OUString aName = rPath.copy(nDelim + 1);

    const OLEStorageMap_t::const_iterator aIt = maStorageMap.find(aDir);

    if (maStorageMap.end() == aIt)
        return nullptr;

    return aIt->second->OpenSotStream(aName, StreamMode::STD_READ);
}

namespace
{
struct ZipStreamData
{
    explicit ZipStreamData(OString aName);

    css::uno::Reference<css::io::XInputStream> xStream;

    /** Name of the stream.
      *
      * This is not @c OUString, because we need to be able to
      * produce const char* from it.
      */
    OString aName;
};
} // anonymous namespace

/** Representation of a Zip storage.
  *
  * This is quite similar to OLEStorageImpl, except that we do not need
  * to keep all storages (folders) open.
  */
struct ZipStorageImpl
{
    explicit ZipStorageImpl(const css::uno::Reference<css::container::XNameAccess>& rxContainer);

    /** Initialize for access.
      *
      * This creates a bidirectional map of stream names to their
      * indexes (index of a stream is determined by deep-first
      * traversal).
      */
    void initialize();

    Reference<css::io::XInputStream> getStream(const OUString& rPath);
    Reference<css::io::XInputStream> const& getStream(std::size_t nId);

private:
    void traverse(const Reference<container::XNameAccess>& rxEnum);

    Reference<XInputStream> createStream(const OUString& rPath);

public:
    Reference<container::XNameAccess> mxContainer; //< root of the Zip
    ::std::vector<ZipStreamData> maStreams; //< list of streams and their names
    NameMap_t maNameMap; //< map of stream names to indexes (into @c maStreams)
    bool mbInitialized;
};

ZipStreamData::ZipStreamData(OString _aName)
    : aName(std::move(_aName))
{
}

ZipStorageImpl::ZipStorageImpl(const Reference<container::XNameAccess>& rxContainer)
    : mxContainer(rxContainer)
    , mbInitialized(false)
{
    assert(mxContainer.is());
}

void ZipStorageImpl::initialize()
{
    traverse(mxContainer);

    mbInitialized = true;
}

Reference<XInputStream> ZipStorageImpl::getStream(const OUString& rPath)
{
    const OUString aPath(lcl_normalizeSubStreamPath(rPath));
    NameMap_t::iterator aIt = maNameMap.find(aPath);

    // For the while don't return stream in this situation.
    // Later, given how libcdr's zip stream implementation behaves,
    // return the first stream in the storage if there is one.
    if (maNameMap.end() == aIt)
        return Reference<XInputStream>();

    if (!maStreams[aIt->second].xStream.is())
        maStreams[aIt->second].xStream = createStream(aPath);

    return maStreams[aIt->second].xStream;
}

Reference<XInputStream> const& ZipStorageImpl::getStream(const std::size_t nId)
{
    if (!maStreams[nId].xStream.is())
        maStreams[nId].xStream
            = createStream(OStringToOUString(maStreams[nId].aName, RTL_TEXTENCODING_UTF8));

    return maStreams[nId].xStream;
}

void ZipStorageImpl::traverse(const Reference<container::XNameAccess>& rxContainer)
{
    const Sequence<OUString> lNames = rxContainer->getElementNames();

    maStreams.reserve(lNames.getLength());

    for (const auto& rName : lNames)
    {
        if (!rName.endsWith("/")) // skip dirs
        {
            maStreams.emplace_back(OUStringToOString(rName, RTL_TEXTENCODING_UTF8));
            maNameMap[rName] = maStreams.size() - 1;
        }
    }
}

Reference<XInputStream> ZipStorageImpl::createStream(const OUString& rPath)
{
    Reference<XInputStream> xStream;

    try
    {
        const Reference<XInputStream> xInputStream(mxContainer->getByName(rPath), UNO_QUERY_THROW);
        const Reference<XSeekable> xSeekable(xInputStream, UNO_QUERY);

        if (xSeekable.is())
            xStream = xInputStream;
        else
            xStream.set(new comphelper::OSeekableInputWrapper(
                xInputStream, comphelper::getProcessComponentContext()));
    }
    catch (const Exception&)
    {
        // nothing needed
    }

    return xStream;
}

WPXSvInputStream::WPXSvInputStream(Reference<XInputStream> const& xStream)
    : mxStream(xStream)
    , mxSeekable(xStream, UNO_QUERY)
    , maData(0)
    , mnLength(0)
    , mnReadBufferPos(0)
    , mbCheckedOLE(false)
    , mbCheckedZip(false)
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
                if (0 < mxSeekable->getPosition())
                    mxSeekable->seek(0);
            }
            catch (...)
            {
                SAL_WARN("writerperfect", "mnLength = mxSeekable->getLength() threw exception");
                mnLength = 0;
            }
        }
    }
}

const unsigned char* WPXSvInputStream::read(unsigned long numBytes, unsigned long& numBytesRead)
{
    numBytesRead = 0;

    if (numBytes == 0 || isEnd())
        return nullptr;

    numBytesRead = mxStream->readSomeBytes(maData, numBytes);
    if (numBytesRead == 0)
        return nullptr;

    return reinterpret_cast<const unsigned char*>(maData.getConstArray());
}

tools::Long WPXSvInputStream::tellImpl()
{
    if ((mnLength == 0) || !mxStream.is() || !mxSeekable.is())
        return -1;
    else
    {
        const sal_Int64 tmpPosition = mxSeekable->getPosition();
        if (tmpPosition < 0)
            return -1;
#if SAL_TYPES_SIZEOFLONG == 4
        if (tmpPosition > LONG_MAX)
            return -1;
#endif
        return static_cast<tools::Long>(tmpPosition);
    }
}

int WPXSvInputStream::seek(tools::Long offset)
{
    if ((mnLength == 0) || !mxStream.is() || !mxSeekable.is())
        return -1;

    const sal_Int64 tmpPosition = mxSeekable->getPosition();
    if (tmpPosition < 0)
        return -1;

    try
    {
        mxSeekable->seek(offset);
    }
    catch (...)
    {
        SAL_WARN("writerperfect", "mxSeekable->seek(offset) threw exception");
        return -1;
    }

    return 0;
}

bool WPXSvInputStream::isStructured()
{
    if ((mnLength == 0) || !mxStream.is() || !mxSeekable.is())
        return false;

    PositionHolder pos(mxSeekable);
    mxSeekable->seek(0);

    if (isOLE())
        return true;

    mxSeekable->seek(0);

    return isZip();
}

unsigned WPXSvInputStream::subStreamCount()
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

    mxSeekable->seek(0);

    if (isZip())
    {
        ensureZipIsInitialized();

        return mpZipStorage->maStreams.size();
    }

    return 0;
}

const char* WPXSvInputStream::subStreamName(const unsigned id)
{
    if ((mnLength == 0) || !mxStream.is() || !mxSeekable.is())
        return nullptr;

    PositionHolder pos(mxSeekable);
    mxSeekable->seek(0);

    if (isOLE())
    {
        ensureOLEIsInitialized();

        if (mpOLEStorage->maStreams.size() <= id)
            return nullptr;

        return mpOLEStorage->maStreams[id].RVNGname.getStr();
    }

    mxSeekable->seek(0);

    if (isZip())
    {
        ensureZipIsInitialized();

        if (mpZipStorage->maStreams.size() <= id)
            return nullptr;

        return mpZipStorage->maStreams[id].aName.getStr();
    }

    return nullptr;
}

bool WPXSvInputStream::existsSubStream(const char* const name)
{
    if (!name)
        return false;

    if ((mnLength == 0) || !mxStream.is() || !mxSeekable.is())
        return false;

    PositionHolder pos(mxSeekable);
    mxSeekable->seek(0);

    const OUString aName(OStringToOUString(std::string_view(name), RTL_TEXTENCODING_UTF8));

    if (isOLE())
    {
        ensureOLEIsInitialized();
        return mpOLEStorage->maNameMap.end() != mpOLEStorage->maNameMap.find(aName);
    }

    mxSeekable->seek(0);

    if (isZip())
    {
        ensureZipIsInitialized();
        return mpZipStorage->maNameMap.end() != mpZipStorage->maNameMap.find(aName);
    }

    return false;
}

librevenge::RVNGInputStream* WPXSvInputStream::getSubStreamByName(const char* const name)
{
    if (!name)
        return nullptr;

    if ((mnLength == 0) || !mxStream.is() || !mxSeekable.is())
        return nullptr;

    PositionHolder pos(mxSeekable);
    mxSeekable->seek(0);

    const OUString aName(OStringToOUString(std::string_view(name), RTL_TEXTENCODING_UTF8));

    if (isOLE())
    {
        ensureOLEIsInitialized();
        return createWPXStream(mpOLEStorage->getStream(aName));
    }

    mxSeekable->seek(0);

    if (isZip())
    {
        ensureZipIsInitialized();

        try
        {
            return createWPXStream(mpZipStorage->getStream(aName));
        }
        catch (const Exception&)
        {
            // nothing needed
        }
    }

    return nullptr;
}

librevenge::RVNGInputStream* WPXSvInputStream::getSubStreamById(const unsigned id)
{
    if ((mnLength == 0) || !mxStream.is() || !mxSeekable.is())
        return nullptr;

    PositionHolder pos(mxSeekable);
    mxSeekable->seek(0);

    if (isOLE())
    {
        ensureOLEIsInitialized();

        if (mpOLEStorage->maStreams.size() <= id)
            return nullptr;

        return createWPXStream(mpOLEStorage->getStream(id));
    }

    mxSeekable->seek(0);

    if (isZip())
    {
        ensureZipIsInitialized();

        if (mpZipStorage->maStreams.size() <= id)
            return nullptr;

        try
        {
            return createWPXStream(mpZipStorage->getStream(id));
        }
        catch (const Exception&)
        {
            // nothing needed
        }
    }
    return nullptr;
}

librevenge::RVNGInputStream*
WPXSvInputStream::createWPXStream(const rtl::Reference<SotStorageStream>& rxStorage)
{
    if (rxStorage.is())
    {
        Reference<XInputStream> xContents(new utl::OSeekableInputStreamWrapper(rxStorage.get()));
        return new WPXSvInputStream(xContents);
    }
    return nullptr;
}

librevenge::RVNGInputStream*
WPXSvInputStream::createWPXStream(const Reference<XInputStream>& rxStream)
{
    if (rxStream.is())
        return new WPXSvInputStream(rxStream);
    else
        return nullptr;
}

bool WPXSvInputStream::isOLE()
{
    if (!mbCheckedOLE)
    {
        assert(0 == mxSeekable->getPosition());

        std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(mxStream));
        if (pStream && SotStorage::IsOLEStorage(pStream.get()))
            mpOLEStorage.reset(new OLEStorageImpl());

        mbCheckedOLE = true;
    }

    return bool(mpOLEStorage);
}

bool WPXSvInputStream::isZip()
{
    if (!mbCheckedZip)
    {
        assert(0 == mxSeekable->getPosition());

        try
        {
            const Reference<XComponentContext> xContext(comphelper::getProcessComponentContext(),
                                                        UNO_SET_THROW);
            const Reference<packages::zip::XZipFileAccess2> xZip(
                xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                    u"com.sun.star.packages.zip.ZipFileAccess"_ustr, { Any(mxStream) }, xContext),
                UNO_QUERY_THROW);
            mpZipStorage.reset(new ZipStorageImpl(xZip));
        }
        catch (const Exception&)
        {
            // ignore
        }

        mbCheckedZip = true;
    }

    return bool(mpZipStorage);
}

void WPXSvInputStream::ensureOLEIsInitialized()
{
    assert(mpOLEStorage);

    if (!mpOLEStorage->mbInitialized)
        mpOLEStorage->initialize(utl::UcbStreamHelper::CreateStream(mxStream));
}

void WPXSvInputStream::ensureZipIsInitialized()
{
    assert(mpZipStorage);

    if (!mpZipStorage->mbInitialized)
        mpZipStorage->initialize();
}

WPXSvInputStream::~WPXSvInputStream() {}

long WPXSvInputStream::tell()
{
    tools::Long retVal = tellImpl();
    return retVal + static_cast<tools::Long>(mnReadBufferPos);
}

int WPXSvInputStream::seek(long offset, librevenge::RVNG_SEEK_TYPE seekType)
{
    sal_Int64 tmpOffset = offset;
    if (seekType == librevenge::RVNG_SEEK_CUR)
        tmpOffset += tell();
    if (seekType == librevenge::RVNG_SEEK_END)
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

    if (tmpOffset < tellImpl() && o3tl::make_unsigned(tmpOffset) >= o3tl::make_unsigned(tellImpl()))
    {
        mnReadBufferPos = static_cast<unsigned long>(tmpOffset - tellImpl());
        return retVal;
    }

    if (seek(tmpOffset))
        return -1;
    return retVal;
}

bool WPXSvInputStream::isEnd()
{
    if (mnReadBufferPos != 0)
        return false;
    if ((mnLength == 0) || !mxStream.is() || !mxSeekable.is())
        return true;
    return (mxSeekable->getPosition() >= mnLength);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
