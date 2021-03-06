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
#include "pcrcommon.hxx"

#include <com/sun/star/inspection/XNumericControl.hpp>
#include <com/sun/star/inspection/XStringListControl.hpp>
#include <com/sun/star/inspection/XHyperlinkControl.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <svtools/ctrlbox.hxx>
#include <svx/colorbox.hxx>

namespace pcr
{
    //= OTimeControl
    typedef CommonBehaviourControl<css::inspection::XPropertyControl, weld::FormattedSpinButton> OTimeControl_Base;
    class OTimeControl : public OTimeControl_Base
    {
        std::unique_ptr<weld::TimeFormatter> m_xFormatter;
    public:
        OTimeControl(std::unique_ptr<weld::FormattedSpinButton> xWidget, std::unique_ptr<weld::Builder> xBuilder, bool bReadOnly);

        virtual void SAL_CALL disposing() override
        {
            m_xFormatter.reset();
            OTimeControl_Base::disposing();
        }

        // XPropertyControl
        virtual css::uno::Any SAL_CALL getValue() override;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) override;
        virtual css::uno::Type SAL_CALL getValueType() override;

        virtual void SetModifyHandler() override
        {
            OTimeControl_Base::SetModifyHandler();
            getTypedControlWindow()->connect_value_changed( LINK( this, CommonBehaviourControlHelper, TimeModifiedHdl ) );
        }

        virtual weld::Widget* getWidget() override { return getTypedControlWindow(); }
    };

    //= ODateControl
    typedef CommonBehaviourControl<css::inspection::XPropertyControl, SvtCalendarBox> ODateControl_Base;
    class ODateControl : public ODateControl_Base
    {
    public:
        ODateControl(std::unique_ptr<SvtCalendarBox> xWidget, std::unique_ptr<weld::Builder> xBuilder, bool bReadOnly);

        // XPropertyControl
        virtual css::uno::Any SAL_CALL getValue() override;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) override;
        virtual css::uno::Type SAL_CALL getValueType() override;

        virtual void SetModifyHandler() override
        {
            ODateControl_Base::SetModifyHandler();
            getTypedControlWindow()->connect_selected( LINK( this, CommonBehaviourControlHelper, DateModifiedHdl ) );
        }

        virtual weld::Widget* getWidget() override { return &getTypedControlWindow()->get_button(); }
    };

    //= OEditControl
    typedef CommonBehaviourControl<css::inspection::XPropertyControl, weld::Entry> OEditControl_Base;
    class OEditControl final : public OEditControl_Base
    {
        bool m_bIsPassword : 1;

    public:
        OEditControl(std::unique_ptr<weld::Entry> xWidget, std::unique_ptr<weld::Builder> xBuilder, bool bPassWord, bool bReadOnly);

        // XPropertyControl
        virtual css::uno::Any SAL_CALL getValue() override;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) override;
        virtual css::uno::Type SAL_CALL getValueType() override;

        virtual void SetModifyHandler() override
        {
            OEditControl_Base::SetModifyHandler();
            getTypedControlWindow()->connect_changed( LINK( this, CommonBehaviourControlHelper, EditModifiedHdl ) );
        }

    private:
        // CommonBehaviourControlHelper::modified
        virtual void setModified() override;
        virtual weld::Widget* getWidget() override { return getTypedControlWindow(); }
    };

    //= ODateTimeControl
    typedef CommonBehaviourControl<css::inspection::XPropertyControl, weld::Container> ODateTimeControl_Base;
    class ODateTimeControl : public ODateTimeControl_Base
    {
    private:
        std::unique_ptr<SvtCalendarBox> m_xDate;
        std::unique_ptr<weld::FormattedSpinButton> m_xTime;
        std::unique_ptr<weld::TimeFormatter> m_xFormatter;

    public:
        ODateTimeControl(std::unique_ptr<weld::Container> xWidget, std::unique_ptr<weld::Builder> xBuilder, bool bReadOnly);

        virtual void SetModifyHandler() override
        {
            m_xDate->connect_focus_in( LINK( this, CommonBehaviourControlHelper, GetFocusHdl ) );
            m_xDate->connect_focus_out( LINK( this, CommonBehaviourControlHelper, LoseFocusHdl ) );
            m_xTime->connect_focus_in( LINK( this, CommonBehaviourControlHelper, GetFocusHdl ) );
            m_xTime->connect_focus_out( LINK( this, CommonBehaviourControlHelper, LoseFocusHdl ) );

            m_xDate->connect_selected( LINK( this, CommonBehaviourControlHelper, DateModifiedHdl ) );
            m_xTime->connect_value_changed( LINK( this, CommonBehaviourControlHelper, TimeModifiedHdl ) );
        }

        virtual void SAL_CALL disposing() override
        {
            m_xFormatter.reset();
            m_xTime.reset();
            m_xDate.reset();
            ODateTimeControl_Base::disposing();
        }

        // XPropertyControl
        virtual css::uno::Any SAL_CALL getValue() override;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) override;
        virtual css::uno::Type SAL_CALL getValueType() override;

        virtual weld::Widget* getWidget() override { return getTypedControlWindow(); }
    };

    //= OHyperlinkControl
    typedef CommonBehaviourControl<css::inspection::XHyperlinkControl, weld::Container> OHyperlinkControl_Base;
    class OHyperlinkControl final : public OHyperlinkControl_Base
    {
    private:
        std::unique_ptr<weld::Entry> m_xEntry;
        std::unique_ptr<weld::Button> m_xButton;

        ::comphelper::OInterfaceContainerHelper2 m_aActionListeners;

    public:
        OHyperlinkControl(std::unique_ptr<weld::Container> xWidget, std::unique_ptr<weld::Builder> xBuilder, bool bReadOnly);

        // XPropertyControl
        virtual css::uno::Any SAL_CALL getValue() override;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) override;
        virtual css::uno::Type SAL_CALL getValueType() override;

        virtual void SetModifyHandler() override
        {
            m_xEntry->connect_focus_in( LINK( this, CommonBehaviourControlHelper, GetFocusHdl ) );
            m_xEntry->connect_focus_out( LINK( this, CommonBehaviourControlHelper, LoseFocusHdl ) );
            m_xButton->connect_focus_in( LINK( this, CommonBehaviourControlHelper, GetFocusHdl ) );
            m_xButton->connect_focus_out( LINK( this, CommonBehaviourControlHelper, LoseFocusHdl ) );

            m_xEntry->connect_changed( LINK( this, CommonBehaviourControlHelper, EditModifiedHdl ) );
        }

        virtual weld::Widget* getWidget() override { return getTypedControlWindow(); }

        // XHyperlinkControl
        virtual void SAL_CALL addActionListener( const css::uno::Reference< css::awt::XActionListener >& listener ) override;
        virtual void SAL_CALL removeActionListener( const css::uno::Reference< css::awt::XActionListener >& listener ) override;

    private:
        // XComponent
        virtual void SAL_CALL disposing() override;

        DECL_LINK(OnHyperlinkClicked, weld::Button&, void);
    };

    //= ONumericControl
    typedef CommonBehaviourControl<css::inspection::XNumericControl, weld::MetricSpinButton> ONumericControl_Base;
    class ONumericControl : public ONumericControl_Base
    {
    private:
        FieldUnit   m_eValueUnit;
        sal_Int16   m_nFieldToUNOValueFactor;

    public:
        ONumericControl(std::unique_ptr<weld::MetricSpinButton> xWidget, std::unique_ptr<weld::Builder> xBuilder, bool bReadOnly);

        // XPropertyControl
        virtual css::uno::Any SAL_CALL getValue() override;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) override;
        virtual css::uno::Type SAL_CALL getValueType() override;

        // XNumericControl
        virtual ::sal_Int16 SAL_CALL getDecimalDigits() override;
        virtual void SAL_CALL setDecimalDigits( ::sal_Int16 _decimaldigits ) override;
        virtual css::beans::Optional< double > SAL_CALL getMinValue() override;
        virtual void SAL_CALL setMinValue( const css::beans::Optional< double >& _minvalue ) override;
        virtual css::beans::Optional< double > SAL_CALL getMaxValue() override;
        virtual void SAL_CALL setMaxValue( const css::beans::Optional< double >& _maxvalue ) override;
        virtual ::sal_Int16 SAL_CALL getDisplayUnit() override;
        virtual void SAL_CALL setDisplayUnit( ::sal_Int16 _displayunit ) override;
        virtual ::sal_Int16 SAL_CALL getValueUnit() override;
        virtual void SAL_CALL setValueUnit( ::sal_Int16 _valueunit ) override;

        virtual void SetModifyHandler() override
        {
            ONumericControl_Base::SetModifyHandler();
            weld::MetricSpinButton* pSpinButton = getTypedControlWindow();
            pSpinButton->connect_value_changed( LINK( this, CommonBehaviourControlHelper, MetricModifiedHdl ) );
            pSpinButton->get_widget().connect_changed( LINK( this, CommonBehaviourControlHelper, EditModifiedHdl ) );
        }

    private:
        virtual weld::Widget* getWidget() override { return &getTypedControlWindow()->get_widget(); }

        /** converts an API value (<code>double</code>, as passed into <code>set[Max|Min|]Value) into
            a <code>int</code> value which can be passed to our NumericField.

            The conversion respects our decimal digits as well as our value factor (<member>m_nFieldToUNOValueFactor</member>).
        */
        int    impl_apiValueToFieldValue_nothrow( double nApiValue ) const;

        /** converts a control value, as obtained from our Numeric field, into a value which can passed
            to outer callers via our UNO API.
        */
        double  impl_fieldValueToApiValue_nothrow( int nFieldValue ) const;
    };

    //= OColorControl
    typedef CommonBehaviourControl<css::inspection::XPropertyControl, ColorListBox> OColorControl_Base;
    class OColorControl : public OColorControl_Base
    {
    public:
        OColorControl(std::unique_ptr<ColorListBox> xWidget, std::unique_ptr<weld::Builder> xBuilder, bool bReadOnly);

        // XPropertyControl
        virtual css::uno::Any SAL_CALL getValue() override;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) override;
        virtual css::uno::Type SAL_CALL getValueType() override;

        virtual void SetModifyHandler() override
        {
            OColorControl_Base::SetModifyHandler();
            getTypedControlWindow()->SetSelectHdl(LINK(this, CommonBehaviourControlHelper, ColorModifiedHdl));
        }

    protected:
        // CommonBehaviourControlHelper::setModified
        virtual void setModified() override;

    private:
        virtual weld::Widget* getWidget() override { return &getTypedControlWindow()->get_widget(); }
    };

    //= OListboxControl
    typedef CommonBehaviourControl<css::inspection::XStringListControl, weld::ComboBox> OListboxControl_Base;
    class OListboxControl : public OListboxControl_Base
    {
    public:
        OListboxControl(std::unique_ptr<weld::ComboBox> xWidget, std::unique_ptr<weld::Builder> xBuilder, bool bReadOnly);

        // XPropertyControl
        virtual css::uno::Any SAL_CALL getValue() override;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) override;
        virtual css::uno::Type SAL_CALL getValueType() override;

        // XStringListControl
        virtual void SAL_CALL clearList(  ) override;
        virtual void SAL_CALL prependListEntry( const OUString& NewEntry ) override;
        virtual void SAL_CALL appendListEntry( const OUString& NewEntry ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getListEntries(  ) override;

        virtual void SetModifyHandler() override
        {
            OListboxControl_Base::SetModifyHandler();
            getTypedControlWindow()->connect_changed(LINK(this, CommonBehaviourControlHelper, ModifiedHdl));
        }

    protected:
        // CommonBehaviourControlHelper::setModified
        virtual void setModified() override;
        virtual weld::Widget* getWidget() override { return getTypedControlWindow(); }
    };

    //= OComboboxControl
    typedef CommonBehaviourControl< css::inspection::XStringListControl, weld::ComboBox > OComboboxControl_Base;
    class OComboboxControl final : public OComboboxControl_Base
    {
    public:
        OComboboxControl(std::unique_ptr<weld::ComboBox> xWidget, std::unique_ptr<weld::Builder> xBuilder, bool bReadOnly);

        // XPropertyControl
        virtual css::uno::Any SAL_CALL getValue() override;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) override;
        virtual css::uno::Type SAL_CALL getValueType() override;

        // XStringListControl
        virtual void SAL_CALL clearList(  ) override;
        virtual void SAL_CALL prependListEntry( const OUString& NewEntry ) override;
        virtual void SAL_CALL appendListEntry( const OUString& NewEntry ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getListEntries(  ) override;

        virtual void SetModifyHandler() override
        {
            OComboboxControl_Base::SetModifyHandler();
            getTypedControlWindow()->connect_changed(LINK(this, CommonBehaviourControlHelper, ModifiedHdl));
        }

        // CommonBehaviourControlHelper::setModified
        virtual weld::Widget* getWidget() override { return getTypedControlWindow(); }

    private:
        DECL_LINK( OnEntrySelected, weld::ComboBox&, void );
    };


    //= DropDownEditControl

    enum MultiLineOperationMode
    {
        eStringList,
        eMultiLineText
    };

    //= OMultilineEditControl
    typedef CommonBehaviourControl<css::inspection::XPropertyControl, weld::Container> OMultilineEditControl_Base;
    class OMultilineEditControl : public OMultilineEditControl_Base
    {
    private:
        MultiLineOperationMode m_nOperationMode;
        std::unique_ptr<weld::Entry> m_xEntry;
        std::unique_ptr<weld::MenuButton> m_xButton;
        std::unique_ptr<weld::Widget> m_xPopover;
        std::unique_ptr<weld::TextView> m_xTextView;
        std::unique_ptr<weld::Button> m_xOk;

        void            SetTextValue(const OUString& rText);
        OUString        GetTextValue() const;

        void            SetStringListValue( const StlSyntaxSequence< OUString >& _rStrings );
        StlSyntaxSequence< OUString >
                        GetStringListValue() const;

        DECL_LINK(ButtonHandler, weld::Button&, void);
        DECL_LINK(TextViewModifiedHdl, weld::TextView&, void);

        void CheckEntryTextViewMisMatch();

    public:
        OMultilineEditControl(std::unique_ptr<weld::Container> xWidget, std::unique_ptr<weld::Builder> xBuilder, MultiLineOperationMode eMode, bool bReadOnly);

        // XPropertyControl
        virtual css::uno::Any SAL_CALL getValue() override;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) override;
        virtual css::uno::Type SAL_CALL getValueType() override;
        virtual weld::Widget* getWidget() override { return getTypedControlWindow(); }

        virtual void editChanged() override;

        virtual void SetModifyHandler() override
        {
            m_xEntry->connect_focus_in( LINK( this, CommonBehaviourControlHelper, GetFocusHdl ) );
            m_xEntry->connect_focus_out( LINK( this, CommonBehaviourControlHelper, LoseFocusHdl ) );
            m_xButton->connect_focus_in( LINK( this, CommonBehaviourControlHelper, GetFocusHdl ) );
            m_xButton->connect_focus_out( LINK( this, CommonBehaviourControlHelper, LoseFocusHdl ) );

            m_xEntry->connect_changed( LINK( this, CommonBehaviourControlHelper, EditModifiedHdl ) );
            m_xTextView->connect_changed( LINK( this, OMultilineEditControl, TextViewModifiedHdl ) );
        }

        virtual void SAL_CALL disposing() override
        {
            m_xOk.reset();
            m_xTextView.reset();
            m_xButton.reset();
            m_xEntry.reset();
            OMultilineEditControl_Base::disposing();
        }

    };

} // namespace pcr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
