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

#include <memory>
#include <unordered_map>
#include <vector>

class XMLHint_Impl;
class XMLIndexMarkHint_Impl;

class XMLHints_Impl
{
private:
    std::vector<std::unique_ptr<XMLHint_Impl>> m_Hints;
    std::unordered_map<OUString, XMLIndexMarkHint_Impl*> m_IndexHintsById;
    css::uno::Reference<css::uno::XInterface> m_xCrossRefHeadingBookmark;

public:
    void push_back(std::unique_ptr<XMLHint_Impl> pHint);

    void push_back(std::unique_ptr<XMLIndexMarkHint_Impl> pHint);

    std::vector<std::unique_ptr<XMLHint_Impl>> const& GetHints() const { return m_Hints; }

    XMLIndexMarkHint_Impl* GetIndexHintById(const OUString& sID)
    {
        auto it = m_IndexHintsById.find(sID);
        return it == m_IndexHintsById.end() ? nullptr : it->second;
    }

    css::uno::Reference<css::uno::XInterface>& GetCrossRefHeadingBookmark()
    {
        return m_xCrossRefHeadingBookmark;
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
