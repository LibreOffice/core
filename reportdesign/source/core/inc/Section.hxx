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
#ifndef REPORTDESIGN_API_SECTION_HXX
#define REPORTDESIGN_API_SECTION_HXX

#include <com/sun/star/report/XSection.hpp>
#include <cppuhelper/compbase6.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/types.hxx>
#include <cppuhelper/propertysetmixin.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XShapeGrouper.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/form/XFormsSupplier2.hpp>
#include <comphelper/stl_types.hxx>
#include <comphelper/implementationreference.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace reportdesign
{
    typedef ::cppu::WeakComponentImplHelper6
        <   ::com::sun::star::report::XSection
        ,   ::com::sun::star::lang::XServiceInfo
        ,   ::com::sun::star::lang::XUnoTunnel
        // SvxDrawPage forward
        ,   ::com::sun::star::drawing::XDrawPage
        ,   ::com::sun::star::drawing::XShapeGrouper
        // SvxFmDrawPage forward
        ,   ::com::sun::star::form::XFormsSupplier2
        > SectionBase;
    typedef ::cppu::PropertySetMixin<com::sun::star::report::XSection> SectionPropertySet;

    class OSection : public comphelper::OMutexAndBroadcastHelper,
                    public SectionBase,
                    public SectionPropertySet
    {
        ::cppu::OInterfaceContainerHelper                                                   m_aContainerListeners;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >        m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >            m_xDrawPage;
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapeGrouper >        m_xDrawPage_ShapeGrouper;
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormsSupplier2 >         m_xDrawPage_FormSupplier;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel >               m_xDrawPage_Tunnel;
        ::com::sun::star::uno::WeakReference< ::com::sun::star::report::XGroup >            m_xGroup;
        ::com::sun::star::uno::WeakReference< ::com::sun::star::report::XReportDefinition > m_xReportDefinition;
        ::rtl::OUString                                                                     m_sName;
        ::rtl::OUString                                                                     m_sConditionalPrintExpression;
        ::sal_uInt32                                                                        m_nHeight;
        ::sal_Int32                                                                         m_nBackgroundColor;
        ::sal_Int16                                                                         m_nForceNewPage;
        ::sal_Int16                                                                         m_nNewRowOrCol;
        ::sal_Bool                                                                          m_bKeepTogether;
        ::sal_Bool                                                                          m_bCanGrow;
        ::sal_Bool                                                                          m_bCanShrink;
        ::sal_Bool                                                                          m_bRepeatSection;
        ::sal_Bool                                                                          m_bVisible;
        ::sal_Bool                                                                          m_bBacktransparent;
        bool                                                                                m_bInRemoveNotify;
        bool                                                                                m_bInInsertNotify;

    private:
        OSection(const OSection&);
        OSection& operator=(const OSection&);

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

        /** checks if this section is eiter the page header or footer and if so it throws an UnknownPropertyException
        *
        */
        void checkNotPageHeaderFooter();

        void init();
    protected:
        // TODO: VirtualFunctionFinder: This is virtual function!
        //
        virtual ~OSection();

        /** this function is called upon disposing the component
        */
        // TODO: VirtualFunctionFinder: This is virtual function!
        //
        virtual void SAL_CALL disposing();
    public:
        typedef ::comphelper::ImplementationReference< OSection ,::com::sun::star::report::XSection,::com::sun::star::uno::XWeak > TSection;

        OSection(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition >& _xParent
                ,const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& context,bool _bPageSection=false);
        OSection(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroup >& _xParent
                ,const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& context,bool _bPageSection=false);

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

        // XSection
        virtual ::sal_Bool SAL_CALL getVisible() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setVisible( ::sal_Bool _visible ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getName() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setName( const ::rtl::OUString& _name ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_uInt32 SAL_CALL getHeight() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setHeight( ::sal_uInt32 _height ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getBackColor() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setBackColor( ::sal_Int32 _backgroundcolor ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL getBackTransparent() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setBackTransparent( ::sal_Bool _backtransparent ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getConditionalPrintExpression() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setConditionalPrintExpression( const ::rtl::OUString& _conditionalprintexpression ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int16 SAL_CALL getForceNewPage() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setForceNewPage( ::sal_Int16 _forcenewpage ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int16 SAL_CALL getNewRowOrCol() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setNewRowOrCol( ::sal_Int16 _newroworcol ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL getKeepTogether() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setKeepTogether( ::sal_Bool _keeptogether ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL getCanGrow() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setCanGrow( ::sal_Bool _cangrow ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL getCanShrink() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setCanShrink( ::sal_Bool _canshrink ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL getRepeatSection() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setRepeatSection( ::sal_Bool _repeatsection ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroup > SAL_CALL getGroup() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition > SAL_CALL getReportDefinition() throw (::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent > SAL_CALL createReportComponent( const ::rtl::OUString& _sReportComponentSpecifier ) throw (::com::sun::star::uno::Exception, ::com::sun::star::lang::IllegalArgumentException,::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAvailableReportComponentNames(  ) throw (::com::sun::star::uno::RuntimeException);
        // XChild
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getParent(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);
        // XContainer
        virtual void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        // XElementAccess
        virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException);
        // XShapes
        virtual void SAL_CALL add( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL remove( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw (::com::sun::star::uno::RuntimeException);
        // XShapeGrouper
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapeGroup > SAL_CALL group( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xShapes ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL ungroup( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapeGroup >& aGroup ) throw (::com::sun::star::uno::RuntimeException);

        // XFormsSupplier
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > SAL_CALL getForms() throw (::com::sun::star::uno::RuntimeException);
        // XFormsSupplier2
        virtual sal_Bool SAL_CALL hasForms() throw (::com::sun::star::uno::RuntimeException);

        // XIndexAccess
        virtual ::sal_Int32 SAL_CALL getCount(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        // XEnumerationAccess
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createEnumeration(  ) throw (::com::sun::star::uno::RuntimeException);

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

        // com::sun::star::lang::XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
        static OSection* getImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxComponent );
        static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

        static void lcl_copySection(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _xSource
                                         ,::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _xDest);

        void notifyElementAdded(const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape);
        void notifyElementRemoved(const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape);
    };
}
#endif //REPORTDESIGN_API_SECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
