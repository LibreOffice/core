/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "TableFieldControl.hxx"
#include <TableController.hxx>
#include <TableDesignView.hxx>
#include "TEditControl.hxx"
#include <stringconstants.hxx>
#include <strings.hxx>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <comphelper/types.hxx>
#include <TypeInfo.hxx>
#include <UITools.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace dbaui;

OTableFieldControl::OTableFieldControl( vcl::Window* pParent, OTableDesignHelpBar* pHelpBar) :OFieldDescControl(pParent,pHelpBar)
{
}

void OTableFieldControl::CellModified(long nRow, sal_uInt16 nColId )
{
    GetCtrl()->CellModified(nRow,nColId);
}

OTableEditorCtrl* OTableFieldControl::GetCtrl() const
{
    OTableDesignView* pDesignWin = static_cast<OTableDesignView*>(GetParent()->GetParent()->GetParent()->GetParent());
    OSL_ENSURE(pDesignWin,"no view!");
    return pDesignWin->GetEditorCtrl();
}

bool OTableFieldControl::IsReadOnly()
{
    bool bRead(GetCtrl()->IsReadOnly());
    if( !bRead )
    {
        // The columns of a css::sdbcx::View could not be locked
        Reference<XPropertySet> xTable = GetCtrl()->GetView()->getController().getTable();
        if(xTable.is() && ::comphelper::getString(xTable->getPropertyValue(PROPERTY_TYPE)) == "VIEW")
            bRead = true;
        else
        {
            std::shared_ptr<OTableRow>  pCurRow = GetCtrl()->GetActRow();
            if( pCurRow )
                bRead = pCurRow->IsReadOnly();
        }
    }
    return bRead;
}

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

void OTableFieldControl::SetModified(bool bModified)
{
    GetCtrl()->GetView()->getController().setModified(bModified);
}

css::uno::Reference< css::sdbc::XConnection> OTableFieldControl::getConnection()
{
    return GetCtrl()->GetView()->getController().getConnection();
}

css::uno::Reference< css::sdbc::XDatabaseMetaData> OTableFieldControl::getMetaData()
{
    Reference<XConnection> xCon = GetCtrl()->GetView()->getController().getConnection();
    if(!xCon.is())
        return nullptr;
    return xCon->getMetaData();
}

Reference< XNumberFormatter >   OTableFieldControl::GetFormatter() const
{
    return GetCtrl()->GetView()->getController().getNumberFormatter();
}

TOTypeInfoSP OTableFieldControl::getTypeInfo(sal_Int32 _nPos)
{
    return GetCtrl()->GetView()->getController().getTypeInfo(_nPos);
}

const OTypeInfoMap* OTableFieldControl::getTypeInfo() const
{
    return &GetCtrl()->GetView()->getController().getTypeInfo();
}

Locale OTableFieldControl::GetLocale() const
{
    return GetCtrl()->GetView()->getLocale();
}

bool OTableFieldControl::isAutoIncrementValueEnabled() const
{
    return GetCtrl()->GetView()->getController().isAutoIncrementValueEnabled();
}

OUString OTableFieldControl::getAutoIncrementValue() const
{
    return GetCtrl()->GetView()->getController().getAutoIncrementValue();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
