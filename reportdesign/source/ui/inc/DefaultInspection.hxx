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

#include <memory>


namespace rptui
{

    class OPropertyInfoService;

    //= DefaultComponentInspectorModel

    typedef ::cppu::WeakAggImplHelper3  <   ::com::sun::star::inspection::XObjectInspectorModel
                                        ,   ::com::sun::star::lang::XServiceInfo
                                        ,   ::com::sun::star::lang::XInitialization
                                        >   DefaultComponentInspectorModel_Base;
    class DefaultComponentInspectorModel : public DefaultComponentInspectorModel_Base
    {
    private:
        ::osl::Mutex                                                                            m_aMutex;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >            m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorModel > m_xComponent; /// delegatee
        bool                                                                                    m_bConstructed;
        bool                                                                                    m_bHasHelpSection;
        bool                                                                                m_bIsReadOnly;
        sal_Int32                                                                               m_nMinHelpTextLines;
        sal_Int32                                                                               m_nMaxHelpTextLines;
        /// access to property meta data
        ::std::unique_ptr< OPropertyInfoService >                                               m_pInfoService;

        DefaultComponentInspectorModel(const DefaultComponentInspectorModel&) = delete;
        DefaultComponentInspectorModel& operator=(const DefaultComponentInspectorModel&) = delete;
    protected:
        virtual ~DefaultComponentInspectorModel();

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

        // XObjectInspectorModel
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getHandlerFactories() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL getHasHelpSection() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getMinHelpTextLines() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getMaxHelpTextLines() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL getIsReadOnly() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setIsReadOnly( sal_Bool _isreadonly ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::inspection::PropertyCategoryDescriptor > SAL_CALL describeCategories(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getPropertyOrderIndex( const OUString& PropertyName ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) override;

    public:
        // XServiceInfo - static versions
        static OUString getImplementationName_Static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                        create(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&);

    public:
        DefaultComponentInspectorModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext);

    protected:
        // Service constructors
        void    createDefault();
        void    createWithHelpSection( sal_Int32 _nMinHelpTextLines, sal_Int32 _nMaxHelpTextLines );
    };


} // namespace rptui


#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_DEFAULTINSPECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
