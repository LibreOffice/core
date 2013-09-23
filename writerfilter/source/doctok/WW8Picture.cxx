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

#include <doctok/resources.hxx>

namespace writerfilter {
namespace doctok
{

void WW8PICF::resolveNoAuto(Properties & /*rHandler*/)
{
}

writerfilter::Reference<Properties>::Pointer_t
WW8PICF::get_DffRecord()
{
    writerfilter::Reference<Properties>::Pointer_t
        pRet(new DffBlock(this, get_cbHeader(), getCount() - get_cbHeader(),
                          0));
    return pRet;
}

writerfilter::Reference<Properties>::Pointer_t
WW8PICF::get_ffdata()
{
    writerfilter::Reference<Properties>::Pointer_t
        pRet(new WW8FFDATA(this, get_cbHeader(), getCount() - get_cbHeader()));

    WW8StructBase::Pointer_t pStruct
        (new WW8StructBase(this, get_cbHeader(),
                           getCount() - get_cbHeader()));

    pStruct->dump(output);

    return pRet;
}

writerfilter::Reference<Properties>::Pointer_t
WW8FSPA::get_shape()
{
    return writerfilter::Reference<Properties>::Pointer_t();
}

void WW8FFDATA::resolveNoAuto(Properties & /*rHandler*/)
{
}

sal_uInt32 WW8FFDATA::get_FLT()
{
    sal_uInt32 nResult = 0;
    return nResult;
}

static sal_uInt32 lcl_FFDATA_default_offset(WW8FFDATA & rRef)
{
    return 0xa + (rRef.getU16(0xa) + 2) * 2;
}

static sal_uInt32 lcl_FFDATA_formatting_offset(WW8FFDATA & rRef)
{
    sal_uInt32 nResult = lcl_FFDATA_default_offset(rRef);

    switch (rRef.get_FLT())
    {
    case 71: // forms checkbox
    case 83: // forms listbox
        nResult += 2;
        break;
    default:
        nResult += (rRef.getU16(nResult) + 2) * 2;
        break;
    }

    return nResult;
}

static sal_uInt32 lcl_FFDATA_help_offset(WW8FFDATA & rRef)
{
    sal_uInt32 nResult = lcl_FFDATA_formatting_offset(rRef);

    nResult += (rRef.getU16(nResult) + 2) * 2;

    return nResult;
}

static sal_uInt32 lcl_FFDATA_tooltip_offset(WW8FFDATA & rRef)
{
    sal_uInt32 nResult = lcl_FFDATA_help_offset(rRef);

    nResult += (rRef.getU16(nResult) + 2) * 2;

    return nResult;
}

OUString WW8FFDATA::get_default()
{
    OUString sResult;

    sal_uInt32 nOffset = lcl_FFDATA_default_offset(*this);
    switch (get_FLT())
    {
    case 70:
        sResult = getString(nOffset);

        break;
    default:
        break;
    }

    return sResult;
}

OUString WW8FFDATA::get_formatting()
{
    return getString(lcl_FFDATA_formatting_offset(*this));
}

OUString WW8FFDATA::get_help()
{
    return getString(lcl_FFDATA_help_offset(*this));
}

OUString WW8FFDATA::get_tooltip()
{
    OUString sResult;

    sResult = getString(lcl_FFDATA_tooltip_offset(*this));

    return sResult;
}

sal_uInt16 WW8FFDATA::get_checked()
{
    sal_uInt16 nResult = 0;

    switch (get_FLT())
    {
    case 71:
        nResult = getU16(lcl_FFDATA_default_offset(*this));
        break;
    default:
        break;
    }

    return nResult;
}
}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
