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

#ifndef _TOOLKIT_HELPERS_GEOMETRYCONTROLMODEL_HXX_
#define _TOOLKIT_HELPERS_GEOMETRYCONTROLMODEL_HXX_

#include <comphelper/broadcasthelper.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/propagg.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/propertycontainer.hxx>
#include <cppuhelper/weakagg.hxx>
#include <cppuhelper/compbase2.hxx>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/script/XScriptEventsSupplier.hpp>
#include <com/sun/star/resource/XStringResourceResolver.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/IdPropArrayHelper.hxx>
#include <comphelper/stl_types.hxx>

namespace com { namespace sun { namespace star {
    namespace lang {
        class XMultiServiceFactory;
    }
    namespace script {
        class XNameContainer;
    }
}}}

//........................................................................
// namespace toolkit
// {
//........................................................................

    //====================================================================
    //= OGeometryControlModel_Base
    //====================================================================
    typedef ::cppu::WeakAggComponentImplHelper2 <   ::com::sun::star::util::XCloneable
                                                ,   ::com::sun::star::script::XScriptEventsSupplier
                                                >   OGCM_Base;
    class OGeometryControlModel_Base
        :public ::comphelper::OMutexAndBroadcastHelper
        ,public ::comphelper::OPropertySetAggregationHelper
        ,public ::comphelper::OPropertyContainer
        ,public OGCM_Base
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation >
                    m_xAggregate;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
                    mxEventContainer;

        // <properties>
        sal_Int32       m_nPosX;
        sal_Int32       m_nPosY;
        sal_Int32       m_nWidth;
        sal_Int32       m_nHeight;
        ::rtl::OUString m_aName;
        sal_Int16       m_nTabIndex;
        sal_Int32       m_nStep;
        ::rtl::OUString m_aTag;
        ::com::sun::star::uno::Reference< ::com::sun::star::resource::XStringResourceResolver > m_xStrResolver;
        // </properties>

        sal_Bool        m_bCloneable;

    protected:
        virtual ::com::sun::star::uno::Any          ImplGetDefaultValueByHandle(sal_Int32 nHandle) const;
        virtual ::com::sun::star::uno::Any          ImplGetPropertyValueByHandle(sal_Int32 nHandle) const;
        virtual void                                ImplSetPropertyValueByHandle(sal_Int32 nHandle, const :: com::sun::star::uno::Any& aValue);

    protected:
        /**
            @param _pAggregateInstance
                the object to be aggregated. The refcount of the instance given MUST be 0!
        */
        OGeometryControlModel_Base(::com::sun::star::uno::XAggregation* _pAggregateInstance);

        /**
            @param _rxAggregateInstance
                is the object to be aggregated. Must be aquired excatly once (by the reference object given).<br/>
                Will be reset to NULL upon leaving
        */
        OGeometryControlModel_Base(::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable >& _rxAggregateInstance);

        /** releases the aggregation
            <p>Can be used if in a derived class, an exception has to be thrown after this base class here already
            did the aggregation</p>
        */
        void releaseAggregation();

    protected:
        ~OGeometryControlModel_Base();

        // XAggregation
        ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _aType ) throw(::com::sun::star::uno::RuntimeException);

        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL acquire(  ) throw();
        virtual void SAL_CALL release(  ) throw();

        // XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw (::com::sun::star::uno::RuntimeException);

        // OPropertySetHelper overridables
        virtual sal_Bool SAL_CALL convertFastPropertyValue(
                ::com::sun::star::uno::Any& _rConvertedValue, ::com::sun::star::uno::Any& _rOldValue,
                sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rValue )
            throw (::com::sun::star::lang::IllegalArgumentException);

        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rValue)
            throw (::com::sun::star::uno::Exception);

        using comphelper::OPropertySetAggregationHelper::getFastPropertyValue;
        virtual void SAL_CALL getFastPropertyValue(
            ::com::sun::star::uno::Any& _rValue, sal_Int32 _nHandle) const;

        // OPropertyStateHelper overridables
        virtual ::com::sun::star::beans::PropertyState  getPropertyStateByHandle(sal_Int32 nHandle);
        virtual void                                    setPropertyToDefaultByHandle(sal_Int32 nHandle);
        virtual ::com::sun::star::uno::Any              getPropertyDefaultByHandle(sal_Int32 nHandle) const;

        // XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException);

        // OPropertySetAggregationHelper overridables
        using OPropertySetAggregationHelper::getInfoHelper;

        // XCloneable
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone(  ) throw(::com::sun::star::uno::RuntimeException);

        //XScriptEventsSupplier
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
            SAL_CALL getEvents(  ) throw(::com::sun::star::uno::RuntimeException);

        // XCloneable implementation - to be overwritten
        virtual OGeometryControlModel_Base* createClone_Impl(
            ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable >& _rxAggregateInstance) = 0;

        // XComponent
        using comphelper::OPropertySetAggregationHelper::disposing;
        virtual void SAL_CALL disposing();

    private:
        void registerProperties();
    };

    //====================================================================
    //= OTemplateInstanceDisambiguation
    //====================================================================
    template <class CONTROLMODEL>
    class OTemplateInstanceDisambiguation
    {
    };

    //====================================================================
    //= OGeometryControlModel
    //====================================================================
    /*  example for usage:
            Reference< XAggregation > xIFace = new ::toolkit::OGeometryControlModel< UnoControlButtonModel > ();
    */
    template <class CONTROLMODEL>
    class OGeometryControlModel
        :public OGeometryControlModel_Base
        ,public ::comphelper::OAggregationArrayUsageHelper< OTemplateInstanceDisambiguation< CONTROLMODEL > >
    {
    public:
        OGeometryControlModel( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory );

    private:
        OGeometryControlModel(::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable >& _rxAggregateInstance);

    protected:
        // OAggregationArrayUsageHelper overridables
        virtual void fillProperties(
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& _rProps,
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& _rAggregateProps
            ) const;

        // OPropertySetAggregationHelper overridables
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

        // OGeometryControlModel_Base
        virtual OGeometryControlModel_Base* createClone_Impl(
            ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable >& _rxAggregateInstance);

        // XTypeProvider
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw (::com::sun::star::uno::RuntimeException);
    };

    //====================================================================
    //= OCommonGeometryControlModel
    //====================================================================
    /** allows to extend an arbitrary <type scope="com.sun.star.awt">UnoControlModel</type> with geometry
        information.
    */
    class OCommonGeometryControlModel
        :public OGeometryControlModel_Base
        ,public ::comphelper::OIdPropertyArrayUsageHelper< OCommonGeometryControlModel >
    {
    private:
        ::rtl::OUString m_sServiceSpecifier;        // the service specifier of our aggregate
        sal_Int32       m_nPropertyMapId;           // our unique property info id, used to look up in s_aAggregateProperties

    public:
        /** instantiate the model

            @param _rxAgg
                the instance to aggregate. Must support the <type scope="com.sun.star.awt">UnoControlModel</type>
                (this is not checked here)
        */
        OCommonGeometryControlModel(
                    ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable >& _rxAgg,
            const   ::rtl::OUString& _rxServiceSpecifier
        );

        // OIdPropertyArrayUsageHelper overridables
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper(sal_Int32 nId) const;

        // OPropertySetAggregationHelper overridables
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

        // OGeometryControlModel_Base
        virtual OGeometryControlModel_Base* createClone_Impl(
            ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable >& _rxAggregateInstance);

        // XTypeProvider
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw (::com::sun::star::uno::RuntimeException);

    private:
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rValue)
            throw (::com::sun::star::uno::Exception);
    };

#include "toolkit/controls/geometrycontrolmodel_impl.hxx"

//........................................................................
// }    // namespace toolkit
//........................................................................

#endif // _TOOLKIT_HELPERS_GEOMETRYCONTROLMODEL_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
