/*************************************************************************
 *
 *  $RCSfile: TableFieldControl.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-19 06:04:38 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef DBAUI_TABLEFIELDCONTROL_HXX
#include "TableFieldControl.hxx"
#endif
#ifndef DBUI_TABLECONTROLLER_HXX
#include "TableController.hxx"
#endif
#ifndef DBAUI_TABLEDESIGNVIEW_HXX
#include "TableDesignView.hxx"
#endif
#ifndef DBAUI_TABLEEDITORCONTROL_HXX
#include "TEditControl.hxx"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef DBAUI_TYPEINFO_HXX
#include "TypeInfo.hxx"
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace dbaui;

//------------------------------------------------------------------
void OTableFieldControl::CellModified(long nRow, sal_uInt16 nColId )
{
    GetCtrl()->CellModified(nRow,nColId);
}
//------------------------------------------------------------------
OTableEditorCtrl* OTableFieldControl::GetCtrl()
{
    OTableDesignView* pDesignWin = static_cast<OTableDesignView*>(GetParent()->GetParent()->GetParent()->GetParent());
    OSL_ENSURE(pDesignWin,"no view!");
    return pDesignWin->GetEditorCtrl();
}
//------------------------------------------------------------------
sal_Bool OTableFieldControl::IsReadOnly()
{
    sal_Bool bRead(GetCtrl()->IsReadOnly());
    if( !bRead )
    {
        // Die Spalten einer ::com::sun::star::sdbcx::View können nicht verändert werden
        Reference<XPropertySet> xTable = GetCtrl()->GetView()->getController()->getTable();
        if(xTable.is() && ::comphelper::getString(xTable->getPropertyValue(PROPERTY_TYPE)) == ::rtl::OUString::createFromAscii("VIEW"))
            bRead = sal_True;
        else
        {
            OTableRow* pCurRow = GetCtrl()->GetActRow();
            if( pCurRow )
                bRead = pCurRow->IsReadOnly();
        }
    }
    return bRead;
}
//------------------------------------------------------------------
void OTableFieldControl::ActivateAggregate( EControlType eType )
{
    switch(eType)
    {
        case tpColumnName:
        case tpType:
            break;
        default:
            OFieldDescControl::ActivateAggregate(eType);
    }
}
//------------------------------------------------------------------
void OTableFieldControl::DeactivateAggregate( EControlType eType )
{
    switch(eType)
    {
        case tpColumnName:
        case tpType:
            break;
        default:
            OFieldDescControl::DeactivateAggregate(eType);
    }
}
// -----------------------------------------------------------------------------
void OTableFieldControl::SetModified(BOOL bModified)
{
    GetCtrl()->GetView()->getController()->setModified(bModified);
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData> OTableFieldControl::getMetaData()
{
    Reference<XConnection> xCon = GetCtrl()->GetView()->getController()->getConnection();
    if(!xCon.is())
        return NULL;
    return xCon->getMetaData();
}
// -----------------------------------------------------------------------------
Reference< XNumberFormatter >   OTableFieldControl::GetFormatter()
{
    return GetCtrl()->GetView()->getController()->getNumberFormatter();
}
// -----------------------------------------------------------------------------
const OTypeInfo* OTableFieldControl::getTypeInfo(sal_Int32 _nPos)
{
    return GetCtrl()->GetView()->getController()->getTypeInfo(_nPos);
}
// -----------------------------------------------------------------------------
const OTypeInfoMap* OTableFieldControl::getTypeInfo() const
{
    return const_cast<OTableFieldControl*>(this)->GetCtrl()->GetView()->getController()->getTypeInfo();
}
// -----------------------------------------------------------------------------
Locale OTableFieldControl::GetLocale() const
{
    return const_cast<OTableFieldControl*>(this)->GetCtrl()->GetView()->getLocale();
}
// -----------------------------------------------------------------------------




