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

#include <config_options.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <package/packagedllapi.hxx>
#include <memory>

struct z_stream_s;

namespace ZipUtils {

class UNLESS_MERGELIBS(DLLPUBLIC_PACKAGE) Inflater final
{
    typedef struct z_stream_s z_stream;

    bool                    bFinished, bNeedDict;
    sal_Int32               nOffset, nLength, nLastInflateError;
    std::unique_ptr<z_stream>  pStream;
    css::uno::Sequence < sal_Int8 >  sInBuffer;
    sal_Int32   doInflateBytes (css::uno::Sequence < sal_Int8 > &rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength);

public:
    Inflater(bool bNoWrap);
    ~Inflater();
    void setInput( const css::uno::Sequence< sal_Int8 >& rBuffer );
    bool needsDictionary() const { return bNeedDict; }
    bool finished() const { return bFinished; }
    sal_Int32 doInflateSegment( css::uno::Sequence< sal_Int8 >& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength );
    void end(  );

    sal_Int32 getLastInflateError() const { return nLastInflateError; }
};

class UNLESS_MERGELIBS(DLLPUBLIC_PACKAGE) InflaterBytes final
{
    typedef struct z_stream_s z_stream;

    bool                    bFinished;
    sal_Int32               nOffset, nLength;
    std::unique_ptr<z_stream>  pStream;
    const sal_Int8*  sInBuffer;
    sal_Int32   doInflateBytes (sal_Int8* pOutBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength);

public:
    InflaterBytes();
    ~InflaterBytes();
    void setInput( const sal_Int8* pBuffer, sal_Int32 nLen );
    bool finished() const { return bFinished; }
    sal_Int32 doInflateSegment( sal_Int8* pOutBuffer, sal_Int32 nBufLen, sal_Int32 nNewOffset, sal_Int32 nNewLength );
    void end(  );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
