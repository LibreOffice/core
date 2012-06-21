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

void test()
{
    portion = appendPortion("text");
    tableContext = new TableContext();
    {
        tableStack.push(tableContext);
        portion = appendPortion("A1");
        tableContext.addPortion(portion);
        tableContext.endCell();
        portion = appendPortion("B1");
        tableContext.addPortion(portion);
        tableContext.endCell();
        portion = appendPortion("C1");
        tableContext.addPortion(portion);
        tableContext.endCell();
        tableContext.endRow(rowProperties);
        portion = appendPortion("A2");
        tableContext.addPortion(portion);
        tableContext.endCell();
        tableContext = new TableContext();
        {
            tableStack.push(tableContext);
            portion = appendPortion("B2A1");
            tableContext.addPortion(portion);
            tableContext.endCell();
            portion = appendPortion("B2B1");
            tableContext.addPortion(portion);
            tableContext.endCell();
            tableContext.endRow(rowProperties);
            portion = appendPortion("B2A2");
            tableContext.addPortion(portion);
            tableContext.endCell();
            portion = appendPortion("B2B2");
            tableContext.addPortion(portion);
            tableContext.endCell();
            tableContext.endRow(rowProperties);
        }
        portion = createTable(tableContext);
        tableContext = tableStack.pop();
        tableContext.addPortion(portion);
        portion = appendPortion("B2");
        tableContext.addPortion(portion);
        tableContext.endCell();
        portion = appendPortion("C2");
        tableContext.addPortion(portion);
        tableContext.endCell();
        tableContext.endRow(rowProperties);
    }
    portion = createTable(tableContext);
    portion = appendPortion("text");
}

/*
  tableContext.endCell:
    merge text ranges of portions to one and add this range to ranges of row.
 */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
