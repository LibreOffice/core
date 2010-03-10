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
#ifndef INCLUDED_BORDERHANDLER_HXX
#define INCLUDED_BORDERHANDLER_HXX

#include <WriterFilterDllApi.hxx>
#include <resourcemodel/WW8ResourceModel.hxx>
#include <boost/shared_ptr.hpp>
#include <com/sun/star/table/BorderLine.hpp>

namespace writerfilter {
namespace dmapper
{
class PropertyMap;
class WRITERFILTER_DLLPRIVATE BorderHandler : public Properties
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
    bool            m_bOOXML;

    bool                                        m_aFilledLines[BORDER_COUNT];
    ::com::sun::star::table::BorderLine         m_aBorderLines[BORDER_COUNT];

public:
    BorderHandler( bool bOOXML );
    virtual ~BorderHandler();

    // Properties
    virtual void attribute(Id Name, Value & val);
    virtual void sprm(Sprm & sprm);

    ::boost::shared_ptr<PropertyMap>            getProperties();
    ::com::sun::star::table::BorderLine         getBorderLine();
    sal_Int32                                   getLineDistance() const { return m_nLineDistance;}
};
typedef boost::shared_ptr< BorderHandler >          BorderHandlerPtr;
}}

#endif //
