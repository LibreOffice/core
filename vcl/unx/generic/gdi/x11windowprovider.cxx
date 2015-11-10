/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/svapp.hxx>

#include "unx/x11windowprovider.hxx"
#include "unx/x11/x11display.hxx"

X11WindowProvider::~X11WindowProvider()
{
}

Display *OpenX11Display(OString& rDisplay)
{
    /*
     * open connection to X11 Display
     * try in this order:
     *  o  -display command line parameter,
     *  o  $DISPLAY environment variable
     *  o  default display
     */

    Display *pDisp = nullptr;

    // is there a -display command line parameter?

    sal_uInt32 nParams = osl_getCommandArgCount();
    OUString aParam;
    for (sal_uInt32 i=0; i<nParams; i++)
    {
        osl_getCommandArg(i, &aParam.pData);
        if ( aParam == "-display" )
        {
            osl_getCommandArg(i+1, &aParam.pData);
            rDisplay = OUStringToOString(
                   aParam, osl_getThreadTextEncoding());

            if ((pDisp = XOpenDisplay(rDisplay.getStr()))!=nullptr)
            {
                /*
                 * if a -display switch was used, we need
                 * to set the environment accoringly since
                 * the clipboard build another connection
                 * to the xserver using $DISPLAY
                 */
                OUString envVar("DISPLAY");
                osl_setEnvironment(envVar.pData, aParam.pData);
            }
            break;
        }
    }

    if (!pDisp && rDisplay.isEmpty())
    {
        // Open $DISPLAY or default...
        char *pDisplay = getenv("DISPLAY");
        if (pDisplay != nullptr)
            rDisplay = OString(pDisplay);
        pDisp  = XOpenDisplay(pDisplay);
    }

    return pDisp;
}

namespace vcl
{

bool IsWindowSystemAvailable()
{
    Display *pDisp;
    OString aDisplay;

    pDisp = OpenX11Display(aDisplay);
    if (pDisp)
        XCloseDisplay(pDisp);

    return (pDisp != nullptr);
}

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
