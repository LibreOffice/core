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

#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_BROWSERLINE_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_BROWSERLINE_HXX

#include <com/sun/star/inspection/XPropertyControl.hpp>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>

namespace com { namespace sun { namespace star { namespace inspection { namespace PropertyLineElement
{
    const sal_Int16 CompleteLine = 0x4000;
} } } } }


namespace pcr
{


    class OBrowserLine;


    class IButtonClickListener
    {
    public:
        virtual void    buttonClicked( OBrowserLine* _pLine, bool _bPrimary ) = 0;

    protected:
        ~IButtonClickListener() {}
    };


    class OBrowserLine
    {
    private:
        OUString                m_sEntryName;
        VclPtr<FixedText>       m_aFtTitle;
        Size                    m_aOutputSize;
        Point                   m_aLinePos;
        css::uno::Reference< css::inspection::XPropertyControl >
                                m_xControl;
        VclPtr<vcl::Window>     m_pControlWindow;
        VclPtr<PushButton>      m_pBrowseButton;
        VclPtr<PushButton>      m_pAdditionalBrowseButton;
        IButtonClickListener*   m_pClickListener;
        VclPtr<vcl::Window>     m_pTheParent;
        sal_uInt16              m_nNameWidth;
        sal_uInt16              m_nEnableFlags;
        bool                    m_bIndentTitle;
        bool                    m_bReadOnly;

    public:
                            OBrowserLine( const OUString& _rEntryName, vcl::Window* pParent);
                            ~OBrowserLine();

        void setControl( const css::uno::Reference< css::inspection::XPropertyControl >& _rxControl );
        const css::uno::Reference< css::inspection::XPropertyControl >& getControl() const
        {
            return m_xControl;
        }
        inline vcl::Window* getControlWindow() const
        {
            return m_pControlWindow;
        }

        const OUString&     GetEntryName() const { return m_sEntryName; }

        void                SetComponentHelpIds( const OString& _rHelpId, const OString& _sPrimaryButtonId, const OString& _sSecondaryButtonId );

        void                SetTitle(const OUString& rString );
        void                FullFillTitleString();
        OUString            GetTitle() const;
        void                SetTitleWidth(sal_uInt16);

        void                SetPosSizePixel(Point aPos,Size aSize);
        void                Show(bool bFlag=true);
        void                Hide();
        bool                IsVisible();

        vcl::Window*        GetRefWindow();
        void                SetTabOrder(vcl::Window* pRefWindow, ZOrderFlags nFlags );

        bool                GrabFocus();
        void                ShowBrowseButton( const OUString& _rImageURL, bool _bPrimary );
        void                ShowBrowseButton( const Image& _rImage, bool _bPrimary );
        void                ShowBrowseButton( bool _bPrimary );
        void                HideBrowseButton( bool _bPrimary );

        void                EnablePropertyControls( sal_Int16 _nControls, bool _bEnable );
        void                EnablePropertyLine( bool _bEnable );

        void                SetReadOnly( bool _bReadOnly );

        void                SetClickListener( IButtonClickListener* _pListener );

        void                IndentTitle( bool _bIndent );

    private:
        DECL_LINK_TYPED( OnButtonClicked, Button*, void );
        DECL_LINK_TYPED( OnButtonFocus, Control&, void );

        void    implHideBrowseButton( bool _bPrimary, bool _bReLayout );
        void    implUpdateEnabledDisabled();

        void    impl_layoutComponents();

        PushButton& impl_ensureButton( bool _bPrimary );
        static void impl_getImagesFromURL_nothrow( const OUString& _rImageURL, Image& _out_rImage );
    };


} // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_BROWSERLINE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
