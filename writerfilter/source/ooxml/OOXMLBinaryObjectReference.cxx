/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
