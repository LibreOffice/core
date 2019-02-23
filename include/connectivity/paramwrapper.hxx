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

#ifndef INCLUDED_CONNECTIVITY_PARAMWRAPPER_HXX
#define INCLUDED_CONNECTIVITY_PARAMWRAPPER_HXX

#include <connectivity/dbtoolsdllapi.hxx>
#include <connectivity/FValue.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>

#include <comphelper/uno3.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/compbase.hxx>

#include <memory>
#include <vector>

namespace com::sun::star::sdbc { class XParameters; }
namespace com::sun::star::sdb { class XSingleSelectQueryAnalyzer; }


namespace dbtools
{
namespace param
{


    //= ParameterWrapper

    /** wraps a parameter column as got from an SQLQueryComposer, so that it has an additional
        property "Value", which is forwarded to an XParameters interface
    */
    class OOO_DLLPUBLIC_DBTOOLS ParameterWrapper  :public ::cppu::OWeakObject
                            ,public css::lang::XTypeProvider
                            ,public ::comphelper::OMutexAndBroadcastHelper
                            ,public ::cppu::OPropertySetHelper
    {
    private:
        typedef ::cppu::OWeakObject         UnoBase;
        typedef ::cppu::OPropertySetHelper  PropertyBase;

    private:
        /// the most recently set value of the parameter
        ::connectivity::ORowSetValue    m_aValue;
        /// the positions (in our m_xValueDestination) at which the value should be set (0-based!)
        ::std::vector< sal_Int32 >      m_aIndexes;

        /// the "delegator" column to which standard property requests are forwarded
        css::uno::Reference< css::beans::XPropertySet >       m_xDelegator;
        /// the property set info for our delegator
        css::uno::Reference< css::beans::XPropertySetInfo >   m_xDelegatorPSI;
        /// the component taking the value
        css::uno::Reference< css::sdbc::XParameters >         m_xValueDestination;
        /// helper for implementing XPropertySetInfo
        ::std::unique_ptr< ::cppu::OPropertyArrayHelper >     m_pInfoHelper;


    public:
        const ::connectivity::ORowSetValue& Value() const { return m_aValue; }
              ::connectivity::ORowSetValue& Value()       { return m_aValue; }

    public:
        ParameterWrapper(
            const css::uno::Reference< css::beans::XPropertySet >& _rxColumn
        );

        ParameterWrapper(
            const css::uno::Reference< css::beans::XPropertySet >& _rxColumn,
            const css::uno::Reference< css::sdbc::XParameters >& _rxAllParameters,
            const ::std::vector< sal_Int32 >& _rIndexes
        );

        DECLARE_XINTERFACE()

        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;

        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() override;
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

        // OPropertySetHelper
        virtual sal_Bool SAL_CALL convertFastPropertyValue( css::uno::Any& rConvertedValue, css::uno::Any& rOldValue, sal_Int32 nHandle, const css::uno::Any& rValue) override;
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const css::uno::Any& rValue ) override;
        virtual void SAL_CALL getFastPropertyValue( css::uno::Any& rValue, sal_Int32 nHandle ) const override;

        // pseudo-XComponent
        void dispose();

    protected:
        virtual ~ParameterWrapper() override;

        // disambiguations
        using ::cppu::OPropertySetHelper::getFastPropertyValue;

    private:
        OUString impl_getPseudoAggregatePropertyName( sal_Int32 _nHandle ) const;
    };


    //= ParameterWrapper

    typedef ::std::vector< ::rtl::Reference< ParameterWrapper > >   Parameters;


    //= ParameterWrapperContainer

    typedef ::cppu::WeakComponentImplHelper    <   css::container::XIndexAccess
                                               ,   css::container::XEnumerationAccess
                                               >   ParameterWrapperContainer_Base;

    /// class for the parameter event @see approveParameter
    class OOO_DLLPUBLIC_DBTOOLS ParameterWrapperContainer :
        public ParameterWrapperContainer_Base
    {
    private:
        ::osl::Mutex    m_aMutex;
        Parameters      m_aParameters;

    protected:
        virtual ~ParameterWrapperContainer() override;

    public:
        /** creates an empty container
        */
        ParameterWrapperContainer();

        /** creates a container from a SingleSelectQuerAnalyzer's parameter columns

            Note that here, the simple constructor of the ParameterWrapper will be used, which does not
            use a XParameters instance to forward values to, but only remembers the values itself.
        */
        ParameterWrapperContainer( const css::uno::Reference< css::sdb::XSingleSelectQueryAnalyzer >& _rxComposer );

        // css::container::XElementAccess
        virtual css::uno::Type SAL_CALL getElementType() override;
        virtual sal_Bool SAL_CALL hasElements() override;

        // css::container::XEnumerationAccess
        virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() override;

        // css::container::XIndexAccess
        virtual sal_Int32 SAL_CALL getCount() override;
        virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 _rIndex) override;

    public:
        const Parameters& getParameters() { return m_aParameters; }

        const ::connectivity::ORowSetValue& operator[]( size_t _index ) const { return m_aParameters[ _index ]->Value(); }
              ::connectivity::ORowSetValue& operator[]( size_t _index )       { return m_aParameters[ _index ]->Value(); }

        /** adds an ParameterWrapper to the end of the array
        */
        void    push_back( ParameterWrapper* _pParameter )
        {
            m_aParameters.push_back( _pParameter );
        }

        size_t  size() const { return m_aParameters.size(); }

    protected:
        // XComponent
        virtual void SAL_CALL disposing() override;

    private:
        void    impl_checkDisposed_throw();
    };


    //= ParametersContainer

    typedef ::rtl::Reference< ParameterWrapperContainer >   ParametersContainerRef;


} } // namespace dbtools::param


#endif // INCLUDED_CONNECTIVITY_PARAMWRAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
