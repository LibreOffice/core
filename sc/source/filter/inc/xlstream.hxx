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



#ifndef SC_XLSTREAM_HXX
#define SC_XLSTREAM_HXX

#include <tools/stream.hxx>
#include <svx/svxerr.hxx>
#include "ftools.hxx"

// Constants ==================================================================

const sal_Size EXC_REC_SEEK_TO_BEGIN        = 0;
const sal_Size EXC_REC_SEEK_TO_END          = static_cast< sal_Size >( -1 );

const sal_uInt16 EXC_MAXRECSIZE_BIFF5       = 2080;
const sal_uInt16 EXC_MAXRECSIZE_BIFF8       = 8224;

const ErrCode EXC_ENCR_ERROR_WRONG_PASS     = ERRCODE_SVX_WRONGPASS;
const ErrCode EXC_ENCR_ERROR_UNSUPP_CRYPT   = ERRCODE_SVX_READ_FILTER_CRYPT;
const sal_uInt16 EXC_ENCR_BLOCKSIZE         = 1024;

const sal_uInt16 EXC_ID_UNKNOWN             = SAL_MAX_UINT16;
const sal_uInt16 EXC_ID_CONT                = 0x003C;

// ============================================================================

#endif

