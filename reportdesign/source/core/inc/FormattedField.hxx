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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_FORMATTEDFIELD_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_FORMATTEDFIELD_HXX

#include <cppuhelper/propertysetmixin.hxx>
#include <com/sun/star/report/XFormattedField.hpp>
#include <com/sun/star/report/Function.hpp>
#include "ReportControlModel.hxx"
#include <cppuhelper/compbase.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include "ReportHelperDefines.hxx"
namespace reportdesign
{
    typedef ::cppu::PropertySetMixin<        com::sun::star::report::XFormattedField    > FormattedFieldPropertySet;
    typedef ::cppu::WeakComponentImplHelper<    com::sun::star::report::XFormattedField
                                                ,com::sun::star::lang::XServiceInfo > FormattedFieldBase;

    /** \class OFormattedField Defines the implementation of a \interface com:::sun::star::report::XFormattedField
     * \ingroup reportdesign_api
     *
     */
    class OFormattedField : public comphelper::OBaseMutex,
                            public FormattedFieldBase,
                            public FormattedFieldPropertySet
    {
        friend class OShapeHelper;

        OReportControlModel                             m_aProps;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >
                                                        m_xFormatsSupplier;
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XFunction> m_xFunction;
        ::sal_Int32                                     m_nFormatKey;

    private:
        OFormattedField(const OFormattedField&) = delete;
        OFormattedField& operator=(const OFormattedField&) = delete;

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
    protected:
        virtual ~OFormattedField();
    public:
        explicit OFormattedField(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & _xContext);
        explicit OFormattedField(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & _xContext
                                 ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & _xFactory
                                 ,::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& _xShape);

        DECLARE_XINTERFACE( )
        // ::com::sun::star::lang::XServiceInfo
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_Static() throw( ::com::sun::star::uno::RuntimeException, std::exception );
        static OUString getImplementationName_Static() throw( ::com::sun::star::uno::RuntimeException, std::exception );
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

        // XReportComponent
        REPORTCOMPONENT_HEADER()

        // XShape
        SHAPE_HEADER()

        // XShapeDescriptor
        virtual OUString SAL_CALL getShapeType(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XReportControlModel
        REPORTCONTROLMODEL_HEADER()

        // XReportControlFormat
        REPORTCONTROLFORMAT_HEADER()

        // XCloneable
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XFormattedField
        virtual ::sal_Int32 SAL_CALL getFormatKey() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setFormatKey(::sal_Int32 the_value) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier > SAL_CALL getFormatsSupplier() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setFormatsSupplier( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& _formatssupplier ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

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
    };
}
#endif // INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_FORMATTEDFIELD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
