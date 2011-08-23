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
#ifndef INCLUDED_MEASUREHANDLER_HXX
#define INCLUDED_MEASUREHANDLER_HXX

#include <WriterFilterDllApi.hxx>
#include <resourcemodel/WW8ResourceModel.hxx>
#include <boost/shared_ptr.hpp>

namespace writerfilter {
namespace dmapper
{
class PropertyMap;
/** Handler for sprms that contain a measure and a unit
    - Left indent of tables
    - Preferred width of tables
 */
class WRITERFILTER_DLLPRIVATE MeasureHandler : public Properties
{
    sal_Int32 m_nMeasureValue;
    sal_Int32 m_nUnit;
    sal_Int16 m_nRowHeightSizeType; //table row height type
    
public:
    MeasureHandler();
    virtual ~MeasureHandler();

    // Properties
    virtual void attribute(Id Name, Value & val);
    virtual void sprm(Sprm & sprm);

    sal_Int32 getMeasureValue() const;
    //at least tables can have automatic width
    bool isAutoWidth() const;

    sal_Int16 GetRowHeightSizeType() const { return m_nRowHeightSizeType;}
};
typedef boost::shared_ptr
    < MeasureHandler >  MeasureHandlerPtr;
}}

#endif //
