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
#include "precompiled_svtools.hxx"
#include <svtools/helpagentwindow.hxx>
#include <osl/diagnose.h>
#include <vcl/button.hxx>
#include <vcl/bitmap.hxx>
#include <svtools/svtdata.hxx>
#include <svtools/svtools.hrc>
#include <svtools/helpid.hrc>

#define WB_AGENT_STYLE  0

//........................................................................
namespace svt
{
//........................................................................

    using namespace ::com::sun::star::uno;
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
        static_cast<CloserButton_Impl*>(m_pCloser)->SetModeImage( aCloserImage );
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
        AlwaysEnableInput( sal_True, sal_True );

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

