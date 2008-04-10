/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: binarystreambase.cxx,v $
 * $Revision: 1.3 $
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

