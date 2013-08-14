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

#ifndef _EXTENSIONS_PROPCTRLR_BROWSERLINE_HXX_
#define _EXTENSIONS_PROPCTRLR_BROWSERLINE_HXX_

#include <com/sun/star/inspection/XPropertyControl.hpp>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>

namespace com { namespace sun { namespace star { namespace inspection { namespace PropertyLineElement
{
    const sal_Int16 CompleteLine = 0x4000;
} } } } }

//............................................................................
namespace pcr
{
//............................................................................

    class OBrowserLine;

    //========================================================================
    class IButtonClickListener
    {
    public:
        virtual void    buttonClicked( OBrowserLine* _pLine, sal_Bool _bPrimary ) = 0;

    protected:
        ~IButtonClickListener() {}
    };

    //========================================================================
    class OBrowserLine
    {
    private:
        OUString         m_sEntryName;
        FixedText               m_aFtTitle;
        Size                    m_aOutputSize;
        Point                   m_aLinePos;
        ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >
                                m_xControl;
        Window*                 m_pControlWindow;
        PushButton*             m_pBrowseButton;
        PushButton*             m_pAdditionalBrowseButton;
        IButtonClickListener*   m_pClickListener;
        Window*                 m_pTheParent;
        sal_uInt16              m_nNameWidth;
        sal_uInt16              m_nEnableFlags;
        bool                    m_bIndentTitle;
        bool                    m_bReadOnly;

    public:
                            OBrowserLine( const OUString& _rEntryName, Window* pParent);
                            ~OBrowserLine();

        void setControl( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >& _rxControl );
        const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >& getControl() const
        {
            return m_xControl;
        }
        inline Window* getControlWindow() const
        {
            return m_pControlWindow;
        }

        const OUString&
                            GetEntryName() const { return m_sEntryName; }

        void                SetComponentHelpIds( const OString& _rHelpId, const OString& _sPrimaryButtonId, const OString& _sSecondaryButtonId );

        void                SetTitle(const OUString& rString );
        void                FullFillTitleString();
        OUString            GetTitle() const;
        void                SetTitleWidth(sal_uInt16);

        void                SetPosSizePixel(Point aPos,Size aSize);
        void                Show(sal_Bool bFlag=sal_True);
        void                Hide();
        sal_Bool            IsVisible();

        Window*             GetRefWindow();
        void                SetTabOrder(Window* pRefWindow, sal_uInt16 nFlags );

        sal_Bool            GrabFocus();
        void                ShowBrowseButton( const OUString& _rImageURL, sal_Bool _bPrimary );
        void                ShowBrowseButton( const Image& _rImage, sal_Bool _bPrimary );
        void                ShowBrowseButton( sal_Bool _bPrimary );
        void                HideBrowseButton( sal_Bool _bPrimary );

        void                EnablePropertyControls( sal_Int16 _nControls, bool _bEnable );
        void                EnablePropertyLine( bool _bEnable );

        void                SetReadOnly( bool _bReadOnly );

        void                SetClickListener( IButtonClickListener* _pListener );

        void                IndentTitle( bool _bIndent );

    private:
        DECL_LINK( OnButtonClicked, PushButton* );
        DECL_LINK( OnButtonFocus, PushButton* );

        void    implHideBrowseButton( sal_Bool _bPrimary, bool _bReLayout );
        void    implUpdateEnabledDisabled();

        void    impl_layoutComponents();

        PushButton& impl_ensureButton( bool _bPrimary );
        void        impl_getImagesFromURL_nothrow( const OUString& _rImageURL, Image& _out_rImage );
    };

//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_BROWSERLINE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
