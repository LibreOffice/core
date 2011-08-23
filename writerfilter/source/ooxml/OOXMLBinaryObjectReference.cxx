/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#include "OOXMLBinaryObjectReference.hxx"
#include <string.h>

namespace writerfilter {
namespace ooxml
{

using namespace ::com::sun::star;

OOXMLBinaryObjectReference::OOXMLBinaryObjectReference
(OOXMLStream::Pointer_t pStream)
: mpStream(pStream), mbRead(false)
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
        mSequence.realloc(nSize);

        memcpy(&mSequence[nOldSize], aSeq.getArray(), nBytesRead);
    }

    mbRead = true;
}

void OOXMLBinaryObjectReference::resolve(BinaryObj & rHandler)
{
    if (! mbRead)
        read();

    writerfilter::Reference<Properties>::Pointer_t pRef =
        writerfilter::Reference<Properties>::Pointer_t();

    rHandler.data(reinterpret_cast<sal_uInt8 *>(&mSequence[0]), 
                  mSequence.getLength(), pRef);
}

string OOXMLBinaryObjectReference::getType() const
{
    return "OOXMLBinaryObjectReference";
}

}}
