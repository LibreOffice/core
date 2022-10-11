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
#include <com/sun/star/text/WrapTextMode.hpp>

#include <utility>

namespace writerfilter::dmapper
{
class PositionHandler : public LoggedProperties
{
public:
    PositionHandler(std::pair<OUString, OUString>& rPositionOffsets,
                    std::pair<OUString, OUString>& rAligns);
    virtual ~PositionHandler() override;
    sal_Int16 orientation() const;
    sal_Int16 relation() const { return m_nRelation; }
    sal_Int32 position() const { return m_nPosition; }
    bool GetPageToggle() const { return m_bPageToggle; }

private:
    virtual void lcl_attribute(Id aName, Value& rVal) override;
    virtual void lcl_sprm(Sprm& rSprm) override;
    sal_Int16 m_nOrient;
    sal_Int16 m_nRelation;
    sal_Int32 m_nPosition;
    std::pair<OUString, OUString>& m_rPositionOffsets;
    std::pair<OUString, OUString>& m_rAligns;
    bool m_bPageToggle = false;
};

class WrapHandler : public LoggedProperties
{
public:
    WrapHandler();
    virtual ~WrapHandler() override;

    css::text::WrapTextMode getWrapMode() const;

private:
    virtual void lcl_attribute(Id aName, Value& rVal) override;
    virtual void lcl_sprm(Sprm& rSprm) override;

    sal_Int32 m_nType;
    sal_Int32 m_nSide;
};

class ExtentHandler : public Properties
{
    css::awt::Size m_Extent; // width and height in EMU

public:
    typedef ::tools::SvRef<ExtentHandler> Pointer_t;
    explicit ExtentHandler();
    virtual ~ExtentHandler() override;

    virtual void attribute(Id nName, Value& rValue) override;
    virtual void sprm(Sprm& rSprm) override;
    css::awt::Size getExtent() const { return m_Extent; }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
