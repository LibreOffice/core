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
#include <map>

#include <sal/config.h>

#include "dllapi.h"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <svx/svdobjkind.hxx>
#include <svx/svdtypes.hxx>

namespace com::sun::star {
    namespace report {
        class XReportDefinition;
    }
}

namespace rptui
{
// not all used at the moment
constexpr SdrLayerID RPT_LAYER_FRONT  (0);
constexpr SdrLayerID RPT_LAYER_BACK   (1);
constexpr SdrLayerID RPT_LAYER_HIDDEN (2);

// allows the alignment and resizing of controls
enum class ControlModification
{
    NONE               =     0,
    LEFT               =     1,
    RIGHT              =     2,
    TOP                =     3,
    BOTTOM             =     4,
    CENTER_HORIZONTAL  =     5,
    CENTER_VERTICAL    =     6,
    WIDTH_SMALLEST     =     7,
    HEIGHT_SMALLEST    =     8,
    WIDTH_GREATEST     =     9,
    HEIGHT_GREATEST    =     10,
};

class AnyConverter
{
public:
    virtual ~AnyConverter(){}
    virtual css::uno::Any operator() (const OUString& /*_sPropertyName*/,const css::uno::Any& lhs) const
    {
        return lhs;
    }
};
typedef ::std::pair< OUString, std::shared_ptr<AnyConverter> > TPropertyConverter;
typedef std::map<OUString, TPropertyConverter> TPropertyNamePair;
/** returns the property name map for the given property id
    @param  _nObjectId  the object id
*/
REPORTDESIGN_DLLPUBLIC const TPropertyNamePair& getPropertyNameMap(SdrObjKind _nObjectId);
REPORTDESIGN_DLLPUBLIC css::uno::Reference< css::style::XStyle> getUsedStyle(const css::uno::Reference< css::report::XReportDefinition>& _xReport);

template < typename T> T getStyleProperty(const css::uno::Reference< css::report::XReportDefinition>& _xReport,const OUString& _sPropertyName)
{
    T nReturn = T();
    css::uno::Reference<css::beans::XPropertySet> xProp(getUsedStyle(_xReport),css::uno::UNO_QUERY_THROW);
    xProp->getPropertyValue(_sPropertyName) >>= nReturn;
    return nReturn;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
