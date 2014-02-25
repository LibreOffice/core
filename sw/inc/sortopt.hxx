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
#ifndef INCLUDED_SW_INC_SORTOPT_HXX
#define INCLUDED_SW_INC_SORTOPT_HXX

#include <rtl/ustring.hxx>
#include "swdllapi.h"
#include <vector>

enum SwSortOrder        { SRT_ASCENDING, SRT_DESCENDING };
enum SwSortDirection    { SRT_COLUMNS, SRT_ROWS         };

struct SW_DLLPUBLIC SwSortKey
{
    SwSortKey();
    SwSortKey( sal_uInt16 nId, const OUString& rSrtType, SwSortOrder eOrder );
    SwSortKey( const SwSortKey& rOld );

    OUString        sSortType;
    SwSortOrder     eSortOrder;
    sal_uInt16      nColumnId;
    bool            bIsNumeric;
};

typedef std::vector<SwSortKey*> SwSortKeys;

struct SW_DLLPUBLIC SwSortOptions
{
    SwSortOptions();
    ~SwSortOptions();
    SwSortOptions(const SwSortOptions& rOpt);

    SwSortKeys      aKeys;
    SwSortDirection eDirection;
    sal_Unicode     cDeli;
    sal_uInt16          nLanguage;
    bool            bTable;
    bool            bIgnoreCase;
};

#endif // INCLUDED_SW_INC_SORTOPT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
