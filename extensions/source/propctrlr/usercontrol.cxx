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
#include <vcl/GraphicObject.hxx>
#include <vcl/event.hxx>
#include <tools/debug.hxx>
#include <svl/zformat.hxx>
#include <connectivity/dbconversion.hxx>
#include <com/sun/star/util/Time.hpp>
#include "modulepcr.hxx"
#include <strings.hrc>


namespace pcr
{


    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Type;

    namespace PropertyControlType = ::com::sun::star::inspection::PropertyControlType;

    IMPL_LINK(OFormatSampleControl, KeyInputHdl, const KeyEvent&, rKeyEvent, bool)
    {
        // want to handle two keys myself : Del/Backspace should empty the window (setting my prop to "standard" this way)
        sal_uInt16 nKey = rKeyEvent.GetKeyCode().GetCode();
        if ((KEY_DELETE == nKey) || (KEY_BACKSPACE == nKey))
        {
            m_xSpinButton->set_text("");
            m_xEntry->set_text("");
            setModified();
        }

        return true;
    }

    void OFormatSampleControl::SetFormatSupplier( const SvNumberFormatsSupplierObj* pSupplier )
    {
        if (pSupplier)
        {
            m_xSpinButton->treat_as_number(true);

            SvNumberFormatter* pFormatter = pSupplier->GetNumberFormatter();
            m_xSpinButton->set_formatter(pFormatter);
            m_xSpinButton->set_value( 1234.56789 );
        }
        else
        {
            m_xSpinButton->treat_as_number(false);
            m_xSpinButton->set_formatter(nullptr);
            m_xSpinButton->set_text( "" );
        }

        m_xEntry->set_text(m_xSpinButton->get_text());
    }

    OFormatSampleControl::OFormatSampleControl(std::unique_ptr<weld::Container> xWidget, std::unique_ptr<weld::Builder> xBuilder, bool bReadOnly)
        : OFormatSampleControl_Base(PropertyControlType::Unknown, std::move(xBuilder), std::move(xWidget), bReadOnly)
        , m_xSpinButton(m_xBuilder->weld_formatted_spin_button("sample"))
        , m_xEntry(m_xBuilder->weld_entry("entry"))
    {
        m_xSpinButton->treat_as_number(true);
        m_xEntry->connect_key_press(LINK(this, OFormatSampleControl, KeyInputHdl));
    }

    void SAL_CALL OFormatSampleControl::setValue( const Any& _rValue )
    {
        sal_Int32 nFormatKey = 0;
        if ( _rValue >>= nFormatKey )
        {
            // else set the new format key, the text will be reformatted
            m_xSpinButton->set_format_key( nFormatKey );

            SvNumberFormatter* pNF = m_xSpinButton->get_formatter();
            const SvNumberformat* pEntry = pNF->GetEntry( nFormatKey );
            OSL_ENSURE( pEntry, "OFormatSampleControl::setValue: invalid format entry!" );

            const bool bIsTextFormat = ( pEntry && pEntry->IsTextFormat() );
            if ( bIsTextFormat )
                m_xSpinButton->set_text( PcrRes( RID_STR_TEXT_FORMAT ) );
            else
                m_xSpinButton->set_value( pEntry ? getPreviewValue( *pEntry ) : 1234.56789 );
        }
        else
            m_xSpinButton->set_text( "" );

        m_xEntry->set_text(m_xSpinButton->get_text());
    }

    double OFormatSampleControl::getPreviewValue( const SvNumberformat& i_rEntry )
    {
        double nValue = 1234.56789;
        switch ( i_rEntry.GetType() & ~SvNumFormatType::DEFINED )
        {
            case SvNumFormatType::DATE:
                {
                    Date aCurrentDate( Date::SYSTEM );
                    static css::util::Date STANDARD_DB_DATE(30,12,1899);
                    nValue = ::dbtools::DBTypeConversion::toDouble(::dbtools::DBTypeConversion::toDate(aCurrentDate.GetDate()),STANDARD_DB_DATE);
                }
                break;
            case SvNumFormatType::TIME:
            case SvNumFormatType::DATETIME:
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


    double OFormatSampleControl::getPreviewValue(SvNumberFormatter const * _pNF, sal_Int32 _nFormatKey)
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
        if ( !m_xSpinButton->get_text().isEmpty() )
            aPropValue <<= m_xSpinButton->get_value();
        return aPropValue;
    }

    Type SAL_CALL OFormatSampleControl::getValueType()
    {
        return ::cppu::UnoType<sal_Int32>::get();
    }

    OFormattedNumericControl::OFormattedNumericControl(std::unique_ptr<weld::FormattedSpinButton> xWidget, std::unique_ptr<weld::Builder> xBuilder, bool bReadOnly)
        : OFormattedNumericControl_Base(PropertyControlType::Unknown, std::move(xBuilder), std::move(xWidget), bReadOnly)
    {
        getTypedControlWindow()->treat_as_number(true);
    }

    OFormattedNumericControl::~OFormattedNumericControl()
    {
    }

    void SAL_CALL OFormattedNumericControl::setValue( const Any& _rValue )
    {
        double nValue( 0 );
        if ( _rValue >>= nValue )
            getTypedControlWindow()->set_value( nValue );
        else
            getTypedControlWindow()->set_text("");
    }

    Any SAL_CALL OFormattedNumericControl::getValue()
    {
        Any aPropValue;
        if ( !getTypedControlWindow()->get_text().isEmpty() )
            aPropValue <<= getTypedControlWindow()->get_value();
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
            getTypedControlWindow()->treat_as_number(true);

            SvNumberFormatter* pFormatter = rDesc.pSupplier->GetNumberFormatter();
            if (pFormatter != getTypedControlWindow()->get_formatter())
                getTypedControlWindow()->set_formatter(pFormatter);
            getTypedControlWindow()->set_format_key(rDesc.nKey);

            const SvNumberformat* pEntry = getTypedControlWindow()->get_formatter()->GetEntry(getTypedControlWindow()->get_format_key());
            DBG_ASSERT( pEntry, "OFormattedNumericControl::SetFormatDescription: invalid format key!" );
            if ( pEntry )
            {
                bFallback = false;
            }

        }

        if ( bFallback )
        {
            getTypedControlWindow()->treat_as_number(false);
            getTypedControlWindow()->set_formatter(nullptr);
            getTypedControlWindow()->set_text("");
        }
    }

    //= OFileUrlControl
    OFileUrlControl::OFileUrlControl(std::unique_ptr<SvtURLBox> xWidget, std::unique_ptr<weld::Builder> xBuilder, bool bReadOnly)
        : OFileUrlControl_Base(PropertyControlType::Unknown, std::move(xBuilder), std::move(xWidget), bReadOnly)
    {
        getTypedControlWindow()->DisableHistory();
        getTypedControlWindow()->SetPlaceHolder( PcrRes( RID_EMBED_IMAGE_PLACEHOLDER ) ) ;
    }

    OFileUrlControl::~OFileUrlControl()
    {
    }

    void SAL_CALL OFileUrlControl::setValue( const Any& _rValue )
    {
        OUString sURL;
        if (  _rValue >>= sURL )
        {
            if (GraphicObject::isGraphicObjectUniqueIdURL(sURL))
                getTypedControlWindow()->set_entry_text(getTypedControlWindow()->GetPlaceHolder());
            else
                getTypedControlWindow()->set_entry_text(sURL);
        }
        else
            getTypedControlWindow()->set_entry_text( "" );
    }

    Any SAL_CALL OFileUrlControl::getValue()
    {
        Any aPropValue;
        if (!getTypedControlWindow()->get_active_text().isEmpty())
            aPropValue <<= getTypedControlWindow()->GetURL();
        return aPropValue;
    }

    Type SAL_CALL OFileUrlControl::getValueType()
    {
        return ::cppu::UnoType<OUString>::get();
    }

} // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
