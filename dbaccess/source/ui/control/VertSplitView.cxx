/*************************************************************************
 *
 *  $RCSfile: VertSplitView.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:35:56 $
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
#ifndef DBAUI_VERTSPLITVIEW_HXX
#include "VertSplitView.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#define SPLITTER_WIDTH  80

using namespace ::dbaui;

//==================================================================
// class OSplitterView
//==================================================================
DBG_NAME(OSplitterView);
OSplitterView::OSplitterView(Window* _pParent,sal_Bool _bVertical) : Window(_pParent,WB_DIALOGCONTROL) // ,WB_BORDER
    ,m_pSplitter( NULL )
    ,m_pLeft(NULL)
    ,m_pRight(NULL)
    ,m_bVertical(_bVertical)
{
    DBG_CTOR(OSplitterView,NULL);
    ImplInitSettings( sal_True, sal_True, sal_True );
}
// -----------------------------------------------------------------------------
OSplitterView::~OSplitterView()
{
    DBG_DTOR(OSplitterView,NULL);
    m_pRight = m_pLeft = NULL;
}
//------------------------------------------------------------------------------
IMPL_LINK( OSplitterView, SplitHdl, Splitter*, pSplit )
{
    OSL_ENSURE(m_pSplitter, "Splitter is NULL!");
    if ( m_bVertical )
    {
        long nTest = m_pSplitter->GetPosPixel().Y();
        m_pSplitter->SetPosPixel( Point( m_pSplitter->GetSplitPosPixel(), m_pSplitter->GetPosPixel().Y() ) );
    }
    else
        m_pSplitter->SetPosPixel( Point( m_pSplitter->GetPosPixel().X(),m_pSplitter->GetSplitPosPixel() ) );

    Resize();
    return 0L;
}
// -----------------------------------------------------------------------------
void OSplitterView::ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground )
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
void OSplitterView::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings( sal_True, sal_True, sal_True );
        Invalidate();
    }
}
// -----------------------------------------------------------------------------
void OSplitterView::GetFocus()
{
    Window::GetFocus();

    // forward the focus to the current cell of the editor control
    if ( m_pLeft )
        m_pLeft->GrabFocus();
    else if ( m_pRight )
        m_pRight->GrabFocus();
}

// -------------------------------------------------------------------------
void OSplitterView::Resize()
{
    Window::Resize();
    OSL_ENSURE( m_pRight, "No init called!");

    Point   aSplitPos;
    Size    aSplitSize;
    Point   aPlaygroundPos( 0,0 );
    Size    aPlaygroundSize( GetOutputSizePixel() );

    if ( m_pLeft && m_pLeft->IsVisible() && m_pSplitter )
    {
        aSplitPos   = m_pSplitter->GetPosPixel();
        aSplitSize  = m_pSplitter->GetOutputSizePixel();
        if ( m_bVertical )
        {
            // calculate the splitter pos and size
            aSplitPos.Y() = aPlaygroundPos.Y();
            aSplitSize.Height() = aPlaygroundSize.Height();

            if( ( aSplitPos.X() + aSplitSize.Width() ) > ( aPlaygroundSize.Width() ))
                aSplitPos.X() = aPlaygroundSize.Width() - aSplitSize.Width();

            if( aSplitPos.X() <= aPlaygroundPos.X() )
                aSplitPos.X() = aPlaygroundPos.X() + sal_Int32(aPlaygroundSize.Width() * 0.3);

            // the tree pos and size
            Point   aTreeViewPos( aPlaygroundPos );
            Size    aTreeViewSize( aSplitPos.X(), aPlaygroundSize.Height() );

            // set the size of treelistbox
            m_pLeft->SetPosSizePixel( aTreeViewPos, aTreeViewSize );

            //set the size of the splitter
            m_pSplitter->SetPosSizePixel( aSplitPos, Size( aSplitSize.Width(), aPlaygroundSize.Height() ) );
            m_pSplitter->SetDragRectPixel( Rectangle(aPlaygroundPos,aPlaygroundSize) );
        }
        else
        {
            aSplitPos.X() = aPlaygroundPos.X();
            aSplitSize.Width() = aPlaygroundSize.Width();

            if( ( aSplitPos.Y() + aSplitSize.Height() ) > ( aPlaygroundSize.Height() ))
                aSplitPos.Y() = aPlaygroundSize.Height() - aSplitSize.Height();

            if( aSplitPos.Y() <= aPlaygroundPos.Y() )
                aSplitPos.Y() = aPlaygroundPos.Y() + sal_Int32(aPlaygroundSize.Height() * 0.3);

            // the tree pos and size
            Point   aTreeViewPos( aPlaygroundPos );
            Size    aTreeViewSize( aPlaygroundSize.Width() ,aSplitPos.Y());

            // set the size of treelistbox
            m_pLeft->SetPosSizePixel( aTreeViewPos, aTreeViewSize );

            //set the size of the splitter
            m_pSplitter->SetPosSizePixel( aSplitPos, Size( aPlaygroundSize.Width(), aSplitSize.Height() ) );
            m_pSplitter->SetDragRectPixel( Rectangle(aPlaygroundPos,aPlaygroundSize) );
        }
    }

    if ( m_pRight )
    {
        if ( m_bVertical )
            m_pRight->SetPosSizePixel( aSplitPos.X() + aSplitSize.Width(), aPlaygroundPos.Y(),
                                   aPlaygroundSize.Width() - aSplitSize.Width() - aSplitPos.X(), aPlaygroundSize.Height());
        else
            m_pRight->SetPosSizePixel( aSplitPos.X(), aPlaygroundPos.Y() + aSplitPos.Y() + aSplitSize.Height(),
                                   aPlaygroundSize.Width() , aPlaygroundSize.Height() - aSplitSize.Height() - aSplitPos.Y());
    }

}
// -----------------------------------------------------------------------------
void OSplitterView::init(Window* _pRight,Window* _pLeft)
{
    m_pLeft = _pLeft;
    m_pRight = _pRight;
}
// -----------------------------------------------------------------------------
void OSplitterView::setSplitter(Splitter* _pSplitter)
{
    m_pSplitter = _pSplitter;
    if ( m_pSplitter )
    {
        m_pSplitter->SetSplitPosPixel( LogicToPixel( Size( SPLITTER_WIDTH, 0 ), MAP_APPFONT ).Width() );
        m_pSplitter->SetSplitHdl( LINK(this, OSplitterView, SplitHdl) );
        m_pSplitter->Show();
        LINK( this, OSplitterView, SplitHdl ).Call(m_pSplitter);
    }
}
