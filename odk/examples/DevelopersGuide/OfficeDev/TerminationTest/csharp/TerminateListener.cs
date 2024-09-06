/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

using System;

using com.sun.star.frame;
using com.sun.star.lang;
using com.sun.star.uno;

public class TerminateListener : WeakBase, XTerminateListener, XServiceInfo
{
    // XTerminateListener
    public void notifyTermination(EventObject eventObject)
    {
        Console.WriteLine("About to terminate...");
    }

    public void queryTermination(EventObject eventObject)
    {
        if (Program.atWork)
        {
            Console.WriteLine("Terminate while we are at work? You can't be serious ;-)!");
            throw new TerminationVetoException();
        }
    }

    public void disposing(EventObject eventObject) { }

    // XServiceInfo
    public string getImplementationName() => GetType().FullName;

    public bool supportsService(string serviceName) => serviceName == "com.sun.star.frame.XTerminateListener";

    public string[] getSupportedServiceNames() => new[] { "com.sun.star.frame.XTerminateListener" };
}
