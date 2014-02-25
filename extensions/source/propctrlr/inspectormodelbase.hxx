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

#ifndef INSPECTORMODELBASE_HXX
#define INSPECTORMODELBASE_HXX

#include <com/sun/star/inspection/XObjectInspectorModel.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/propshlp.hxx>

#include <comphelper/broadcasthelper.hxx>
#include <comphelper/uno3.hxx>

#include <memory>


namespace pcr
{


    class InspectorModelProperties;

    //= ImplInspectorModel

    typedef ::cppu::WeakImplHelper3 <   ::com::sun::star::inspection::XObjectInspectorModel
                                    ,   ::com::sun::star::lang::XInitialization
                                    ,   ::com::sun::star::lang::XServiceInfo
                                    >   ImplInspectorModel_Base;
    typedef ::cppu::OPropertySetHelper  ImplInspectorModel_PBase;

    class ImplInspectorModel
            :public ::comphelper::OMutexAndBroadcastHelper
            ,public ImplInspectorModel_Base
            ,public ImplInspectorModel_PBase
    {
    protected:
        ::std::auto_ptr< InspectorModelProperties >                     m_pProperties;

    protected:
        ~ImplInspectorModel();

    public:
        ImplInspectorModel();

        DECLARE_XINTERFACE()
        DECLARE_XTYPEPROVIDER()

        // ::com::sun::star::beans::XPropertySet and friends
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();
        virtual sal_Bool SAL_CALL convertFastPropertyValue( ::com::sun::star::uno::Any & rConvertedValue, ::com::sun::star::uno::Any & rOldValue, sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::lang::IllegalArgumentException);
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::uno::Exception, std::exception);
        virtual void SAL_CALL getFastPropertyValue( ::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;

        // ::com::sun::star::inspection::XObjectInspectorModel
        virtual ::sal_Bool SAL_CALL getHasHelpSection() throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::sal_Int32 SAL_CALL getMinHelpTextLines() throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::sal_Int32 SAL_CALL getMaxHelpTextLines() throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::sal_Bool SAL_CALL getIsReadOnly() throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL setIsReadOnly( ::sal_Bool _IsReadOnly ) throw (::com::sun::star::uno::RuntimeException, std::exception);

        // ::com::sun::star::lang::XServiceInfo
        virtual ::sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    protected:
        void enableHelpSectionProperties( sal_Int32 _nMinHelpTextLines, sal_Int32 _nMaxHelpTextLines );

    private:
        using ImplInspectorModel_PBase::getFastPropertyValue;
    };


} // namespace pcr


#endif // INSPECTORMODELBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
