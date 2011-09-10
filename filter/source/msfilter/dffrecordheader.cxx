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

#include "precompiled_filter.hxx"

#include <filter/msfilter/dffrecordheader.hxx>

SvStream& operator>>( SvStream& rIn, DffRecordHeader& rRec )
{
    rRec.nFilePos = rIn.Tell();
    sal_uInt16 nTmp(0);
    rIn >> nTmp;
    rRec.nImpVerInst = nTmp;
    rRec.nRecVer = sal::static_int_cast< sal_uInt8 >(nTmp & 0x000F);
    rRec.nRecInstance = nTmp >> 4;
    rIn >> rRec.nRecType;
    rIn >> rRec.nRecLen;
    return rIn;
}

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
