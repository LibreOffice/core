/*************************************************************************
 *
 *  $RCSfile: TableDesignView.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-22 07:54:07 $
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
#ifndef DBAUI_TABLEDESIGNVIEW_HXX
#include "TableDesignView.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef DBUI_TABLECONTROLLER_HXX
#include "TableController.hxx"
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif
#ifndef DBAUI_FIELDDESCRIPTIONS_HXX
#include "FieldDescriptions.hxx"
#endif
#ifndef DBAUI_TABLEEDITORCONTROL_HXX
#include "TEditControl.hxx"
#endif
#ifndef DBAUI_TABLEDESCRIPTIONWINDOW_HXX
#include "TableDescWin.hxx"
#endif
#ifndef DBAUI_TABLEFIELDDESCRIPTION_HXX
#include "TableFieldDescWin.hxx"
#endif
#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif
#ifndef DBAUI_TABLEROW_HXX
#include "TableRow.hxx"
#endif
#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARD_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#endif


using namespace ::dbaui;
using namespace ::utl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::datatransfer::clipboard;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

//==================================================================
// class OTableBorderWindow
//==================================================================
OTableBorderWindow::OTableBorderWindow(Window* pParent) : Window(pParent,WB_BORDER)
    ,m_aHorzSplitter( this )
{
    ImplInitSettings( sal_True, sal_True, sal_True );
    //////////////////////////////////////////////////////////////////////
    // Childs erzeugen
    m_pEditorCtrl   = new OTableEditorCtrl( this);
    m_pFieldDescWin = new OTableFieldDescWin( this );

    m_pFieldDescWin->SetHelpId(HID_TAB_DESIGN_DESCWIN);

    // set depending windows and controls
    m_pEditorCtrl->SetDescrWin(m_pFieldDescWin);

    //////////////////////////////////////////////////////////////////////
    // Splitter einrichten
    m_aHorzSplitter.SetSplitHdl( LINK(this, OTableBorderWindow, SplitHdl) );
    m_aHorzSplitter.Show();
}
// -----------------------------------------------------------------------------
OTableBorderWindow::~OTableBorderWindow()
{
    //////////////////////////////////////////////////////////////////////
    // Childs zerstoeren
    m_pEditorCtrl->Hide();
    m_pFieldDescWin->Hide();

    delete m_pEditorCtrl;
    delete m_pFieldDescWin;
}
// -----------------------------------------------------------------------------
void OTableBorderWindow::Resize()
{
    const long nSplitterHeight(3);

    //////////////////////////////////////////////////////////////////////
    // Abmessungen parent window
    Size aOutputSize( GetOutputSize() );
    long nOutputWidth   = aOutputSize.Width();
    long nOutputHeight  = aOutputSize.Height();
    long nSplitPos      = m_aHorzSplitter.GetSplitPosPixel();

    //////////////////////////////////////////////////////////////////////
    // Verschiebebereich Splitter mittleres Drittel des Outputs
    long nDragPosY = nOutputHeight/3;
    long nDragSizeHeight = nOutputHeight/3;
    m_aHorzSplitter.SetDragRectPixel( Rectangle(Point(0,nDragPosY), Size(nOutputWidth,nDragSizeHeight) ), this );
    if( (nSplitPos < nDragPosY) || (nSplitPos > (nDragPosY+nDragSizeHeight)) )
        nSplitPos = nDragPosY+nDragSizeHeight-5;

    //////////////////////////////////////////////////////////////////////
    // Splitter setzen
    m_aHorzSplitter.SetPosSizePixel( Point( 0, nSplitPos ), Size(nOutputWidth, nSplitterHeight));
    m_aHorzSplitter.SetSplitPosPixel( nSplitPos );

    //////////////////////////////////////////////////////////////////////
    // Fenster setzen
    long nEditCrtl_X = long(0.3 * nOutputWidth);

    m_pEditorCtrl->SetPosSizePixel( Point(0, 0), Size(nOutputWidth , nSplitPos) );

    m_pFieldDescWin->SetPosSizePixel(   Point(0, nSplitPos+nSplitterHeight),
                        Size(nOutputWidth, nOutputHeight-nSplitPos-nSplitterHeight) );
}
//------------------------------------------------------------------------------
IMPL_LINK( OTableBorderWindow, SplitHdl, Splitter*, pSplit )
{
    long nSplitPos = pSplit->GetSplitPosPixel();

    if(pSplit == &m_aHorzSplitter)
    {
        long nTest = m_aHorzSplitter.GetPosPixel().Y();
        m_aHorzSplitter.SetPosPixel( Point( m_aHorzSplitter.GetPosPixel().X(),m_aHorzSplitter.GetSplitPosPixel() ) );
        Resize();
    }
    return 0;
}
// -----------------------------------------------------------------------------
void OTableBorderWindow::ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if ( bFont )
    {
        Font aFont = rStyleSettings.GetAppFont();
        if ( IsControlFont() )
            aFont.Merge( GetControlFont() );
        SetPointFont( aFont );
//      Set/*Zoomed*/PointFont( aFont );
    }

    if ( bFont || bForeground )
    {
        Color aTextColor = rStyleSettings.GetButtonTextColor();
        if ( IsControlForeground() )
            aTextColor = GetControlForeground();
        SetTextColor( aTextColor );
    }

    if ( bBackground )
    {
        if( IsControlBackground() )
            SetBackground( GetControlBackground() );
        else
            SetBackground( rStyleSettings.GetFaceColor() );
    }
}
// -----------------------------------------------------------------------
void OTableBorderWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings( sal_True, sal_True, sal_True );
        Invalidate();
    }
    else
        Window::DataChanged( rDCEvt );
}
// -----------------------------------------------------------------------------
void OTableBorderWindow::GetFocus()
{
    Window::GetFocus();

    // forward the focus to the current cell of the editor control
    if (m_pEditorCtrl)
        m_pEditorCtrl->GrabFocus();
}

//==================================================================
// class OTableDesignView
//==================================================================
DBG_NAME(OTableDesignView);
//------------------------------------------------------------------------------
OTableDesignView::OTableDesignView( Window* pParent,
                                    const Reference< XMultiServiceFactory >& _rxOrb,
                                    OTableController* _pController
                                   ) :
    ODataView( pParent ,_rxOrb )
    ,m_pController(_pController)
{
    DBG_CTOR(OTableDesignView,NULL);

    try
    {
        Any aValue = ConfigManager::GetDirectConfigProperty(ConfigManager::LOCALE);
        m_aLocale.Language = ::comphelper::getString(aValue);
    }
    catch(Exception&)
    {
    }

    m_pWin = new OTableBorderWindow(this);
    m_pWin->Show();
}

//------------------------------------------------------------------------------
OTableDesignView::~OTableDesignView()
{
    DBG_DTOR(OTableDesignView,NULL);
    m_pWin->Hide();
    delete m_pWin;
}
// -----------------------------------------------------------------------------
void OTableDesignView::initialize()
{
    GetEditorCtrl()->Init();
    GetDescWin()->Init();
    // first call after the editctrl has been set

    GetEditorCtrl()->Show();
    GetDescWin()->Show();

    GetEditorCtrl()->DisplayData(0);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void OTableDesignView::resizeControl(Rectangle& _rRect)
{
    Size aToolBoxSize;
    ToolBox* pToolBox = getToolBox();
    if(pToolBox)
        aToolBoxSize = pToolBox->GetOutputSizePixel();
    Size aSize = GetOutputSizePixel();

    m_pWin->SetPosSizePixel(Point(0,aToolBoxSize.Height()),Size(aSize.Width(),aSize.Height() - aToolBoxSize.Height()));

    aToolBoxSize.Width() = _rRect.getWidth();
    _rRect.SetPos(Point(0,0));
    _rRect.SetSize(aToolBoxSize);
}
//------------------------------------------------------------------------------
IMPL_LINK( OTableDesignView, SwitchHdl, Accelerator*, pAcc )
{
    if( getController()->isReadOnly() )
        return 0;

    if( GetDescWin()->HasChildPathFocus() )
    {
        GetDescWin()->LoseFocus();
        GetEditorCtrl()->GrabFocus();
    }
    else
    {
        OTableRow* pRow = (*GetEditorCtrl()->GetRowList())[GetEditorCtrl()->GetCurRow()];
        OFieldDescription* pFieldDescr = pRow ? pRow->GetActFieldDescr() : NULL;
        if (pFieldDescr)
            GetDescWin()->GrabFocus();
        else
            GetEditorCtrl()->GrabFocus();
    }

    return 0;
}
// -----------------------------------------------------------------------------
sal_Bool OTableDesignView::isCutAllowed()
{
    sal_Bool bAllowed = sal_False;
    if( GetDescWin()->HasChildPathFocus() )
    {
        bAllowed = GetDescWin()->isCutAllowed();
    }
    else
    {
        bAllowed = GetEditorCtrl()->IsCutAllowed();
    }
    return bAllowed;
}
// -----------------------------------------------------------------------------
void OTableDesignView::copy()
{
    if( GetDescWin()->HasChildPathFocus() )
    {
        GetDescWin()->copy();
    }
    else
    {
        GetEditorCtrl()->Copy();
    }
}
// -----------------------------------------------------------------------------
void OTableDesignView::cut()
{
    if( GetDescWin()->HasChildPathFocus() )
    {
        GetDescWin()->cut();
    }
    else
    {
        GetEditorCtrl()->Cut();
    }
}
// -----------------------------------------------------------------------------
void OTableDesignView::paste()
{
    if( GetDescWin()->HasChildPathFocus() )
    {
        GetDescWin()->paste();
    }
    else
    {
        GetEditorCtrl()->Paste();
    }
}
// -----------------------------------------------------------------------------
// set the view readonly or not
void OTableDesignView::setReadOnly(sal_Bool _bReadOnly)
{
    GetDescWin()->SetReadOnly(_bReadOnly);
    GetEditorCtrl()->SetReadOnly(_bReadOnly);
}
// -----------------------------------------------------------------------------
void OTableDesignView::reSync()
{
    GetEditorCtrl()->DeactivateCell();
}
// -----------------------------------------------------------------------------

