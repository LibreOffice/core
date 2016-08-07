/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbui.hxx"
#ifndef DBAUI_COLUMNCONTROLWINDOW_HXX
#include "ColumnControlWindow.hxx"
#endif
#ifndef DBAUI_FIELDCONTROLS_HXX
#include "FieldControls.hxx"
#endif
#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX
#include <unotools/syslocale.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif


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
//      case tpAutoIncrement:
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
//      case tpAutoIncrement:
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
