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

#include "LoggedResources.hxx"
#include <vector>

namespace writerfilter::dmapper
{
struct Column_
{
    sal_Int32 nWidth;
    sal_Int32 nSpace;
};


class SectionColumnHandler : public LoggedProperties
{
    bool        m_bEqualWidth;
    sal_Int32   m_nSpace;
    sal_Int32   m_nNum;
    bool        m_bSep;
    std::vector<Column_> m_aCols;

    Column_   m_aTempColumn;

    // Properties
    virtual void lcl_attribute(Id Name, Value & val) override;
    virtual void lcl_sprm(Sprm & sprm) override;

public:
    SectionColumnHandler();
    virtual ~SectionColumnHandler() override;

    bool        IsEqualWidth() const { return m_bEqualWidth; }
    sal_Int32   GetSpace() const { return m_nSpace; }
    sal_Int32   GetNum() const { return m_nNum; }
    bool        IsSeparator() const { return m_bSep; }

    const std::vector<Column_>& GetColumns() const { return m_aCols;}

};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
