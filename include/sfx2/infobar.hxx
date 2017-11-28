/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SFX2_INFOBAR_HXX
#define INCLUDED_SFX2_INFOBAR_HXX

#include <vector>

#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <basegfx/color/bcolor.hxx>

#include <sfx2/dllapi.h>
#include <sfx2/childwin.hxx>

enum class InfoBarType {
    Info,
    Success,
    Warning,
    Danger
};

/** SfxChildWindow for positioning the InfoBar in the view.
  */
class SFX2_DLLPUBLIC SfxInfoBarContainerChild : public SfxChildWindow
{
    private:
        SfxBindings* m_pBindings;

    public:
        SfxInfoBarContainerChild( vcl::Window* pParent,
                                  sal_uInt16 nId,
                                  SfxBindings* pBindings,
                                  SfxChildWinInfo* pInfo );
        virtual ~SfxInfoBarContainerChild() override;

        SFX_DECL_CHILDWINDOW_WITHID( SfxInfoBarContainerChild );

        void Update( );
};

/** Class representing a single InfoBar to be added in a SfxInfoBarContainerWindow.
  */
class SfxInfoBarWindow : public vcl::Window
{
    private:
        OUString                           m_sId;
        VclPtr<FixedImage>        m_pImage;
        VclPtr<FixedText>           m_pMessage;
        VclPtr<Button>                m_pCloseBtn;
        std::vector< VclPtr<PushButton> >  m_aActionBtns;

    public:
        SfxInfoBarWindow( vcl::Window* parent, const OUString& sId,
                          const OUString& sMessage,
                          InfoBarType infoBarType,
                          WinBits nMessageStyle);
        virtual ~SfxInfoBarWindow( ) override;
        virtual void dispose() override;

        const OUString& getId() const { return m_sId; }
        virtual void Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& ) override;
        virtual void Resize( ) override;
        basegfx::BColor                m_aBackgroundColor;
        basegfx::BColor                m_aForegroundColor;

        /** Add button to Infobar.
         * Infobar takes ownership of the button so the button is
         * destroyed when the infobar gets destroyed.
         */
        void addButton(PushButton* pButton);

    private:
        DECL_LINK( CloseHandler, Button*, void );
};

class SfxInfoBarContainerWindow : public vcl::Window
{
    private:
        SfxInfoBarContainerChild*               m_pChildWin;
        std::vector< VclPtr<SfxInfoBarWindow> > m_pInfoBars;

    public:
        SfxInfoBarContainerWindow(SfxInfoBarContainerChild* pChildWin);
        virtual ~SfxInfoBarContainerWindow( ) override;
        virtual void dispose() override;

        VclPtr<SfxInfoBarWindow> appendInfoBar(const OUString& sId,
                                        const OUString& sMessage,
                                        InfoBarType ibType,
                                        WinBits nMessageStyle);
        VclPtr<SfxInfoBarWindow> getInfoBar(const OUString& sId);
        bool hasInfoBarWithID(const OUString& sId);
        void removeInfoBar(VclPtr<SfxInfoBarWindow> const & pInfoBar);

        virtual void Resize() override;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
