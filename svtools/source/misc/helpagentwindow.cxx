/*************************************************************************
 *
 *  $RCSfile: helpagentwindow.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: vg $ $Date: 2003-07-22 11:12:03 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SVTOOLS_HELPAGENTWIDNOW_HXX_
#include "helpagentwindow.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif
#ifndef _SVTOOLS_SVTDATA_HXX
#include <svtdata.hxx>
#endif

#ifndef _SVTOOLS_HRC
#include "svtools.hrc"
#endif
#ifndef _SVT_HELPID_HRC
#include "helpid.hrc"
#endif

#define WB_AGENT_STYLE  0

//........................................................................
namespace svt
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::lang;

    //====================================================================
    //= CloserButton_Impl
    //= overload of ImageButton, because sometimes vcl doesn't call the click handler
    //====================================================================
    //--------------------------------------------------------------------
    class CloserButton_Impl : public ImageButton
    {
    public:
        CloserButton_Impl( Window* pParent, WinBits nBits ) : ImageButton( pParent, nBits ) {}

        virtual void        MouseButtonUp( const MouseEvent& rMEvt );
    };

    //--------------------------------------------------------------------
    void CloserButton_Impl::MouseButtonUp( const MouseEvent& rMEvt )
    {
        ImageButton::MouseButtonUp( rMEvt );
        GetClickHdl().Call( this );
    }

    //====================================================================
    //= HelpAgentWindow
    //====================================================================
    //--------------------------------------------------------------------
    HelpAgentWindow::HelpAgentWindow( Window* _pParent )
        :FloatingWindow( _pParent, WB_AGENT_STYLE)
        ,m_pCloser(NULL)
        ,m_pCallback(NULL)
    {
        // -----------------
        // the closer button
        Bitmap aCloserBitmap(SvtResId(BMP_HELP_AGENT_CLOSER));
        Image aCloserImage( aCloserBitmap, Color(COL_LIGHTMAGENTA) );
        m_pCloser = new CloserButton_Impl( this, WB_NOTABSTOP | WB_NOPOINTERFOCUS );
        static_cast<CloserButton_Impl*>(m_pCloser)->SetImage( aCloserImage );
        static_cast<CloserButton_Impl*>(m_pCloser)->SetClickHdl( LINK(this, HelpAgentWindow, OnButtonClicked) );
        m_pCloser->SetSizePixel( implOptimalButtonSize(aCloserImage) );
        m_pCloser->Show();
        m_pCloser->SetZOrder( NULL, WINDOW_ZORDER_LAST );

        // ----------------------------
        // calculate our preferred size
        Bitmap aHelpAgentBitmap(SvtResId(BMP_HELP_AGENT_IMAGE));
        m_aPicture = Image( aHelpAgentBitmap );
        m_aPreferredSize = m_aPicture.GetSizePixel();
        m_aPreferredSize.Width() += 2;
        m_aPreferredSize.Height() += 2;

        Size aSize = GetSizePixel();
        Size aOutputSize = GetOutputSizePixel();
        m_aPreferredSize.Width() += aSize.Width() - aOutputSize.Width();
        m_aPreferredSize.Height() += aSize.Height() - aOutputSize.Height();

        SetPointer(Pointer(POINTER_REFHAND));
        AlwaysEnableInput( TRUE, TRUE );

        // unique id for the testtool
        SetUniqueId( HID_HELPAGENT_WINDOW );
    }

    //--------------------------------------------------------------------
    HelpAgentWindow::~HelpAgentWindow()
    {
        if (m_pCloser && m_pCloser->IsTracking())
            m_pCloser->EndTracking();
        if (m_pCloser && m_pCloser->IsMouseCaptured())
            m_pCloser->ReleaseMouse();

        delete m_pCloser;
    }

    //--------------------------------------------------------------------
    void HelpAgentWindow::Paint( const Rectangle& rRect )
    {
        FloatingWindow::Paint(rRect);

        Size        aOutputSize( GetOutputSizePixel() );
        Point       aPoint=Point();
        Rectangle   aOutputRect( aPoint, aOutputSize );
        Rectangle   aInnerRect( aOutputRect );

        // paint the background
        SetLineColor( GetSettings().GetStyleSettings().GetFaceColor() );
        SetFillColor( GetSettings().GetStyleSettings().GetFaceColor() );
        DrawRect( aOutputRect );

        // paint the image
        Size aPictureSize( m_aPicture.GetSizePixel() );
        Point aPicturePos(
            aOutputRect.Left() + (aInnerRect.GetWidth() - aPictureSize.Width()) / 2,
            aOutputRect.Top() + (aInnerRect.GetHeight() - aPictureSize.Height()) / 2 );

        DrawImage( aPicturePos, m_aPicture, 0 );
    }

    //--------------------------------------------------------------------
    void HelpAgentWindow::MouseButtonUp( const MouseEvent& rMEvt )
    {
        FloatingWindow::MouseButtonUp(rMEvt);

        if (m_pCallback)
            m_pCallback->helpRequested();
    }

    //--------------------------------------------------------------------
    Size HelpAgentWindow::implOptimalButtonSize( const Image& _rButtonImage )
    {
        Size aPreferredSize = _rButtonImage.GetSizePixel();
        // add a small frame, needed by the button
        aPreferredSize.Width() += 5;
        aPreferredSize.Height() += 5;
        return aPreferredSize;
    }

    //--------------------------------------------------------------------
    void HelpAgentWindow::Resize()
    {
        FloatingWindow::Resize();

        Size aOutputSize = GetOutputSizePixel();
        Size aCloserSize = m_pCloser->GetSizePixel();
        if (m_pCloser)
            m_pCloser->SetPosPixel( Point(aOutputSize.Width() - aCloserSize.Width() - 3, 4) );
    }

    //--------------------------------------------------------------------
    IMPL_LINK( HelpAgentWindow, OnButtonClicked, Window*, _pWhichOne )
    {
        if (m_pCloser == _pWhichOne)
            if (m_pCallback)
                m_pCallback->closeAgent();
        return 0L;
    }

//........................................................................
}   // namespace svt
//........................................................................

