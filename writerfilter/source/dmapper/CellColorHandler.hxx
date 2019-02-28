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
#ifndef INCLUDED_WRITERFILTER_SOURCE_DMAPPER_CELLCOLORHANDLER_HXX
#define INCLUDED_WRITERFILTER_SOURCE_DMAPPER_CELLCOLORHANDLER_HXX

#include "LoggedResources.hxx"
#include "PropertyMap.hxx"
#include <memory>
#include <vector>

#include <com/sun/star/beans/PropertyValue.hpp>

namespace writerfilter {
namespace dmapper
{
class TablePropertyMap;
class CellColorHandler : public LoggedProperties
{
public:
    enum OutputFormat { Form, Paragraph, Character }; // for what part of the document
private:
    sal_Int32 m_nShadingPattern;
    sal_Int32 m_nColor;
    sal_Int32 m_nFillColor;
    bool      m_bAutoFillColor;
    bool      m_bFillSpecified;
    OutputFormat m_OutputFormat;

    OUString m_aInteropGrabBagName;
    std::vector<css::beans::PropertyValue> m_aInteropGrabBag;

    // Properties
    virtual void lcl_attribute(Id Name, Value & val) override;
    virtual void lcl_sprm(Sprm & sprm) override;

    void createGrabBag(const OUString& aName, const css::uno::Any& rValue);

public:
    CellColorHandler( );
    virtual ~CellColorHandler() override;

    TablePropertyMapPtr getProperties();

    void setOutputFormat( OutputFormat format ) { m_OutputFormat = format; }

    void enableInteropGrabBag(const OUString& aName);
    css::beans::PropertyValue getInteropGrabBag();
    void disableInteropGrabBag();
    bool isInteropGrabBagEnabled();
};
}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
