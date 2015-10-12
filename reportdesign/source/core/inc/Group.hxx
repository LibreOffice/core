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
    typedef ::cppu::WeakComponentImplHelper< com::sun::star::report::XGroup
                                         ,   ::com::sun::star::lang::XServiceInfo> GroupBase;
    typedef ::cppu::PropertySetMixin< com::sun::star::report::XGroup> GroupPropertySet;

    /** \class OGroup Defines the implementation of a \interface com:::sun::star::report::XGroup
     * \ingroup reportdesign_api
     *
     */
    class OGroup :   public comphelper::OMutexAndBroadcastHelper
                    ,public GroupBase
                    ,public GroupPropertySet
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >    m_xContext;
        ::com::sun::star::uno::WeakReference< ::com::sun::star::report::XGroups >       m_xParent;
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>           m_xHeader;
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>           m_xFooter;
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XFunctions >        m_xFunctions;
        ::rptshared::GroupProperties                                                    m_aProps;

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
                    prepareSet(_sProperty, ::com::sun::star::uno::makeAny(_member), ::com::sun::star::uno::makeAny(_Value), &l);
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
                    prepareSet(_sProperty, ::com::sun::star::uno::makeAny(_member), ::com::sun::star::uno::makeAny(_Value), &l);
                    _member = _Value;
                }
            }
            l.notify();
        }
        void setSection(     const OUString& _sProperty
                            ,const bool& _bOn
                            ,const OUString& _sName
                            ,::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _member);
    protected:
        // TODO: VirtualFunctionFinder: This is virtual function!

        virtual ~OGroup();

        /** this function is called upon disposing the component
        */
        // TODO: VirtualFunctionFinder: This is virtual function!

        virtual void SAL_CALL disposing() SAL_OVERRIDE;
    public:
        OGroup(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroups >& _xParent
            ,const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& context);

        DECLARE_XINTERFACE( )
        // ::com::sun::star::lang::XServiceInfo
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_Static() throw( ::com::sun::star::uno::RuntimeException );

        // com::sun::star::beans::XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XGroup
        virtual sal_Bool SAL_CALL getSortAscending() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setSortAscending( sal_Bool _sortascending ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL getHeaderOn() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setHeaderOn( sal_Bool _headeron ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL getFooterOn() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setFooterOn( sal_Bool _footeron ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection > SAL_CALL getHeader() throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection > SAL_CALL getFooter() throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::sal_Int16 SAL_CALL getGroupOn() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setGroupOn( ::sal_Int16 _groupon ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::sal_Int32 SAL_CALL getGroupInterval() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setGroupInterval( ::sal_Int32 _groupinterval ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::sal_Int16 SAL_CALL getKeepTogether() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setKeepTogether( ::sal_Int16 _keeptogether ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroups > SAL_CALL getGroups() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual OUString SAL_CALL getExpression() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setExpression( const OUString& _expression ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL getStartNewColumn() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setStartNewColumn( sal_Bool _startnewcolumn ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL getResetPageNumber() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setResetPageNumber( sal_Bool _resetpagenumber ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        //XFunctionsSupplier
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::report::XFunctions > SAL_CALL getFunctions() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XChild
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getParent(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XComponent
        virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
            cppu::WeakComponentImplHelperBase::addEventListener(aListener);
        }
        virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
            cppu::WeakComponentImplHelperBase::removeEventListener(aListener);
        }

        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > getContext(){ return m_xContext; }
    };

} // namespace reportdesign

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
