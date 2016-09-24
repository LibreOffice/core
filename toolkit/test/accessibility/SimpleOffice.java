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

import com.sun.star.frame.XDesktop;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.awt.XExtendedToolkit;


/** This class tries to simplify some tasks like loading a document or
    getting various objects.
*/
public class SimpleOffice
{
    private XDesktop mxDesktop = null;
    private OfficeConnection aConnection;
    private int mnPortNumber;

    public SimpleOffice (int nPortNumber)
    {
        mnPortNumber = nPortNumber;
        connect ();
        getDesktop ();
    }

    public void connect ()
    {
        aConnection = new OfficeConnection (mnPortNumber);
        mxDesktop = null;
        getDesktop ();
    }











    public  XDesktop getDesktop ()
    {
        if (mxDesktop != null)
            return mxDesktop;
        try
        {
            //  Get the factory of the connected office.
            XMultiServiceFactory xMSF = aConnection.getServiceManager ();
            if (xMSF == null)
            {
                MessageArea.println ("can't connect to office");
                return null;
            }
            else
                MessageArea.println ("Connected successfully.");

            //  Create a new desktop.
            mxDesktop = UnoRuntime.queryInterface(
                XDesktop.class,
                xMSF.createInstance ("com.sun.star.frame.Desktop")
                );
        }
        catch (Exception e)
        {
            MessageArea.println ("caught exception while creating desktop: "
                + e);
        }

        return mxDesktop;
    }


    /** Return a reference to the extended toolkit which is a broadcaster of
        top window, key, and focus events.
    */
    public XExtendedToolkit getExtendedToolkit ()
    {
        XExtendedToolkit xToolkit = null;
        try
        {
            //  Get the factory of the connected office.
            XMultiServiceFactory xMSF = aConnection.getServiceManager ();
            if (xMSF != null)
            {
                xToolkit = UnoRuntime.queryInterface(
                    XExtendedToolkit.class,
                    xMSF.createInstance ("stardiv.Toolkit.VCLXToolkit")
                    );
            }
        }
        catch (Exception e)
        {
            MessageArea.println ("caught exception while creating extended toolkit: " + e);
        }

        return xToolkit;
    }



    public XAccessible getAccessibleObject (XInterface xObject)
    {
        XAccessible xAccessible = null;
        try
        {
            xAccessible = UnoRuntime.queryInterface(
                XAccessible.class, xObject);
        }
        catch (Exception e)
        {
            MessageArea.println (
                "caught exception while getting accessible object" + e);
            e.printStackTrace();
        }
        return xAccessible;
    }



}
