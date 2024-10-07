/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include "basctldllapi.h"
#include "scriptdocument.hxx"
#include <sfx2/dispatch.hxx>
#include <vcl/weld.hxx>

namespace basctl
{
BASCTL_DLLPUBLIC bool IsValidSbxName(std::u16string_view rName);
BASCTL_DLLPUBLIC SAL_RET_MAYBENULL SfxDispatcher* GetDispatcher();
BASCTL_DLLPUBLIC void MarkDocumentModified(const ScriptDocument& rDocument);

BASCTL_DLLPUBLIC bool QueryDelDialog(std::u16string_view rName, weld::Widget* pParent);
BASCTL_DLLPUBLIC bool QueryDelModule(std::u16string_view rName, weld::Widget* pParent);
BASCTL_DLLPUBLIC bool QueryDelLib(std::u16string_view rName, bool bRef, weld::Widget* pParent);
BASCTL_DLLPUBLIC bool
QueryPassword(weld::Widget* pDialogParent,
              const css::uno::Reference<css::script::XLibraryContainer>& xLibContainer,
              const OUString& rLibName, OUString& rPassword, bool bRepeat = false,
              bool bNewTitle = false);

BASCTL_DLLPUBLIC void ImportLib(const ScriptDocument& rDocument, weld::Dialog* pDialog,
                                const std::function<void(OUString& rLibName)>& func_remove_entry,
                                const std::function<void(OUString& rLibName)>& func_insert_entry,
                                const std::function<void()>& func_insert_entries);

BASCTL_DLLPUBLIC void Export(const ScriptDocument& rDocument, const OUString& aLibName,
                             weld::Dialog* pDialog);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
