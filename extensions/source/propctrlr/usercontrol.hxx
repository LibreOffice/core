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

#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_USERCONTROL_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_USERCONTROL_HXX

#include "commoncontrol.hxx"
#include <svtools/fmtfield.hxx>
#include <svtools/fileurlbox.hxx>
#include "standardcontrol.hxx"

class SvNumberFormatsSupplierObj;


namespace pcr
{



    //= NumberFormatSampleField

    class NumberFormatSampleField : public FormattedField
    {
    public:
        NumberFormatSampleField( vcl::Window* _pParent, WinBits _nStyle )
            : FormattedField(_pParent, _nStyle)
            , m_pHelper(nullptr)
        {
        }

        void         SetFormatSupplier( const SvNumberFormatsSupplierObj* pSupplier );
        void         setControlHelper( CommonBehaviourControlHelper& _rControlHelper ) { m_pHelper = &_rControlHelper; }

    protected:
        virtual bool PreNotify( NotifyEvent& rNEvt ) override;
    private:
        CommonBehaviourControlHelper* m_pHelper;
    };


    //= OFormatSampleControl

    typedef CommonBehaviourControl< css::inspection::XPropertyControl, NumberFormatSampleField > OFormatSampleControl_Base;
    class OFormatSampleControl : public OFormatSampleControl_Base
    {
    public:
        OFormatSampleControl( vcl::Window* pParent, WinBits nWinStyle );

        // XPropertyControl
        virtual css::uno::Any SAL_CALL getValue() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) throw (css::beans::IllegalTypeException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Type SAL_CALL getValueType() throw (css::uno::RuntimeException, std::exception) override;

        inline void SetFormatSupplier( const SvNumberFormatsSupplierObj* _pSupplier )
        {
            getTypedControlWindow()->SetFormatSupplier( _pSupplier );
        }

        /** returns the default preview value for the given format key
        *
        * \param _pNF the number formatter
        * \param _nFormatKey the format key
        * \return current date or time or the value 1234.56789
        */
        static double getPreviewValue(SvNumberFormatter* _pNF,sal_Int32 _nFormatKey);

    private:
        static double getPreviewValue( const SvNumberformat& i_rEntry );
    };


    //= FormatDescription

    struct FormatDescription
    {
        SvNumberFormatsSupplierObj*     pSupplier;
        sal_Int32                       nKey;
    };


    //= OFormattedNumericControl

    typedef CommonBehaviourControl< css::inspection::XPropertyControl, FormattedField > OFormattedNumericControl_Base;
    class OFormattedNumericControl : public OFormattedNumericControl_Base
    {
    private:
        sal_Int32   m_nLastDecimalDigits;

    public:
        OFormattedNumericControl( vcl::Window* pParent, WinBits nWinStyle = WB_TABSTOP);

        // XPropertyControl
        virtual css::uno::Any SAL_CALL getValue() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) throw (css::beans::IllegalTypeException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Type SAL_CALL getValueType() throw (css::uno::RuntimeException, std::exception) override;

        void SetFormatDescription( const FormatDescription& rDesc );

        // make some FormattedField methods available
        void SetDecimalDigits(sal_uInt16 nPrecision) { getTypedControlWindow()->SetDecimalDigits(nPrecision); m_nLastDecimalDigits = nPrecision; }
        void SetDefaultValue(double dDef) { getTypedControlWindow()->SetDefaultValue(dDef); }
        void EnableEmptyField(bool bEnable) { getTypedControlWindow()->EnableEmptyField(bEnable); }
        void SetThousandsSep(bool bEnable) { getTypedControlWindow()->SetThousandsSep(bEnable); }

    protected:
        virtual ~OFormattedNumericControl();
    };


    //= OFileUrlControl

    typedef CommonBehaviourControl< css::inspection::XPropertyControl, ::svt::FileURLBox > OFileUrlControl_Base;
    class OFileUrlControl : public OFileUrlControl_Base
    {
    public:
        OFileUrlControl( vcl::Window* pParent, WinBits nWinStyle );

        // XPropertyControl
        virtual css::uno::Any SAL_CALL getValue() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) throw (css::beans::IllegalTypeException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Type SAL_CALL getValueType() throw (css::uno::RuntimeException, std::exception) override;

    protected:
        virtual ~OFileUrlControl();
    };


    //= OTimeDurationControl

    class OTimeDurationControl : public ONumericControl
    {
    public:
        OTimeDurationControl( vcl::Window* pParent, WinBits nWinStyle );
        virtual ~OTimeDurationControl();

        // XPropertyControl
        ::sal_Int16 SAL_CALL getControlType() throw (css::uno::RuntimeException) override;

    private:
        DECL_LINK_TYPED( OnCustomConvert, MetricFormatter&, void );
    };


} // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_USERCONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
