/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef INCLUDED_TDEFTABLEHANDLER_HXX
#define INCLUDED_TDEFTABLEHANDLER_HXX

#include <WriterFilterDllApi.hxx>
#include <resourcemodel/WW8ResourceModel.hxx>
#include <boost/shared_ptr.hpp>
#include <vector>
namespace com{ namespace sun{ namespace star{namespace table {
    struct BorderLine2;
}}}}

namespace writerfilter {
namespace dmapper
{
class PropertyMap;
class TablePropertyMap;
class WRITERFILTER_DLLPRIVATE TDefTableHandler : public Properties
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

    void localResolve(Id Name, writerfilter::Reference<Properties>::Pointer_t pProperties);
public:
    TDefTableHandler( bool bOOXML );
    virtual ~TDefTableHandler();

    // Properties
    virtual void attribute(Id Name, Value & val);
    virtual void sprm(Sprm & sprm);

    size_t                                      getCellCount() const;
    void                                        fillCellProperties( size_t nCell, ::boost::shared_ptr< TablePropertyMap > pCellProperties) const;
    ::boost::shared_ptr<PropertyMap>            getRowProperties() const;
    sal_Int32                                   getTableWidth() const;
};
typedef boost::shared_ptr< TDefTableHandler >          TDefTableHandlerPtr;
}}

#endif //

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
