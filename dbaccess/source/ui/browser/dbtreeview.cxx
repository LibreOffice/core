/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dbtreeview.cxx,v $
 * $Revision: 1.23 $
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
void    DBTreeView::setCopyHandler(const Link& _rHdl)
{
    m_pTreeListBox->setCopyHandler(_rHdl);
}
// -----------------------------------------------------------------------------
void DBTreeView::Resize()
{
    Window::Resize();
    m_pTreeListBox->SetPosSizePixel(Point(0,0),GetOutputSizePixel());
}
// -------------------------------------------------------------------------
void DBTreeView::setModel(SvLBoxTreeList* _pTreeModel)
{
    if (_pTreeModel)
        _pTreeModel->InsertView(m_pTreeListBox);
    m_pTreeListBox->SetModel(_pTreeModel);
}

// -------------------------------------------------------------------------
void DBTreeView::setSelChangeHdl( const Link& _rHdl )
{
    m_pTreeListBox->SetSelChangeHdl( _rHdl );
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


