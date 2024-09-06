/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

using System;

using com.sun.star.beans;
using com.sun.star.lang;
using com.sun.star.uno;
using com.sun.star.util;

// List of pre-defined path variables supported by the PathSettings service
string[] predefinedPathProperties =
{
    "Addin", "AutoCorrect", "AutoText", "Backup", "Basic",
    "Bitmap", "Config", "Dictionary", "Favorite", "Filter",
    "Gallery", "Graphic", "Help", "Linguistic", "Module",
    "Palette", "Plugin", "Storage", "Temp", "Template",
    "UIConfig", "UserConfig", "UserDictionary", "Work"
};

// Provides an example for accessing and using the PathSettings service
try
{
    // Start a new office process and get the remote office context
    XComponentContext context = NativeBootstrap.bootstrap();
    Console.WriteLine("Connected to a running office...");

    XPropertySet pathSettings = thePathSettings.get(context);
    WorkWithPathSettings(pathSettings);

    return 0;
}
catch (Exception e)
{
    Console.Error.WriteLine(e);

    return 1;
}

// Retrieve and set path properties using the PathSettings service
void WorkWithPathSettings(XPropertySet pathSettings)
{
    if (pathSettings != null)
    {
        foreach (string prop in predefinedPathProperties)
        {
            try
            {
                string value = pathSettings.getPropertyValue(prop).cast<string>();
                Console.WriteLine($"Property = {prop} | Path = {value}");
            }
            catch (UnknownPropertyException)
            {
                Console.Error.WriteLine($"UnknownPropertyException was thrown while accessing {prop}");
            }
            catch (WrappedTargetException)
            {
                Console.Error.WriteLine($"WrappedTargetException was thrown while accessing {prop}");
            }
        }

        // Try to modify the work path property.
        // After running this example you should see the new value of "My Documents"
        // in the Paths tab, accessible via "Tools - Options - LibreOffice - Paths".
        // The change can be reverted through the Paths tab as well.
        try
        {
            pathSettings.setPropertyValue("Work", new Any("$(temp)"));
            string value = pathSettings.getPropertyValue("Work").cast<string>();
            Console.WriteLine("Note: The example changes your current work path setting!");
            Console.WriteLine($"The work path should now be {value}");
        }
        catch (UnknownPropertyException)
        {
            Console.Error.WriteLine("UnknownPropertyException was thrown while setting Work path");
        }
        catch (WrappedTargetException)
        {
            Console.Error.WriteLine("WrappedTargetException was thrown while setting Work path");
        }
        catch (PropertyVetoException)
        {
            Console.Error.WriteLine("PropertyVetoException was thrown while setting Work path");
        }
        catch (IllegalArgumentException)
        {
            Console.Error.WriteLine("IllegalArgumentException was thrown while setting Work path");
        }
    }
}
