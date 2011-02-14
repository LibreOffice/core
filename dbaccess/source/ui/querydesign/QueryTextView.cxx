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
#ifndef DBAUI_QUERYVIEW_TEXT_HXX
#include "QueryTextView.hxx"
#endif
#ifndef DBAUI_QUERYCONTAINERWINDOW_HXX
#include "querycontainerwindow.hxx"
#endif
#ifndef DBAUI_QUERYVIEWSWITCH_HXX
#include "QueryViewSwitch.hxx"
#endif
#ifndef DBAUI_SQLEDIT_HXX
#include "sqledit.hxx"
#endif
#ifndef DBAUI_UNDOSQLEDIT_HXX
#include "undosqledit.hxx"
#endif
#ifndef DBACCESS_UI_BROWSER_ID_HXX
#include "browserids.hxx"
#endif
#ifndef DBAUI_QUERYCONTROLLER_HXX
#include "querycontroller.hxx"
#endif
#ifndef _DBU_QRY_HRC_
#include "dbu_qry.hrc"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif
#ifndef _SV_SPLIT_HXX
#include <vcl/split.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef DBAUI_QUERYDESIGNVIEW_HXX
#include "QueryDesignView.hxx"
#endif

using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
// -----------------------------------------------------------------------------

// end of temp classes
// -------------------------------------------------------------------------
DBG_NAME(OQueryTextView)
OQueryTextView::OQueryTextView(OQueryContainerWindow* _pParent)
    :Window(_pParent)
{
    DBG_CTOR(OQueryTextView,NULL);
    m_pEdit = new OSqlEdit(this);
    m_pEdit->SetRightToLeft(sal_False);
    m_pEdit->ClearModifyFlag();
    m_pEdit->SaveValue();
    m_pEdit->SetPosPixel( Point( 0, 0 ) );
    m_pEdit->Show();
    //  m_pEdit->GrabFocus();
}
// -----------------------------------------------------------------------------
OQueryTextView::~OQueryTextView()
{
    DBG_DTOR(OQueryTextView,NULL);
    ::std::auto_ptr<Window> aTemp(m_pEdit);
    m_pEdit = NULL;
}
// -----------------------------------------------------------------------------
void OQueryTextView::GetFocus()
{
    if ( m_pEdit )
        m_pEdit->GrabFocus();
}
// -------------------------------------------------------------------------
void OQueryTextView::Resize()
{
    Window::Resize();
    m_pEdit->SetSizePixel( GetOutputSizePixel() );
}
// -----------------------------------------------------------------------------
// check if the statement is correct when not returning false
sal_Bool OQueryTextView::checkStatement()
{
    return sal_True;
}
// -----------------------------------------------------------------------------
::rtl::OUString OQueryTextView::getStatement()
{
    return m_pEdit->GetText();
}
// -----------------------------------------------------------------------------
void OQueryTextView::setReadOnly(sal_Bool _bReadOnly)
{
    m_pEdit->SetReadOnly(_bReadOnly);
}
// -----------------------------------------------------------------------------
void OQueryTextView::clear()
{
    OSqlEditUndoAct* pUndoAct = new OSqlEditUndoAct( m_pEdit );

    pUndoAct->SetOriginalText( m_pEdit->GetText() );
    getContainerWindow()->getDesignView()->getController().addUndoActionAndInvalidate( pUndoAct );

    m_pEdit->SetText(String());
}
// -----------------------------------------------------------------------------
void OQueryTextView::setStatement(const ::rtl::OUString& _rsStatement)
{
    m_pEdit->SetText(_rsStatement);
}
// -----------------------------------------------------------------------------
void OQueryTextView::copy()
{
    if(!m_pEdit->IsInAccelAct() )
        m_pEdit->Copy();
}
// -----------------------------------------------------------------------------
sal_Bool OQueryTextView::isCutAllowed()
{
    return m_pEdit->GetSelected().Len() != 0;
}
// -----------------------------------------------------------------------------
sal_Bool OQueryTextView::isPasteAllowed()
{
    return sal_True;
}
// -----------------------------------------------------------------------------
sal_Bool OQueryTextView::isCopyAllowed()
{
    return sal_True;
}
// -----------------------------------------------------------------------------
void OQueryTextView::cut()
{
    if(!m_pEdit->IsInAccelAct() )
        m_pEdit->Cut();
    getContainerWindow()->getDesignView()->getController().setModified(sal_True);
}
// -----------------------------------------------------------------------------
void OQueryTextView::paste()
{
    if(!m_pEdit->IsInAccelAct() )
        m_pEdit->Paste();
    getContainerWindow()->getDesignView()->getController().setModified(sal_True);
}
// -----------------------------------------------------------------------------
