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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_SHAPE_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_SHAPE_HXX

#include <cppuhelper/propertysetmixin.hxx>
#include <com/sun/star/report/XShape.hpp>
#include "ReportControlModel.hxx"
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <ReportHelperDefines.hxx>
#include <comphelper/propagg.hxx>
#include <memory>

namespace reportdesign
{
    typedef ::cppu::PropertySetMixin<        css::report::XShape > ShapePropertySet;
    typedef ::cppu::WeakComponentImplHelper<    css::report::XShape
                                                ,css::lang::XServiceInfo > ShapeBase;

    /** \class OShape Defines the implementation of a \interface com:::sun::star::report::XShape
     * \ingroup reportdesign_api
     *
     */
    class OShape :  public cppu::BaseMutex,
                    public ShapeBase,
                    public ShapePropertySet
    {
        friend class OShapeHelper;
        ::std::unique_ptr< ::comphelper::OPropertyArrayAggregationHelper> m_pAggHelper;
        OReportControlModel                                             m_aProps;
        css::drawing::HomogenMatrix3                                    m_Transformation;
        sal_Int32                                                       m_nZOrder;
        bool                                                            m_bOpaque;

        OUString                                                        m_sServiceName;
        OUString                                                        m_CustomShapeEngine;
        OUString                                                        m_CustomShapeData;
        css::uno::Sequence< css::beans::PropertyValue >                 m_CustomShapeGeometry;

    private:
        OShape(const OShape&) = delete;
        OShape& operator=(const OShape&) = delete;

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
        cppu::IPropertyArrayHelper& getInfoHelper();
    protected:
        virtual ~OShape() override;
    public:
        explicit OShape(css::uno::Reference< css::uno::XComponentContext > const & _xContext);
        explicit OShape(css::uno::Reference< css::uno::XComponentContext > const & _xContext
                        ,const css::uno::Reference< css::lang::XMultiServiceFactory > & _xFactory
                        ,css::uno::Reference< css::drawing::XShape >& _xShape
                        ,const OUString& _sServiceName);

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

        virtual OUString SAL_CALL getCustomShapeEngine() override;
        virtual void SAL_CALL setCustomShapeEngine( const OUString& _customshapeengine ) override;
        virtual OUString SAL_CALL getCustomShapeData() override;
        virtual void SAL_CALL setCustomShapeData( const OUString& _customshapedata ) override;
        virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getCustomShapeGeometry() override;
        virtual void SAL_CALL setCustomShapeGeometry( const css::uno::Sequence< css::beans::PropertyValue >& _customshapegeometry ) override;

        virtual sal_Bool SAL_CALL getOpaque() override;
        virtual void SAL_CALL setOpaque( sal_Bool _opaque ) override;

        // XShapeDescriptor
        virtual OUString SAL_CALL getShapeType(  ) override;

        // XReportControlModel
        REPORTCONTROLMODEL_HEADER()

        // XReportControlFormat
        REPORTCONTROLFORMAT_HEADER()
        // XShape
        virtual ::sal_Int32 SAL_CALL getZOrder() override;
        virtual void SAL_CALL setZOrder( ::sal_Int32 _zorder ) override;
        virtual css::drawing::HomogenMatrix3 SAL_CALL getTransformation() override;
        virtual void SAL_CALL setTransformation( const css::drawing::HomogenMatrix3& _transformation ) override;

        // XCloneable
        virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) override;

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
#endif // INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_SHAPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
