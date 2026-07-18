/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <WrappedPropertySet.hxx>
#include "ReferenceSizePropertyProvider.hxx"
#include <cppuhelper/implbase.hxx>
#include <comphelper/interfacecontainer4.hxx>
#include <com/sun/star/chart/XAxis.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <rtl/ref.hxx>
#include <memory>

namespace com::sun::star::chart2 { class XAxis; }

namespace chart::wrapper
{
class Chart2ModelContact;
class TitleWrapper;
class GridWrapper;

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

    AxisWrapper(tAxisType eType, std::shared_ptr<Chart2ModelContact> spChart2ModelContact);
    virtual ~AxisWrapper() override;

    static void getDimensionAndMainAxisBool( tAxisType eType, sal_Int32& rnDimensionIndex, bool& rbMainAxis );

    /// XServiceInfo declarations
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    //ReferenceSizePropertyProvider
    virtual void updateReferenceSize() override;
    virtual cpo::uno::Any getReferenceSize() override;
    virtual css::awt::Size getCurrentSizeForReference() override;

    // ____ XComponent ____
    virtual void dispose() override;
    virtual void addEventListener( const css::uno::Reference<
                                            css::lang::XEventListener >& xListener ) override;
    virtual void removeEventListener( const css::uno::Reference<
                                               css::lang::XEventListener >& aListener ) override;

    // ____ chart::XAxis ____
    virtual css::uno::Reference< css::beans::XPropertySet > getAxisTitle(  ) override;
    virtual css::uno::Reference< css::beans::XPropertySet > getMajorGrid(  ) override;
    virtual css::uno::Reference< css::beans::XPropertySet > getMinorGrid(  ) override;

    // ____ XShape ____
    virtual css::awt::Point getPosition() override;
    virtual void setPosition( const css::awt::Point& aPosition ) override;
    virtual css::awt::Size getSize() override;
    virtual void setSize( const css::awt::Size& aSize ) override;

    // ____ XShapeDescriptor (base of XShape) ____
    virtual OUString getShapeType() override;

    // ____ XNumberFormatsSupplier ____
    virtual css::uno::Reference<
                css::beans::XPropertySet > getNumberFormatSettings() override;
    virtual css::uno::Reference<
                css::util::XNumberFormats > getNumberFormats() override;

protected:
    // ____ WrappedPropertySet ____
    virtual const cpo::uno::Sequence< css::beans::Property >& getPropertySequence() override;
    virtual std::vector< std::unique_ptr<WrappedProperty> > createWrappedProperties() override;
    virtual css::uno::Reference< css::beans::XPropertySet > getInnerPropertySet() override;

private: //methods
    css::uno::Reference< css::chart2::XAxis > getAxis();

private: //member
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    ::comphelper::OInterfaceContainerHelper4<css::lang::XEventListener> m_aEventListenerContainer;

    tAxisType           m_eType;

    rtl::Reference< TitleWrapper >   m_xAxisTitle;
    rtl::Reference< GridWrapper >   m_xMajorGrid;
    rtl::Reference< GridWrapper >   m_xMinorGrid;
};

} //  namespace chart::wrapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
