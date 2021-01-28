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
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <vcl/weldutils.hxx>

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

    OBrowserLine::OBrowserLine(const OUString& rEntryName, weld::Container* pParent, weld::SizeGroup* pLabelGroup,
                               weld::Container* pInitialControlParent)
        : m_sEntryName(rEntryName)
        , m_xBuilder(Application::CreateBuilder(pParent, "modules/spropctrlr/ui/browserline.ui"))
        , m_xContainer(m_xBuilder->weld_container("BrowserLine"))
        , m_xFtTitle(m_xBuilder->weld_label("label"))
        , m_xBrowseButton(m_xBuilder->weld_button("browse"))
        , m_xAdditionalBrowseButton(m_xBuilder->weld_button("morebrowse"))
        , m_pInitialControlParent(pInitialControlParent) // controls start with this as their parent and need to be moved into m_xContainer
        , m_pParent(pParent)
        , m_pControlWindow( nullptr )
        , m_pBrowseButton(nullptr)
        , m_pAdditionalBrowseButton( nullptr )
        , m_pClickListener( nullptr )
        , m_nNameWidth(0)
        , m_nEnableFlags( 0xFFFF )
        , m_bIndentTitle( false )
        , m_bReadOnly( false )
    {
        pLabelGroup->add_widget(m_xFtTitle.get());
    }

    OBrowserLine::~OBrowserLine()
    {
        implHideBrowseButton(true);
        implHideBrowseButton(false);
        m_pParent->move(m_xContainer.get(), nullptr);
    }

    void OBrowserLine::IndentTitle( bool _bIndent )
    {
        if ( m_bIndentTitle != _bIndent )
        {
            m_bIndentTitle = _bIndent;
        }
    }

    void OBrowserLine::SetComponentHelpIds(const OString& rHelpId)
    {
        if (m_pControlWindow)
            m_pControlWindow->set_help_id(rHelpId);

        if ( m_pBrowseButton )
        {
            m_pBrowseButton->set_help_id(rHelpId);

            if ( m_pAdditionalBrowseButton )
            {
                m_pAdditionalBrowseButton->set_help_id(rHelpId);
            }
        }
    }

    void OBrowserLine::setControl( const Reference< XPropertyControl >& rxControl )
    {
        m_xControl = rxControl;
        auto xWindow = m_xControl->getControlWindow();
        if (weld::TransportAsXWindow* pTunnel = dynamic_cast<weld::TransportAsXWindow*>(xWindow.get()))
            m_pControlWindow = pTunnel->getWidget();
        else
            m_pControlWindow = nullptr;
        DBG_ASSERT( m_pControlWindow, "OBrowserLine::setControl: setting NULL controls/windows is not allowed!" );

        if ( m_pControlWindow )
        {
            m_pInitialControlParent->move(m_pControlWindow, m_xContainer.get());
            m_pControlWindow->set_grid_left_attach(1);
            m_xFtTitle->set_mnemonic_widget(m_pControlWindow);
            m_pControlWindow->show();
        }
    }

    bool OBrowserLine::GrabFocus()
    {
        bool bRes=false;

        if (m_pControlWindow && m_pControlWindow->get_sensitive())
        {
            m_pControlWindow->grab_focus();
            bRes = true;
        }
        else if ( m_pAdditionalBrowseButton && m_pAdditionalBrowseButton->get_sensitive() )
        {
            m_pAdditionalBrowseButton->grab_focus();
            bRes = true;
        }
        else if ( m_pBrowseButton && m_pBrowseButton->get_sensitive() )
        {
            m_pBrowseButton->grab_focus();
            bRes = true;
        }
        return bRes;
    }

    void OBrowserLine::Show(bool bFlag)
    {
        m_xFtTitle->set_visible(bFlag);
        if (m_pControlWindow)
            m_pControlWindow->set_visible( bFlag );
        if ( m_pBrowseButton )
            m_pBrowseButton->set_visible( bFlag );
        if ( m_pAdditionalBrowseButton )
            m_pAdditionalBrowseButton->set_visible( bFlag );
    }

    void OBrowserLine::Hide()
    {
        Show(false);
    }

    void OBrowserLine::SetTitle(const OUString& rNewTitle )
    {
        if ( GetTitle() == rNewTitle )
            return;
        m_xFtTitle->set_label( rNewTitle );
        if (m_pControlWindow)
            m_pControlWindow->set_accessible_name(rNewTitle);
        if ( m_pBrowseButton )
            m_pBrowseButton->set_accessible_name( rNewTitle );
        FullFillTitleString();
    }

    void OBrowserLine::FullFillTitleString()
    {
        OUStringBuffer aText(m_xFtTitle->get_label());

        int n10DotsWidth = m_xFtTitle->get_pixel_size("..........").Width();
        int nTextWidth = m_xFtTitle->get_pixel_size(aText.toString()).Width();
        int nDiff = m_nNameWidth - nTextWidth;
        int nExtraChars = (nDiff * 10) / n10DotsWidth;
        for (int i = 0; i < nExtraChars; ++i)
            aText.append(".");

        // for Issue 69452
        if (AllSettings::GetLayoutRTL())
        {
            sal_Unicode const cRTL_mark = 0x200F;
            aText.append( cRTL_mark );
        }

        m_xFtTitle->set_label(aText.makeStringAndClear());
    }

    OUString OBrowserLine::GetTitle() const
    {
        OUString sDisplayName = m_xFtTitle->get_label();

        // for Issue 69452
        if (AllSettings::GetLayoutRTL())
        {
            sal_Unicode const cRTL_mark = 0x200F;
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
        void implSetBitIfAffected(sal_uInt16& nEnabledBits, sal_Int16 _nAffectedMask, sal_Int16 _nTestBit, bool _bSet)
        {
            if ( _nAffectedMask & _nTestBit )
            {
                if ( _bSet )
                    nEnabledBits |= _nTestBit;
                else
                    nEnabledBits &= ~_nTestBit;
            }
        }

        void implEnable(weld::Widget* pWindow, bool bEnable)
        {
            // tdf#138131 get_sensitive comparison as bodge for
            // vcl's recursive Enable behavior
            if (pWindow && pWindow->get_sensitive() != bEnable)
                pWindow->set_sensitive(bEnable);
        }

        void implEnable(weld::Widget* pWindow, sal_uInt16 nEnabledBits, sal_uInt16 nMatchBits)
        {
            bool bEnable = ((nEnabledBits & nMatchBits) == nMatchBits);
            implEnable(pWindow, bEnable);
        }
    }

    void OBrowserLine::implUpdateEnabledDisabled()
    {
        implEnable( m_xFtTitle.get(),           m_nEnableFlags, PropertyLineElement::CompleteLine );
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

    weld::Button& OBrowserLine::impl_ensureButton(bool bPrimary)
    {
        weld::Button* pButton;
        if (bPrimary)
            pButton = m_pBrowseButton;
        else
            pButton = m_pAdditionalBrowseButton;

        if (!pButton )
        {
            if (bPrimary)
                pButton = m_pBrowseButton = m_xBrowseButton.get();
            else
                pButton = m_pAdditionalBrowseButton = m_xAdditionalBrowseButton.get();
            pButton->connect_focus_in(LINK(this, OBrowserLine, OnButtonFocus));
            pButton->connect_clicked(LINK(this, OBrowserLine, OnButtonClicked));
        }

        pButton->show();

        return *pButton;
    }

    void OBrowserLine::ShowBrowseButton( const OUString& rImageURL, bool bPrimary )
    {
        weld::Button& rButton( impl_ensureButton( bPrimary ) );

        OSL_PRECOND( !rImageURL.isEmpty(), "OBrowserLine::ShowBrowseButton: use the other version if you don't have an image!" );
        Reference<XGraphic> xGraphic;
        try
        {
            Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
            Reference< XGraphicProvider > xGraphicProvider( GraphicProvider::create(xContext) );

            Sequence< PropertyValue > aMediaProperties(1);
            aMediaProperties[0].Name = "URL";
            aMediaProperties[0].Value <<= rImageURL;

            xGraphic = Reference<XGraphic>(xGraphicProvider->queryGraphic(aMediaProperties), css::uno::UNO_SET_THROW);
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("extensions.propctrlr");
        }

        rButton.set_image(xGraphic);
    }

    void OBrowserLine::ShowBrowseButton(const css::uno::Reference<css::graphic::XGraphic>& rGraphic, bool bPrimary)
    {
        weld::Button& rButton( impl_ensureButton( bPrimary ) );
        rButton.set_image(rGraphic);
    }

    void OBrowserLine::ShowBrowseButton( bool bPrimary )
    {
        impl_ensureButton(bPrimary);
    }

    void OBrowserLine::implHideBrowseButton(bool bPrimary)
    {
        if (bPrimary)
        {
            if (m_pBrowseButton)
            {
                m_pBrowseButton->hide();
                m_pBrowseButton = nullptr;
            }
        }
        else
        {
            if (m_pAdditionalBrowseButton)
            {
                m_pAdditionalBrowseButton->hide();
                m_pAdditionalBrowseButton = nullptr;
            }
        }
    }

    void OBrowserLine::HideBrowseButton(bool bPrimary)
    {
        implHideBrowseButton(bPrimary);
    }

    void OBrowserLine::SetTitleWidth(sal_uInt16 nWidth)
    {
        int nMinDotsWidth = m_xFtTitle->get_pixel_size("...").Width();
        if (m_nNameWidth != nWidth + nMinDotsWidth)
            m_nNameWidth = nWidth + nMinDotsWidth;
        FullFillTitleString();
    }

    void OBrowserLine::SetClickListener( IButtonClickListener* _pListener )
    {
        m_pClickListener = _pListener;
    }

    IMPL_LINK(OBrowserLine, OnButtonClicked, weld::Button&, rButton, void)
    {
        if ( m_pClickListener )
            m_pClickListener->buttonClicked(this, &rButton == m_pBrowseButton);
    }

    IMPL_LINK_NOARG( OBrowserLine, OnButtonFocus, weld::Widget&, void )
    {
        if ( m_xControl.is() )
        {
            try
            {
                Reference< XPropertyControlContext > xContext( m_xControl->getControlContext(), css::uno::UNO_SET_THROW );
                xContext->focusGained( m_xControl );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("extensions.propctrlr");
            }
        }
    }

} // namespace pcr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
