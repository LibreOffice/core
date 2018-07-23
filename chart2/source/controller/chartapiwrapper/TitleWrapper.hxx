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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_CHARTAPIWRAPPER_TITLEWRAPPER_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_CHARTAPIWRAPPER_TITLEWRAPPER_HXX

#include <WrappedPropertySet.hxx>
#include "ReferenceSizePropertyProvider.hxx"
#include "Chart2ModelContact.hxx"
#include <TitleHelper.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/chart2/XTitle.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <memory>

namespace chart
{
namespace wrapper
{

class TitleWrapper final : public ::cppu::ImplInheritanceHelper<
                      WrappedPropertySet
                    , css::drawing::XShape
                    , css::lang::XComponent
                    , css::lang::XServiceInfo
                    >
                    , public ReferenceSizePropertyProvider
{
public:
    TitleWrapper( ::chart::TitleHelper::eTitleType eTitleType,
                  const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact );
    virtual ~TitleWrapper() override;

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    //ReferenceSizePropertyProvider
    virtual void updateReferenceSize() override;
    virtual css::uno::Any getReferenceSize() override;
    virtual css::awt::Size getCurrentSizeForReference() override;

private:
    // ____ XShape ____
    virtual css::awt::Point SAL_CALL getPosition() override;
    virtual void SAL_CALL setPosition( const css::awt::Point& aPosition ) override;
    virtual css::awt::Size SAL_CALL getSize() override;
    virtual void SAL_CALL setSize( const css::awt::Size& aSize ) override;

    // ____ XShapeDescriptor (base of XShape) ____
    virtual OUString SAL_CALL getShapeType() override;

    // ____ XComponent ____
    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

    // character properties have to be handled differently (via the XFormattedString elements)
    void getFastCharacterPropertyValue( sal_Int32 nHandle, css::uno::Any& rValue );
    /// @throws css::uno::Exception
    void setFastCharacterPropertyValue( sal_Int32 nHandle, const css::uno::Any& rValue );

    // ____ WrappedPropertySet ____
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;

    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;

    virtual css::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) override;

    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) override;

    virtual css::uno::Reference< css::beans::XPropertySet > getInnerPropertySet() override;

    virtual const css::uno::Sequence< css::beans::Property >& getPropertySequence() override;
    virtual const std::vector< std::unique_ptr<WrappedProperty> > createWrappedProperties() override;

    css::uno::Reference< css::beans::XPropertySet > getFirstCharacterPropertySet();

    css::uno::Reference< css::chart2::XTitle > getTitleObject();

    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    ::comphelper::OInterfaceContainerHelper2           m_aEventListenerContainer;

    ::chart::TitleHelper::eTitleType        m_eTitleType;
};

} //  namespace wrapper
} //  namespace chart

// INCLUDED_CHART2_SOURCE_CONTROLLER_CHARTAPIWRAPPER_TITLEWRAPPER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
