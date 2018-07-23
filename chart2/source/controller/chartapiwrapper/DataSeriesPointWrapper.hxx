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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_CHARTAPIWRAPPER_DATASERIESPOINTWRAPPER_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_CHARTAPIWRAPPER_DATASERIESPOINTWRAPPER_HXX

#include <WrappedPropertySet.hxx>
#include "ReferenceSizePropertyProvider.hxx"
#include <cppuhelper/implbase.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <memory>

namespace chart
{

namespace wrapper
{

class Chart2ModelContact;

class DataSeriesPointWrapper final : public ::cppu::ImplInheritanceHelper<
                                          WrappedPropertySet
                                        , css::lang::XServiceInfo
                                        , css::lang::XInitialization
                                        , css::lang::XComponent
                                        , css::lang::XEventListener
                                        >
                                        , public ReferenceSizePropertyProvider

{
public:
    enum eType
    {
        DATA_SERIES,
        DATA_POINT
    };

    //this constructor needs an initialize call afterwards
    explicit DataSeriesPointWrapper(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact);

    DataSeriesPointWrapper(eType eType
            , sal_Int32 nSeriesIndexInNewAPI
            , sal_Int32 nPointIndex //ignored for series
            , const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact);

    virtual ~DataSeriesPointWrapper() override;

    bool isSupportingAreaProperties();
    bool isLinesForbidden() { return !m_bLinesAllowed;}

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // ___lang::XInitialization___
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    //ReferenceSizePropertyProvider
    virtual void updateReferenceSize() override;
    virtual css::uno::Any getReferenceSize() override;
    virtual css::awt::Size getCurrentSizeForReference() override;

private:
    // ____ XComponent ____
    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

    // ____ XEventListener ____
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

    // ____ WrappedPropertySet ____
    virtual const css::uno::Sequence< css::beans::Property >& getPropertySequence() override;
    virtual const std::vector< std::unique_ptr<WrappedProperty> > createWrappedProperties() override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual css::uno::Reference< css::beans::XPropertySet > getInnerPropertySet() override;

    virtual css::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) override;
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) override;

    //own methods
    css::uno::Reference< css::chart2::XDataSeries > getDataSeries();
    css::uno::Reference< css::beans::XPropertySet > getDataPointProperties();

    std::shared_ptr< Chart2ModelContact >         m_spChart2ModelContact;
    ::comphelper::OInterfaceContainerHelper2      m_aEventListenerContainer;

    eType               m_eType;
    sal_Int32           m_nSeriesIndexInNewAPI;
    sal_Int32           m_nPointIndex;

    bool                m_bLinesAllowed;

    //this should only be used, if the DataSeriesPointWrapper is initialized via the XInitialize interface
    //because a big change in the chartmodel may lead to an dataseriespointer that is not connected to the model anymore
    //with the indices instead we can always get the new dataseries
    css::uno::Reference< css::chart2::XDataSeries >     m_xDataSeries;
};

} //  namespace wrapper
} //  namespace chart

// INCLUDED_CHART2_SOURCE_CONTROLLER_CHARTAPIWRAPPER_DATASERIESPOINTWRAPPER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
