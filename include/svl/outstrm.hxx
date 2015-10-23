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

#ifndef INCLUDED_SVL_OUTSTRM_HXX
#define INCLUDED_SVL_OUTSTRM_HXX

#include <svl/svldllapi.h>
#include <com/sun/star/uno/Reference.h>
#include <tools/stream.hxx>

namespace com { namespace sun { namespace star { namespace io {
    class XOutputStream;
} } } }


class SVL_DLLPUBLIC SvOutputStream: public SvStream
{
    css::uno::Reference< css::io::XOutputStream > m_xStream;

    SVL_DLLPRIVATE virtual sal_uLong GetData(void *, sal_uLong) override;

    SVL_DLLPRIVATE virtual sal_uLong PutData(void const * pData, sal_uLong nSize) override;

    SVL_DLLPRIVATE virtual sal_uInt64 SeekPos(sal_uInt64) override;

    SVL_DLLPRIVATE virtual void FlushData() override;

    SVL_DLLPRIVATE virtual void SetSize(sal_uInt64) override;

public:
    SvOutputStream(css::uno::Reference< css::io::XOutputStream > const & rTheStream);

    virtual ~SvOutputStream();
};

#endif // INCLUDED_SVL_OUTSTRM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
