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

#ifndef INCLUDED_SVL_INSTRM_HXX
#define INCLUDED_SVL_INSTRM_HXX

#include <svl/svldllapi.h>
#include <com/sun/star/uno/Reference.h>
#include <tools/stream.hxx>
#include <memory>

namespace com { namespace sun { namespace star { namespace io {
    class XInputStream;
    class XSeekable;
} } } }

class SvDataPipe_Impl;


class SVL_DLLPUBLIC SvInputStream: public SvStream
{
    css::uno::Reference< css::io::XInputStream >   m_xStream;
    css::uno::Reference< css::io::XSeekable >      m_xSeekable;
    std::unique_ptr<SvDataPipe_Impl>               m_pPipe;
    sal_uInt64                                     m_nSeekedFrom;

    SVL_DLLPRIVATE bool open();

    SVL_DLLPRIVATE virtual std::size_t GetData(void * pData, std::size_t nSize) override;

    SVL_DLLPRIVATE virtual std::size_t PutData(void const *, std::size_t) override;

    SVL_DLLPRIVATE virtual sal_uInt64 SeekPos(sal_uInt64 nPos) override;

    SVL_DLLPRIVATE virtual void FlushData() override;

    SVL_DLLPRIVATE virtual void SetSize(sal_uInt64) override;

public:
    SvInputStream( css::uno::Reference< css::io::XInputStream > const & rTheStream );

    virtual ~SvInputStream() override;
};

#endif // INCLUDED_SVL_INSTRM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
