/*************************************************************************
 *
 *  $RCSfile: QueryTextView.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2001-04-02 10:50:54 $
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
#include "QueryViewSwitch.hxx"
#ifndef DBAUI_QUERYVIEW_TEXT_HXX
#include "QueryTextView.hxx"
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
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
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
// temporary class
OQueryContainerWindow::OQueryContainerWindow(Window* pParent, OQueryController* _pController,const Reference< XMultiServiceFactory >& _rFactory)
    : Window(pParent)
    ,m_pBeamer(NULL)
{
    m_pView = new OQueryViewSwitch(this,_pController,_rFactory);

    m_pSplitter = new Splitter(this,WB_VSCROLL);
    m_pSplitter->Hide();
    m_pSplitter->SetSplitHdl( LINK( this, OQueryContainerWindow, SplitHdl ) );
    m_pSplitter->SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetDialogColor() ) );

    // our UNO representation
    m_xMe = VCLUnoHelper::CreateControlContainer(this);
}
// -----------------------------------------------------------------------------
OQueryContainerWindow::~OQueryContainerWindow()
{
    if(m_xBeamer.is())
        m_xBeamer->setComponent(NULL,NULL);
    m_xBeamer   = NULL;
    m_xMe       = NULL;
    delete m_pBeamer;
    delete m_pSplitter;
    delete m_pView;
}
// -----------------------------------------------------------------------------
void OQueryContainerWindow::switchView()
{
    m_pView->switchView();
}
// -----------------------------------------------------------------------------
IMPL_LINK( OQueryContainerWindow, SplitHdl, void*, p )
{
    long nTest = m_pSplitter->GetPosPixel().Y();
    m_pSplitter->SetPosPixel( Point( m_pSplitter->GetPosPixel().X(),m_pSplitter->GetSplitPosPixel() ) );
    Resize();

    return 0L;
}
// -----------------------------------------------------------------------------
void OQueryContainerWindow::Resize()
{
    Window::Resize();
    Size aSize = GetOutputSizePixel();
    if(!m_pBeamer || !m_pBeamer->IsVisible())
        m_pView->SetPosSizePixel(Point(0,0),aSize);
    else
    {
        Point   aSplitPos(0,0);
        Size    aSplitSize(0,0);

        aSplitPos       = m_pSplitter->GetPosPixel();
        aSplitSize      = m_pSplitter->GetOutputSizePixel();
        aSplitSize.Width() = aSize.Width();

        if( ( aSplitPos.Y() + aSplitSize.Height() ) > ( aSize.Height() ))
            aSplitPos.Y() = aSize.Height() - aSplitSize.Height();

        if( aSplitPos.Y() <= 0)
            aSplitPos.Y() = LogicToPixel( Size(0,sal_Int32(aSize.Height() * 0.2) ), MAP_APPFONT ).Height();

        Size aBeamer(aSize.Width(),aSplitPos.Y());
        m_pBeamer->SetPosSizePixel(Point(0,0),aBeamer);
        //set the size of the splitter
        m_pSplitter->SetPosSizePixel( aSplitPos, Size( aSize.Width(), aSplitSize.Height()) );
        m_pSplitter->SetDragRectPixel(  Rectangle( Point( 0, 0 ), aSize) );

        Point aPos(0,aSplitPos.Y()+aSplitSize.Height());
        m_pView->SetPosSizePixel(aPos,Size( aSize.Width(), aSize.Height() - aSplitSize.Height() - aSplitPos.Y() ));
    }
}
// -----------------------------------------------------------------------------
void OQueryContainerWindow::initialize(const Reference<XFrame>& _xFrame)
{
    // append our frame
    Reference < XFramesSupplier > xSup(_xFrame,UNO_QUERY);
    Reference < XFrames > xFrames = xSup->getFrames();
    xFrames->append( m_xBeamer );
}
// -----------------------------------------------------------------------------
void OQueryContainerWindow::hideBeamer()
{
    // here I know that we will be destroyed from the frame
    m_pBeamer = NULL;
    m_xBeamer = NULL;
    m_pSplitter->Hide();
    Resize();
}
// -----------------------------------------------------------------------------
void OQueryContainerWindow::showBeamer(const Reference<XFrame>& _xFrame)
{
    if(!m_pBeamer)
    {
        m_pBeamer = new OBeamer(this);
        m_xBeamer = Reference<XFrame>(m_pView->getORB()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.frame.Frame")),UNO_QUERY);
        OSL_ENSURE(m_xBeamer.is(),"No frame created!");
        m_xBeamer->initialize( VCLUnoHelper::GetInterface ( m_pBeamer ) );
        m_xBeamer->setName(::rtl::OUString::createFromAscii("_beamer"));

        // append our frame
        Reference < XFramesSupplier > xSup(_xFrame,UNO_QUERY);
        Reference < XFrames > xFrames = xSup->getFrames();
        xFrames->append( m_xBeamer );
//  }
//
//
//  if(!m_pBeamer->IsVisible())
//  {
        Size aSize = GetOutputSizePixel();
        Size aBeamer(aSize.Width(),sal_Int32(aSize.Height()*0.33));

        const long  nFrameHeight = LogicToPixel( Size( 0, 3 ), MAP_APPFONT ).Height();
        Point aPos(0,aBeamer.Height()+nFrameHeight);

        m_pBeamer->SetPosSizePixel(Point(0,0),aBeamer);
        m_pBeamer->Show();

        m_pSplitter->SetPosSizePixel( Point(0,aBeamer.Height()), Size(aSize.Width(),nFrameHeight) );
        // a default pos for the splitter, so that the listbox is about 80 (logical) pixels wide
        m_pSplitter->SetSplitPosPixel( aBeamer.Height() );
        m_pView->SetPosSizePixel(aPos,Size(aBeamer.Width(),aSize.Height() - aBeamer.Height()-nFrameHeight));

        m_pSplitter->Show();
        Resize();
    }
}
// -----------------------------------------------------------------------------

// end of temp classes
// -------------------------------------------------------------------------
OQueryTextView::OQueryTextView(Window* _pParent,ToolBox*    _pToolBox)
    :Window(_pParent)
    ,m_pToolBox(_pToolBox)
{
    m_pEdit = new OSqlEdit(this);
    m_pEdit->ClearModifyFlag();
    m_pEdit->SaveValue();
    m_pEdit->Show();
    m_pEdit->GrabFocus();
}
// -----------------------------------------------------------------------------
OQueryTextView::~OQueryTextView()
{
    m_pToolBox = NULL;
    delete m_pEdit;
}
// -------------------------------------------------------------------------
void OQueryTextView::Construct(const Reference< ::com::sun::star::awt::XControlModel >& xModel)
{
}
// -------------------------------------------------------------------------
void OQueryTextView::Resize()
{
    Size aToolBoxSize;
    ToolBox* pToolBox = m_pToolBox;
    if(pToolBox)
        aToolBoxSize = pToolBox->GetOutputSizePixel();

    Size aSize = GetOutputSizePixel();
    Point aTopLeft(0,0);
    aTopLeft.Y() += aToolBoxSize.Height();
    m_pEdit->SetPosSizePixel(aTopLeft,Size(aSize.Width(),aSize.Height()-aTopLeft.Y()));
    m_pToolBox->SetPosSizePixel(Point(0,0),Size(aSize.Width(),aToolBoxSize.Height()));
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
    SfxUndoManager* pUndoMgr = static_cast<OQueryContainerWindow*>(GetParent())->getView()->getRealView()->getController()->getUndoMgr();
    OSqlEditUndoAct* pUndoAct = new OSqlEditUndoAct( m_pEdit );

    pUndoAct->SetOriginalText( m_pEdit->GetText() );
    pUndoMgr->AddUndoAction( pUndoAct );

    m_pEdit->SetText(String());
}
// -----------------------------------------------------------------------------
void OQueryTextView::setStatement(const ::rtl::OUString& _rsStatement)
{
    m_pEdit->SetText(_rsStatement);
    m_pEdit->Modify();
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
void OQueryTextView::cut()
{
    if(!m_pEdit->IsInAccelAct() )
        m_pEdit->Cut();
    static_cast<OQueryContainerWindow*>(GetParent())->getView()->getRealView()->getController()->setModified(sal_True);
}
// -----------------------------------------------------------------------------
void OQueryTextView::paste()
{
    if(!m_pEdit->IsInAccelAct() )
        m_pEdit->Paste();
    static_cast<OQueryContainerWindow*>(GetParent())->getView()->getRealView()->getController()->setModified(sal_True);
}
// -----------------------------------------------------------------------------