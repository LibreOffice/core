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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
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
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace dbaui;
OTableFieldControl::OTableFieldControl( Window* pParent, OTableDesignHelpBar* pHelpBar) :OFieldDescControl(pParent,pHelpBar)
{
}
//------------------------------------------------------------------
void OTableFieldControl::CellModified(long nRow, sal_uInt16 nColId )
{
    GetCtrl()->CellModified(nRow,nColId);
}
//------------------------------------------------------------------
OTableEditorCtrl* OTableFieldControl::GetCtrl() const
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
        Reference<XPropertySet> xTable = GetCtrl()->GetView()->getController().getTable();
        if(xTable.is() && ::comphelper::getString(xTable->getPropertyValue(PROPERTY_TYPE)) == ::rtl::OUString::createFromAscii("VIEW"))
            bRead = sal_True;
        else
        {
             ::boost::shared_ptr<OTableRow>  pCurRow = GetCtrl()->GetActRow();
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
void OTableFieldControl::SetModified(sal_Bool bModified)
{
    GetCtrl()->GetView()->getController().setModified(bModified);
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> OTableFieldControl::getConnection()
{
    return GetCtrl()->GetView()->getController().getConnection();
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData> OTableFieldControl::getMetaData()
{
    Reference<XConnection> xCon = GetCtrl()->GetView()->getController().getConnection();
    if(!xCon.is())
        return NULL;
    return xCon->getMetaData();
}
// -----------------------------------------------------------------------------
Reference< XNumberFormatter >   OTableFieldControl::GetFormatter() const
{
    return GetCtrl()->GetView()->getController().getNumberFormatter();
}
// -----------------------------------------------------------------------------
TOTypeInfoSP OTableFieldControl::getTypeInfo(sal_Int32 _nPos)
{
    return GetCtrl()->GetView()->getController().getTypeInfo(_nPos);
}
// -----------------------------------------------------------------------------
const OTypeInfoMap* OTableFieldControl::getTypeInfo() const
{
    return const_cast<OTableFieldControl*>(this)->GetCtrl()->GetView()->getController().getTypeInfo();
}
// -----------------------------------------------------------------------------
Locale OTableFieldControl::GetLocale() const
{
    return const_cast<OTableFieldControl*>(this)->GetCtrl()->GetView()->getLocale();
}
// -----------------------------------------------------------------------------
sal_Bool OTableFieldControl::isAutoIncrementValueEnabled() const
{
    return const_cast<OTableFieldControl*>(this)->GetCtrl()->GetView()->getController().isAutoIncrementValueEnabled();
}
// -----------------------------------------------------------------------------
::rtl::OUString OTableFieldControl::getAutoIncrementValue() const
{
    return const_cast<OTableFieldControl*>(this)->GetCtrl()->GetView()->getController().getAutoIncrementValue();
}
// -----------------------------------------------------------------------------




