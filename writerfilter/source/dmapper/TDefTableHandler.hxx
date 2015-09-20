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
#ifndef INCLUDED_WRITERFILTER_SOURCE_DMAPPER_TDEFTABLEHANDLER_HXX
#define INCLUDED_WRITERFILTER_SOURCE_DMAPPER_TDEFTABLEHANDLER_HXX

#include "LoggedResources.hxx"
#include <memory>
#include <vector>
namespace com{ namespace sun{ namespace star{
    namespace table {
        struct BorderLine2;
    }
    namespace beans {
        struct PropertyValue;
    }
}}}

namespace writerfilter {
namespace dmapper
{
class PropertyMap;
class TablePropertyMap;
class TDefTableHandler : public LoggedProperties
{
public:

private:
    ::std::vector<sal_Int32>                                m_aCellBorderPositions;
    ::std::vector<sal_Int32>                                m_aCellVertAlign;

    std::vector<css::table::BorderLine2> m_aLeftBorderLines;
    std::vector<css::table::BorderLine2> m_aRightBorderLines;
    std::vector<css::table::BorderLine2> m_aTopBorderLines;
    std::vector<css::table::BorderLine2> m_aBottomBorderLines;
    std::vector<css::table::BorderLine2> m_aInsideHBorderLines;
    std::vector<css::table::BorderLine2> m_aInsideVBorderLines;
    std::vector<css::table::BorderLine2> m_aTl2brBorderLines;
    std::vector<css::table::BorderLine2> m_aTr2blBorderLines;

    //values of the current border
    sal_Int32                                           m_nLineWidth;
    sal_Int32                                           m_nLineType;
    sal_Int32                                           m_nLineColor;
    sal_Int32                                           m_nLineDistance;

    OUString m_aInteropGrabBagName;
    std::vector<css::beans::PropertyValue> m_aInteropGrabBag;
    void appendGrabBag(const OUString& aKey, const OUString& aValue);

    void localResolve(Id Name, writerfilter::Reference<Properties>::Pointer_t pProperties);

    // Properties
    virtual void lcl_attribute(Id Name, Value & val) SAL_OVERRIDE;
    virtual void lcl_sprm(Sprm & sprm) SAL_OVERRIDE;

public:
    TDefTableHandler();
    virtual ~TDefTableHandler();

    void                                        fillCellProperties( size_t nCell, ::std::shared_ptr< TablePropertyMap > pCellProperties) const;
    void enableInteropGrabBag(const OUString& aName);
    css::beans::PropertyValue getInteropGrabBag(const OUString& aName = OUString());
    static OUString getBorderTypeString(sal_Int32 nType);
    static OUString getThemeColorTypeString(sal_Int32 nType);
    static sal_Int16 getThemeColorTypeIndex(sal_Int32 nType);
};
}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
