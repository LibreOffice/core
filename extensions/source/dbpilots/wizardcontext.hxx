/*************************************************************************
 *
 *  $RCSfile: wizardcontext.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-05 14:52:51 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _EXTENSIONS_DBP_WIZARDCONTEXT_HXX_
#define _EXTENSIONS_DBP_WIZARDCONTEXT_HXX_

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSET_HPP_
#include <com/sun/star/sdbc/XRowSet.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XCONTROLSHAPE_HPP_
#include <com/sun/star/drawing/XControlShape.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

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
        // the column container of the object the form is bound to (table, query or SQL statement)
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                    xFields;
        // the column names of the object the form is bound to (table, query or SQL statement)
        ::com::sun::star::uno::Sequence< ::rtl::OUString >
                    aFieldNames;
    };

//.........................................................................
}   // namespace dbp
//.........................................................................

#endif // _EXTENSIONS_DBP_WIZARDCONTEXT_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 *  Revision 1.0 05.03.01 13:40:41  fs
 ************************************************************************/

