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

#include "TablePropertiesHandler.hxx"

#include "DomainMapper.hxx"
#include "LoggedResources.hxx"
#include <memory>
#include <vector>

namespace writerfilter::dmapper {

class DomainMapper;

enum class TblStyleType
{
    Unknown,
    WholeTable,
    FirstRow,
    LastRow,
    FirstCol,
    LastCol,
    Band1Vert,
    Band2Vert,
    Band1Horz,
    Band2Horz,
    NECell,
    NWCell,
    SECell,
    SWCell
};

class TblStylePrHandler : public LoggedProperties
{
private:
    DomainMapper &              m_rDMapper;
    std::unique_ptr<TablePropertiesHandler>  m_pTablePropsHandler;

    TblStyleType                m_nType;
    PropertyMapPtr              m_pProperties;
    std::vector<css::beans::PropertyValue> m_aInteropGrabBag;

    // Properties
    virtual void lcl_attribute(Id Name, const Value & val) override;
    virtual void lcl_sprm(Sprm & sprm) override;

public:
    explicit TblStylePrHandler( DomainMapper & rDMapper );
    virtual ~TblStylePrHandler( ) override;

    const PropertyMapPtr&       getProperties() const { return m_pProperties; };
    TblStyleType         getType() const { return m_nType; };
    OUString getTypeString() const;
    void appendInteropGrabBag(const OUString& aKey, const OUString& aValue);
    css::beans::PropertyValue getInteropGrabBag(const OUString& aName);

private:

    void resolveSprmProps(Sprm & rSprm);
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
