/*************************************************************************
 *
 *  $RCSfile: AppDetailPageHelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:28:48 $
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
 *  Contributor(s): Ocke Janssen
 *
 *
 ************************************************************************/
#ifndef DBAUI_APPDETAILPAGEHELPER_HXX
#include "AppDetailPageHelper.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _DBAUI_TABLETREE_HXX_
#include "tabletree.hxx"
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef DBAUI_DBTREELISTBOX_HXX
#include "dbtreelistbox.hxx"
#endif
#ifndef _COM_SUN_STAR_AWT_XTABCONTROLLER_HPP_
#include <com/sun/star/awt/XTabController.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XVIEWSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMES_HPP_
#include <com/sun/star/frame/XFrames.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMESSUPPLIER_HPP_
#include <com/sun/star/frame/XFramesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTCREATOR_HPP_
#include <com/sun/star/ucb/XContentCreator.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDPROCESSOR_HPP_
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMAND_HPP_
#include <com/sun/star/ucb/Command.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XLOADABLE_HPP_
#include <com/sun/star/form/XLoadable.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif
#ifndef DBAUI_APPVIEW_HXX
#include "AppView.hxx"
#endif
#ifndef DBAUI_IAPPELEMENTNOTIFICATION_HXX
#include "IAppElementNotification.hxx"
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif
#ifndef _DBU_APP_HRC_
#include "dbu_app.hrc"
#endif
#ifndef _DBACCESS_UI_CALLBACKS_HXX_
#include "callbacks.hxx"
#endif
#ifndef DBAUI_ICONTROLLER_HXX
#include "IController.hxx"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _DBACCESS_SLOTID_HRC_
#include "dbaccess_slotid.hrc"
#endif
#ifndef _DBAUI_QUERYDESIGNACCESS_HXX_
#include "querydesignaccess.hxx"
#endif
#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

using namespace ::dbaui;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star;

namespace
{
    SvLBoxEntry* lcl_findEntry_impl(DBTreeListBox& rTree,const ::rtl::OUString& _rName,SvLBoxEntry* _pFirst)
    {
        SvLBoxEntry* pReturn = NULL;
        sal_Int32 nIndex = 0;
        String sName( _rName.getToken(0,'/',nIndex) );

        SvLBoxEntry* pEntry = _pFirst;
        while( pEntry )
        {
            if ( rTree.GetEntryText(pEntry) == sName )
            {
                if ( nIndex != -1 )
                {
                    sName = _rName.getToken(0,'/',nIndex);
                    pEntry = rTree.FirstChild(pEntry);
                }
                else
                {
                    pReturn = pEntry;
                    break;
                }
            }
            else
                pEntry = rTree.NextSibling(pEntry);
        }
        return pReturn;
    }
    SvLBoxEntry* lcl_findEntry(DBTreeListBox& rTree,const ::rtl::OUString& _rName,SvLBoxEntry* _pFirst)
    {
        sal_Int32 nIndex = 0;
        ::rtl::OUString sErase = _rName.getToken(0,'/',nIndex); // we don't want to have the "private:forms" part
        return (nIndex != -1 ? lcl_findEntry_impl(rTree,_rName.copy(sErase.getLength() + 1),_pFirst) : NULL);
    }
    //==================================================================
    // class OPreviewWindow
    //==================================================================
    class OTablePreviewWindow : public Window
    {
        DECL_LINK(OnDisableInput, void*);
    public:
        OTablePreviewWindow( Window* pParent, WinBits nStyle = 0 );
        virtual long Notify( NotifyEvent& rNEvt );
    };
    // -----------------------------------------------------------------------------
    OTablePreviewWindow::OTablePreviewWindow(Window* pParent, WinBits nStyle) : Window( pParent, nStyle)
    {
    }
    // -----------------------------------------------------------------------------
    long OTablePreviewWindow::Notify( NotifyEvent& rNEvt )
    {
        long nRet = Window::Notify( rNEvt );
        if ( rNEvt.GetType() == EVENT_INPUTENABLE && IsInputEnabled() )
            PostUserEvent( LINK( this, OTablePreviewWindow, OnDisableInput) );
        return nRet;
    }
    // -----------------------------------------------------------------------------
    IMPL_LINK(OTablePreviewWindow, OnDisableInput, void*, EMPTYARG)
    {
        EnableInput(FALSE);
        return 0L;
    }
}
//==================================================================
// class OAppDetailPageHelper
//==================================================================
OAppDetailPageHelper::OAppDetailPageHelper(Window* _pParent,OAppBorderWindow* _pBorderWin) : Window(_pParent,WB_DIALOGCONTROL)
    ,m_pBorderWin(_pBorderWin)
    ,m_aFL(this,WB_VERT)
    ,m_aTBPreview(this,WB_TABSTOP )
    ,m_aBorder(this,WB_BORDER | WB_READONLY)
    ,m_aPreview(&m_aBorder)
    ,m_aDocumentInfo(&m_aBorder,WB_LEFT | WB_VSCROLL | WB_READONLY )
    ,m_pTablePreview(NULL)
    ,m_ePreviewMode(E_PREVIEWNONE)
{
    m_aBorder.SetBorderStyle(WINDOW_BORDER_MONO);

    m_aMenu.reset(new PopupMenu( ModuleRes( RID_MENU_APP_PREVIEW ) ));

    m_aTBPreview.SetOutStyle(TOOLBOX_STYLE_FLAT);
    m_aTBPreview.InsertItem(SID_DB_APP_DISABLE_PREVIEW,m_aMenu->GetItemText(SID_DB_APP_DISABLE_PREVIEW),TIB_DROPDOWN|TIB_AUTOSIZE|TIB_RADIOCHECK);
    m_aTBPreview.SetHelpId(HID_APP_VIEW_PREVIEW_CB);
    m_aTBPreview.SetSelectHdl( LINK( this, OAppDetailPageHelper, OnToolBoxSelected ) );
    m_aTBPreview.SetDropdownClickHdl( LINK( this, OAppDetailPageHelper, OnToolBoxClicked ) );
    m_aTBPreview.EnableMenuStrings();
    m_aTBPreview.Enable(!m_pBorderWin->getView()->getCommandController()->isDataSourceReadOnly());

    m_aPreviewTimer.SetTimeoutHdl( LINK( this, OAppDetailPageHelper, PreviewChangeHdl) );
    m_aPreviewTimer.SetTimeout( 300 );

    m_aBorder.SetUniqueId(UID_APP_VIEW_PREVIEW_1);

    m_aPreview.SetHelpId(HID_APP_VIEW_PREVIEW_1);

    m_pTablePreview = new OTablePreviewWindow(&m_aBorder,WB_READONLY | WB_DIALOGCONTROL );
    m_pTablePreview->SetHelpId(HID_APP_VIEW_PREVIEW_2);

    m_aDocumentInfo.SetHelpId(HID_APP_VIEW_PREVIEW_3);

    m_xWindow = VCLUnoHelper::GetInterface( m_pTablePreview );

    SetUniqueId(UID_APP_DETAILPAGE_HELPER);
    for (int i=0; i < CONTROL_COUNT; ++i)
        m_pLists[i] = NULL;
}
// -----------------------------------------------------------------------------
OAppDetailPageHelper::~OAppDetailPageHelper()
{
    try
    {
        Reference< ::util::XCloseable> xCloseable(m_xFrame,UNO_QUERY);
        if ( xCloseable.is() )
            xCloseable->close(sal_True);
    }
    catch(Exception)
    {
        OSL_ENSURE(0,"Exception thrown while disposing preview frame!");
    }

    for (int i=0; i < CONTROL_COUNT; ++i)
    {
        if ( m_pLists[i] )
        {
            m_pLists[i]->Hide();
            ::std::auto_ptr<Window> aTemp(m_pLists[i]);
            m_pLists[i] = NULL;
        }

    }
}
// -----------------------------------------------------------------------------
int OAppDetailPageHelper::getVisibleControlIndex() const
{
    int i = 0;
    for (; i < CONTROL_COUNT ; ++i)
    {
        if ( m_pLists[i] && m_pLists[i]->IsVisible() )
            break;
    }
    return i;
}
// -----------------------------------------------------------------------------
void OAppDetailPageHelper::selectAll()
{
    int nPos = getVisibleControlIndex();
    if ( nPos < CONTROL_COUNT )
    {
        m_pLists[nPos]->lockAutoSelect();
        m_pLists[nPos]->SelectAll(TRUE);
        m_pLists[nPos]->unlockAutoSelect();
    }
}
// -----------------------------------------------------------------------------
void OAppDetailPageHelper::sort(int _nPos,SvSortMode _eSortMode )
{
    OSL_ENSURE(m_pLists[_nPos],"List can not be NULL! ->GPF");
    SvLBoxTreeList* pModel = m_pLists[_nPos]->GetModel();
    SvSortMode eOldSortMode = pModel->GetSortMode();
    pModel->SetSortMode(_eSortMode);
    if ( eOldSortMode != _eSortMode )
        pModel->Resort();
}
// -----------------------------------------------------------------------------
sal_Bool OAppDetailPageHelper::isSortUp() const
{
    SvSortMode eSortMode = SortNone;
    int nPos = getVisibleControlIndex();
    if ( nPos < CONTROL_COUNT )
    {
        SvLBoxTreeList* pModel = m_pLists[nPos]->GetModel();
        eSortMode = pModel->GetSortMode();
    }
    return eSortMode == SortAscending;
}
// -----------------------------------------------------------------------------
void OAppDetailPageHelper::sortDown()
{
    int nPos = getVisibleControlIndex();
    if ( nPos < CONTROL_COUNT )
        sort(nPos,SortDescending);
}
// -----------------------------------------------------------------------------
void OAppDetailPageHelper::sortUp()
{
    int nPos = getVisibleControlIndex();
    if ( nPos < CONTROL_COUNT )
        sort(nPos,SortAscending);
}
// -----------------------------------------------------------------------------
void OAppDetailPageHelper::getSelectionElementNames(::std::vector< ::rtl::OUString>& _rNames,const Reference< XDatabaseMetaData>& _xMetaData) const
{
    int nPos = getVisibleControlIndex();
    if ( nPos < CONTROL_COUNT )
    {
        DBTreeListBox& rTree = *m_pLists[nPos];
        sal_Int32 nCount = rTree.GetEntryCount();
        _rNames.reserve(nCount);
        SvLBoxEntry* pEntry = rTree.FirstSelected();
        ElementType eType = getElementType();
        while( pEntry )
        {
            if ( eType == E_TABLE )
            {
                if( rTree.GetChildCount(pEntry) == 0 )
                    _rNames.push_back(getQualifiedName(pEntry,_xMetaData));
            }
            else
            {
                ::rtl::OUString sName = rTree.GetEntryText(pEntry);
                SvLBoxEntry* pParent = rTree.GetParent(pEntry);
                while(pParent)
                {
                    sName = rTree.GetEntryText(pParent) + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/")) + sName;
                    pParent = rTree.GetParent(pParent);
                }
                _rNames.push_back(sName);
            }
            pEntry = rTree.NextSelected(pEntry);
        }
    }
}
// -----------------------------------------------------------------------------
::rtl::OUString OAppDetailPageHelper::getQualifiedName(SvLBoxEntry* _pEntry,const Reference< XDatabaseMetaData>& _xMetaData) const
{
    int nPos = getVisibleControlIndex();
    ::rtl::OUString sComposedName;

    if ( nPos < CONTROL_COUNT )
    {
        OSL_ENSURE(m_pLists[nPos],"Tables tree view is NULL! -> GPF");
        DBTreeListBox& rTree = *m_pLists[nPos];

        SvLBoxEntry* pEntry = _pEntry;
        if ( !pEntry )
            pEntry = rTree.FirstSelected();

        if ( pEntry )
        {
            if ( getElementType() == E_TABLE )
            {
                OSL_ENSURE(_xMetaData.is(),"The database metadata are NULL! -> GPF");


                ::rtl::OUString sCatalog;
                ::rtl::OUString sSchema;
                ::rtl::OUString sTable;

                SvLBoxEntry* pSchema = rTree.GetParent(pEntry);
                if ( pSchema )
                {
                    SvLBoxEntry* pCatalog = rTree.GetParent(pSchema);
                    if ( pCatalog || _xMetaData->supportsCatalogsInDataManipulation() ) // here we support catalog but no schema
                    {
                        if ( pCatalog == NULL )
                        {
                            pCatalog = pSchema;
                            pSchema = NULL;
                        }
                        sCatalog = rTree.GetEntryText( pCatalog );
                    }
                    if ( pSchema )
                    {
                        sSchema = rTree.GetEntryText(pSchema);
                    }
                }
                sTable = rTree.GetEntryText(pEntry);

                ::dbtools::composeTableName(_xMetaData,sCatalog,sSchema,sTable,sComposedName,sal_False,::dbtools::eInDataManipulation);
            }
            else
            {
                sComposedName = rTree.GetEntryText(pEntry);
                SvLBoxEntry* pParent = rTree.GetParent(pEntry);
                while(pParent)
                {
                    sComposedName = rTree.GetEntryText(pParent) + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/")) + sComposedName;
                    pParent = rTree.GetParent(pParent);
                }
            }
        }
    }
    return sComposedName;
}
// -----------------------------------------------------------------------------
ElementType OAppDetailPageHelper::getElementType() const
{
    int nPos = getVisibleControlIndex();
    return static_cast<ElementType>(nPos);
}
// -----------------------------------------------------------------------------
sal_Int32 OAppDetailPageHelper::getSelectionCount()
{
    sal_Int32 nCount = 0;
    int nPos = getVisibleControlIndex();
    if ( nPos < CONTROL_COUNT )
    {
        DBTreeListBox& rTree = *m_pLists[nPos];
        SvLBoxEntry* pEntry = rTree.FirstSelected();
        while( pEntry )
        {
            ++nCount;
            pEntry = rTree.NextSelected(pEntry);
        }
    }
    return nCount;
}
// -----------------------------------------------------------------------------
sal_Int32 OAppDetailPageHelper::getElementCount()
{
    sal_Int32 nCount = 0;
    int nPos = getVisibleControlIndex();
    if ( nPos < CONTROL_COUNT )
    {
        nCount = m_pLists[nPos]->GetEntryCount();
    }
    return nCount;
}
// -----------------------------------------------------------------------------
sal_Bool OAppDetailPageHelper::isCutAllowed()
{
    return sal_False;
}
// -----------------------------------------------------------------------------
sal_Bool OAppDetailPageHelper::isCopyAllowed()
{
    //  int nPos = getVisibleControlIndex();
    return sal_True;
}
// -----------------------------------------------------------------------------
sal_Bool OAppDetailPageHelper::isPasteAllowed()
{
    //  int nPos = getVisibleControlIndex();
    return sal_True;
}
// -----------------------------------------------------------------------------
void OAppDetailPageHelper::copy()
{
}
// -----------------------------------------------------------------------------
void OAppDetailPageHelper::cut()
{
}
// -----------------------------------------------------------------------------
void OAppDetailPageHelper::paste()
{
}
// -----------------------------------------------------------------------------
sal_Bool OAppDetailPageHelper::isLeaf(SvLBoxEntry* _pEntry) const
{
    int nPos = getVisibleControlIndex();
    sal_Bool bLeafSelected = sal_False;
    if ( nPos < CONTROL_COUNT && _pEntry )
    {
        DBTreeListBox& rTree = *m_pLists[nPos];
        bLeafSelected = reinterpret_cast<sal_Int32>(_pEntry->GetUserData()) != FOLDER_TYPE;
    }
    return bLeafSelected;
}
// -----------------------------------------------------------------------------
sal_Bool OAppDetailPageHelper::isALeafSelected() const
{
    int nPos = getVisibleControlIndex();
    sal_Bool bLeafSelected = sal_False;
    if ( nPos < CONTROL_COUNT )
    {
        DBTreeListBox& rTree = *m_pLists[nPos];
        SvLBoxEntry* pEntry = rTree.FirstSelected( );
        while( !bLeafSelected && pEntry )
        {
            bLeafSelected = reinterpret_cast<sal_Int32>(pEntry->GetUserData()) != FOLDER_TYPE;
            pEntry = rTree.NextSelected(pEntry);
        }
    }
    return bLeafSelected;
}
// -----------------------------------------------------------------------------
SvLBoxEntry* OAppDetailPageHelper::getEntry( const Point& _aPosPixel) const
{
    SvLBoxEntry* pReturn = NULL;
    int nPos = getVisibleControlIndex();
    sal_Bool bLeafSelected = sal_False;
    if ( nPos < CONTROL_COUNT )
        pReturn = m_pLists[nPos]->GetEntry( _aPosPixel,TRUE );
    return pReturn;
}
// -----------------------------------------------------------------------------
void OAppDetailPageHelper::createTablesPage(const Reference< XConnection>& _xConnection)
{
    OSL_ENSURE(_xConnection.is(),"Connection is NULL! -> GPF");

    if ( !m_pLists[E_TABLE] )
    {
        OTableTreeListBox* pTreeView = new OTableTreeListBox(this,getBorderWin()->getView()->getORB()
                                                            ,sal_False
                                                            ,WB_HASLINES | WB_SORT | WB_HASBUTTONS | WB_HSCROLL |WB_HASBUTTONSATROOT | WB_TABSTOP
                                                            ,sal_False);
        pTreeView->SetHelpId(HID_APP_TABLE_TREE);
        m_pLists[E_TABLE] = pTreeView;
        createTree(pTreeView,TABLE_TREE_ICON,TABLE_TREE_ICON_SCH);
        pTreeView->notifyHiContrastChanged();
        m_aBorder.SetZOrder(pTreeView, WINDOW_ZORDER_BEHIND);
    }
    if ( !m_pLists[E_TABLE]->GetEntryCount() )
    {
        try
        {
            Reference< XTablesSupplier > xTableSupp(_xConnection,UNO_QUERY);
            Reference< XViewsSupplier > xViewSupp;
            Reference< XNameAccess > xTables, xViews;

            xTables = xTableSupp->getTables();

            // get the views supplier and the views
            Sequence< ::rtl::OUString> sTables,sViews;
            if ( xTables.is() )
                sTables = xTables->getElementNames();

            xViewSupp.set(xTableSupp,UNO_QUERY);
            if ( xViewSupp.is() )
            {
                xViews = xViewSupp->getViews();
                if (xViews.is())
                    sViews = xViews->getElementNames();
            }
            static_cast<OTableTreeListBox*>(m_pLists[E_TABLE])->UpdateTableList(_xConnection->getMetaData(),sTables,sViews);

            SvLBoxEntry* pEntry = m_pLists[E_TABLE]->First();
            if ( pEntry )
                m_pLists[E_TABLE]->Expand(pEntry);
        }
        catch(Exception&)
        {
            OSL_ENSURE(0,"Exception catched!");
        }
        m_pLists[E_TABLE]->SelectAll(FALSE);
    }

    setDetailPage(m_pLists[E_TABLE]);
}
// -----------------------------------------------------------------------------
void OAppDetailPageHelper::createPage(ElementType _eType,const Reference< XNameAccess >& _xContainer)
{
    OSL_ENSURE(E_TABLE != _eType,"E_TABLE isn't allowed.");

    USHORT nHelpId = 0, nIcon = 0, nIconH = 0, nImageId = 0;
    sal_Bool bHiContrast = GetBackground().GetColor().IsDark();
    switch(_eType )
    {
        case E_FORM:
            nHelpId = HID_APP_FORM_TREE;
            nIcon = IMG_FORMFOLDER_TREE_S;
            nIconH = IMG_FORMFOLDER_TREE_SCH;
            nImageId = bHiContrast ? FORM_TREE_ICON_SCH : FORM_TREE_ICON;
            break;
        case E_REPORT:
            nHelpId = HID_APP_REPORT_TREE;
            nIcon = IMG_REPORTFOLDER_TREE_S;
            nIconH = IMG_REPORTFOLDER_TREE_SCH;
            nImageId = bHiContrast ? REPORT_TREE_ICON_SCH : REPORT_TREE_ICON;
            break;
        case E_QUERY:
            nHelpId = HID_APP_QUERY_TREE;
            nIcon = QUERYFOLDER_TREE_ICON;
            nIconH = QUERYFOLDER_TREE_ICON_SCH;
            nImageId = bHiContrast ? QUERY_TREE_ICON_SCH : QUERY_TREE_ICON;
            break;
        default:
            OSL_ENSURE(0,"Illegal call!");
    }

    if ( !m_pLists[_eType] )
    {
        m_pLists[_eType] = createSimpleTree(nHelpId,nIcon,nIconH);
    }

    if ( m_pLists[_eType] )
    {
        if ( !m_pLists[_eType]->GetEntryCount() && _xContainer.is() )
        {
            fillNames(_xContainer,*m_pLists[_eType],nImageId);

            m_pLists[_eType]->SelectAll(FALSE);
        }
        setDetailPage(m_pLists[_eType]);
    }
}
// -----------------------------------------------------------------------------
void OAppDetailPageHelper::setDetailPage(Window* _pWindow)
{
    OSL_ENSURE(_pWindow,"OAppDetailPageHelper::setDetailPage: Window is NULL!");
    Window* pCurrent = getCurrentView();
    if ( pCurrent )
        pCurrent->Hide();

    showPreview(NULL);
    m_aFL.Show();
    _pWindow->Show();
    m_aTBPreview.Show();
    m_aBorder.Show();
    switchPreview(m_ePreviewMode,TRUE);

    Resize();
}
// -----------------------------------------------------------------------------
void OAppDetailPageHelper::fillNames(const Reference< XNameAccess >& _xContainer,DBTreeListBox& _rList,USHORT _nImageId,SvLBoxEntry* _pParent)
{
    fillTreeListNames(_xContainer,_rList,_nImageId,_pParent,m_pBorderWin->getView()->getContainerListener());
}
// -----------------------------------------------------------------------------
DBTreeListBox* OAppDetailPageHelper::createSimpleTree(ULONG _nHelpId, USHORT _nCollapsedBitmap,USHORT _nCollapsedBitmap_HI)
{
    DBTreeListBox* pTreeView = new DBTreeListBox(this,getBorderWin()->getView()->getORB(),WB_HASLINES | WB_SORT | WB_HASBUTTONS | WB_HSCROLL |WB_HASBUTTONSATROOT | WB_TABSTOP);
    pTreeView->SetHelpId(_nHelpId);
    return createTree(pTreeView,_nCollapsedBitmap,_nCollapsedBitmap_HI);
}
// -----------------------------------------------------------------------------
DBTreeListBox* OAppDetailPageHelper::createTree(DBTreeListBox* _pTreeView,USHORT _nCollapsedBitmap,USHORT _nCollapsedBitmap_HI)
{
    WaitObject aWaitCursor(this);

    _pTreeView->SetWindowBits(WB_HASLINES | WB_SORT | WB_HASBUTTONS | WB_HSCROLL |WB_HASBUTTONSATROOT | WB_TABSTOP);
    _pTreeView->GetModel()->SetSortMode(SortAscending);
    _pTreeView->EnableCheckButton( NULL ); // do not show any buttons
    _pTreeView->SetSelectionMode(MULTIPLE_SELECTION);

    _pTreeView->SetDefaultCollapsedEntryBmp(Image(ModuleRes(_nCollapsedBitmap)));
    _pTreeView->SetDefaultCollapsedEntryBmp(Image(ModuleRes(_nCollapsedBitmap_HI)),BMP_COLOR_HIGHCONTRAST);
    _pTreeView->SetDefaultExpandedEntryBmp(Image(ModuleRes(_nCollapsedBitmap)));
    _pTreeView->SetDefaultExpandedEntryBmp(Image(ModuleRes(_nCollapsedBitmap_HI)),BMP_COLOR_HIGHCONTRAST);

    _pTreeView->SetDoubleClickHdl(LINK(this, OAppDetailPageHelper, OnEntryDoubleClick));
    _pTreeView->SetEnterKeyHdl(LINK(this, OAppDetailPageHelper, OnEntryDoubleClick));
    _pTreeView->SetSelectHdl(LINK(this, OAppDetailPageHelper, OnEntrySelectHdl));
    //  _pTreeView->SetDeselectHdl(LINK(this, OAppDetailPageHelper, OnDeSelectHdl));

    _pTreeView->setCutHandler(LINK(this, OAppDetailPageHelper, OnCutEntry));
    _pTreeView->setCopyHandler(LINK(this, OAppDetailPageHelper, OnCopyEntry));
    _pTreeView->setPasteHandler(LINK(this, OAppDetailPageHelper, OnPasteEntry));
    _pTreeView->setDeleteHandler(LINK(this, OAppDetailPageHelper, OnDeleteEntry));

    _pTreeView->setContextMenuActionListener(getBorderWin()->getView()->getCommandController());
    _pTreeView->setControlActionListener(getBorderWin()->getView()->getActionListener());

    return _pTreeView;
}
// -----------------------------------------------------------------------------
void OAppDetailPageHelper::clearPages()
{
    showPreview(NULL);
    for (size_t i=0; i < CONTROL_COUNT; ++i)
    {
        if ( m_pLists[i] )
            m_pLists[i]->Clear();
    }
}
// -----------------------------------------------------------------------------
sal_Bool OAppDetailPageHelper::isFilled() const
{
    size_t i = 0;
    for (; i < CONTROL_COUNT && !m_pLists[i]; ++i)
        ;
    return i != CONTROL_COUNT;
}
// -----------------------------------------------------------------------------
void OAppDetailPageHelper::elementReplaced(ElementType _eType
                                                    ,const ::rtl::OUString& _rOldName
                                                    ,const ::rtl::OUString& _rNewName
                                                    ,const Reference< XConnection >& _rxConn )
{
    DBTreeListBox* pTreeView = getCurrentView();
    if ( pTreeView )
    {
        ::rtl::OUString sNewName = _rNewName;
        SvLBoxEntry* pEntry = NULL;
        switch( _eType )
        {
            case E_TABLE:
                OSL_ENSURE(_rxConn.is(),"Connection is NULL! ->GPF");
                static_cast<OTableTreeListBox*>(pTreeView)->removedTable(_rxConn,_rOldName);
                static_cast<OTableTreeListBox*>(pTreeView)->addedTable(_rxConn,_rNewName, Any());
                return;

            case E_QUERY:
                pEntry = lcl_findEntry_impl(*pTreeView,_rOldName,pTreeView->First());
                break;
            case E_FORM:
            case E_REPORT:
                pEntry = lcl_findEntry(*pTreeView,_rOldName,pTreeView->First());
                break;
            default:
                OSL_ENSURE(0,"Invalid element type");
        }
        OSL_ENSURE(pEntry,"Do you know that the name isn't existence!");
        if ( pEntry )
        {
            pTreeView->SetEntryText(pEntry,sNewName);
        }
    }
}
// -----------------------------------------------------------------------------
SvLBoxEntry* OAppDetailPageHelper::elementAdded(ElementType _eType,const ::rtl::OUString& _rName, const Any& _rObject, const Reference< XConnection >& _rxConn )
{
    SvLBoxEntry* pRet = NULL;
    DBTreeListBox* pTreeView = m_pLists[_eType];
    if( _eType == E_TABLE && pTreeView )
    {
        OSL_ENSURE(_rxConn.is(),"Connection is NULL! ->GPF");
        pRet = static_cast<OTableTreeListBox*>(pTreeView)->addedTable(_rxConn,_rName, _rObject);
    }
    else if ( pTreeView )
    {

        SvLBoxEntry* pEntry = NULL;
        Reference<XChild> xChild(_rObject,UNO_QUERY);
        if ( xChild.is() && E_QUERY != _eType )
        {
            Reference<XContent> xContent(xChild->getParent(),UNO_QUERY);
            if ( xContent.is() )
            {
                ::rtl::OUString sName = xContent->getIdentifier()->getContentIdentifier();
                pEntry = lcl_findEntry(*pTreeView,sName,pTreeView->First());
            }
        }

        USHORT nImageId = 0;
        sal_Bool bHiContrast = GetBackground().GetColor().IsDark();
        switch(_eType )
        {
            case E_FORM:
                nImageId = bHiContrast ? FORM_TREE_ICON_SCH : FORM_TREE_ICON;
                break;
            case E_REPORT:
                nImageId = bHiContrast ? REPORT_TREE_ICON_SCH : REPORT_TREE_ICON;
                break;
            case E_QUERY:
                nImageId = bHiContrast ? QUERY_TREE_ICON_SCH : QUERY_TREE_ICON;
                break;
            default:
                OSL_ENSURE(0,"Illegal call!");
        }

        Reference<XNameAccess> xContainer(_rObject,UNO_QUERY);
        if ( xContainer.is() )
        {
            pRet = pTreeView->InsertEntry(_rName,pEntry,FALSE,LIST_APPEND,reinterpret_cast<void*>(FOLDER_TYPE));
            fillNames(xContainer,*pTreeView,nImageId,pRet);
        }
        else
        {
            Image aImage = Image(ModuleRes(nImageId));

            pRet = pTreeView->InsertEntry(_rName,aImage,aImage,pEntry);
        }
    }
    return pRet;
}
// -----------------------------------------------------------------------------
void OAppDetailPageHelper::elementRemoved(const ::rtl::OUString& _rName, const Reference< XConnection >& _rxConn )
{
    DBTreeListBox* pTreeView = getCurrentView();
    if ( pTreeView )
    {
        switch( getElementType() )
        {
            case E_TABLE:
                OSL_ENSURE(_rxConn.is(),"Connection is NULL! ->GPF");
                static_cast<OTableTreeListBox*>(pTreeView)->removedTable(_rxConn,_rName);
                break;
            case E_QUERY:
                if ( pTreeView )
                {
                    SvLBoxEntry* pEntry = lcl_findEntry_impl(*pTreeView,_rName,pTreeView->First());
                    if ( pEntry )
                        pTreeView->GetModel()->Remove(pEntry);
                }
                break;
            case E_FORM:
            case E_REPORT:
                {
                    if ( pTreeView )
                    {
                        SvLBoxEntry* pEntry = lcl_findEntry(*pTreeView,_rName,pTreeView->First());
                        if ( pEntry )
                            pTreeView->GetModel()->Remove(pEntry);
                    }
                }
                break;
            default:
                OSL_ENSURE(0,"Invalid element type");
        }
    }
}
// -----------------------------------------------------------------------------
IMPL_LINK(OAppDetailPageHelper, OnEntryDoubleClick, SvTreeListBox*, _pTree)
{
    getBorderWin()->getView()->getElementNotification()->onEntryDoubleClick(_pTree);
    return 1L;
}
// -----------------------------------------------------------------------------
IMPL_LINK(OAppDetailPageHelper, OnDeSelectHdl, SvTreeListBox*, _pTree)
{
    getBorderWin()->getView()->getElementNotification()->onEntryDeSelect(_pTree);
    return 1L;
}
// -----------------------------------------------------------------------------
IMPL_LINK(OAppDetailPageHelper, OnEntrySelectHdl, SvLBoxEntry*, _pEntry)
{
    if ( 1 == getSelectionCount() )
    {
        getBorderWin()->getView()->getElementNotification()->onEntrySelect(_pEntry);
    }
    else
    {
        showPreview(NULL);
    }
    return 1L;
}
// -----------------------------------------------------------------------------
IMPL_LINK( OAppDetailPageHelper, OnCutEntry, SvLBoxEntry*, _pEntry )
{
    getBorderWin()->getView()->getElementNotification()->onCutEntry(_pEntry);
    return 1L;
}
// -----------------------------------------------------------------------------
IMPL_LINK( OAppDetailPageHelper, OnCopyEntry, SvLBoxEntry*, _pEntry )
{
    getBorderWin()->getView()->getElementNotification()->onCopyEntry(_pEntry);
    return 1L;
}
// -----------------------------------------------------------------------------
IMPL_LINK( OAppDetailPageHelper, OnPasteEntry, SvLBoxEntry*, _pEntry )
{
    getBorderWin()->getView()->getElementNotification()->onPasteEntry(_pEntry);
    return 1L;
}
// -----------------------------------------------------------------------------
IMPL_LINK( OAppDetailPageHelper, OnDeleteEntry, SvLBoxEntry*, _pEntry )
{
    getBorderWin()->getView()->getElementNotification()->onDeleteEntry(_pEntry);
    return 1L;
}
// -----------------------------------------------------------------------------
void OAppDetailPageHelper::Resize()
{
    //////////////////////////////////////////////////////////////////////
    // Abmessungen parent window
    Size aOutputSize( GetOutputSize() );
    long nOutputWidth   = aOutputSize.Width();
    long nOutputHeight  = aOutputSize.Height();

    Window* pWindow = getCurrentView();
    if ( pWindow )
    {
        Size aFLSize = LogicToPixel( Size( 2, 6 ), MAP_APPFONT );
        sal_Int32 n6PPT = aFLSize.Height();
        Size aFTSize = LogicToPixel( Size( 8, 14 ), MAP_APPFONT );
        sal_Int32 n14PPT = aFTSize.Height();
        long nHalfOutputWidth = static_cast<long>(nOutputWidth * 0.5);

        pWindow->SetPosSizePixel( Point(0, 0), Size(nHalfOutputWidth - n6PPT, nOutputHeight) );

        m_aFL.SetPosSizePixel( Point(nHalfOutputWidth , 0 ), Size(aFLSize.Width(), nOutputHeight ) );

        String sText = m_aTBPreview.GetItemText(SID_DB_APP_DISABLE_PREVIEW);
        long nTextWidth = m_aTBPreview.GetTextWidth(sText);
        m_aTBPreview.SetPosSizePixel(   Point(nOutputWidth - 2*n6PPT - nTextWidth, 0 ),
                                        Size(nTextWidth + n6PPT, n14PPT) );

        m_aBorder.SetPosSizePixel(      Point(nHalfOutputWidth + aFLSize.Width() + n6PPT, n14PPT + n6PPT ),
                                        Size(nHalfOutputWidth - aFLSize.Width() - n6PPT, nOutputHeight - 2*n6PPT - n14PPT) );
        m_aPreview.SetPosSizePixel(     Point(0,0),m_aBorder.GetSizePixel() );
        m_aDocumentInfo.SetPosSizePixel(Point(0,0),m_aBorder.GetSizePixel() );
        m_pTablePreview->SetPosSizePixel(Point(0,0),m_aBorder.GetSizePixel() );
    }
}
// -----------------------------------------------------------------------------
IMPL_LINK( OAppDetailPageHelper, PreviewChangeHdl, void*, EMPTY )
{
    m_aTBPreview.EndSelection();

    // tell the toolbox that the item is pressed down
    m_aTBPreview.SetItemDown( SID_DB_APP_DISABLE_PREVIEW, sal_True );

    // simulate a mouse move (so the "down" state is really painted)
    Point aPoint = m_aTBPreview.GetItemRect( SID_DB_APP_DISABLE_PREVIEW ).TopLeft();
    MouseEvent aMove( aPoint, 0, MOUSE_SIMPLEMOVE | MOUSE_SYNTHETIC );
    m_aTBPreview.MouseMove( aMove );

    m_aTBPreview.Update();

    // execute the menu
    IController* pControler = getBorderWin()->getView()->getCommandController();
    ::std::auto_ptr<PopupMenu> aMenu(new PopupMenu( ModuleRes( RID_MENU_APP_PREVIEW ) ));

    sal_uInt16 pActions[] = { SID_DB_APP_DISABLE_PREVIEW
                            , SID_DB_APP_VIEW_DOC_PREVIEW
                            , SID_DB_APP_VIEW_DOCINFO_PREVIEW
    };

    for(sal_Int32 i=0; i < sizeof(pActions)/sizeof(pActions[0]);++i)
    {
        aMenu->CheckItem(pActions[i],m_aMenu->IsItemChecked(pActions[i]));
    }
    aMenu->EnableItem( SID_DB_APP_VIEW_DOCINFO_PREVIEW, pControler->isCommandEnabled(SID_DB_APP_VIEW_DOCINFO_PREVIEW) );

    // no disabled entries
    aMenu->RemoveDisabledEntries();

    sal_uInt16 nSelectedAction = aMenu->Execute(&m_aTBPreview, m_aTBPreview.GetItemRect( SID_DB_APP_DISABLE_PREVIEW ));
    // "cleanup" the toolbox state
    MouseEvent aLeave( aPoint, 0, MOUSE_LEAVEWINDOW | MOUSE_SYNTHETIC );
    m_aTBPreview.MouseMove( aLeave );
    m_aTBPreview.SetItemDown( SID_DB_APP_DISABLE_PREVIEW, sal_False);
    if ( nSelectedAction )
    {
        m_aTBPreview.SetItemText(SID_DB_APP_DISABLE_PREVIEW, aMenu->GetItemText(nSelectedAction));
        Resize();
        getBorderWin()->getView()->getCommandController()->executeChecked(nSelectedAction);
    }
    return 0;
}
// -----------------------------------------------------------------------------
PreviewMode OAppDetailPageHelper::getPreviewMode()
{
    return m_ePreviewMode;
}
// -----------------------------------------------------------------------------
sal_Bool OAppDetailPageHelper::isPreviewEnabled()
{
    return m_ePreviewMode != E_PREVIEWNONE;
}
// -----------------------------------------------------------------------------
void OAppDetailPageHelper::switchPreview()
{
    //  m_aTBPreview.GetClickHdl().Call(&m_aTBPreview);
}
// -----------------------------------------------------------------------------
void OAppDetailPageHelper::switchPreview(PreviewMode _eMode,BOOL _bForce)
{
    if ( m_ePreviewMode != _eMode || _bForce )
    {
        m_ePreviewMode = _eMode;

        sal_uInt16 nSelectedAction = SID_DB_APP_DISABLE_PREVIEW;
        switch(m_ePreviewMode)
        {
            case E_PREVIEWNONE:
                nSelectedAction = SID_DB_APP_DISABLE_PREVIEW;
                break;
            case E_DOCUMENT:
                nSelectedAction = SID_DB_APP_VIEW_DOC_PREVIEW;
                break;
            case E_DOCUMENTINFO:
                if ( getBorderWin()->getView()->getCommandController()->isCommandEnabled(SID_DB_APP_VIEW_DOCINFO_PREVIEW) )
                    nSelectedAction = SID_DB_APP_VIEW_DOCINFO_PREVIEW;
                else
                    m_ePreviewMode = E_PREVIEWNONE;
                break;
        }

        getBorderWin()->getView()->getViewChangeListener()->previewChanged(static_cast<sal_Int32>(m_ePreviewMode));

        m_aMenu->CheckItem(nSelectedAction);
        m_aTBPreview.SetItemText(SID_DB_APP_DISABLE_PREVIEW, m_aMenu->GetItemText(nSelectedAction));
        Resize();

        if ( isPreviewEnabled() )
        {
            DBTreeListBox* pTree = getCurrentView();
            if ( pTree )
            {
                SvLBoxEntry* pEntry = pTree->GetSelectedEntry();
                if ( pEntry )
                {
                    getBorderWin()->getView()->getElementNotification()->onEntrySelect(pEntry);
                }
            }
        }
        else
        {
            m_pTablePreview->Hide();
            m_aPreview.Hide();
            m_aDocumentInfo.Hide();
        }
    }
}
// -----------------------------------------------------------------------------
void OAppDetailPageHelper::showPreview(const Reference< XContent >& _xContent)
{
    if ( isPreviewEnabled() )
    {
        m_pTablePreview->Hide();

        WaitObject aWaitCursor( this );
        try
        {
            Reference<XCommandProcessor> xContent(_xContent,UNO_QUERY);
            if ( xContent.is() )
            {
                com::sun::star::ucb::Command aCommand;
                if ( m_ePreviewMode == E_DOCUMENT )
                    aCommand.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("preview"));
                else
                    aCommand.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("getdocumentinfo"));

                Any aPreview = xContent->execute(aCommand,xContent->createCommandIdentifier(),Reference< XCommandEnvironment >());
                if ( m_ePreviewMode == E_DOCUMENT )
                {
                    m_aDocumentInfo.Hide();
                    m_aPreview.Show();

                    Graphic aGraphic;
                    Sequence < sal_Int8 > aBmpSequence;
                    if ( aPreview >>= aBmpSequence )
                    {
                        SvMemoryStream  aData( aBmpSequence.getArray(),
                                            aBmpSequence.getLength(),
                                            STREAM_READ );

                        GraphicConverter::Import(aData,aGraphic);
                    }
                    m_aPreview.setGraphic( aGraphic );
                    m_aPreview.Invalidate();
                }
                else
                {
                    m_aPreview.Hide();
                    m_aDocumentInfo.Clear();
                    m_aDocumentInfo.Show();
                    Reference<XPropertySet> xProp(aPreview,UNO_QUERY);
                    if ( xProp.is() )
                        m_aDocumentInfo.fill(xProp,String());
                }
            }
            else
            {
                m_aPreview.Hide();
                m_aDocumentInfo.Hide();
                DBTreeListBox* pTreeView = getCurrentView();
                if ( pTreeView )
                    pTreeView->clearCurrentSelectionEntry();
            }
        }
        catch(Exception)
        {
            OSL_ENSURE(0,"Exception catched while extracting the preview!");
        }
    }
}
// -----------------------------------------------------------------------------
void OAppDetailPageHelper::showPreview( const ::rtl::OUString& _sDataSourceName,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection,
                                        const ::rtl::OUString& _sName,
                                        sal_Bool _bTable)
{
    if ( isPreviewEnabled() )
    {
        WaitObject aWaitCursor( this );
        m_aPreview.Hide();
        m_aDocumentInfo.Hide();
        m_pTablePreview->Show();
        if ( !m_xFrame.is() )
        {
            try
            {
                m_xFrame = Reference < XFrame > ( getBorderWin()->getView()->getORB()->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Frame")) ), UNO_QUERY );
                m_xFrame->initialize( m_xWindow );

                Reference<XFramesSupplier> xSup(getBorderWin()->getView()->getController()->getFrame(),UNO_QUERY);
                if ( xSup.is() )
                {
                    Reference<XFrames> xFrames = xSup->getFrames();
                    xFrames->append(m_xFrame);
                }
            }
            catch(Exception)
            {
            }
        }
        ::std::auto_ptr< ODesignAccess> pDispatcher( new OTableAccess(getBorderWin()->getView()->getORB(),_bTable,Reference<XComponentLoader>(m_xFrame,UNO_QUERY)));

        Sequence < PropertyValue > aArgs( 4 );
        aArgs[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Preview"));
        aArgs[0].Value <<= sal_True;
        aArgs[1].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReadOnly"));
        aArgs[1].Value <<= sal_True;
        aArgs[2].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AsTemplate"));
        aArgs[2].Value <<= sal_False;
        aArgs[3].Name = PROPERTY_SHOWMENU;
        aArgs[3].Value <<= sal_False;

        Reference<XFrame> xFrame(pDispatcher->edit(makeAny(_sDataSourceName), _sName,_xConnection,aArgs),UNO_QUERY);
        sal_Bool bClearPreview = !xFrame.is();

        // clear the preview when the query or table could not be loaded
        if ( !bClearPreview )
        {
            Reference<awt::XTabController> xController(xFrame->getController(),UNO_QUERY);
            if ( !(bClearPreview = !xController.is()) )
            {
                Reference<XLoadable> xLoadable(xController->getModel(),UNO_QUERY);
                bClearPreview = !(xLoadable.is() && xLoadable->isLoaded());
            }
        }
        if ( bClearPreview )
            showPreview(NULL);
    }
}
// -----------------------------------------------------------------------------
IMPL_LINK(OAppDetailPageHelper, OnToolBoxClicked, ToolBox*, pToolBox)
{
    m_aPreviewTimer.Start();
    return 0L;
}
// -----------------------------------------------------------------------------
IMPL_LINK(OAppDetailPageHelper, OnToolBoxSelected, ToolBox*, pToolBox)
{
    if ( m_aPreviewTimer.IsActive() )
        m_aPreviewTimer.Stop();
    return 0L;
}
// -----------------------------------------------------------------------------
void OAppDetailPageHelper::disableControls(sal_Bool _bDisable)
{
    m_aTBPreview.Enable(!_bDisable);
}
// -----------------------------------------------------------------------------
void OAppDetailPageHelper::KeyInput( const KeyEvent& rKEvt )
{
    KeyFuncType eFunc = rKEvt.GetKeyCode().GetFunction();
    USHORT      nCode = rKEvt.GetKeyCode().GetCode();
    sal_Bool bHandled = sal_False;

    if ( KEY_RETURN == nCode )
    {
        getBorderWin()->getView()->getElementNotification()->onEntryDoubleClick(getCurrentView());
    }
    else
        Window::KeyInput(rKEvt);
}
// -----------------------------------------------------------------------------

BOOL OPreviewWindow::ImplGetGraphicCenterRect( const Graphic& rGraphic, Rectangle& rResultRect ) const
{
    const Size  aWinSize( GetOutputSizePixel() );
    Size        aNewSize( LogicToPixel( rGraphic.GetPrefSize(), rGraphic.GetPrefMapMode() ) );
    BOOL        bRet = FALSE;

    if( aNewSize.Width() && aNewSize.Height() )
    {
        // scale to fit window
        const double fGrfWH = (double) aNewSize.Width() / aNewSize.Height();
        const double fWinWH = (double) aWinSize.Width() / aWinSize.Height();

        if ( fGrfWH < fWinWH )
        {
            aNewSize.Width() = (long) ( aWinSize.Height() * fGrfWH );
            aNewSize.Height()= aWinSize.Height();
        }
        else
        {
            aNewSize.Width() = aWinSize.Width();
            aNewSize.Height()= (long) ( aWinSize.Width() / fGrfWH);
        }

        const Point aNewPos( ( aWinSize.Width()  - aNewSize.Width() ) >> 1,
                             ( aWinSize.Height() - aNewSize.Height() ) >> 1 );

        rResultRect = Rectangle( aNewPos, aNewSize );
        bRet = TRUE;
    }

    return bRet;
}

// ------------------------------------------------------------------------

void OPreviewWindow::Paint( const Rectangle& rRect )
{
    Window::Paint( rRect );

    if( ImplGetGraphicCenterRect( m_aGraphicObj.GetGraphic(), m_aPreviewRect ) )
    {
        const Point aPos( m_aPreviewRect.TopLeft() );
        const Size  aSize( m_aPreviewRect.GetSize() );

        if( m_aGraphicObj.IsAnimated() )
            m_aGraphicObj.StartAnimation( this, aPos, aSize );
        else
            m_aGraphicObj.Draw( this, aPos, aSize );
    }
}
// -----------------------------------------------------------------------------
