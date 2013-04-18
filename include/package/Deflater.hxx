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
    sal_Int64               nOffset, nLength;
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
    sal_Int64 SAL_CALL getTotalIn(  );
    sal_Int64 SAL_CALL getTotalOut(  );
    void SAL_CALL reset(  );
    void SAL_CALL end(  );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
