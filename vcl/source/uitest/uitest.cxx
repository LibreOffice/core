/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/uitest/uitest.hxx>
#include <vcl/uitest/uiobject.hxx>

#include <vcl/dialog.hxx>

#include "uitest/factory.hxx"

#include "svdata.hxx"

#include <comphelper/dispatchcommand.hxx>

void UITest::executeCommand(const OUString& rCommand)
{
    comphelper::dispatchCommand(rCommand, css::uno::Sequence<css::beans::PropertyValue>());
}

std::unique_ptr<UIObject> UITest::getFocusTopWindow()
{
    ImplSVData* pSVData = ImplGetSVData();
    ImplSVWinData& rWinData = pSVData->maWinData;

    if (rWinData.mpLastExecuteDlg)
        return UITestWrapperFactory::createObject(rWinData.mpLastExecuteDlg.get());

    return UITestWrapperFactory::createObject(rWinData.mpFirstFrame.get());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
