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
#include "vbaformatcondition.hxx"
#include "vbaformatconditions.hxx"
#include "unonames.hxx"
#include <ooo/vba/excel/XlFormatConditionType.hpp>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

static ScVbaFormatConditions*
lcl_getScVbaFormatConditionsPtr( const uno::Reference< excel::XFormatConditions >& xFormatConditions ) throw ( script::BasicErrorException )
{
    ScVbaFormatConditions* pFormatConditions = static_cast< ScVbaFormatConditions* >( xFormatConditions.get() );
    if ( !pFormatConditions )
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, OUString() );
    return pFormatConditions;
}

ScVbaFormatCondition::ScVbaFormatCondition( const uno::Reference< XHelperInterface >& xParent,
                                            const uno::Reference< uno::XComponentContext > & xContext,
                                            const uno::Reference< sheet::XSheetConditionalEntry >& _xSheetConditionalEntry,
                                            const uno::Reference< excel::XStyle >& _xStyle,
                                            const uno::Reference< excel::XFormatConditions >& _xFormatConditions,
                                            const uno::Reference< css::beans::XPropertySet >& _xPropertySet ) throw ( css::uno::RuntimeException, css::script::BasicErrorException )
    : ScVbaFormatCondition_BASE( xParent, xContext,
                                 uno::Reference< sheet::XSheetCondition >( _xSheetConditionalEntry, css::uno::UNO_QUERY_THROW ) ),
                                 moFormatConditions( _xFormatConditions ), mxStyle( _xStyle ), mxParentRangePropertySet( _xPropertySet )
{
        mxSheetConditionalEntries = lcl_getScVbaFormatConditionsPtr( moFormatConditions )->getSheetConditionalEntries();

        mxSheetConditionalEntry = _xSheetConditionalEntry;
        msStyleName = mxStyle->getName();
}

void SAL_CALL
ScVbaFormatCondition::Delete(  ) throw (script::BasicErrorException, uno::RuntimeException, std::exception)
{
    ScVbaFormatConditions* pFormatConditions = lcl_getScVbaFormatConditionsPtr( moFormatConditions );
    pFormatConditions->removeFormatCondition(msStyleName, true);
        notifyRange();
}

void SAL_CALL
ScVbaFormatCondition::Modify( ::sal_Int32 _nType, const uno::Any& _aOperator, const uno::Any& _aFormula1, const uno::Any& _aFormula2 ) throw (script::BasicErrorException, uno::RuntimeException, std::exception)
{
    try
    {
        ScVbaFormatConditions* pFormatConditions = lcl_getScVbaFormatConditionsPtr( moFormatConditions );
        pFormatConditions->removeFormatCondition(msStyleName, false);
        pFormatConditions->Add(_nType, _aOperator, _aFormula1, _aFormula2, mxStyle);
    }
    catch (const uno::Exception&)
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, OUString() );
    }
}

uno::Reference< excel::XInterior > SAL_CALL
ScVbaFormatCondition::Interior(  ) throw (script::BasicErrorException, uno::RuntimeException, std::exception)
{
    return mxStyle->Interior();
}

uno::Reference< excel::XFont > SAL_CALL
ScVbaFormatCondition::Font(  ) throw (script::BasicErrorException, uno::RuntimeException, std::exception)
{
    return mxStyle->Font();
}
uno::Any SAL_CALL
ScVbaFormatCondition::Borders( const uno::Any& Index ) throw (script::BasicErrorException, uno::RuntimeException, std::exception)
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
            DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, OUString() );
    }
    return aAPIType;
}

void
ScVbaFormatCondition::setFormula1( const uno::Any& _aFormula1) throw ( script::BasicErrorException )
{
    // getA1Formula *SHOULD* detect whether the formula is r1c1 or A1 syntax
    // and if R1C1 convert to A1
    ScVbaFormatCondition_BASE::setFormula1( uno::makeAny( ScVbaFormatConditions::getA1Formula(_aFormula1) ) );
}

::sal_Int32 SAL_CALL
ScVbaFormatCondition::Type(  ) throw ( script::BasicErrorException, uno::RuntimeException, std::exception )
{
    sal_Int32 nReturnType = 0;
    if ( mxSheetCondition->getOperator() == sheet::ConditionOperator_FORMULA)
        nReturnType = excel::XlFormatConditionType::xlExpression;
    else
        nReturnType = excel::XlFormatConditionType::xlCellValue;
    return nReturnType;
}

::sal_Int32
ScVbaFormatCondition::Operator( bool bVal ) throw (script::BasicErrorException )
{
    return ScVbaFormatCondition_BASE::Operator( bVal );
}
::sal_Int32 SAL_CALL
ScVbaFormatCondition::Operator(  ) throw (script::BasicErrorException, uno::RuntimeException)
{
    return ScVbaFormatCondition_BASE::Operator( true );
}

void
ScVbaFormatCondition::notifyRange() throw ( script::BasicErrorException )
{
    try
    {
        mxParentRangePropertySet->setPropertyValue(SC_UNONAME_CONDFMT, uno::makeAny( mxSheetConditionalEntries));
    }
    catch (uno::Exception& )
    {
        DebugHelper::basicexception(ERRCODE_BASIC_METHOD_FAILED, OUString() );
    }
}

OUString
ScVbaFormatCondition::getServiceImplName()
{
    return OUString("ScVbaFormatCondition");
}

uno::Sequence< OUString >
ScVbaFormatCondition::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.excel.FormatCondition";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
