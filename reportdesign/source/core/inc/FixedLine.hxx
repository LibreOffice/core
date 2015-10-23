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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_FIXEDLINE_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_FIXEDLINE_HXX

#include <cppuhelper/propertysetmixin.hxx>
#include <com/sun/star/report/XFixedLine.hpp>
#include "ReportControlModel.hxx"
#include <cppuhelper/compbase.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include "ReportHelperDefines.hxx"


namespace reportdesign
{
    typedef ::cppu::PropertySetMixin<        css::report::XFixedLine > FixedLinePropertySet;
    typedef ::cppu::WeakComponentImplHelper<    css::report::XFixedLine
                                                ,css::lang::XServiceInfo > FixedLineBase;

    /** \class OFixedLine Defines the implementation of a \interface com:::sun::star::report::XFixedLine
     * \ingroup reportdesign_api
     *
     */
    class OFixedLine :  public comphelper::OBaseMutex,
                            public FixedLineBase,
                            public FixedLinePropertySet
    {
        friend class OShapeHelper;
        OReportControlModel     m_aProps;
        css::drawing::LineStyle m_LineStyle;
        css::drawing::LineDash  m_LineDash;
        sal_Int32               m_nOrientation;
        ::sal_Int32             m_LineColor;
        ::sal_Int16             m_LineTransparence;
        ::sal_Int32             m_LineWidth;

    private:
        OFixedLine(const OFixedLine&) = delete;
        OFixedLine& operator=(const OFixedLine&) = delete;

        template <typename T> void set(  const OUString& _sProperty
                                        ,const T& _Value
                                        ,T& _member)
        {
            BoundListeners l;
            {
                ::osl::MutexGuard aGuard(m_aMutex);
                prepareSet(_sProperty, css::uno::makeAny(_member), css::uno::makeAny(_Value), &l);
                _member = _Value;
            }
            l.notify();
        }
    protected:
        virtual ~OFixedLine();
    public:
        explicit OFixedLine(css::uno::Reference< css::uno::XComponentContext > const & _xContext);
        explicit OFixedLine(css::uno::Reference< css::uno::XComponentContext > const & _xContext
                            ,const css::uno::Reference< css::lang::XMultiServiceFactory > & _xFactory
                            ,css::uno::Reference< css::drawing::XShape >& _xShape
                            ,sal_Int32 _nOrientation);

        DECLARE_XINTERFACE( )
        // css::lang::XServiceInfo
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(css::uno::RuntimeException, std::exception) override;

        static css::uno::Sequence< OUString > getSupportedServiceNames_Static() throw( css::uno::RuntimeException );
        static OUString getImplementationName_Static() throw( css::uno::RuntimeException );
        static css::uno::Reference< css::uno::XInterface > SAL_CALL
            create(css::uno::Reference< css::uno::XComponentContext > const & xContext);
        // css::beans::XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) throw (css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

        // XReportComponent
        REPORTCOMPONENT_HEADER()

        // XShape
        SHAPE_HEADER()

        // XShapeDescriptor
        virtual OUString SAL_CALL getShapeType(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XReportControlModel
        REPORTCONTROLMODEL_HEADER()

        // XFixedLine
        virtual ::sal_Int32 SAL_CALL getOrientation() throw (css::beans::UnknownPropertyException,css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setOrientation( ::sal_Int32 _orientation ) throw (css::beans::UnknownPropertyException,css::uno::RuntimeException, std::exception) override;
        virtual css::drawing::LineStyle SAL_CALL getLineStyle() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setLineStyle( css::drawing::LineStyle _linestyle ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::drawing::LineDash SAL_CALL getLineDash() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setLineDash( const css::drawing::LineDash& _linedash ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getLineColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setLineColor( ::sal_Int32 _linecolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int16 SAL_CALL getLineTransparence() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setLineTransparence( ::sal_Int16 _linetransparence ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getLineWidth() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setLineWidth( ::sal_Int32 _linewidth ) throw (css::uno::RuntimeException, std::exception) override;

        // css::report::XReportControlFormat
        REPORTCONTROLFORMAT_HEADER()

        // XCloneable
        virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) throw (css::uno::RuntimeException, std::exception) override;

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

        // XChild
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) throw (css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;

        // XContainer
        virtual void SAL_CALL addContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;

        // XElementAccess
        virtual css::uno::Type SAL_CALL getElementType(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL hasElements(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XIndexReplace
        virtual void SAL_CALL replaceByIndex( ::sal_Int32 Index, const css::uno::Any& Element ) throw (css::lang::IllegalArgumentException, css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

        // XIndexContainer
        virtual void SAL_CALL insertByIndex( ::sal_Int32 Index, const css::uno::Any& Element ) throw (css::lang::IllegalArgumentException, css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeByIndex( ::sal_Int32 Index ) throw (css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

        // XIndexAccess
        virtual ::sal_Int32 SAL_CALL getCount(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    };
}
#endif // INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_FIXEDLINE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
