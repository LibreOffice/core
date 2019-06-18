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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_GROUP_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_GROUP_HXX

#include <com/sun/star/report/XGroup.hpp>
#include <cppuhelper/compbase.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <cppuhelper/propertysetmixin.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <GroupProperties.hxx>
#include <comphelper/uno3.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace reportdesign
{
    typedef ::cppu::WeakComponentImplHelper< css::report::XGroup
                                         ,   css::lang::XServiceInfo> GroupBase;
    typedef ::cppu::PropertySetMixin< css::report::XGroup> GroupPropertySet;

    /** \class OGroup Defines the implementation of a \interface com:::sun::star::report::XGroup
     * \ingroup reportdesign_api
     *
     */
    class OGroup :   public comphelper::OMutexAndBroadcastHelper
                    ,public GroupBase
                    ,public GroupPropertySet
    {
        css::uno::Reference< css::uno::XComponentContext >    m_xContext;
        css::uno::WeakReference< css::report::XGroups >       m_xParent;
        css::uno::Reference< css::report::XSection>           m_xHeader;
        css::uno::Reference< css::report::XSection>           m_xFooter;
        css::uno::Reference< css::report::XFunctions >        m_xFunctions;
        ::rptshared::GroupProperties                          m_aProps;

    private:
        OGroup& operator=(const OGroup&) = delete;
        OGroup(const OGroup&) = delete;

        template <typename T> void set(  const OUString& _sProperty
                                        ,const T& Value
                                        ,T& _member)
        {
            BoundListeners l;
            {
                ::osl::MutexGuard aGuard(m_aMutex);
                if ( _member != Value )
                {
                    prepareSet(_sProperty, css::uno::makeAny(_member), css::uno::makeAny(Value), &l);
                    _member = Value;
                }
            }
            l.notify();
        }
        void set(  const OUString& _sProperty
                  ,bool Value
                  ,bool& _member)
        {
            BoundListeners l;
            {
                ::osl::MutexGuard aGuard(m_aMutex);
                if ( _member != Value )
                {
                    prepareSet(_sProperty, css::uno::makeAny(_member), css::uno::makeAny(Value), &l);
                    _member = Value;
                }
            }
            l.notify();
        }
        void setSection(     const OUString& _sProperty
                            ,bool _bOn
                            ,const OUString& _sName
                            ,css::uno::Reference< css::report::XSection>& _member);
    protected:
        // TODO: VirtualFunctionFinder: This is virtual function!

        virtual ~OGroup() override;

        /** this function is called upon disposing the component
        */
        // TODO: VirtualFunctionFinder: This is virtual function!

        virtual void SAL_CALL disposing() override;
    public:
        OGroup(const css::uno::Reference< css::report::XGroups >& _xParent
            ,const css::uno::Reference< css::uno::XComponentContext >& context);

        DECLARE_XINTERFACE( )
        // css::lang::XServiceInfo
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

        /// @throws css::uno::RuntimeException
        static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

        // css::beans::XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
        virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
        virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
        virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
        virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
        virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
        virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

        // XGroup
        virtual sal_Bool SAL_CALL getSortAscending() override;
        virtual void SAL_CALL setSortAscending( sal_Bool _sortascending ) override;
        virtual sal_Bool SAL_CALL getHeaderOn() override;
        virtual void SAL_CALL setHeaderOn( sal_Bool _headeron ) override;
        virtual sal_Bool SAL_CALL getFooterOn() override;
        virtual void SAL_CALL setFooterOn( sal_Bool _footeron ) override;
        virtual css::uno::Reference< css::report::XSection > SAL_CALL getHeader() override;
        virtual css::uno::Reference< css::report::XSection > SAL_CALL getFooter() override;
        virtual ::sal_Int16 SAL_CALL getGroupOn() override;
        virtual void SAL_CALL setGroupOn( ::sal_Int16 _groupon ) override;
        virtual ::sal_Int32 SAL_CALL getGroupInterval() override;
        virtual void SAL_CALL setGroupInterval( ::sal_Int32 _groupinterval ) override;
        virtual ::sal_Int16 SAL_CALL getKeepTogether() override;
        virtual void SAL_CALL setKeepTogether( ::sal_Int16 _keeptogether ) override;
        virtual css::uno::Reference< css::report::XGroups > SAL_CALL getGroups() override;
        virtual OUString SAL_CALL getExpression() override;
        virtual void SAL_CALL setExpression( const OUString& _expression ) override;
        virtual sal_Bool SAL_CALL getStartNewColumn() override;
        virtual void SAL_CALL setStartNewColumn( sal_Bool _startnewcolumn ) override;
        virtual sal_Bool SAL_CALL getResetPageNumber() override;
        virtual void SAL_CALL setResetPageNumber( sal_Bool _resetpagenumber ) override;

        //XFunctionsSupplier
        virtual css::uno::Reference< css::report::XFunctions > SAL_CALL getFunctions() override;

        // XChild
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent(  ) override;
        virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) override;

        // XComponent
        virtual void SAL_CALL dispose() override;
        virtual void SAL_CALL addEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) override
        {
            cppu::WeakComponentImplHelperBase::addEventListener(aListener);
        }
        virtual void SAL_CALL removeEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) override
        {
            cppu::WeakComponentImplHelperBase::removeEventListener(aListener);
        }

        const css::uno::Reference< css::uno::XComponentContext >& getContext() const { return m_xContext; }
    };

} // namespace reportdesign

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
