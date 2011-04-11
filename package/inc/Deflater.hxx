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
#ifndef _DEFLATER_HXX_
#define _DEFLATER_HXX_

#include <com/sun/star/uno/Sequence.hxx>
#include "packagedllapi.hxx"

struct z_stream_s;

namespace ZipUtils {

class DLLPUBLIC_PACKAGE Deflater
{
    typedef struct z_stream_s z_stream;

protected:
    com::sun::star::uno::Sequence< sal_Int8 > sInBuffer;
    sal_Bool                bFinish;
    sal_Bool                bFinished;
    sal_Bool                bSetParams;
    sal_Int32               nLevel, nStrategy;
    sal_Int32               nOffset, nLength;
    z_stream*               pStream;

    void init (sal_Int32 nLevel, sal_Int32 nStrategy, sal_Bool bNowrap);
    sal_Int32 doDeflateBytes (com::sun::star::uno::Sequence < sal_Int8 > &rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength);

public:
    ~Deflater();
    Deflater(sal_Int32 nSetLevel, sal_Bool bNowrap);
    void SAL_CALL setInputSegment( const ::com::sun::star::uno::Sequence< sal_Int8 >& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength );
    void SAL_CALL setLevel( sal_Int32 nNewLevel );
    sal_Bool SAL_CALL needsInput(  );
    void SAL_CALL finish(  );
    sal_Bool SAL_CALL finished(  );
    sal_Int32 SAL_CALL doDeflateSegment( ::com::sun::star::uno::Sequence< sal_Int8 >& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength );
    sal_Int32 SAL_CALL getTotalIn(  );
    sal_Int32 SAL_CALL getTotalOut(  );
    void SAL_CALL reset(  );
    void SAL_CALL end(  );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
