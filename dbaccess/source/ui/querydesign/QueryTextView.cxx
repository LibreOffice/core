/*************************************************************************
 *
 *  $RCSfile: QueryTextView.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:13:27 $
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
#ifndef DBAUI_QUERYVIEW_TEXT_HXX
#include "QueryTextView.hxx"
#endif
#ifndef DBAUI_QUERYCONTAINERWINDOW_HXX
#include "querycontainerwindow.hxx"
#endif
#ifndef DBAUI_QUERYVIEWSWITCH_HXX
#include "QueryViewSwitch.hxx"
#endif
#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
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
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
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
DBG_NAME(OQueryTextView);
OQueryTextView::OQueryTextView(OQueryContainerWindow* _pParent)
    :Window(_pParent)
{
    DBG_CTOR(OQueryTextView,NULL);
    m_pEdit = new OSqlEdit(this);
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
    getContainerWindow()->getDesignView()->getController()->addUndoActionAndInvalidate( pUndoAct );

    m_pEdit->SetText(String());
}
// -----------------------------------------------------------------------------
void OQueryTextView::setStatement(const ::rtl::OUString& _rsStatement)
{
    m_pEdit->OverloadedSetText(_rsStatement);
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
    getContainerWindow()->getDesignView()->getController()->setModified(sal_True);
}
// -----------------------------------------------------------------------------
void OQueryTextView::paste()
{
    if(!m_pEdit->IsInAccelAct() )
        m_pEdit->Paste();
    getContainerWindow()->getDesignView()->getController()->setModified(sal_True);
}
// -----------------------------------------------------------------------------
