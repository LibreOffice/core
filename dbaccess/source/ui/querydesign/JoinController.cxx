/*************************************************************************
 *
 *  $RCSfile: JoinController.cxx,v $
 *
 *  $Revision: 1.30 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:12:25 $
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
#ifndef _SFXSIDS_HRC
#include <sfx2/sfxsids.hrc>
#endif
#ifndef _DBU_QRY_HRC_
#include "dbu_qry.hrc"
#endif
#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif
#ifndef DBACCESS_UI_BROWSER_ID_HXX
#include "browserids.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDATADESCRIPTORFACTORY_HPP_
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_KEYTYPE_HPP_
#include <com/sun/star/sdbcx/KeyType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDROP_HPP_
#include <com/sun/star/sdbcx/XDrop.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XALTERTABLE_HPP_
#include <com/sun/star/sdbcx/XAlterTable.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XAPPEND_HPP_
#include <com/sun/star/sdbcx/XAppend.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_SQLWARNING_HPP_
#include <com/sun/star/sdbc/SQLWarning.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
#ifndef _COMPHELPER_STREAMSECTION_HXX_
#include <comphelper/streamsection.hxx>
#endif
#ifndef _COMPHELPER_BASIC_IO_HXX_
#include <comphelper/basicio.hxx>
#endif
#ifndef _COMPHELPER_SEQSTREAM_HXX
#include <comphelper/seqstream.hxx>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
#endif
#ifndef DBAUI_JOINCONTROLLER_HXX
#include "JoinController.hxx"
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef DBAUI_TABLEWINDOWDATA_HXX
#include "TableWindowData.hxx"
#endif
#ifndef DBAUI_TABLEWINDOW_HXX
#include "TableWindow.hxx"
#endif
#ifndef DBAUI_TABLECONNECTIONDATA_HXX
#include "TableConnectionData.hxx"
#endif
#ifndef DBAUI_QYDLGTAB_HXX
#include "adtabdlg.hxx"
#endif
#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::dbtools;
using namespace ::dbaui;
using namespace ::comphelper;

DBG_NAME(OJoinController);
// -----------------------------------------------------------------------------
OJoinController::OJoinController(const Reference< XMultiServiceFactory >& _rM) : OJoinController_BASE(_rM)
    ,m_bViewsAllowed(sal_True)
    ,m_pAddTabDlg(NULL)
{
    DBG_CTOR(OJoinController,NULL);
}
// -----------------------------------------------------------------------------
OJoinController::~OJoinController()
{
    DBG_DTOR(OJoinController,NULL);
}

// -----------------------------------------------------------------------------
void SAL_CALL OJoinController::disposing( const EventObject& _rSource ) throw(RuntimeException)
{
    OJoinController_BASE::disposing( _rSource );
}

// -----------------------------------------------------------------------------
OJoinDesignView* OJoinController::getJoinView()
{
    return static_cast< OJoinDesignView* >( getView() );
}

// -----------------------------------------------------------------------------
void OJoinController::disposing()
{
    OJoinController_BASE::disposing();

    m_pView         = NULL;
    m_pAddTabDlg    = NULL;

    {
        ::std::vector< OTableConnectionData*>::iterator aIter = m_vTableConnectionData.begin();
        for(;aIter != m_vTableConnectionData.end();++aIter)
            delete *aIter;
        m_vTableConnectionData.clear();
    }
    {
        ::std::vector< OTableWindowData*>::iterator aIter = m_vTableData.begin();
        for(;aIter != m_vTableData.end();++aIter)
            delete *aIter;
        m_vTableData.clear();
    }
}
// -----------------------------------------------------------------------------
void OJoinController::setModified(sal_Bool _bModified)
{
    OJoinController_BASE::setModified(_bModified);
    InvalidateFeature(ID_RELATION_ADD_RELATION);
}
// -----------------------------------------------------------------------------
void OJoinController::SaveTabWinPosSize(OTableWindow* pTabWin, long nOffsetX, long nOffsetY)
{
    // die Daten zum Fenster
    OTableWindowData* pData = pTabWin->GetData();
    OSL_ENSURE(pData != NULL, "SaveTabWinPosSize : TabWin hat keine Daten !");

    // Position & Size der Daten neu setzen (aus den aktuellen Fenster-Parametern)
    Point aPos = pTabWin->GetPosPixel();
    aPos.X() += nOffsetX;
    aPos.Y() += nOffsetY;
    pData->SetPosition(aPos);
    pData->SetSize(pTabWin->GetSizePixel());

}
// -----------------------------------------------------------------------------
FeatureState OJoinController::GetState(sal_uInt16 _nId) const
{
    FeatureState aReturn;
        // (disabled automatically)
    aReturn.bEnabled = sal_True;

    switch (_nId)
    {
        case ID_BROWSER_EDITDOC:
            aReturn.aState = ::cppu::bool2any(isEditable());
            break;
        case ID_BROWSER_SAVEDOC:
            aReturn.bEnabled = isConnected() && isModified();
            break;
        case ID_BROWSER_ADDTABLE:
            if (aReturn.bEnabled = getView() && const_cast< OJoinController* >( this )->getJoinView()->getTableView()->IsAddAllowed())
                aReturn.aState = ::cppu::bool2any(m_pAddTabDlg && m_pAddTabDlg->IsVisible());
            else
                aReturn.aState = ::cppu::bool2any(sal_False);
            break;
        default:
            aReturn = OJoinController_BASE::GetState(_nId);
    }
    return aReturn;
}
// -----------------------------------------------------------------------------
void OJoinController::Execute(sal_uInt16 _nId)
{
    switch(_nId)
    {
        case ID_BROWSER_EDITDOC:
            if(isEditable())
            { // the state should be changed to not editable
                switch (saveModified())
                {
                    case RET_CANCEL:
                        // don't change anything here so return
                        return;
                        break;
                    case RET_NO:
                        reset();
                        setModified(sal_False);     // and we are not modified yet
                        break;
                    default:
                        break;
                }
            }
            setEditable(!isEditable());
            getJoinView()->setReadOnly(!isEditable());
            InvalidateAll();
            return;
            break;
        case ID_BROWSER_ADDTABLE:
            if(!m_pAddTabDlg)
                m_pAddTabDlg = getJoinView()->getAddTableDialog();
            if(m_pAddTabDlg->IsVisible())
            {
                ::dbaui::notifySystemWindow(getView(),m_pAddTabDlg,::comphelper::mem_fun(&TaskPaneList::RemoveWindow));
                m_pAddTabDlg->Show(!m_pAddTabDlg->IsVisible());
                m_pView->GrabFocus();
            }
            else if(getJoinView()->getTableView()->IsAddAllowed())
            {
                {
                    WaitObject aWaitCursor(getView());
                    m_pAddTabDlg->Update();
                }
                m_pAddTabDlg->Show(!m_pAddTabDlg->IsVisible());
                ::dbaui::notifySystemWindow(getView(),m_pAddTabDlg,::comphelper::mem_fun(&TaskPaneList::AddWindow));
            }
            break;
        default:
            OJoinController_BASE::Execute(_nId);
    }
    InvalidateFeature(_nId);
}
// -----------------------------------------------------------------------------
void OJoinController::SaveTabWinsPosSize( OJoinTableView::OTableWindowMap* pTabWinList, long nOffsetX, long nOffsetY )
{
    // Das Loeschen und Neuanlegen der alten Implementation ist unter dem aktuellen Modell nicht mehr richtig : Die TabWins
    // habe einen Zeiger auf ihre Daten, verwaltet werden sie aber von mir. Wenn ich die alten loesche, haben die TabWins
    // ploetzlich Zeiger auf nicht mehr existente Objekte.
    // Wenn die TabWins ein SetData haetten, koennte ich mir das sparen ... haben sie aber nicht, ausserdem muesste ich dann immer
    // noch Informationen, die sich eigentlich nicht geaendert haben, auch neu setzen.
    // Also loesche ich die TabWinDatas nicht, sondern aktualisiere sie nur.
    DBG_ASSERT(m_vTableData.size() == pTabWinList->size(),
        "OJoinController::SaveTabWinsPosSize : inkonsistenter Zustand : sollte genausviel TabWinDatas haben wie TabWins !");

    OJoinTableView::OTableWindowMap::iterator aIter = pTabWinList->begin();
    for(;aIter != pTabWinList->end();++aIter)
        SaveTabWinPosSize(aIter->second, nOffsetX, nOffsetY);
}
// -----------------------------------------------------------------------------
void OJoinController::removeConnectionData(OTableConnectionData* _pData)
{
    m_vTableConnectionData.erase( ::std::find(m_vTableConnectionData.begin(),m_vTableConnectionData.end(),_pData));
}
// -----------------------------------------------------------------------------
void OJoinController::AddSupportedFeatures()
{
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:Redo")]        = ID_BROWSER_REDO;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:Save")]        = ID_BROWSER_SAVEDOC;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:Undo")]        = ID_BROWSER_UNDO;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:DB/AddTable")] = ID_BROWSER_ADDTABLE;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OJoinController::suspend(sal_Bool _bSuspend) throw( RuntimeException )
{
    if ( getBroadcastHelper().bInDispose || getBroadcastHelper().bDisposed )
        return sal_True;
    sal_Bool bCheck = sal_True;
    if ( _bSuspend )
    {
        bCheck = saveModified() != RET_CANCEL;
        if ( bCheck )
            OSingleDocumentController::suspend(_bSuspend);
    }
    return bCheck;
}
// -----------------------------------------------------------------------------
void OJoinController::loadTableWindows(const Sequence<PropertyValue>& aViewProps)
{
    ::std::vector< OTableWindowData*>::iterator aIter = m_vTableData.begin();
    for(;aIter != m_vTableData.end();++aIter)
        delete *aIter;
    m_vTableData.clear();

    const PropertyValue *pIter = aViewProps.getConstArray();
    const PropertyValue *pEnd = pIter + aViewProps.getLength();
    for (; pIter != pEnd; ++pIter)
    {
        if ( pIter->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "Tables" ) ) )
        {
            Sequence<PropertyValue> aWindow;
            pIter->Value >>= aWindow;
            const PropertyValue *pTablesIter = aWindow.getConstArray();
            const PropertyValue *pTablesEnd = pTablesIter + aWindow.getLength();
            for (; pTablesIter != pTablesEnd; ++pTablesIter)
            {
                Sequence<PropertyValue> aTable;
                pTablesIter->Value >>= aTable;
                loadTableWindow(aTable);
            }
        }
    }
}
// -----------------------------------------------------------------------------
void OJoinController::loadTableWindow(const Sequence<PropertyValue>& _rTable)
{
    OTableWindowData* pData = createTableWindowData();

    sal_Int32 nX = -1, nY = -1, nHeight = -1, nWidth = -1;

    const PropertyValue *pIter = _rTable.getConstArray();
    const PropertyValue *pEnd = pIter + _rTable.getLength();
    for (; pIter != pEnd; ++pIter)
    {
        if ( pIter->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "ComposedName" ) ) )
        {
            ::rtl::OUString sName;
            pIter->Value >>= sName;
            pData->SetComposedName(sName);
        }
        else if ( pIter->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "TableName" ) ) )
        {
            ::rtl::OUString sName;
            pIter->Value >>= sName;
            pData->SetTableName(sName);
        }
        else if ( pIter->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "WindowName" ) ) )
        {
            ::rtl::OUString sName;
            pIter->Value >>= sName;
            pData->SetWinName(sName);
        }
        else if ( pIter->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "WindowTop" ) ) )
        {
            pIter->Value >>= nY;
        }
        else if ( pIter->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "WindowLeft" ) ) )
        {
            pIter->Value >>= nX;
        }
        else if ( pIter->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "WindowWidth" ) ) )
        {
            pIter->Value >>= nWidth;
        }
        else if ( pIter->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "WindowHeight" ) ) )
        {
            pIter->Value >>= nHeight;
        }
        else if ( pIter->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "ShowAll" ) ) )
        {
            sal_Bool bShowAll;
            pIter->Value >>= bShowAll;
            pData->ShowAll(bShowAll);
        }
    }
    pData->SetPosition(Point(nX,nY));
    pData->SetSize(Size(nHeight, nWidth));
    m_vTableData.push_back(pData);
}
// -----------------------------------------------------------------------------
void OJoinController::saveTableWindows(Sequence<PropertyValue>& _rViewProps)
{
    if ( !m_vTableData.empty() )
    {
        PropertyValue *pViewIter = _rViewProps.getArray();
        PropertyValue *pEnd = pViewIter + _rViewProps.getLength();
        const static ::rtl::OUString s_sTables(RTL_CONSTASCII_USTRINGPARAM("Tables"));
        for (; pViewIter != pEnd && pViewIter->Name != s_sTables; ++pViewIter)
            ;

        if ( pViewIter == pEnd )
        {
            sal_Int32 nLen = _rViewProps.getLength();
            _rViewProps.realloc( nLen + 1 );
            pViewIter = _rViewProps.getArray() + nLen;
            pViewIter->Name = s_sTables;
        }

        Sequence<PropertyValue> aTables(m_vTableData.size());
        PropertyValue *pIter = aTables.getArray();

        Sequence<PropertyValue> aWindow(8);

        ::std::vector< OTableWindowData*>::iterator aIter = m_vTableData.begin();
        ::std::vector< OTableWindowData*>::iterator aEnd = m_vTableData.end();
        for(sal_Int32 i = 1;aIter != aEnd;++aIter,++pIter,++i)
        {
            pIter->Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Table")) + ::rtl::OUString::valueOf(i);

            sal_Int32 nPos = 0;
            aWindow[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ComposedName"));
            aWindow[nPos++].Value <<= (*aIter)->GetComposedName();
            aWindow[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TableName"));
            aWindow[nPos++].Value <<= (*aIter)->GetTableName();
            aWindow[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("WindowName"));
            aWindow[nPos++].Value <<= (*aIter)->GetWinName();
            aWindow[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("WindowTop"));
            aWindow[nPos++].Value <<= (*aIter)->GetPosition().Y();
            aWindow[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("WindowLeft"));
            aWindow[nPos++].Value <<= (*aIter)->GetPosition().X();
            aWindow[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("WindowWidth"));
            aWindow[nPos++].Value <<= (*aIter)->GetSize().Width();
            aWindow[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("WindowHeight"));
            aWindow[nPos++].Value <<= (*aIter)->GetSize().Height();
            aWindow[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowAll"));
            aWindow[nPos++].Value <<= (*aIter)->IsShowAll();

            pIter->Value <<= aWindow;
        }
        pViewIter->Value <<= aTables;
    }
}
// -----------------------------------------------------------------------------
