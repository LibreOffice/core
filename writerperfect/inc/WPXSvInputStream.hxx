/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WRITERPERFECT_WPXSVINPUTSTREAM_HXX
#define INCLUDED_WRITERPERFECT_WPXSVINPUTSTREAM_HXX

#include <librevenge-stream/librevenge-stream.h>
#include <com/sun/star/uno/Reference.h>
#include <writerperfectdllapi.h>
#include <memory>

namespace com
{
namespace sun
{
namespace star
{
namespace io
{
class XInputStream;
class XSeekable;
}
}
}
}

namespace writerperfect
{

class WPXSvInputStreamImpl;

class WRITERPERFECT_DLLPUBLIC WPXSvInputStream : public librevenge::RVNGInputStream
{
public:
    WPXSvInputStream(css::uno::Reference< css::io::XInputStream > xStream);
    virtual ~WPXSvInputStream();

    virtual bool isStructured() override;
    virtual unsigned subStreamCount() override;
    virtual const char *subStreamName(unsigned id) override;
    virtual bool existsSubStream(const char *name) override;
    virtual librevenge::RVNGInputStream *getSubStreamByName(const char *name) override;
    virtual librevenge::RVNGInputStream *getSubStreamById(unsigned id) override;

    virtual const unsigned char *read(unsigned long numBytes, unsigned long &numBytesRead) override;
    virtual int seek(long offset, librevenge::RVNG_SEEK_TYPE seekType) override;
    virtual long tell() override;
    virtual bool isEnd() override;

private:
    std::unique_ptr<WPXSvInputStreamImpl> mpImpl;
};

}

#endif // INCLUDED_WRITERPERFECT_WPXSVINPUTSTREAM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
