/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wizardcontext.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:33:52 $
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
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
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

