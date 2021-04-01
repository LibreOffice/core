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

#pragma once

#include "commoncontrol.hxx"
#include <svtools/inettbc.hxx>
#include <svl/zforlist.hxx>

class SvNumberFormatsSupplierObj;

namespace pcr
{
    //= OFormatSampleControl
    typedef CommonBehaviourControl<css::inspection::XPropertyControl, weld::Container> OFormatSampleControl_Base;
    class OFormatSampleControl : public OFormatSampleControl_Base
    {
    private:
        std::unique_ptr<weld::FormattedSpinButton> m_xSpinButton;
        std::unique_ptr<weld::Entry> m_xEntry;

        DECL_LINK(KeyInputHdl, const KeyEvent&, bool);

    public:
        OFormatSampleControl(std::unique_ptr<weld::Container> xWidget, std::unique_ptr<weld::Builder> xBuilder, bool bReadOnly);

        // XPropertyControl
        virtual css::uno::Any SAL_CALL getValue() override;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) override;
        virtual css::uno::Type SAL_CALL getValueType() override;

        virtual void SAL_CALL disposing() override
        {
            m_xEntry.reset();
            m_xSpinButton.reset();
            OFormatSampleControl_Base::disposing();
        }

        virtual void SetModifyHandler() override
        {
            m_xEntry->connect_focus_in( LINK( this, CommonBehaviourControlHelper, GetFocusHdl ) );
            m_xEntry->connect_focus_out( LINK( this, CommonBehaviourControlHelper, LoseFocusHdl ) );
            m_xSpinButton->connect_value_changed(LINK(this, CommonBehaviourControlHelper, FormattedModifiedHdl));
            m_xSpinButton->connect_changed(LINK(this, CommonBehaviourControlHelper, EditModifiedHdl));
        }

        void SetFormatSupplier(const SvNumberFormatsSupplierObj* pSupplier);

        virtual weld::Widget* getWidget() override { return getTypedControlWindow(); }

        /** returns the default preview value for the given format key
        *
        * \param _pNF the number formatter
        * \param _nFormatKey the format key
        * \return current date or time or the value 1234.56789
        */
        static double getPreviewValue(SvNumberFormatter const * pNF, sal_Int32 nFormatKey);

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
    typedef CommonBehaviourControl<css::inspection::XPropertyControl, weld::FormattedSpinButton> OFormattedNumericControl_Base;
    class OFormattedNumericControl : public OFormattedNumericControl_Base
    {
    public:
        OFormattedNumericControl(std::unique_ptr<weld::FormattedSpinButton> xWidget, std::unique_ptr<weld::Builder> xBuilder, bool bReadOnly);

        // XPropertyControl
        virtual css::uno::Any SAL_CALL getValue() override;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) override;
        virtual css::uno::Type SAL_CALL getValueType() override;

        void SetFormatDescription( const FormatDescription& rDesc );

        // make some FormattedField methods available
        void SetDecimalDigits(sal_uInt16 nPrecision) { getTypedControlWindow()->GetFormatter().SetDecimalDigits(nPrecision); }
        void SetDefaultValue(double dDef) { getTypedControlWindow()->GetFormatter().SetDefaultValue(dDef); }

        virtual void SetModifyHandler() override
        {
            OFormattedNumericControl_Base::SetModifyHandler();
            getTypedControlWindow()->connect_value_changed(LINK(this, CommonBehaviourControlHelper, FormattedModifiedHdl));
            getTypedControlWindow()->connect_changed(LINK(this, CommonBehaviourControlHelper, EditModifiedHdl));
        }

        virtual weld::Widget* getWidget() override { return getTypedControlWindow(); }

    protected:
        virtual ~OFormattedNumericControl() override;
    };

    //= OFileUrlControl
    typedef CommonBehaviourControl<css::inspection::XPropertyControl, SvtURLBox> OFileUrlControl_Base;
    class OFileUrlControl : public OFileUrlControl_Base
    {
    private:
        DECL_LINK(URLModifiedHdl, weld::ComboBox&, void);
    public:
        OFileUrlControl(std::unique_ptr<SvtURLBox> xWidget, std::unique_ptr<weld::Builder> xBuilder, bool bReadOnly);

        // XPropertyControl
        virtual css::uno::Any SAL_CALL getValue() override;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) override;
        virtual css::uno::Type SAL_CALL getValueType() override;

        virtual void SetModifyHandler() override
        {
            OFileUrlControl_Base::SetModifyHandler();
            SvtURLBox* pControlWindow = getTypedControlWindow();
            // tdf#140239 and tdf#141084 don't notify that the control has changed content until focus-out
            pControlWindow->connect_focus_out(LINK(this, CommonBehaviourControlHelper, LoseFocusHdl));
            pControlWindow->connect_changed(LINK(this, OFileUrlControl, URLModifiedHdl));
        }

        virtual weld::Widget* getWidget() override { return getTypedControlWindow()->getWidget(); }

    protected:
        virtual ~OFileUrlControl() override;
    };

} // namespace pcr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
