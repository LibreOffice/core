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
#ifndef INCLUDED_WRITERFILTER_SOURCE_DMAPPER_BORDERHANDLER_HXX
#define INCLUDED_WRITERFILTER_SOURCE_DMAPPER_BORDERHANDLER_HXX

#include <vector>
#include "LoggedResources.hxx"
#include <memory>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

namespace writerfilter {
namespace dmapper
{
class PropertyMap;
class BorderHandler : public LoggedProperties
{
public:
    //todo: order is a guess
    enum BorderPosition
    {
        BORDER_TOP,
        BORDER_LEFT,
        BORDER_BOTTOM,
        BORDER_RIGHT,
        BORDER_HORIZONTAL,
        BORDER_VERTICAL,
        BORDER_COUNT
    };

private:
    sal_Int8        m_nCurrentBorderPosition;
    //values of the current border
    sal_Int32       m_nLineWidth;
    sal_Int32       m_nLineType;
    sal_Int32       m_nLineColor;
    sal_Int32       m_nLineDistance;
    bool            m_bShadow;
    bool            m_bOOXML;

    bool                                        m_aFilledLines[BORDER_COUNT];
    css::table::BorderLine2 m_aBorderLines[BORDER_COUNT];
    OUString m_aInteropGrabBagName;
    std::vector<css::beans::PropertyValue> m_aInteropGrabBag;
    void appendGrabBag(const OUString& aKey, const OUString& aValue);

    // Properties
    virtual void lcl_attribute(Id Name, Value & val) override;
    virtual void lcl_sprm(Sprm & sprm) override;

public:
    explicit BorderHandler( bool bOOXML );
    virtual ~BorderHandler();

    ::std::shared_ptr<PropertyMap>            getProperties();
    css::table::BorderLine2 getBorderLine();
    sal_Int32                                   getLineDistance() const { return m_nLineDistance;}
    bool                                        getShadow() { return m_bShadow;}
    void enableInteropGrabBag(const OUString& aName);
    css::beans::PropertyValue getInteropGrabBag(const OUString& aName = OUString());
};
typedef std::shared_ptr< BorderHandler >          BorderHandlerPtr;
}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
