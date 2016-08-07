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
#include <toolkit/helper/vclunohelper.hxx>
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
