/*************************************************************************
 *
 *  $RCSfile: browserline.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 12:00:17 $
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

#ifndef _EXTENSIONS_PROPCTRLR_BROWSERLINE_HXX_
#include "browserline.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_BRWCONTROLLISTENER_HXX_
#include "brwcontrollistener.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

//............................................................................
namespace pcr
{
//............................................................................

    //==================================================================
    //= OBrowserLine
    //==================================================================
    DBG_NAME(OBrowserLine)
    //------------------------------------------------------------------

    OBrowserLine::OBrowserLine( Window* pParent)
            :m_aFtTitle(pParent)
            ,m_nFlags( 0 )
            ,m_pBrowseButton(NULL)
            ,m_pAdditionalBrowseButton( NULL )
            ,m_pBrowserControl(NULL)
            ,m_bIndentTitle( sal_False )
            ,m_nNameWidth(0)
            ,m_pTheParent(pParent)
            ,m_eControlType(BCT_UNDEFINED)
            ,m_pClickListener( NULL )
            ,m_nEnableFlags( ENABLED_ALL )
    {
        DBG_CTOR(OBrowserLine,NULL);
        m_aFtTitle.Show();
    }

    //------------------------------------------------------------------
    OBrowserLine::~OBrowserLine()
    {
        implHideBrowseButton( true, false );
        implHideBrowseButton( false, false );

        DBG_DTOR(OBrowserLine,NULL);
    }

    //------------------------------------------------------------------
    void OBrowserLine::IndentTitle( sal_Bool _bIndent )
    {
        if ( m_bIndentTitle != _bIndent )
        {
            m_bIndentTitle = _bIndent;
            layoutComponents();
        }
    }

    //------------------------------------------------------------------
    void OBrowserLine::SetComponentHelpIds( sal_uInt32 _nControlId, sal_uInt32 _bPrimaryButtonId, sal_uInt32 _nSecondaryButtonId )
    {
        if ( m_pBrowserControl )
            m_pBrowserControl->GetMe()->SetHelpId( _nControlId );

        bool bTwoButtons = ( m_pBrowseButton != NULL ) && ( m_pAdditionalBrowseButton != NULL );

        if ( m_pBrowseButton )
        {
            m_pBrowseButton->SetHelpId( bTwoButtons ? _bPrimaryButtonId : _nControlId );
            m_pBrowseButton->SetUniqueId( _bPrimaryButtonId );

            if ( m_pAdditionalBrowseButton )
            {
                m_pAdditionalBrowseButton->SetHelpId( _nSecondaryButtonId );
                m_pAdditionalBrowseButton->SetUniqueId( _nSecondaryButtonId );
            }
        }
    }

    //------------------------------------------------------------------
    void OBrowserLine::setControl(IBrowserControl* pXControl)
    {
        m_pBrowserControl = pXControl;
        m_pBrowserControl->GetMe()->Show();
        layoutComponents();
    }

    //------------------------------------------------------------------
    IBrowserControl* OBrowserLine::getControl()
    {
        return m_pBrowserControl;
    }

    //------------------------------------------------------------------
    void OBrowserLine::SetPosPixel(Point aPosPoint)
    {
        m_aLinePos = aPosPoint;
        Point aPos( m_aLinePos );
        aPos.Y()+=2;

        Point aTitlePos(m_aLinePos);
        aTitlePos.Y()+=8;
        m_aFtTitle.SetPosPixel(aTitlePos);

        if ( m_pBrowserControl )
        {
            Point aControlPos( aPos );
            aControlPos.X() = m_pBrowserControl->GetCtrPos().X();
            m_pBrowserControl->SetCtrPos( aControlPos );
        }

        if ( m_pBrowseButton )
        {
            Point aButtonPos( aPos );
            aButtonPos.X() = m_pBrowseButton->GetPosPixel().X();
            m_pBrowseButton->SetPosPixel( aButtonPos );
        }

        if ( m_pAdditionalBrowseButton )
        {
            Point aButtonPos( aPos );
            aButtonPos.X() = m_pAdditionalBrowseButton->GetPosPixel().X();
            m_pAdditionalBrowseButton->SetPosPixel( aButtonPos );
        }
    }

    //------------------------------------------------------------------
    Window* OBrowserLine::GetRefWindow()
    {
        Window* pRefWindow=&m_aFtTitle;

        if(m_pBrowseButton)
        {
            pRefWindow=(Window*)m_pBrowseButton;
        }
        else if(m_pBrowserControl)
        {
            pRefWindow=m_pBrowserControl->GetMe();
        }
        return pRefWindow;
    }

    //------------------------------------------------------------------
    void OBrowserLine::SetTabOrder(Window* pRefWindow, sal_uInt16 nFlags )
    {
        m_aFtTitle.SetZOrder(pRefWindow,nFlags);
        if ( m_pBrowserControl )
            m_pBrowserControl->GetMe()->SetZOrder( (Window*)&m_aFtTitle, WINDOW_ZORDER_BEHIND );

        if ( m_pBrowseButton )
            m_pBrowseButton->SetZOrder( m_pBrowserControl->GetMe(), WINDOW_ZORDER_BEHIND );

        if ( m_pAdditionalBrowseButton )
            m_pAdditionalBrowseButton->SetZOrder( m_pBrowseButton, WINDOW_ZORDER_BEHIND );
    }

    //------------------------------------------------------------------
    sal_Bool OBrowserLine::GrabFocus()
    {
        sal_Bool bRes=sal_False;

        if ( m_pBrowserControl && m_pBrowserControl->GetMe()->IsEnabled() )
        {
            m_pBrowserControl->GetMe()->GrabFocus();
            bRes = sal_True;
        }
        else if ( m_pAdditionalBrowseButton && m_pAdditionalBrowseButton->IsEnabled() )
        {
            m_pAdditionalBrowseButton->GrabFocus();
            bRes = sal_True;
        }
        else if ( m_pBrowseButton && m_pBrowseButton->IsEnabled() )
        {
            m_pBrowseButton->GrabFocus();
            bRes = sal_True;
        }
        return bRes;
    }

    //------------------------------------------------------------------
    void OBrowserLine::SetPosSizePixel( Point _rPos, Size _rSize )
    {
        m_aLinePos = _rPos;
        m_aOutputSize = _rSize;

        layoutComponents();
    }

    //------------------------------------------------------------------
    Size OBrowserLine::GetSizePixel()
    {
        return m_aOutputSize;
    }

    //------------------------------------------------------------------
    void OBrowserLine::Show(sal_Bool bFlag)
    {
        m_aFtTitle.Show(bFlag);
        if ( m_pBrowserControl )
            m_pBrowserControl->GetMe()->Show( bFlag );
        if ( m_pBrowseButton )
            m_pBrowseButton->Show( bFlag );
        if ( m_pAdditionalBrowseButton )
            m_pAdditionalBrowseButton->Show( bFlag );
    }

    //------------------------------------------------------------------
    void OBrowserLine::Hide()
    {
        Show(sal_False);
    }

    //------------------------------------------------------------------
    sal_Bool OBrowserLine::IsVisible()
    {
        return m_aFtTitle.IsVisible();
    }

    //------------------------------------------------------------------
    void OBrowserLine::SetFlags( sal_uInt16 _nFlags )
    {
        m_nFlags = _nFlags;
    }

    //------------------------------------------------------------------
    sal_uInt16 OBrowserLine::GetFlags()
    {
        return m_nFlags;
    }

    //------------------------------------------------------------------
    void OBrowserLine::layoutComponents()
    {
        {
            Point aTitlePos( m_aLinePos.X(), m_aLinePos.Y() + 8 );
            Size aTitleSize( m_nNameWidth - 3, m_aOutputSize.Height() );

            if ( m_bIndentTitle )
            {
                Size aIndent( m_pTheParent->LogicToPixel( Size( 8, 0 ), MAP_APPFONT ) );
                aTitlePos.X() += aIndent.Width();
                aTitleSize.Width() -= aIndent.Width();
            }
            m_aFtTitle.SetPosSizePixel( aTitlePos, aTitleSize );
        }

        sal_Int32 nBrowseButtonSize = m_aOutputSize.Height() - 4;

        if ( m_pBrowserControl )
        {
            Point aControlPos( m_aLinePos.X() + m_nNameWidth, m_aLinePos.Y() + 2 );
            m_pBrowserControl->SetCtrPos( aControlPos );

            Size aControlSize( m_aOutputSize.Width() - 4 - m_nNameWidth - nBrowseButtonSize - 4, m_pBrowserControl->GetCtrSize().Height() );
            if ( m_pAdditionalBrowseButton )
                aControlSize.Width() -= nBrowseButtonSize + 4;
            m_pBrowserControl->SetCtrSize( aControlSize );
        }

        if ( m_pBrowseButton )
        {
            Point aButtonPos( m_aOutputSize.Width() - 4 - nBrowseButtonSize, m_aLinePos.Y() + 2 );
            Size aButtonSize( nBrowseButtonSize, nBrowseButtonSize );
            m_pBrowseButton->SetPosSizePixel( aButtonPos, aButtonSize );

            if ( m_pAdditionalBrowseButton )
            {
                aButtonPos.X() -= nBrowseButtonSize + 4;
                m_pAdditionalBrowseButton->SetPosSizePixel( aButtonPos, aButtonSize );
            }
        }
    }

    //------------------------------------------------------------------
    void OBrowserLine::SetTitle(const XubString& rString )
    {
        String aText(rString);
        // #99102# --------------
        m_aFtTitle.SetText(aText);
        if ( m_pBrowserControl )
            m_pBrowserControl->GetMe()->SetAccessibleName( rString );
        if ( m_pBrowseButton )
            m_pBrowseButton->SetAccessibleName( rString );
        FullFillTitleString();
    }

    // #99102# ---------------------------------------------------------
    void OBrowserLine::FullFillTitleString()
    {
        if( m_pTheParent )
        {
            String aText = m_aFtTitle.GetText();
            while( m_pTheParent->GetTextWidth( aText ) < m_nNameWidth )
                aText.AppendAscii("...........");
            m_aFtTitle.SetText(aText);
        }
    }

    //------------------------------------------------------------------
    XubString OBrowserLine::GetTitle() const
    {
        return m_aFtTitle.GetText();
    }

    //------------------------------------------------------------------
    void OBrowserLine::SetKindOfControl(BrowserControlType eKOC)
    {
        m_eControlType=eKOC;
    }

    //------------------------------------------------------------------
    BrowserControlType OBrowserLine::GetKindOfControl()
    {
        return m_eControlType;
    }

    //------------------------------------------------------------------
    sal_Bool OBrowserLine::IsPropertyInputEnabled( ) const
    {
        return ( m_nEnableFlags & ENABLED_INPUT ) != 0;
    }

    //------------------------------------------------------------------
    namespace
    {
        void implSetFlag( sal_uInt16& _nEnabledFlags, bool _bSet, sal_uInt16 _nMask )
        {
            if ( _bSet )
                _nEnabledFlags |= _nMask;
            else
                _nEnabledFlags &= ~_nMask;
        }

        void implEnable( Window* _pWindow, sal_uInt16 _nEnabledFlags, sal_uInt16 _nMatchAllMask )
        {
            if ( _pWindow )
                _pWindow->Enable( ( _nEnabledFlags & _nMatchAllMask ) == _nMatchAllMask );
        }
    }

    //------------------------------------------------------------------
    void OBrowserLine::implUpdateEnabledDisabled()
    {
        implEnable( &m_aFtTitle,               m_nEnableFlags, ENABLED_LINE );
        if ( m_pBrowserControl )
            m_pBrowserControl->GetMe()->Enable( ( m_nEnableFlags & ( ENABLED_LINE | ENABLED_INPUT ) ) == ( ENABLED_LINE | ENABLED_INPUT ) );
        implEnable( m_pBrowseButton,           m_nEnableFlags, ENABLED_LINE | ENABLED_PRIMARY );
        implEnable( m_pAdditionalBrowseButton, m_nEnableFlags, ENABLED_LINE | ENABLED_SECONDARY );
    }

    //------------------------------------------------------------------
    void OBrowserLine::EnablePropertyLine( bool _bEnable )
    {
        implSetFlag( m_nEnableFlags, _bEnable, ENABLED_LINE );
        implUpdateEnabledDisabled();
    }

    //------------------------------------------------------------------
    void OBrowserLine::EnablePropertyControls( bool _bEnableInput, bool _bEnablePrimaryButton, bool _bEnableSecondaryButton )
    {
        implSetFlag( m_nEnableFlags, _bEnableInput, ENABLED_INPUT );
        implSetFlag( m_nEnableFlags, _bEnablePrimaryButton, ENABLED_PRIMARY );
        implSetFlag( m_nEnableFlags, _bEnableSecondaryButton, ENABLED_SECONDARY );
        implUpdateEnabledDisabled();
    }

    //------------------------------------------------------------------
    void OBrowserLine::ShowBrowseButton( const Image& _rImage, bool _bPrimary )
    {
        PushButton*& rpButton = _bPrimary ? m_pBrowseButton : m_pAdditionalBrowseButton;

        if ( !rpButton )
        {
            rpButton = new PushButton( m_pTheParent, WB_NOPOINTERFOCUS );
            rpButton->SetGetFocusHdl( LINK( this, OBrowserLine, OnButtonFocus ) );
            rpButton->SetClickHdl( LINK( this, OBrowserLine, OnButtonClicked ) );
            rpButton->SetData( (void*)this );
            if ( !_rImage )
                rpButton->SetText( String::CreateFromAscii( "..." ) );
            else
                rpButton->SetModeImage( _rImage );
        }
        rpButton->Show();

        layoutComponents();
    }

    //------------------------------------------------------------------
    void OBrowserLine::implHideBrowseButton( bool _bPrimary, bool _bReLayout )
    {
        PushButton*& rpButton = _bPrimary ? m_pBrowseButton : m_pAdditionalBrowseButton;

        if ( rpButton )
        {
            rpButton->Hide();
            delete rpButton;
            rpButton = NULL;
        }

        if ( _bReLayout )
            layoutComponents();
    }

    //------------------------------------------------------------------
    void OBrowserLine::HideBrowseButton( bool _bPrimary )
    {
        implHideBrowseButton( _bPrimary, true );
    }

    //------------------------------------------------------------------
    void OBrowserLine::SetTitleWidth(sal_uInt16 nWidth)
    {
        if (m_nNameWidth != nWidth+10)
        {
            m_nNameWidth = nWidth+10;
            layoutComponents();
        }
        // #99102# ---------
        FullFillTitleString();
    }

    //------------------------------------------------------------------
    void OBrowserLine::SetClickListener( IButtonClickListener* _pListener )
    {
        m_pClickListener = _pListener;
    }

    //------------------------------------------------------------------
    IMPL_LINK( OBrowserLine, OnButtonClicked, PushButton*, _pButton )
    {
        if ( m_pClickListener )
            m_pClickListener->buttonClicked( this, _pButton == m_pBrowseButton );

        return 0L;
    }

    //------------------------------------------------------------------
    IMPL_LINK( OBrowserLine, OnButtonFocus, PushButton*, pPB )
    {
        if(m_pBrowserControl)
        {
            IBrowserControlListener* pListener = m_pBrowserControl->getListener();
            if (pListener)
                pListener->GetFocus(m_pBrowserControl);
        }
        return 0;
    }
//............................................................................
} // namespace pcr
//............................................................................

