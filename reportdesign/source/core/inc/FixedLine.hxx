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
    typedef ::cppu::PropertySetMixin<        com::sun::star::report::XFixedLine > FixedLinePropertySet;
    typedef ::cppu::WeakComponentImplHelper<    com::sun::star::report::XFixedLine
                                                ,com::sun::star::lang::XServiceInfo > FixedLineBase;

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
        ::com::sun::star::drawing::LineStyle    m_LineStyle;
        ::com::sun::star::drawing::LineDash     m_LineDash;
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
                prepareSet(_sProperty, ::com::sun::star::uno::makeAny(_member), ::com::sun::star::uno::makeAny(_Value), &l);
                _member = _Value;
            }
            l.notify();
        }
    protected:
        virtual ~OFixedLine();
    public:
        explicit OFixedLine(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & _xContext);
        explicit OFixedLine(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & _xContext
                            ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & _xFactory
                            ,::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& _xShape
                            ,sal_Int32 _nOrientation);

        DECLARE_XINTERFACE( )
        // ::com::sun::star::lang::XServiceInfo
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_Static() throw( ::com::sun::star::uno::RuntimeException );
        static OUString getImplementationName_Static() throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
            create(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext);
        // com::sun::star::beans::XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XReportComponent
        REPORTCOMPONENT_HEADER()

        // XShape
        SHAPE_HEADER()

        // XShapeDescriptor
        virtual OUString SAL_CALL getShapeType(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XReportControlModel
        REPORTCONTROLMODEL_HEADER()

        // XFixedLine
        virtual ::sal_Int32 SAL_CALL getOrientation() throw (::com::sun::star::beans::UnknownPropertyException,::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setOrientation( ::sal_Int32 _orientation ) throw (::com::sun::star::beans::UnknownPropertyException,::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::drawing::LineStyle SAL_CALL getLineStyle() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setLineStyle( ::com::sun::star::drawing::LineStyle _linestyle ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::drawing::LineDash SAL_CALL getLineDash() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setLineDash( const ::com::sun::star::drawing::LineDash& _linedash ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::sal_Int32 SAL_CALL getLineColor() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setLineColor( ::sal_Int32 _linecolor ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::sal_Int16 SAL_CALL getLineTransparence() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setLineTransparence( ::sal_Int16 _linetransparence ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::sal_Int32 SAL_CALL getLineWidth() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setLineWidth( ::sal_Int32 _linewidth ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // ::com::sun::star::report::XReportControlFormat
        REPORTCONTROLFORMAT_HEADER()

        // XCloneable
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

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

        // XChild
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getParent(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XContainer
        virtual void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XElementAccess
        virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XIndexReplace
        virtual void SAL_CALL replaceByIndex( ::sal_Int32 Index, const ::com::sun::star::uno::Any& Element ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XIndexContainer
        virtual void SAL_CALL insertByIndex( ::sal_Int32 Index, const ::com::sun::star::uno::Any& Element ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL removeByIndex( ::sal_Int32 Index ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XIndexAccess
        virtual ::sal_Int32 SAL_CALL getCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    };
}
#endif // INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_FIXEDLINE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
