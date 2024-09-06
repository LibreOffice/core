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

public class Program
{
    public static bool atWork = false;

    private static int Main()
    {
        try
        {
            // Connect to a new instance of the Office
            XComponentContext context = NativeBootstrap.bootstrap();
            Console.WriteLine("Connected to a running office...");

            // Get a reference to the Desktop service
            XDesktop2 desktop = Desktop.create(context);

            // Create our termination request listener, and register it
            TerminateListener listener = new TerminateListener();
            desktop.addTerminateListener(listener);

            // Try to terminate while we are at work.
            atWork = true;
            bool terminated = desktop.terminate();
            Console.WriteLine("The Office {0}", terminated
                ? "has been terminated"
                : "is still running, we are at work");

            // Try to terminate when we are NOT at work.
            atWork = false;
            terminated = desktop.terminate();
            Console.WriteLine("The Office {0}", terminated
                ? "has been terminated"
                : "is still running. Something else prevents termination, such as the quickstarter.");

            return 0;
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine(ex);

            return 1;
        }
    }
}