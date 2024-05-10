/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svx/FileExportedDialog.hxx>

#include <comphelper/diagnose_ex.hxx>
#include <comphelper/backupfilehelper.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/system/XSystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>

FileExportedDialog::FileExportedDialog(weld::Window* pParent, OUString atitle)
    : GenericDialogController(pParent, u"svx/ui/fileexporteddialog.ui"_ustr,
                              u"FileExportedDialog"_ustr)
    , m_xFileLabel(m_xBuilder->weld_label(u"Filelabel"_ustr))
    , m_xButton(m_xBuilder->weld_button(u"ok"_ustr))
{
    m_xFileLabel->set_label(atitle);
    m_xButton->connect_clicked(LINK(this, FileExportedDialog, OpenHdl));
}

IMPL_LINK_NOARG(FileExportedDialog, OpenHdl, weld::Button&, void)
{
    const OUString uri(comphelper::BackupFileHelper::getUserProfileURL());
    css::uno::Reference<css::system::XSystemShellExecute> exec(
        css::system::SystemShellExecute::create(comphelper::getProcessComponentContext()));
    try
    {
        exec->execute(uri, OUString(), css::system::SystemShellExecuteFlags::URIS_ONLY);
    }
    catch (const css::uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("svx.dialog", "opening <" << uri << "> failed:");
    }
    m_xDialog->response(RET_OK);
}