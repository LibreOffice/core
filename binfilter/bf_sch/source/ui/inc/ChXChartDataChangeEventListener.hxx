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

#ifndef _CHXCHARTDATACHANGEEVENTLISTENER_HXX
#define _CHXCHARTDATACHANGEEVENTLISTENER_HXX

#include <cppuhelper/implbase1.hxx>	// helper for implementations

#include <com/sun/star/chart/XChartDataChangeEventListener.hpp>
namespace binfilter {

class ChXChartDocument;

class ChXChartDataChangeEventListener :
    public cppu::WeakImplHelper1< ::com::sun::star::chart::XChartDataChangeEventListener >
{
private:
    ChXChartDocument *mpXDoc;

public:
    ChXChartDataChangeEventListener();
    virtual ~ChXChartDataChangeEventListener(){};

    void Reset() throw();
    void SetOwner( ChXChartDocument* pXDoc ) throw();

    // XChartDataChangeEventListener
    virtual void SAL_CALL chartDataChanged( const ::com::sun::star::chart::ChartDataChangeEvent& aEvent )
        throw( ::com::sun::star::uno::RuntimeException );

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
        throw( ::com::sun::star::uno::RuntimeException );

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw( ::com::sun::star::uno::RuntimeException );
};

} //namespace binfilter
#endif	// _CHXCHARTDATACHANGEEVENTLISTENER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
