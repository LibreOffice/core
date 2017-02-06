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
#include <rtl/math.hxx>
#include <sfx2/objsh.hxx>


// ugly dependencies for the OColorControl
#include <svx/svxids.hrc>
#include <svx/drawitem.hxx>
#include <svx/xtable.hxx>

#include <vcl/floatwin.hxx>
#include <svtools/svmedit.hxx>
#include <svtools/colorcfg.hxx>
#include <unotools/syslocale.hxx>
#include <unotools/datetime.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <vcl/button.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

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


    OTimeControl::OTimeControl( vcl::Window* pParent, WinBits nWinStyle )
        :OTimeControl_Base( PropertyControlType::TimeField, pParent, nWinStyle )
    {
        getTypedControlWindow()->SetStrictFormat( true );
        getTypedControlWindow()->SetFormat( TimeFieldFormat::F_SEC );
        getTypedControlWindow()->EnableEmptyFieldValue( true );
    }


    void SAL_CALL OTimeControl::setValue( const Any& _rValue )
    {
        util::Time aUNOTime;
        if ( !( _rValue >>= aUNOTime ) )
        {
            getTypedControlWindow()->SetText( "" );
            getTypedControlWindow()->SetEmptyTime();
        }
        else
        {
            getTypedControlWindow()->SetTime( ::tools::Time(aUNOTime) );
        }
    }


    Any SAL_CALL OTimeControl::getValue()
    {
        Any aPropValue;
        if ( !getTypedControlWindow()->GetText().isEmpty() )
        {
            aPropValue <<= getTypedControlWindow()->GetTime().GetUNOTime();
        }
        return aPropValue;
    }


    Type SAL_CALL OTimeControl::getValueType()
    {
        return ::cppu::UnoType<util::Time>::get();
    }


    //= ODateControl


    ODateControl::ODateControl( vcl::Window* pParent, WinBits nWinStyle )
        :ODateControl_Base( PropertyControlType::DateField, pParent, nWinStyle | WB_DROPDOWN )
    {
        CalendarField* pControlWindow = getTypedControlWindow();
        pControlWindow->SetStrictFormat(true);

        pControlWindow->SetMin( ::Date( 1,1,1600 ) );
        pControlWindow->SetFirst( ::Date( 1,1,1600 ) );
        pControlWindow->SetLast( ::Date( 1, 1, 9999 ) );
        pControlWindow->SetMax( ::Date( 1, 1, 9999 ) );

        pControlWindow->SetExtDateFormat( ExtDateFieldFormat::SystemShortYYYY );
        pControlWindow->EnableEmptyFieldValue( true );
    }


    void SAL_CALL ODateControl::setValue( const Any& _rValue )
    {
        util::Date aUNODate;
        if ( !( _rValue >>= aUNODate ) )
        {
            getTypedControlWindow()->SetText( "" );
            getTypedControlWindow()->SetEmptyDate();
        }
        else
        {
            ::Date aDate( aUNODate.Day, aUNODate.Month, aUNODate.Year );
            getTypedControlWindow()->SetDate( aDate );
        }
    }


    Any SAL_CALL ODateControl::getValue()
    {
        Any aPropValue;
        if ( !getTypedControlWindow()->GetText().isEmpty() )
        {
            ::Date aDate( getTypedControlWindow()->GetDate() );
            aPropValue <<= aDate.GetUNODate();
        }
        return aPropValue;
    }


    Type SAL_CALL ODateControl::getValueType()
    {
        return ::cppu::UnoType<util::Date>::get();
    }


    //= OEditControl


    OEditControl::OEditControl(vcl::Window* _pParent, bool _bPW, WinBits _nWinStyle)
        :OEditControl_Base( _bPW ? PropertyControlType::CharacterField : PropertyControlType::TextField, _pParent, _nWinStyle )
    {
        m_bIsPassword = _bPW;

        if ( m_bIsPassword )
           getTypedControlWindow()->SetMaxTextLen( 1 );
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

        getTypedControlWindow()->SetText( sText );
    }


    Any SAL_CALL OEditControl::getValue()
    {
        Any aPropValue;

        OUString sText( getTypedControlWindow()->GetText() );
        if ( m_bIsPassword )
        {
            if ( !sText.isEmpty() )
                aPropValue <<= (sal_Int16)sText[0];
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


    static long ImplCalcLongValue( double nValue, sal_uInt16 nDigits )
    {
        double n = nValue;
        for ( sal_uInt16 d = 0; d < nDigits; ++d )
            n *= 10;

        if ( n > ::std::numeric_limits< long >::max() )
            return ::std::numeric_limits< long >::max();
        return (long)n;
    }


    static double ImplCalcDoubleValue( long nValue, sal_uInt16 nDigits )
    {
        double n = nValue;
        for ( sal_uInt16 d = 0; d < nDigits; ++d )
            n /= 10;
        return n;
    }


    // class ODateTimeControl


    ODateTimeControl::ODateTimeControl( vcl::Window* _pParent, WinBits _nWinStyle)
        :ODateTimeControl_Base( PropertyControlType::DateTimeField, _pParent, _nWinStyle )
    {
        getTypedControlWindow()->EnableEmptyField( true );

        // determine a default format
        LanguageType eSysLanguage = SvtSysLocale().GetLanguageTag().getLanguageType( false);

        getTypedControlWindow()->SetFormatter( getTypedControlWindow()->StandardFormatter() );
        SvNumberFormatter* pFormatter = getTypedControlWindow()->GetFormatter();
        sal_uLong nStandardDateTimeFormat = pFormatter->GetStandardFormat( css::util::NumberFormat::DATETIME, eSysLanguage );

        getTypedControlWindow()->SetFormatKey( nStandardDateTimeFormat );
    }


    void SAL_CALL ODateTimeControl::setValue( const Any& _rValue )
    {
        if ( !_rValue.hasValue() )
        {
            getTypedControlWindow()->SetText( "" );
        }
        else
        {
            util::DateTime aUNODateTime;
            OSL_VERIFY( _rValue >>= aUNODateTime );

            ::DateTime aDateTime( ::DateTime::EMPTY );
            ::utl::typeConvert( aUNODateTime, aDateTime );

            double nValue = aDateTime - ::DateTime( *getTypedControlWindow()->GetFormatter()->GetNullDate() );
            getTypedControlWindow()->SetValue( nValue );
        }
    }


    Any SAL_CALL ODateTimeControl::getValue()
    {
        Any aPropValue;
        if ( !getTypedControlWindow()->GetText().isEmpty() )
        {
            double nValue = getTypedControlWindow()->GetValue();

            ::DateTime aDateTime( *getTypedControlWindow()->GetFormatter()->GetNullDate() );

            // add the "days" part
            double nDays = floor( nValue );
            aDateTime += nDays;

            // add the "time" part
            double nTime = nValue - nDays;
            nTime = ::rtl::math::round( nTime * 86400.0 ) / 86400.0;
                // we're not interested in 100th seconds, and this here prevents rounding errors
            aDateTime += nTime;

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


    //= HyperlinkInput


    HyperlinkInput::HyperlinkInput( vcl::Window* _pParent, WinBits _nWinStyle )
        :Edit( _pParent, _nWinStyle )
    {
        ::svtools::ColorConfig aColorConfig;
        ::svtools::ColorConfigValue aLinkColor( aColorConfig.GetColorValue( ::svtools::LINKS ) );

        AllSettings aAllSettings( GetSettings() );
        StyleSettings aStyleSettings( aAllSettings.GetStyleSettings() );

        vcl::Font aFieldFont( aStyleSettings.GetFieldFont() );
        aFieldFont.SetUnderline( LINESTYLE_SINGLE );
        aFieldFont.SetColor( aLinkColor.nColor );
        aStyleSettings.SetFieldFont( aFieldFont );

        aStyleSettings.SetFieldTextColor( aLinkColor.nColor );

        aAllSettings.SetStyleSettings( aStyleSettings );
        SetSettings( aAllSettings );
    }


    void HyperlinkInput::MouseMove( const ::MouseEvent& rMEvt )
    {
        Edit::MouseMove( rMEvt );

        PointerStyle ePointerStyle( PointerStyle::Text );

        if ( !rMEvt.IsLeaveWindow() )
        {
            if ( impl_textHitTest( rMEvt.GetPosPixel() ) )
                ePointerStyle = PointerStyle::RefHand;
        }

        SetPointer( Pointer( ePointerStyle ) );
    }


    void HyperlinkInput::MouseButtonDown( const ::MouseEvent& rMEvt )
    {
        Edit::MouseButtonDown( rMEvt );

        if ( impl_textHitTest( rMEvt.GetPosPixel() ) )
            m_aMouseButtonDownPos = rMEvt.GetPosPixel();
        else
            m_aMouseButtonDownPos.X() = m_aMouseButtonDownPos.Y() = -1;
    }


    void HyperlinkInput::MouseButtonUp( const ::MouseEvent& rMEvt )
    {
        Edit::MouseButtonUp( rMEvt );

        impl_checkEndClick( rMEvt );
    }


    bool HyperlinkInput::impl_textHitTest( const ::Point& _rWindowPos )
    {
        sal_Int32 nPos = GetCharPos( _rWindowPos );
        return ( ( nPos != EDIT_NOLIMIT ) && ( nPos < GetText().getLength() ) );
    }


    void HyperlinkInput::impl_checkEndClick( const ::MouseEvent& rMEvt )
    {
        const MouseSettings& rMouseSettings( GetSettings().GetMouseSettings() );
        if  (   ( std::abs( rMEvt.GetPosPixel().X() - m_aMouseButtonDownPos.X() ) < rMouseSettings.GetStartDragWidth() )
            &&  ( std::abs( rMEvt.GetPosPixel().Y() - m_aMouseButtonDownPos.Y() ) < rMouseSettings.GetStartDragHeight() )
            )
            Application::PostUserEvent( m_aClickHandler );
    }


    void HyperlinkInput::Tracking( const TrackingEvent& rTEvt )
    {
        Edit::Tracking( rTEvt );

        if ( rTEvt.IsTrackingEnded() )
            impl_checkEndClick( rTEvt.GetMouseEvent() );
    }


    //= OHyperlinkControl


    OHyperlinkControl::OHyperlinkControl( vcl::Window* _pParent, WinBits _nWinStyle )
        :OHyperlinkControl_Base( PropertyControlType::HyperlinkField, _pParent, _nWinStyle )
        ,m_aActionListeners( m_aMutex )
    {
        getTypedControlWindow()->SetClickHdl( LINK( this, OHyperlinkControl, OnHyperlinkClicked ) );
    }


    Any SAL_CALL OHyperlinkControl::getValue()
    {
        OUString sText = getTypedControlWindow()->GetText();
        return makeAny( sText );
    }


    void SAL_CALL OHyperlinkControl::setValue( const Any& _value )
    {
        OUString sText;
        _value >>= sText;
        getTypedControlWindow()->SetText( sText );
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
        OHyperlinkControl_Base::disposing();

        EventObject aEvent( *this );
        m_aActionListeners.disposeAndClear( aEvent );
    }


    IMPL_LINK_NOARG( OHyperlinkControl, OnHyperlinkClicked, void*, void )
    {
        ActionEvent aEvent( *this, "clicked" );
        m_aActionListeners.forEach< XActionListener >(
            [&aEvent] (uno::Reference<awt::XActionListener> const& xListener)
                { return xListener->actionPerformed(aEvent); });
    }


    //= ONumericControl


    ONumericControl::ONumericControl( vcl::Window* _pParent, WinBits _nWinStyle )
        :ONumericControl_Base( PropertyControlType::NumericField, _pParent, _nWinStyle )
        ,m_eValueUnit( FUNIT_NONE )
        ,m_nFieldToUNOValueFactor( 1 )
    {
        MetricField::SetDefaultUnit( FUNIT_NONE );

        getTypedControlWindow()->EnableEmptyFieldValue( true );
        getTypedControlWindow()->SetStrictFormat( true );
        Optional< double > value( getMaxValue() );
        value.Value = -value.Value;
        setMinValue( value );
    }


    ::sal_Int16 SAL_CALL ONumericControl::getDecimalDigits()
    {
        return getTypedControlWindow()->GetDecimalDigits();
    }


    void SAL_CALL ONumericControl::setDecimalDigits( ::sal_Int16 _decimaldigits )
    {
        getTypedControlWindow()->SetDecimalDigits( _decimaldigits );
    }


    Optional< double > SAL_CALL ONumericControl::getMinValue()
    {
        Optional< double > aReturn( true, 0 );

        sal_Int64 minValue = getTypedControlWindow()->GetMin();
        if ( minValue == ::std::numeric_limits< sal_Int64 >::min() )
            aReturn.IsPresent = false;
        else
            aReturn.Value = (double)minValue;

        return aReturn;
    }


    void SAL_CALL ONumericControl::setMinValue( const Optional< double >& _minvalue )
    {
        if ( !_minvalue.IsPresent )
            getTypedControlWindow()->SetMin( ::std::numeric_limits< sal_Int64 >::min() );
        else
            getTypedControlWindow()->SetMin( impl_apiValueToFieldValue_nothrow( _minvalue.Value ) , m_eValueUnit);
    }


    Optional< double > SAL_CALL ONumericControl::getMaxValue()
    {
        Optional< double > aReturn( true, 0 );

        sal_Int64 maxValue = getTypedControlWindow()->GetMax();
        if ( maxValue == ::std::numeric_limits< sal_Int64 >::max() )
            aReturn.IsPresent = false;
        else
            aReturn.Value = (double)maxValue;

        return aReturn;
    }


    void SAL_CALL ONumericControl::setMaxValue( const Optional< double >& _maxvalue )
    {
        if ( !_maxvalue.IsPresent )
            getTypedControlWindow()->SetMax( ::std::numeric_limits< sal_Int64 >::max() );
        else
            getTypedControlWindow()->SetMax( impl_apiValueToFieldValue_nothrow( _maxvalue.Value ), m_eValueUnit );
    }


    ::sal_Int16 SAL_CALL ONumericControl::getDisplayUnit()
    {
        return VCLUnoHelper::ConvertToMeasurementUnit( getTypedControlWindow()->GetUnit(), 1 );
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
        getTypedControlWindow()->MetricFormatter::SetUnit( eFieldUnit );
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
            getTypedControlWindow()->SetText( "" );
            getTypedControlWindow()->SetEmptyFieldValue();
        }
        else
        {
            double nValue( 0 );
            OSL_VERIFY( _rValue >>= nValue );
            long nControlValue = impl_apiValueToFieldValue_nothrow( nValue );
            getTypedControlWindow()->SetValue( nControlValue, m_eValueUnit );
        }
    }


    long ONumericControl::impl_apiValueToFieldValue_nothrow( double _nApiValue ) const
    {
        long nControlValue = ImplCalcLongValue( _nApiValue, getTypedControlWindow()->GetDecimalDigits() );
        nControlValue /= m_nFieldToUNOValueFactor;
        return nControlValue;
    }


    double ONumericControl::impl_fieldValueToApiValue_nothrow( sal_Int64 _nFieldValue ) const
    {
        double nApiValue = ImplCalcDoubleValue( (long)_nFieldValue, getTypedControlWindow()->GetDecimalDigits() );
        nApiValue *= m_nFieldToUNOValueFactor;
        return nApiValue;
    }


    Any SAL_CALL ONumericControl::getValue()
    {
        Any aPropValue;
        if ( !getTypedControlWindow()->GetText().isEmpty() )
        {
            double nValue = impl_fieldValueToApiValue_nothrow( getTypedControlWindow()->GetValue( m_eValueUnit ) );
            aPropValue <<= nValue;
        }
        return aPropValue;
    }


    Type SAL_CALL ONumericControl::getValueType()
    {
        return ::cppu::UnoType<double>::get();
    }


    //= OColorControl

    #define LB_DEFAULT_COUNT 20

    OUString MakeHexStr(sal_uInt32 nVal, sal_Int32 nLength)
    {
        OUStringBuffer aStr;
        while (nVal>0)
        {
            char c = char(nVal & 0x000F);
            nVal >>= 4;
            if (c<=9) c += '0';
            else c += 'A' - 10;
            aStr.insert(0, c);
        }
        while (aStr.getLength() < nLength) aStr.insert(0, '0');
        return aStr.makeStringAndClear();
    }

    OColorControl::OColorControl(vcl::Window* pParent, WinBits nWinStyle)
        : OColorControl_Base(PropertyControlType::ColorListBox, pParent, nWinStyle)
    {
    }

    void SAL_CALL OColorControl::setValue( const Any& _rValue )
    {
        if ( _rValue.hasValue() )
        {
            css::util::Color nColor = COL_TRANSPARENT;
            _rValue >>= nColor;
            ::Color aRgbCol((ColorData)nColor);
           getTypedControlWindow()->SelectEntry(std::make_pair(aRgbCol, MakeHexStr(nColor, 8)));
        }
        else
            getTypedControlWindow()->SetNoSelection();
    }

    Any SAL_CALL OColorControl::getValue()
    {
        Any aPropValue;
        if (!getTypedControlWindow()->IsNoSelection())
        {
            ::Color aRgbCol = getTypedControlWindow()->GetSelectEntryColor();
            aPropValue <<= (css::util::Color)aRgbCol.GetColor();
        }
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

    OListboxControl::OListboxControl( vcl::Window* pParent, WinBits nWinStyle)
        :OListboxControl_Base( PropertyControlType::ListBox, pParent, nWinStyle )
    {
        getTypedControlWindow()->SetDropDownLineCount( LB_DEFAULT_COUNT );
        if ( ( nWinStyle & WB_READONLY ) != 0 )
        {
            getTypedControlWindow()->SetReadOnly();
            getTypedControlWindow()->Enable();
        }
    }


    Any SAL_CALL OListboxControl::getValue()
    {
        OUString sControlValue( getTypedControlWindow()->GetSelectEntry() );

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
            getTypedControlWindow()->SetNoSelection();
        else
        {
            OUString sSelection;
            _rValue >>= sSelection;

            if ( !sSelection.equals( getTypedControlWindow()->GetSelectEntry() ) )
                getTypedControlWindow()->SelectEntry( sSelection );

            if ( !getTypedControlWindow()->IsEntrySelected( sSelection ) )
            {
                getTypedControlWindow()->InsertEntry( sSelection, 0 );
                getTypedControlWindow()->SelectEntry( sSelection );
            }
        }
    }


    void SAL_CALL OListboxControl::clearList()
    {
        getTypedControlWindow()->Clear();
    }


    void SAL_CALL OListboxControl::prependListEntry( const OUString& NewEntry )
    {
        getTypedControlWindow()->InsertEntry( NewEntry, 0 );
    }


    void SAL_CALL OListboxControl::appendListEntry( const OUString& NewEntry )
    {
        getTypedControlWindow()->InsertEntry( NewEntry );
    }

    Sequence< OUString > SAL_CALL OListboxControl::getListEntries(  )
    {
        const sal_Int32 nCount = getTypedControlWindow()->GetEntryCount();
        Sequence< OUString > aRet(nCount);
        OUString* pIter = aRet.getArray();
        for (sal_Int32 i = 0; i < nCount ; ++i,++pIter)
            *pIter = getTypedControlWindow()->GetEntry(i);

        return aRet;
    }


    void OListboxControl::setModified()
    {
        OListboxControl_Base::setModified();

        if ( !getTypedControlWindow()->IsTravelSelect() )
            // fire a commit
            notifyModifiedValue();
    }


    //= OComboboxControl


    OComboboxControl::OComboboxControl( vcl::Window* pParent, WinBits nWinStyle)
        :OComboboxControl_Base( PropertyControlType::ComboBox, pParent, nWinStyle )
    {
        getTypedControlWindow()->SetDropDownLineCount( LB_DEFAULT_COUNT );
        getTypedControlWindow()->SetSelectHdl( LINK( this, OComboboxControl, OnEntrySelected ) );
    }


    void SAL_CALL OComboboxControl::setValue( const Any& _rValue )
    {
        OUString sText;
        _rValue >>= sText;
        getTypedControlWindow()->SetText( sText );
    }


    Any SAL_CALL OComboboxControl::getValue()
    {
        return makeAny( OUString( getTypedControlWindow()->GetText() ) );
    }


    Type SAL_CALL OComboboxControl::getValueType()
    {
        return ::cppu::UnoType<OUString>::get();
    }


    void SAL_CALL OComboboxControl::clearList()
    {
        getTypedControlWindow()->Clear();
    }


    void SAL_CALL OComboboxControl::prependListEntry( const OUString& NewEntry )
    {
        getTypedControlWindow()->InsertEntry( NewEntry, 0 );
    }


    void SAL_CALL OComboboxControl::appendListEntry( const OUString& NewEntry )
    {
        getTypedControlWindow()->InsertEntry( NewEntry );
    }

    Sequence< OUString > SAL_CALL OComboboxControl::getListEntries(  )
    {
        const sal_Int32 nCount = getTypedControlWindow()->GetEntryCount();
        Sequence< OUString > aRet(nCount);
        OUString* pIter = aRet.getArray();
        for (sal_Int32 i = 0; i < nCount ; ++i,++pIter)
            *pIter = getTypedControlWindow()->GetEntry(i);

        return aRet;
    }


    IMPL_LINK_NOARG( OComboboxControl, OnEntrySelected, ComboBox&, void )
    {
        if ( !getTypedControlWindow()->IsTravelSelect() )
            // fire a commit
            notifyModifiedValue();
    }


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


    OMultilineFloatingEdit::OMultilineFloatingEdit(vcl::Window* _pParent)
        :FloatingWindow(_pParent, WB_BORDER)
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


    DropDownEditControl::DropDownEditControl( vcl::Window* _pParent, WinBits _nStyle )
        :Edit( _pParent, _nStyle )
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


    namespace
    {

        StlSyntaxSequence< OUString > lcl_convertMultiLineToList( const OUString& _rCompsedTextWithLineBreaks )
        {
            sal_Int32 nLines = comphelper::string::getTokenCount(_rCompsedTextWithLineBreaks, '\n');
            StlSyntaxSequence< OUString > aStrings( nLines );
            StlSyntaxSequence< OUString >::iterator stringItem = aStrings.begin();
            for ( sal_Int32 token = 0; token < nLines; ++token, ++stringItem )
                *stringItem = _rCompsedTextWithLineBreaks.getToken( token, '\n' );
            return aStrings;
        }

        OUString lcl_convertListToMultiLine( const StlSyntaxSequence< OUString >& _rStrings )
        {
            OUString sMultiLineText;
            for (   StlSyntaxSequence< OUString >::const_iterator item = _rStrings.begin();
                    item != _rStrings.end();
                )
            {
                sMultiLineText += *item;
                if ( ++item != _rStrings.end() )
                    sMultiLineText += "\n";
            }
            return sMultiLineText;
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


    #define STD_HEIGHT  100
    void DropDownEditControl::ShowDropDown( bool bShow )
    {
        if (bShow)
        {
            ::Point aMePos= GetPosPixel();
            aMePos = GetParent()->OutputToScreenPixel( aMePos );
            ::Size aSize=GetSizePixel();
            ::Rectangle aRect(aMePos,aSize);
            aSize.Height() = STD_HEIGHT;
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
            sal_Int32 nCount = comphelper::string::getTokenCount(aStr, '\n');

            OUString aInput = aStr.getToken(0,'\n' );

            if (!aInput.isEmpty())
            {
                aOutput += "\"";
                nDiff++;
                aOutput += aInput;
                aOutput += "\"";
            }

            if (nSinglePos <= aOutput.getLength())
            {
                nPos=nSinglePos-nDiff;
            }
            else
            {
                for (sal_Int32 i=1; i<nCount; ++i)
                {
                    aInput=aStr.getToken((sal_uInt16)i, '\n');
                    if (!aInput.isEmpty())
                    {
                        aOutput += ";";
                        aOutput += "\"";
                        nDiff += 2;
                        aOutput += aInput;
                        aOutput += "\"";

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


    void DropDownEditControl::SetStringListValue( const StlSyntaxSequence< OUString >& _rStrings )
    {
        SetText( lcl_convertListToDisplayText( _rStrings ) );
        m_pFloatingEdit->getEdit().SetText( lcl_convertListToMultiLine( _rStrings ) );
    }


    StlSyntaxSequence< OUString > DropDownEditControl::GetStringListValue() const
    {
        return lcl_convertMultiLineToList( m_pFloatingEdit->getEdit().GetText() );
    }


    void DropDownEditControl::SetTextValue( const OUString& _rText )
    {
        OSL_PRECOND( m_nOperationMode == eMultiLineText, "DropDownEditControl::SetTextValue: illegal call!" );

        m_pFloatingEdit->getEdit().SetText( _rText );
        SetText( _rText );
    }


    OUString DropDownEditControl::GetTextValue() const
    {
        OSL_PRECOND( m_nOperationMode == eMultiLineText, "DropDownEditControl::GetTextValue: illegal call!" );
        return GetText();
    }


    //= OMultilineEditControl


    OMultilineEditControl::OMultilineEditControl( vcl::Window* pParent, MultiLineOperationMode _eMode, WinBits nWinStyle )
        :OMultilineEditControl_Base( _eMode == eMultiLineText ? PropertyControlType::MultiLineTextField : PropertyControlType::StringListField
                                   , pParent
                                   , ( nWinStyle | WB_DIALOGCONTROL ) & ( ~WB_READONLY | ~WB_DROPDOWN )
                                   , false )
    {
        getTypedControlWindow()->setOperationMode( _eMode );
        getTypedControlWindow()->setControlHelper( *this );
    }


    void SAL_CALL OMultilineEditControl::setValue( const Any& _rValue )
    {
        impl_checkDisposed_throw();

        switch ( getTypedControlWindow()->getOperationMode() )
        {
        case eMultiLineText:
        {
            OUString sText;
            if ( !( _rValue >>= sText ) && _rValue.hasValue() )
                throw IllegalTypeException();
            getTypedControlWindow()->SetTextValue( sText );
        }
        break;
        case eStringList:
        {
            Sequence< OUString > aStringLines;
            if ( !( _rValue >>= aStringLines ) && _rValue.hasValue() )
                throw IllegalTypeException();
            getTypedControlWindow()->SetStringListValue( StlSyntaxSequence<OUString>(aStringLines) );
        }
        break;
        }
    }


    Any SAL_CALL OMultilineEditControl::getValue()
    {
        impl_checkDisposed_throw();

        Any aValue;
        switch ( getTypedControlWindow()->getOperationMode() )
        {
        case eMultiLineText:
            aValue <<= getTypedControlWindow()->GetTextValue();
            break;
        case eStringList:
            aValue <<= getTypedControlWindow()->GetStringListValue();
            break;
        }
        return aValue;
    }


    Type SAL_CALL OMultilineEditControl::getValueType()
    {
        if ( getTypedControlWindow()->getOperationMode() == eMultiLineText )
            return ::cppu::UnoType<OUString>::get();
        return cppu::UnoType<Sequence< OUString >>::get();
    }


} // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
