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

#ifndef INCLUDED_TOOLKIT_CONTROLS_FORMATTEDCONTROL_HXX
#define INCLUDED_TOOLKIT_CONTROLS_FORMATTEDCONTROL_HXX

#include <toolkit/controls/unocontrols.hxx>
#include <toolkit/controls/unocontrolmodel.hxx>
#include <toolkit/helper/servicenames.hxx>

#include <com/sun/star/util/XNumberFormatter.hpp>


namespace toolkit
{



    // = UnoControlFormattedFieldModel

    class UnoControlFormattedFieldModel : public UnoControlModel
    {
    protected:
        ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
        ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;
        ::com::sun::star::uno::Any      m_aCachedFormat;
        bool                            m_bRevokedAsClient;
        bool                            m_bSettingValueAndText;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >
                                        m_xCachedFormatter;

    protected:
        sal_Bool SAL_CALL convertFastPropertyValue(
                    ::com::sun::star::uno::Any& rConvertedValue,
                    ::com::sun::star::uno::Any& rOldValue,
                    sal_Int32 nPropId,
                    const ::com::sun::star::uno::Any& rValue
                ) throw (::com::sun::star::lang::IllegalArgumentException, std::exception) override;

        void SAL_CALL setFastPropertyValue_NoBroadcast(
                    sal_Int32 nHandle,
                    const ::com::sun::star::uno::Any& rValue
                ) throw (::com::sun::star::uno::Exception, std::exception) override;

    public:
        UnoControlFormattedFieldModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
        UnoControlFormattedFieldModel( const UnoControlFormattedFieldModel& rModel )
            : UnoControlModel(rModel)
            , m_bRevokedAsClient(false)
            , m_bSettingValueAndText(false)
        {
        }

        UnoControlModel*    Clone() const override { return new UnoControlFormattedFieldModel( *this ); }

        // ::com::sun::star::io::XPersistObject
        OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

        // ::com::sun::star::beans::XMultiPropertySet
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;


        // ::com::sun::star::lang::XServiceInfo
        OUString SAL_CALL getImplementationName()
            throw (css::uno::RuntimeException, std::exception) override;

        css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
            throw (css::uno::RuntimeException, std::exception) override;

    protected:
        virtual ~UnoControlFormattedFieldModel();

        // XComponent
        void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

        // XPropertySet
        void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& PropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Values ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

        // UnoControlModel
        virtual void ImplNormalizePropertySequence(
                        const sal_Int32                 _nCount,        /// the number of entries in the arrays
                        sal_Int32*                      _pHandles,      /// the handles of the properties to set
                        ::com::sun::star::uno::Any*     _pValues,       /// the values of the properties to set
                        sal_Int32*                      _pValidHandles  /// pointer to the valid handles, allowed to be adjusted
                    )   const override;
    private:
        void    impl_updateTextFromValue_nothrow();
        void    impl_updateCachedFormatter_nothrow();
        void    impl_updateCachedFormatKey_nothrow();
    };


    // = UnoFormattedFieldControl

    class UnoFormattedFieldControl : public UnoSpinFieldControl
    {
    public:
                            UnoFormattedFieldControl();
        OUString     GetComponentServiceName() override;

        // ::com::sun::star::awt::XTextListener
        void SAL_CALL textChanged( const ::com::sun::star::awt::TextEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

        // ::com::sun::star::lang::XServiceInfo
        OUString SAL_CALL getImplementationName()
            throw (css::uno::RuntimeException, std::exception) override;

        css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
            throw (css::uno::RuntimeException, std::exception) override;
    };


}   // namespace toolkit


#endif // INCLUDED_TOOLKIT_CONTROLS_FORMATTEDCONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
