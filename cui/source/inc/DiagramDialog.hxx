/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef _CUI_DIAGRAMDIALOG_HXX
#define _CUI_DIAGRAMDIALOG_HXX

#include "tools/link.hxx"

#include <vcl/weld.hxx>

class DiagramDataInterface;

class DiagramDialog : public weld::GenericDialogController
{
public:
    DiagramDialog(weld::Window* pWindow, std::shared_ptr<DiagramDataInterface> pDiagramData);
    virtual ~DiagramDialog() override;

private:
    std::shared_ptr<DiagramDataInterface> mpDiagramData;
    std::unique_ptr<weld::Button> mpBtnOk;
    std::unique_ptr<weld::Button> mpBtnCancel;
    std::unique_ptr<weld::TextView> mpTextDiagram;
};

#endif // _CUI_DIAGRAMDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
