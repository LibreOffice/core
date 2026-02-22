/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/ustring.hxx>
#include <vcl/dllapi.h>

#include <functional>

namespace weld
{
class Builder;
class Container;
class Dialog;
class MessageDialog;
class Widget;

class VCL_DLLPUBLIC DialogController : public std::enable_shared_from_this<DialogController>
{
public:
    virtual Dialog* getDialog() = 0;
    const Dialog* getConstDialog() const
    {
        return const_cast<DialogController*>(this)->getDialog();
    }
    virtual short run();
    static bool runAsync(const std::shared_ptr<DialogController>& rController,
                         const std::function<void(sal_Int32)>&);
    void set_title(const OUString& rTitle);
    OUString get_title() const;
    void set_help_id(const OUString& rHelpId);
    OUString get_help_id() const;
    void response(int nResponse);
    virtual ~DialogController();
};

class VCL_DLLPUBLIC GenericDialogController : public DialogController
{
protected:
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::shared_ptr<weld::Dialog> m_xDialog;

public:
    GenericDialogController(weld::Widget* pParent, const OUString& rUIFile,
                            const OUString& rDialogId, bool bMobile = false);
    virtual Dialog* getDialog() override;
    virtual ~GenericDialogController() override;
};

class VCL_DLLPUBLIC MessageDialogController : public DialogController
{
protected:
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::MessageDialog> m_xDialog;
    std::unique_ptr<weld::Container> m_xContentArea;
    std::unique_ptr<weld::Widget> m_xRelocate;
    std::unique_ptr<weld::Container> m_xOrigParent;

public:
    /* @param rRelocateId - optional argument of the name of a widget in the .ui file
                            which should be relocated into the content area of the dialog.

                            e.g. a checkbox for a "Never show this again" option.

                            This results in the named widget relocating to the same container
                            as the messages.  This enables aligning the extra widget with the
                            message labels in the content area container which doesn't
                            explicitly exist in the ui description, but is only implied.
    */
    MessageDialogController(weld::Widget* pParent, const OUString& rUIFile,
                            const OUString& rDialogId, const OUString& rRelocateId = {});
    virtual Dialog* getDialog() override;
    virtual ~MessageDialogController() override;
    void set_primary_text(const OUString& rText);
    OUString get_primary_text() const;
    void set_secondary_text(const OUString& rText);
    OUString get_secondary_text() const;
    void set_default_response(int nResponse);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
