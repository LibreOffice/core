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

/** This enumeration contains a constant for each kind of accessible object of
    a BrowseBox. */
enum class AccessibleBrowseBoxObjType
{
    BrowseBox,           /// The BrowseBox itself.
    Table,               /// The data table.
    RowHeaderBar,        /// The row header bar ("handle column").
    ColumnHeaderBar,     /// The horizontal column header bar.
    TableCell,           /// A cell of the data table.
    RowHeaderCell,       /// A cell of the row header bar.
    ColumnHeaderCell,    /// A cell of the column header bar.
    CheckBoxCell         /// A cell with a checkbox.
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
