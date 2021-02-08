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

#include <swcache.hxx>
#include "porlay.hxx"
#include <memory>

class SwTextFrame;

class SwTextLine : public SwCacheObj
{
    std::unique_ptr<SwParaPortion> m_pLine;

    virtual void UpdateCachePos() override;

public:
    SwTextLine(SwTextFrame const* pFrame, std::unique_ptr<SwParaPortion> pNew = nullptr);
    virtual ~SwTextLine() override;

    SwParaPortion* GetPara() { return m_pLine.get(); }
    const SwParaPortion* GetPara() const { return m_pLine.get(); }

    void SetPara(SwParaPortion* pNew, bool bDelete)
    {
        if (!bDelete)
            (void)m_pLine.release();
        m_pLine.reset(pNew);
    }
};

class SwTextLineAccess : public SwCacheAccess
{
protected:
    virtual SwCacheObj* NewObj() override;

public:
    explicit SwTextLineAccess(const SwTextFrame* pOwner);

    SwParaPortion* GetPara();

    bool IsAvailable() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
