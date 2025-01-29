/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SAX_SOURCE_TOOLS_CACHEDOUTPUTSTREAM_HXX
#define INCLUDED_SAX_SOURCE_TOOLS_CACHEDOUTPUTSTREAM_HXX

#include <sal/types.h>
#include <rtl/byteseq.hxx>

#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <cstring>
#include <memory>

namespace sax_fastparser {

class ForMergeBase
{
public:
    virtual ~ForMergeBase() {}
    virtual void append( const css::uno::Sequence<sal_Int8>& rWhat ) = 0;
};

class CachedOutputStream
{
    /// When buffer hits this size, it's written to mxOutputStream
    static const sal_Int32 mnMaximumSize = 0x100000; // 1Mbyte

    /// ForMerge structure is used for sorting elements in Writer
    std::shared_ptr< ForMergeBase > mpForMerge;
    const rtl::ByteSequence maCache;
    /// Output stream, usually writing data into files.
    css::uno::Reference< css::io::XOutputStream > mxOutputStream;
    uno_Sequence *pSeq;
    sal_Int32 mnCacheWrittenSize;
    bool mbWriteToOutStream;

public:
    CachedOutputStream() : maCache(mnMaximumSize, rtl::BYTESEQ_NODEFAULT)
                         , pSeq(maCache.get())
                         , mnCacheWrittenSize(0)
                         , mbWriteToOutStream(true)
    {}

    const css::uno::Reference< css::io::XOutputStream >& getOutputStream() const
    {
        return mxOutputStream;
    }

    void setOutputStream( const css::uno::Reference< css::io::XOutputStream >& xOutputStream )
    {
        mxOutputStream = xOutputStream;
    }

    void setOutput(const std::shared_ptr<ForMergeBase>& pForMerge)
    {
        flush();
        mbWriteToOutStream = false;
        mpForMerge = pForMerge;
    }

    void resetOutputToStream()
    {
        flush();
        mbWriteToOutStream = true;
        mpForMerge.reset();
    }

    /// cache string and if limit is hit, flush
    void writeBytes( const sal_Int8* pStr, sal_Int32 nLen )
    {
        // Write when the buffer gets big enough
        if (mnCacheWrittenSize + nLen > mnMaximumSize)
        {
            flush();

            // Writer does some elements sorting, so it can accumulate
            // pretty big strings in FastSaxSerializer::ForMerge.
            // In that case, just flush data and write immediately.
            if (nLen > mnMaximumSize)
            {
                if (mbWriteToOutStream)
                    mxOutputStream->writeBytes( css::uno::Sequence<sal_Int8>(pStr, nLen) );
                else
                    mpForMerge->append( css::uno::Sequence<sal_Int8>(pStr, nLen) );
                return;
            }
        }

        memcpy(pSeq->elements + mnCacheWrittenSize, pStr, nLen);
        mnCacheWrittenSize += nLen;
    }

    /// immediately write buffer into mxOutputStream and clear
    void flush()
    {
        // resize the Sequence to written size
        pSeq->nElements = mnCacheWrittenSize;
        if (mbWriteToOutStream)
            mxOutputStream->writeBytes( css::uno::toUnoSequence(maCache) );
        else
            mpForMerge->append( css::uno::toUnoSequence(maCache) );
        // and next time write to the beginning
        mnCacheWrittenSize = 0;
    }
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
