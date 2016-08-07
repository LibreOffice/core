/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#include "precompiled_reportdesign.hxx"
#include "conditionupdater.hxx"
#include "reportformula.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/report/XFormatCondition.hpp>
/** === end UNO includes === **/

#include <tools/diagnose_ex.h>

//........................................................................
namespace rptui
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::beans::PropertyChangeEvent;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::report::XReportControlModel;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::report::XFormatCondition;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::Exception;
    /** === end UNO using === **/

    //====================================================================
    //= ConditionUpdater
    //====================================================================
    //--------------------------------------------------------------------
    ConditionUpdater::ConditionUpdater()
    {
    }

    //--------------------------------------------------------------------
    ConditionUpdater::~ConditionUpdater()
    {
    }

    //--------------------------------------------------------------------
    void ConditionUpdater::notifyPropertyChange( const PropertyChangeEvent& _rEvent )
    {
        if ( !impl_lateInit_nothrow() )
            return;

        Reference< XReportControlModel > xRptControlModel( _rEvent.Source, UNO_QUERY );
        if ( xRptControlModel.is() && _rEvent.PropertyName.equalsAscii( "DataField" ) )
        {
            ::rtl::OUString sOldDataSource, sNewDataSource;
            OSL_VERIFY( _rEvent.OldValue >>= sOldDataSource );
            OSL_VERIFY( _rEvent.NewValue >>= sNewDataSource );
            impl_adjustFormatConditions_nothrow( xRptControlModel, sOldDataSource, sNewDataSource );
        }
    }

    //--------------------------------------------------------------------
    bool ConditionUpdater::impl_lateInit_nothrow()
    {
        if ( !m_aConditionalExpressions.empty() )
            return true;

        ConditionalExpressionFactory::getKnownConditionalExpressions( m_aConditionalExpressions );
        return true;
    }

    //--------------------------------------------------------------------
    void ConditionUpdater::impl_adjustFormatConditions_nothrow( const Reference< XReportControlModel >& _rxRptControlModel,
        const ::rtl::OUString& _rOldDataSource, const ::rtl::OUString& _rNewDataSource )
    {
        try
        {
            ReportFormula aOldContentFormula( _rOldDataSource );
            ::rtl::OUString sOldUnprefixed( aOldContentFormula.getBracketedFieldOrExpression() );
            ReportFormula aNewContentFormula( _rNewDataSource );
            ::rtl::OUString sNewUnprefixed( aNewContentFormula.getBracketedFieldOrExpression() );

            sal_Int32 nCount( _rxRptControlModel->getCount() );
            Reference< XFormatCondition > xFormatCondition;
            ::rtl::OUString sFormulaExpression, sLHS, sRHS;
            for ( sal_Int32 i=0; i<nCount; ++i )
            {
                xFormatCondition.set( _rxRptControlModel->getByIndex( i ), UNO_QUERY_THROW );
                ReportFormula aFormula( xFormatCondition->getFormula() );
                sFormulaExpression = aFormula.getExpression();

                for (   ConditionalExpressions::const_iterator loop = m_aConditionalExpressions.begin();
                        loop != m_aConditionalExpressions.end();
                        ++loop
                    )
                {
                    if ( !loop->second->matchExpression( sFormulaExpression, sOldUnprefixed, sLHS, sRHS ) )
                        continue;

                    // the expression matches -> translate it to the new data source of the report control model
                    sFormulaExpression = loop->second->assembleExpression( sNewUnprefixed, sLHS, sRHS );
                    aFormula = ReportFormula( ReportFormula::Expression, sFormulaExpression );
                    xFormatCondition->setFormula( aFormula.getCompleteFormula() );
                    break;
                }
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

//........................................................................
} // namespace rptui
//........................................................................
