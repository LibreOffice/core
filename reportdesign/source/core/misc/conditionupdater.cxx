/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: conditionupdater.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-02 14:31:20 $
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

#include "conditionupdater.hxx"
#include "reportformula.hxx"

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_REPORT_XFORMATCONDITION_HPP_
#include <com/sun/star/report/XFormatCondition.hpp>
#endif
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
