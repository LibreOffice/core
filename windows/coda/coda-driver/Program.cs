// -*- tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*-
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// coda-driver: a WebDriver server for the native user interface of CODA-W, for the WebdriverIO
// tests in qt/test/specs. It is the browser.native of those tests, as AT-SPI is for CODA-Q and
// macos/coda-driver is for CODA-M. The web content is driven through msedgedriver instead.
//
// It answers the same small part of the W3C WebDriver protocol as macos/coda-driver, and finds
// elements and describes the windows of one process through UI Automation.
//
// Usage: coda-driver --target-pid <pid> --port <port>
//
// The driver exits when the target process does.

using System;
using System.Diagnostics;

namespace CodaDriver
{
    static class Program
    {
        static string? ArgValue(string[] args, string name)
        {
            int index = Array.IndexOf(args, name);
            if (index < 0 || index + 1 >= args.Length)
                return null;
            return args[index + 1];
        }

        static int Main(string[] args)
        {
            if (!int.TryParse(ArgValue(args, "--target-pid"), out int targetPid)
                || !int.TryParse(ArgValue(args, "--port"), out int port))
            {
                Console.Error.WriteLine("Usage: coda-driver --target-pid <pid> --port <port>");
                return 2;
            }

            Process target;
            try
            {
                target = Process.GetProcessById(targetPid);
            }
            catch (ArgumentException)
            {
                Console.Error.WriteLine($"coda-driver: no process with id {targetPid}");
                return 3;
            }

            target.EnableRaisingEvents = true;
            target.Exited += (sender, e) =>
            {
                Console.Error.WriteLine("coda-driver: the target process exited, exiting too");
                Environment.Exit(0);
            };

            var server = new NativeUIServer(port, targetPid);
            Console.Error.WriteLine(
                $"coda-driver: listening on port {port} for process {targetPid}");
            server.Run();
            return 0;
        }
    }
}
