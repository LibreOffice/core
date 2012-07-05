/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef WPXSVSTREAM_H
#define WPXSVSTREAM_H

#include <sot/storage.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>

#include <libwpd-stream/libwpd-stream.h>

typedef struct
{
    SotStorageRef ref;
} SotStorageRefWrapper;

typedef struct
{
    SotStorageStreamRef ref;
} SotStorageStreamRefWrapper;

class WPXSvInputStream : public WPXInputStream
{
public:
    WPXSvInputStream( ::com::sun::star::uno::Reference<
                      ::com::sun::star::io::XInputStream > xStream );
    virtual ~WPXSvInputStream();

    virtual bool isOLEStream();
    virtual WPXInputStream * getDocumentOLEStream(const char *name);

    virtual const unsigned char *read(unsigned long numBytes, unsigned long &numBytesRead);
    virtual int seek(long offset, WPX_SEEK_TYPE seekType);
    virtual long tell();
    virtual bool atEOS();

private:
    std::vector< SotStorageRefWrapper > mxChildrenStorages;
    std::vector< SotStorageStreamRefWrapper > mxChildrenStreams;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::io::XInputStream > mxStream;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::io::XSeekable > mxSeekable;
    ::com::sun::star::uno::Sequence< sal_Int8 > maData;
    sal_Int64 mnLength;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
