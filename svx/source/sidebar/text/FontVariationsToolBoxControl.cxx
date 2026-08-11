/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <FontVariationsToolBoxControl.hxx>

#include <com/sun/star/awt/FontDescriptor.hpp>
#include <comphelper/propertyvalue.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <editeng/flstitem.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/tbxctrl.hxx>
#include <svtools/toolbarmenu.hxx>
#include <svx/FontVariationsPopup.hxx>
#include <svx/svxids.hrc>
#include <svx/tbcontrl.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/virdev.hxx>
#include <vcl/weld/Toolbar.hxx>
#include <vcl/weld/Window.hxx>

using namespace css;

namespace svx
{
namespace
{
/// The axes of a font, which the status only names.
std::vector<vcl::font::VariationAxis> getAxes(const OUString& rFontName)
{
    if (rFontName.isEmpty())
        return {};

    ScopedVclPtrInstance<VirtualDevice> pVDev(*Application::GetDefaultDevice(),
                                              DeviceFormat::WITH_ALPHA);
    pVDev->SetOutputSizePixel(Size(10, 10));

    vcl::Font aFont = pVDev->GetFont();
    aFont.SetFamilyName(rFontName);
    pVDev->SetFont(aFont);

    std::vector<vcl::font::VariationAxis> aAxes;
    pVDev->GetFontVariationAxes(aAxes);
    return aAxes;
}

class FontVariationsWindow final : public WeldToolbarPopup
{
private:
    rtl::Reference<FontVariationsToolBoxControl> m_xControl;
    std::unique_ptr<weld::ScrolledWindow> m_xContentWindow;
    std::unique_ptr<weld::Grid> m_xContentGrid;
    std::unique_ptr<weld::Button> m_xResetButton;
    std::unique_ptr<FontVariationsControl> m_xVariations;

    DECL_LINK(ChangedHdl, FontVariationsControl&, void);
    DECL_LINK(ActivateHdl, weld::Entry&, bool);

public:
    FontVariationsWindow(FontVariationsToolBoxControl* pControl, weld::Widget* pParent);

    virtual void GrabFocus() override;
};

FontVariationsWindow::FontVariationsWindow(FontVariationsToolBoxControl* pControl,
                                           weld::Widget* pParent)
    : WeldToolbarPopup(pControl->getFrameInterface(), pParent,
                       u"svx/ui/fontvariationscontrol.ui"_ustr, u"FontVariationsControl"_ustr)
    , m_xControl(pControl)
    , m_xContentWindow(m_xBuilder->weld_scrolled_window(u"contentWindow"_ustr))
    , m_xContentGrid(m_xBuilder->weld_grid(u"contentGrid"_ustr))
    , m_xResetButton(m_xBuilder->weld_button(u"reset"_ustr))
{
    m_xVariations.reset(new FontVariationsControl(*m_xContentWindow, *m_xContentGrid,
                                                  *m_xResetButton, pControl->GetAxes(),
                                                  pControl->GetVariations()));
    m_xVariations->connect_changed(LINK(this, FontVariationsWindow, ChangedHdl));
    m_xVariations->connect_activate(LINK(this, FontVariationsWindow, ActivateHdl));
}

void FontVariationsWindow::GrabFocus() { m_xResetButton->grab_focus(); }

IMPL_LINK_NOARG(FontVariationsWindow, ChangedHdl, FontVariationsControl&, void)
{
    m_xControl->Apply(m_xVariations->getVariations());
}

IMPL_LINK_NOARG(FontVariationsWindow, ActivateHdl, weld::Entry&, bool)
{
    m_xControl->EndPopupMode();
    return true;
}
}

FontVariationsToolBoxControl::FontVariationsToolBoxControl(
    const uno::Reference<uno::XComponentContext>& rContext)
    : PopupWindowController(rContext, nullptr, OUString())
{
}

FontVariationsToolBoxControl::~FontVariationsToolBoxControl() {}

void FontVariationsToolBoxControl::initialize(const uno::Sequence<uno::Any>& rArguments)
{
    PopupWindowController::initialize(rArguments);

    // the status names the font, whose axes have to be read from the font itself
    addStatusListener(u".uno:CharFontName"_ustr);
    // and the style it is, for the settings a named instance stands for
    addStatusListener(u".uno:FontStyleName"_ustr);

    if (m_pToolbar)
    {
        mxPopoverContainer.reset(new ToolbarPopupContainer(m_pToolbar));
        m_pToolbar->set_item_popover(m_aCommandURL, mxPopoverContainer->getTopLevel());
    }

    ToolBox* pToolBox = nullptr;
    ToolBoxItemId nId;
    if (getToolboxId(nId, &pToolBox))
        pToolBox->SetItemBits(nId, ToolBoxItemBits::DROPDOWNONLY | pToolBox->GetItemBits(nId));
}

void FontVariationsToolBoxControl::execute(sal_Int16 /*nKeyModifier*/)
{
    // Nothing to apply, the tool box drops the popover down by itself for a
    // DROPDOWNONLY item.
    if (m_pToolbar)
        m_pToolbar->set_menu_item_active(m_aCommandURL,
                                         !m_pToolbar->get_menu_item_active(m_aCommandURL));
    else
        createPopupWindow();
}

void FontVariationsToolBoxControl::statusChanged(const frame::FeatureStateEvent& rEvent)
{
    SolarMutexGuard aGuard;

    if (rEvent.FeatureURL.Complete == ".uno:FontStyleName")
    {
        awt::FontDescriptor aFontDesc;
        m_aStyleName = (rEvent.State >>= aFontDesc) ? aFontDesc.StyleName : OUString();
        return;
    }

    if (rEvent.FeatureURL.Complete == ".uno:CharFontName")
    {
        awt::FontDescriptor aFontDesc;
        if (rEvent.State >>= aFontDesc)
            m_aFontName = aFontDesc.Name;
        UpdateSensitivity();
        return;
    }

    OUString sVariations;
    if (rEvent.State >>= sVariations)
        m_aVariations = vcl::font::VariationsFromString(sVariations);
    else
        m_aVariations.clear();

    m_bSupported = rEvent.IsEnabled;
    UpdateSensitivity();
}

const std::vector<vcl::font::VariationAxis>& FontVariationsToolBoxControl::GetAxes()
{
    // Reading the axes makes a device and sets a font on it, too much per status update.
    if (m_aAxesFontName != m_aFontName)
    {
        m_aAxes = getAxes(m_aFontName);
        m_aAxesFontName = m_aFontName;
    }
    return m_aAxes;
}

void FontVariationsToolBoxControl::UpdateSensitivity()
{
    // Enabled only for a variable font, and only in a shell that accepts the
    // settings.
    const bool bEnabled = m_bSupported && !GetAxes().empty();
    if (m_pToolbar)
        m_pToolbar->set_item_sensitive(m_aCommandURL, bEnabled);
    else
    {
        ToolBox* pToolBox = nullptr;
        ToolBoxItemId nId;
        if (getToolboxId(nId, &pToolBox))
            pToolBox->EnableItem(nId, bEnabled);
    }
}

const FontList* FontVariationsToolBoxControl::GetFontList()
{
    const SfxObjectShell* pDocSh = SfxObjectShell::Current();
    const SvxFontListItem* pFontListItem
        = pDocSh ? pDocSh->GetItem(SID_ATTR_CHAR_FONTLIST) : nullptr;
    if (pFontListItem)
        m_pFontList = pFontListItem->GetFontList();
    else if (!m_pFontList)
    {
        m_xOwnFontList.reset(new FontList(Application::GetDefaultDevice()));
        m_pFontList = m_xOwnFontList.get();
    }
    return m_pFontList;
}

std::vector<vcl::font::Variation> FontVariationsToolBoxControl::GetVariations()
{
    if (!m_aVariations.empty())
        return m_aVariations;

    // With nothing set explicitly, the font is the named instance its style
    // names, so start from that instance's settings.
    const FontList* pFontList = GetFontList();
    return pFontList ? pFontList->GetStyleVariations(m_aFontName, m_aStyleName)
                     : std::vector<vcl::font::Variation>();
}

void FontVariationsToolBoxControl::Apply(const std::vector<vcl::font::Variation>& rVariations)
{
    const FontList* pFontList = GetFontList();
    const OUString sStyle
        = pFontList ? pFontList->FindStyleForVariations(m_aFontName, rVariations, m_aStyleName)
                    : OUString();

    // Settings that match a named instance are applied as the font style
    // instead, as an instance is better supported than explicit settings.
    uno::Sequence<beans::PropertyValue> aArgs{ comphelper::makePropertyValue(
        u"FontVariations"_ustr,
        sStyle.isEmpty() ? vcl::font::VariationsToString(rVariations) : OUString()) };
    dispatchCommand(m_aCommandURL, aArgs);

    if (!sStyle.isEmpty())
        ApplyFontStyle(getFrameInterface(), *pFontList, m_aFontName, sStyle);
}

std::unique_ptr<WeldToolbarPopup> FontVariationsToolBoxControl::weldPopupWindow()
{
    return std::make_unique<FontVariationsWindow>(this, m_pToolbar);
}

VclPtr<vcl::Window> FontVariationsToolBoxControl::createVclPopupWindow(vcl::Window* pParent)
{
    mxInterimPopover = VclPtr<InterimToolbarPopup>::Create(
        getFrameInterface(), pParent,
        std::make_unique<FontVariationsWindow>(this, pParent->GetFrameWeld()));

    mxInterimPopover->Show();

    return mxInterimPopover;
}

OUString FontVariationsToolBoxControl::getImplementationName()
{
    return u"com.sun.star.comp.svx.FontVariationsToolBoxControl"_ustr;
}

uno::Sequence<OUString> FontVariationsToolBoxControl::getSupportedServiceNames()
{
    return { u"com.sun.star.frame.ToolbarController"_ustr };
}

} // namespace svx

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_svx_FontVariationsToolBoxControl_get_implementation(
    uno::XComponentContext* rContext, uno::Sequence<uno::Any> const&)
{
    return cppu::acquire(new svx::FontVariationsToolBoxControl(rContext));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
