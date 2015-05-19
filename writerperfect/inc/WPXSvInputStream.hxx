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

namespace com { namespace sun { namespace star { namespace io {
    class XInputStream;
    class XSeekable;
} } } }

namespace writerperfect
{

class WPXSvInputStreamImpl;

class WRITERPERFECT_DLLPUBLIC WPXSvInputStream : public librevenge::RVNGInputStream
{
public:
    WPXSvInputStream( ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > xStream );
    virtual ~WPXSvInputStream();

    virtual bool isStructured() SAL_OVERRIDE;
    virtual unsigned subStreamCount() SAL_OVERRIDE;
    virtual const char * subStreamName(unsigned id) SAL_OVERRIDE;
    virtual bool existsSubStream(const char *name) SAL_OVERRIDE;
    virtual librevenge::RVNGInputStream * getSubStreamByName(const char *name) SAL_OVERRIDE;
    virtual librevenge::RVNGInputStream * getSubStreamById(unsigned id) SAL_OVERRIDE;

    virtual const unsigned char *read(unsigned long numBytes, unsigned long &numBytesRead) SAL_OVERRIDE;
    virtual int seek(long offset, librevenge::RVNG_SEEK_TYPE seekType) SAL_OVERRIDE;
    virtual long tell() SAL_OVERRIDE;
    virtual bool isEnd() SAL_OVERRIDE;

private:
    WPXSvInputStreamImpl *mpImpl;
};

}

#endif // INCLUDED_WRITERPERFECT_WPXSVINPUTSTREAM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
