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

#include <swtblfmt.hxx>

SwTableFormat::SwTableFormat( SwAttrPool& rPool, const sal_Char* pFormatNm,
                    SwFrameFormat *pDrvdFrm )
    : SwFrameFormat( rPool, pFormatNm, pDrvdFrm, RES_FRMFMT, aTableSetRange )
    {
        pFstLineFormat = NULL;
        pLstLineFormat = NULL;
        pOddLineFormat = NULL;
        pEvnLineFormat = NULL;

        pFstColFormat = NULL;
        pLstColFormat = NULL;
        pOddColFormat = NULL;
        pEvnColFormat = NULL;
    }

SwTableFormat::SwTableFormat( SwAttrPool& rPool, const OUString &rFormatNm,
                    SwFrameFormat *pDrvdFrm )
    : SwFrameFormat( rPool, rFormatNm, pDrvdFrm, RES_FRMFMT, aTableSetRange )
    {
        pFstLineFormat = NULL;
        pLstLineFormat = NULL;
        pOddLineFormat = NULL;
        pEvnLineFormat = NULL;

        pFstColFormat = NULL;
        pLstColFormat = NULL;
        pOddColFormat = NULL;
        pEvnColFormat = NULL;
    }

SwTableLineFormat::SwTableLineFormat( SwAttrPool& rPool, const sal_Char* pFormatNm,
                    SwFrameFormat *pDrvdFrm )
    : SwFrameFormat( rPool, pFormatNm, pDrvdFrm, RES_FRMFMT, aTableLineSetRange )
    {
        pFstBoxFormat = NULL;
        pLstBoxFormat = NULL;
        pOddBoxFormat = NULL;
        pEvnBoxFormat = NULL;
    }

SwTableLineFormat::SwTableLineFormat( SwAttrPool& rPool, const OUString &rFormatNm,
                    SwFrameFormat *pDrvdFrm )
    : SwFrameFormat( rPool, rFormatNm, pDrvdFrm, RES_FRMFMT, aTableLineSetRange )
    {
        pFstBoxFormat = NULL;
        pLstBoxFormat = NULL;
        pOddBoxFormat = NULL;
        pEvnBoxFormat = NULL;
    }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
