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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_XLSTREAM_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_XLSTREAM_HXX

#include <svx/svxerr.hxx>
#include "ftools.hxx"

// Constants ==================================================================

const std::size_t EXC_REC_SEEK_TO_BEGIN     = 0;
const std::size_t EXC_REC_SEEK_TO_END       = static_cast<std::size_t>( -1 );

const sal_uInt16 EXC_MAXRECSIZE_BIFF5       = 2080;
const sal_uInt16 EXC_MAXRECSIZE_BIFF8       = 8224;

const ErrCode EXC_ENCR_ERROR_WRONG_PASS     = ERRCODE_SVX_WRONGPASS;
const ErrCode EXC_ENCR_ERROR_UNSUPP_CRYPT   = ERRCODE_SVX_READ_FILTER_CRYPT;
const sal_uInt16 EXC_ENCR_BLOCKSIZE         = 1024;

const sal_uInt16 EXC_ID_UNKNOWN             = SAL_MAX_UINT16;
const sal_uInt16 EXC_ID_CONT                = 0x003C;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
