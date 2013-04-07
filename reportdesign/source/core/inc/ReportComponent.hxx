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
        ::com::sun::star::uno::Sequence< OUString >                          m_aMasterFields;
        ::com::sun::star::uno::Sequence< OUString >                          m_aDetailFields;
        OUString                                                             m_sName;
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
