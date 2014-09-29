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

#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <cstring>
#include <cstdlib>

namespace sax_fastparser {

class CachedOutputStream
{
    /// realloc aligns to this value
    static const sal_Int32 mnMinimumResize = 0x1000;
    /// When buffer hits this size, it's written to mxOutputStream
    static const sal_Int32 mnMaximumSize = 0x10000;

    /// Output stream, usually writing data into files.
    css::uno::Reference< css::io::XOutputStream > mxOutputStream;
    sal_Int32 mnCacheAllocatedSize;
    sal_Int32 mnCacheWrittenSize;
    sal_Int8* mpCache;

public:
    CachedOutputStream() : mnCacheAllocatedSize(mnMinimumResize)
                         , mnCacheWrittenSize(0)
    {
        mpCache = static_cast<sal_Int8 *>(malloc(mnCacheAllocatedSize));
    }

    ~CachedOutputStream()
    {
        free(mpCache);
    }

    css::uno::Reference< css::io::XOutputStream > getOutputStream() const
    {
        return mxOutputStream;
    }

    void setOutputStream( const css::uno::Reference< css::io::XOutputStream >& xOutputStream )
    {
        mxOutputStream = xOutputStream;
    }

    /// cache string and if limit is hit, flush
    void writeBytes( const sal_Int8* pStr, sal_Int32 nLen )
    {
        // Writer does some elements sorting, so it can accumulate
        // pretty big strings in FastSaxSerializer::ForMerge.
        // In that case, just flush data and write immediately.
        if (nLen > mnMaximumSize)
        {
            flush();
            mxOutputStream->writeBytes( css::uno::Sequence<sal_Int8>(pStr, nLen) );
            return;
        }

        // Write when the buffer gets big enough
        if (mnCacheWrittenSize + nLen > mnMaximumSize)
            flush();

        sal_Int32 nMissingBytes = mnCacheWrittenSize + nLen - mnCacheAllocatedSize;
        // Ensure the buffer has enough space left
        if (nMissingBytes > 0)
        {
            // Round off to the next multiple of mnMinimumResize
            mnCacheAllocatedSize = mnCacheAllocatedSize +
                ((nMissingBytes + mnMinimumResize - 1) / mnMinimumResize) * mnMinimumResize;
            mpCache = static_cast<sal_Int8 *>(realloc(mpCache, mnCacheAllocatedSize));
        }
        assert(mnCacheWrittenSize + nLen <= mnCacheAllocatedSize);
        memcpy(mpCache + mnCacheWrittenSize, pStr, nLen);
        mnCacheWrittenSize += nLen;
    }

    /// immediately write buffer into mxOutputStream and clear
    void flush()
    {
        mxOutputStream->writeBytes( css::uno::Sequence<sal_Int8>(mpCache, mnCacheWrittenSize) );
        // and next time write to the beginning
        mnCacheWrittenSize = 0;
    }
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
