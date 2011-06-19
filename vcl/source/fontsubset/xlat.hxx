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

/*|     Author: Alexander Gelfenbain                    |*/


#ifndef __XLAT_H
#define __XLAT_H

//####include "sft.hxx"

namespace vcl
{
// TODO: sal_UCS4

    sal_uInt16 TranslateChar12(sal_uInt16);
    sal_uInt16 TranslateChar13(sal_uInt16);
    sal_uInt16 TranslateChar14(sal_uInt16);
    sal_uInt16 TranslateChar15(sal_uInt16);
    sal_uInt16 TranslateChar16(sal_uInt16);

    void TranslateString12(sal_uInt16 *, sal_uInt16 *, sal_uInt32);
    void TranslateString13(sal_uInt16 *, sal_uInt16 *, sal_uInt32);
    void TranslateString14(sal_uInt16 *, sal_uInt16 *, sal_uInt32);
    void TranslateString15(sal_uInt16 *, sal_uInt16 *, sal_uInt32);
    void TranslateString16(sal_uInt16 *, sal_uInt16 *, sal_uInt32);
}

#endif /* __XLAT_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
