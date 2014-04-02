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

#include <tools/solar.h>

// - SalJobSetupFlags -

#define SAL_JOBSET_ORIENTATION                  ((sal_uLong)0x00000001)
#define SAL_JOBSET_PAPERBIN                     ((sal_uLong)0x00000002)
#define SAL_JOBSET_PAPERSIZE                    ((sal_uLong)0x00000004)
#define SAL_JOBSET_DUPLEXMODE                   ((sal_uLong)0x00000008)
#define SAL_JOBSET_ALL                          (SAL_JOBSET_ORIENTATION |\
                                                 SAL_JOBSET_PAPERBIN    |\
                                                 SAL_JOBSET_PAPERSIZE   |\
                                                 SAL_JOBSET_DUPLEXMODE)

// - SalPrinterError -

#define SAL_PRINTER_ERROR_GENERALERROR          1
#define SAL_PRINTER_ERROR_ABORT                 2

// - SalPrinterProcs -

class SalPrinter;
typedef long (*SALPRNABORTPROC)( void* pInst, SalPrinter* pPrinter );

#endif // INCLUDED_VCL_INC_SALPTYPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
