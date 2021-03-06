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

#include "inspectormodelbase.hxx"

#include <memory>


namespace pcr
{


    class OPropertyInfoService;

    class DefaultFormComponentInspectorModel final : public ImplInspectorModel
    {
        bool                                        m_bUseFormComponentHandlers;
        bool                                        m_bConstructed;

        /// access to property meta data
        std::unique_ptr< OPropertyInfoService >   m_pInfoService;

        virtual ~DefaultFormComponentInspectorModel() override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

        // XObjectInspectorModel
        virtual css::uno::Sequence< css::uno::Any > SAL_CALL getHandlerFactories() override;
        virtual css::uno::Sequence< css::inspection::PropertyCategoryDescriptor > SAL_CALL describeCategories(  ) override;
        virtual ::sal_Int32 SAL_CALL getPropertyOrderIndex( const OUString& PropertyName ) override;

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    public:
        explicit DefaultFormComponentInspectorModel( bool _bUseFormFormComponentHandlers = true );

    private:
        // Service constructors
        void    createWithHelpSection( sal_Int32 _nMinHelpTextLines, sal_Int32 _nMaxHelpTextLines );
    };


} // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
