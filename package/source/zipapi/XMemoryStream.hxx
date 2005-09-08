/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMemoryStream.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:14:15 $
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
#ifndef _XMEMORY_STREAM_HXX
#define _XMEMORY_STREAM_HXX

#ifndef _ZIP_PACKAGE_BUFFER_HXX
#include <ZipPackageBuffer.hxx>
#endif

class ZipPackage;

class XMemoryStream: public ZipPackageBuffer
{
public:
    XMemoryStream ( com::sun::star::uno::Sequence < sal_Int8 > & rNewBuffer );
    virtual ~XMemoryStream(void);
    virtual com::sun::star::uno::Any SAL_CALL queryInterface( const com::sun::star::uno::Type& rType )
        throw(com::sun::star::uno::RuntimeException);
};
#endif
