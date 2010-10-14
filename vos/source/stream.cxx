/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
