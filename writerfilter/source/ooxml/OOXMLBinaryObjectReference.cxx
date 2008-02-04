/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OOXMLBinaryObjectReference.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2008-02-04 13:49:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
    uno::Reference<io::XInputStream> xInputStream = mpStream->getInputStream();

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
