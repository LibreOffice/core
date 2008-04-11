/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: wizardcontext.hxx,v $
 * $Revision: 1.6 $
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

#ifndef _EXTENSIONS_DBP_WIZARDCONTEXT_HXX_
#define _EXTENSIONS_DBP_WIZARDCONTEXT_HXX_

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <comphelper/stl_types.hxx>

//.........................................................................
namespace dbp
{
//.........................................................................

    //=====================================================================
    //= OControlWizardContext
    //=====================================================================
    struct OControlWizardContext
    {
        // the global data source context
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                    xDatasourceContext;

        // the control mode
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                    xObjectModel;
        // the form the control model belongs to
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                    xForm;
        // the form as rowset
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >
                    xRowSet;

        // the model of the document
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
                    xDocumentModel;
        // the page where the control mode resides
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >
                    xDrawPage;
        // the shape which carries the control
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XControlShape >
                    xObjectShape;

        // the tables or queries of the data source the form is bound to (if any)
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                    xObjectContainer;
        // the column types container of the object the form is bound to (table, query or SQL statement)
        DECLARE_STL_USTRINGACCESS_MAP(sal_Int32,TNameTypeMap);
        TNameTypeMap aTypes;
        // the column names of the object the form is bound to (table, query or SQL statement)
        ::com::sun::star::uno::Sequence< ::rtl::OUString >
                    aFieldNames;

        sal_Bool bEmbedded;
    };

//.........................................................................
}   // namespace dbp
//.........................................................................

#endif // _EXTENSIONS_DBP_WIZARDCONTEXT_HXX_

