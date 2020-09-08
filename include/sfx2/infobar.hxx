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

#include <basegfx/color/bcolor.hxx>

#include <sfx2/childwin.hxx>
#include <sfx2/dllapi.h>
#include <vcl/InterimItemWindow.hxx>

// These must match the values in offapi/com/sun/star/frame/InfobarType.idl
enum class InfobarType
{
    INFO = 0,
    SUCCESS = 1,
    WARNING = 2,
    DANGER = 3
};

class InfobarData
{
public:
    OUString msId;
    OUString msPrimaryMessage;
    OUString msSecondaryMessage;
    InfobarType maInfobarType;
    bool mbShowCloseButton;
};

/** SfxChildWindow for positioning the InfoBar in the view.
  */
class SFX2_DLLPUBLIC SfxInfoBarContainerChild final : public SfxChildWindow
{
private:
    SfxBindings* m_pBindings;

public:
    SfxInfoBarContainerChild(vcl::Window* pParent, sal_uInt16 nId, SfxBindings* pBindings,
                             SfxChildWinInfo* pInfo);
    virtual ~SfxInfoBarContainerChild() override;

    SFX_DECL_CHILDWINDOW_WITHID(SfxInfoBarContainerChild);

    void Update();
};

class ExtraButton;

/** Class representing a single InfoBar to be added in a SfxInfoBarContainerWindow.
  */
class SFX2_DLLPUBLIC SfxInfoBarWindow final : public InterimItemWindow
{
private:
    OUString m_sId;
    InfobarType m_eType;
    Size m_aSize;
    Size m_aMessageSize;
    Size m_aOrigMessageSize;
    bool m_bLayingOut;
    std::unique_ptr<weld::Image> m_xImage;
    std::unique_ptr<weld::Label> m_xPrimaryMessage;
    std::unique_ptr<weld::TextView> m_xSecondaryMessage;
    std::unique_ptr<weld::Container> m_xButtonBox;
    std::unique_ptr<weld::Toolbar> m_xCloseBtn;
    std::vector<std::unique_ptr<ExtraButton>> m_aActionBtns;

    DECL_LINK(SizeAllocHdl, const Size&, void);

    void SetForeAndBackgroundColors(InfobarType eType);
    void SetCloseButtonImage();

public:
    SfxInfoBarWindow(vcl::Window* parent, const OUString& sId, const OUString& sPrimaryMessage,
                     const OUString& sSecondaryMessage, InfobarType InfobarType,
                     bool bShowCloseButton);
    Size DoLayout();
    virtual void Layout() override;
    virtual ~SfxInfoBarWindow() override;
    virtual void dispose() override;

    const OUString& getId() const { return m_sId; }
    void Update(const OUString& sPrimaryMessage, const OUString& sSecondaryMessage,
                InfobarType eType);
    basegfx::BColor m_aBackgroundColor;
    basegfx::BColor m_aForegroundColor;

    /** Add button to Infobar.
      * Infobar takes ownership of the button so the button is
      * destroyed when the infobar gets destroyed.
      *
      * The optional "pCommand" is used by extensions, via XInfobarProvider, to
      * dispatch pCommand on click.
      */
    weld::Button& addButton(const OUString* pCommand = nullptr);

    void SetCommandHandler(weld::Button& rBtn, const OUString& aCommand);

private:
    DECL_LINK(CloseHandler, const OString&, void);
};

class SfxInfoBarContainerWindow final : public vcl::Window
{
private:
    SfxInfoBarContainerChild* m_pChildWin;
    std::vector<VclPtr<SfxInfoBarWindow>> m_pInfoBars;
    Idle m_aLayoutIdle;
    bool m_bResizing;

    DECL_LINK(DoUpdateLayout, Timer*, void);

public:
    SfxInfoBarContainerWindow(SfxInfoBarContainerChild* pChildWin);
    virtual ~SfxInfoBarContainerWindow() override;
    virtual void dispose() override;

    VclPtr<SfxInfoBarWindow> appendInfoBar(const OUString& sId, const OUString& sPrimaryMessage,
                                           const OUString& sSecondaryMessage, InfobarType ibType,
                                           bool bShowCloseButton);
    VclPtr<SfxInfoBarWindow> getInfoBar(const OUString& sId);
    bool hasInfoBarWithID(const OUString& sId);
    void removeInfoBar(VclPtr<SfxInfoBarWindow> const& pInfoBar);
    static bool isInfobarEnabled(const OUString& sId);

    void TriggerUpdateLayout();

    virtual void Resize() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
