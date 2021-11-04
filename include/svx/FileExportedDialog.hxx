/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/weld.hxx>
#include <svx/svxdllapi.h>

class SVX_DLLPUBLIC FileExportedDialog final : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::Label> m_xFileLabel;
    std::unique_ptr<weld::Button> m_xButton;

    DECL_DLLPRIVATE_LINK(OpenHdl, weld::Button&, void);

public:
    explicit FileExportedDialog(weld::Window* pParent, OUString atitle);
};
