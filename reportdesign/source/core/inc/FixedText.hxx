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

#ifndef INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_FIXEDTEXT_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_FIXEDTEXT_HXX

#include <cppuhelper/propertysetmixin.hxx>
#include <com/sun/star/report/XFixedText.hpp>
#include "ReportControlModel.hxx"
#include <comphelper/uno3.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <ReportHelperDefines.hxx>


namespace reportdesign
{
    typedef ::cppu::PropertySetMixin<        css::report::XFixedText > FixedTextPropertySet;
    typedef ::cppu::WeakComponentImplHelper<    css::report::XFixedText
                                                ,css::lang::XServiceInfo > FixedTextBase;

    /** \class OFixedText Defines the implementation of a \interface com:::sun::star::report::XFixedText
     * \ingroup reportdesign_api
     *
     */
    class OFixedText :  public cppu::BaseMutex,
                            public FixedTextBase,
                            public FixedTextPropertySet
    {
        friend class OShapeHelper;
        OReportControlModel                 m_aProps;
        OUString                     m_sLabel;
    private:
        OFixedText(const OFixedText&) = delete;
        OFixedText& operator=(const OFixedText&) = delete;

        // internally, we store PROPERTY_PARAADJUST as css::style::ParagraphAdjust, but externally the property is visible as a sal_Int16
        void set(  const OUString& _sProperty
                  ,sal_Int16 Value
                  ,css::style::ParagraphAdjust& _member)
        {
            BoundListeners l;
            {
                ::osl::MutexGuard aGuard(m_aMutex);
                if ( static_cast<sal_Int16>(_member) != Value )
                {
                    prepareSet(_sProperty, css::uno::makeAny(static_cast<sal_Int16>(_member)), css::uno::makeAny(Value), &l);
                    _member = static_cast<css::style::ParagraphAdjust>(Value);
                }
            }
            l.notify();
        }
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
    protected:
        virtual ~OFixedText() override;
    public:
        explicit OFixedText(css::uno::Reference< css::uno::XComponentContext > const & _xContext);
        explicit OFixedText(css::uno::Reference< css::uno::XComponentContext > const & _xContext
                            ,const css::uno::Reference< css::lang::XMultiServiceFactory > & _xFactory
                            ,css::uno::Reference< css::drawing::XShape >& _xShape);

        DECLARE_XINTERFACE( )
        // css::lang::XServiceInfo
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

        // css::beans::XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
        virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
        virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
        virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
        virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
        virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
        virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

        // XReportComponent
        REPORTCOMPONENT_HEADER()
        // XShape
        SHAPE_HEADER()

        // XShapeDescriptor
        virtual OUString SAL_CALL getShapeType(  ) override;

        // XReportControlModel
        REPORTCONTROLMODEL_HEADER()

        // XCloneable
        virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) override;

        // XFixedText
        virtual OUString SAL_CALL getLabel() override;
        virtual void SAL_CALL setLabel( const OUString& _label ) override;

        // css::report::XReportControlFormat
        REPORTCONTROLFORMAT_HEADER()

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

        // XChild
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent(  ) override;
        virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) override;
        // XContainer
        virtual void SAL_CALL addContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) override;
        virtual void SAL_CALL removeContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) override;

        // XElementAccess
        virtual css::uno::Type SAL_CALL getElementType(  ) override;
        virtual sal_Bool SAL_CALL hasElements(  ) override;

        // XIndexReplace
        virtual void SAL_CALL replaceByIndex( ::sal_Int32 Index, const css::uno::Any& Element ) override;

        // XIndexContainer
        virtual void SAL_CALL insertByIndex( ::sal_Int32 Index, const css::uno::Any& Element ) override;
        virtual void SAL_CALL removeByIndex( ::sal_Int32 Index ) override;

        // XIndexAccess
        virtual ::sal_Int32 SAL_CALL getCount(  ) override;
        virtual css::uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) override;
    };
}
#endif // INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_FIXEDTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
