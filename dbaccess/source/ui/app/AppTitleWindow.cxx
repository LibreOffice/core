/*************************************************************************
 *
 *  $RCSfile: AppTitleWindow.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:30:14 $
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

namespace dbaui
{
OTitleWindow::OTitleWindow(Window* _pParent,USHORT _nTitleId,WinBits _nBits,BOOL _bShift)
: Window(_pParent,_nBits | WB_DIALOGCONTROL)
, m_aSpace1(this)
, m_aSpace2(this)
, m_aTitle(this)
, m_pChild(NULL)
, m_bShift(_bShift)
{
    setTitle(_nTitleId);
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    Window* pWindows [] = { &m_aSpace1, &m_aSpace2, &m_aTitle};
    for (sal_Int32 i=0; i < sizeof(pWindows)/sizeof(pWindows[0]); ++i)
    {
        Font aFont = pWindows[i]->GetFont();
        aFont.SetWeight(WEIGHT_BOLD);
        pWindows[i]->SetFont(aFont);
        pWindows[i]->SetTextColor( rSettings.GetLightColor() );
        pWindows[i]->SetBackground( Wallpaper( rSettings.GetShadowColor() ) );
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
}
// -----------------------------------------------------------------------------
void OTitleWindow::setChildWindow(Window* _pChild)
{
    m_pChild = _pChild;
}
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

    m_aSpace1.SetPosSizePixel(  Point(0, 0 ),
                                Size(nXOffset, nHeight ) );
    m_aSpace2.SetPosSizePixel(  Point(nXOffset, 0 ),
                                Size(nOutputWidth - nXOffset, nYOffset) );
    m_aTitle.SetPosSizePixel(   Point(nXOffset, nYOffset ),
                                Size(nOutputWidth - nXOffset, nHeight - nYOffset ) );
    if ( m_pChild )
    {
        m_pChild->SetPosSizePixel(  Point(m_bShift ? nXOffset : sal_Int32(0), nHeight + nXOffset ),
                                    Size(nOutputWidth - ( m_bShift ? 2*nXOffset : sal_Int32(0) ), nOutputHeight - nHeight - 2*nXOffset) );
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
    sal_Int32 nWidth = GetTextWidth(m_aTitle.GetText());

    return nWidth;
}
// .............................................................
} // namespace dbaui
// .............................................................
