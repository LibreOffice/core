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
#include <comphelper/sequence.hxx>
#include <cppuhelper/propertysetmixin.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include "GroupProperties.hxx"
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
                                        ,const T& _Value
                                        ,T& _member)
        {
            BoundListeners l;
            {
                ::osl::MutexGuard aGuard(m_aMutex);
                if ( _member != _Value )
                {
                    prepareSet(_sProperty, css::uno::makeAny(_member), css::uno::makeAny(_Value), &l);
                    _member = _Value;
                }
            }
            l.notify();
        }
        void set(  const OUString& _sProperty
                  ,bool _Value
                  ,bool& _member)
        {
            BoundListeners l;
            {
                ::osl::MutexGuard aGuard(m_aMutex);
                if ( _member != _Value )
                {
                    prepareSet(_sProperty, css::uno::makeAny(_member), css::uno::makeAny(_Value), &l);
                    _member = _Value;
                }
            }
            l.notify();
        }
        void setSection(     const OUString& _sProperty
                            ,const bool& _bOn
                            ,const OUString& _sName
                            ,css::uno::Reference< css::report::XSection>& _member);
    protected:
        // TODO: VirtualFunctionFinder: This is virtual function!

        virtual ~OGroup();

        /** this function is called upon disposing the component
        */
        // TODO: VirtualFunctionFinder: This is virtual function!

        virtual void SAL_CALL disposing() override;
    public:
        OGroup(const css::uno::Reference< css::report::XGroups >& _xParent
            ,const css::uno::Reference< css::uno::XComponentContext >& context);

        DECLARE_XINTERFACE( )
        // css::lang::XServiceInfo
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(css::uno::RuntimeException, std::exception) override;

        static css::uno::Sequence< OUString > getSupportedServiceNames_Static() throw( css::uno::RuntimeException );

        // css::beans::XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) throw (css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

        // XGroup
        virtual sal_Bool SAL_CALL getSortAscending() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setSortAscending( sal_Bool _sortascending ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL getHeaderOn() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setHeaderOn( sal_Bool _headeron ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL getFooterOn() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setFooterOn( sal_Bool _footeron ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::report::XSection > SAL_CALL getHeader() throw (css::container::NoSuchElementException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::report::XSection > SAL_CALL getFooter() throw (css::container::NoSuchElementException, css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int16 SAL_CALL getGroupOn() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setGroupOn( ::sal_Int16 _groupon ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getGroupInterval() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setGroupInterval( ::sal_Int32 _groupinterval ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int16 SAL_CALL getKeepTogether() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setKeepTogether( ::sal_Int16 _keeptogether ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::report::XGroups > SAL_CALL getGroups() throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getExpression() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setExpression( const OUString& _expression ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL getStartNewColumn() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setStartNewColumn( sal_Bool _startnewcolumn ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL getResetPageNumber() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setResetPageNumber( sal_Bool _resetpagenumber ) throw (css::uno::RuntimeException, std::exception) override;

        //XFunctionsSupplier
        virtual css::uno::Reference< css::report::XFunctions > SAL_CALL getFunctions() throw (css::uno::RuntimeException, std::exception) override;

        // XChild
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) throw (css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;

        // XComponent
        virtual void SAL_CALL dispose() throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) throw(css::uno::RuntimeException, std::exception) override
        {
            cppu::WeakComponentImplHelperBase::addEventListener(aListener);
        }
        virtual void SAL_CALL removeEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) throw(css::uno::RuntimeException, std::exception) override
        {
            cppu::WeakComponentImplHelperBase::removeEventListener(aListener);
        }

        css::uno::Reference< css::uno::XComponentContext > getContext(){ return m_xContext; }
    };

} // namespace reportdesign

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
