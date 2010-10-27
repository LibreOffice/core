/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
