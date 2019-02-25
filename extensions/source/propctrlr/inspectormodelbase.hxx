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

#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_INSPECTORMODELBASE_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_INSPECTORMODELBASE_HXX

#include <com/sun/star/inspection/XObjectInspectorModel.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/propshlp.hxx>

#include <comphelper/broadcasthelper.hxx>
#include <comphelper/uno3.hxx>

#include <memory>


namespace pcr
{


    class InspectorModelProperties;

    //= ImplInspectorModel

    typedef ::cppu::WeakImplHelper <   css::inspection::XObjectInspectorModel
                                    ,   css::lang::XInitialization
                                    ,   css::lang::XServiceInfo
                                    >   ImplInspectorModel_Base;
    typedef ::cppu::OPropertySetHelper  ImplInspectorModel_PBase;

    class ImplInspectorModel
            :public ::comphelper::OMutexAndBroadcastHelper
            ,public ImplInspectorModel_Base
            ,public ImplInspectorModel_PBase
    {
        std::unique_ptr< InspectorModelProperties >                     m_pProperties;

    protected:
        virtual ~ImplInspectorModel() override;

    public:
        ImplInspectorModel();

        DECLARE_XINTERFACE()
        DECLARE_XTYPEPROVIDER()

        // css::beans::XPropertySet and friends
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
        virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;
        virtual sal_Bool SAL_CALL convertFastPropertyValue( css::uno::Any & rConvertedValue, css::uno::Any & rOldValue, sal_Int32 nHandle, const css::uno::Any& rValue ) override;
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const css::uno::Any& rValue ) override;
        virtual void SAL_CALL getFastPropertyValue( css::uno::Any& rValue, sal_Int32 nHandle ) const override;

        // css::inspection::XObjectInspectorModel
        virtual sal_Bool SAL_CALL getHasHelpSection() override;
        virtual ::sal_Int32 SAL_CALL getMinHelpTextLines() override;
        virtual ::sal_Int32 SAL_CALL getMaxHelpTextLines() override;
        virtual sal_Bool SAL_CALL getIsReadOnly() override;
        virtual void SAL_CALL setIsReadOnly( sal_Bool IsReadOnly ) override;

        // css::lang::XServiceInfo
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;

    protected:
        void enableHelpSectionProperties( sal_Int32 _nMinHelpTextLines, sal_Int32 _nMaxHelpTextLines );

    private:
        using ImplInspectorModel_PBase::getFastPropertyValue;
    };


} // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_INSPECTORMODELBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
