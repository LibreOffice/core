/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
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

    sal_Int64 SAL_CALL updateStream (::com::sun::star::uno::Reference < ::com::sun::star::io::XInputStream > & xStream)
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
