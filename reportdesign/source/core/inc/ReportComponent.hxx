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

#ifndef RPT_REPORTCOMPONENT_HXX
#define RPT_REPORTCOMPONENT_HXX

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/uno/XAggregation.hpp>
#include <com/sun/star/report/XReportComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/weakref.hxx>
#include <comphelper/uno3.hxx>

namespace reportdesign
{
    class OReportComponentProperties
    {
    public:
        ::com::sun::star::uno::WeakReference< ::com::sun::star::container::XChild > m_xParent;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                                                                                    m_xFactory;
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >       m_xShape;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation >     m_xProxy;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   m_xProperty;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XTypeProvider >   m_xTypeProvider;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel >      m_xUnoTunnel;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XServiceInfo >    m_xServiceInfo;
        ::com::sun::star::uno::Sequence< ::rtl::OUString >                          m_aMasterFields;
        ::com::sun::star::uno::Sequence< ::rtl::OUString >                          m_aDetailFields;
        ::rtl::OUString                                                             m_sName;
        ::sal_Int32                                                                 m_nHeight;
        ::sal_Int32                                                                 m_nWidth;
        ::sal_Int32                                                                 m_nPosX;
        ::sal_Int32                                                                 m_nPosY;
        ::sal_Int32                                                                 m_nBorderColor;
        ::sal_Int16                                                                 m_nBorder;
        ::sal_Bool                                                                  m_bPrintRepeatedValues;

        OReportComponentProperties(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & _xContext
            )
            :m_xContext(_xContext)
            ,m_nHeight(0)
            ,m_nWidth(0)
            ,m_nPosX(0)
            ,m_nPosY(0)
            ,m_nBorderColor(0)
            ,m_nBorder(2)
            ,m_bPrintRepeatedValues(sal_True)
        {}
        ~OReportComponentProperties();

        void setShape(::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& _xShape
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent>& _xTunnel
                    ,oslInterlockedCount& _rRefCount);
    };
}
#endif // RPT_REPORTCOMPONENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
