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



#ifndef CONDITIONUPDATER_HXX
#define CONDITIONUPDATER_HXX

#include "conditionalexpression.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/report/XReportControlModel.hpp>
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
