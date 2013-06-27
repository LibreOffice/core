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

#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/inspection/PropertyControlType.hpp>
#include <comphelper/string.hxx>
#include <rtl/math.hxx>
#include <sfx2/objsh.hxx>

//==================================================================
// ugly dependencies for the OColorControl
#include <svx/svxids.hrc>
#include <svx/drawitem.hxx>
#include <svx/xtable.hxx>
//==================================================================
#include <vcl/floatwin.hxx>
#include <svtools/svmedit.hxx>
#include <svtools/colorcfg.hxx>
#include <unotools/syslocale.hxx>
#include <unotools/datetime.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <vcl/button.hxx>
#include <vcl/svapp.hxx>
//==================================================================

#include <memory>
#include <limits>
#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>

//............................................................................
namespace pcr
{
//............................................................................

    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::inspection;

    //==================================================================
    //= OTimeControl
    //==================================================================
    //------------------------------------------------------------------
    OTimeControl::OTimeControl( Window* pParent, WinBits nWinStyle )
        :OTimeControl_Base( PropertyControlType::TimeField, pParent, nWinStyle )
    {
        getTypedControlWindow()->SetStrictFormat( sal_True );
        getTypedControlWindow()->SetFormat( TIMEF_SEC );
        getTypedControlWindow()->EnableEmptyFieldValue( sal_True );
    }

    //------------------------------------------------------------------
    void SAL_CALL OTimeControl::setValue( const Any& _rValue ) throw (IllegalTypeException, RuntimeException)
    {
        util::Time aUNOTime;
        if ( !( _rValue >>= aUNOTime ) )
        {
            getTypedControlWindow()->SetText( String() );
            getTypedControlWindow()->SetEmptyTime();
        }
        else
        {
            ::Time aTime( aUNOTime.Hours, aUNOTime.Minutes, aUNOTime.Seconds, aUNOTime.NanoSeconds );
            getTypedControlWindow()->SetTime( aTime );
        }
    }

    //------------------------------------------------------------------
    Any SAL_CALL OTimeControl::getValue() throw (RuntimeException)
    {
        Any aPropValue;
        if ( !getTypedControlWindow()->GetText().isEmpty() )
        {
            ::Time aTime( getTypedControlWindow()->GetTime() );
            util::Time const aUNOTime( aTime.GetNanoSec(), aTime.GetSec(),
                    aTime.GetMin(), aTime.GetHour(), false );
            aPropValue <<= aUNOTime;
        }
        return aPropValue;
    }

    //------------------------------------------------------------------
    Type SAL_CALL OTimeControl::getValueType() throw (RuntimeException)
    {
        return ::getCppuType( static_cast< util::Time* >( NULL ) );
    }

    //==================================================================
    //= ODateControl
    //==================================================================
    //------------------------------------------------------------------
    ODateControl::ODateControl( Window* pParent, WinBits nWinStyle )
        :ODateControl_Base( PropertyControlType::DateField, pParent, nWinStyle | WB_DROPDOWN )
    {
        WindowType* pControlWindow = getTypedControlWindow();
        pControlWindow->SetStrictFormat(sal_True);

        pControlWindow->SetMin( ::Date( 1,1,1600 ) );
        pControlWindow->SetFirst( ::Date( 1,1,1600 ) );
        pControlWindow->SetLast( ::Date( 1, 1, 9999 ) );
        pControlWindow->SetMax( ::Date( 1, 1, 9999 ) );

        pControlWindow->SetExtDateFormat( XTDATEF_SYSTEM_SHORT_YYYY );
        pControlWindow->EnableEmptyFieldValue( sal_True );
    }

    //------------------------------------------------------------------
    void SAL_CALL ODateControl::setValue( const Any& _rValue ) throw (IllegalTypeException, RuntimeException)
    {
        util::Date aUNODate;
        if ( !( _rValue >>= aUNODate ) )
        {
            getTypedControlWindow()->SetText( String() );
            getTypedControlWindow()->SetEmptyDate();
        }
        else
        {
            ::Date aDate( aUNODate.Day, aUNODate.Month, aUNODate.Year );
            getTypedControlWindow()->SetDate( aDate );
        }
    }

    //------------------------------------------------------------------
    Any SAL_CALL ODateControl::getValue() throw (RuntimeException)
    {
        Any aPropValue;
        if ( !getTypedControlWindow()->GetText().isEmpty() )
        {
            ::Date aDate( getTypedControlWindow()->GetDate() );
            util::Date aUNODate( aDate.GetDay(), aDate.GetMonth(), aDate.GetYear() );
            aPropValue <<= aUNODate;
        }
        return aPropValue;
    }

    //------------------------------------------------------------------
    Type SAL_CALL ODateControl::getValueType() throw (RuntimeException)
    {
        return ::getCppuType( static_cast< util::Date* >( NULL ) );
    }

    //==================================================================
    //= OEditControl
    //==================================================================
    //------------------------------------------------------------------
    OEditControl::OEditControl(Window* _pParent, sal_Bool _bPW, WinBits _nWinStyle)
        :OEditControl_Base( _bPW ? PropertyControlType::CharacterField : PropertyControlType::TextField, _pParent, _nWinStyle )
    {
        m_bIsPassword = _bPW;

        if ( m_bIsPassword )
           getTypedControlWindow()->SetMaxTextLen( 1 );
    }

    //------------------------------------------------------------------
    void SAL_CALL OEditControl::setValue( const Any& _rValue ) throw (IllegalTypeException, RuntimeException)
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

    //------------------------------------------------------------------
    Any SAL_CALL OEditControl::getValue() throw (RuntimeException)
    {
        Any aPropValue;

        OUString sText( getTypedControlWindow()->GetText() );
        if ( m_bIsPassword )
        {
            if ( !sText.isEmpty() )
                aPropValue <<= (sal_Int16)sText.getStr()[0];
        }
        else
            aPropValue <<= sText;

        return aPropValue;
    }

    //------------------------------------------------------------------
    Type SAL_CALL OEditControl::getValueType() throw (RuntimeException)
    {
        return m_bIsPassword ? ::getCppuType( static_cast< sal_Int16* >( NULL ) ) : ::getCppuType( static_cast< OUString* >( NULL ) );
    }

    //------------------------------------------------------------------
    void OEditControl::modified()
    {
        OEditControl_Base::modified();

        // for pasword controls, we fire a commit for every single change
        if ( m_bIsPassword )
            m_aImplControl.notifyModifiedValue();
    }

    //------------------------------------------------------------------
    static long ImplCalcLongValue( double nValue, sal_uInt16 nDigits )
    {
        double n = nValue;
        for ( sal_uInt16 d = 0; d < nDigits; ++d )
            n *= 10;

        if ( n > ::std::numeric_limits< long >::max() )
            return ::std::numeric_limits< long >::max();
        return (long)n;
    }

    //------------------------------------------------------------------
    static double ImplCalcDoubleValue( long nValue, sal_uInt16 nDigits )
    {
        double n = nValue;
        for ( sal_uInt16 d = 0; d < nDigits; ++d )
            n /= 10;
        return n;
    }

    //==================================================================
    // class ODateTimeControl
    //==================================================================
    //------------------------------------------------------------------
    ODateTimeControl::ODateTimeControl( Window* _pParent, WinBits _nWinStyle)
        :ODateTimeControl_Base( PropertyControlType::DateTimeField, _pParent, _nWinStyle )
    {
        getTypedControlWindow()->EnableEmptyField( sal_True );

        // determine a default format
        LanguageType eSysLanguage = SvtSysLocale().GetLanguageTag().getLanguageType( false);

        getTypedControlWindow()->SetFormatter( getTypedControlWindow()->StandardFormatter() );
        SvNumberFormatter* pFormatter = getTypedControlWindow()->GetFormatter();
        sal_uLong nStandardDateTimeFormat = pFormatter->GetStandardFormat( NUMBERFORMAT_DATETIME, eSysLanguage );

        getTypedControlWindow()->SetFormatKey( nStandardDateTimeFormat );
    }

    //------------------------------------------------------------------
    void SAL_CALL ODateTimeControl::setValue( const Any& _rValue ) throw (IllegalTypeException, RuntimeException)
    {
        if ( !_rValue.hasValue() )
        {
            getTypedControlWindow()->SetText( String() );
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

    //------------------------------------------------------------------
    Any SAL_CALL ODateTimeControl::getValue() throw (RuntimeException)
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

    //------------------------------------------------------------------
    Type SAL_CALL ODateTimeControl::getValueType() throw (RuntimeException)
    {
        return ::getCppuType( static_cast< util::DateTime* >( NULL ) );
    }

    //========================================================================
    //= HyperlinkInput
    //========================================================================
    //--------------------------------------------------------------------
    HyperlinkInput::HyperlinkInput( Window* _pParent, WinBits _nWinStyle )
        :Edit( _pParent, _nWinStyle )
    {
        ::svtools::ColorConfig aColorConfig;
        ::svtools::ColorConfigValue aLinkColor( aColorConfig.GetColorValue( ::svtools::LINKS ) );

        AllSettings aAllSettings( GetSettings() );
        StyleSettings aStyleSettings( aAllSettings.GetStyleSettings() );

        Font aFieldFont( aStyleSettings.GetFieldFont() );
        aFieldFont.SetUnderline( UNDERLINE_SINGLE );
        aFieldFont.SetColor( aLinkColor.nColor );
        aStyleSettings.SetFieldFont( aFieldFont );

        aStyleSettings.SetFieldTextColor( aLinkColor.nColor );

        aAllSettings.SetStyleSettings( aStyleSettings );
        SetSettings( aAllSettings );
    }

    //--------------------------------------------------------------------
    void HyperlinkInput::MouseMove( const ::MouseEvent& rMEvt )
    {
        Edit::MouseMove( rMEvt );

        PointerStyle ePointerStyle( POINTER_TEXT );

        if ( !rMEvt.IsLeaveWindow() )
        {
            if ( impl_textHitTest( rMEvt.GetPosPixel() ) )
                ePointerStyle = POINTER_REFHAND;
        }

        SetPointer( Pointer( ePointerStyle ) );
    }

    //--------------------------------------------------------------------
    void HyperlinkInput::MouseButtonDown( const ::MouseEvent& rMEvt )
    {
        Edit::MouseButtonDown( rMEvt );

        if ( impl_textHitTest( rMEvt.GetPosPixel() ) )
            m_aMouseButtonDownPos = rMEvt.GetPosPixel();
        else
            m_aMouseButtonDownPos.X() = m_aMouseButtonDownPos.Y() = -1;
    }

    //--------------------------------------------------------------------
    void HyperlinkInput::MouseButtonUp( const ::MouseEvent& rMEvt )
    {
        Edit::MouseButtonUp( rMEvt );

        impl_checkEndClick( rMEvt );
    }

    //--------------------------------------------------------------------
    bool HyperlinkInput::impl_textHitTest( const ::Point& _rWindowPos )
    {
        xub_StrLen nPos = GetCharPos( _rWindowPos );
        return ( ( nPos != STRING_LEN ) && ( nPos < GetText().getLength() ) );
    }

    //--------------------------------------------------------------------
    void HyperlinkInput::impl_checkEndClick( const ::MouseEvent rMEvt )
    {
        const MouseSettings& rMouseSettings( GetSettings().GetMouseSettings() );
        if  (   ( abs( rMEvt.GetPosPixel().X() - m_aMouseButtonDownPos.X() ) < rMouseSettings.GetStartDragWidth() )
            &&  ( abs( rMEvt.GetPosPixel().Y() - m_aMouseButtonDownPos.Y() ) < rMouseSettings.GetStartDragHeight() )
            )
            Application::PostUserEvent( m_aClickHandler );
    }

    //--------------------------------------------------------------------
    void HyperlinkInput::Tracking( const TrackingEvent& rTEvt )
    {
        Edit::Tracking( rTEvt );

        if ( rTEvt.IsTrackingEnded() )
            impl_checkEndClick( rTEvt.GetMouseEvent() );
    }

    //========================================================================
    //= OHyperlinkControl
    //========================================================================
    //--------------------------------------------------------------------
    OHyperlinkControl::OHyperlinkControl( Window* _pParent, WinBits _nWinStyle )
        :OHyperlinkControl_Base( PropertyControlType::HyperlinkField, _pParent, _nWinStyle )
        ,m_aActionListeners( m_aMutex )
    {
        getTypedControlWindow()->SetClickHdl( LINK( this, OHyperlinkControl, OnHyperlinkClicked ) );
    }

    //--------------------------------------------------------------------
    Any SAL_CALL OHyperlinkControl::getValue() throw (RuntimeException)
    {
        OUString sText = getTypedControlWindow()->GetText();
        return makeAny( sText );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OHyperlinkControl::setValue( const Any& _value ) throw (IllegalTypeException, RuntimeException)
    {
        OUString sText;
        _value >>= sText;
        getTypedControlWindow()->SetText( sText );
    }

    //--------------------------------------------------------------------
    Type SAL_CALL OHyperlinkControl::getValueType() throw (RuntimeException)
    {
        return ::getCppuType( static_cast< OUString* >( NULL ) );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OHyperlinkControl::addActionListener( const Reference< XActionListener >& listener ) throw (RuntimeException)
    {
        if ( listener.is() )
            m_aActionListeners.addInterface( listener );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OHyperlinkControl::removeActionListener( const Reference< XActionListener >& listener ) throw (RuntimeException)
    {
        m_aActionListeners.removeInterface( listener );
    }

    //------------------------------------------------------------------
    void SAL_CALL OHyperlinkControl::disposing()
    {
        OHyperlinkControl_Base::disposing();

        EventObject aEvent( *this );
        m_aActionListeners.disposeAndClear( aEvent );
    }

    //------------------------------------------------------------------
    IMPL_LINK( OHyperlinkControl, OnHyperlinkClicked, void*, /*_NotInterestedIn*/ )
    {
        ActionEvent aEvent( *this, OUString( "clicked" ) );
        m_aActionListeners.forEach< XActionListener >(
            boost::bind(
                &XActionListener::actionPerformed,
                _1, boost::cref(aEvent) ) );

        return 0;
    }

    //==================================================================
    //= ONumericControl
    //==================================================================
    //------------------------------------------------------------------
    ONumericControl::ONumericControl( Window* _pParent, WinBits _nWinStyle )
        :ONumericControl_Base( PropertyControlType::NumericField, _pParent, _nWinStyle )
        ,m_eValueUnit( FUNIT_NONE )
        ,m_nFieldToUNOValueFactor( 1 )
    {
        getTypedControlWindow()->SetDefaultUnit( FUNIT_NONE );

        getTypedControlWindow()->EnableEmptyFieldValue( sal_True );
        getTypedControlWindow()->SetStrictFormat( sal_True );
        Optional< double > value( getMaxValue() );
        value.Value = -value.Value;
        setMinValue( value );
    }

    //--------------------------------------------------------------------
    ::sal_Int16 SAL_CALL ONumericControl::getDecimalDigits() throw (RuntimeException)
    {
        return getTypedControlWindow()->GetDecimalDigits();
    }

    //--------------------------------------------------------------------
    void SAL_CALL ONumericControl::setDecimalDigits( ::sal_Int16 _decimaldigits ) throw (RuntimeException)
    {
        getTypedControlWindow()->SetDecimalDigits( _decimaldigits );
    }

    //--------------------------------------------------------------------
    Optional< double > SAL_CALL ONumericControl::getMinValue() throw (RuntimeException)
    {
        Optional< double > aReturn( sal_True, 0 );

        sal_Int64 minValue = getTypedControlWindow()->GetMin();
        if ( minValue == ::std::numeric_limits< sal_Int64 >::min() )
            aReturn.IsPresent = sal_False;
        else
            aReturn.Value = (double)minValue;

        return aReturn;
    }

    //--------------------------------------------------------------------
    void SAL_CALL ONumericControl::setMinValue( const Optional< double >& _minvalue ) throw (RuntimeException)
    {
        if ( !_minvalue.IsPresent )
            getTypedControlWindow()->SetMin( ::std::numeric_limits< sal_Int64 >::min() );
        else
            getTypedControlWindow()->SetMin( impl_apiValueToFieldValue_nothrow( _minvalue.Value ) , m_eValueUnit);
    }

    //--------------------------------------------------------------------
    Optional< double > SAL_CALL ONumericControl::getMaxValue() throw (RuntimeException)
    {
        Optional< double > aReturn( sal_True, 0 );

        sal_Int64 maxValue = getTypedControlWindow()->GetMax();
        if ( maxValue == ::std::numeric_limits< sal_Int64 >::max() )
            aReturn.IsPresent = sal_False;
        else
            aReturn.Value = (double)maxValue;

        return aReturn;
    }

    //--------------------------------------------------------------------
    void SAL_CALL ONumericControl::setMaxValue( const Optional< double >& _maxvalue ) throw (RuntimeException)
    {
        if ( !_maxvalue.IsPresent )
            getTypedControlWindow()->SetMax( ::std::numeric_limits< sal_Int64 >::max() );
        else
            getTypedControlWindow()->SetMax( impl_apiValueToFieldValue_nothrow( _maxvalue.Value ), m_eValueUnit );
    }

    //--------------------------------------------------------------------
    ::sal_Int16 SAL_CALL ONumericControl::getDisplayUnit() throw (RuntimeException)
    {
        return VCLUnoHelper::ConvertToMeasurementUnit( getTypedControlWindow()->GetUnit(), 1 );
    }

    //--------------------------------------------------------------------
    void SAL_CALL ONumericControl::setDisplayUnit( ::sal_Int16 _displayunit ) throw (IllegalArgumentException, RuntimeException)
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

    //--------------------------------------------------------------------
    ::sal_Int16 SAL_CALL ONumericControl::getValueUnit() throw (RuntimeException)
    {
        return VCLUnoHelper::ConvertToMeasurementUnit( m_eValueUnit, m_nFieldToUNOValueFactor );
    }

    //--------------------------------------------------------------------
    void SAL_CALL ONumericControl::setValueUnit( ::sal_Int16 _valueunit ) throw (RuntimeException)
    {
        if ( ( _valueunit < MeasureUnit::MM_100TH ) || ( _valueunit > MeasureUnit::PERCENT ) )
            throw IllegalArgumentException();
        m_eValueUnit = VCLUnoHelper::ConvertToFieldUnit( _valueunit, m_nFieldToUNOValueFactor );
    }

    //--------------------------------------------------------------------
    void SAL_CALL ONumericControl::setValue( const Any& _rValue ) throw (IllegalTypeException, RuntimeException)
    {
        if ( !_rValue.hasValue() )
        {
            getTypedControlWindow()->SetText( String() );
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

    //------------------------------------------------------------------
    long ONumericControl::impl_apiValueToFieldValue_nothrow( double _nApiValue ) const
    {
        long nControlValue = ImplCalcLongValue( _nApiValue, getTypedControlWindow()->GetDecimalDigits() );
        nControlValue /= m_nFieldToUNOValueFactor;
        return nControlValue;
    }

    //------------------------------------------------------------------
    double ONumericControl::impl_fieldValueToApiValue_nothrow( sal_Int64 _nFieldValue ) const
    {
        double nApiValue = ImplCalcDoubleValue( (long)_nFieldValue, getTypedControlWindow()->GetDecimalDigits() );
        nApiValue *= m_nFieldToUNOValueFactor;
        return nApiValue;
    }

    //------------------------------------------------------------------
    Any SAL_CALL ONumericControl::getValue() throw (RuntimeException)
    {
        Any aPropValue;
        if ( !getTypedControlWindow()->GetText().isEmpty() )
        {
            double nValue = impl_fieldValueToApiValue_nothrow( getTypedControlWindow()->GetValue( m_eValueUnit ) );
            aPropValue <<= nValue;
        }
        return aPropValue;
    }

    //------------------------------------------------------------------
    Type SAL_CALL ONumericControl::getValueType() throw (RuntimeException)
    {
        return ::getCppuType( static_cast< double* >( NULL ) );
    }

    //==================================================================
    //= OColorControl
    //==================================================================
    #define LB_DEFAULT_COUNT 20
    //------------------------------------------------------------------
    String MakeHexStr(sal_uInt32 nVal, sal_uInt32 nLength)
    {
        String aStr;
        while (nVal>0)
        {
            char c=char(nVal & 0x000F);
            nVal>>=4;
            if (c<=9) c+='0';
            else c+='A'-10;
            aStr.Insert(c,0);
        }
        while (aStr.Len() < nLength) aStr.Insert('0',0);
        return aStr;
    }

    //------------------------------------------------------------------
    OColorControl::OColorControl(Window* pParent, WinBits nWinStyle)
        :OColorControl_Base( PropertyControlType::ColorListBox, pParent, nWinStyle )
    {
        // initialize the color listbox
        XColorListRef pColorList;
        SfxObjectShell* pDocSh = SfxObjectShell::Current();
        const SfxPoolItem* pItem = pDocSh ? pDocSh->GetItem( SID_COLOR_TABLE ) : NULL;
        if ( pItem )
        {
            DBG_ASSERT(pItem->ISA(SvxColorListItem), "OColorControl::OColorControl: invalid color item!");
            pColorList = ( (SvxColorListItem*)pItem )->GetColorList();
        }

        if ( !pColorList.is() )
            pColorList = XColorList::GetStdColorList();


        DBG_ASSERT(pColorList.is(), "OColorControl::OColorControl: no color table!");

        if ( pColorList.is() )
        {
            for (sal_uInt16 i = 0; i < pColorList->Count(); ++i)
            {
                XColorEntry* pEntry = pColorList->GetColor( i );
                getTypedControlWindow()->InsertEntry( pEntry->GetColor(), pEntry->GetName() );
            }
        }

        getTypedControlWindow()->SetDropDownLineCount( LB_DEFAULT_COUNT );
        if ( ( nWinStyle & WB_READONLY ) != 0 )
        {
            getTypedControlWindow()->SetReadOnly( sal_True );
            getTypedControlWindow()->Enable( sal_True );
        }
    }

    //------------------------------------------------------------------
    void SAL_CALL OColorControl::setValue( const Any& _rValue ) throw (IllegalTypeException, RuntimeException)
    {
        if ( _rValue.hasValue() )
        {
            ::com::sun::star::util::Color nColor = COL_TRANSPARENT;
            if ( _rValue >>= nColor )
            {
                ::Color aRgbCol((ColorData)nColor);

                getTypedControlWindow()->SelectEntry( aRgbCol );
                if ( !getTypedControlWindow()->IsEntrySelected( aRgbCol ) )
                {   // the given color is not part of the list -> insert a new entry with the hex code of the color
                    String aStr = OUString("0x");
                    aStr += MakeHexStr(nColor,8);
                    getTypedControlWindow()->InsertEntry( aRgbCol, aStr );
                    getTypedControlWindow()->SelectEntry( aRgbCol );
                }
            }
            else
            {
                OUString sNonColorValue;
                if ( !( _rValue >>= sNonColorValue ) )
                    throw IllegalTypeException();
                getTypedControlWindow()->SelectEntry( sNonColorValue );
                if ( !getTypedControlWindow()->IsEntrySelected( sNonColorValue ) )
                    getTypedControlWindow()->SetNoSelection();
            }
        }
        else
            getTypedControlWindow()->SetNoSelection();
    }

    //------------------------------------------------------------------
    Any SAL_CALL OColorControl::getValue() throw (RuntimeException)
    {
        Any aPropValue;
        if ( getTypedControlWindow()->GetSelectEntryCount() > 0 )
        {
            OUString sSelectedEntry = getTypedControlWindow()->GetSelectEntry();
            if ( m_aNonColorEntries.find( sSelectedEntry ) != m_aNonColorEntries.end() )
                aPropValue <<= sSelectedEntry;
            else
            {
                ::Color aRgbCol = getTypedControlWindow()->GetSelectEntryColor();
                aPropValue <<= (::com::sun::star::util::Color)aRgbCol.GetColor();
            }
        }
        return aPropValue;
    }

    //------------------------------------------------------------------
    Type SAL_CALL OColorControl::getValueType() throw (RuntimeException)
    {
        return ::getCppuType( static_cast< sal_Int32* >( NULL ) );
    }

    //------------------------------------------------------------------
    void SAL_CALL OColorControl::clearList() throw (RuntimeException)
    {
        getTypedControlWindow()->Clear();
    }

    //------------------------------------------------------------------
    void SAL_CALL OColorControl::prependListEntry( const OUString& NewEntry ) throw (RuntimeException)
    {
        getTypedControlWindow()->InsertEntry( NewEntry, 0 );
        m_aNonColorEntries.insert( NewEntry );
    }

    //------------------------------------------------------------------
    void SAL_CALL OColorControl::appendListEntry( const OUString& NewEntry ) throw (RuntimeException)
    {
        getTypedControlWindow()->InsertEntry( NewEntry );
        m_aNonColorEntries.insert( NewEntry );
    }
    //------------------------------------------------------------------
    Sequence< OUString > SAL_CALL OColorControl::getListEntries(  ) throw (RuntimeException)
    {
        if ( !m_aNonColorEntries.empty() )
            return Sequence< OUString >(&(*m_aNonColorEntries.begin()),m_aNonColorEntries.size());
        return Sequence< OUString >();
    }

    //------------------------------------------------------------------
    void OColorControl::modified()
    {
        OColorControl_Base::modified();

        if ( !getTypedControlWindow()->IsTravelSelect() )
            // fire a commit
            m_aImplControl.notifyModifiedValue();
    }

    //==================================================================
    //= OListboxControl
    //==================================================================
    //------------------------------------------------------------------
    OListboxControl::OListboxControl( Window* pParent, WinBits nWinStyle)
        :OListboxControl_Base( PropertyControlType::ListBox, pParent, nWinStyle )
    {
        getTypedControlWindow()->SetDropDownLineCount( LB_DEFAULT_COUNT );
        if ( ( nWinStyle & WB_READONLY ) != 0 )
        {
            getTypedControlWindow()->SetReadOnly( sal_True );
            getTypedControlWindow()->Enable( sal_True );
        }
    }

    //------------------------------------------------------------------
    Any SAL_CALL OListboxControl::getValue() throw (RuntimeException)
    {
        OUString sControlValue( getTypedControlWindow()->GetSelectEntry() );

        Any aPropValue;
        if ( !sControlValue.isEmpty() )
            aPropValue <<= sControlValue;
        return aPropValue;
    }

    //------------------------------------------------------------------
    Type SAL_CALL OListboxControl::getValueType() throw (RuntimeException)
    {
        return ::getCppuType( static_cast< OUString* >( NULL ) );
    }

    //------------------------------------------------------------------
    void SAL_CALL OListboxControl::setValue( const Any& _rValue ) throw (IllegalTypeException, RuntimeException)
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

    //------------------------------------------------------------------
    void SAL_CALL OListboxControl::clearList() throw (RuntimeException)
    {
        getTypedControlWindow()->Clear();
    }

    //------------------------------------------------------------------
    void SAL_CALL OListboxControl::prependListEntry( const OUString& NewEntry ) throw (RuntimeException)
    {
        getTypedControlWindow()->InsertEntry( NewEntry, 0 );
    }

    //------------------------------------------------------------------
    void SAL_CALL OListboxControl::appendListEntry( const OUString& NewEntry ) throw (RuntimeException)
    {
        getTypedControlWindow()->InsertEntry( NewEntry );
    }
    //------------------------------------------------------------------
    Sequence< OUString > SAL_CALL OListboxControl::getListEntries(  ) throw (RuntimeException)
    {
        const sal_uInt16 nCount = getTypedControlWindow()->GetEntryCount();
        Sequence< OUString > aRet(nCount);
        OUString* pIter = aRet.getArray();
        for (sal_uInt16 i = 0; i < nCount ; ++i,++pIter)
            *pIter = getTypedControlWindow()->GetEntry(i);

        return aRet;
    }

    //------------------------------------------------------------------
    void OListboxControl::modified()
    {
        OListboxControl_Base::modified();

        if ( !getTypedControlWindow()->IsTravelSelect() )
            // fire a commit
            m_aImplControl.notifyModifiedValue();
    }

    //==================================================================
    //= OComboboxControl
    //==================================================================
    //------------------------------------------------------------------
    OComboboxControl::OComboboxControl( Window* pParent, WinBits nWinStyle)
        :OComboboxControl_Base( PropertyControlType::ComboBox, pParent, nWinStyle )
    {
        getTypedControlWindow()->SetDropDownLineCount( LB_DEFAULT_COUNT );
        getTypedControlWindow()->SetSelectHdl( LINK( this, OComboboxControl, OnEntrySelected ) );
    }

    //------------------------------------------------------------------
    void SAL_CALL OComboboxControl::setValue( const Any& _rValue ) throw (IllegalTypeException, RuntimeException)
    {
        OUString sText;
        _rValue >>= sText;
        getTypedControlWindow()->SetText( sText );
    }

    //------------------------------------------------------------------
    Any SAL_CALL OComboboxControl::getValue() throw (RuntimeException)
    {
        return makeAny( OUString( getTypedControlWindow()->GetText() ) );
    }

    //------------------------------------------------------------------
    Type SAL_CALL OComboboxControl::getValueType() throw (RuntimeException)
    {
        return ::getCppuType( static_cast< OUString* >( NULL ) );
    }

    //------------------------------------------------------------------
    void SAL_CALL OComboboxControl::clearList() throw (RuntimeException)
    {
        getTypedControlWindow()->Clear();
    }

    //------------------------------------------------------------------
    void SAL_CALL OComboboxControl::prependListEntry( const OUString& NewEntry ) throw (RuntimeException)
    {
        getTypedControlWindow()->InsertEntry( NewEntry, 0 );
    }

    //------------------------------------------------------------------
    void SAL_CALL OComboboxControl::appendListEntry( const OUString& NewEntry ) throw (RuntimeException)
    {
        getTypedControlWindow()->InsertEntry( NewEntry );
    }
    //------------------------------------------------------------------
    Sequence< OUString > SAL_CALL OComboboxControl::getListEntries(  ) throw (RuntimeException)
    {
        const sal_uInt16 nCount = getTypedControlWindow()->GetEntryCount();
        Sequence< OUString > aRet(nCount);
        OUString* pIter = aRet.getArray();
        for (sal_uInt16 i = 0; i < nCount ; ++i,++pIter)
            *pIter = getTypedControlWindow()->GetEntry(i);

        return aRet;
    }

    //------------------------------------------------------------------
    IMPL_LINK( OComboboxControl, OnEntrySelected, void*, /*_pNothing*/ )
    {
        if ( !getTypedControlWindow()->IsTravelSelect() )
            // fire a commit
            m_aImplControl.notifyModifiedValue();
        return 0L;
    }

    //==================================================================
    //= OMultilineFloatingEdit
    //==================================================================
    class OMultilineFloatingEdit : public FloatingWindow
    {
    private:
        MultiLineEdit   m_aImplEdit;

    protected:
        virtual void    Resize();

    public:
                        OMultilineFloatingEdit(Window* _pParen);
        MultiLineEdit*  getEdit() { return &m_aImplEdit; }

    protected:
        virtual long    PreNotify(NotifyEvent& _rNEvt);
    };

    //------------------------------------------------------------------
    OMultilineFloatingEdit::OMultilineFloatingEdit(Window* _pParent)
        :FloatingWindow(_pParent, WB_BORDER)
        ,m_aImplEdit(this, WB_VSCROLL|WB_IGNORETAB|WB_NOBORDER)
    {
        m_aImplEdit.Show();
    }

    //------------------------------------------------------------------
    void OMultilineFloatingEdit::Resize()
    {
        m_aImplEdit.SetSizePixel(GetOutputSizePixel());
    }

    //------------------------------------------------------------------
    long OMultilineFloatingEdit::PreNotify(NotifyEvent& _rNEvt)
    {
        long nResult = sal_True;

        sal_uInt16 nSwitch = _rNEvt.GetType();
        if (EVENT_KEYINPUT == nSwitch)
        {
            const KeyCode& aKeyCode = _rNEvt.GetKeyEvent()->GetKeyCode();
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
                nResult=FloatingWindow::PreNotify(_rNEvt);
        }
        else
            nResult=FloatingWindow::PreNotify(_rNEvt);

        return nResult;
    }

    //==================================================================
    //= DropDownEditControl_Base
    //==================================================================
    //------------------------------------------------------------------
    DropDownEditControl::DropDownEditControl( Window* _pParent, WinBits _nStyle )
        :DropDownEditControl_Base( _pParent, _nStyle )
        ,m_pFloatingEdit( NULL )
        ,m_pImplEdit( NULL )
        ,m_pDropdownButton( NULL )
        ,m_nOperationMode( eStringList )
        ,m_bDropdown( sal_False )
    {
        SetCompoundControl( sal_True );

        m_pImplEdit = new MultiLineEdit( this, WB_TABSTOP | WB_IGNORETAB | WB_NOBORDER | (_nStyle & WB_READONLY) );
        SetSubEdit( m_pImplEdit );
        m_pImplEdit->Show();

        if ( _nStyle & WB_DROPDOWN )
        {
            m_pDropdownButton = new PushButton( this, WB_NOLIGHTBORDER | WB_RECTSTYLE | WB_NOTABSTOP);
            m_pDropdownButton->SetSymbol(SYMBOL_SPIN_DOWN);
            m_pDropdownButton->SetClickHdl( LINK( this, DropDownEditControl, DropDownHdl ) );
            m_pDropdownButton->Show();
        }

        m_pFloatingEdit = new OMultilineFloatingEdit(this); //FloatingWindow

        m_pFloatingEdit->SetPopupModeEndHdl( LINK( this, DropDownEditControl, ReturnHdl ) );
        m_pFloatingEdit->getEdit()->SetReadOnly( ( _nStyle & WB_READONLY ) != 0 );
    }

    //------------------------------------------------------------------
    void DropDownEditControl::setControlHelper( ControlHelper& _rControlHelper )
    {
        DropDownEditControl_Base::setControlHelper( _rControlHelper );
        m_pFloatingEdit->getEdit()->SetModifyHdl( LINK( &_rControlHelper, ControlHelper, ModifiedHdl ) );
        m_pImplEdit->SetGetFocusHdl( LINK( &_rControlHelper, ControlHelper, GetFocusHdl ) );
        m_pImplEdit->SetModifyHdl( LINK( &_rControlHelper, ControlHelper, ModifiedHdl ) );
        m_pImplEdit->SetLoseFocusHdl( LINK( &_rControlHelper, ControlHelper, LoseFocusHdl ) );
    }

    //------------------------------------------------------------------
    DropDownEditControl::~DropDownEditControl()
    {
        {
            boost::scoped_ptr<Window> aTemp(m_pFloatingEdit);
            m_pFloatingEdit = NULL;
        }
        {
            boost::scoped_ptr<Window> aTemp(m_pImplEdit);
            SetSubEdit( NULL );
            m_pImplEdit = NULL;
        }
        {
            boost::scoped_ptr<Window> aTemp(m_pDropdownButton);
            m_pDropdownButton = NULL;
        }
    }

    //------------------------------------------------------------------
    void DropDownEditControl::Resize()
    {
        ::Size aOutSz = GetOutputSizePixel();

        if (m_pDropdownButton!=NULL)
        {
            long nSBWidth = GetSettings().GetStyleSettings().GetScrollBarSize();
            nSBWidth = CalcZoom( nSBWidth );
            m_pImplEdit->setPosSizePixel( 0, 1, aOutSz.Width() - nSBWidth, aOutSz.Height()-2 );
            m_pDropdownButton->setPosSizePixel( aOutSz.Width() - nSBWidth, 0, nSBWidth, aOutSz.Height() );
        }
        else
            m_pImplEdit->setPosSizePixel( 0, 1, aOutSz.Width(), aOutSz.Height()-2 );
    }

    //------------------------------------------------------------------
    long DropDownEditControl::PreNotify( NotifyEvent& rNEvt )
    {
        long nResult = 1;

        if (rNEvt.GetType() == EVENT_KEYINPUT)
        {
            const KeyCode& aKeyCode = rNEvt.GetKeyEvent()->GetKeyCode();
            sal_uInt16 nKey = aKeyCode.GetCode();

            if ( nKey == KEY_RETURN && !aKeyCode.IsShift() )
            {
                if ( m_pHelper )
                {
                    m_pHelper->LoseFocusHdl( m_pImplEdit );
                    m_pHelper->activateNextControl();
                }
            }
            else if ( nKey == KEY_DOWN && aKeyCode.IsMod2() )
            {
                Invalidate();
                ShowDropDown( sal_True );
            }
            else if (   KEYGROUP_CURSOR == aKeyCode.GetGroup()
                    ||  nKey == KEY_HELP
                    ||  KEYGROUP_FKEYS == aKeyCode.GetGroup()
                    ||  m_nOperationMode == eMultiLineText
                    )
            {
                nResult = DropDownEditControl_Base::PreNotify( rNEvt );
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
                ShowDropDown( sal_True );
                m_pFloatingEdit->getEdit()->GrabFocus();
                m_pFloatingEdit->getEdit()->SetSelection( aSel );
                Window* pFocusWin = Application::GetFocusWindow();
                pFocusWin->KeyInput( *rNEvt.GetKeyEvent() );
            }
        }
        else
            nResult = DropDownEditControl_Base::PreNotify(rNEvt);

        return nResult;
    }

    //------------------------------------------------------------------
    namespace
    {
        //..............................................................
        StlSyntaxSequence< OUString > lcl_convertMultiLineToList( const String& _rCompsedTextWithLineBreaks )
        {
            xub_StrLen nLines( comphelper::string::getTokenCount(_rCompsedTextWithLineBreaks, '\n') );
            StlSyntaxSequence< OUString > aStrings( nLines );
            StlSyntaxSequence< OUString >::iterator stringItem = aStrings.begin();
            for ( xub_StrLen token = 0; token < nLines; ++token, ++stringItem )
                *stringItem = _rCompsedTextWithLineBreaks.GetToken( token, '\n' );
            return aStrings;
        }

        String lcl_convertListToMultiLine( const StlSyntaxSequence< OUString >& _rStrings )
        {
            String sMultiLineText;
            for (   StlSyntaxSequence< OUString >::const_iterator item = _rStrings.begin();
                    item != _rStrings.end();
                )
            {
                sMultiLineText += String( *item );
                if ( ++item != _rStrings.end() )
                    sMultiLineText += '\n';
            }
            return sMultiLineText;
        }

        //..............................................................
        String lcl_convertListToDisplayText( const StlSyntaxSequence< OUString >& _rStrings )
        {
            OUStringBuffer aComposed;
            for (   StlSyntaxSequence< OUString >::const_iterator strings = _rStrings.begin();
                    strings != _rStrings.end();
                    ++strings
                )
            {
                if ( strings != _rStrings.begin() )
                    aComposed.append( (sal_Unicode)';' );
                aComposed.append( (sal_Unicode)'\"' );
                aComposed.append( *strings );
                aComposed.append( (sal_Unicode)'\"' );
            }
            return aComposed.makeStringAndClear();
        }
    }

    //------------------------------------------------------------------
    #define STD_HEIGHT  100
    sal_Bool DropDownEditControl::ShowDropDown( sal_Bool bShow )
    {
        if (bShow)
        {
            ::Point aMePos= GetPosPixel();
            aMePos = GetParent()->OutputToScreenPixel( aMePos );
            ::Size aSize=GetSizePixel();
            ::Rectangle aRect(aMePos,aSize);
            aSize.Height() = STD_HEIGHT;
            m_pFloatingEdit->SetOutputSizePixel(aSize);
            m_pFloatingEdit->StartPopupMode( aRect, FLOATWIN_POPUPMODE_DOWN );

            m_pFloatingEdit->Show();
            m_pFloatingEdit->getEdit()->GrabFocus();
            m_pFloatingEdit->getEdit()->SetSelection(Selection(m_pFloatingEdit->getEdit()->GetText().getLength()));
            m_bDropdown=sal_True;
            if ( m_nOperationMode == eMultiLineText )
                m_pFloatingEdit->getEdit()->SetText( m_pImplEdit->GetText() );
            m_pImplEdit->SetText(String());
        }
        else
        {
            m_pFloatingEdit->Hide();
            m_pFloatingEdit->Invalidate();
            m_pFloatingEdit->Update();

            // transfer the text from the floating edit to our own edit
            String sDisplayText( m_pFloatingEdit->getEdit()->GetText() );
            if ( m_nOperationMode == eStringList )
                sDisplayText = lcl_convertListToDisplayText( lcl_convertMultiLineToList( sDisplayText ) );

            m_pImplEdit->SetText( sDisplayText );
            GetParent()->Invalidate( INVALIDATE_CHILDREN );
            m_bDropdown = sal_False;
            m_pImplEdit->GrabFocus();
        }
        return m_bDropdown;

    }

    //------------------------------------------------------------------
    long DropDownEditControl::FindPos(long nSinglePos)
    {
        long nPos=0;
        String aOutput;
        String aStr=m_pFloatingEdit->getEdit()->GetText();
        String aStr1 = GetText();

        if ((nSinglePos == 0) || (nSinglePos == aStr1.Len()))
        {
            return nSinglePos;
        }

        if (aStr.Len()>0)
        {
            long nDiff=0;
            sal_Int32 nCount = comphelper::string::getTokenCount(aStr, '\n');

            String aInput = aStr.GetToken(0,'\n' );

            if (aInput.Len()>0)
            {
                aOutput+='\"';
                nDiff++;
                aOutput+=aInput;
                aOutput+='\"';
            }

            if (nSinglePos <= aOutput.Len())
            {
                nPos=nSinglePos-nDiff;
            }
            else
            {
                for (sal_Int32 i=1; i<nCount; ++i)
                {
                    aInput=aStr.GetToken((sal_uInt16)i, '\n');
                    if (aInput.Len()>0)
                    {
                        aOutput += ';';
                        aOutput += '\"';
                        nDiff += 2;
                        aOutput += aInput;
                        aOutput += '\"';

                        if (nSinglePos <= aOutput.Len())
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

    //------------------------------------------------------------------
    IMPL_LINK( DropDownEditControl, ReturnHdl, OMultilineFloatingEdit*, /*pMEd*/)
    {

        String aStr = m_pFloatingEdit->getEdit()->GetText();
        String aStr2 = GetText();
        ShowDropDown(sal_False);

        if (aStr!=aStr2 || ( m_nOperationMode == eStringList ) )
        {
            if ( m_pHelper )
                m_pHelper->notifyModifiedValue();
        }

        return 0;
    }

    //------------------------------------------------------------------
    IMPL_LINK( DropDownEditControl, DropDownHdl, PushButton*, /*pPb*/ )
    {
        ShowDropDown(!m_bDropdown);
        return 0;
    }

    //------------------------------------------------------------------
    void DropDownEditControl::SetStringListValue( const StlSyntaxSequence< OUString >& _rStrings )
    {
        SetText( lcl_convertListToDisplayText( _rStrings ) );
        m_pFloatingEdit->getEdit()->SetText( lcl_convertListToMultiLine( _rStrings ) );
    }

    //------------------------------------------------------------------
    StlSyntaxSequence< OUString > DropDownEditControl::GetStringListValue() const
    {
        return lcl_convertMultiLineToList( m_pFloatingEdit->getEdit()->GetText() );
    }

    //------------------------------------------------------------------
    void DropDownEditControl::SetTextValue( const OUString& _rText )
    {
        OSL_PRECOND( m_nOperationMode == eMultiLineText, "DropDownEditControl::SetTextValue: illegal call!" );

        m_pFloatingEdit->getEdit()->SetText( _rText );
        SetText( _rText );
    }

    //------------------------------------------------------------------
    OUString DropDownEditControl::GetTextValue() const
    {
        OSL_PRECOND( m_nOperationMode == eMultiLineText, "DropDownEditControl::GetTextValue: illegal call!" );
        return GetText();
    }

    //==================================================================
    //= OMultilineEditControl
    //==================================================================
    //------------------------------------------------------------------
    OMultilineEditControl::OMultilineEditControl( Window* pParent, MultiLineOperationMode _eMode, WinBits nWinStyle )
        :OMultilineEditControl_Base( _eMode == eMultiLineText ? PropertyControlType::MultiLineTextField : PropertyControlType::StringListField
                                   , pParent
                                   , ( nWinStyle | WB_DIALOGCONTROL ) & ( ~WB_READONLY | ~WB_DROPDOWN )
                                   , false )
    {
        getTypedControlWindow()->setOperationMode( _eMode );
    }

    //------------------------------------------------------------------
    void SAL_CALL OMultilineEditControl::setValue( const Any& _rValue ) throw (IllegalTypeException, RuntimeException)
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
            getTypedControlWindow()->SetStringListValue( aStringLines );
        }
        break;
        }
    }

    //------------------------------------------------------------------
    Any SAL_CALL OMultilineEditControl::getValue() throw (RuntimeException)
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

    //------------------------------------------------------------------
    Type SAL_CALL OMultilineEditControl::getValueType() throw (RuntimeException)
    {
        if ( getTypedControlWindow()->getOperationMode() == eMultiLineText )
            return ::getCppuType( static_cast< OUString* >( NULL ) );
        return ::getCppuType( static_cast< Sequence< OUString >* >( NULL ) );
    }

//............................................................................
} // namespace pcr
//............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
