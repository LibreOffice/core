/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: binaryfilterbase.cxx,v $
 * $Revision: 1.3.22.1 $
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

#include "oox/core/binaryfilterbase.hxx"
#include "oox/helper/olestorage.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::io::XInputStream;
using ::com::sun::star::io::XStream;

namespace oox {
namespace core {

// ============================================================================

BinaryFilterBase::BinaryFilterBase( const Reference< XMultiServiceFactory >& rxGlobalFactory ) :
    FilterBase( rxGlobalFactory )
{
}

BinaryFilterBase::~BinaryFilterBase()
{
}

// private --------------------------------------------------------------------

StorageRef BinaryFilterBase::implCreateStorage( const Reference< XInputStream >& rxInStream ) const
{
    return StorageRef( new OleStorage( getGlobalFactory(), rxInStream, true ) );
}

StorageRef BinaryFilterBase::implCreateStorage( const Reference< XStream >& rxOutStream ) const
{
    return StorageRef( new OleStorage( getGlobalFactory(), rxOutStream, true ) );
}

// ============================================================================

} // namespace core
} // namespace oox

