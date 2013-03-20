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

#include "ServiceMacros.hxx"
#include "WrappedPropertySet.hxx"
#include "ReferenceSizePropertyProvider.hxx"
#include <cppuhelper/implbase4.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <boost/shared_ptr.hpp>

namespace chart
{

namespace wrapper
{

class Chart2ModelContact;

class DataSeriesPointWrapper : public ::cppu::ImplInheritanceHelper4<
                                          WrappedPropertySet
                                        , com::sun::star::lang::XServiceInfo
                                        , com::sun::star::lang::XInitialization
                                           , com::sun::star::lang::XComponent
                                           , com::sun::star::lang::XEventListener
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
    DataSeriesPointWrapper( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );

    DataSeriesPointWrapper( eType eType
            , sal_Int32 nSeriesIndexInNewAPI
            , sal_Int32 nPointIndex //ignored for series
            , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact  );

    virtual ~DataSeriesPointWrapper();

    bool isSupportingAreaProperties();
    bool isLinesForbidden();

    /// XServiceInfo declarations
    APPHELPER_XSERVICEINFO_DECL()

    // ___lang::XInitialization___
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
                throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    //ReferenceSizePropertyProvider
    virtual void updateReferenceSize();
    virtual ::com::sun::star::uno::Any getReferenceSize();
    virtual ::com::sun::star::awt::Size getCurrentSizeForReference();

protected:
    // ____ XComponent ____
    virtual void SAL_CALL dispose()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference<
                                            ::com::sun::star::lang::XEventListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference<
                                               ::com::sun::star::lang::XEventListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XEventListener ____
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);

protected:
    // ____ WrappedPropertySet ____
    virtual const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& getPropertySequence();
    virtual const std::vector< WrappedProperty* > createWrappedProperties();
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
                        ::com::sun::star::beans::XPropertySet > getInnerPropertySet();

    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const ::rtl::OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyToDefault( const ::rtl::OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& aPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //own methods
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries > getDataSeries();
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > getDataPointProperties();

private:

    void SAL_CALL getStatisticsPropertyValue
        ( ::com::sun::star::uno::Any& rValue,
          sal_Int32 nHandle ) const;

    void SAL_CALL setStatisticsPropertyValue_NoBroadcast(
        sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::uno::Exception);

    ::boost::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    ::cppu::OInterfaceContainerHelper           m_aEventListenerContainer;

    eType               m_eType;
    sal_Int32           m_nSeriesIndexInNewAPI;
    sal_Int32           m_nPointIndex;

    sal_Bool            m_bLinesAllowed;

    //this should only be used, if the DataSeriesPointWrapper is initialized via the XInitialize interface
    //because a big change in the chartmodel may leed to an dataseriespointer thats not connected to the model anymore
    //with the indizes instead we are can aleays get the new dataseries
    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XDataSeries >     m_xDataSeries;
};

} //  namespace wrapper
} //  namespace chart

// CHART_DATASERIESPOINTWRAPPER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
