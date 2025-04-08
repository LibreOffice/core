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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_UI_INC_DEFAULTINSPECTION_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_UI_INC_DEFAULTINSPECTION_HXX

#include <com/sun/star/inspection/XObjectInspectorModel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <cppuhelper/implbase3.hxx>
#include <mutex>


namespace rptui
{

    //= DefaultComponentInspectorModel

    typedef ::cppu::WeakAggImplHelper3  <   css::inspection::XObjectInspectorModel
                                        ,   css::lang::XServiceInfo
                                        ,   css::lang::XInitialization
                                        >   DefaultComponentInspectorModel_Base;
    class DefaultComponentInspectorModel final : public DefaultComponentInspectorModel_Base
    {
    private:
        std::mutex                                                                              m_aMutex;
        css::uno::Reference< css::uno::XComponentContext >                                      m_xContext;
        css::uno::Reference< css::inspection::XObjectInspectorModel >                           m_xComponent; /// delegatee
        bool                                                                                    m_bConstructed;
        bool                                                                                    m_bHasHelpSection;
        bool                                                                                    m_bIsReadOnly;
        sal_Int32                                                                               m_nMinHelpTextLines;
        sal_Int32                                                                               m_nMaxHelpTextLines;

        DefaultComponentInspectorModel(const DefaultComponentInspectorModel&) = delete;
        DefaultComponentInspectorModel& operator=(const DefaultComponentInspectorModel&) = delete;

        virtual ~DefaultComponentInspectorModel() override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

        // XObjectInspectorModel
        virtual css::uno::Sequence< css::uno::Any > SAL_CALL getHandlerFactories() override;
        virtual sal_Bool SAL_CALL getHasHelpSection() override;
        virtual ::sal_Int32 SAL_CALL getMinHelpTextLines() override;
        virtual ::sal_Int32 SAL_CALL getMaxHelpTextLines() override;
        virtual sal_Bool SAL_CALL getIsReadOnly() override;
        virtual void SAL_CALL setIsReadOnly( sal_Bool _isreadonly ) override;

        virtual css::uno::Sequence< css::inspection::PropertyCategoryDescriptor > SAL_CALL describeCategories(  ) override;
        virtual ::sal_Int32 SAL_CALL getPropertyOrderIndex( const OUString& PropertyName ) override;

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

        // Service constructors
        void    createWithHelpSection( sal_Int32 _nMinHelpTextLines, sal_Int32 _nMaxHelpTextLines );

    public:
        DefaultComponentInspectorModel( const css::uno::Reference< css::uno::XComponentContext >& _rxContext);
    };


} // namespace rptui


#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_DEFAULTINSPECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
