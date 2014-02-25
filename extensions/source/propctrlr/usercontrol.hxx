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

#ifndef _EXTENSIONS_PROPCTRLR_USERCONTROL_HXX_
#define _EXTENSIONS_PROPCTRLR_USERCONTROL_HXX_

#include "commoncontrol.hxx"
#include <svtools/fmtfield.hxx>
#include <svtools/fileurlbox.hxx>
#include "standardcontrol.hxx"

class SvNumberFormatsSupplierObj;

//............................................................................
namespace pcr
{
//............................................................................

    //========================================================================
    //= NumberFormatSampleField
    //========================================================================
    class NumberFormatSampleField : public ControlWindow< FormattedField >
    {
    private:
        typedef ControlWindow< FormattedField > BaseClass;

    public:
        NumberFormatSampleField( Window* _pParent, WinBits _nStyle )
            :BaseClass( _pParent, _nStyle )
        {
        }

        void SetFormatSupplier( const SvNumberFormatsSupplierObj* pSupplier );

    protected:
        virtual bool PreNotify( NotifyEvent& rNEvt );
    };

    //========================================================================
    //= OFormatSampleControl
    //========================================================================
    typedef CommonBehaviourControl< ::com::sun::star::inspection::XPropertyControl, NumberFormatSampleField > OFormatSampleControl_Base;
    class OFormatSampleControl : public OFormatSampleControl_Base
    {
    public:
        OFormatSampleControl( Window* pParent, WinBits nWinStyle );

        // XPropertyControl
        virtual ::com::sun::star::uno::Any SAL_CALL getValue() throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL setValue( const ::com::sun::star::uno::Any& _value ) throw (::com::sun::star::beans::IllegalTypeException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Type SAL_CALL getValueType() throw (::com::sun::star::uno::RuntimeException, std::exception);

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

    //========================================================================
    //= FormatDescription
    //========================================================================
    struct FormatDescription
    {
        SvNumberFormatsSupplierObj*     pSupplier;
        sal_Int32                       nKey;
    };

    //========================================================================
    //= OFormattedNumericControl
    //========================================================================
    typedef CommonBehaviourControl< ::com::sun::star::inspection::XPropertyControl, ControlWindow< FormattedField > > OFormattedNumericControl_Base;
    class OFormattedNumericControl : public OFormattedNumericControl_Base
    {
    private:
        sal_Int32   m_nLastDecimalDigits;

    public:
        OFormattedNumericControl( Window* pParent, WinBits nWinStyle = WB_TABSTOP);

        // XPropertyControl
        virtual ::com::sun::star::uno::Any SAL_CALL getValue() throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL setValue( const ::com::sun::star::uno::Any& _value ) throw (::com::sun::star::beans::IllegalTypeException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Type SAL_CALL getValueType() throw (::com::sun::star::uno::RuntimeException, std::exception);

        void SetFormatDescription( const FormatDescription& rDesc );

        // make some FormattedField methods available
        void SetDecimalDigits(sal_uInt16 nPrecision) { getTypedControlWindow()->SetDecimalDigits(nPrecision); m_nLastDecimalDigits = nPrecision; }
        void SetDefaultValue(double dDef) { getTypedControlWindow()->SetDefaultValue(dDef); }
        void EnableEmptyField(sal_Bool bEnable) { getTypedControlWindow()->EnableEmptyField(bEnable); }
        void SetThousandsSep(sal_Bool bEnable) { getTypedControlWindow()->SetThousandsSep(bEnable); }

    protected:
        ~OFormattedNumericControl();
    };

    //========================================================================
    //= OFileUrlControl
    //========================================================================
    typedef CommonBehaviourControl< ::com::sun::star::inspection::XPropertyControl, ControlWindow< ::svt::FileURLBox > > OFileUrlControl_Base;
    class OFileUrlControl : public OFileUrlControl_Base
    {
    public:
        OFileUrlControl( Window* pParent, WinBits nWinStyle );

        // XPropertyControl
        virtual ::com::sun::star::uno::Any SAL_CALL getValue() throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL setValue( const ::com::sun::star::uno::Any& _value ) throw (::com::sun::star::beans::IllegalTypeException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Type SAL_CALL getValueType() throw (::com::sun::star::uno::RuntimeException, std::exception);

    protected:
        ~OFileUrlControl();
    };

    //========================================================================
    //= OTimeDurationControl
    //========================================================================
    class OTimeDurationControl : public ONumericControl
    {
    public:
        OTimeDurationControl( ::Window* pParent, WinBits nWinStyle );
        ~OTimeDurationControl();

        // XPropertyControl
        ::sal_Int16 SAL_CALL getControlType() throw (::com::sun::star::uno::RuntimeException);

    private:
        DECL_LINK( OnCustomConvert, MetricField* );
    };

//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_USERCONTROL_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
