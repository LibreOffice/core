/*************************************************************************
 *
 *  $RCSfile: AccessibleSpreadsheet.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sab $ $Date: 2002-01-18 09:54:14 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#include "AccessibleSpreadsheet.hxx"

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEROLE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEET_HPP_
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XCELLRANGEADDRESSABLE_HPP_
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYCHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#endif

#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif


using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::accessibility;

//=====  internal  ============================================================

ScAccessibleSpreadsheet::ScAccessibleSpreadsheet (
        const uno::Reference<XAccessible>& rxParent,
        const uno::Reference<sheet::XSpreadsheetView >& rxSheetView)
    :
    ScAccessibleTableBase (rxParent, rxSheetView, getRange(rxSheetView))
{
}

ScAccessibleSpreadsheet::~ScAccessibleSpreadsheet ()
{
}

    //=====  XServiceInfo  ====================================================

::rtl::OUString SAL_CALL ScAccessibleSpreadsheet::getImplementationName (void)
        throw (uno::RuntimeException)
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("ScAccessibleSpreadsheet"));
}

    //====  internal  =========================================================

table::CellRangeAddress
    ScAccessibleSpreadsheet::getRange(
    const uno::Reference<sheet::XSpreadsheetView>& rxSheetView)
{
    table::CellRangeAddress aRangeAddress;
    if (rxSheetView.is())
    {
        uno::Reference<sheet::XCellRangeAddressable> xCellRangeAddressable(
            rxSheetView->getActiveSheet(), uno::UNO_QUERY);
        if (xCellRangeAddressable.is())
            aRangeAddress = xCellRangeAddressable->getRangeAddress();
    }

    return aRangeAddress;
}

