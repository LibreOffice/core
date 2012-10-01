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
#include "conditionupdater.hxx"
#include "reportformula.hxx"

#include <com/sun/star/report/XFormatCondition.hpp>

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
        if ( xRptControlModel.is() && _rEvent.PropertyName == "DataField" )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
