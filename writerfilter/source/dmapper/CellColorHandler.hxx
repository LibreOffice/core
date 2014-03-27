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
#ifndef INCLUDED_CELLCOLORHANDLER_HXX
#define INCLUDED_CELLCOLORHANDLER_HXX

#include <WriterFilterDllApi.hxx>
#include <resourcemodel/LoggedResources.hxx>
#include <boost/shared_ptr.hpp>

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
    OutputFormat m_OutputFormat;

    OUString m_aInteropGrabBagName;
    std::vector<beans::PropertyValue> m_aInteropGrabBag;

    // Properties
    virtual void lcl_attribute(Id Name, Value & val) SAL_OVERRIDE;
    virtual void lcl_sprm(Sprm & sprm) SAL_OVERRIDE;

    void createGrabBag(const OUString& aName, uno::Any aValue);

public:
    CellColorHandler( );
    virtual ~CellColorHandler();

    ::boost::shared_ptr<TablePropertyMap>            getProperties();

    void setOutputFormat( OutputFormat format ) { m_OutputFormat = format; }

    void enableInteropGrabBag(const OUString& aName);
    beans::PropertyValue getInteropGrabBag();
    void disableInteropGrabBag();
    sal_Bool isInteropGrabBagEnabled();
};
typedef boost::shared_ptr< CellColorHandler >          CellColorHandlerPtr;
}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
