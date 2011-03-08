/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef REPORTDESIGN_API_GROUP_HXX
#define REPORTDESIGN_API_GROUP_HXX

#include <com/sun/star/report/XGroup.hpp>
#include <cppuhelper/compbase2.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/propertysetmixin.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include "GroupProperties.hxx"
#include <comphelper/uno3.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace reportdesign
{
    typedef ::cppu::WeakComponentImplHelper2< com::sun::star::report::XGroup
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
        OGroup& operator=(const OGroup&);
        OGroup(const OGroup&);

        template <typename T> void set(  const ::rtl::OUString& _sProperty
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
        void setSection(     const ::rtl::OUString& _sProperty
                            ,const sal_Bool& _bOn
                            ,const ::rtl::OUString& _sName
                            ,::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _member);
    protected:
        // TODO: VirtualFunctionFinder: This is virtual function!
        //
        virtual ~OGroup();

        /** this function is called upon disposing the component
        */
        // TODO: VirtualFunctionFinder: This is virtual function!
        //
        virtual void SAL_CALL disposing();
    public:
        OGroup(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroups >& _xParent
            ,const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& context);

        void copyGroup(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroup >& _xSource);

        DECLARE_XINTERFACE( )
        // ::com::sun::star::lang::XServiceInfo
        virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );

        // com::sun::star::beans::XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // XGroup
        virtual ::sal_Bool SAL_CALL getSortAscending() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setSortAscending( ::sal_Bool _sortascending ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL getHeaderOn() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setHeaderOn( ::sal_Bool _headeron ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL getFooterOn() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setFooterOn( ::sal_Bool _footeron ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection > SAL_CALL getHeader() throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection > SAL_CALL getFooter() throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int16 SAL_CALL getGroupOn() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setGroupOn( ::sal_Int16 _groupon ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getGroupInterval() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setGroupInterval( ::sal_Int32 _groupinterval ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int16 SAL_CALL getKeepTogether() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setKeepTogether( ::sal_Int16 _keeptogether ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroups > SAL_CALL getGroups() throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getExpression() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setExpression( const ::rtl::OUString& _expression ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL getStartNewColumn() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setStartNewColumn( ::sal_Bool _startnewcolumn ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL getResetPageNumber() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setResetPageNumber( ::sal_Bool _resetpagenumber ) throw (::com::sun::star::uno::RuntimeException);

        //XFunctionsSupplier
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::report::XFunctions > SAL_CALL getFunctions() throw (::com::sun::star::uno::RuntimeException);

        // XChild
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getParent(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

        // XComponent
        virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw(::com::sun::star::uno::RuntimeException)
        {
            cppu::WeakComponentImplHelperBase::addEventListener(aListener);
        }
        virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw(::com::sun::star::uno::RuntimeException)
        {
            cppu::WeakComponentImplHelperBase::removeEventListener(aListener);
        }

        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > getContext(){ return m_xContext; }
    };
// =============================================================================
} // namespace reportdesign
// =============================================================================
#endif //

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
