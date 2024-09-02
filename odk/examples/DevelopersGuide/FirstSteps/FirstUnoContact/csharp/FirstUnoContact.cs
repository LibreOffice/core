/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

using System;

using com.sun.star.lang;
using com.sun.star.uno;

try
{
    XComponentContext xContext = NativeBootstrap.bootstrap();
    Console.WriteLine("Connected to a running office...");

    XMultiComponentFactory xMCF = xContext.getServiceManager();
    Console.WriteLine("Remote service manager is {0}", xMCF is null ? "not available" : "available");

    return 0;
}
catch (UnoException e)
{
    Console.Error.WriteLine(e.Message);

    return 1;
}
