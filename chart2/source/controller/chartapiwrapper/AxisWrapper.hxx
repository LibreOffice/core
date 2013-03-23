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
#pragma once
#if 1

#include "WrappedPropertySet.hxx"
#include "ReferenceSizePropertyProvider.hxx"
#include "ServiceMacros.hxx"
#include <cppuhelper/implbase5.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <com/sun/star/chart/XAxis.hpp>
#include <com/sun/star/chart2/XAxis.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>

#include <boost/shared_ptr.hpp>

namespace chart
{
namespace wrapper
{
class Chart2ModelContact;

class AxisWrapper : public ::cppu::ImplInheritanceHelper5<
                      WrappedPropertySet
                    , com::sun::star::chart::XAxis
                    , com::sun::star::drawing::XShape
                    , com::sun::star::lang::XComponent
                    , com::sun::star::lang::XServiceInfo
                    , com::sun::star::util::XNumberFormatsSupplier
                    >
                    , public ReferenceSizePropertyProvider
{
public:
    enum tAxisType
    {
        X_AXIS,
        Y_AXIS,
        Z_AXIS,
        SECOND_X_AXIS,
        SECOND_Y_AXIS
    };

    AxisWrapper( tAxisType eType, ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~AxisWrapper();

    static void getDimensionAndMainAxisBool( tAxisType eType, sal_Int32& rnDimensionIndex, sal_Bool& rbMainAxis );

    /// XServiceInfo declarations
    APPHELPER_XSERVICEINFO_DECL()

    //ReferenceSizePropertyProvider
    virtual void updateReferenceSize();
    virtual ::com::sun::star::uno::Any getReferenceSize();
    virtual ::com::sun::star::awt::Size getCurrentSizeForReference();

    // ____ XComponent ____
    virtual void SAL_CALL dispose()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference<
                                            ::com::sun::star::lang::XEventListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference<
                                               ::com::sun::star::lang::XEventListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ chart::XAxis ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL getAxisTitle(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL getMajorGrid(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL getMinorGrid(  ) throw (::com::sun::star::uno::RuntimeException);

    // ____ XShape ____
    virtual ::com::sun::star::awt::Point SAL_CALL getPosition()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPosition( const ::com::sun::star::awt::Point& aPosition )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Size SAL_CALL getSize()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setSize( const ::com::sun::star::awt::Size& aSize )
        throw (::com::sun::star::beans::PropertyVetoException,
               ::com::sun::star::uno::RuntimeException);

    // ____ XShapeDescriptor (base of XShape) ____
    virtual ::rtl::OUString SAL_CALL getShapeType()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XNumberFormatsSupplier ____
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > SAL_CALL getNumberFormatSettings()
            throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::util::XNumberFormats > SAL_CALL getNumberFormats()
            throw (::com::sun::star::uno::RuntimeException);

protected:
    // ____ WrappedPropertySet ____
    virtual const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& getPropertySequence();
    virtual const std::vector< WrappedProperty* > createWrappedProperties();
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > getInnerPropertySet();

private: //methods
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis > getAxis();

private: //member
    ::boost::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    ::cppu::OInterfaceContainerHelper           m_aEventListenerContainer;

    tAxisType           m_eType;
    ::com::sun::star::uno::Any m_aTemporaryHelpStepValue;

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   m_xAxisTitle;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   m_xMajorGrid;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   m_xMinorGrid;
};

} //  namespace wrapper
} //  namespace chart

// CHART_AXISWRAPPER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
