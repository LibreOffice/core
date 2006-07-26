/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: browserline.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-26 07:52:16 $
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

#ifndef _EXTENSIONS_PROPCTRLR_BROWSERLINE_HXX_
#include "browserline.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_INSPECTION_PROPERTYLINEELEMENT_HPP_
#include <com/sun/star/inspection/PropertyLineElement.hpp>
#endif
/** === end UNO includes === **/

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif

//............................................................................
namespace pcr
{
//............................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::inspection::XPropertyControl;
    using ::com::sun::star::inspection::XPropertyControlContext;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::Exception;
    /** === end UNO using === **/

    namespace PropertyLineElement = ::com::sun::star::inspection::PropertyLineElement;

    //==================================================================
    //= OBrowserLine
    //==================================================================
    DBG_NAME(OBrowserLine)
    //------------------------------------------------------------------

    OBrowserLine::OBrowserLine( const ::rtl::OUString& _rEntryName, Window* pParent )
            :m_sEntryName( _rEntryName )
            ,m_aFtTitle(pParent)
            ,m_pControlWindow( NULL )
            ,m_pBrowseButton(NULL)
            ,m_pAdditionalBrowseButton( NULL )
            ,m_bIndentTitle( sal_False )
            ,m_nNameWidth(0)
            ,m_pTheParent(pParent)
            ,m_pClickListener( NULL )
            ,m_nEnableFlags( 0xFFFF )
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
            impl_layoutComponents();
        }
    }

    //------------------------------------------------------------------
    void OBrowserLine::SetComponentHelpIds( sal_uInt32 _nControlId, sal_uInt32 _bPrimaryButtonId, sal_uInt32 _nSecondaryButtonId )
    {
        if ( m_pControlWindow )
            m_pControlWindow->SetHelpId( _nControlId );

        if ( m_pBrowseButton )
        {
            m_pBrowseButton->SetHelpId( _nControlId );
            m_pBrowseButton->SetUniqueId( _bPrimaryButtonId );

            if ( m_pAdditionalBrowseButton )
            {
                m_pAdditionalBrowseButton->SetHelpId( _nControlId );
                m_pAdditionalBrowseButton->SetUniqueId( _nSecondaryButtonId );
            }
        }
    }

    //------------------------------------------------------------------
    void OBrowserLine::setControl( const Reference< XPropertyControl >& _rxControl )
    {
        m_xControl = _rxControl;
        m_pControlWindow = m_xControl.is() ? VCLUnoHelper::GetWindow( _rxControl->getControlWindow() ) : NULL;
        DBG_ASSERT( m_pControlWindow, "OBrowserLine::setControl: setting NULL controls/windows is not allowed!" );

        if ( m_pControlWindow )
        {
            m_pControlWindow->SetParent( m_pTheParent );
            m_pControlWindow->Show();
        }
        impl_layoutComponents();
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

        if ( m_pControlWindow )
        {
            Point aControlPos( aPos );
            aControlPos.X() = m_pControlWindow->GetPosPixel().X();
            m_pControlWindow->SetPosPixel( aControlPos );
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
        else if ( m_pControlWindow )
        {
            pRefWindow = m_pControlWindow;
        }
        return pRefWindow;
    }

    //------------------------------------------------------------------
    void OBrowserLine::SetTabOrder(Window* pRefWindow, sal_uInt16 nFlags )
    {
        m_aFtTitle.SetZOrder(pRefWindow,nFlags);
        if ( m_pControlWindow )
            m_pControlWindow->SetZOrder( (Window*)&m_aFtTitle, WINDOW_ZORDER_BEHIND );

        if ( m_pBrowseButton )
            m_pBrowseButton->SetZOrder( m_pControlWindow, WINDOW_ZORDER_BEHIND );

        if ( m_pAdditionalBrowseButton )
            m_pAdditionalBrowseButton->SetZOrder( m_pBrowseButton, WINDOW_ZORDER_BEHIND );
    }

    //------------------------------------------------------------------
    sal_Bool OBrowserLine::GrabFocus()
    {
        sal_Bool bRes=sal_False;

        if ( m_pControlWindow && m_pControlWindow->IsEnabled() )
        {
            m_pControlWindow->GrabFocus();
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

        impl_layoutComponents();
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
        if ( m_pControlWindow )
            m_pControlWindow->Show( bFlag );
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
    void OBrowserLine::impl_layoutComponents()
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

        if ( m_pControlWindow )
        {
            Point aControlPos( m_aLinePos.X() + m_nNameWidth, m_aLinePos.Y() + 2 );
            m_pControlWindow->SetPosPixel( aControlPos );

            Size aControlSize( m_aOutputSize.Width() - 4 - m_nNameWidth - nBrowseButtonSize - 4, m_pControlWindow->GetSizePixel().Height() );
            if ( m_pAdditionalBrowseButton )
                aControlSize.Width() -= nBrowseButtonSize + 4;
            m_pControlWindow->SetSizePixel( aControlSize );
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
    void OBrowserLine::SetTitle(const XubString& _rNewTtile )
    {
        if ( GetTitle() == _rNewTtile )
            return;
        // #99102# --------------
        m_aFtTitle.SetText( _rNewTtile );
        if ( m_pControlWindow )
            m_pControlWindow->SetAccessibleName( _rNewTtile );
        if ( m_pBrowseButton )
            m_pBrowseButton->SetAccessibleName( _rNewTtile );
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
        String sDisplayName = m_aFtTitle.GetText();
        sDisplayName.EraseTrailingChars( '.' );
        return sDisplayName;
    }

    //------------------------------------------------------------------
    sal_Bool OBrowserLine::IsPropertyInputEnabled( ) const
    {
        return ( m_nEnableFlags & PropertyLineElement::InputControl ) != 0;
    }

    //------------------------------------------------------------------
    namespace
    {
        void implSetBitIfAffected( sal_uInt16& _nEnabledBits, sal_Int16 _nAffectedMask, sal_Int16 _nTestBit, bool _bSet )
        {
            if ( _nAffectedMask & _nTestBit )
                if ( _bSet )
                    _nEnabledBits |= _nTestBit;
                else
                    _nEnabledBits &= ~_nTestBit;
        }

        void implEnable( Window* _pWindow, sal_uInt16 _nEnabledBits, sal_uInt16 _nMatchBits )
        {
            if ( _pWindow )
                _pWindow->Enable( ( _nEnabledBits & _nMatchBits ) == _nMatchBits );
        }
    }

    //------------------------------------------------------------------
    void OBrowserLine::implUpdateEnabledDisabled()
    {
        implEnable( &m_aFtTitle,                m_nEnableFlags, PropertyLineElement::CompleteLine );
        if ( m_pControlWindow )
            implEnable( m_pControlWindow,       m_nEnableFlags, PropertyLineElement::CompleteLine | PropertyLineElement::InputControl );
        implEnable( m_pBrowseButton,            m_nEnableFlags, PropertyLineElement::CompleteLine | PropertyLineElement::PrimaryButton );
        implEnable( m_pAdditionalBrowseButton,  m_nEnableFlags, PropertyLineElement::CompleteLine | PropertyLineElement::SecondaryButton );
    }

    //------------------------------------------------------------------
    void OBrowserLine::EnablePropertyLine( bool _bEnable )
    {
        implSetBitIfAffected( m_nEnableFlags, PropertyLineElement::CompleteLine, PropertyLineElement::CompleteLine, _bEnable );
        implUpdateEnabledDisabled();
    }

    //------------------------------------------------------------------
    void OBrowserLine::EnablePropertyControls( sal_Int16 _nControls, bool _bEnable )
    {
        implSetBitIfAffected( m_nEnableFlags, _nControls, PropertyLineElement::InputControl, _bEnable );
        implSetBitIfAffected( m_nEnableFlags, _nControls, PropertyLineElement::PrimaryButton, _bEnable );
        implSetBitIfAffected( m_nEnableFlags, _nControls, PropertyLineElement::SecondaryButton, _bEnable );
        implUpdateEnabledDisabled();
    }

    //------------------------------------------------------------------
    void OBrowserLine::ShowBrowseButton( const Image& _rImage, sal_Bool _bPrimary )
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

        impl_layoutComponents();
    }

    //------------------------------------------------------------------
    void OBrowserLine::implHideBrowseButton( sal_Bool _bPrimary, bool _bReLayout )
    {
        PushButton*& rpButton = _bPrimary ? m_pBrowseButton : m_pAdditionalBrowseButton;

        if ( rpButton )
        {
            rpButton->Hide();
            delete rpButton;
            rpButton = NULL;
        }

        if ( _bReLayout )
            impl_layoutComponents();
    }

    //------------------------------------------------------------------
    void OBrowserLine::HideBrowseButton( sal_Bool _bPrimary )
    {
        implHideBrowseButton( _bPrimary, true );
    }

    //------------------------------------------------------------------
    void OBrowserLine::SetTitleWidth(sal_uInt16 nWidth)
    {
        if (m_nNameWidth != nWidth+10)
        {
            m_nNameWidth = nWidth+10;
            impl_layoutComponents();
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
    IMPL_LINK( OBrowserLine, OnButtonFocus, PushButton*, /*pPB*/ )
    {
        if ( m_xControl.is() )
        {
            try
            {
                Reference< XPropertyControlContext > xContext( m_xControl->getControlContext(), UNO_QUERY_THROW );
                xContext->focusGained( m_xControl );
            }
            catch( const Exception& e )
            {
            #if OSL_DEBUG_LEVEL > 0
                ::rtl::OString sMessage( "OBrowserLine, OnButtonFocus: caught an exception!\n" );
                sMessage += "message:\n";
                sMessage += ::rtl::OString( e.Message.getStr(), e.Message.getLength(), osl_getThreadTextEncoding() );
                OSL_ENSURE( false, sMessage );
            #else
                e; // make compiler happy
            #endif
            }
        }
        return 0;
    }
//............................................................................
} // namespace pcr
//............................................................................

