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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_CHARTAPIWRAPPER_LEGENDWRAPPER_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_CHARTAPIWRAPPER_LEGENDWRAPPER_HXX

#include "WrappedPropertySet.hxx"
#include "ReferenceSizePropertyProvider.hxx"
#include <cppuhelper/implbase.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/chart2/XLegend.hpp>

#include <memory>

namespace chart
{

namespace wrapper
{

class Chart2ModelContact;

class LegendWrapper : public ::cppu::ImplInheritanceHelper<
                      WrappedPropertySet
                    , css::drawing::XShape
                    , css::lang::XComponent
                    , css::lang::XServiceInfo
                    >
                    , public ReferenceSizePropertyProvider
{
public:
    explicit LegendWrapper( std::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~LegendWrapper();

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw( css::uno::RuntimeException, std::exception ) override;

    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    //ReferenceSizePropertyProvider
    virtual void updateReferenceSize() override;
    virtual css::uno::Any getReferenceSize() override;
    virtual css::awt::Size getCurrentSizeForReference() override;

    // ____ XShape ____
    virtual css::awt::Point SAL_CALL getPosition()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPosition( const css::awt::Point& aPosition )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Size SAL_CALL getSize()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setSize( const css::awt::Size& aSize )
        throw (css::beans::PropertyVetoException,
               css::uno::RuntimeException, std::exception) override;

    // ____ XShapeDescriptor (base of XShape) ____
    virtual OUString SAL_CALL getShapeType()
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XComponent ____
    virtual void SAL_CALL dispose()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference<
                                            css::lang::XEventListener >& xListener )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference<
                                               css::lang::XEventListener >& aListener )
        throw (css::uno::RuntimeException, std::exception) override;

protected:
    // ____ WrappedPropertySet ____
    virtual const css::uno::Sequence< css::beans::Property >& getPropertySequence() override;
    virtual const std::vector< WrappedProperty* > createWrappedProperties() override;
    virtual css::uno::Reference< css::beans::XPropertySet > getInnerPropertySet() override;

private:
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    ::comphelper::OInterfaceContainerHelper2      m_aEventListenerContainer;
};

} //  namespace wrapper
} //  namespace chart

// INCLUDED_CHART2_SOURCE_CONTROLLER_CHARTAPIWRAPPER_LEGENDWRAPPER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
