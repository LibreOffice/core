/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WRITERPERFECT_DIRECTORYSTREAM_HXX
#define INCLUDED_WRITERPERFECT_DIRECTORYSTREAM_HXX

#include <librevenge-stream/librevenge-stream.h>

#include <com/sun/star/uno/Reference.h>

#include <writerperfectdllapi.h>

namespace com
{
namespace sun
{
namespace star
{
namespace ucb
{
class XContent;
}
}
}
}

namespace writerperfect
{

class WRITERPERFECT_DLLPUBLIC DirectoryStream : public librevenge::RVNGInputStream
{
    struct Impl;

public:
    explicit DirectoryStream(const com::sun::star::uno::Reference<com::sun::star::ucb::XContent> &xContent);
    virtual ~DirectoryStream();

    static DirectoryStream *createForParent(const com::sun::star::uno::Reference<com::sun::star::ucb::XContent> &xContent);
    static bool isDirectory(const com::sun::star::uno::Reference<com::sun::star::ucb::XContent> &xContent);

    virtual bool isStructured() SAL_OVERRIDE;
    virtual unsigned subStreamCount() SAL_OVERRIDE;
    virtual const char *subStreamName(unsigned id) SAL_OVERRIDE;
    virtual bool existsSubStream(const char *name) SAL_OVERRIDE;
    virtual librevenge::RVNGInputStream *getSubStreamByName(const char *name) SAL_OVERRIDE;
    virtual librevenge::RVNGInputStream *getSubStreamById(unsigned id) SAL_OVERRIDE;

    virtual const unsigned char *read(unsigned long numBytes, unsigned long &numBytesRead) SAL_OVERRIDE;
    virtual int seek(long offset, librevenge::RVNG_SEEK_TYPE seekType) SAL_OVERRIDE;
    virtual long tell() SAL_OVERRIDE;
    virtual bool isEnd() SAL_OVERRIDE;

private:
    Impl *m_pImpl;
};

}

#endif //  INCLUDED_WRITERPERFECT_DIRECTORYSTREAM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
