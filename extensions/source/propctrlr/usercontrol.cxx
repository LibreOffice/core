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

#include "usercontrol.hxx"

#include <com/sun/star/inspection/PropertyControlType.hpp>
#include <svl/numuno.hxx>
#include <rtl/math.hxx>
#include <tools/debug.hxx>
#include <svl/zformat.hxx>
#include <connectivity/dbconversion.hxx>
#include <com/sun/star/util/Time.hpp>
#include "modulepcr.hxx"
#include "propresid.hrc"


namespace pcr
{


    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::uno::RuntimeException;

    namespace PropertyControlType = ::com::sun::star::inspection::PropertyControlType;


    // NumberFormatSampleField


    bool NumberFormatSampleField::PreNotify( NotifyEvent& rNEvt )
    {
        // want to handle two keys myself : Del/Backspace should empty the window (setting my prop to "standard" this way)
        if (MouseNotifyEvent::KEYINPUT == rNEvt.GetType())
        {
            sal_uInt16 nKey = rNEvt.GetKeyEvent()->GetKeyCode().GetCode();

            if ((KEY_DELETE == nKey) || (KEY_BACKSPACE == nKey))
            {
                SetText( "" );
                if ( m_pHelper )
                    m_pHelper->setModified();
                return true;
            }
        }

        return FormattedField::PreNotify( rNEvt );
    }


    void NumberFormatSampleField::SetFormatSupplier( const SvNumberFormatsSupplierObj* pSupplier )
    {
        if ( pSupplier )
        {
            TreatAsNumber( true );

            SvNumberFormatter* pFormatter = pSupplier->GetNumberFormatter();
            SetFormatter( pFormatter );
            SetValue( 1234.56789 );
        }
        else
        {
            TreatAsNumber( false );
            SetFormatter( nullptr );
            SetText( "" );
        }
    }


    // OFormatSampleControl


    OFormatSampleControl::OFormatSampleControl( vcl::Window* pParent )
        :OFormatSampleControl_Base( PropertyControlType::Unknown, pParent, WB_READONLY | WB_TABSTOP | WB_BORDER )
    {
        getTypedControlWindow()->setControlHelper(*this);
    }


    void SAL_CALL OFormatSampleControl::setValue( const Any& _rValue )
    {
        sal_Int32 nFormatKey = 0;
        if ( _rValue >>= nFormatKey )
        {
            // else set the new format key, the text will be reformatted
            getTypedControlWindow()->SetFormatKey( nFormatKey );

            SvNumberFormatter* pNF = getTypedControlWindow()->GetFormatter();
            const SvNumberformat* pEntry = pNF->GetEntry( nFormatKey );
            OSL_ENSURE( pEntry, "OFormatSampleControl::setValue: invalid format entry!" );

            const bool bIsTextFormat = ( pEntry && pEntry->IsTextFormat() );
            if ( bIsTextFormat )
                getTypedControlWindow()->SetText( PcrRes( RID_STR_TEXT_FORMAT ).toString() );
            else
                getTypedControlWindow()->SetValue( pEntry ? getPreviewValue( *pEntry ) : 1234.56789 );
        }
        else
            getTypedControlWindow()->SetText( "" );
    }

    double OFormatSampleControl::getPreviewValue( const SvNumberformat& i_rEntry )
    {
        double nValue = 1234.56789;
        switch ( i_rEntry.GetType() & ~css::util::NumberFormat::DEFINED )
        {
            case css::util::NumberFormat::DATE:
                {
                    Date aCurrentDate( Date::SYSTEM );
                    static css::util::Date STANDARD_DB_DATE(30,12,1899);
                    nValue = ::dbtools::DBTypeConversion::toDouble(::dbtools::DBTypeConversion::toDate(static_cast<sal_Int32>(aCurrentDate.GetDate())),STANDARD_DB_DATE);
                }
                break;
            case css::util::NumberFormat::TIME:
            case css::util::NumberFormat::DATETIME:
                {
                    tools::Time aCurrentTime( tools::Time::SYSTEM );
                    nValue = ::dbtools::DBTypeConversion::toDouble(::dbtools::DBTypeConversion::toTime(aCurrentTime.GetTime()));
                }
                break;
            default:
                break;
        }
        return nValue;
    }


    double OFormatSampleControl::getPreviewValue(SvNumberFormatter* _pNF,sal_Int32 _nFormatKey)
    {
        const SvNumberformat* pEntry = _pNF->GetEntry(_nFormatKey);
        DBG_ASSERT( pEntry, "OFormattedNumericControl::SetFormatDescription: invalid format key!" );
        double nValue = 1234.56789;
        if ( pEntry )
            nValue = getPreviewValue( *pEntry );
        return nValue;
    }

    Any SAL_CALL OFormatSampleControl::getValue()
    {
        Any aPropValue;
        if ( !getTypedControlWindow()->GetText().isEmpty() )
            aPropValue <<= (sal_Int32)getTypedControlWindow()->GetFormatKey();
        return aPropValue;
    }


    Type SAL_CALL OFormatSampleControl::getValueType()
    {
        return ::cppu::UnoType<sal_Int32>::get();
    }


    // class OFormattedNumericControl


    OFormattedNumericControl::OFormattedNumericControl( vcl::Window* pParent, WinBits nWinStyle )
        :OFormattedNumericControl_Base( PropertyControlType::Unknown, pParent, nWinStyle )
    {
        getTypedControlWindow()->TreatAsNumber(true);

        m_nLastDecimalDigits = getTypedControlWindow()->GetDecimalDigits();
    }


    OFormattedNumericControl::~OFormattedNumericControl()
    {
    }


    void SAL_CALL OFormattedNumericControl::setValue( const Any& _rValue )
    {
        double nValue( 0 );
        if ( _rValue >>= nValue )
            getTypedControlWindow()->SetValue( nValue );
        else
            getTypedControlWindow()->SetText("");
    }


    Any SAL_CALL OFormattedNumericControl::getValue()
    {
        Any aPropValue;
        if ( !getTypedControlWindow()->GetText().isEmpty() )
            aPropValue <<= (double)getTypedControlWindow()->GetValue();
        return aPropValue;
    }


    Type SAL_CALL OFormattedNumericControl::getValueType()
    {
        return ::cppu::UnoType<double>::get();
    }


    void OFormattedNumericControl::SetFormatDescription(const FormatDescription& rDesc)
    {
        bool bFallback = true;

        if (rDesc.pSupplier)
        {
            getTypedControlWindow()->TreatAsNumber(true);

            SvNumberFormatter* pFormatter = rDesc.pSupplier->GetNumberFormatter();
            if (pFormatter != getTypedControlWindow()->GetFormatter())
                getTypedControlWindow()->SetFormatter(pFormatter);
            getTypedControlWindow()->SetFormatKey(rDesc.nKey);

            const SvNumberformat* pEntry = getTypedControlWindow()->GetFormatter()->GetEntry(getTypedControlWindow()->GetFormatKey());
            DBG_ASSERT( pEntry, "OFormattedNumericControl::SetFormatDescription: invalid format key!" );
            if ( pEntry )
            {
                switch (pEntry->GetType() & ~css::util::NumberFormat::DEFINED)
                {
                    case css::util::NumberFormat::NUMBER:
                    case css::util::NumberFormat::CURRENCY:
                    case css::util::NumberFormat::SCIENTIFIC:
                    case css::util::NumberFormat::FRACTION:
                    case css::util::NumberFormat::PERCENT:
                        m_nLastDecimalDigits = getTypedControlWindow()->GetDecimalDigits();
                        break;
                    case css::util::NumberFormat::DATETIME:
                    case css::util::NumberFormat::DATE:
                    case css::util::NumberFormat::TIME:
                        m_nLastDecimalDigits = 7;
                        break;
                    default:
                        m_nLastDecimalDigits = 0;
                        break;
                }
                bFallback = false;
            }

        }

        if ( bFallback )
        {
            getTypedControlWindow()->TreatAsNumber(false);
            getTypedControlWindow()->SetFormatter(nullptr);
            getTypedControlWindow()->SetText("");
            m_nLastDecimalDigits = 0;
        }
    }


    //= OFileUrlControl


    OFileUrlControl::OFileUrlControl( vcl::Window* pParent )
        :OFileUrlControl_Base( PropertyControlType::Unknown, pParent, WB_TABSTOP | WB_BORDER | WB_DROPDOWN )
    {
        getTypedControlWindow()->SetDropDownLineCount( 10 );
        getTypedControlWindow()->SetPlaceHolder( PcrRes( RID_EMBED_IMAGE_PLACEHOLDER ).toString() ) ;
    }


    OFileUrlControl::~OFileUrlControl()
    {
    }


    void SAL_CALL OFileUrlControl::setValue( const Any& _rValue )
    {
        OUString sURL;
        if ( ( _rValue >>= sURL ) )
        {
            if ( sURL.startsWith( "vnd.sun.star.GraphicObject:" ) )
                getTypedControlWindow()->DisplayURL( getTypedControlWindow()->GetPlaceHolder() );
            else
                getTypedControlWindow()->DisplayURL( sURL );
        }
        else
            getTypedControlWindow()->SetText( "" );
    }


    Any SAL_CALL OFileUrlControl::getValue()
    {
        Any aPropValue;
        if ( !getTypedControlWindow()->GetText().isEmpty() )
                aPropValue <<= getTypedControlWindow()->GetURL();
        return aPropValue;
    }


    Type SAL_CALL OFileUrlControl::getValueType()
    {
        return ::cppu::UnoType<OUString>::get();
    }


    //= OTimeDurationControl


    OTimeDurationControl::OTimeDurationControl( vcl::Window* pParent )
        :ONumericControl( pParent, WB_BORDER | WB_TABSTOP )
    {
        getTypedControlWindow()->SetUnit( FUNIT_CUSTOM );
        getTypedControlWindow()->SetCustomUnitText(" ms");
        getTypedControlWindow()->SetCustomConvertHdl( LINK( this, OTimeDurationControl, OnCustomConvert ) );
    }


    OTimeDurationControl::~OTimeDurationControl()
    {
    }


    ::sal_Int16 SAL_CALL OTimeDurationControl::getControlType()
    {
        // don't use the base class'es method, it would claim we're a standard control, which
        // we in fact aren't
        return PropertyControlType::Unknown;
    }


    IMPL_LINK_NOARG( OTimeDurationControl, OnCustomConvert, MetricFormatter&, void )
    {
        long nMultiplier = 1;
        if ( getTypedControlWindow()->GetCurUnitText().equalsIgnoreAsciiCase( "ms" ) )
            nMultiplier = 1;
        if ( getTypedControlWindow()->GetCurUnitText().equalsIgnoreAsciiCase( "s" ) )
            nMultiplier = 1000;
        else if ( getTypedControlWindow()->GetCurUnitText().equalsIgnoreAsciiCase( "m" ) )
            nMultiplier = 1000 * 60;
        else if ( getTypedControlWindow()->GetCurUnitText().equalsIgnoreAsciiCase( "h" ) )
            nMultiplier = 1000 * 60 * 60;

        getTypedControlWindow()->SetValue( getTypedControlWindow()->GetLastValue() * nMultiplier );
    }


} // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
