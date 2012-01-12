/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SV_SALPTYPE_HXX
#define _SV_SALPTYPE_HXX

#include <vcl/sv.h>

// --------------------
// - SalJobSetupFlags -
// --------------------

#define SAL_JOBSET_ORIENTATION                  ((sal_uLong)0x00000001)
#define SAL_JOBSET_PAPERBIN                     ((sal_uLong)0x00000002)
#define SAL_JOBSET_PAPERSIZE                    ((sal_uLong)0x00000004)
#define SAL_JOBSET_DUPLEXMODE                   ((sal_uLong)0x00000008)
#define SAL_JOBSET_ALL                          (SAL_JOBSET_ORIENTATION |\
                                                 SAL_JOBSET_PAPERBIN    |\
                                                 SAL_JOBSET_PAPERSIZE   |\
                                                 SAL_JOBSET_DUPLEXMODE)

// -------------------
// - SalPrinterError -
// -------------------

#define SAL_PRINTER_ERROR_GENERALERROR          1
#define SAL_PRINTER_ERROR_ABORT                 2

// -------------------
// - SalPrinterProcs -
// -------------------

class SalPrinter;
typedef long (*SALPRNABORTPROC)( void* pInst, SalPrinter* pPrinter );

#endif // _SV_SALPTYPE_HXX
