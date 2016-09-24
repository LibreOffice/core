/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Properties;

/** Load from and save options into a file.
*/
class Options
    extends Properties
{
    public static Options Instance ()
    {
        if (saOptions == null)
            saOptions = new Options ();
        return saOptions;
    }

    public static void SetBoolean (String sName, boolean bValue)
    {
        Instance().setProperty (sName, Boolean.toString(bValue));
    }

    public static boolean GetBoolean (String sName)
    {
        return Boolean.getBoolean(Instance().getProperty (sName));
    }

    public static void SetInteger (String sName, int nValue)
    {
        Instance().setProperty (sName, Integer.toString(nValue));
    }

    public static int GetInteger (String sName, int nDefault)
    {
        String sValue = Instance().getProperty (sName);
        if (sValue == null)
            return nDefault;
        else
            return Integer.parseInt (sValue);
    }

    public void Load (String sBaseName)
    {
        FileInputStream fis = null;
        try
        {
            fis = new FileInputStream (ProvideFile(sBaseName));
            load (fis);
        }
        catch (IOException e)
        {
            // Ignore a non-existing options file.
        }
        finally
        {
            try
            {
                if (fis != null)
                    fis.close();
            }
            catch (IOException ex)
            {
            }
        }
    }

    public void Save (String sBaseName)
    {
        FileOutputStream fos = null;
        try
        {
            fos = new FileOutputStream (ProvideFile(sBaseName));
            store (fos, null);
        }
        catch (IOException e)
        {
        }
        finally
        {
            try
            {
                if (fos != null)
                    fos.close();
            }
            catch (IOException ex)
            {
            }
        }
    }

    private Options ()
    {
    }

    private File ProvideFile (String sBaseName)
    {
        return new File (
            System.getProperty ("user.home"),
            sBaseName);
    }

    private static Options saOptions = null;
}
