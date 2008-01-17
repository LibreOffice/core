/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: binarystreambase.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:59 $
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

#include "oox/helper/binarystreambase.hxx"
#include <osl/diagnose.h>

using ::com::sun::star::uno::Exception;

namespace oox {

// ============================================================================

BinaryStreamBase::~BinaryStreamBase()
{
}

sal_Int64 BinaryStreamBase::getLength() const
{
    try
    {
        return mxSeekable.is() ? mxSeekable->getLength() : -1;
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, "BinaryStreamBase::getLength - exception caught" );
    }
    return -1;
}

sal_Int64 BinaryStreamBase::tell() const
{
    try
    {
        return mxSeekable.is() ? mxSeekable->getPosition() : -1;
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, "BinaryStreamBase::tell - exception caught" );
    }
    return -1;
}

void BinaryStreamBase::seek( sal_Int64 nPos )
{
    try
    {
        if( mxSeekable.is() )
            mxSeekable->seek( nPos );
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, "BinaryStreamBase::seek - exception caught" );
    }
}

// ============================================================================

} // namespace oox

