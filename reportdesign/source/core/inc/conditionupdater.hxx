/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: conditionupdater.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-02 14:31:03 $
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

#ifndef CONDITIONUPDATER_HXX
#define CONDITIONUPDATER_HXX

#include "conditionalexpression.hxx"

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_BEANS_PROPERTYCHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_XREPORTCONTROLMODEL_HPP_
#include <com/sun/star/report/XReportControlModel.hpp>
#endif
/** === end UNO includes === **/

#include <boost/noncopyable.hpp>

//........................................................................
namespace rptui
{
//........................................................................

    //====================================================================
    //= ConditionUpdater
    //====================================================================
    class ConditionUpdater : public ::boost::noncopyable
    {
    public:
        ConditionUpdater();
        ~ConditionUpdater();

        /// notifies the object about the change of a property value, somewhere in the report definition
        void    notifyPropertyChange( const ::com::sun::star::beans::PropertyChangeEvent& _rEvent );

    private:
        /// does late initializations, return whether or not successful
        bool impl_lateInit_nothrow();

        void impl_adjustFormatConditions_nothrow(
            const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportControlModel >& _rxRptControlModel,
            const ::rtl::OUString& _rOldDataSource,
            const ::rtl::OUString& _rNewDataSource
        );

    private:
        ConditionalExpressions  m_aConditionalExpressions;
    };

//........................................................................
} // namespace rptui
//........................................................................

#endif // CONDITIONUPDATER_HXX
