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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_CHARTAPIWRAPPER_AXISWRAPPER_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_CHARTAPIWRAPPER_AXISWRAPPER_HXX

#include "WrappedPropertySet.hxx"
#include "ReferenceSizePropertyProvider.hxx"
#include <cppuhelper/implbase.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <com/sun/star/chart/XAxis.hpp>
#include <com/sun/star/chart2/XAxis.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>

#include <memory>

namespace chart
{
namespace wrapper
{
class Chart2ModelContact;

class AxisWrapper : public ::cppu::ImplInheritanceHelper<
                      WrappedPropertySet
                    , css::chart::XAxis
                    , css::drawing::XShape
                    , css::lang::XComponent
                    , css::lang::XServiceInfo
                    , css::util::XNumberFormatsSupplier
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

    AxisWrapper( tAxisType eType, std::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~AxisWrapper();

    static void getDimensionAndMainAxisBool( tAxisType eType, sal_Int32& rnDimensionIndex, bool& rbMainAxis );

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

    // ____ XComponent ____
    virtual void SAL_CALL dispose()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference<
                                            css::lang::XEventListener >& xListener )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference<
                                               css::lang::XEventListener >& aListener )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ chart::XAxis ____
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getAxisTitle(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getMajorGrid(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getMinorGrid(  ) throw (css::uno::RuntimeException, std::exception) override;

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

    // ____ XNumberFormatsSupplier ____
    virtual css::uno::Reference<
                css::beans::XPropertySet > SAL_CALL getNumberFormatSettings()
            throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference<
                css::util::XNumberFormats > SAL_CALL getNumberFormats()
            throw (css::uno::RuntimeException, std::exception) override;

protected:
    // ____ WrappedPropertySet ____
    virtual const css::uno::Sequence< css::beans::Property >& getPropertySequence() override;
    virtual const std::vector< WrappedProperty* > createWrappedProperties() override;
    virtual css::uno::Reference< css::beans::XPropertySet > getInnerPropertySet() override;

private: //methods
    css::uno::Reference< css::chart2::XAxis > getAxis();

private: //member
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    ::comphelper::OInterfaceContainerHelper2      m_aEventListenerContainer;

    tAxisType           m_eType;

    css::uno::Reference< css::beans::XPropertySet >   m_xAxisTitle;
    css::uno::Reference< css::beans::XPropertySet >   m_xMajorGrid;
    css::uno::Reference< css::beans::XPropertySet >   m_xMinorGrid;
};

} //  namespace wrapper
} //  namespace chart

// INCLUDED_CHART2_SOURCE_CONTROLLER_CHARTAPIWRAPPER_AXISWRAPPER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
