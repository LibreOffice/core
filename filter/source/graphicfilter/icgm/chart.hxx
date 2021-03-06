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

#pragma once

#include <sal/types.h>

#include <memory>
#include <vector>

/* FILE TYPE CONSTANTS: */
#define BULCHART        32      /* Bullet chart file.       */
/* the following were added although SPC doesn't have a #define */
/* for them...                                                  */
#define AUTOTTLCHT      95      /* Autobuild TTL CHT        */
#define AUTOBULCHT      96      /* Autobuild BUL CHT        */
#define AUTOTABCHT      97      /* Autobuild TAB CHT        */

typedef struct TextEntry
{
    sal_uInt16          nTypeOfText;
    sal_uInt16          nRowOrLineNum;
    sal_uInt16          nColumnNum;
    sal_uInt16          nZoneSize;          // textzone attributes
    sal_uInt16          nLineType;
    sal_uInt16          nAttributes;
    char*           pText;              // null terminated text
} TextEntry;

struct DataNode
{
    sal_Int16           nBoxX1;
    sal_Int16           nBoxY1;
    sal_Int16           nBoxX2;
    sal_Int16           nBoxY2;
    sal_Int8            nZoneEnum;
    DataNode()
        : nBoxX1(0)
        , nBoxY1(0)
        , nBoxX2(0)
        , nBoxY2(0)
        , nZoneEnum(0)
    {
    }
};

class CGM;
class CGMImpressOutAct;
class CGMChart final
{
    friend class CGM;
    friend class CGMImpressOutAct;

        sal_Int8                mnCurrentFileType;
        ::std::vector< std::unique_ptr<TextEntry> > maTextEntryList;
        DataNode                mDataNode[ 7 ];

    public:
                                CGMChart();
                                ~CGMChart();

        void                    InsertTextEntry( std::unique_ptr<TextEntry> );

        void                    ResetAnnotation();
        bool                    IsAnnotation() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
