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

#include "vbavalidation.hxx"
#include "vbaformatcondition.hxx" //#i108860
#include <com/sun/star/sheet/XSheetCondition.hpp>
#include <com/sun/star/sheet/ValidationType.hpp>
#include <com/sun/star/sheet/ValidationAlertStyle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <ooo/vba/excel/XlDVType.hpp>
#include <ooo/vba/excel/XlFormatConditionOperator.hpp>
#include <ooo/vba/excel/XlDVAlertStyle.hpp>

#include "unonames.hxx"
#include "rangelst.hxx"
#include "excelvbahelper.hxx"
#include "vbarange.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

const static OUString VALIDATION( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_VALIDAT ) );
const static OUString IGNOREBLANK( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_IGNOREBL ) );
const static OUString SHOWINPUT( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_SHOWINP  ) );
const static OUString SHOWERROR( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_SHOWERR  ) );
const static OUString ERRORTITLE( RTL_CONSTASCII_USTRINGPARAM(  SC_UNONAME_ERRTITLE  ) );
const static OUString INPUTTITLE( RTL_CONSTASCII_USTRINGPARAM(  SC_UNONAME_INPTITLE  ) );
const static OUString INPUTMESS( RTL_CONSTASCII_USTRINGPARAM(  SC_UNONAME_INPMESS  ) );
const static OUString ERRORMESS( RTL_CONSTASCII_USTRINGPARAM(  SC_UNONAME_ERRMESS  ) );
const static OUString STYPE( RTL_CONSTASCII_USTRINGPARAM(  SC_UNONAME_TYPE  ) );
const static OUString SHOWLIST( RTL_CONSTASCII_USTRINGPARAM(  SC_UNONAME_SHOWLIST  ) );
const static OUString ALERTSTYLE( RTL_CONSTASCII_USTRINGPARAM(  SC_UNONAME_ERRALSTY  ) );

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

OUString SAL_CALL
ScVbaValidation::getErrorTitle() throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps = lcl_getValidationProps( m_xRange );
    OUString sErrorTitle;
    xProps->getPropertyValue( ERRORTITLE )  >>= sErrorTitle;
    return sErrorTitle;
}

void
ScVbaValidation::setErrorTitle( const OUString& _errormessage ) throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps( lcl_getValidationProps( m_xRange ) );
    xProps->setPropertyValue( ERRORTITLE, uno::makeAny( _errormessage ) );
    lcl_setValidationProps( m_xRange, xProps );
}

OUString SAL_CALL
ScVbaValidation::getInputMessage() throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps = lcl_getValidationProps( m_xRange );
    OUString sMsg;
    xProps->getPropertyValue( INPUTMESS )  >>= sMsg;
    return sMsg;
}

void SAL_CALL
ScVbaValidation::setInputMessage( const OUString& _inputmessage ) throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps( lcl_getValidationProps( m_xRange ) );
    xProps->setPropertyValue( INPUTMESS, uno::makeAny( _inputmessage ) );
    lcl_setValidationProps( m_xRange, xProps );
}

OUString SAL_CALL
ScVbaValidation::getInputTitle() throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps = lcl_getValidationProps( m_xRange );
    OUString sString;
    xProps->getPropertyValue( INPUTTITLE )  >>= sString;
    return sString;
}

void SAL_CALL
ScVbaValidation::setInputTitle( const OUString& _inputtitle ) throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps( lcl_getValidationProps( m_xRange ) );
    xProps->setPropertyValue( INPUTTITLE, uno::makeAny( _inputtitle ) );
    lcl_setValidationProps( m_xRange, xProps );
}

OUString SAL_CALL
ScVbaValidation::getErrorMessage() throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps = lcl_getValidationProps( m_xRange );
    OUString sString;
    xProps->getPropertyValue( ERRORMESS )  >>= sString;
    return sString;
}

void SAL_CALL
ScVbaValidation::setErrorMessage( const OUString& _errormessage ) throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps( lcl_getValidationProps( m_xRange ) );
    xProps->setPropertyValue( ERRORMESS, uno::makeAny( _errormessage ) );
    lcl_setValidationProps( m_xRange, xProps );
}


void SAL_CALL
ScVbaValidation::Delete(  ) throw (uno::RuntimeException)
{
    OUString sBlank;
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
        throw uno::RuntimeException( OUString( "validation object already exists" ), uno::Reference< uno::XInterface >() );
    sal_Int32 nType = -1;
    if ( !Type.hasValue()  || !( Type >>= nType ) )
        throw uno::RuntimeException( OUString( "missing required param" ), uno::Reference< uno::XInterface >() );

    Delete(); // set up defaults
    OUString sFormula1;
    Formula1 >>= sFormula1;
    OUString sFormula2;
    Formula2 >>= sFormula2;
    switch ( nType )
    {
        case excel::XlDVType::xlValidateList:
            {
                // for validate list
                // at least formula1 is required
                if ( !Formula1.hasValue() )
                    throw uno::RuntimeException( OUString( "missing param" ), uno::Reference< uno::XInterface >() );
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
            throw uno::RuntimeException( OUString( "unsupported operation..." ), uno::Reference< uno::XInterface >() );
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
            throw uno::RuntimeException( OUString( "bad param..." ), uno::Reference< uno::XInterface >() );

        }
    }

    xProps->setPropertyValue( ALERTSTYLE, uno::makeAny( eStyle ) );

    // i#108860: fix the defect that validation cannot work when the input
    // should be limited between a lower bound and an upper bound
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

OUString SAL_CALL
ScVbaValidation::getFormula1() throw (uno::RuntimeException)
{
    uno::Reference< sheet::XSheetCondition > xCond( lcl_getValidationProps( m_xRange ), uno::UNO_QUERY_THROW );
    OUString sString = xCond->getFormula1();

    sal_uInt16 nFlags = 0;
    ScRangeList aCellRanges;
    formula::FormulaGrammar::AddressConvention eConv = formula::FormulaGrammar::CONV_XL_A1;

    ScDocShell* pDocSh = excel::GetDocShellFromRange( m_xRange );
    // in calc validation formula is either a range or formula
    // that results in range.
    // In VBA both formula and address can have a leading '='
    // in result of getFormula1, however it *seems* that a named range or
    // real formula has to (or is expected to) have the '='
    if ( pDocSh && !ScVbaRange::getCellRangesForAddress(  nFlags, sString, pDocSh, aCellRanges, eConv ) )
        sString = "=" + sString;
    return sString;
}

OUString SAL_CALL
ScVbaValidation::getFormula2() throw (uno::RuntimeException)
{
        uno::Reference< sheet::XSheetCondition > xCond( lcl_getValidationProps( m_xRange ), uno::UNO_QUERY_THROW );
    return xCond->getFormula2();
}

sal_Int32 SAL_CALL
ScVbaValidation::getType() throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps( lcl_getValidationProps( m_xRange ) );
    sheet::ValidationType nValType = sheet::ValidationType_ANY;
    xProps->getPropertyValue( STYPE )  >>= nValType;
    sal_Int32 nExcelType = excel::XlDVType::xlValidateList; // pick a default
    if ( xProps.is() )
    {
        switch ( nValType )
        {
            case sheet::ValidationType_LIST:
                nExcelType = excel::XlDVType::xlValidateList;
                break;
            case sheet::ValidationType_ANY: // not ANY not really a great match for anything I fear:-(
                nExcelType = excel::XlDVType::xlValidateInputOnly;
                break;
            case sheet::ValidationType_CUSTOM:
                nExcelType = excel::XlDVType::xlValidateCustom;
                break;
            case sheet::ValidationType_WHOLE:
                nExcelType = excel::XlDVType::xlValidateWholeNumber;
                break;
            case sheet::ValidationType_DECIMAL:
                nExcelType = excel::XlDVType::xlValidateDecimal;
                break;
            case sheet::ValidationType_DATE:
                nExcelType = excel::XlDVType::xlValidateDate;
                break;
            case sheet::ValidationType_TIME:
                nExcelType = excel::XlDVType::xlValidateTime;
                break;
            case sheet::ValidationType_TEXT_LEN:
                nExcelType = excel::XlDVType::xlValidateTextLength;
                break;
            case sheet::ValidationType_MAKE_FIXED_SIZE:
            default:
                break;
        };
    }
    return nExcelType;
}

OUString
ScVbaValidation::getServiceImplName()
{
    return OUString("ScVbaValidation");
}

uno::Sequence< OUString >
ScVbaValidation::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = OUString("ooo.vba.excel.Validation" );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
