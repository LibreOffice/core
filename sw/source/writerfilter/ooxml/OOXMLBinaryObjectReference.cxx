/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#include "OOXMLBinaryObjectReference.hxx"
#include <string.h>

#include <utility>

namespace writerfilter::ooxml
{

using namespace ::com::sun::star;

OOXMLBinaryObjectReference::OOXMLBinaryObjectReference
(OOXMLStream::Pointer_t pStream)
: mpStream(std::move(pStream)), mbRead(false)
{
}

OOXMLBinaryObjectReference::~OOXMLBinaryObjectReference()
{
}

void OOXMLBinaryObjectReference::read()
{
    sal_uInt32 nMaxReadBytes = 1024*1024;
    uno::Sequence<sal_Int8> aSeq(nMaxReadBytes);
    uno::Reference<io::XInputStream> xInputStream =
        mpStream->getDocumentStream();

    sal_uInt32 nSize = 0;
    sal_uInt32 nOldSize = 0;
    sal_uInt32 nBytesRead = 0;

    while ((nBytesRead = xInputStream->readSomeBytes(aSeq, nMaxReadBytes)) > 0)
    {
        nOldSize = nSize;
        nSize += nBytesRead;
        mSequence.resize(nSize);

        memcpy(&mSequence[nOldSize], aSeq.getArray(), nBytesRead);
    }

    mbRead = true;
}

void OOXMLBinaryObjectReference::resolve(BinaryObj & rHandler)
{
    if (! mbRead)
        read();

    rHandler.data(reinterpret_cast<sal_uInt8 *>(mSequence.data()),
                  mSequence.size());
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
