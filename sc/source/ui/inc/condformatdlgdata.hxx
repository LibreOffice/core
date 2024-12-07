/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>
#include <memory>

namespace condformat::dialog
{
enum ScCondFormatDialogType
{
    NONE,
    CONDITION,
    COLORSCALE,
    DATABAR,
    ICONSET,
    DATE
};
}

class ScConditionalFormatList;

class ScCondFormatDlgData
{
public:
    ScCondFormatDlgData(std::shared_ptr<ScConditionalFormatList> pCondFormats, sal_uInt32 nItem,
                        bool bManaged);

    ScCondFormatDlgData(ScCondFormatDlgData const&) = default;
    ScCondFormatDlgData(ScCondFormatDlgData&&) = default;

    bool IsManaged() const;
    condformat::dialog::ScCondFormatDialogType GetDialogType() const;
    sal_uInt32 GetIndex() const;

    void SetDialogType(condformat::dialog::ScCondFormatDialogType eType);

    ScConditionalFormatList* GetConditionalFormatList();

private:
    std::shared_ptr<ScConditionalFormatList> mpCondFormats;
    sal_uInt32 mnItem;
    condformat::dialog::ScCondFormatDialogType meDialogType;
    bool mbManaged;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
