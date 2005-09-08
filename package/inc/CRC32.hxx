/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CRC32.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 15:54:49 $
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
#ifndef _CRC32_HXX
#define _CRC32_HXX

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif

namespace com { namespace sun { namespace star {
    namespace io { class XInputStream; }
} } }
class CRC32
{
protected:
    sal_uInt32 nCRC;
public:
    CRC32();
    ~CRC32();

    sal_Int32 SAL_CALL updateStream (::com::sun::star::uno::Reference < ::com::sun::star::io::XInputStream > & xStream)
        throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL updateByte (sal_Int8 nByte)
        throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL updateSegment(const ::com::sun::star::uno::Sequence< sal_Int8 > &b, sal_Int32 off, sal_Int32 len)
        throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL update(const ::com::sun::star::uno::Sequence< sal_Int8 > &b)
        throw(::com::sun::star::uno::RuntimeException);
    sal_Int32 SAL_CALL getValue()
        throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL reset()
        throw(::com::sun::star::uno::RuntimeException);
};

#endif
