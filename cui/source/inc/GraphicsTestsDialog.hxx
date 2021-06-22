/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vcl/bitmapex.hxx>
#include <vcl/weld.hxx>
#include <ImageViewerDialog.hxx>

#include <map>
#include <vector>

class GraphicsTestsDialog : public weld::GenericDialogController
{
    std::unique_ptr<weld::Label> m_xResultLog;
    std::unique_ptr<weld::Button> m_xDownloadResults;

    std::vector<std::unique_ptr<weld::Button>> m_xTestLabels;

    std::map<OString, Bitmap> m_xResultImage;
    std::map<OString, bool> m_xSkippedTests;

    DECL_STATIC_LINK(GraphicsTestsDialog, HandleDownloadRequest, weld::Button&, void);
    DECL_LINK(HandleResultViewRequest, weld::Button&, void);

public:
    GraphicsTestsDialog(weld::Window* pParent);
    void runGraphicsTestandUpdateLog();
};
