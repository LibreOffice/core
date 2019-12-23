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

#include <map>

namespace weld
{
class Toolbar;
}

class SFX2_DLLPUBLIC ToolbarUnoDispatcher
{
private:
    css::uno::Reference<css::frame::XFrame> m_xFrame;
    weld::Toolbar* m_pToolbar;

    DECL_LINK(SelectHdl, const OString&, void);

    void CreateController(const OUString& rCommand);

    typedef std::map<OUString, css::uno::Reference<css::frame::XToolbarController>>
        ControllerContainer;
    ControllerContainer maControllers;

    css::uno::Reference<css::frame::XToolbarController>
    GetControllerForCommand(const OUString& rCommand) const;

public:
    // fill in the label and icons for actions and dispatch the action on item click
    ToolbarUnoDispatcher(weld::Toolbar& rToolbar,
                         const css::uno::Reference<css::frame::XFrame>& rFrame);
    void dispose();
    ~ToolbarUnoDispatcher();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
