/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_WELDUTILS_HXX
#define INCLUDED_SFX2_WELDUTILS_HXX

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XToolbarController.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <tools/link.hxx>
#include <sfx2/dllapi.h>
#include <svtools/miscopt.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

#include <map>

namespace weld
{
class Builder;
class Toolbar;
}

class SFX2_DLLPUBLIC ToolbarUnoDispatcher
{
private:
    css::uno::Reference<css::frame::XFrame> m_xFrame;
    SvtMiscOptions m_aToolbarOptions;
    weld::Toolbar* m_pToolbar;
    weld::Builder* m_pBuilder;
    bool m_bSideBar;

    DECL_LINK(SelectHdl, const OString&, void);
    DECL_LINK(ToggleMenuHdl, const OString&, void);
    DECL_LINK(ChangedIconSizeHandler, LinkParamNone*, void);

    void CreateController(const OUString& rCommand);
    vcl::ImageType GetIconSize() const;

    typedef std::map<OUString, css::uno::Reference<css::frame::XToolbarController>>
        ControllerContainer;
    ControllerContainer maControllers;

public:
    // fill in the label and icons for actions and dispatch the action on item click
    ToolbarUnoDispatcher(weld::Toolbar& rToolbar, weld::Builder& rBuilder,
                         const css::uno::Reference<css::frame::XFrame>& rFrame,
                         bool bSideBar = true);

    css::uno::Reference<css::frame::XToolbarController>
    GetControllerForCommand(const OUString& rCommand) const;

    void dispose();
    ~ToolbarUnoDispatcher();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
