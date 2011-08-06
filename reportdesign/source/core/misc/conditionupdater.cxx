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
        if ( xRptControlModel.is() && _rEvent.PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "DataField" ) ) )
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
