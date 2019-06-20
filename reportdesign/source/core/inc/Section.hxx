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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_SECTION_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_SECTION_HXX

#include <com/sun/star/report/XSection.hpp>
#include <cppuhelper/compbase.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/types.hxx>
#include <cppuhelper/propertysetmixin.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XShapeGrouper.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/form/XFormsSupplier2.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <rtl/ref.hxx>

namespace reportdesign
{
    typedef ::cppu::WeakComponentImplHelper
        <   css::report::XSection
        ,   css::lang::XServiceInfo
        ,   css::lang::XUnoTunnel
        // SvxDrawPage forward
        ,   css::drawing::XDrawPage
        ,   css::drawing::XShapeGrouper
        // SvxFmDrawPage forward
        ,   css::form::XFormsSupplier2
        > SectionBase;
    typedef ::cppu::PropertySetMixin<css::report::XSection> SectionPropertySet;

    class OSection : public comphelper::OMutexAndBroadcastHelper,
                    public SectionBase,
                    public SectionPropertySet
    {
        ::comphelper::OInterfaceContainerHelper2                                            m_aContainerListeners;
        css::uno::Reference< css::drawing::XDrawPage >                                      m_xDrawPage;
        css::uno::Reference< css::drawing::XShapeGrouper >                                  m_xDrawPage_ShapeGrouper;
        css::uno::Reference< css::form::XFormsSupplier2 >                                   m_xDrawPage_FormSupplier;
        css::uno::Reference< css::lang::XUnoTunnel >                                        m_xDrawPage_Tunnel;
        css::uno::WeakReference< css::report::XGroup >                                      m_xGroup;
        css::uno::WeakReference< css::report::XReportDefinition >                           m_xReportDefinition;
        OUString                                                                            m_sName;
        OUString                                                                            m_sConditionalPrintExpression;
        ::sal_uInt32                                                                        m_nHeight;
        ::sal_Int32                                                                         m_nBackgroundColor;
        ::sal_Int16                                                                         m_nForceNewPage;
        ::sal_Int16                                                                         m_nNewRowOrCol;
        bool                                                                                m_bKeepTogether;
        bool                                                                                m_bRepeatSection;
        bool                                                                                m_bVisible;
        bool                                                                                m_bBacktransparent;
        bool                                                                                m_bInRemoveNotify;
        bool                                                                                m_bInInsertNotify;

    private:
        OSection(const OSection&) = delete;
        OSection& operator=(const OSection&) = delete;

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

        /** checks if this section is either the page header or footer and if so it throws an UnknownPropertyException
        *
        */
        void checkNotPageHeaderFooter();

        void init();
    protected:
        // TODO: VirtualFunctionFinder: This is virtual function!

        virtual ~OSection() override;

        /** this function is called upon disposing the component
        */
        // TODO: VirtualFunctionFinder: This is virtual function!

        virtual void SAL_CALL disposing() override;
    private:
        OSection(const css::uno::Reference< css::report::XReportDefinition >& xParentDef
                ,const css::uno::Reference< css::report::XGroup >& xParentGroup
                ,const css::uno::Reference< css::uno::XComponentContext >& context,
                css::uno::Sequence< OUString> const&);
    public:
        static css::uno::Reference< css::report::XSection>
        createOSection(const css::uno::Reference< css::report::XReportDefinition >& _xParent
                ,const css::uno::Reference< css::uno::XComponentContext >& context,bool _bPageSection=false);
        static css::uno::Reference< css::report::XSection>
        createOSection(const css::uno::Reference< css::report::XGroup >& _xParent
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

        // XSection
        virtual sal_Bool SAL_CALL getVisible() override;
        virtual void SAL_CALL setVisible( sal_Bool _visible ) override;
        virtual OUString SAL_CALL getName() override;
        virtual void SAL_CALL setName( const OUString& _name ) override;
        virtual ::sal_uInt32 SAL_CALL getHeight() override;
        virtual void SAL_CALL setHeight( ::sal_uInt32 _height ) override;
        virtual ::sal_Int32 SAL_CALL getBackColor() override;
        virtual void SAL_CALL setBackColor( ::sal_Int32 _backgroundcolor ) override;
        virtual sal_Bool SAL_CALL getBackTransparent() override;
        virtual void SAL_CALL setBackTransparent( sal_Bool _backtransparent ) override;
        virtual OUString SAL_CALL getConditionalPrintExpression() override;
        virtual void SAL_CALL setConditionalPrintExpression( const OUString& _conditionalprintexpression ) override;
        virtual ::sal_Int16 SAL_CALL getForceNewPage() override;
        virtual void SAL_CALL setForceNewPage( ::sal_Int16 _forcenewpage ) override;
        virtual ::sal_Int16 SAL_CALL getNewRowOrCol() override;
        virtual void SAL_CALL setNewRowOrCol( ::sal_Int16 _newroworcol ) override;
        virtual sal_Bool SAL_CALL getKeepTogether() override;
        virtual void SAL_CALL setKeepTogether( sal_Bool _keeptogether ) override;
        virtual sal_Bool SAL_CALL getCanGrow() override;
        virtual void SAL_CALL setCanGrow( sal_Bool _cangrow ) override;
        virtual sal_Bool SAL_CALL getCanShrink() override;
        virtual void SAL_CALL setCanShrink( sal_Bool _canshrink ) override;
        virtual sal_Bool SAL_CALL getRepeatSection() override;
        virtual void SAL_CALL setRepeatSection( sal_Bool _repeatsection ) override;
        virtual css::uno::Reference< css::report::XGroup > SAL_CALL getGroup() override;
        virtual css::uno::Reference< css::report::XReportDefinition > SAL_CALL getReportDefinition() override;

        // XChild
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent(  ) override;
        virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) override;
        // XContainer
        virtual void SAL_CALL addContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) override;
        virtual void SAL_CALL removeContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) override;
        // XElementAccess
        virtual css::uno::Type SAL_CALL getElementType(  ) override;
        virtual sal_Bool SAL_CALL hasElements(  ) override;
        // XShapes
        virtual void SAL_CALL add( const css::uno::Reference< css::drawing::XShape >& xShape ) override;
        virtual void SAL_CALL remove( const css::uno::Reference< css::drawing::XShape >& xShape ) override;
        // XShapeGrouper
        virtual css::uno::Reference< css::drawing::XShapeGroup > SAL_CALL group( const css::uno::Reference< css::drawing::XShapes >& xShapes ) override;
        virtual void SAL_CALL ungroup( const css::uno::Reference< css::drawing::XShapeGroup >& aGroup ) override;

        // XFormsSupplier
        virtual css::uno::Reference< css::container::XNameContainer > SAL_CALL getForms() override;
        // XFormsSupplier2
        virtual sal_Bool SAL_CALL hasForms() override;

        // XIndexAccess
        virtual ::sal_Int32 SAL_CALL getCount(  ) override;
        virtual css::uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) override;
        // XEnumerationAccess
        virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration(  ) override;

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

        // css::lang::XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;
        static css::uno::Sequence< sal_Int8 > getUnoTunnelId();

        void notifyElementAdded(const css::uno::Reference< css::drawing::XShape >& xShape);
        void notifyElementRemoved(const css::uno::Reference< css::drawing::XShape >& xShape);
    };
}
#endif // INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_SECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
