/*************************************************************************
 *
 *  $RCSfile: QTableWindow.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-14 14:54:11 $
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
#ifndef DBAUI_QUERY_TABLEWINDOWDATA_HXX
#include "QTableWindow.hxx"
#endif
#ifndef DBAUI_QUERYTABLEVIEW_HXX
#include "QueryTableView.hxx"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#include "dbaccess_helpid.hrc"
#ifndef DBAUI_QUERYDESIGNVIEW_HXX
#include "QueryDesignView.hxx"
#endif
#ifndef DBACCESS_UI_BROWSER_ID_HXX
#include "browserids.hxx"
#endif
#ifndef DBAUI_QUERYCONTROLLER_HXX
#include "querycontroller.hxx"
#endif
#ifndef _SV_IMAGE_HXX
#include <vcl/image.hxx>
#endif
#ifndef DBAUI_TABLEWINDOWLISTBOX_HXX
#include "TableWindowListBox.hxx"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef DBAUI_QUERY_HRC
#include "Query.hrc"
#endif
#ifndef _COM_SUN_STAR_SDBCX_XKEYSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_KEYTYPE_HPP_
#include <com/sun/star/sdbcx/KeyType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef DBAUI_TABLEFIELDINFO_HXX
#include "TableFieldInfo.hxx"
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif


using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace dbaui;
TYPEINIT1(OQueryTableWindow, OTableWindow);
//========================================================================
// class OQueryTableWindow
//========================================================================
DBG_NAME(OQueryTableWindow);
//------------------------------------------------------------------------------
OQueryTableWindow::OQueryTableWindow( Window* pParent, OQueryTableWindowData* pTabWinData, sal_Unicode* pszInitialAlias)
    :OTableWindow( pParent, pTabWinData )
    ,m_nAliasNum(0)
{
    DBG_CTOR(OQueryTableWindow,NULL);
    if (pszInitialAlias != NULL)
        m_strInitialAlias = ::rtl::OUString(pszInitialAlias);
    else
        m_strInitialAlias = pTabWinData->GetAliasName();

    // wenn der Tabellen- gleich dem Aliasnamen ist, dann darf ich das nicht an InitialAlias weiterreichen, denn das Anhaengen
    // eines eventuelle Tokens nicht klappen ...
    if (m_strInitialAlias == pTabWinData->GetTableName())
        m_strInitialAlias = ::rtl::OUString();

    SetHelpId(HID_CTL_QRYDGNTAB);
}

//------------------------------------------------------------------------------
OQueryTableWindow::~OQueryTableWindow()
{
    EmptyListBox();
    DBG_DTOR(OQueryTableWindow,NULL);
}

//------------------------------------------------------------------------------
sal_Bool OQueryTableWindow::Init()
{
    sal_Bool bSuccess = OTableWindow::Init();

    OQueryTableView* pContainer = static_cast<OQueryTableView*>(getTableView());

    // zuerst Alias bestimmen
    ::rtl::OUString strAliasName;

    OTableWindowData* pWinData = GetData();
    DBG_ASSERT(pWinData->ISA(OQueryTableWindowData), "OQueryTableWindow::Init() : habe keine OQueryTableWindowData");

    if (m_strInitialAlias.getLength() )
        // Der Alias wurde explizit mit angegeben
        strAliasName = m_strInitialAlias;
    else
    {
        ::rtl::OUString aInitialTitle = pWinData->GetTableName();
        sal_Bool bOwner =  sal_False;
        if(GetTable().is())
        {
            ::rtl::OUString sName;
            GetTable()->getPropertyValue(PROPERTY_NAME) >>= sName;
            strAliasName = sName.getStr();
        }
    }

    // Alias mit fortlaufender Nummer versehen
    if (pContainer->CountTableAlias(strAliasName, m_nAliasNum))
    {
        strAliasName += ::rtl::OUString('_');
        strAliasName += ::rtl::OUString::valueOf(m_nAliasNum);
    }


    strAliasName = String(strAliasName).EraseAllChars('"');
    SetAliasName(strAliasName);
        // SetAliasName reicht das als WinName weiter, dadurch benutzt es die Basisklasse
    // reset the titel
    m_aTitle.SetText( pWinData->GetWinName() );
    m_aTitle.Show();

    //  sal_Bool bSuccess(sal_True);
    if (!bSuccess)
    {   // es soll nur ein Dummy-Window aufgemacht werden ...
        DBG_ASSERT(GetAliasName().getLength(), "OQueryTableWindow::Init : kein Alias- UND kein Tabellenname geht nicht !");
            // .. aber das braucht wenigstens einen Alias

        // ::com::sun::star::form::ListBox anlegen
        if (!m_pListBox)
            m_pListBox = CreateListBox();

        // Titel setzen
        m_aTitle.SetText(GetAliasName());
        m_aTitle.Show();

        EmptyListBox();
            // neu zu fuellen brauche ich die nicht, da ich ja keine Tabelle habe
        m_pListBox->Show();
    }

    getTableView()->getDesignView()->getController()->InvalidateFeature(ID_BROWSER_QUERY_EXECUTE);
    return bSuccess;
}
//------------------------------------------------------------------------------
sal_Bool OQueryTableWindow::FillListBox()
{
    ImageList aImageList(ModuleRes(IMG_JOINS));
    Image aPrimKeyImage = aImageList.GetImage(IMG_PRIMARY_KEY);



    // first we need the keys from the table
    Reference<XKeysSupplier> xKeys(GetTable(),UNO_QUERY);
    Reference<XNameAccess> xPKeyColumns;
    if(xKeys.is())
    {
        Reference< XIndexAccess> xKeyIndex = xKeys->getKeys();
        Reference<XColumnsSupplier> xColumnsSupplier;
        // search the one and only primary key
        for(sal_Int32 i=0;i< xKeyIndex->getCount();++i)
        {
            Reference<XPropertySet> xProp;
            ::cppu::extractInterface(xProp,xKeyIndex->getByIndex(i));
            sal_Int32 nKeyType = 0;
            xProp->getPropertyValue(PROPERTY_TYPE) >>= nKeyType;
            if(KeyType::PRIMARY == nKeyType)
            {
                xColumnsSupplier = Reference<XColumnsSupplier>(xProp,UNO_QUERY);
                break;
            }
        }
        if(xColumnsSupplier.is())
            xPKeyColumns = xColumnsSupplier->getColumns();
    }

    // first we need a *
    SvLBoxEntry* pEntry = NULL;
    if (GetData()->IsShowAll())
    {
        pEntry = m_pListBox->InsertEntry( ::rtl::OUString::createFromAscii("*") );
        pEntry->SetUserData( new OTableFieldInfo() );
    }

    Sequence< ::rtl::OUString> aColumns = GetOriginalColumns()->getElementNames();
    const ::rtl::OUString* pBegin = aColumns.getConstArray();
    const ::rtl::OUString* pEnd = pBegin + aColumns.getLength();

    for (; pBegin != pEnd; ++pBegin)
    {
        OTableFieldInfo* pInfo = new OTableFieldInfo();
        // is this column in the primary key
        if (xPKeyColumns.is() && xPKeyColumns->hasByName(*pBegin))
        {
            pEntry = m_pListBox->InsertEntry(*pBegin, aPrimKeyImage, aPrimKeyImage);
            pInfo->SetKey(TAB_PRIMARY_FIELD);
        }
        else
        {
            pEntry = m_pListBox->InsertEntry(*pBegin);
            pInfo->SetKey(TAB_NORMAL_FIELD);
        }
        Reference<XPropertySet> xColumn;
        ::cppu::extractInterface(xColumn,GetOriginalColumns()->getByName(*pBegin));
        OSL_ENSURE(xColumn.is(),"No column!");
        pInfo->SetDataType(::comphelper::getINT32(xColumn->getPropertyValue(PROPERTY_TYPE)));
        pEntry->SetUserData( pInfo );
    }
    return sal_True;
}

//------------------------------------------------------------------------------
void OQueryTableWindow::EmptyListBox()
{
    if(m_pListBox)
    {
        SvLBoxEntry* pEntry = m_pListBox->First();

        while(pEntry)
        {
            OTableFieldInfo* pInf = (OTableFieldInfo*)pEntry->GetUserData();
            delete pInf;

            SvLBoxEntry* pNextEntry = m_pListBox->Next(pEntry);
            m_pListBox->GetModel()->Remove(pEntry);
            pEntry = pNextEntry;
        }
    }
}

//------------------------------------------------------------------------------
void OQueryTableWindow::OnEntryDoubleClicked(SvLBoxEntry* pEntry)
{
    DBG_ASSERT(pEntry != NULL, "OQueryTableWindow::OnEntryDoubleClicked : pEntry darf nicht NULL sein !");
        // man koennte das auch abfragen und dann ein return hinsetzen, aber so weist es vielleicht auf Fehler bei Aufrufer hin

    if (getTableView()->getDesignView()->getController()->isReadOnly())
        return;

    OTableFieldInfo* pInf = static_cast<OTableFieldInfo*>(pEntry->GetUserData());
    DBG_ASSERT(pInf != NULL, "OQueryTableWindow::OnEntryDoubleClicked : Feld hat keine FieldInfo !");

    // eine DragInfo aufbauen
    OTableFieldDesc aInfo;
    aInfo.SetTabWindow(this);
    aInfo.SetField(m_pListBox->GetEntryText(pEntry));
    aInfo.SetTable(GetTableName());
    aInfo.SetAlias(GetAliasName());
    aInfo.SetDatabase(GetComposedName());
    aInfo.SetFieldIndex(m_pListBox->GetModel()->GetAbsPos(pEntry));
    aInfo.SetDataType(pInf->GetDataType());

    // und das entsprechende Feld einfuegen
    static_cast<OQueryTableView*>(getTableView())->InsertField(aInfo);
}

//------------------------------------------------------------------------------
sal_Bool OQueryTableWindow::ExistsField(const ::rtl::OUString& strFieldName, OTableFieldDesc& rInfo)
{
    DBG_ASSERT(m_pListBox != NULL, "OQueryTableWindow::ExistsField : habe keine ::com::sun::star::form::ListBox !");
    SvLBoxEntry* pEntry = m_pListBox->First();
    ::comphelper::UStringMixEqual bCase(getTableView()->getDesignView()->getController()->getConnection()->getMetaData()->storesMixedCaseQuotedIdentifiers());

    while (pEntry)
    {
        if (bCase(strFieldName,::rtl::OUString(m_pListBox->GetEntryText(pEntry))))
        {
            OTableFieldInfo* pInf = static_cast<OTableFieldInfo*>(pEntry->GetUserData());
            DBG_ASSERT(pInf != NULL, "OQueryTableWindow::ExistsField : Feld hat keine FieldInfo !");

            rInfo.SetTabWindow(this);
            rInfo.SetField(strFieldName);
            rInfo.SetTable(GetTableName());
            rInfo.SetAlias(GetAliasName());
            rInfo.SetDatabase(GetComposedName());
            rInfo.SetFieldIndex(m_pListBox->GetModel()->GetAbsPos(pEntry));
            rInfo.SetDataType(pInf->GetDataType());
            return sal_True;
        }
        pEntry = m_pListBox->Next(pEntry);
    }

    return sal_False;
}

//------------------------------------------------------------------------------
sal_Bool OQueryTableWindow::ExistsAVisitedConn() const
{
    return static_cast<const OQueryTableView*>(getTableView())->ExistsAVisitedConn(this);
}

//------------------------------------------------------------------------------
void OQueryTableWindow::KeyInput( const KeyEvent& rEvt )
{
    OTableWindow::KeyInput( rEvt );
}
// -----------------------------------------------------------------------------



