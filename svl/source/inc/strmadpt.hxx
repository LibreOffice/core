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

#ifndef INCLUDED_SVL_STRMADPT_HXX
#define INCLUDED_SVL_STRMADPT_HXX

#include <sal/config.h>

#include <com/sun/star/io/XOutputStream.hpp>
#include <tools/stream.hxx>


class SvOutputStreamOpenLockBytes: public SvOpenLockBytes
{
    com::sun::star::uno::Reference< com::sun::star::io::XOutputStream >
        m_xOutputStream;
    sal_uInt64 m_nPosition;

public:
    TYPEINFO_OVERRIDE();

    SvOutputStreamOpenLockBytes(
            const com::sun::star::uno::Reference<
                      com::sun::star::io::XOutputStream > &
                rTheOutputStream):
        m_xOutputStream(rTheOutputStream), m_nPosition(0) {}

    virtual ErrCode ReadAt(sal_uInt64, void *, sal_uLong, sal_Size *) const SAL_OVERRIDE;

    virtual ErrCode WriteAt(sal_uInt64 nPos, const void * pBuffer, sal_uLong nCount,
                            sal_Size * pWritten) SAL_OVERRIDE;

    virtual ErrCode Flush() const SAL_OVERRIDE;

    virtual ErrCode SetSize(sal_uInt64) SAL_OVERRIDE;

    virtual ErrCode Stat(SvLockBytesStat * pStat, SvLockBytesStatFlag) const SAL_OVERRIDE;

    virtual ErrCode FillAppend(const void * pBuffer, sal_uLong nCount,
                               sal_uLong * pWritten) SAL_OVERRIDE;

    virtual void Terminate() SAL_OVERRIDE;
};

#endif // INCLUDED_SVL_STRMADPT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
