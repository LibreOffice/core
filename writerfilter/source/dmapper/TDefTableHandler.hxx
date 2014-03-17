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
#ifndef INCLUDED_TDEFTABLEHANDLER_HXX
#define INCLUDED_TDEFTABLEHANDLER_HXX

#include <WriterFilterDllApi.hxx>
#include <resourcemodel/LoggedResources.hxx>
#include <boost/shared_ptr.hpp>
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

    ::std::vector< ::com::sun::star::table::BorderLine2 >    m_aLeftBorderLines;
    ::std::vector< ::com::sun::star::table::BorderLine2 >    m_aRightBorderLines;
    ::std::vector< ::com::sun::star::table::BorderLine2 >    m_aTopBorderLines;
    ::std::vector< ::com::sun::star::table::BorderLine2 >    m_aBottomBorderLines;
    ::std::vector< ::com::sun::star::table::BorderLine2 >    m_aInsideHBorderLines;
    ::std::vector< ::com::sun::star::table::BorderLine2 >    m_aInsideVBorderLines;
    ::std::vector< ::com::sun::star::table::BorderLine2 >    m_aTl2brBorderLines;
    ::std::vector< ::com::sun::star::table::BorderLine2 >    m_aTr2blBorderLines;

    //values of the current border
    sal_Int32                                           m_nLineWidth;
    sal_Int32                                           m_nLineType;
    sal_Int32                                           m_nLineColor;
    sal_Int32                                           m_nLineDistance;

    bool                                                m_bOOXML;

    OUString m_aInteropGrabBagName;
    std::vector<beans::PropertyValue> m_aInteropGrabBag;
    void appendGrabBag(const OUString& aKey, const OUString& aValue);

    void localResolve(Id Name, writerfilter::Reference<Properties>::Pointer_t pProperties);

    // Properties
    virtual void lcl_attribute(Id Name, Value & val);
    virtual void lcl_sprm(Sprm & sprm);

public:
    TDefTableHandler( bool bOOXML );
    virtual ~TDefTableHandler();

    size_t                                      getCellCount() const;
    void                                        fillCellProperties( size_t nCell, ::boost::shared_ptr< TablePropertyMap > pCellProperties) const;
    ::boost::shared_ptr<PropertyMap>            getRowProperties() const;
    sal_Int32                                   getTableWidth() const;
    void enableInteropGrabBag(const OUString& aName);
    beans::PropertyValue getInteropGrabBag(const OUString& aName = OUString());
    static OUString getBorderTypeString(sal_Int32 nType);
    static OUString getThemeColorTypeString(sal_Int32 nType);
};
typedef boost::shared_ptr< TDefTableHandler >          TDefTableHandlerPtr;
}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
