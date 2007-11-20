/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: usercontrol.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-20 19:53:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#ifndef _EXTENSIONS_PROPCTRLR_USERCONTROL_HXX_
#include "usercontrol.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_INSPECTION_PROPERTYCONTROLTYPE_HPP_
#include <com/sun/star/inspection/PropertyControlType.hpp>
#endif
#ifndef _COM_SUN_STAR_INSPECTION_PROPERTYCONTROLTYPE_HPP_
#include <com/sun/star/inspection/PropertyControlType.hpp>
#endif
/** === end UNO includes === **/

#ifndef _NUMUNO_HXX
#include <svtools/numuno.hxx>
#endif
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _ZFORMAT_HXX
#include <svtools/zformat.hxx>
#endif
#include <connectivity/dbconversion.hxx>
#include <com/sun/star/util/Time.hpp>

//............................................................................
namespace pcr
{
//............................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::beans::IllegalTypeException;
    using ::com::sun::star::uno::RuntimeException;
    /** === end UNO using === **/
    namespace PropertyControlType = ::com::sun::star::inspection::PropertyControlType;

    //==================================================================
    // NumberFormatSampleField
    //==================================================================
    //------------------------------------------------------------------
    long NumberFormatSampleField::PreNotify( NotifyEvent& rNEvt )
    {
        // want to handle two keys myself : Del/Backspace should empty the window (setting my prop to "standard" this way)
        if (EVENT_KEYINPUT == rNEvt.GetType())
        {
            sal_uInt16 nKey = rNEvt.GetKeyEvent()->GetKeyCode().GetCode();

            if ((KEY_DELETE == nKey) || (KEY_BACKSPACE == nKey))
            {
                SetText( String() );
                if ( m_pHelper )
                    m_pHelper->ModifiedHdl( this );
                return 1;
            }
        }

        return BaseClass::PreNotify( rNEvt );
    }

    //------------------------------------------------------------------
    void NumberFormatSampleField::SetFormatSupplier( const SvNumberFormatsSupplierObj* pSupplier )
    {
        if ( pSupplier )
        {
            TreatAsNumber( sal_True );

            SvNumberFormatter* pFormatter = pSupplier->GetNumberFormatter();
            SetFormatter( pFormatter, sal_True );
            SetValue( 1234.56789 );
        }
        else
        {
            TreatAsNumber( sal_False );
            SetFormatter( NULL, sal_True );
            SetText( String() );
        }
    }

    //==================================================================
    // OFormatSampleControl
    //==================================================================
    //------------------------------------------------------------------
    OFormatSampleControl::OFormatSampleControl( Window* pParent, WinBits nWinStyle )
        :OFormatSampleControl_Base( PropertyControlType::Unknown, pParent, nWinStyle )
    {
    }

    //------------------------------------------------------------------
    void SAL_CALL OFormatSampleControl::setValue( const Any& _rValue ) throw (IllegalTypeException, RuntimeException)
    {
        sal_Int32 nFormatKey = 0;
        if ( _rValue >>= nFormatKey )
        {
            // else set the new format key, the text will be reformatted
            getTypedControlWindow()->SetFormatKey( nFormatKey );

            SvNumberFormatter* pNF = getTypedControlWindow()->GetFormatter();
            getTypedControlWindow()->SetValue( getPreviewValue(pNF,getTypedControlWindow()->GetFormatKey()) );
        }
        else
            getTypedControlWindow()->SetText( String() );
    }
    //------------------------------------------------------------------
    double OFormatSampleControl::getPreviewValue(SvNumberFormatter* _pNF,sal_Int32 _nFormatKey)
    {
        const SvNumberformat* pEntry = _pNF->GetEntry(_nFormatKey);
        DBG_ASSERT( pEntry, "OFormattedNumericControl::SetFormatDescription: invalid format key!" );
        double nValue = 1234.56789;
        if ( pEntry )
        {
            switch (pEntry->GetType() & ~NUMBERFORMAT_DEFINED)
            {
                case NUMBERFORMAT_DATE:
                    {
                        Date aCurrentDate;
                        static ::com::sun::star::util::Date STANDARD_DB_DATE(30,12,1899);
                        nValue = ::dbtools::DBTypeConversion::toDouble(::dbtools::DBTypeConversion::toDate(static_cast<sal_Int32>(aCurrentDate.GetDate())),STANDARD_DB_DATE);
                    }
                    break;
                case NUMBERFORMAT_TIME:
                case NUMBERFORMAT_DATETIME:
                    {
                        Time aCurrentTime;
                        nValue = ::dbtools::DBTypeConversion::toDouble(::dbtools::DBTypeConversion::toTime(aCurrentTime.GetTime()));
                    }
                    break;
                default:
                    break;
            }
        }
        return nValue;
    }
    //------------------------------------------------------------------
    Any SAL_CALL OFormatSampleControl::getValue() throw (RuntimeException)
    {
        Any aPropValue;
        if ( getTypedControlWindow()->GetText().Len() )
            aPropValue <<= (sal_Int32)getTypedControlWindow()->GetFormatKey();
        return aPropValue;
    }

    //------------------------------------------------------------------
    Type SAL_CALL OFormatSampleControl::getValueType() throw (RuntimeException)
    {
        return ::getCppuType( static_cast< sal_Int32* >( NULL ) );
    }

    //==================================================================
    // class OFormattedNumericControl
    //==================================================================
    DBG_NAME(OFormattedNumericControl);
    //------------------------------------------------------------------
    OFormattedNumericControl::OFormattedNumericControl( Window* pParent, WinBits nWinStyle )
        :OFormattedNumericControl_Base( PropertyControlType::Unknown, pParent, nWinStyle )
    {
        DBG_CTOR(OFormattedNumericControl,NULL);

        getTypedControlWindow()->TreatAsNumber(sal_True);

        m_nLastDecimalDigits = getTypedControlWindow()->GetDecimalDigits();
    }

    //------------------------------------------------------------------
    OFormattedNumericControl::~OFormattedNumericControl()
    {
        DBG_DTOR(OFormattedNumericControl,NULL);
    }

    //------------------------------------------------------------------
    void SAL_CALL OFormattedNumericControl::setValue( const Any& _rValue ) throw (IllegalTypeException, RuntimeException)
    {
        double nValue( 0 );
        if ( _rValue >>= nValue )
            getTypedControlWindow()->SetValue( nValue );
        else
            getTypedControlWindow()->SetText(String());
    }

    //------------------------------------------------------------------
    Any SAL_CALL OFormattedNumericControl::getValue() throw (RuntimeException)
    {
        Any aPropValue;
        if ( getTypedControlWindow()->GetText().Len() )
            aPropValue <<= (double)getTypedControlWindow()->GetValue();
        return aPropValue;
    }

    //------------------------------------------------------------------
    Type SAL_CALL OFormattedNumericControl::getValueType() throw (RuntimeException)
    {
        return ::getCppuType( static_cast< double* >( NULL ) );
    }

    //------------------------------------------------------------------
    void OFormattedNumericControl::SetFormatDescription(const FormatDescription& rDesc)
    {
        sal_Bool bFallback = sal_True;

        if (rDesc.pSupplier)
        {
            getTypedControlWindow()->TreatAsNumber(sal_True);

            SvNumberFormatter* pFormatter = rDesc.pSupplier->GetNumberFormatter();
            if (pFormatter != getTypedControlWindow()->GetFormatter())
                getTypedControlWindow()->SetFormatter(pFormatter, sal_True);
            getTypedControlWindow()->SetFormatKey(rDesc.nKey);

            const SvNumberformat* pEntry = getTypedControlWindow()->GetFormatter()->GetEntry(getTypedControlWindow()->GetFormatKey());
            DBG_ASSERT( pEntry, "OFormattedNumericControl::SetFormatDescription: invalid format key!" );
            if ( pEntry )
            {
                switch (pEntry->GetType() & ~NUMBERFORMAT_DEFINED)
                {
                    case NUMBERFORMAT_NUMBER:
                    case NUMBERFORMAT_CURRENCY:
                    case NUMBERFORMAT_SCIENTIFIC:
                    case NUMBERFORMAT_FRACTION:
                    case NUMBERFORMAT_PERCENT:
                        m_nLastDecimalDigits = getTypedControlWindow()->GetDecimalDigits();
                        break;
                    case NUMBERFORMAT_DATETIME:
                    case NUMBERFORMAT_DATE:
                    case NUMBERFORMAT_TIME:
                        m_nLastDecimalDigits = 7;
                        break;
                    default:
                        m_nLastDecimalDigits = 0;
                        break;
                }
                bFallback = sal_False;
            }

        }

        if ( bFallback )
        {
            getTypedControlWindow()->TreatAsNumber(sal_False);
            getTypedControlWindow()->SetFormatter(NULL, sal_True);
            getTypedControlWindow()->SetText(String());
            m_nLastDecimalDigits = 0;
        }
    }

    //========================================================================
    //= OFileUrlControl
    //========================================================================
    //------------------------------------------------------------------
    OFileUrlControl::OFileUrlControl( Window* pParent, WinBits nWinStyle )
        :OFileUrlControl_Base( PropertyControlType::Unknown, pParent, nWinStyle | WB_DROPDOWN )
    {
        getTypedControlWindow()->SetDropDownLineCount( 10 );
    }

    //------------------------------------------------------------------
    OFileUrlControl::~OFileUrlControl()
    {
    }

    //------------------------------------------------------------------
    void SAL_CALL OFileUrlControl::setValue( const Any& _rValue ) throw (IllegalTypeException, RuntimeException)
    {
        ::rtl::OUString sURL;
        if ( _rValue >>= sURL )
            getTypedControlWindow()->DisplayURL( sURL );
        else
            getTypedControlWindow()->SetText( String() );
    }

    //------------------------------------------------------------------
    Any SAL_CALL OFileUrlControl::getValue() throw (RuntimeException)
    {
        Any aPropValue;
        if ( getTypedControlWindow()->GetText().Len() )
            aPropValue <<= (::rtl::OUString)getTypedControlWindow()->GetURL();
        return aPropValue;
    }

    //------------------------------------------------------------------
    Type SAL_CALL OFileUrlControl::getValueType() throw (RuntimeException)
    {
        return ::getCppuType( static_cast< ::rtl::OUString* >( NULL ) );
    }

    //========================================================================
    //= OTimeDurationControl
    //========================================================================
    //------------------------------------------------------------------
    OTimeDurationControl::OTimeDurationControl( ::Window* pParent, WinBits nWinStyle )
        :ONumericControl( pParent, nWinStyle )
    {
        getTypedControlWindow()->SetUnit( FUNIT_CUSTOM );
        getTypedControlWindow()->SetCustomUnitText( String::CreateFromAscii( " ms" ) );
        getTypedControlWindow()->SetCustomConvertHdl( LINK( this, OTimeDurationControl, OnCustomConvert ) );
    }

    //------------------------------------------------------------------
    OTimeDurationControl::~OTimeDurationControl()
    {
    }

    //------------------------------------------------------------------
    ::sal_Int16 SAL_CALL OTimeDurationControl::getControlType() throw (::com::sun::star::uno::RuntimeException)
    {
        // don't use the base class'es method, it would claim we're a standard control, which
        // we in fact aren't
        return PropertyControlType::Unknown;
    }

    //------------------------------------------------------------------
    IMPL_LINK( OTimeDurationControl, OnCustomConvert, MetricField*, /*pField*/ )
    {
        long nMultiplier = 1;
        if ( getTypedControlWindow()->GetCurUnitText().EqualsIgnoreCaseAscii( "ms" ) )
            nMultiplier = 1;
        if ( getTypedControlWindow()->GetCurUnitText().EqualsIgnoreCaseAscii( "s" ) )
            nMultiplier = 1000;
        else if ( getTypedControlWindow()->GetCurUnitText().EqualsIgnoreCaseAscii( "m" ) )
            nMultiplier = 1000 * 60;
        else if ( getTypedControlWindow()->GetCurUnitText().EqualsIgnoreCaseAscii( "h" ) )
            nMultiplier = 1000 * 60 * 60;

        getTypedControlWindow()->SetValue( getTypedControlWindow()->GetLastValue() * nMultiplier );

        return 0L;
    }

//............................................................................
} // namespace pcr
//............................................................................

