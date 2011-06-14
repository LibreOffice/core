/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"
#include "browserline.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/inspection/PropertyLineElement.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
/** === end UNO includes === **/

#include <vcl/svapp.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <tools/urlobj.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/componentcontext.hxx>

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
    using ::com::sun::star::graphic::XGraphicProvider;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::graphic::XGraphic;
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
            ,m_pClickListener( NULL )
            ,m_pTheParent(pParent)
            ,m_nNameWidth(0)
            ,m_nEnableFlags( 0xFFFF )
            ,m_bIndentTitle( false )
            ,m_bReadOnly( false )
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
    void OBrowserLine::IndentTitle( bool _bIndent )
    {
        if ( m_bIndentTitle != _bIndent )
        {
            m_bIndentTitle = _bIndent;
            impl_layoutComponents();
        }
    }

    //------------------------------------------------------------------
    void OBrowserLine::SetComponentHelpIds( const rtl::OString& _rHelpId, const rtl::OString& _sPrimaryButtonId, const rtl::OString& _sSecondaryButtonId )
    {
        if ( m_pControlWindow )
            m_pControlWindow->SetHelpId( _rHelpId );

        if ( m_pBrowseButton )
        {
            m_pBrowseButton->SetHelpId( _rHelpId );
            m_pBrowseButton->SetUniqueId( _sPrimaryButtonId );

            if ( m_pAdditionalBrowseButton )
            {
                m_pAdditionalBrowseButton->SetHelpId( _rHelpId );
                m_pAdditionalBrowseButton->SetUniqueId( _sSecondaryButtonId );
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

            // for Issue 69452
            if (Application::GetSettings().GetLayoutRTL())
            {
                sal_Unicode cRTL_mark = 0x200F;
                aText.Append(cRTL_mark);
            }

            m_aFtTitle.SetText(aText);
        }
    }

    //------------------------------------------------------------------
    XubString OBrowserLine::GetTitle() const
    {
        String sDisplayName = m_aFtTitle.GetText();

    // for Issue 69452
    if (Application::GetSettings().GetLayoutRTL())
    {
        sal_Unicode cRTL_mark = 0x200F;
        sDisplayName.EraseTrailingChars(cRTL_mark);
    }

        sDisplayName.EraseTrailingChars( '.' );

        return sDisplayName;
    }

    //------------------------------------------------------------------
    void OBrowserLine::SetReadOnly( bool _bReadOnly )
    {
        if ( m_bReadOnly != _bReadOnly )
        {
            m_bReadOnly = _bReadOnly;
            implUpdateEnabledDisabled();
        }
    }

    //------------------------------------------------------------------
    namespace
    {
        void implSetBitIfAffected( sal_uInt16& _nEnabledBits, sal_Int16 _nAffectedMask, sal_Int16 _nTestBit, bool _bSet )
        {
            if ( _nAffectedMask & _nTestBit )
            {
                if ( _bSet )
                    _nEnabledBits |= _nTestBit;
                else
                    _nEnabledBits &= ~_nTestBit;
            }
        }

        void implEnable( Window* _pWindow, sal_uInt16 _nEnabledBits, sal_uInt16 _nMatchBits  )
        {
            if ( _pWindow )
                _pWindow->Enable( ( _nEnabledBits & _nMatchBits ) == _nMatchBits );
        }

        void implEnable( Window* _pWindow, bool _bEnable )
        {
            if ( _pWindow )
                _pWindow->Enable( _bEnable );
        }
    }

    //------------------------------------------------------------------
    void OBrowserLine::implUpdateEnabledDisabled()
    {
        implEnable( &m_aFtTitle,                m_nEnableFlags, PropertyLineElement::CompleteLine );
        if ( m_pControlWindow )
            implEnable( m_pControlWindow,       m_nEnableFlags, PropertyLineElement::CompleteLine | PropertyLineElement::InputControl );

        if ( m_bReadOnly )
        {
            implEnable( m_pBrowseButton,            false );
            implEnable( m_pAdditionalBrowseButton,  false );
        }
        else
        {
            implEnable( m_pBrowseButton,            m_nEnableFlags, PropertyLineElement::CompleteLine | PropertyLineElement::PrimaryButton );
            implEnable( m_pAdditionalBrowseButton,  m_nEnableFlags, PropertyLineElement::CompleteLine | PropertyLineElement::SecondaryButton );
        }
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
    PushButton& OBrowserLine::impl_ensureButton( bool _bPrimary )
    {
        PushButton*& rpButton = _bPrimary ? m_pBrowseButton : m_pAdditionalBrowseButton;

        if ( !rpButton )
        {
            rpButton = new PushButton( m_pTheParent, WB_NOPOINTERFOCUS );
            rpButton->SetGetFocusHdl( LINK( this, OBrowserLine, OnButtonFocus ) );
            rpButton->SetClickHdl( LINK( this, OBrowserLine, OnButtonClicked ) );
            rpButton->SetText( String::CreateFromAscii( "..." ) );
        }

        rpButton->Show();

        impl_layoutComponents();

        return *rpButton;
    }

    //------------------------------------------------------------------
    void OBrowserLine::impl_getImagesFromURL_nothrow( const ::rtl::OUString& _rImageURL, Image& _out_rImage )
    {
        try
        {
            ::comphelper::ComponentContext aContext( ::comphelper::getProcessServiceFactory() );
            Reference< XGraphicProvider > xGraphicProvider( aContext.createComponent( "com.sun.star.graphic.GraphicProvider" ), UNO_QUERY_THROW );

            Sequence< PropertyValue > aMediaProperties(1);
            aMediaProperties[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "URL" ) );
            aMediaProperties[0].Value <<= _rImageURL;

            Reference< XGraphic > xGraphic( xGraphicProvider->queryGraphic( aMediaProperties ), UNO_QUERY_THROW );
            _out_rImage = Image( xGraphic );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //------------------------------------------------------------------
    void OBrowserLine::ShowBrowseButton( const ::rtl::OUString& _rImageURL, sal_Bool _bPrimary )
    {
        PushButton& rButton( impl_ensureButton( _bPrimary ) );

        OSL_PRECOND( _rImageURL.getLength(), "OBrowserLine::ShowBrowseButton: use the other version if you don't have an image!" );
        Image aImage;
        impl_getImagesFromURL_nothrow( _rImageURL, aImage );

        rButton.SetModeImage( aImage );
   }

    //------------------------------------------------------------------
    void OBrowserLine::ShowBrowseButton( const Image& _rImage, sal_Bool _bPrimary )
    {
        PushButton& rButton( impl_ensureButton( _bPrimary ) );
        if ( !!_rImage )
            rButton.SetModeImage( _rImage );
    }

    //------------------------------------------------------------------
    void OBrowserLine::ShowBrowseButton( sal_Bool _bPrimary )
    {
        impl_ensureButton( _bPrimary );
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
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
        return 0;
    }
//............................................................................
} // namespace pcr
//............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
