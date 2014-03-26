/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef DIRECTORYSTREAM_H_INCLUDED
#define DIRECTORYSTREAM_H_INCLUDED

#include <libwpd-stream/libwpd-stream.h>

#include <com/sun/star/uno/Reference.h>

namespace com { namespace sun { namespace star { namespace ucb {
    class XContent;
} } } }

namespace writerperfect
{

class DirectoryStream : public WPXInputStream
{
    struct Impl;

public:
    explicit DirectoryStream(const com::sun::star::uno::Reference<com::sun::star::ucb::XContent> &xContent);
    virtual ~DirectoryStream();

    virtual bool isOLEStream() SAL_OVERRIDE;
    virtual WPXInputStream *getDocumentOLEStream(const char *pName) SAL_OVERRIDE;

    virtual const unsigned char *read(unsigned long nNumBytes, unsigned long &nNumBytesRead) SAL_OVERRIDE;
    virtual int seek(long nOffset, WPX_SEEK_TYPE eSeekType) SAL_OVERRIDE;
    virtual long tell() SAL_OVERRIDE;
    virtual bool atEOS() SAL_OVERRIDE;

private:
    Impl *m_pImpl;
};

}

#endif //  DIRECTORYSTREAM_H_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
