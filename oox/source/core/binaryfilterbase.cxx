/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: binaryfilterbase.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:50 $
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

#include "oox/core/binaryfilterbase.hxx"
#include "oox/helper/olestorage.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::io::XInputStream;
using ::com::sun::star::io::XOutputStream;

namespace oox {
namespace core {

// ============================================================================

BinaryFilterBase::BinaryFilterBase( const Reference< XMultiServiceFactory >& rxFactory ) :
    FilterBase( rxFactory )
{
}

BinaryFilterBase::~BinaryFilterBase()
{
}

StorageRef BinaryFilterBase::implCreateStorage(
        Reference< XInputStream >& rxInStream, Reference< XOutputStream >& rxOutStream ) const
{
    StorageRef xStorage;
    if( rxInStream.is() )
        xStorage.reset( new OleStorage( getServiceFactory(), rxInStream, true ) );
    else if( rxOutStream.is() )
        xStorage.reset( new OleStorage( getServiceFactory(), rxOutStream, true ) );
    return xStorage;
}

// ============================================================================

} // namespace core
} // namespace oox

