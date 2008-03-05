/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: validationpropertyhelper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 19:07:49 $
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
 * *    This library is distributed in the hope that it will be useful,
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

#include "oox/xls/validationpropertyhelper.hxx"
#include <com/sun/star/sheet/ValidationType.hpp>
#include <com/sun/star/sheet/ValidationAlertStyle.hpp>
#include <com/sun/star/sheet/TableValidationVisibility.hpp>
#include <com/sun/star/sheet/XSheetCondition.hpp>
#include <com/sun/star/sheet/XMultiFormulaTokens.hpp>
#include "oox/helper/propertyset.hxx"
#include "oox/xls/excelhandlers.hxx"
#include "oox/xls/worksheethelper.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::sheet::ValidationType;
using ::com::sun::star::sheet::ValidationAlertStyle;
using ::com::sun::star::sheet::ConditionOperator;
using ::com::sun::star::sheet::XSheetCondition;
using ::com::sun::star::sheet::XMultiFormulaTokens;

namespace oox {
namespace xls {

// ============================================================================

namespace {

const sal_Char* const sppcPropNames[] =
{
    "Type",
    "ShowInputMessage",
    "InputTitle",
    "InputMessage",
    "ShowErrorMessage",
    "ErrorTitle",
    "ErrorMessage",
    "ErrorAlertStyle",
    "ShowList",
    "IgnoreBlankCells",
    0
};

} // namespace

// ----------------------------------------------------------------------------

ValidationPropertyHelper::ValidationPropertyHelper( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    maValProps( sppcPropNames ),
    maValidationProp( CREATE_OUSTRING( "Validation" ) )
{
}

void ValidationPropertyHelper::writeValidationProperties( PropertySet& rPropSet, const OoxValidationData& rValData )
{
    Reference< XPropertySet > xValidation;
    if( rPropSet.getProperty( xValidation, maValidationProp ) && xValidation.is() )
    {
        PropertySet aValProps( xValidation );
        namespace csss = ::com::sun::star::sheet;

        // convert validation type to API enum
        ValidationType eType = csss::ValidationType_ANY;
        switch( rValData.mnType )
        {
            case XML_custom:        eType = csss::ValidationType_CUSTOM;    break;
            case XML_date:          eType = csss::ValidationType_DATE;      break;
            case XML_decimal:       eType = csss::ValidationType_DECIMAL;   break;
            case XML_list:          eType = csss::ValidationType_LIST;      break;
            case XML_none:          eType = csss::ValidationType_ANY;       break;
            case XML_textLength:    eType = csss::ValidationType_TEXT_LEN;  break;
            case XML_time:          eType = csss::ValidationType_TIME;      break;
            case XML_whole:         eType = csss::ValidationType_WHOLE;     break;
            default:    OSL_ENSURE( false, "ValidationPropertyHelper::writeValidationProperties - unknown validation type" );
        }

        // convert error alert style to API enum
        ValidationAlertStyle nAlertStyle = csss::ValidationAlertStyle_STOP;
        switch( rValData.mnErrorStyle )
        {
            case XML_information:   nAlertStyle = csss::ValidationAlertStyle_INFO;      break;
            case XML_stop:          nAlertStyle = csss::ValidationAlertStyle_STOP;      break;
            case XML_warning:       nAlertStyle = csss::ValidationAlertStyle_WARNING;   break;
            default:    OSL_ENSURE( false, "ValidationPropertyHelper::writeValidationProperties - unknown error style" );
        }

        // convert dropdown style to API visibility constants
        sal_Int16 nVisibility = rValData.mbNoDropDown ? csss::TableValidationVisibility::INVISIBLE : csss::TableValidationVisibility::UNSORTED;

        // write all properties
        maValProps
            << eType
            << rValData.mbShowInputMsg << rValData.maInputTitle << rValData.maInputMessage
            << rValData.mbShowErrorMsg << rValData.maErrorTitle << rValData.maErrorMessage
            << nAlertStyle << nVisibility << rValData.mbAllowBlank
            >> aValProps;

        try
        {
            // condition operator
            Reference< XSheetCondition > xSheetCond( xValidation, UNO_QUERY_THROW );
            xSheetCond->setOperator( convertToApiOperator( rValData.mnOperator ) );

            // condition formulas
            Reference< XMultiFormulaTokens > xTokens( xValidation, UNO_QUERY_THROW );
            xTokens->setTokens( 0, rValData.maTokens1 );
            xTokens->setTokens( 1, rValData.maTokens2 );
        }
        catch( Exception& )
        {
        }

        // write back validation settings to cell range(s)
        rPropSet.setProperty( maValidationProp, xValidation );
    }
}

ConditionOperator ValidationPropertyHelper::convertToApiOperator( sal_Int32 nToken )
{
    using namespace ::com::sun::star::sheet;
    switch( nToken )
    {
        case XML_between:               return ConditionOperator_BETWEEN;
        case XML_equal:                 return ConditionOperator_EQUAL;
        case XML_greaterThan:           return ConditionOperator_GREATER;
        case XML_greaterThanOrEqual:    return ConditionOperator_GREATER_EQUAL;
        case XML_lessThan:              return ConditionOperator_LESS;
        case XML_lessThanOrEqual:       return ConditionOperator_LESS_EQUAL;
        case XML_notBetween:            return ConditionOperator_NOT_BETWEEN;
        case XML_notEqual:              return ConditionOperator_NOT_EQUAL;
    }
    return ConditionOperator_NONE;
}

// ============================================================================

} // namespace xls
} // namespace oox

