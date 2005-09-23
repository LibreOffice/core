/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AppTitleWindow.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-23 12:17:30 $
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
#ifndef DBAUI_TITLE_WINDOW_HXX
#include "AppTitleWindow.hxx"
#endif
#include "memory"
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

namespace dbaui
{

DBG_NAME(OTitleWindow)
OTitleWindow::OTitleWindow(Window* _pParent,USHORT _nTitleId,WinBits _nBits,BOOL _bShift)
: Window(_pParent,_nBits | WB_DIALOGCONTROL)
, m_aSpace1(this)
, m_aSpace2(this)
, m_aTitle(this)
, m_pChild(NULL)
, m_bShift(_bShift)
{
    DBG_CTOR(OTitleWindow,NULL);

    setTitle(_nTitleId);
    SetBorderStyle(WINDOW_BORDER_MONO);
    ImplInitSettings( sal_True, sal_True, sal_True );

    Window* pWindows [] = { &m_aSpace1, &m_aSpace2, &m_aTitle};
    for (sal_Int32 i=0; i < sizeof(pWindows)/sizeof(pWindows[0]); ++i)
    {
        pWindows[i]->Show();
    }
}
// -----------------------------------------------------------------------------
OTitleWindow::~OTitleWindow()
{
    if ( m_pChild )
    {
        m_pChild->Hide();
        ::std::auto_ptr<Window> aTemp(m_pChild);
        m_pChild = NULL;
    }

    DBG_DTOR(OTitleWindow,NULL);
}
// -----------------------------------------------------------------------------
void OTitleWindow::setChildWindow(Window* _pChild)
{
    m_pChild = _pChild;
}
#define SPACE_BORDER    1
// -----------------------------------------------------------------------------
void OTitleWindow::Resize()
{
    //////////////////////////////////////////////////////////////////////
    // Abmessungen parent window
    Size aOutputSize( GetOutputSize() );
    long nOutputWidth   = aOutputSize.Width();
    long nOutputHeight  = aOutputSize.Height();

    Size aTextSize = LogicToPixel( Size( 6, 3 ), MAP_APPFONT );
    sal_Int32 nXOffset = aTextSize.Width();
    sal_Int32 nYOffset = aTextSize.Height();
    sal_Int32 nHeight = GetTextHeight() + 2*nYOffset;

    m_aSpace1.SetPosSizePixel(  Point(SPACE_BORDER, SPACE_BORDER ),
                                Size(nXOffset , nHeight - SPACE_BORDER) );
    m_aSpace2.SetPosSizePixel(  Point(nXOffset + SPACE_BORDER, SPACE_BORDER ),
                                Size(nOutputWidth - nXOffset - 2*SPACE_BORDER, nYOffset) );
    m_aTitle.SetPosSizePixel(   Point(nXOffset + SPACE_BORDER, nYOffset + SPACE_BORDER),
                                Size(nOutputWidth - nXOffset - 2*SPACE_BORDER, nHeight - nYOffset - SPACE_BORDER) );
    if ( m_pChild )
    {
        m_pChild->SetPosSizePixel(  Point(m_bShift ? (nXOffset+SPACE_BORDER) : sal_Int32(SPACE_BORDER), nHeight + nXOffset + SPACE_BORDER),
                                    Size(nOutputWidth - ( m_bShift ? (2*nXOffset - 2*SPACE_BORDER) : sal_Int32(SPACE_BORDER) ), nOutputHeight - nHeight - 2*nXOffset - 2*SPACE_BORDER) );
    }
}
// -----------------------------------------------------------------------------
void OTitleWindow::setTitle(USHORT _nTitleId)
{
    if ( _nTitleId != 0 )
    {
        m_aTitle.SetText(ModuleRes(_nTitleId));
    }
}
// -----------------------------------------------------------------------------
void OTitleWindow::GetFocus()
{
    Window::GetFocus();
    if ( m_pChild )
        m_pChild->GrabFocus();
}
// -----------------------------------------------------------------------------
long OTitleWindow::GetWidthPixel() const
{
    Size aTextSize = LogicToPixel( Size( 12, 0 ), MAP_APPFONT );
    sal_Int32 nWidth = GetTextWidth(m_aTitle.GetText()) + 2*aTextSize.Width();

    return nWidth;
}
// -----------------------------------------------------------------------
void OTitleWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings( sal_True, sal_True, sal_True );
        Invalidate();
    }
}
//-----------------------------------------------------------------------------
void OTitleWindow::ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground )
{
    AllSettings aAllSettings = GetSettings();
    StyleSettings aStyle = aAllSettings.GetStyleSettings();
    aStyle.SetMonoColor(aStyle.GetActiveBorderColor());//GetMenuBorderColor());
    aAllSettings.SetStyleSettings(aStyle);
    SetSettings(aAllSettings);

    Window* pWindows [] = { &m_aSpace1, &m_aSpace2, &m_aTitle};
    for (sal_Int32 i=0; i < sizeof(pWindows)/sizeof(pWindows[0]); ++i)
    {
        Font aFont = pWindows[i]->GetFont();
        aFont.SetWeight(WEIGHT_BOLD);
        pWindows[i]->SetFont(aFont);
        pWindows[i]->SetTextColor( aStyle.GetLightColor() );
        pWindows[i]->SetBackground( Wallpaper( aStyle.GetShadowColor() ) );
    }
}
// .............................................................
} // namespace dbaui
// .............................................................
