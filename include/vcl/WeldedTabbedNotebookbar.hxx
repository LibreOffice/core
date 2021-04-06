/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_NOTEBOOKBAR_WRITERTABBEDNOTEBOOKBAR_HXX
#define INCLUDED_SFX2_NOTEBOOKBAR_WRITERTABBEDNOTEBOOKBAR_HXX

#include <vcl/dllapi.h>
#include <rtl/ustring.hxx>
#include <vcl/weld.hxx>
#include <com/sun/star/frame/XFrame.hpp>

/**
 * Welded wrapper for NotebookBar used for online
*/
class VCL_DLLPUBLIC WeldedTabbedNotebookbar
{
    std::unique_ptr<weld::Builder> m_xBuilder;

    std::unique_ptr<weld::Container> m_xContainer;

public:
    WeldedTabbedNotebookbar(const VclPtr<vcl::Window>& pContainerWindow,
                            const OUString& rUIFilePath,
                            const css::uno::Reference<css::frame::XFrame>& rFrame,
                            sal_uInt64 nWindowId);
};

#endif // INCLUDED_SFX2_NOTEBOOKBAR_SFXNOTEBOOKBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
