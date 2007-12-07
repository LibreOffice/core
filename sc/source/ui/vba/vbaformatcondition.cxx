/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbaformatcondition.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:52:10 $
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
#include "vbaformatcondition.hxx"
#include "vbaformatconditions.hxx"
#include <org/openoffice/excel/XlFormatConditionType.hpp>

using namespace ::org::openoffice;
using namespace ::com::sun::star;

ScVbaFormatConditions*
lcl_getScVbaFormatConditionsPtr( const uno::Reference< excel::XFormatConditions >& xFormatConditions ) throw ( script::BasicErrorException )
{
    ScVbaFormatConditions* pFormatConditions = static_cast< ScVbaFormatConditions* >( xFormatConditions.get() );
    if ( !pFormatConditions )
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString() );
    return pFormatConditions;
}
ScVbaFormatCondition::ScVbaFormatCondition(  const uno::Reference< vba::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< sheet::XSheetConditionalEntry >& _xSheetConditionalEntry, const uno::Reference< excel::XStyle >& _xStyle, const uno::Reference< excel::XFormatConditions >& _xFormatConditions, const uno::Reference< css::beans::XPropertySet >& _xPropertySet ) throw ( css::uno::RuntimeException ) : ScVbaFormatCondition_BASE( xParent, xContext, uno::Reference< sheet::XSheetCondition >( _xSheetConditionalEntry, css::uno::UNO_QUERY_THROW ) ), moFormatConditions( _xFormatConditions ), mxStyle( _xStyle ), mxParentRangePropertySet( _xPropertySet )
{
        mxSheetConditionalEntries = lcl_getScVbaFormatConditionsPtr( moFormatConditions )->getSheetConditionalEntries();

        mxSheetConditionalEntry = _xSheetConditionalEntry;
        msStyleName = mxStyle->getName();
}


void SAL_CALL
ScVbaFormatCondition::Delete(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    ScVbaFormatConditions* pFormatConditions = lcl_getScVbaFormatConditionsPtr( moFormatConditions );
    pFormatConditions->removeFormatCondition(msStyleName, sal_True);
        notifyRange();
}

void SAL_CALL
ScVbaFormatCondition::Modify( ::sal_Int32 _nType, const uno::Any& _aOperator, const uno::Any& _aFormula1, const uno::Any& _aFormula2 ) throw (script::BasicErrorException, uno::RuntimeException)
{
    try
    {
        ScVbaFormatConditions* pFormatConditions = lcl_getScVbaFormatConditionsPtr( moFormatConditions );
        pFormatConditions->removeFormatCondition(msStyleName, sal_False);
        pFormatConditions->Add(_nType, _aOperator, _aFormula1, _aFormula2, mxStyle);
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString() );
    }
}

uno::Reference< excel::XInterior > SAL_CALL
ScVbaFormatCondition::Interior(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    return mxStyle->Interior();
}

uno::Reference< excel::XFont > SAL_CALL
ScVbaFormatCondition::Font(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    return mxStyle->Font();
}
uno::Any SAL_CALL
ScVbaFormatCondition::Borders( const uno::Any& Index ) throw (script::BasicErrorException, uno::RuntimeException)
{ return mxStyle->Borders( Index );
}

sheet::ConditionOperator
ScVbaFormatCondition::retrieveAPIType(sal_Int32 _nVBAType, const uno::Reference< sheet::XSheetCondition >& _xSheetCondition ) throw ( script::BasicErrorException )
{
    sheet::ConditionOperator aAPIType = sheet::ConditionOperator_NONE;
    switch (_nVBAType)
    {
        case excel::XlFormatConditionType::xlExpression:
            aAPIType = sheet::ConditionOperator_FORMULA;
            break;
        case excel::XlFormatConditionType::xlCellValue:
            if ( _xSheetCondition.is() && (_xSheetCondition->getOperator() == sheet::ConditionOperator_FORMULA ) )
                aAPIType = sheet::ConditionOperator_NONE;
            break;
        default:
            DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString() );
    }
    return aAPIType;
}

void
ScVbaFormatCondition::setFormula1( const uno::Any& _aFormula1) throw ( script::BasicErrorException )
{
    // getA1Formula *SHOULD* detect whether the formula is r1c1 or A1 syntax
    // and if R1C1 convert to A1
    ScVbaFormatCondition_BASE::setFormula1( uno::makeAny( lcl_getScVbaFormatConditionsPtr( moFormatConditions )->getA1Formula(_aFormula1) ) );
}

void
ScVbaFormatCondition::setFormula2( const uno::Any& _aFormula2) throw ( script::BasicErrorException )
{
    ScVbaFormatCondition_BASE::setFormula1( uno::makeAny( lcl_getScVbaFormatConditionsPtr( moFormatConditions )->getA1Formula(_aFormula2)) );
}

::sal_Int32 SAL_CALL
ScVbaFormatCondition::Type(  ) throw ( script::BasicErrorException, uno::RuntimeException )
{
    sal_Int32 nReturnType = 0;
    if ( mxSheetCondition->getOperator() == sheet::ConditionOperator_FORMULA)
        nReturnType = excel::XlFormatConditionType::xlExpression;
    else
        nReturnType = excel::XlFormatConditionType::xlCellValue;
    return nReturnType;
}


::sal_Int32
ScVbaFormatCondition::Operator( sal_Bool bVal ) throw (script::BasicErrorException )
{
    return ScVbaFormatCondition_BASE::Operator( bVal );
}
::sal_Int32 SAL_CALL
ScVbaFormatCondition::Operator(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    return ScVbaFormatCondition_BASE::Operator( sal_True );
}

void
ScVbaFormatCondition::notifyRange() throw ( script::BasicErrorException )
{
    try
    {
        mxParentRangePropertySet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ConditionalFormat") ), uno::makeAny( mxSheetConditionalEntries) );
    }
    catch (uno::Exception& )
    {
        DebugHelper::exception(SbERR_METHOD_FAILED, rtl::OUString() );
    }
}

rtl::OUString&
ScVbaFormatCondition::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaFormatCondition") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaFormatCondition::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.excel.FormatCondition" ) );
    }
    return aServiceNames;
}
