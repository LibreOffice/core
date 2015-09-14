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

#include "WrappedPropertySet.hxx"
#include "ReferenceSizePropertyProvider.hxx"
#include "Chart2ModelContact.hxx"
#include "TitleHelper.hxx"
#include <cppuhelper/implbase.hxx>
#include <comphelper/uno3.hxx>
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

class TitleWrapper : public ::cppu::ImplInheritanceHelper<
                      WrappedPropertySet
                    , com::sun::star::drawing::XShape
                    , com::sun::star::lang::XComponent
                    , com::sun::star::lang::XServiceInfo
                    >
                    , public ReferenceSizePropertyProvider
{
public:
    TitleWrapper( ::chart::TitleHelper::eTitleType eTitleType,
                  std::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~TitleWrapper();

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    //ReferenceSizePropertyProvider
    virtual void updateReferenceSize() SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any getReferenceSize() SAL_OVERRIDE;
    virtual ::com::sun::star::awt::Size getCurrentSizeForReference() SAL_OVERRIDE;

protected:
    // ____ XShape ____
    virtual ::com::sun::star::awt::Point SAL_CALL getPosition()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setPosition( const ::com::sun::star::awt::Point& aPosition )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::awt::Size SAL_CALL getSize()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setSize( const ::com::sun::star::awt::Size& aSize )
        throw (::com::sun::star::beans::PropertyVetoException,
               ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ____ XShapeDescriptor (base of XShape) ____
    virtual OUString SAL_CALL getShapeType()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ____ XComponent ____
    virtual void SAL_CALL dispose()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference<
                                            ::com::sun::star::lang::XEventListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference<
                                               ::com::sun::star::lang::XEventListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // character properties have to be handled differently (via the XFormattedString elements)
    void getFastCharacterPropertyValue( sal_Int32 nHandle, ::com::sun::star::uno::Any& rValue );
    void setFastCharacterPropertyValue( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::uno::Exception);

    // ____ WrappedPropertySet ____
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > getInnerPropertySet() SAL_OVERRIDE;

    virtual const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& getPropertySequence() SAL_OVERRIDE;
    virtual const std::vector< WrappedProperty* > createWrappedProperties() SAL_OVERRIDE;

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > getFirstCharacterPropertySet();

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XTitle > getTitleObject();

private:
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    ::cppu::OInterfaceContainerHelper           m_aEventListenerContainer;

    ::chart::TitleHelper::eTitleType        m_eTitleType;
};

} //  namespace wrapper
} //  namespace chart

// INCLUDED_CHART2_SOURCE_CONTROLLER_CHARTAPIWRAPPER_TITLEWRAPPER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
