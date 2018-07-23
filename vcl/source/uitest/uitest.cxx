/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <vector>
#include <vcl/uitest/uitest.hxx>
#include <vcl/uitest/uiobject.hxx>

#include <vcl/dialog.hxx>

#include <svdata.hxx>

#include <comphelper/dispatchcommand.hxx>


bool UITest::executeCommand(const OUString& rCommand, const css::uno::Sequence< css::beans::PropertyValue >& rArgs)
{
    css::uno::Sequence< css::beans::PropertyValue > lNewArgs =
        {{"SynchronMode", -1, css::uno::Any(true),
          css::beans::PropertyState_DIRECT_VALUE}};

    sal_uInt32 nArgs = rArgs.getLength();
    if ( nArgs > 0 )
    {
        sal_uInt32 nIndex( lNewArgs.getLength() );
        lNewArgs.realloc( lNewArgs.getLength()+rArgs.getLength() );

        for ( sal_uInt32 i = 0; i < nArgs; i++ )
            lNewArgs[nIndex++] = rArgs[i];
    }
    return comphelper::dispatchCommand(rCommand,lNewArgs);
}

bool UITest::executeDialog(const OUString& rCommand)
{
    return comphelper::dispatchCommand(
        rCommand,
        {{"SynchronMode", -1, css::uno::Any(false),
          css::beans::PropertyState_DIRECT_VALUE}});
}

std::unique_ptr<UIObject> UITest::getFocusTopWindow()
{
    ImplSVData* pSVData = ImplGetSVData();
    ImplSVWinData& rWinData = pSVData->maWinData;

    if (!rWinData.mpExecuteDialogs.empty())
    {
        return rWinData.mpExecuteDialogs.back()->GetUITestFactory()(rWinData.mpExecuteDialogs.back());
    }

    return rWinData.mpFirstFrame->GetUITestFactory()(rWinData.mpFirstFrame);
}

std::unique_ptr<UIObject> UITest::getFloatWindow()
{
    ImplSVData* pSVData = ImplGetSVData();
    ImplSVWinData& rWinData = pSVData->maWinData;

    VclPtr<vcl::Window> pFloatWin = rWinData.mpFirstFloat;
    if (pFloatWin)
        return pFloatWin->GetUITestFactory()(pFloatWin);

    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
