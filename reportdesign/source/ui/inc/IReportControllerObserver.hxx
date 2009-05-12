/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: UndoEnv.hxx,v $
 * $Revision: 1.4 $
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

#ifndef INCLUDED_BEAUTIFIER_HXX
#define INCLUDED_BEAUTIFIER_HXX

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_reportdesign.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
/** === end UNO includes === **/

namespace rptui
{
    /* abstract */ class IReportControllerObserver
    {
    protected:
        // IReportControllerObserver(){}
        virtual ~IReportControllerObserver() {}

    public:

        virtual void notifyPropertyChange( const ::com::sun::star::beans::PropertyChangeEvent& _rEvent ) = 0;
        virtual void notifyElementInserted( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxElement ) = 0;
        virtual void handle( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxElement ) = 0;
    };

} // namespace rptui

#endif


