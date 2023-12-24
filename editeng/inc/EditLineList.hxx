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

#include <vector>
#include <sal/types.h>

class EditLineList
{
    typedef std::vector<std::unique_ptr<EditLine>> LinesType;
    LinesType maLines;

public:
    EditLineList();
    ~EditLineList();

    void Reset();
    void DeleteFromLine(sal_Int32 nDelFrom);
    sal_Int32 FindLine(sal_Int32 nChar, bool bInclEnd);
    sal_Int32 Count() const;
    const EditLine& operator[](sal_Int32 nPos) const;
    EditLine& operator[](sal_Int32 nPos);

    void Append(EditLine* p);
    void Insert(sal_Int32 nPos, EditLine* p);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
