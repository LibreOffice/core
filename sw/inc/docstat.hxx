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
#ifndef INCLUDED_SW_INC_DOCSTAT_HXX
#define INCLUDED_SW_INC_DOCSTAT_HXX

#include <tools/solar.h>
#include "swdllapi.h"

struct SW_DLLPUBLIC SwDocStat
{
    sal_uInt16          nTable;
    sal_uInt16          nGrf;
    sal_uInt16          nOLE;
    sal_uLong           nPage;
    /// paragraphs for document statistic: non-empty and non-hidden ones
    sal_uLong           nPara;
    /// all paragraphs, including empty/hidden ones
    sal_uLong           nAllPara;
    sal_uLong           nWord;
    sal_uLong           nAsianWord;
    sal_uLong           nChar;
    sal_uLong           nCharExcludingSpaces;
    bool            bModified;

    SwDocStat();
    void Reset();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
