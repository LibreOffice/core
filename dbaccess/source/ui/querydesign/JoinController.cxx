/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: JoinController.cxx,v $
 *
 *  $Revision: 1.45 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:29:49 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#ifndef _SFXSIDS_HRC
#include <sfx2/sfxsids.hrc>
#endif
#ifndef _DBU_QRY_HRC_
#include "dbu_qry.hrc"
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
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif

#include <boost/optional.hpp>

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
using namespace ::comphelper;

// .............................................................................
namespace dbaui
{
// .............................................................................

// =============================================================================
// = AddTableDialogContext
// =============================================================================
class AddTableDialogContext : public IAddTableDialogContext
{
    OJoinController& m_rController;

public:
    AddTableDialogContext( OJoinController& _rController )
        :m_rController( _rController )
    {
    }

    // IAddTableDialogContext
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                    getConnection() const;
    virtual bool    allowViews() const;
    virtual bool    allowQueries() const;
    virtual bool    allowAddition() const;
    virtual void    addTableWindow( const String& _rQualifiedTableName, const String& _rAliasName );
    virtual void    onWindowClosing( const Window* _pWindow );

private:
    OJoinTableView* getTableView() const;
};

// -----------------------------------------------------------------------------
Reference< XConnection > AddTableDialogContext::getConnection() const
{
    return m_rController.getConnection();
}

// -----------------------------------------------------------------------------
bool AddTableDialogContext::allowViews() const
{
    return m_rController.allowViews();
}

// -----------------------------------------------------------------------------
bool AddTableDialogContext::allowQueries() const
{
    return m_rController.allowQueries();
}

// -----------------------------------------------------------------------------
bool AddTableDialogContext::allowAddition() const
{
    return const_cast< OJoinController& >( m_rController ).getJoinView()->getTableView()->IsAddAllowed();
}

// -----------------------------------------------------------------------------
void AddTableDialogContext::addTableWindow( const String& _rQualifiedTableName, const String& _rAliasName )
{
    getTableView()->AddTabWin( _rQualifiedTableName, _rAliasName, TRUE );
}

// -----------------------------------------------------------------------------
void AddTableDialogContext::onWindowClosing( const Window* _pWindow )
{
    if ( !m_rController.getView() )
        return;

    ::dbaui::notifySystemWindow(
        m_rController.getView(), const_cast< Window* >( _pWindow ), ::comphelper::mem_fun( &TaskPaneList::RemoveWindow ) );

    m_rController.InvalidateFeature( ID_BROWSER_ADDTABLE );
    m_rController.getView()->GrabFocus();
}

// -----------------------------------------------------------------------------
OJoinTableView* AddTableDialogContext::getTableView() const
{
    if ( m_rController.getJoinView() )
        return m_rController.getJoinView()->getTableView();
    return NULL;
}

// =============================================================================
// = OJoinController
// =============================================================================

DBG_NAME(OJoinController)
// -----------------------------------------------------------------------------
OJoinController::OJoinController(const Reference< XMultiServiceFactory >& _rM)
    :OJoinController_BASE(_rM)
    ,m_pAddTableDialog(NULL)
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
    {
        ::std::auto_ptr< Window > pEnsureDelete( m_pAddTableDialog );
        m_pAddTableDialog   = NULL;
    }

    OJoinController_BASE::disposing();

    m_pView             = NULL;

    m_vTableConnectionData.clear();
    m_vTableData.clear();
}
// -----------------------------------------------------------------------------
void OJoinController::reconnect( sal_Bool _bUI )
{
    OJoinController_BASE::reconnect( _bUI );
    if ( isConnected() && m_pAddTableDialog )
        m_pAddTableDialog->Update();
}

// -----------------------------------------------------------------------------
void OJoinController::setModified(sal_Bool _bModified)
{
    OJoinController_BASE::setModified(_bModified);
    InvalidateFeature(SID_RELATION_ADD_RELATION);
}
// -----------------------------------------------------------------------------
void OJoinController::SaveTabWinPosSize(OTableWindow* pTabWin, long nOffsetX, long nOffsetY)
{
    // die Daten zum Fenster
    TTableWindowData::value_type pData = pTabWin->GetData();
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
            aReturn.bChecked = isEditable();
            break;
        case ID_BROWSER_SAVEDOC:
            aReturn.bEnabled = isConnected() && isModified();
            break;
        case ID_BROWSER_ADDTABLE:
            aReturn.bEnabled = ( getView() != NULL )
                            && const_cast< OJoinController* >( this )->getJoinView()->getTableView()->IsAddAllowed();
            aReturn.bChecked = aReturn.bEnabled && m_pAddTableDialog != NULL && m_pAddTableDialog->IsVisible() ;
            if ( aReturn.bEnabled )
                aReturn.sTitle = OAddTableDlg::getDialogTitleForContext( impl_getDialogContext() );
            break;

        default:
            aReturn = OJoinController_BASE::GetState(_nId);
    }
    return aReturn;
}

// -----------------------------------------------------------------------------
AddTableDialogContext& OJoinController::impl_getDialogContext() const
{
    if ( !m_pDialogContext.get() )
    {
        OJoinController* pNonConstThis = const_cast< OJoinController* >( this );
        pNonConstThis->m_pDialogContext.reset( new AddTableDialogContext( *pNonConstThis ) );
    }
    return *m_pDialogContext;
}

// -----------------------------------------------------------------------------
void OJoinController::Execute(sal_uInt16 _nId, const Sequence< PropertyValue >& aArgs)
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
        case ID_BROWSER_ADDTABLE:
            if ( !m_pAddTableDialog )
                m_pAddTableDialog = new OAddTableDlg( getView(), impl_getDialogContext() );

            if ( m_pAddTableDialog->IsVisible() )
            {
                m_pAddTableDialog->Show( FALSE );
                getView()->GrabFocus();
            }
            else
            {
                {
                    WaitObject aWaitCursor( getView() );
                    m_pAddTableDialog->Update();
                }
                m_pAddTableDialog->Show( TRUE );
                ::dbaui::notifySystemWindow(getView(),m_pAddTableDialog,::comphelper::mem_fun(&TaskPaneList::AddWindow));
            }
            break;
        default:
            OJoinController_BASE::Execute(_nId,aArgs);
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
void OJoinController::removeConnectionData(const TTableConnectionData::value_type& _pData)
{
    m_vTableConnectionData.erase( ::std::remove(m_vTableConnectionData.begin(),m_vTableConnectionData.end(),_pData),m_vTableConnectionData.end());
}
// -----------------------------------------------------------------------------
void OJoinController::describeSupportedFeatures()
{
    OJoinController_BASE::describeSupportedFeatures();
    implDescribeSupportedFeature( ".uno:Redo",      ID_BROWSER_REDO,    CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:Save",      ID_BROWSER_SAVEDOC, CommandGroup::DOCUMENT );
    implDescribeSupportedFeature( ".uno:Undo",      ID_BROWSER_UNDO,    CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:AddTable",  ID_BROWSER_ADDTABLE,CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:EditDoc",   ID_BROWSER_EDITDOC, CommandGroup::EDIT );
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OJoinController::suspend(sal_Bool _bSuspend) throw( RuntimeException )
{
    if ( getBroadcastHelper().bInDispose || getBroadcastHelper().bDisposed )
        return sal_True;

    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( getView() && getView()->IsInModalMode() )
        return sal_False;
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
    m_vTableData.clear();

    const PropertyValue *pIter = aViewProps.getConstArray();
    const PropertyValue *pEnd = pIter + aViewProps.getLength();
    for (; pIter != pEnd; ++pIter)
    {
        if ( pIter->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "Tables" ) ) )
        {
            m_aMinimumTableViewSize = Point();
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
            if ( m_aMinimumTableViewSize != Point() )
            {
                getJoinView()->getScrollHelper()->resetRange(m_aMinimumTableViewSize);
            }
            break;
        }
    }
}
// -----------------------------------------------------------------------------
void OJoinController::loadTableWindow(const Sequence<PropertyValue>& _rTable)
{
    sal_Int32 nX = -1, nY = -1, nHeight = -1, nWidth = -1;

    ::rtl::OUString sComposedName,sTableName,sWindowName;
    sal_Bool bShowAll = false;
    const PropertyValue *pIter = _rTable.getConstArray();
    const PropertyValue *pEnd = pIter + _rTable.getLength();
    for (; pIter != pEnd; ++pIter)
    {
        if ( pIter->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "ComposedName" ) ) )
            pIter->Value >>= sComposedName;
        else if ( pIter->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "TableName" ) ) )
            pIter->Value >>= sTableName;
        else if ( pIter->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "WindowName" ) ) )
            pIter->Value >>= sWindowName;
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
            pIter->Value >>= bShowAll;
        }
    }
    TTableWindowData::value_type pData = createTableWindowData(sComposedName,sTableName,sWindowName);
    pData->SetPosition(Point(nX,nY));
    pData->SetSize( Size( nWidth, nHeight ) );
    pData->ShowAll(bShowAll);
    m_vTableData.push_back(pData);
    if ( m_aMinimumTableViewSize.X() < (nX+nWidth) )
        m_aMinimumTableViewSize.X() = (nX+nWidth);
    if ( m_aMinimumTableViewSize.Y() < (nY+nHeight) )
        m_aMinimumTableViewSize.Y() = (nY+nHeight);
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

        TTableWindowData::iterator aIter = m_vTableData.begin();
        TTableWindowData::iterator aEnd = m_vTableData.end();
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
TTableWindowData::value_type OJoinController::createTableWindowData(const ::rtl::OUString& _sComposedName,const ::rtl::OUString& _sTableName,const ::rtl::OUString& _sWindowName)
{
    OJoinDesignView* pView = getJoinView();
    if( pView )
        return pView->getTableView()->createTableWindowData(_sComposedName,_sTableName,_sWindowName);
    OSL_ENSURE(0,"We should never ever reach this point!");

    return TTableWindowData::value_type();
}
// .............................................................................
}   // namespace dbaui
// .............................................................................
