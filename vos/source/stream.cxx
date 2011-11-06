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



#include <limits>

#include <osl/diagnose.h>
#include <vos/object.hxx>
#include <vos/stream.hxx>

using namespace vos;

/////////////////////////////////////////////////////////////////////////////
//
// Stream class
//

VOS_IMPLEMENT_CLASSINFO(VOS_CLASSNAME(OStream, vos), VOS_NAMESPACE(OStream, vos), VOS_NAMESPACE(OObject, vos), 0);

OStream::OStream(IPositionableStream& rStream)
    : m_rStream(rStream)
{
}

OStream::~OStream()
{
}

sal_Int32 OStream::read(void* pbuffer, sal_uInt32 n) const
{
    return (m_rStream.read(pbuffer, n));
}

sal_Int32 OStream::read(IPositionableStream::Offset offset,
                      void* pbuffer, sal_uInt32 n) const
{
    return (seekTo(offset) ? read(pbuffer, n) : -1);
}

sal_Int32 OStream::write(const void* pbuffer, sal_uInt32 n)
{
    return
        n <= static_cast< sal_uInt32 >(std::numeric_limits< sal_Int32 >::max())
        && (m_rStream.write(pbuffer, n) == static_cast< sal_Int32 >(n));
}

sal_Int32 OStream::write(IPositionableStream::Offset offset,
                       const void* pbuffer, sal_uInt32 n)
{
    return (seekTo(offset) && write(pbuffer, n));
}

sal_Bool OStream::append(void* pbuffer, sal_uInt32 n)
{
    return (seekToEnd() && write(pbuffer, n));
}

sal_Bool OStream::seekTo(IPositionableStream::Offset pos) const
{
    return (m_rStream.seekTo(pos));
}

sal_Bool OStream::seekToEnd() const
{
    return (m_rStream.seekToEnd());
}

sal_Bool OStream::seekRelative(sal_Int32 change) const
{
    return (m_rStream.seekRelative(change));
}

sal_Bool OStream::changeSize(sal_uInt32 new_size)
{
    return (m_rStream.changeSize(new_size));
}

sal_uInt32 OStream::getSize() const
{
    return (m_rStream.getSize());
}

sal_Bool OStream::isEof() const
{
    return (m_rStream.isEof());
}

IPositionableStream::Offset OStream::getOffset() const
{
    return (m_rStream.getOffset());
}

