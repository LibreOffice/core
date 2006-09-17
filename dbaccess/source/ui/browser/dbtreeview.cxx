/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbtreeview.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 06:57:33 $
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

#ifndef DBACCESS_UI_DBTREEVIEW_HXX
#include "dbtreeview.hxx"
#endif
#ifndef _SVTREEBOX_HXX
#include <svtools/svtreebx.hxx>
#endif
#ifndef DBAUI_DBTREELISTBOX_HXX
#include "dbtreelistbox.hxx"
#endif
#ifndef DBAUI_DBTREEMODEL_HXX
#include "dbtreemodel.hxx"
#endif
#include "dbaccess_helpid.hrc"

// .........................................................................
namespace dbaui
{
// .........................................................................

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

DBG_NAME(DBTreeView)
//========================================================================
// class DBTreeView
//========================================================================
DBTreeView::DBTreeView( Window* pParent, const Reference< XMultiServiceFactory >& _rxORB, WinBits nBits)
                    :   Window( pParent, nBits )
                    , m_pTreeListBox(NULL)
{
    DBG_CTOR(DBTreeView,NULL);

    m_pTreeListBox = new DBTreeListBox(this, _rxORB ,WB_HASLINES | WB_SORT | WB_HASBUTTONS | WB_HSCROLL |WB_HASBUTTONSATROOT,sal_True);
    m_pTreeListBox->EnableCheckButton(NULL);
    m_pTreeListBox->SetDragDropMode( 0 );
    m_pTreeListBox->EnableInplaceEditing( sal_True );
    m_pTreeListBox->SetHelpId(HID_TLB_TREELISTBOX);
    m_pTreeListBox->Show();
}

// -----------------------------------------------------------------------------

DBTreeView::~DBTreeView()
{
    DBG_DTOR(DBTreeView,NULL);
    if (m_pTreeListBox)
    {
        if (m_pTreeListBox->GetModel())
        {
            m_pTreeListBox->GetModel()->RemoveView(m_pTreeListBox);
            m_pTreeListBox->DisconnectFromModel();
        }
        ::std::auto_ptr<Window> aTemp(m_pTreeListBox);
        m_pTreeListBox = NULL;
    }
}

// -----------------------------------------------------------------------------
void DBTreeView::SetPreExpandHandler(const Link& _rHdl)
{
    m_pTreeListBox->SetPreExpandHandler(_rHdl);
}

// -----------------------------------------------------------------------------
Link DBTreeView::GetPreExpandHandler() const
{
    return m_pTreeListBox->GetPreExpandHandler();
}
// -----------------------------------------------------------------------------
void    DBTreeView::setCutHandler(const Link& _rHdl)
{
    m_pTreeListBox->setCutHandler(_rHdl);
}
// -----------------------------------------------------------------------------
Link    DBTreeView::getCutHandler() const
{
    return m_pTreeListBox->getCutHandler();
}
// -----------------------------------------------------------------------------
void    DBTreeView::setCopyHandler(const Link& _rHdl)
{
    m_pTreeListBox->setCopyHandler(_rHdl);
}
// -----------------------------------------------------------------------------
Link    DBTreeView::getCopyHandler() const
{
    return m_pTreeListBox->getCopyHandler();
}
// -----------------------------------------------------------------------------
void    DBTreeView::setPasteHandler(const Link& _rHdl)
{
    m_pTreeListBox->setPasteHandler(_rHdl);
}
// -----------------------------------------------------------------------------
Link    DBTreeView::getPasteHandler() const
{
    return m_pTreeListBox->getPasteHandler();
}
// -----------------------------------------------------------------------------
void    DBTreeView::setDeleteHandler(const Link& _rHdl)
{
    m_pTreeListBox->setDeleteHandler(_rHdl);
}
// -----------------------------------------------------------------------------
Link    DBTreeView::getDeleteHandler() const
{
    return m_pTreeListBox->getDeleteHandler();
}
// -----------------------------------------------------------------------------
void    DBTreeView::setEditingHandler(const Link& _rHdl)
{
    m_pTreeListBox->setEditingHandler(_rHdl);
}
// -----------------------------------------------------------------------------
Link    DBTreeView::getEditingHandler() const
{
    return m_pTreeListBox->getEditingHandler();
}
// -----------------------------------------------------------------------------
void    DBTreeView::setEditedHandler(const Link& _rHdl)
{
    m_pTreeListBox->setEditedHandler(_rHdl);
}
// -----------------------------------------------------------------------------
Link    DBTreeView::getEditedHandler() const
{
    return m_pTreeListBox->getEditedHandler();
}
// -----------------------------------------------------------------------------
void DBTreeView::Resize()
{
    Window::Resize();
    m_pTreeListBox->SetPosSizePixel(Point(0,0),GetOutputSizePixel());
}

// -------------------------------------------------------------------------
DBTreeListModel* DBTreeView::getModel() const
{
    return (DBTreeListModel*)m_pTreeListBox->GetModel();
}

// -------------------------------------------------------------------------
void DBTreeView::setModel(DBTreeListModel* _pTreeModel)
{
    if (_pTreeModel)
        _pTreeModel->InsertView(m_pTreeListBox);
    m_pTreeListBox->SetModel(_pTreeModel);
}

// -------------------------------------------------------------------------
DBTreeListBox* DBTreeView::getListBox() const
{
    return m_pTreeListBox;
}

// -------------------------------------------------------------------------
void DBTreeView::setSelectHdl(const Link& _rHdl)
{
    m_pTreeListBox->SetSelectHdl(_rHdl);
}
// -----------------------------------------------------------------------------
void DBTreeView::GetFocus()
{
    Window::GetFocus();
    if ( m_pTreeListBox )//&& !m_pTreeListBox->HasChildPathFocus())
        m_pTreeListBox->GrabFocus();
}


// .........................................................................
}   // namespace dbaui
// .........................................................................


