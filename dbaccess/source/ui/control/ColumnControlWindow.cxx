/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ColumnControlWindow.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-23 12:22:29 $
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
#ifndef DBAUI_COLUMNCONTROLWINDOW_HXX
#include "ColumnControlWindow.hxx"
#endif
#ifndef DBAUI_FIELDCONTROLS_HXX
#include "FieldControls.hxx"
#endif
#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX
#include <svtools/syslocale.hxx>
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

    m_bRight = sal_True;
    try
    {
        m_aLocale = SvtSysLocale().GetLocaleData().getLocale();
    }
    catch(Exception&)
    {
    }
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
// -----------------------------------------------------------------------
void OColumnControlWindow::SetModified(sal_Bool bModified)
{
}
// -----------------------------------------------------------------------------
void OColumnControlWindow::CellModified(long nRow, USHORT nColId )
{
    SaveData(pActFieldDescr);
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
