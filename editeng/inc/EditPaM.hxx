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

#include "ContentNode.hxx"

class EditDoc;

class EditPaM
{
private:
    ContentNode* pNode = nullptr;
    sal_Int32 nIndex = 0;

public:
    EditPaM() = default;
    EditPaM(ContentNode* p, sal_Int32 n)
        : pNode(p)
        , nIndex(n)
    {
    }

    const ContentNode* GetNode() const { return pNode; }
    ContentNode* GetNode() { return pNode; }
    void SetNode(ContentNode* p) { pNode = p; }

    sal_Int32 GetIndex() const { return nIndex; }
    void SetIndex(sal_Int32 n) { nIndex = n; }

    bool DbgIsBuggy(EditDoc const& rDoc) const;

    bool operator==(const EditPaM& rOther) const = default;

    bool operator!() const { return !pNode && !nIndex; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
