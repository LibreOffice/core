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
#ifndef CHART_RENDERER_HXX
#define CHART_RENDERER_HXX

#include <svtools/chartprettypainter.hxx>
#include "ServiceMacros.hxx"

#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/weakref.hxx>

//.............................................................................
namespace chart
{
//.............................................................................

//#i82893#, #i75867#: charts must be painted resolution dependent!!
class ChartRenderer : public ::cppu::WeakImplHelper2<
          ::com::sun::star::lang::XServiceInfo
        , ::com::sun::star::lang::XUnoTunnel
        >, public ChartPrettyPainter
{
public:
    ChartRenderer( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel );
    virtual ~ChartRenderer();

    // ___lang::XServiceInfo___
    APPHELPER_XSERVICEINFO_DECL()

    // ____ XUnoTunnel ___
    virtual ::sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& aIdentifier )
            throw (::com::sun::star::uno::RuntimeException);

    // ____ ChartPrettyPainter ___
    virtual bool DoPaint(OutputDevice* pOutDev, const Rectangle& rLogicObjectRect) const;

private:
    ChartRenderer();
    ChartRenderer( const ChartRenderer& );

    ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XModel > m_xChartModel;
};

//.............................................................................
}  // namespace chart
//.............................................................................

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
