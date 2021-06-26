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
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <tools/link.hxx>
#include <svx/FileExportedDialog.hxx>

#include "ImageViewerDialog.hxx"

#include <vector>

class GraphicTestEntry final
{
private:
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Container> m_xContainer;
    std::unique_ptr<weld::Label> m_xTestLabel;
    std::unique_ptr<weld::Button> m_xTestButton;

    weld::Dialog* m_xParentDialog;

    Bitmap m_xResultBitmap;

public:
    DECL_LINK(HandleResultViewRequest, weld::Button&, void);
    GraphicTestEntry(weld::Container* pParent, weld::Dialog* pDialog, OUString aTestName,
                     OUString aTestStatus, Bitmap aTestBitmap);
    weld::Widget* get_widget() const { return m_xContainer.get(); }
};

class GraphicsTestsDialog : public weld::GenericDialogController
{
    std::unique_ptr<weld::TextView> m_xResultLog;
    std::unique_ptr<weld::Button> m_xDownloadResults;
    std::unique_ptr<weld::Box> m_xContainerBox;

    std::vector<std::unique_ptr<GraphicTestEntry>> m_xGraphicTestEntries;

    OUString m_xZipFileUrl;
    OUString m_xCreateFolderUrl;

    DECL_LINK(HandleDownloadRequest, weld::Button&, void);
    DECL_LINK(HandleResultViewRequest, weld::Button&, void);

public:
    GraphicsTestsDialog(weld::Window* pParent);
    ~GraphicsTestsDialog();
    virtual short run() override;
};
