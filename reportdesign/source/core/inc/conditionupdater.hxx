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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
