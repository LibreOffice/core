/*************************************************************************
 *
 *  $RCSfile: stream.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:18:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#include <osl/diagnose.h>
#include <vos/object.hxx>
#include <vos/stream.hxx>

#ifdef _USE_NAMESPACE
using namespace vos;
#endif

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
    return (m_rStream.write(pbuffer, n) == n);
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
    return (OStream::getOffset());
}

