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
#ifndef INCLUDED_SW_INC_TEXTLINEBREAK_HXX
#define INCLUDED_SW_INC_TEXTLINEBREAK_HXX

#include "txatbase.hxx"

/**
 * SwTextAttr subclass that tracks the location of the wrapped SwFormatLineBreak.
 *
 * This text attribute doesn't have an end: it's specified on a new line character of an SwTextNode.
 */
class SW_DLLPUBLIC SwTextLineBreak final : public SwTextAttr
{
    SwTextNode* m_pTextNode;

public:
    SwTextLineBreak(const SfxPoolItemHolder& rAttr, sal_Int32 nStart);

    ~SwTextLineBreak() override;

    const SwTextNode& GetTextNode() const;
    void SetTextNode(SwTextNode* pNew);

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
