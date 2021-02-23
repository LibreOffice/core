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
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/inspection/PropertyControlType.hpp>
#include <comphelper/string.hxx>
#include <o3tl/float_int_conversion.hxx>
#include <toolkit/helper/vclunohelper.hxx>


// ugly dependencies for the OColorControl
#include <svx/svxids.hrc>

#include <tools/datetime.hxx>
#include <unotools/datetime.hxx>

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
    OTimeControl::OTimeControl(std::unique_ptr<weld::FormattedSpinButton> xWidget, std::unique_ptr<weld::Builder> xBuilder, bool bReadOnly)
        : OTimeControl_Base(PropertyControlType::TimeField, std::move(xBuilder), std::move(xWidget), bReadOnly)
        , m_xFormatter(new weld::TimeFormatter(*getTypedControlWindow()))
    {
        m_xFormatter->SetExtFormat(ExtTimeFieldFormat::LongDuration);
    }

    void SAL_CALL OTimeControl::setValue( const Any& _rValue )
    {
        util::Time aUNOTime;
        if ( !( _rValue >>= aUNOTime ) )
        {
            getTypedControlWindow()->set_text("");
            m_xFormatter->SetTime(tools::Time(tools::Time::EMPTY));
        }
        else
        {
            m_xFormatter->SetTime(::tools::Time(aUNOTime));
        }
    }

    Any SAL_CALL OTimeControl::getValue()
    {
        Any aPropValue;
        if ( !getTypedControlWindow()->get_text().isEmpty() )
        {
            aPropValue <<= m_xFormatter->GetTime().GetUNOTime();
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
            getTypedControlWindow()->set_date(::Date(::Date::SYSTEM));
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

    ODateTimeControl::ODateTimeControl(std::unique_ptr<weld::Container> xWidget, std::unique_ptr<weld::Builder> xBuilder, bool bReadOnly)
        : ODateTimeControl_Base(PropertyControlType::DateTimeField, std::move(xBuilder), std::move(xWidget), bReadOnly)
        , m_xDate(std::make_unique<SvtCalendarBox>(m_xBuilder->weld_menu_button("datefield")))
        , m_xTime(m_xBuilder->weld_formatted_spin_button("timefield"))
        , m_xFormatter(new weld::TimeFormatter(*m_xTime))
    {
        m_xFormatter->SetExtFormat(ExtTimeFieldFormat::LongDuration);
    }

    void SAL_CALL ODateTimeControl::setValue( const Any& _rValue )
    {
        if ( !_rValue.hasValue() )
        {
            m_xDate->set_date(::Date(::Date::SYSTEM));
            m_xTime->set_text("");
            m_xFormatter->SetTime(tools::Time(tools::Time::EMPTY));
        }
        else
        {
            util::DateTime aUNODateTime;
            OSL_VERIFY( _rValue >>= aUNODateTime );

            ::DateTime aDateTime( ::DateTime::EMPTY );
            ::utl::typeConvert( aUNODateTime, aDateTime );

            m_xDate->set_date(aDateTime);
            m_xFormatter->SetTime(aDateTime);
        }
    }

    Any SAL_CALL ODateTimeControl::getValue()
    {
        Any aPropValue;
        if (!m_xTime->get_text().isEmpty())
        {
            ::DateTime aDateTime(m_xDate->get_date(), m_xFormatter->GetTime());

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
        Optional< double > value( getMaxValue() );
        value.Value = -value.Value;
        setMinValue( value );
    }

    ::sal_Int16 SAL_CALL ONumericControl::getDecimalDigits()
    {
        return getTypedControlWindow()->get_digits();
    }

    void SAL_CALL ONumericControl::setDecimalDigits( ::sal_Int16 decimaldigits )
    {
        weld::MetricSpinButton* pControlWindow = getTypedControlWindow();
        int min, max;
        pControlWindow->get_range(min, max, FieldUnit::NONE);
        pControlWindow->set_digits(decimaldigits);
        pControlWindow->set_range(min, max, FieldUnit::NONE);
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
        getTypedControlWindow()->SelectEntry(::Color(ColorTransparency, nColor));
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
        weld::ComboBox* pControlWindow = getTypedControlWindow();
        // tdf#138701 leave current cursor valid if the contents won't change
        if (pControlWindow->get_active_text() != sText)
            pControlWindow->set_entry_text(sText);
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
        // fire a commit
        notifyModifiedValue();
    }

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

    void OMultilineEditControl::CheckEntryTextViewMisMatch()
    {
        // if there are newlines or something else which the entry cannot show, then make
        // just the multiline dropdown editable as the canonical source for text
        m_xEntry->set_sensitive(m_xEntry->get_text() == m_xTextView->get_text());
    }

    void OMultilineEditControl::SetStringListValue(const StlSyntaxSequence<OUString>& rStrings)
    {
        m_xEntry->set_text(lcl_convertListToDisplayText(rStrings));
        m_xTextView->set_text(lcl_convertListToMultiLine(rStrings));
        CheckEntryTextViewMisMatch();
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
        CheckEntryTextViewMisMatch();
    }

    OUString OMultilineEditControl::GetTextValue() const
    {
        OSL_PRECOND( m_nOperationMode == eMultiLineText, "OMultilineEditControl::GetTextValue: illegal call!" );
        return m_xTextView->get_text();
    }

    //= OMultilineEditControl
    OMultilineEditControl::OMultilineEditControl(std::unique_ptr<weld::Container> xWidget, std::unique_ptr<weld::Builder> xBuilder, MultiLineOperationMode eMode, bool bReadOnly)
        : OMultilineEditControl_Base(eMode == eMultiLineText ? PropertyControlType::MultiLineTextField : PropertyControlType::StringListField,
                                    std::move(xBuilder), std::move(xWidget), bReadOnly)
        , m_nOperationMode(eMode)
        , m_xEntry(m_xBuilder->weld_entry("entry"))
        , m_xButton(m_xBuilder->weld_menu_button("button"))
        , m_xPopover(m_xBuilder->weld_widget("popover"))
        , m_xTextView(m_xBuilder->weld_text_view("textview"))
        , m_xOk(m_xBuilder->weld_button("ok"))
    {
        m_xButton->set_popover(m_xPopover.get());
        m_xTextView->set_size_request(m_xTextView->get_approximate_digit_width() * 30, m_xTextView->get_height_rows(8));
        m_xOk->connect_clicked(LINK(this, OMultilineEditControl, ButtonHandler));
    }

    IMPL_LINK_NOARG(OMultilineEditControl, TextViewModifiedHdl, weld::TextView&, void)
    {
        // tdf#139070 during editing update the entry to look like how it will
        // look once editing is finished so that the default behaviour of vcl
        // to strip newlines and the default behaviour of gtk to show a newline
        // symbol is suppressed
        OUString sText = m_xTextView->get_text();
        auto aSeq = lcl_convertMultiLineToList(sText);
        if (aSeq.getLength() > 1)
            m_xEntry->set_text(lcl_convertListToDisplayText(aSeq));
        else
            m_xEntry->set_text(sText);
        CheckEntryTextViewMisMatch();
        setModified();
    }

    void OMultilineEditControl::editChanged()
    {
        m_xTextView->set_text(m_xEntry->get_text());
        CheckEntryTextViewMisMatch();
        setModified();
    }

    IMPL_LINK_NOARG(OMultilineEditControl, ButtonHandler, weld::Button&, void)
    {
        m_xButton->set_active(false);
        notifyModifiedValue();
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
