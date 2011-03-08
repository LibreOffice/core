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
#ifndef _CRC32_HXX
#define _CRC32_HXX

#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/RuntimeException.hpp>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
