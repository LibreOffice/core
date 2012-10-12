/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#include "vbavalidation.hxx"
#include "vbaformatcondition.hxx"
#include <com/sun/star/sheet/XSheetCondition.hpp>
#include <com/sun/star/sheet/ValidationType.hpp>
#include <com/sun/star/sheet/ValidationAlertStyle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <ooo/vba/excel/XlDVType.hpp>
#include <ooo/vba/excel/XlFormatConditionOperator.hpp>
#include <ooo/vba/excel/XlDVAlertStyle.hpp>

#include "unonames.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

const static rtl::OUString VALIDATION( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_VALIDAT ) );
const static rtl::OUString IGNOREBLANK( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_IGNOREBL ) );
const static rtl::OUString SHOWINPUT( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_SHOWINP  ) );
const static rtl::OUString SHOWERROR( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_SHOWERR  ) );
const static rtl::OUString ERRORTITLE( RTL_CONSTASCII_USTRINGPARAM(  SC_UNONAME_ERRTITLE  ) );
const static rtl::OUString INPUTTITLE( RTL_CONSTASCII_USTRINGPARAM(  SC_UNONAME_INPTITLE  ) );
const static rtl::OUString INPUTMESS( RTL_CONSTASCII_USTRINGPARAM(  SC_UNONAME_INPMESS  ) );
const static rtl::OUString ERRORMESS( RTL_CONSTASCII_USTRINGPARAM(  SC_UNONAME_ERRMESS  ) );
const static rtl::OUString STYPE( RTL_CONSTASCII_USTRINGPARAM(  SC_UNONAME_TYPE  ) );
const static rtl::OUString SHOWLIST( RTL_CONSTASCII_USTRINGPARAM(  SC_UNONAME_SHOWLIST  ) );
const static rtl::OUString ALERTSTYLE( RTL_CONSTASCII_USTRINGPARAM(  SC_UNONAME_ERRALSTY  ) );

static void
lcl_setValidationProps( const uno::Reference< table::XCellRange >& xRange, const uno::Reference< beans::XPropertySet >& xProps )
{
    uno::Reference< beans::XPropertySet > xRangeProps( xRange, uno::UNO_QUERY_THROW );
    xRangeProps->setPropertyValue( VALIDATION , uno::makeAny( xProps ) );
}

static uno::Reference< beans::XPropertySet >
lcl_getValidationProps( const uno::Reference< table::XCellRange >& xRange )
{
    uno::Reference< beans::XPropertySet > xProps( xRange, uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xValProps;
    xValProps.set( xProps->getPropertyValue( VALIDATION ), uno::UNO_QUERY_THROW );
    return xValProps;
}

::sal_Bool SAL_CALL
ScVbaValidation::getIgnoreBlank() throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps( lcl_getValidationProps( m_xRange ) );
    sal_Bool bBlank = false;
    xProps->getPropertyValue( IGNOREBLANK )  >>= bBlank;
    return bBlank;
}

void SAL_CALL
ScVbaValidation::setIgnoreBlank( ::sal_Bool _ignoreblank ) throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps( lcl_getValidationProps( m_xRange ) );
    xProps->setPropertyValue( IGNOREBLANK, uno::makeAny( _ignoreblank ) );
    lcl_setValidationProps( m_xRange, xProps );
}

::sal_Bool SAL_CALL
ScVbaValidation::getInCellDropdown() throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps = lcl_getValidationProps( m_xRange );
    sal_Int32 nShowList = 0;
    xProps->getPropertyValue( SHOWLIST )  >>= nShowList;
    return ( nShowList ? sal_True : false );
}

void SAL_CALL
ScVbaValidation::setInCellDropdown( ::sal_Bool  _incelldropdown  ) throw (uno::RuntimeException)
{
    sal_Int32 nDropDown = false;
    if ( _incelldropdown )
        nDropDown = 1;
    uno::Reference< beans::XPropertySet > xProps( lcl_getValidationProps(m_xRange) );
    xProps->setPropertyValue( SHOWLIST, uno::makeAny( nDropDown ) );
    lcl_setValidationProps( m_xRange, xProps );
}

::sal_Bool SAL_CALL
ScVbaValidation::getShowInput() throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps = lcl_getValidationProps( m_xRange );
    sal_Bool bShowInput = false;
    xProps->getPropertyValue( SHOWINPUT )  >>= bShowInput;
    return bShowInput;
}

void SAL_CALL
ScVbaValidation:: setShowInput( ::sal_Bool _showinput ) throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps( lcl_getValidationProps(m_xRange) );
    xProps->setPropertyValue( IGNOREBLANK, uno::makeAny( _showinput ) );
    lcl_setValidationProps( m_xRange, xProps );
}

::sal_Bool SAL_CALL
ScVbaValidation::getShowError() throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps = lcl_getValidationProps( m_xRange );
    sal_Bool bShowError = false;
    xProps->getPropertyValue( SHOWERROR )  >>= bShowError;
    return bShowError;
}

void SAL_CALL
ScVbaValidation::setShowError( ::sal_Bool _showerror ) throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps( lcl_getValidationProps( m_xRange ) );
    xProps->setPropertyValue( SHOWERROR, uno::makeAny( _showerror ) );
    lcl_setValidationProps( m_xRange, xProps );
}

::rtl::OUString SAL_CALL
ScVbaValidation::getErrorTitle() throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps = lcl_getValidationProps( m_xRange );
    rtl::OUString sErrorTitle;
    xProps->getPropertyValue( ERRORTITLE )  >>= sErrorTitle;
    return sErrorTitle;
}

void
ScVbaValidation::setErrorTitle( const rtl::OUString& _errormessage ) throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps( lcl_getValidationProps( m_xRange ) );
    xProps->setPropertyValue( ERRORTITLE, uno::makeAny( _errormessage ) );
    lcl_setValidationProps( m_xRange, xProps );
}

::rtl::OUString SAL_CALL
ScVbaValidation::getInputMessage() throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps = lcl_getValidationProps( m_xRange );
    rtl::OUString sMsg;
    xProps->getPropertyValue( INPUTMESS )  >>= sMsg;
    return sMsg;
}

void SAL_CALL
ScVbaValidation::setInputMessage( const ::rtl::OUString& _inputmessage ) throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps( lcl_getValidationProps( m_xRange ) );
    xProps->setPropertyValue( INPUTMESS, uno::makeAny( _inputmessage ) );
    lcl_setValidationProps( m_xRange, xProps );
}

::rtl::OUString SAL_CALL
ScVbaValidation::getInputTitle() throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps = lcl_getValidationProps( m_xRange );
    rtl::OUString sString;
    xProps->getPropertyValue( INPUTTITLE )  >>= sString;
    return sString;
}

void SAL_CALL
ScVbaValidation::setInputTitle( const ::rtl::OUString& _inputtitle ) throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps( lcl_getValidationProps( m_xRange ) );
    xProps->setPropertyValue( INPUTTITLE, uno::makeAny( _inputtitle ) );
    lcl_setValidationProps( m_xRange, xProps );
}

::rtl::OUString SAL_CALL
ScVbaValidation::getErrorMessage() throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps = lcl_getValidationProps( m_xRange );
    rtl::OUString sString;
    xProps->getPropertyValue( ERRORMESS )  >>= sString;
    return sString;
}

void SAL_CALL
ScVbaValidation::setErrorMessage( const ::rtl::OUString& _errormessage ) throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps( lcl_getValidationProps( m_xRange ) );
    xProps->setPropertyValue( ERRORMESS, uno::makeAny( _errormessage ) );
    lcl_setValidationProps( m_xRange, xProps );
}


void SAL_CALL
ScVbaValidation::Delete(  ) throw (uno::RuntimeException)
{
    rtl::OUString sBlank;
    uno::Reference< beans::XPropertySet > xProps( lcl_getValidationProps( m_xRange ) );
    uno::Reference< sheet::XSheetCondition > xCond( xProps, uno::UNO_QUERY_THROW );
    xProps->setPropertyValue( IGNOREBLANK, uno::makeAny( sal_True ) );
    xProps->setPropertyValue( SHOWINPUT, uno::makeAny( sal_True ) );
    xProps->setPropertyValue( SHOWERROR, uno::makeAny( sal_True ) );
    xProps->setPropertyValue( ERRORTITLE, uno::makeAny( sBlank ) );
    xProps->setPropertyValue( INPUTMESS, uno::makeAny( sBlank) );
    xProps->setPropertyValue( ALERTSTYLE, uno::makeAny( sheet::ValidationAlertStyle_STOP) );
    xProps->setPropertyValue( STYPE, uno::makeAny( sheet::ValidationType_ANY ) );
    xCond->setFormula1( sBlank );
    xCond->setFormula2( sBlank );
    xCond->setOperator( sheet::ConditionOperator_NONE );

    lcl_setValidationProps( m_xRange, xProps );
}

// Fix the defect that validatation cannot work when the input should be limited between a lower bound and an upper bound
void SAL_CALL
ScVbaValidation::Add( const uno::Any& Type, const uno::Any& AlertStyle, const uno::Any& Operator, const uno::Any& Formula1, const uno::Any& Formula2 ) throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps( lcl_getValidationProps( m_xRange ) );
    uno::Reference< sheet::XSheetCondition > xCond( xProps, uno::UNO_QUERY_THROW );

    sheet::ValidationType nValType = sheet::ValidationType_ANY;
    xProps->getPropertyValue( STYPE )  >>= nValType;
    if ( nValType  != sheet::ValidationType_ANY  )
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "validation object already exists" ) ), uno::Reference< uno::XInterface >() );
    sal_Int32 nType = -1;
    if ( !Type.hasValue()  || !( Type >>= nType ) )
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "missing required param" ) ), uno::Reference< uno::XInterface >() );

    Delete(); // set up defaults
    rtl::OUString sFormula1;
    Formula1 >>= sFormula1;
    rtl::OUString sFormula2;
    Formula2 >>= sFormula2;
    switch ( nType )
    {
        case excel::XlDVType::xlValidateList:
            {
                // for validate list
                // at least formula1 is required
                if ( !Formula1.hasValue() )
                    throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "missing param" ) ), uno::Reference< uno::XInterface >() );
                nValType = sheet::ValidationType_LIST;
                xProps->setPropertyValue( STYPE, uno::makeAny(nValType ));
                // #TODO validate required params
                // #TODO need to correct the ';' delimited formula on get/set
                break;
            }
        case excel::XlDVType::xlValidateWholeNumber:
            nValType = sheet::ValidationType_WHOLE;
            xProps->setPropertyValue( STYPE, uno::makeAny(nValType ));
            break;
        default:
            throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "unsupported operation..." ) ), uno::Reference< uno::XInterface >() );
    }

    sheet::ValidationAlertStyle eStyle = sheet::ValidationAlertStyle_STOP;
    sal_Int32 nVbaAlertStyle = excel::XlDVAlertStyle::xlValidAlertStop;
    if ( AlertStyle.hasValue() && ( AlertStyle >>= nVbaAlertStyle ) )
    {
        switch( nVbaAlertStyle )
        {
            case excel::XlDVAlertStyle::xlValidAlertStop:
                // yes I know it's already defaulted but safer to assume
                // someone propbably could change the code above
                eStyle = sheet::ValidationAlertStyle_STOP;
                break;
            case excel::XlDVAlertStyle::xlValidAlertWarning:
                eStyle = sheet::ValidationAlertStyle_WARNING;
                break;
            case excel::XlDVAlertStyle::xlValidAlertInformation:
                eStyle = sheet::ValidationAlertStyle_INFO;
                break;
            default:
            throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "bad param..." ) ), uno::Reference< uno::XInterface >() );

        }
    }

    xProps->setPropertyValue( ALERTSTYLE, uno::makeAny( eStyle ) );

    // fix the defect that validatation cannot work when the input should be limited between a lower bound and an upper bound
    if ( Operator.hasValue() )
    {
        css::sheet::ConditionOperator conOperator = ScVbaFormatCondition::retrieveAPIOperator( Operator );
        xCond->setOperator( conOperator );
    }

    if ( !sFormula1.isEmpty() )
        xCond->setFormula1( sFormula1 );
    if ( !sFormula2.isEmpty() )
        xCond->setFormula2( sFormula2 );

    lcl_setValidationProps( m_xRange, xProps );
}

::rtl::OUString SAL_CALL
ScVbaValidation::getFormula1() throw (uno::RuntimeException)
{
    uno::Reference< sheet::XSheetCondition > xCond( lcl_getValidationProps( m_xRange ), uno::UNO_QUERY_THROW );
    return xCond->getFormula1();
}

::rtl::OUString SAL_CALL
ScVbaValidation::getFormula2() throw (uno::RuntimeException)
{
        uno::Reference< sheet::XSheetCondition > xCond( lcl_getValidationProps( m_xRange ), uno::UNO_QUERY_THROW );
    return xCond->getFormula2();
}

rtl::OUString
ScVbaValidation::getServiceImplName()
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ScVbaValidation"));
}

uno::Sequence< rtl::OUString >
ScVbaValidation::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.Validation" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
