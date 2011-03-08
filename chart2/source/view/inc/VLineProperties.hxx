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

#ifndef _CHART2_VLINEPROPERTIES_HXX
#define _CHART2_VLINEPROPERTIES_HXX

#include <com/sun/star/beans/XPropertySet.hpp>

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

struct VLineProperties
{
    com::sun::star::uno::Any Color; //type sal_Int32 UNO_NAME_LINECOLOR
    com::sun::star::uno::Any LineStyle; //type drawing::LineStyle for property UNO_NAME_LINESTYLE
    com::sun::star::uno::Any Transparence;//type sal_Int16 for property UNO_NAME_LINETRANSPARENCE
    com::sun::star::uno::Any Width;//type sal_Int32 for property UNO_NAME_LINEWIDTH
    com::sun::star::uno::Any DashName;//type OUString for property "LineDashName"

    VLineProperties();
    void initFromPropertySet( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::beans::XPropertySet >& xProp
                              , bool bUseSeriesPropertyNames=false );

    bool isLineVisible() const;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
