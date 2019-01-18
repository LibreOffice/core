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

#ifndef INCLUDED_VCL_GENERIC_PRINT_PSPUTIL_HXX
#define INCLUDED_VCL_GENERIC_PRINT_PSPUTIL_HXX

#include <osl/file.hxx>

#include <rtl/math.hxx>
#include <rtl/ustring.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/string.hxx>
#include <rtl/tencinfo.h>
#include <rtl/textcvt.h>

#include <map>

namespace psp {

/*
 *  string convenience routines
 */
sal_Int32   getHexValueOf (sal_Int32 nValue, OStringBuffer& pBuffer);
sal_Int32   getAlignedHexValueOf (sal_Int32 nValue, OStringBuffer& pBuffer);
sal_Int32   getValueOf    (sal_Int32 nValue, OStringBuffer& pBuffer);
sal_Int32   appendStr     (const sal_Char* pSrc, OStringBuffer& pDst);

inline void getValueOfDouble( OStringBuffer& pBuffer, double f, int nPrecision = 0)
{
    pBuffer.append(rtl::math::doubleToString( f, rtl_math_StringFormat_G, nPrecision, '.', true ));
}

bool    WritePS (osl::File* pFile, const sal_Char* pString);
bool    WritePS (osl::File* pFile, const sal_Char* pString, sal_uInt64 nInLength);
bool    WritePS (osl::File* pFile, const OString &rString);
bool    WritePS (osl::File* pFile, const OUString &rString);

}  /* namespace psp */

#endif // INCLUDED_VCL_GENERIC_PRINT_PSPUTIL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
