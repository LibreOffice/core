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

#ifndef INCLUDED_SAL_TEXTENC_CONVERTEUCTW_HXX
#define INCLUDED_SAL_TEXTENC_CONVERTEUCTW_HXX

#include "sal/config.h"

#include "sal/types.h"

struct ImplEucTwConverterData
{
    sal_uInt16 const * m_pCns116431992ToUnicodeData;
    sal_Int32 const * m_pCns116431992ToUnicodeRowOffsets;
    sal_Int32 const * m_pCns116431992ToUnicodePlaneOffsets;
    sal_uInt8 const * m_pUnicodeToCns116431992Data;
    sal_Int32 const * m_pUnicodeToCns116431992PageOffsets;
    sal_Int32 const * m_pUnicodeToCns116431992PlaneOffsets;
};

void * ImplCreateEucTwToUnicodeContext();

void ImplResetEucTwToUnicodeContext(void * pContext);

void ImplDestroyEucTwToUnicodeContext(void * pContext);

sal_Size ImplConvertEucTwToUnicode(void const * pData,
                                   void * pContext,
                                   char const * pSrcBuf,
                                   sal_Size nSrcBytes,
                                   sal_Unicode * pDestBuf,
                                   sal_Size nDestChars,
                                   sal_uInt32 nFlags,
                                   sal_uInt32 * pInfo,
                                   sal_Size * pSrcCvtBytes);

sal_Size ImplConvertUnicodeToEucTw(void const * pData,
                                   void * pContext,
                                   sal_Unicode const * pSrcBuf,
                                   sal_Size nSrcChars,
                                   char * pDestBuf,
                                   sal_Size nDestBytes,
                                   sal_uInt32 nFlags,
                                   sal_uInt32 * pInfo,
                                   sal_Size * pSrcCvtChars);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
