/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

using System;

using com.sun.star.beans;
using com.sun.star.container;
using com.sun.star.lang;
using com.sun.star.uno;
using com.sun.star.util;

// List of pre-defined path variables supported by the path substitution service
string[] predefinedPathVariables =
{
    "$(home)","$(inst)","$(prog)","$(temp)","$(user)", "$(username)",
    "$(work)","$(path)","$(langid)","$(vlang)"
};

try
{
    // Start a new office process and get the remote office context
    XComponentContext context = NativeBootstrap.bootstrap();
    Console.WriteLine("Connected to a running office...");

    XStringSubstitution pathSubst = context.getServiceManager()
        .createInstanceWithContext("com.sun.star.comp.framework.PathSubstitution", context)
        .query<XStringSubstitution>();
    WorkWithPathVariables(pathSubst);

    return 0;
}
catch (Exception e)
{
    Console.Error.WriteLine(e);

    return 1;
}

void WorkWithPathVariables(XStringSubstitution pathSubst)
{
    if (pathSubst != null)
    {
        foreach (string pathVar in predefinedPathVariables)
        {
            try
            {
                string value = pathSubst.getSubstituteVariableValue(pathVar);
                Console.WriteLine($"Property = {pathVar} | Path = {value}");
            }
            catch (NoSuchElementException)
            {
                Console.Error.WriteLine($"NoSuchElementException has been thrown accessing {pathVar}");
            }
        }

        try
        {
            // Check the resubstitution function
            // Use $(home) as starting point and extend the path
            string value = pathSubst.getSubstituteVariableValue(predefinedPathVariables[0]);
            value += "/test";
            
            Console.WriteLine($"Path = {value}");
            string resub = pathSubst.reSubstituteVariables(value);
            Console.WriteLine($"Resubstituted Path = {resub}");
        }
        catch (NoSuchElementException)
        {
            Console.Error.WriteLine($"NoSuchElementException has been thrown accessing {predefinedPathVariables[0]}");
        }
    }
}
