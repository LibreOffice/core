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
#ifndef _DOCSTAT_HXX
#define _DOCSTAT_HXX

#include <tools/solar.h>
#include "swdllapi.h"


struct SW_DLLPUBLIC SwDocStat
{
    sal_uInt16          nTbl;
    sal_uInt16          nGrf;
    sal_uInt16          nOLE;
    sal_uLong           nPage;
    /// paragraphs for document statistic: non-empty and non-hidden ones
    sal_uLong           nPara;
    /// all paragraphs, including empty/hidden ones
    sal_uLong           nAllPara;
    sal_uLong           nWord;
    sal_uLong           nChar;
    sal_Bool            bModified;

    SwDocStat();
    void Reset();
};

#endif
