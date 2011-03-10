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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#include "ColumnControlWindow.hxx"
#include "FieldControls.hxx"
#include <unotools/syslocale.hxx>
#include <connectivity/dbtools.hxx>
#include "UITools.hxx"
#include "dbu_resource.hrc"


using namespace ::dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;

//========================================================================
// OColumnControlWindow
DBG_NAME(OColumnControlWindow)
//========================================================================
OColumnControlWindow::OColumnControlWindow(Window* pParent
                                           ,const Reference<XMultiServiceFactory>& _rxFactory)
            : OFieldDescControl(pParent,NULL)
            , m_xORB(_rxFactory)
            , m_sTypeNames(ModuleRes(STR_TABLEDESIGN_DBFIELDTYPES))
            , m_bAutoIncrementEnabled(sal_True)
{
    DBG_CTOR(OColumnControlWindow,NULL);

    setRightAligned();
    m_aLocale = SvtSysLocale().GetLocaleData().getLocale();
}
// -----------------------------------------------------------------------------
OColumnControlWindow::~OColumnControlWindow()
{

    DBG_DTOR(OColumnControlWindow,NULL);
}
// -----------------------------------------------------------------------
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
// -----------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
void OColumnControlWindow::CellModified(long /*nRow*/, sal_uInt16 /*nColId*/ )
{
    saveCurrentFieldDescData();
}
// -----------------------------------------------------------------------------
::com::sun::star::lang::Locale  OColumnControlWindow::GetLocale() const
{
    return m_aLocale;
}
// -----------------------------------------------------------------------------
Reference< XNumberFormatter > OColumnControlWindow::GetFormatter() const
{
    if ( !m_xFormatter.is() )
        try
        {
            Reference< XNumberFormatsSupplier >  xSupplier(::dbtools::getNumberFormats(m_xConnection, sal_True,m_xORB));

            if ( xSupplier.is() )
            {
                // create a new formatter
                m_xFormatter.set( m_xORB->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.NumberFormatter"))), UNO_QUERY);
                if (m_xFormatter.is())
                    m_xFormatter->attachNumberFormatsSupplier(xSupplier);
            }
        }
        catch(Exception&)
        {
        }
    return m_xFormatter;
}
// -----------------------------------------------------------------------------
TOTypeInfoSP OColumnControlWindow::getTypeInfo(sal_Int32 _nPos)
{
    return ( _nPos >= 0 && _nPos < static_cast<sal_Int32>(m_aDestTypeInfoIndex.size())) ? m_aDestTypeInfoIndex[_nPos]->second : TOTypeInfoSP();
}
// -----------------------------------------------------------------------------
const OTypeInfoMap* OColumnControlWindow::getTypeInfo() const
{
    return &m_aDestTypeInfo;
}
// -----------------------------------------------------------------------------
Reference< XDatabaseMetaData> OColumnControlWindow::getMetaData()
{
    if ( m_xConnection.is() )
        return m_xConnection->getMetaData();
    return Reference< XDatabaseMetaData>();
}
// -----------------------------------------------------------------------------
Reference< XConnection> OColumnControlWindow::getConnection()
{
    return m_xConnection;
}
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
sal_Bool OColumnControlWindow::isAutoIncrementValueEnabled() const
{
    return m_bAutoIncrementEnabled;
}
// -----------------------------------------------------------------------------
::rtl::OUString OColumnControlWindow::getAutoIncrementValue() const
{
    return m_sAutoIncrementValue;
}
// -----------------------------------------------------------------------------
TOTypeInfoSP OColumnControlWindow::getDefaultTyp() const
{
    if ( !m_pTypeInfo.get() )
    {
        m_pTypeInfo = TOTypeInfoSP(new OTypeInfo());
        m_pTypeInfo->aUIName = m_sTypeNames.GetToken(TYPE_OTHER);
    }
    return m_pTypeInfo;
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
