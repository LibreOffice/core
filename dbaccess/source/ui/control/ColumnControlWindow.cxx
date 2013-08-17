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

#include "ColumnControlWindow.hxx"
#include "FieldControls.hxx"
#include <unotools/syslocale.hxx>
#include <connectivity/dbtools.hxx>
#include "UITools.hxx"
#include "dbu_resource.hrc"
#include <comphelper/processfactory.hxx>
#include <com/sun/star/util/NumberFormatter.hpp>

using namespace ::dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;

// OColumnControlWindow
DBG_NAME(OColumnControlWindow)
OColumnControlWindow::OColumnControlWindow(Window* pParent
                                           ,const Reference<XComponentContext>& _rxContext)
            : OFieldDescControl(pParent,NULL)
            , m_xContext(_rxContext)
            , m_sTypeNames(ModuleRes(STR_TABLEDESIGN_DBFIELDTYPES))
            , m_bAutoIncrementEnabled(sal_True)
{
    DBG_CTOR(OColumnControlWindow,NULL);

    setRightAligned();
    m_aLocale = SvtSysLocale().GetLanguageTag().getLocale();
}

OColumnControlWindow::~OColumnControlWindow()
{

    DBG_DTOR(OColumnControlWindow,NULL);
}

void OColumnControlWindow::ActivateAggregate( EControlType eType )
{
    switch(eType )
    {
        case tpFormat:
        case tpDefault:
        case tpColumnName:
            break;
        default:
            OFieldDescControl::ActivateAggregate( eType );
    }
}

void OColumnControlWindow::DeactivateAggregate( EControlType eType )
{
    switch(eType )
    {
        case tpFormat:
        case tpDefault:
        case tpColumnName:
            break;
        default:
            OFieldDescControl::DeactivateAggregate( eType );
    }
}

void OColumnControlWindow::CellModified(long /*nRow*/, sal_uInt16 /*nColId*/ )
{
    saveCurrentFieldDescData();
}

::com::sun::star::lang::Locale  OColumnControlWindow::GetLocale() const
{
    return m_aLocale;
}

Reference< XNumberFormatter > OColumnControlWindow::GetFormatter() const
{
    if ( !m_xFormatter.is() )
        try
        {
            Reference< XNumberFormatsSupplier >  xSupplier(::dbtools::getNumberFormats(m_xConnection, sal_True, m_xContext));

            if ( xSupplier.is() )
            {
                // create a new formatter
                m_xFormatter.set( NumberFormatter::create(m_xContext), UNO_QUERY_THROW);
                m_xFormatter->attachNumberFormatsSupplier(xSupplier);
            }
        }
        catch(Exception&)
        {
        }
    return m_xFormatter;
}

TOTypeInfoSP OColumnControlWindow::getTypeInfo(sal_Int32 _nPos)
{
    return ( _nPos >= 0 && _nPos < static_cast<sal_Int32>(m_aDestTypeInfoIndex.size())) ? m_aDestTypeInfoIndex[_nPos]->second : TOTypeInfoSP();
}

const OTypeInfoMap* OColumnControlWindow::getTypeInfo() const
{
    return &m_aDestTypeInfo;
}

Reference< XDatabaseMetaData> OColumnControlWindow::getMetaData()
{
    if ( m_xConnection.is() )
        return m_xConnection->getMetaData();
    return Reference< XDatabaseMetaData>();
}

Reference< XConnection> OColumnControlWindow::getConnection()
{
    return m_xConnection;
}

void OColumnControlWindow::setConnection(const Reference< XConnection>& _xCon)
{
    m_xConnection = _xCon;
    m_xFormatter = NULL;
    m_aDestTypeInfoIndex.clear();
    m_aDestTypeInfo.clear();

    if ( m_xConnection.is() )
    {
        Init();

        ::dbaui::fillTypeInfo(m_xConnection,m_sTypeNames,m_aDestTypeInfo,m_aDestTypeInfoIndex);
        // read autoincrement value set in the datasource
        ::dbaui::fillAutoIncrementValue(m_xConnection,m_bAutoIncrementEnabled,m_sAutoIncrementValue);
    }
}

sal_Bool OColumnControlWindow::isAutoIncrementValueEnabled() const
{
    return m_bAutoIncrementEnabled;
}

OUString OColumnControlWindow::getAutoIncrementValue() const
{
    return m_sAutoIncrementValue;
}

TOTypeInfoSP OColumnControlWindow::getDefaultTyp() const
{
    if ( !m_pTypeInfo.get() )
    {
        m_pTypeInfo = TOTypeInfoSP(new OTypeInfo());
        m_pTypeInfo->aUIName = m_sTypeNames.GetToken(TYPE_OTHER);
    }
    return m_pTypeInfo;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
