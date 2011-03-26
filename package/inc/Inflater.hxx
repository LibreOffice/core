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
#ifndef _INFLATER_HXX_
#define _INFLATER_HXX_

#include <com/sun/star/uno/Sequence.hxx>
#include "packagedllapi.hxx"

extern "C"
{
    typedef struct z_stream_s z_stream;
}
class DLLPUBLIC_PACKAGE Inflater
{
protected:
    sal_Bool                bFinish, bFinished, bSetParams, bNeedDict;
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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
