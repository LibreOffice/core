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

#ifndef INCLUDED_PACKAGE_INFLATER_HXX
#define INCLUDED_PACKAGE_INFLATER_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <package/packagedllapi.hxx>

struct z_stream_s;

namespace ZipUtils {

class DLLPUBLIC_PACKAGE Inflater
{
    typedef struct z_stream_s z_stream;

protected:
    sal_Bool                bFinished, bSetParams, bNeedDict;
    sal_Int32               nOffset, nLength, nLastInflateError;
    z_stream*               pStream;
    com::sun::star::uno::Sequence < sal_Int8 >  sInBuffer;
    sal_Int32   doInflateBytes (com::sun::star::uno::Sequence < sal_Int8 > &rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength);

public:
    Inflater(sal_Bool bNoWrap = sal_False);
    ~Inflater();
    void SAL_CALL setInput( const ::com::sun::star::uno::Sequence< sal_Int8 >& rBuffer );
    sal_Bool SAL_CALL needsDictionary(  );
    sal_Bool SAL_CALL finished(  );
    sal_Int32 SAL_CALL doInflateSegment( ::com::sun::star::uno::Sequence< sal_Int8 >& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength );
    void SAL_CALL end(  );

    sal_Int32 getLastInflateError() { return nLastInflateError; }
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
