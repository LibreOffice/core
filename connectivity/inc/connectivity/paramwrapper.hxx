/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef CONNECTIVITY_PARAMWRAPPER_HXX
#define CONNECTIVITY_PARAMWRAPPER_HXX

#include "connectivity/dbtoolsdllapi.hxx"
#include <connectivity/FValue.hxx>

/** === begin UNO includes === **/
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryAnalyzer.hpp>
/** === end UNO includes === **/

#include <comphelper/uno3.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/compbase2.hxx>

#include <memory>
#include <vector>

//........................................................................
namespace dbtools
{
namespace param
{
//........................................................................

    //====================================================================
    //= ParameterWrapper
    //====================================================================
    /** wraps a parameter column as got from an SQLQueryComposer, so that it has an additional
        property "Value", which is forwarded to an XParameters interface
    */
    class OOO_DLLPUBLIC_DBTOOLS ParameterWrapper  :public ::cppu::OWeakObject
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
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xDelegator;
        /// the property set info for our delegator
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >   m_xDelegatorPSI;
        /// the component taking the value
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XParameters >         m_xValueDestination;
        /// helper for implementing XPropertySetInfo
        ::std::auto_ptr< ::cppu::OPropertyArrayHelper >                                 m_pInfoHelper;


    public:
        const ::connectivity::ORowSetValue& Value() const { return m_aValue; }
              ::connectivity::ORowSetValue& Value()       { return m_aValue; }

    public:
        ParameterWrapper(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxColumn
        );

        ParameterWrapper(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxColumn,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XParameters >& _rxAllParameters,
            const ::std::vector< sal_Int32 >& _rIndexes
        );

        DECLARE_XINTERFACE()
        DECLARE_XTYPEPROVIDER()

        // XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw( ::com::sun::star::uno::RuntimeException );
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

        // OPropertySetHelper
        virtual sal_Bool SAL_CALL convertFastPropertyValue( ::com::sun::star::uno::Any& rConvertedValue, ::com::sun::star::uno::Any& rOldValue, sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue) throw( ::com::sun::star::lang::IllegalArgumentException );
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue ) throw( ::com::sun::star::uno::Exception );
        virtual void SAL_CALL getFastPropertyValue( ::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;

        // pseudo-XComponent
        virtual void SAL_CALL dispose();

    protected:
        virtual ~ParameterWrapper();

        // disambiguations
        using ::cppu::OPropertySetHelper::getFastPropertyValue;

    private:
        ::rtl::OUString impl_getPseudoAggregatePropertyName( sal_Int32 _nHandle ) const;

    private:
        ParameterWrapper(); // not implemented
    };

    //====================================================================
    //= ParameterWrapperContainer
    //====================================================================
    typedef ::std::vector< ::rtl::Reference< ParameterWrapper > >   Parameters;

    //====================================================================
    //= ParameterWrapperContainer
    //====================================================================
    typedef ::cppu::WeakComponentImplHelper2    <   ::com::sun::star::container::XIndexAccess
                                                ,   ::com::sun::star::container::XEnumerationAccess
                                                >   ParameterWrapperContainer_Base;

    /// class for the parameter event @see approveParameter
    class OOO_DLLPUBLIC_DBTOOLS ParameterWrapperContainer :
        public ParameterWrapperContainer_Base
    {
    private:
        ::osl::Mutex    m_aMutex;
        Parameters      m_aParameters;

    protected:
        virtual ~ParameterWrapperContainer();

    public:
        /** creates an empty container
        */
        ParameterWrapperContainer();

        /** creates a container from a SingleSelectQuerAnalyzer's parameter columns

            Note that here, the simple constructor of the ParameterWrapper will be used, which does not
            use a XParameters instance to forward values to, but only remembers the values itself.
        */
        ParameterWrapperContainer( const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryAnalyzer >& _rxComposer );

        // ::com::sun::star::container::XElementAccess
        virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw( ::com::sun::star::uno::RuntimeException );
        virtual sal_Bool SAL_CALL hasElements() throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::container::XEnumerationAccess
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createEnumeration() throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::container::XIndexAccess
        virtual sal_Int32 SAL_CALL getCount() throw( ::com::sun::star::uno::RuntimeException );
        virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 _rIndex) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

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
        virtual void SAL_CALL disposing();

    private:
        void    impl_checkDisposed_throw();
    };

    //====================================================================
    //= ParamatersContainer
    //====================================================================
    typedef ::rtl::Reference< ParameterWrapperContainer >   ParametersContainerRef;

//........................................................................
} } // namespace dbtools::param
//........................................................................

#endif // CONNECTIVITY_PARAMWRAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
