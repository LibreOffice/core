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

#pragma once

#include <com/sun/star/inspection/XPropertyControl.hpp>
#include <vcl/weld.hxx>

namespace com::sun::star::inspection::PropertyLineElement
{
    const sal_Int16 CompleteLine = 0x4000;
}


namespace pcr
{


    class OBrowserLine;


    class IButtonClickListener
    {
    public:
        virtual void    buttonClicked( OBrowserLine* pLine, bool bPrimary ) = 0;

    protected:
        ~IButtonClickListener() {}
    };


    class OBrowserLine
    {
    private:
        OUString                m_sEntryName;
        std::unique_ptr<weld::Builder> m_xBuilder;
        std::unique_ptr<weld::Container> m_xContainer;
        std::unique_ptr<weld::Label> m_xFtTitle;
        std::unique_ptr<weld::Button> m_xBrowseButton;
        std::unique_ptr<weld::Button> m_xAdditionalBrowseButton;
        css::uno::Reference< css::inspection::XPropertyControl >
                                m_xControl;
        weld::Container*        m_pInitialControlParent;
        weld::Container*        m_pParent;
        weld::Widget*           m_pControlWindow;
        weld::Button*           m_pBrowseButton;
        weld::Button*           m_pAdditionalBrowseButton;
        IButtonClickListener*   m_pClickListener;
        sal_uInt16              m_nNameWidth;
        sal_uInt16              m_nEnableFlags;
        bool                    m_bIndentTitle;
        bool                    m_bReadOnly;

    public:
        OBrowserLine(const OUString& rEntryName, weld::Container* pParent, weld::SizeGroup* pLabelGroup,
                     weld::Container* pInitialControlParent);
        ~OBrowserLine();

        void setControl( const css::uno::Reference< css::inspection::XPropertyControl >& rxControl );
        const css::uno::Reference< css::inspection::XPropertyControl >& getControl() const
        {
            return m_xControl;
        }
        weld::Widget* getControlWindow() const
        {
            return m_pControlWindow;
        }

        const OUString&     GetEntryName() const { return m_sEntryName; }

        void                SetComponentHelpIds(const OString& rHelpId);

        void                SetTitle(const OUString& rString );
        void                FullFillTitleString();
        OUString            GetTitle() const;
        void                SetTitleWidth(sal_uInt16);

        int                 GetRowHeight() const { return m_xContainer->get_preferred_size().Height(); }
        void                Show(bool bFlag=true);
        void                Hide();

        bool                GrabFocus();
        void                ShowBrowseButton( const OUString& rImageURL, bool bPrimary );
        void                ShowBrowseButton( const css::uno::Reference<css::graphic::XGraphic>& rGraphic, bool bPrimary );
        void                ShowBrowseButton( bool bPrimary );
        void                HideBrowseButton( bool bPrimary );

        void                EnablePropertyControls( sal_Int16 nControls, bool bEnable );
        void                EnablePropertyLine( bool bEnable );

        void                SetReadOnly( bool bReadOnly );

        void                SetClickListener( IButtonClickListener* pListener );

        void                IndentTitle( bool bIndent );

    private:
        DECL_LINK( OnButtonClicked, weld::Button&, void );
        DECL_LINK( OnButtonFocus, weld::Widget&, void );

        void    implHideBrowseButton(bool bPrimary);
        void    implUpdateEnabledDisabled();

        weld::Button& impl_ensureButton(bool bPrimary);
    };


} // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
