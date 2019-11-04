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

#include "standardcontrol.hxx"
#include "pcrcommon.hxx"

#include <com/sun/star/beans/IllegalTypeException.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/inspection/PropertyControlType.hpp>
#include <comphelper/string.hxx>
#include <o3tl/float_int_conversion.hxx>
#include <rtl/math.hxx>
#include <sfx2/objsh.hxx>


// ugly dependencies for the OColorControl
#include <svx/svxids.hrc>
#include <svx/drawitem.hxx>
#include <svx/xtable.hxx>

#include <vcl/floatwin.hxx>
#include <svtools/svmedit.hxx>
#include <svtools/colorcfg.hxx>
#include <svl/zforlist.hxx>
#include <unotools/syslocale.hxx>
#include <unotools/datetime.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <vcl/button.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/ptrstyle.hxx>

#include <cstdlib>
#include <limits>
#include <memory>

namespace pcr
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::inspection;

    //= OTimeControl
    OTimeControl::OTimeControl(std::unique_ptr<weld::TimeSpinButton> xWidget, std::unique_ptr<weld::Builder> xBuilder, bool bReadOnly)
        : OTimeControl_Base(PropertyControlType::TimeField, std::move(xBuilder), std::move(xWidget), bReadOnly)
    {
    }

    void SAL_CALL OTimeControl::setValue( const Any& _rValue )
    {
        util::Time aUNOTime;
        if ( !( _rValue >>= aUNOTime ) )
        {
            getTypedControlWindow()->set_text("");
            getTypedControlWindow()->set_value(tools::Time(tools::Time::EMPTY));
        }
        else
        {
            getTypedControlWindow()->set_value(::tools::Time(aUNOTime));
        }
    }

    Any SAL_CALL OTimeControl::getValue()
    {
        Any aPropValue;
        if ( !getTypedControlWindow()->get_text().isEmpty() )
        {
            aPropValue <<= getTypedControlWindow()->get_value().GetUNOTime();
        }
        return aPropValue;
    }

    Type SAL_CALL OTimeControl::getValueType()
    {
        return ::cppu::UnoType<util::Time>::get();
    }

    //= ODateControl
    ODateControl::ODateControl(std::unique_ptr<SvtCalendarBox> xWidget, std::unique_ptr<weld::Builder> xBuilder, bool bReadOnly)
        : ODateControl_Base(PropertyControlType::DateField, std::move(xBuilder), std::move(xWidget), bReadOnly)
    {
    }

    void SAL_CALL ODateControl::setValue( const Any& _rValue )
    {
        util::Date aUNODate;
        if ( !( _rValue >>= aUNODate ) )
        {
            getTypedControlWindow()->set_date(::Date(::Date::EMPTY));
        }
        else
        {
            ::Date aDate( aUNODate.Day, aUNODate.Month, aUNODate.Year );
            getTypedControlWindow()->set_date(aDate);
        }
    }

    Any SAL_CALL ODateControl::getValue()
    {
        Any aPropValue;
        ::Date aDate(getTypedControlWindow()->get_date());
        if (!aDate.IsEmpty())
            aPropValue <<= aDate.GetUNODate();
        return aPropValue;
    }

    Type SAL_CALL ODateControl::getValueType()
    {
        return ::cppu::UnoType<util::Date>::get();
    }

    //= OEditControl
    OEditControl::OEditControl(std::unique_ptr<weld::Entry> xWidget, std::unique_ptr<weld::Builder> xBuilder, bool bPW, bool bReadOnly)
        : OEditControl_Base( bPW ? PropertyControlType::CharacterField : PropertyControlType::TextField, std::move(xBuilder), std::move(xWidget), bReadOnly )
    {
        m_bIsPassword = bPW;

        auto pWidget = getTypedControlWindow();
        pWidget->set_sensitive(true);
        pWidget->set_editable(!bReadOnly);

        if (m_bIsPassword)
           pWidget->set_max_length( 1 );
    }

    void SAL_CALL OEditControl::setValue( const Any& _rValue )
    {
        OUString sText;
        if ( m_bIsPassword )
        {
            sal_Int16 nValue = 0;
            _rValue >>= nValue;
            if ( nValue )
            {
                sText = OUString(static_cast<sal_Unicode>(nValue));
            }
        }
        else
            _rValue >>= sText;

        getTypedControlWindow()->set_text( sText );
    }

    Any SAL_CALL OEditControl::getValue()
    {
        Any aPropValue;

        OUString sText( getTypedControlWindow()->get_text() );
        if ( m_bIsPassword )
        {
            if ( !sText.isEmpty() )
                aPropValue <<= static_cast<sal_Int16>(sText[0]);
        }
        else
            aPropValue <<= sText;

        return aPropValue;
    }

    Type SAL_CALL OEditControl::getValueType()
    {
        return m_bIsPassword ? ::cppu::UnoType<sal_Int16>::get() : ::cppu::UnoType<OUString>::get();
    }

    void OEditControl::setModified()
    {
        OEditControl_Base::setModified();

        // for password controls, we fire a commit for every single change
        if ( m_bIsPassword )
            notifyModifiedValue();
    }

    static int ImplCalcLongValue( double nValue, sal_uInt16 nDigits )
    {
        double n = nValue;
        for ( sal_uInt16 d = 0; d < nDigits; ++d )
            n *= 10;

        if ( !o3tl::convertsToAtMost(n, std::numeric_limits<int>::max()) )
            return std::numeric_limits<int>::max();
        return static_cast<int>(n);
    }

    static double ImplCalcDoubleValue( int nValue, sal_uInt16 nDigits )
    {
        double n = nValue;
        for ( sal_uInt16 d = 0; d < nDigits; ++d )
            n /= 10;
        return n;
    }

    // class ODateTimeControl
    ODateTimeControl::ODateTimeControl(std::unique_ptr<weld::Container> xWidget, std::unique_ptr<weld::Builder> xBuilder, bool bReadOnly)
        : ODateTimeControl_Base(PropertyControlType::DateTimeField, std::move(xBuilder), std::move(xWidget), bReadOnly)
        , m_xDate(std::make_unique<SvtCalendarBox>(m_xBuilder->weld_menu_button("datefield")))
        , m_xTime(m_xBuilder->weld_time_spin_button("timefield", TimeFieldFormat::F_SEC))
    {
    }

    void SAL_CALL ODateTimeControl::setValue( const Any& _rValue )
    {
        if ( !_rValue.hasValue() )
        {
            m_xDate->set_date(::Date(::Date::EMPTY));
            m_xTime->set_text("");
            m_xTime->set_value(tools::Time(tools::Time::EMPTY));
        }
        else
        {
            util::DateTime aUNODateTime;
            OSL_VERIFY( _rValue >>= aUNODateTime );

            ::DateTime aDateTime( ::DateTime::EMPTY );
            ::utl::typeConvert( aUNODateTime, aDateTime );

            m_xDate->set_date(aDateTime);
            m_xTime->set_value(aDateTime);
        }
    }

    Any SAL_CALL ODateTimeControl::getValue()
    {
        Any aPropValue;
        if (!m_xTime->get_text().isEmpty())
        {
            ::DateTime aDateTime(m_xDate->get_date(), m_xTime->get_value());

            util::DateTime aUNODateTime;
            ::utl::typeConvert( aDateTime, aUNODateTime );

            aPropValue <<= aUNODateTime;
        }
        return aPropValue;
    }

    Type SAL_CALL ODateTimeControl::getValueType()
    {
        return ::cppu::UnoType<util::DateTime>::get();
    }

    //= OHyperlinkControl
    OHyperlinkControl::OHyperlinkControl(std::unique_ptr<weld::Container> xWidget, std::unique_ptr<weld::Builder> xBuilder, bool bReadOnly)
        : OHyperlinkControl_Base(PropertyControlType::HyperlinkField, std::move(xBuilder), std::move(xWidget), bReadOnly)
        , m_xEntry(m_xBuilder->weld_entry("entry"))
        , m_xButton(m_xBuilder->weld_button("button"))
        , m_aActionListeners(m_aMutex)
    {
        auto pWidget = getTypedControlWindow();
        pWidget->set_sensitive(true);
        m_xEntry->set_editable(!bReadOnly);

        m_xButton->connect_clicked(LINK(this, OHyperlinkControl, OnHyperlinkClicked));
    }

    Any SAL_CALL OHyperlinkControl::getValue()
    {
        OUString sText = m_xEntry->get_text();
        return makeAny( sText );
    }

    void SAL_CALL OHyperlinkControl::setValue( const Any& _value )
    {
        OUString sText;
        _value >>= sText;
        m_xEntry->set_text( sText );
    }

    Type SAL_CALL OHyperlinkControl::getValueType()
    {
        return ::cppu::UnoType<OUString>::get();
    }

    void SAL_CALL OHyperlinkControl::addActionListener( const Reference< XActionListener >& listener )
    {
        if ( listener.is() )
            m_aActionListeners.addInterface( listener );
    }

    void SAL_CALL OHyperlinkControl::removeActionListener( const Reference< XActionListener >& listener )
    {
        m_aActionListeners.removeInterface( listener );
    }

    void SAL_CALL OHyperlinkControl::disposing()
    {
        m_xButton.reset();
        m_xEntry.reset();
        OHyperlinkControl_Base::disposing();

        EventObject aEvent( *this );
        m_aActionListeners.disposeAndClear( aEvent );
    }

    IMPL_LINK_NOARG( OHyperlinkControl, OnHyperlinkClicked, weld::Button&, void )
    {
        ActionEvent aEvent( *this, "clicked" );
        m_aActionListeners.forEach< XActionListener >(
            [&aEvent] (uno::Reference<awt::XActionListener> const& xListener)
                { return xListener->actionPerformed(aEvent); });
    }

    //= ONumericControl
    ONumericControl::ONumericControl(std::unique_ptr<weld::MetricSpinButton> xWidget, std::unique_ptr<weld::Builder> xBuilder, bool bReadOnly)
        : ONumericControl_Base(PropertyControlType::NumericField, std::move(xBuilder), std::move(xWidget), bReadOnly)
        , m_eValueUnit( FieldUnit::NONE )
        , m_nFieldToUNOValueFactor( 1 )
    {
        MetricField::SetDefaultUnit( FieldUnit::NONE );

//TODO        getTypedControlWindow()->EnableEmptyFieldValue( true );
//TODO        getTypedControlWindow()->SetStrictFormat( true );
        Optional< double > value( getMaxValue() );
        value.Value = -value.Value;
        setMinValue( value );
    }

    ::sal_Int16 SAL_CALL ONumericControl::getDecimalDigits()
    {
        return getTypedControlWindow()->get_digits();
    }

    void SAL_CALL ONumericControl::setDecimalDigits( ::sal_Int16 _decimaldigits )
    {
        getTypedControlWindow()->set_digits( _decimaldigits );
    }

    Optional< double > SAL_CALL ONumericControl::getMinValue()
    {
        Optional< double > aReturn( true, 0 );

        int minValue = getTypedControlWindow()->get_min(FieldUnit::NONE);
        if ( minValue == std::numeric_limits<int>::min() )
            aReturn.IsPresent = false;
        else
            aReturn.Value = static_cast<double>(minValue);

        return aReturn;
    }

    void SAL_CALL ONumericControl::setMinValue( const Optional< double >& _minvalue )
    {
        if ( !_minvalue.IsPresent )
            getTypedControlWindow()->set_min( std::numeric_limits<int>::min(), FieldUnit::NONE );
        else
            getTypedControlWindow()->set_min( impl_apiValueToFieldValue_nothrow( _minvalue.Value ) , m_eValueUnit);
    }

    Optional< double > SAL_CALL ONumericControl::getMaxValue()
    {
        Optional< double > aReturn( true, 0 );

        int maxValue = getTypedControlWindow()->get_max(FieldUnit::NONE);
        if ( maxValue == std::numeric_limits<int>::max() )
            aReturn.IsPresent = false;
        else
            aReturn.Value = static_cast<double>(maxValue);

        return aReturn;
    }

    void SAL_CALL ONumericControl::setMaxValue( const Optional< double >& _maxvalue )
    {
        if ( !_maxvalue.IsPresent )
            getTypedControlWindow()->set_max( std::numeric_limits<int>::max(), FieldUnit::NONE );
        else
            getTypedControlWindow()->set_max( impl_apiValueToFieldValue_nothrow( _maxvalue.Value ), m_eValueUnit );
    }

    ::sal_Int16 SAL_CALL ONumericControl::getDisplayUnit()
    {
        return VCLUnoHelper::ConvertToMeasurementUnit( getTypedControlWindow()->get_unit(), 1 );
    }

    void SAL_CALL ONumericControl::setDisplayUnit( ::sal_Int16 _displayunit )
    {
        if ( ( _displayunit < MeasureUnit::MM_100TH ) || ( _displayunit > MeasureUnit::PERCENT ) )
            throw IllegalArgumentException();
        if  (   ( _displayunit == MeasureUnit::MM_100TH )
            ||  ( _displayunit == MeasureUnit::MM_10TH )
            ||  ( _displayunit == MeasureUnit::INCH_1000TH )
            ||  ( _displayunit == MeasureUnit::INCH_100TH )
            ||  ( _displayunit == MeasureUnit::INCH_10TH )
            ||  ( _displayunit == MeasureUnit::PERCENT )
            )
            throw IllegalArgumentException();

        sal_Int16 nDummyFactor = 1;
        FieldUnit eFieldUnit = VCLUnoHelper::ConvertToFieldUnit( _displayunit, nDummyFactor );
        if ( nDummyFactor != 1 )
            // everything which survived the checks above should result in a factor of 1, i.e.,
            // it should have a direct counterpart as FieldUnit
            throw RuntimeException();
        getTypedControlWindow()->set_unit(eFieldUnit);
    }

    ::sal_Int16 SAL_CALL ONumericControl::getValueUnit()
    {
        return VCLUnoHelper::ConvertToMeasurementUnit( m_eValueUnit, m_nFieldToUNOValueFactor );
    }

    void SAL_CALL ONumericControl::setValueUnit( ::sal_Int16 _valueunit )
    {
        if ( ( _valueunit < MeasureUnit::MM_100TH ) || ( _valueunit > MeasureUnit::PERCENT ) )
            throw IllegalArgumentException();
        m_eValueUnit = VCLUnoHelper::ConvertToFieldUnit( _valueunit, m_nFieldToUNOValueFactor );
    }

    void SAL_CALL ONumericControl::setValue( const Any& _rValue )
    {
        if ( !_rValue.hasValue() )
        {
            getTypedControlWindow()->set_text( "" );
//TODO            getTypedControlWindow()->SetEmptyFieldValue();
        }
        else
        {
            double nValue( 0 );
            OSL_VERIFY( _rValue >>= nValue );
            auto nControlValue = impl_apiValueToFieldValue_nothrow( nValue );
            getTypedControlWindow()->set_value( nControlValue, m_eValueUnit );
        }
    }

    int ONumericControl::impl_apiValueToFieldValue_nothrow( double _nApiValue ) const
    {
        int nControlValue = ImplCalcLongValue( _nApiValue, getTypedControlWindow()->get_digits() );
        nControlValue /= m_nFieldToUNOValueFactor;
        return nControlValue;
    }

    double ONumericControl::impl_fieldValueToApiValue_nothrow( int nFieldValue ) const
    {
        double nApiValue = ImplCalcDoubleValue( nFieldValue, getTypedControlWindow()->get_digits() );
        nApiValue *= m_nFieldToUNOValueFactor;
        return nApiValue;
    }

    Any SAL_CALL ONumericControl::getValue()
    {
        Any aPropValue;
        if ( !getTypedControlWindow()->get_text().isEmpty() )
        {
            double nValue = impl_fieldValueToApiValue_nothrow( getTypedControlWindow()->get_value( m_eValueUnit ) );
            aPropValue <<= nValue;
        }
        return aPropValue;
    }

    Type SAL_CALL ONumericControl::getValueType()
    {
        return ::cppu::UnoType<double>::get();
    }

    //= OColorControl
    OColorControl::OColorControl(std::unique_ptr<ColorListBox> xWidget, std::unique_ptr<weld::Builder> xBuilder, bool bReadOnly)
        : OColorControl_Base(PropertyControlType::ColorListBox, std::move(xBuilder), std::move(xWidget), bReadOnly)
    {
        getTypedControlWindow()->SetSlotId(SID_FM_CTL_PROPERTIES);
    }

    void SAL_CALL OColorControl::setValue( const Any& _rValue )
    {
        css::util::Color nColor = sal_uInt32(COL_TRANSPARENT);
        if (_rValue.hasValue())
            _rValue >>= nColor;
        getTypedControlWindow()->SelectEntry(nColor);
    }

    Any SAL_CALL OColorControl::getValue()
    {
        Any aPropValue;
        ::Color aRgbCol = getTypedControlWindow()->GetSelectEntryColor();
        if (aRgbCol == COL_TRANSPARENT)
            return aPropValue;
        aPropValue <<= aRgbCol;
        return aPropValue;
    }

    Type SAL_CALL OColorControl::getValueType()
    {
        return ::cppu::UnoType<sal_Int32>::get();
    }

    void OColorControl::setModified()
    {
        OColorControl_Base::setModified();

        // fire a commit
        notifyModifiedValue();
    }

    //= OListboxControl
    OListboxControl::OListboxControl(std::unique_ptr<weld::ComboBox> xWidget, std::unique_ptr<weld::Builder> xBuilder, bool bReadOnly)
        : OListboxControl_Base(PropertyControlType::ListBox, std::move(xBuilder), std::move(xWidget), bReadOnly)
    {
    }

    Any SAL_CALL OListboxControl::getValue()
    {
        OUString sControlValue( getTypedControlWindow()->get_active_text() );

        Any aPropValue;
        if ( !sControlValue.isEmpty() )
            aPropValue <<= sControlValue;
        return aPropValue;
    }

    Type SAL_CALL OListboxControl::getValueType()
    {
        return ::cppu::UnoType<OUString>::get();
    }

    void SAL_CALL OListboxControl::setValue( const Any& _rValue )
    {
        if ( !_rValue.hasValue() )
            getTypedControlWindow()->set_active(-1);
        else
        {
            OUString sSelection;
            _rValue >>= sSelection;

            if (getTypedControlWindow()->find_text(sSelection) == -1)
                getTypedControlWindow()->insert_text(0, sSelection);

            if (sSelection != getTypedControlWindow()->get_active_text())
                getTypedControlWindow()->set_active_text(sSelection);
        }
    }

    void SAL_CALL OListboxControl::clearList()
    {
        getTypedControlWindow()->clear();
    }

    void SAL_CALL OListboxControl::prependListEntry( const OUString& NewEntry )
    {
        getTypedControlWindow()->insert_text(0, NewEntry);
    }

    void SAL_CALL OListboxControl::appendListEntry( const OUString& NewEntry )
    {
        getTypedControlWindow()->append_text(NewEntry);
    }

    Sequence< OUString > SAL_CALL OListboxControl::getListEntries()
    {
        const sal_Int32 nCount = getTypedControlWindow()->get_count();
        Sequence< OUString > aRet(nCount);
        OUString* pIter = aRet.getArray();
        for (sal_Int32 i = 0; i < nCount ; ++i,++pIter)
            *pIter = getTypedControlWindow()->get_text(i);

        return aRet;
    }

    void OListboxControl::setModified()
    {
        OListboxControl_Base::setModified();

        //TODO if ( !getTypedControlWindow()->IsTravelSelect() )
            // fire a commit
            notifyModifiedValue();
    }

    //= OComboboxControl
    OComboboxControl::OComboboxControl(std::unique_ptr<weld::ComboBox> xWidget, std::unique_ptr<weld::Builder> xBuilder, bool bReadOnly)
        : OComboboxControl_Base(PropertyControlType::ComboBox, std::move(xBuilder), std::move(xWidget), bReadOnly)
    {
        getTypedControlWindow()->connect_changed( LINK( this, OComboboxControl, OnEntrySelected ) );
    }

    void SAL_CALL OComboboxControl::setValue( const Any& _rValue )
    {
        OUString sText;
        _rValue >>= sText;
        getTypedControlWindow()->set_entry_text( sText );
    }

    Any SAL_CALL OComboboxControl::getValue()
    {
        return makeAny( getTypedControlWindow()->get_active_text() );
    }

    Type SAL_CALL OComboboxControl::getValueType()
    {
        return ::cppu::UnoType<OUString>::get();
    }

    void SAL_CALL OComboboxControl::clearList()
    {
        getTypedControlWindow()->clear();
    }

    void SAL_CALL OComboboxControl::prependListEntry( const OUString& NewEntry )
    {
        getTypedControlWindow()->insert_text(0, NewEntry);
    }

    void SAL_CALL OComboboxControl::appendListEntry( const OUString& NewEntry )
    {
        getTypedControlWindow()->append_text( NewEntry );
    }

    Sequence< OUString > SAL_CALL OComboboxControl::getListEntries(  )
    {
        const sal_Int32 nCount = getTypedControlWindow()->get_count();
        Sequence< OUString > aRet(nCount);
        OUString* pIter = aRet.getArray();
        for (sal_Int32 i = 0; i < nCount ; ++i,++pIter)
            *pIter = getTypedControlWindow()->get_text(i);

        return aRet;
    }

    IMPL_LINK_NOARG( OComboboxControl, OnEntrySelected, weld::ComboBox&, void )
    {
//TODO        if ( !getTypedControlWindow()->IsTravelSelect() )
            // fire a commit
            notifyModifiedValue();
    }

#if 0

    //= OMultilineFloatingEdit
    class OMultilineFloatingEdit : public FloatingWindow
    {
    private:
        VclPtr<MultiLineEdit>   m_aImplEdit;

    protected:
        virtual void    Resize() override;

    public:
        explicit        OMultilineFloatingEdit(vcl::Window* _pParen);
        virtual         ~OMultilineFloatingEdit() override;
        virtual void    dispose() override;
        MultiLineEdit&  getEdit() { return *m_aImplEdit.get(); }

    protected:
        virtual bool    PreNotify(NotifyEvent& _rNEvt) override;
    };


    OMultilineFloatingEdit::OMultilineFloatingEdit(vcl::Window* pParent)
        :FloatingWindow(pParent, WB_BORDER)
        ,m_aImplEdit(VclPtr<MultiLineEdit>::Create(this, WB_VSCROLL|WB_IGNORETAB|WB_NOBORDER))
    {
        m_aImplEdit->Show();
    }

    OMultilineFloatingEdit::~OMultilineFloatingEdit()
    {
        disposeOnce();
    }

    void OMultilineFloatingEdit::dispose()
    {
        m_aImplEdit.disposeAndClear();
        FloatingWindow::dispose();
    }

    void OMultilineFloatingEdit::Resize()
    {
        m_aImplEdit->SetSizePixel(GetOutputSizePixel());
    }


    bool OMultilineFloatingEdit::PreNotify(NotifyEvent& _rNEvt)
    {
        bool bResult = true;

        MouseNotifyEvent nSwitch = _rNEvt.GetType();
        if (MouseNotifyEvent::KEYINPUT == nSwitch)
        {
            const vcl::KeyCode& aKeyCode = _rNEvt.GetKeyEvent()->GetKeyCode();
            sal_uInt16 nKey = aKeyCode.GetCode();

            if  (   (   (KEY_RETURN == nKey)
                    && !aKeyCode.IsShift()
                    )
                ||  (   (KEY_UP == nKey)
                    &&  aKeyCode.IsMod2()
                    )
                )
            {
                EndPopupMode();
            }
            else
                bResult=FloatingWindow::PreNotify(_rNEvt);
        }
        else
            bResult=FloatingWindow::PreNotify(_rNEvt);

        return bResult;
    }

    //= DropDownEditControl_Base
    DropDownEditControl::DropDownEditControl( vcl::Window* pParent, WinBits _nStyle )
        :Edit( pParent, _nStyle )
        ,m_pFloatingEdit( nullptr )
        ,m_pDropdownButton( nullptr )
        ,m_nOperationMode( eStringList )
        ,m_bDropdown(false)
        ,m_pHelper(nullptr)
    {
        SetCompoundControl( true );

        m_pImplEdit = VclPtr<MultiLineEdit>::Create( this, WB_TABSTOP | WB_IGNORETAB | WB_NOBORDER | (_nStyle & WB_READONLY) );
        SetSubEdit( m_pImplEdit );
        m_pImplEdit->Show();

        if ( _nStyle & WB_DROPDOWN )
        {
            m_pDropdownButton = VclPtr<PushButton>::Create( this, WB_NOLIGHTBORDER | WB_RECTSTYLE | WB_NOTABSTOP);
            m_pDropdownButton->SetSymbol(SymbolType::SPIN_DOWN);
            m_pDropdownButton->SetClickHdl( LINK( this, DropDownEditControl, DropDownHdl ) );
            m_pDropdownButton->Show();
        }

        m_pFloatingEdit = VclPtr<OMultilineFloatingEdit>::Create(this);

        m_pFloatingEdit->SetPopupModeEndHdl( LINK( this, DropDownEditControl, ReturnHdl ) );
        m_pFloatingEdit->getEdit().SetReadOnly( ( _nStyle & WB_READONLY ) != 0 );
    }

    void DropDownEditControl::setControlHelper( CommonBehaviourControlHelper& _rControlHelper )
    {
        m_pHelper = &_rControlHelper;
        m_pFloatingEdit->getEdit().SetModifyHdl( LINK( &_rControlHelper, CommonBehaviourControlHelper, EditModifiedHdl ) );
        m_pImplEdit->SetGetFocusHdl( LINK( &_rControlHelper, CommonBehaviourControlHelper, GetFocusHdl ) );
        m_pImplEdit->SetModifyHdl( LINK( &_rControlHelper, CommonBehaviourControlHelper, EditModifiedHdl ) );
        m_pImplEdit->SetLoseFocusHdl( LINK( &_rControlHelper, CommonBehaviourControlHelper, LoseFocusHdl ) );
    }

    DropDownEditControl::~DropDownEditControl()
    {
        disposeOnce();
    }

    void DropDownEditControl::dispose()
    {
        SetSubEdit(nullptr);
        m_pImplEdit.disposeAndClear();
        m_pFloatingEdit.disposeAndClear();
        m_pDropdownButton.disposeAndClear();
        Edit::dispose();
    }


    void DropDownEditControl::Resize()
    {
        ::Size aOutSz = GetOutputSizePixel();

        if (m_pDropdownButton!=nullptr)
        {
            long nSBWidth = GetSettings().GetStyleSettings().GetScrollBarSize();
            nSBWidth = CalcZoom( nSBWidth );
            m_pImplEdit->setPosSizePixel( 0, 1, aOutSz.Width() - nSBWidth, aOutSz.Height()-2 );
            m_pDropdownButton->setPosSizePixel( aOutSz.Width() - nSBWidth, 0, nSBWidth, aOutSz.Height() );
        }
        else
            m_pImplEdit->setPosSizePixel( 0, 1, aOutSz.Width(), aOutSz.Height()-2 );
    }


    bool DropDownEditControl::PreNotify( NotifyEvent& rNEvt )
    {
        bool bResult = true;

        if (rNEvt.GetType() == MouseNotifyEvent::KEYINPUT)
        {
            const vcl::KeyCode& aKeyCode = rNEvt.GetKeyEvent()->GetKeyCode();
            sal_uInt16 nKey = aKeyCode.GetCode();

            if ( nKey == KEY_RETURN && !aKeyCode.IsShift() )
            {
                if ( m_pHelper )
                {
                    m_pHelper->LoseFocusHdl( *m_pImplEdit.get() );
                    m_pHelper->activateNextControl();
                }
            }
            else if ( nKey == KEY_DOWN && aKeyCode.IsMod2() )
            {
                Invalidate();
                ShowDropDown( true );
            }
            else if (   KEYGROUP_CURSOR == aKeyCode.GetGroup()
                    ||  nKey == KEY_HELP
                    ||  KEYGROUP_FKEYS == aKeyCode.GetGroup()
                    ||  m_nOperationMode == eMultiLineText
                    )
            {
                bResult = Edit::PreNotify( rNEvt );
            }
            else if ( m_nOperationMode == eStringList )
            {
                Selection aSel = m_pImplEdit->GetSelection();
                if ( aSel.Min() != aSel.Max() )
                {
                    aSel.Min() = FindPos( aSel.Min() );
                    aSel.Max() = FindPos( aSel.Max() );
                }
                else
                {
                    aSel.Min() = FindPos( aSel.Min() );
                    aSel.Max() = aSel.Min();
                }
                Invalidate();
                ShowDropDown( true );
                m_pFloatingEdit->getEdit().GrabFocus();
                m_pFloatingEdit->getEdit().SetSelection( aSel );
                vcl::Window* pFocusWin = Application::GetFocusWindow();
                pFocusWin->KeyInput( *rNEvt.GetKeyEvent() );
            }
        }
        else
            bResult = Edit::PreNotify(rNEvt);

        return bResult;
    }

#endif

    namespace
    {

        StlSyntaxSequence< OUString > lcl_convertMultiLineToList( const OUString& _rCompsedTextWithLineBreaks )
        {
            sal_Int32 nLines = comphelper::string::getTokenCount(_rCompsedTextWithLineBreaks, '\n');
            StlSyntaxSequence< OUString > aStrings( nLines );
            if (nLines)
            {
                StlSyntaxSequence< OUString >::iterator stringItem = aStrings.begin();
                sal_Int32 nIdx {0};
                do
                {
                    *stringItem = _rCompsedTextWithLineBreaks.getToken( 0, '\n', nIdx );
                    ++stringItem;
                }
                while (nIdx>0);
            }
            return aStrings;
        }

        OUString lcl_convertListToMultiLine( const StlSyntaxSequence< OUString >& _rStrings )
        {
            OUStringBuffer sMultiLineText;
            for (   StlSyntaxSequence< OUString >::const_iterator item = _rStrings.begin();
                    item != _rStrings.end();
                )
            {
                sMultiLineText.append(*item);
                if ( ++item != _rStrings.end() )
                    sMultiLineText.append("\n");
            }
            return sMultiLineText.makeStringAndClear();
        }


        OUString lcl_convertListToDisplayText( const StlSyntaxSequence< OUString >& _rStrings )
        {
            OUStringBuffer aComposed;
            for (   StlSyntaxSequence< OUString >::const_iterator strings = _rStrings.begin();
                    strings != _rStrings.end();
                    ++strings
                )
            {
                if ( strings != _rStrings.begin() )
                    aComposed.append( ';' );
                aComposed.append( '\"' );
                aComposed.append( *strings );
                aComposed.append( '\"' );
            }
            return aComposed.makeStringAndClear();
        }
    }

#if 0

    #define STD_HEIGHT  100
    void DropDownEditControl::ShowDropDown( bool bShow )
    {
        if (bShow)
        {
            ::Point aMePos= GetPosPixel();
            aMePos = GetParent()->OutputToScreenPixel( aMePos );
            ::Size aSize=GetSizePixel();
            ::tools::Rectangle aRect(aMePos,aSize);
            aSize.setHeight( STD_HEIGHT );
            m_pFloatingEdit->SetOutputSizePixel(aSize);
            m_pFloatingEdit->StartPopupMode( aRect, FloatWinPopupFlags::Down );

            m_pFloatingEdit->Show();
            m_pFloatingEdit->getEdit().GrabFocus();
            m_pFloatingEdit->getEdit().SetSelection(Selection(m_pFloatingEdit->getEdit().GetText().getLength()));
            m_bDropdown = true;
            if ( m_nOperationMode == eMultiLineText )
                m_pFloatingEdit->getEdit().SetText( m_pImplEdit->GetText() );
            m_pImplEdit->SetText("");
        }
        else
        {
            m_pFloatingEdit->Hide();
            m_pFloatingEdit->Invalidate();
            m_pFloatingEdit->Update();

            // transfer the text from the floating edit to our own edit
            OUString sDisplayText( m_pFloatingEdit->getEdit().GetText() );
            if ( m_nOperationMode == eStringList )
                sDisplayText = lcl_convertListToDisplayText( lcl_convertMultiLineToList( sDisplayText ) );

            m_pImplEdit->SetText( sDisplayText );
            GetParent()->Invalidate( InvalidateFlags::Children );
            m_bDropdown = false;
            m_pImplEdit->GrabFocus();
        }
    }


    long DropDownEditControl::FindPos(long nSinglePos)
    {
        long nPos = 0;
        OUString aOutput;
        OUString aStr = m_pFloatingEdit->getEdit().GetText();
        OUString aStr1 = GetText();

        if ((nSinglePos == 0) || (nSinglePos == aStr1.getLength()))
        {
            return nSinglePos;
        }

        if (!aStr.isEmpty())
        {
            long nDiff=0;

            sal_Int32 nIdx {0};
            OUString aInput = aStr.getToken(0, '\n', nIdx );

            if (!aInput.isEmpty())
            {
                aOutput += "\"" + aInput + "\"";
                nDiff++;
            }

            if (nSinglePos <= aOutput.getLength())
            {
                nPos=nSinglePos-nDiff;
            }
            else
            {
                while (nIdx>0)
                {
                    aInput=aStr.getToken(0, '\n', nIdx);
                    if (!aInput.isEmpty())
                    {
                        aOutput += ";\"" + aInput + "\"";
                        nDiff += 2;

                        if (nSinglePos <= aOutput.getLength())
                        {
                            nPos=nSinglePos-nDiff;
                            break;
                        }
                    }
                }
            }
        }
        return nPos;
    }


    IMPL_LINK_NOARG( DropDownEditControl, ReturnHdl, FloatingWindow*, void)
    {
        OUString aStr = m_pFloatingEdit->getEdit().GetText();
        OUString aStr2 = GetText();
        ShowDropDown(false);

        if (aStr!=aStr2 || ( m_nOperationMode == eStringList ) )
        {
            if ( m_pHelper )
                m_pHelper->notifyModifiedValue();
        }
    }


    IMPL_LINK_NOARG( DropDownEditControl, DropDownHdl, Button*, void )
    {
        ShowDropDown(!m_bDropdown);
    }

#endif

    void OMultilineEditControl::SetStringListValue(const StlSyntaxSequence<OUString>& rStrings)
    {
        m_xEntry->set_text(lcl_convertListToDisplayText(rStrings));
        m_xTextView->set_text(lcl_convertListToMultiLine(rStrings));
    }

    StlSyntaxSequence<OUString> OMultilineEditControl::GetStringListValue() const
    {
        return lcl_convertMultiLineToList(m_xTextView->get_text());
    }

    void OMultilineEditControl::SetTextValue(const OUString& rText)
    {
        OSL_PRECOND( m_nOperationMode == eMultiLineText, "OMultilineEditControl::SetTextValue: illegal call!" );

        m_xTextView->set_text(rText);
        m_xEntry->set_text(rText);
    }

    OUString OMultilineEditControl::GetTextValue() const
    {
        OSL_PRECOND( m_nOperationMode == eMultiLineText, "OMultilineEditControl::GetTextValue: illegal call!" );
        return m_xEntry->get_text();
    }

    //= OMultilineEditControl
    OMultilineEditControl::OMultilineEditControl(std::unique_ptr<weld::Container> xWidget, std::unique_ptr<weld::Builder> xBuilder, MultiLineOperationMode eMode, bool bReadOnly)
        : OMultilineEditControl_Base(eMode == eMultiLineText ? PropertyControlType::MultiLineTextField : PropertyControlType::StringListField,
                                    std::move(xBuilder), std::move(xWidget), bReadOnly)
        , m_nOperationMode(eMode)
        , m_xEntry(m_xBuilder->weld_entry("entry"))
        , m_xButton(m_xBuilder->weld_menu_button("button"))
        , m_xTextView(m_xBuilder->weld_text_view("textview"))
        , m_xOk(m_xBuilder->weld_button("ok"))
    {
        m_xTextView->set_size_request(m_xTextView->get_approximate_digit_width() * 30, m_xTextView->get_height_rows(8));
        m_xOk->connect_clicked(LINK(this, OMultilineEditControl, ButtonHandler));
#if 0
        getTypedControlWindow()->setOperationMode(eMode);
        getTypedControlWindow()->setControlHelper(*this);
#endif
    }

    IMPL_LINK_NOARG(OMultilineEditControl, TextViewModifiedHdl, weld::TextView&, void)
    {
        m_xEntry->set_text(m_xTextView->get_text());
        setModified();
    }

    IMPL_LINK_NOARG(OMultilineEditControl, ButtonHandler, weld::Button&, void)
    {
        m_xButton->set_active(false);
    }

    void SAL_CALL OMultilineEditControl::setValue( const Any& _rValue )
    {
        impl_checkDisposed_throw();

        switch (m_nOperationMode)
        {
            case eMultiLineText:
            {
                OUString sText;
                if ( !( _rValue >>= sText ) && _rValue.hasValue() )
                    throw IllegalTypeException();
                SetTextValue(sText);
                break;
            }
            case eStringList:
            {
                Sequence< OUString > aStringLines;
                if ( !( _rValue >>= aStringLines ) && _rValue.hasValue() )
                    throw IllegalTypeException();
                SetStringListValue( StlSyntaxSequence<OUString>(aStringLines) );
                break;
            }
        }
    }

    Any SAL_CALL OMultilineEditControl::getValue()
    {
        impl_checkDisposed_throw();

        Any aValue;
        switch (m_nOperationMode)
        {
            case eMultiLineText:
                aValue <<= GetTextValue();
                break;
            case eStringList:
                aValue <<= GetStringListValue();
                break;
        }
        return aValue;
    }

    Type SAL_CALL OMultilineEditControl::getValueType()
    {
        if (m_nOperationMode == eMultiLineText)
            return ::cppu::UnoType<OUString>::get();
        return cppu::UnoType<Sequence< OUString >>::get();
    }

} // namespace pcr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
