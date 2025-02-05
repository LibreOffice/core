/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <DrawViewShell.hxx>
#include <ViewShellBase.hxx>
#include <sdmod.hxx>

#include <com/sun/star/presentation/SlideShow.hpp>

#include <comphelper/lok.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/servicehelper.hxx>
#include <sfx2/lokhelper.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <unomodel.hxx>

using namespace css;

namespace sd {

void DrawViewShell::ConfigurationChanged( utl::ConfigurationBroadcaster* pCb, ConfigurationHints )
{
    svtools::ColorConfig *pColorConfig = dynamic_cast<svtools::ColorConfig*>(pCb);
    ConfigureAppBackgroundColor(pColorConfig);
    if (!comphelper::LibreOfficeKit::isActive())
    {
        SdViewOptions aViewOptions = GetViewOptions();
        aViewOptions.mnDocBackgroundColor = pColorConfig->GetColorValue(svtools::DOCCOLOR).nColor;
        SetViewOptions(aViewOptions);
    }
    else
    {
        SfxViewShell* pCurrentShell = SfxViewShell::Current();
        ViewShellBase* pShellBase = dynamic_cast<ViewShellBase*>(pCurrentShell);
        if (!pShellBase)
            return;
        SdViewOptions aViewOptions = pShellBase->GetViewOptions();
        aViewOptions.mnDocBackgroundColor = pColorConfig->GetColorValue(svtools::DOCCOLOR).nColor;
        aViewOptions.msColorSchemeName = svtools::ColorConfig::GetCurrentSchemeName();
        pShellBase->SetViewOptions(aViewOptions);
        SdXImpressDocument* pDoc = comphelper::getFromUnoTunnel<SdXImpressDocument>(pCurrentShell->GetCurrentDocument());
        SfxLokHelper::notifyViewRenderState(pCurrentShell, pDoc);
        Color aFillColor(pColorConfig->GetColorValue(svtools::APPBACKGROUND).nColor);
        pCurrentShell->libreOfficeKitViewCallback(LOK_CALLBACK_APPLICATION_BACKGROUND_COLOR,
                    aFillColor.AsRGBHexString().toUtf8());
    }
}

void DrawViewShell::ConfigureAppBackgroundColor( svtools::ColorConfig *pColorConfig )
{
    if (!pColorConfig)
        pColorConfig = &SdModule::get()->GetColorConfig();
    Color aFillColor( pColorConfig->GetColorValue( svtools::APPBACKGROUND ).nColor );
    if (comphelper::LibreOfficeKit::isActive())
        aFillColor = COL_TRANSPARENT;
    // tdf#87905 Use darker background color for master view
    if (meEditMode == EditMode::MasterPage)
        aFillColor.DecreaseLuminance( 64 );
    SdViewOptions aViewOptions = GetViewOptions();
    aViewOptions.mnAppBackgroundColor = aFillColor;
    SetViewOptions(aViewOptions);
}

void DrawViewShell::destroyXSlideShowInstance()
{
    if (!mxSlideShow.is())
        return;

    try
    {
        uno::Reference<lang::XComponent> xComponent(mxSlideShow, uno::UNO_QUERY);
        if (xComponent.is())
            xComponent->dispose();
    }
    catch (uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION( "sd", "DrawViewShell::destroyXSlideShowInstance dispose");
    }

    mxSlideShow.clear();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
