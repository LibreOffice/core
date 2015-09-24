/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "browserline.hxx"

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/inspection/PropertyLineElement.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>


namespace pcr
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::inspection::XPropertyControl;
    using ::com::sun::star::inspection::XPropertyControlContext;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::graphic::GraphicProvider;
    using ::com::sun::star::graphic::XGraphicProvider;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::graphic::XGraphic;

    namespace PropertyLineElement = ::com::sun::star::inspection::PropertyLineElement;

    OBrowserLine::OBrowserLine( const OUString& _rEntryName, vcl::Window* pParent )
            :m_sEntryName( _rEntryName )
            ,m_aFtTitle(VclPtr<FixedText>::Create(pParent))
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
        m_aFtTitle->Show();
    }

    OBrowserLine::~OBrowserLine()
    {
        implHideBrowseButton( true, false );
        implHideBrowseButton( false, false );
        m_aFtTitle.disposeAndClear();
    }


    void OBrowserLine::IndentTitle( bool _bIndent )
    {
        if ( m_bIndentTitle != _bIndent )
        {
            m_bIndentTitle = _bIndent;
            impl_layoutComponents();
        }
    }


    void OBrowserLine::SetComponentHelpIds( const OString& _rHelpId, const OString& _sPrimaryButtonId, const OString& _sSecondaryButtonId )
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


    void OBrowserLine::setControl( const Reference< XPropertyControl >& _rxControl )
    {
        m_xControl = _rxControl;
        m_pControlWindow = m_xControl.is() ? VCLUnoHelper::GetWindow( _rxControl->getControlWindow() ) : VclPtr<vcl::Window>();
        DBG_ASSERT( m_pControlWindow, "OBrowserLine::setControl: setting NULL controls/windows is not allowed!" );

        if ( m_pControlWindow )
        {
            m_pControlWindow->SetParent( m_pTheParent );
            m_pControlWindow->Show();
        }
        impl_layoutComponents();
    }


    vcl::Window* OBrowserLine::GetRefWindow()
    {
        vcl::Window* pRefWindow = m_aFtTitle.get();

        if(m_pBrowseButton)
        {
            pRefWindow=static_cast<vcl::Window*>(m_pBrowseButton);
        }
        else if ( m_pControlWindow )
        {
            pRefWindow = m_pControlWindow;
        }
        return pRefWindow;
    }


    void OBrowserLine::SetTabOrder(vcl::Window* pRefWindow, ZOrderFlags nFlags )
    {
        m_aFtTitle->SetZOrder(pRefWindow,nFlags);
        if ( m_pControlWindow )
            m_pControlWindow->SetZOrder( m_aFtTitle.get(), ZOrderFlags::Behind );

        if ( m_pBrowseButton && m_pControlWindow )
            m_pBrowseButton->SetZOrder( m_pControlWindow, ZOrderFlags::Behind );

        if ( m_pAdditionalBrowseButton && m_pBrowseButton )
            m_pAdditionalBrowseButton->SetZOrder( m_pBrowseButton, ZOrderFlags::Behind );
    }


    bool OBrowserLine::GrabFocus()
    {
        bool bRes=false;

        if ( m_pControlWindow && m_pControlWindow->IsEnabled() )
        {
            m_pControlWindow->GrabFocus();
            bRes = true;
        }
        else if ( m_pAdditionalBrowseButton && m_pAdditionalBrowseButton->IsEnabled() )
        {
            m_pAdditionalBrowseButton->GrabFocus();
            bRes = true;
        }
        else if ( m_pBrowseButton && m_pBrowseButton->IsEnabled() )
        {
            m_pBrowseButton->GrabFocus();
            bRes = true;
        }
        return bRes;
    }


    void OBrowserLine::SetPosSizePixel( Point _rPos, Size _rSize )
    {
        m_aLinePos = _rPos;
        m_aOutputSize = _rSize;

        impl_layoutComponents();
    }


    void OBrowserLine::Show(bool bFlag)
    {
        m_aFtTitle->Show(bFlag);
        if ( m_pControlWindow )
            m_pControlWindow->Show( bFlag );
        if ( m_pBrowseButton )
            m_pBrowseButton->Show( bFlag );
        if ( m_pAdditionalBrowseButton )
            m_pAdditionalBrowseButton->Show( bFlag );
    }


    void OBrowserLine::Hide()
    {
        Show(false);
    }


    bool OBrowserLine::IsVisible()
    {
        return m_aFtTitle->IsVisible();
    }


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
            m_aFtTitle->SetPosSizePixel( aTitlePos, aTitleSize );
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


    void OBrowserLine::SetTitle(const OUString& _rNewTtile )
    {
        if ( GetTitle() == _rNewTtile )
            return;
        // #99102# --------------
        m_aFtTitle->SetText( _rNewTtile );
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
            OUStringBuffer aText( m_aFtTitle->GetText() );

            while( m_pTheParent->GetTextWidth( aText.toString() ) < m_nNameWidth )
                        aText.append("...........");

            // for Issue 69452
            if (AllSettings::GetLayoutRTL())
            {
                sal_Unicode cRTL_mark = 0x200F;
                aText.append( OUString(cRTL_mark) );
            }

            m_aFtTitle->SetText( aText.makeStringAndClear() );
        }
    }


    OUString OBrowserLine::GetTitle() const
    {
        OUString sDisplayName = m_aFtTitle->GetText();

        // for Issue 69452
        if (AllSettings::GetLayoutRTL())
        {
            sal_Unicode cRTL_mark = 0x200F;
            sDisplayName = comphelper::string::stripEnd(sDisplayName, cRTL_mark);
        }

        sDisplayName = comphelper::string::stripEnd(sDisplayName, '.');

        return sDisplayName;
    }


    void OBrowserLine::SetReadOnly( bool _bReadOnly )
    {
        if ( m_bReadOnly != _bReadOnly )
        {
            m_bReadOnly = _bReadOnly;
            implUpdateEnabledDisabled();
        }
    }


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

        void implEnable( vcl::Window* _pWindow, sal_uInt16 _nEnabledBits, sal_uInt16 _nMatchBits  )
        {
            if ( _pWindow )
                _pWindow->Enable( ( _nEnabledBits & _nMatchBits ) == _nMatchBits );
        }

        void implEnable( vcl::Window* _pWindow, bool _bEnable )
        {
            if ( _pWindow )
                _pWindow->Enable( _bEnable );
        }
    }


    void OBrowserLine::implUpdateEnabledDisabled()
    {
        implEnable( m_aFtTitle.get(),           m_nEnableFlags, PropertyLineElement::CompleteLine );
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


    void OBrowserLine::EnablePropertyLine( bool _bEnable )
    {
        implSetBitIfAffected( m_nEnableFlags, PropertyLineElement::CompleteLine, PropertyLineElement::CompleteLine, _bEnable );
        implUpdateEnabledDisabled();
    }


    void OBrowserLine::EnablePropertyControls( sal_Int16 _nControls, bool _bEnable )
    {
        implSetBitIfAffected( m_nEnableFlags, _nControls, PropertyLineElement::InputControl, _bEnable );
        implSetBitIfAffected( m_nEnableFlags, _nControls, PropertyLineElement::PrimaryButton, _bEnable );
        implSetBitIfAffected( m_nEnableFlags, _nControls, PropertyLineElement::SecondaryButton, _bEnable );
        implUpdateEnabledDisabled();
    }


    PushButton& OBrowserLine::impl_ensureButton( bool _bPrimary )
    {
        VclPtr<PushButton>& rpButton = _bPrimary ? m_pBrowseButton : m_pAdditionalBrowseButton;

        if ( !rpButton )
        {
            rpButton = VclPtr<PushButton>::Create( m_pTheParent, WB_NOPOINTERFOCUS );
            rpButton->SetGetFocusHdl( LINK( this, OBrowserLine, OnButtonFocus ) );
            rpButton->SetClickHdl( LINK( this, OBrowserLine, OnButtonClicked ) );
            rpButton->SetText(OUString("..."));
        }

        rpButton->Show();

        impl_layoutComponents();

        return *rpButton;
    }


    void OBrowserLine::impl_getImagesFromURL_nothrow( const OUString& _rImageURL, Image& _out_rImage )
    {
        try
        {
            Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
            Reference< XGraphicProvider > xGraphicProvider( GraphicProvider::create(xContext) );

            Sequence< PropertyValue > aMediaProperties(1);
            aMediaProperties[0].Name = "URL";
            aMediaProperties[0].Value <<= _rImageURL;

            Reference< XGraphic > xGraphic( xGraphicProvider->queryGraphic( aMediaProperties ), UNO_QUERY_THROW );
            _out_rImage = Image( xGraphic );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }


    void OBrowserLine::ShowBrowseButton( const OUString& _rImageURL, bool _bPrimary )
    {
        PushButton& rButton( impl_ensureButton( _bPrimary ) );

        OSL_PRECOND( !_rImageURL.isEmpty(), "OBrowserLine::ShowBrowseButton: use the other version if you don't have an image!" );
        Image aImage;
        impl_getImagesFromURL_nothrow( _rImageURL, aImage );

        rButton.SetModeImage( aImage );
   }


    void OBrowserLine::ShowBrowseButton( const Image& _rImage, bool _bPrimary )
    {
        PushButton& rButton( impl_ensureButton( _bPrimary ) );
        if ( !!_rImage )
            rButton.SetModeImage( _rImage );
    }


    void OBrowserLine::ShowBrowseButton( bool _bPrimary )
    {
        impl_ensureButton( _bPrimary );
    }


    void OBrowserLine::implHideBrowseButton( bool _bPrimary, bool _bReLayout )
    {
        VclPtr<PushButton>& rpButton = _bPrimary ? m_pBrowseButton : m_pAdditionalBrowseButton;

        if ( rpButton )
        {
            rpButton->Hide();
            rpButton.disposeAndClear();
        }

        if ( _bReLayout )
            impl_layoutComponents();
    }


    void OBrowserLine::HideBrowseButton( bool _bPrimary )
    {
        implHideBrowseButton( _bPrimary, true );
    }


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


    void OBrowserLine::SetClickListener( IButtonClickListener* _pListener )
    {
        m_pClickListener = _pListener;
    }


    IMPL_LINK_TYPED( OBrowserLine, OnButtonClicked, Button*, _pButton, void )
    {
        if ( m_pClickListener )
            m_pClickListener->buttonClicked( this, _pButton == m_pBrowseButton );
    }


    IMPL_LINK_NOARG_TYPED( OBrowserLine, OnButtonFocus, Control&, void )
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
    }

} // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
