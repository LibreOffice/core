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

/*|     Author: Alexander Gelfenbain                    |*/

#ifndef INCLUDED_VCL_SOURCE_FONTSUBSET_XLAT_HXX
#define INCLUDED_VCL_SOURCE_FONTSUBSET_XLAT_HXX

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

#endif // INCLUDED_VCL_SOURCE_FONTSUBSET_XLAT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
