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
#ifndef INCLUDED_CHART2_SOURCE_INC_STYLEHANDLER_HXX
#define INCLUDED_CHART2_SOURCE_INC_STYLEHANDLER_HXX

#include <osl/file.hxx>
#include <com/sun/star/uno/Any.hxx>
#include "charttoolsdllapi.hxx"
#include <vector>
#include <memory>
#include <map>

namespace chart
{
typedef
    std::map< sal_Int32, css::uno::Any >
    tStylePropertyMap;

enum StyleType
{
    STYLETYPE_AXIS,
    STYLETYPE_BASE_COORD,
    STYLETYPE_DIAGRAM,
    STYLETYPE_DATA_POINT,
    STYLETYPE_DATA_SERIES,
    STYLETYPE_GRID,
    STYLETYPE_FORMATTED,
    STYLETYPE_LEGEND,
    STYLETYPE_PAGE,
    STYLETYPE_STOCK,
    STYLETYPE_TITLE,
    STYLETYPE_WALL,
    STYLETYPE_UNKNOWN
};

class StyleHandler
{
public:
    StyleHandler( const StyleType eType );
    ~StyleHandler();

    tStylePropertyMap& getStyle();

    static void createStyle( const tStylePropertyMap& rPropMap, StyleType eType );

    static void deleteStyle( const sal_Int16 nValue );

    static void setDefaultStyle( const sal_Int16 nValue );

    void setLocalStyle( const sal_Int16 nValue );

private:
    static void loadStyleData();

    static std::vector< std::vector< tStylePropertyMap > > m_pStyleMap;

    static sal_Int16 m_nDefaultStyle;

    static std::unique_ptr< osl::File > m_pStyleSheet;

    static bool bIsDataLoaded;
    StyleType m_eType;

    sal_Int16 m_nLocalStyle;
};

} // namespace chart

// INCLUDED_CHART2_SOURCE_INC_OPROPERTYSET_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
