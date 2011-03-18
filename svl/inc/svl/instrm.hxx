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

#ifndef SVTOOLS_INSTRM_HXX
#define SVTOOLS_INSTRM_HXX

#include "svl/svldllapi.h"
#include <com/sun/star/uno/Reference.h>
#include <tools/stream.hxx>

namespace com { namespace sun { namespace star { namespace io {
    class XInputStream;
    class XSeekable;
} } } }

class SvDataPipe_Impl;

//============================================================================
class SVL_DLLPUBLIC SvInputStream: public SvStream
{
    com::sun::star::uno::Reference< com::sun::star::io::XInputStream >
        m_xStream;
    com::sun::star::uno::Reference< com::sun::star::io::XSeekable >
        m_xSeekable;
    SvDataPipe_Impl * m_pPipe;
    sal_uLong m_nSeekedFrom;

    SVL_DLLPRIVATE bool open();

    SVL_DLLPRIVATE virtual sal_uLong GetData(void * pData, sal_uLong nSize);

    SVL_DLLPRIVATE virtual sal_uLong PutData(void const *, sal_uLong);

    SVL_DLLPRIVATE virtual sal_uLong SeekPos(sal_uLong nPos);

    SVL_DLLPRIVATE virtual void FlushData();

    SVL_DLLPRIVATE virtual void SetSize(sal_uLong);

public:
    SvInputStream(
        com::sun::star::uno::Reference< com::sun::star::io::XInputStream >
                const &
            rTheStream);

    virtual ~SvInputStream();

    virtual sal_uInt16 IsA() const;

    virtual void AddMark(sal_uLong nPos);

    virtual void RemoveMark(sal_uLong nPos);
};

#endif // SVTOOLS_INSTRM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
