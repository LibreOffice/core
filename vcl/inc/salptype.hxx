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

#ifndef INCLUDED_VCL_INC_SALPTYPE_HXX
#define INCLUDED_VCL_INC_SALPTYPE_HXX

#include <sal/config.h>

#include <o3tl/typed_flags_set.hxx>
#include <sal/types.h>

enum class JobSetFlags : sal_uInt16 {
    ORIENTATION  = 1,
    PAPERBIN     = 2,
    PAPERSIZE    = 4,
    DUPLEXMODE   = 8,
    ALL          = ORIENTATION | PAPERBIN | PAPERSIZE | DUPLEXMODE
};

namespace o3tl {

template<> struct typed_flags<JobSetFlags>: is_typed_flags<JobSetFlags, 0xF> {};

}

#define SAL_PRINTER_ERROR_GENERALERROR          1
#define SAL_PRINTER_ERROR_ABORT                 2

class SalPrinter;
typedef long (*SALPRNABORTPROC)( void* pInst, SalPrinter* pPrinter );

#endif // INCLUDED_VCL_INC_SALPTYPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
