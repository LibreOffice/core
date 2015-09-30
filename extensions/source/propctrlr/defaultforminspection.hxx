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
#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_DEFAULTFORMINSPECTION_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_DEFAULTFORMINSPECTION_HXX

#include "inspectormodelbase.hxx"

#include <memory>


namespace pcr
{


    class OPropertyInfoService;

    class DefaultFormComponentInspectorModel : public ImplInspectorModel
    {
    private:
        bool                                        m_bUseFormComponentHandlers;
        bool                                        m_bConstructed;

        /// access to property meta data
        ::std::unique_ptr< OPropertyInfoService >   m_pInfoService;

    protected:
        virtual ~DefaultFormComponentInspectorModel();

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XObjectInspectorModel
        virtual css::uno::Sequence< css::uno::Any > SAL_CALL getHandlerFactories() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Sequence< css::inspection::PropertyCategoryDescriptor > SAL_CALL describeCategories(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::sal_Int32 SAL_CALL getPropertyOrderIndex( const OUString& PropertyName ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    public:
        // XServiceInfo - static versions
        static OUString getImplementationName_static(  ) throw(css::uno::RuntimeException);
        static css::uno::Sequence< OUString > getSupportedServiceNames_static(  ) throw(css::uno::RuntimeException);
        static css::uno::Reference< css::uno::XInterface > SAL_CALL
                        Create(const css::uno::Reference< css::uno::XComponentContext >&);

    public:
        DefaultFormComponentInspectorModel( bool _bUseFormFormComponentHandlers = true );

    protected:
        // Service constructors
        void    createDefault();
        void    createWithHelpSection( sal_Int32 _nMinHelpTextLines, sal_Int32 _nMaxHelpTextLines );
    };


} // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_DEFAULTFORMINSPECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
