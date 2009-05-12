/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AcceleratorsConfigurationTest.java,v $
 * $Revision: 1.1.2.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package complex.accelerators;

// imports
import com.sun.star.awt.*;
import com.sun.star.beans.*;
import com.sun.star.container.*;
import com.sun.star.embed.*;
import com.sun.star.lang.*;
import com.sun.star.ui.*;
import com.sun.star.uno.*;
import com.sun.star.util.*;

import complexlib.ComplexTestCase;

import java.lang.*;
import java.util.*;

import helper.*;

//-----------------------------------------------
/** @short todo document me
 */
public class AcceleratorsConfigurationTest extends ComplexTestCase
{
    /** points to the global uno service manager. */
    private XMultiServiceFactory m_xSmgr = null;

    /** the accelerator configuration for testing. */
    private XAcceleratorConfiguration m_xGlobalAccelCfg = null;
    private XAcceleratorConfiguration m_xModuleAccelCfg = null;
    private XAcceleratorConfiguration m_xDocumentAccelCfg = null;

    /** XCS/XCU based accelerator configuration. */
    private XNameAccess m_xConfig = null;
    private XNameAccess m_xPrimaryKeys   = null;
    private XNameAccess m_xSecondaryKeys = null;

    //-------------------------------------------
    // test environment

    //-----------------------------------------------
    /** @short todo document me
     */
    public String[] getTestMethodNames()
    {
        return new String[]
            {
                "checkGlobalAccelCfg",
                "checkModuleAccelCfg",
                "checkDocumentAccelCfg"
            };
    }

    //-----------------------------------------------
    /** @short Create the environment for following tests.
     */
    public void before()
        throws java.lang.Exception
    {
        // get uno service manager from global test environment
        m_xSmgr = (XMultiServiceFactory)param.getMSF();

        m_xGlobalAccelCfg = (XAcceleratorConfiguration)UnoRuntime.queryInterface(
            XAcceleratorConfiguration.class,
            m_xSmgr.createInstance("com.sun.star.ui.GlobalAcceleratorConfiguration"));
        m_xModuleAccelCfg = (XAcceleratorConfiguration)UnoRuntime.queryInterface(
            XAcceleratorConfiguration.class,
            m_xSmgr.createInstance("com.sun.star.ui.ModuleAcceleratorConfiguration"));
        m_xDocumentAccelCfg = (XAcceleratorConfiguration)UnoRuntime.queryInterface(
            XAcceleratorConfiguration.class,
            m_xSmgr.createInstance("com.sun.star.ui.DocumentAcceleratorConfiguration"));

        String sConfigPath = "org.openoffice.Office.Accelerators";
        boolean bReadOnly = false;
        XNameAccess m_xConfig = openConfig(m_xSmgr, sConfigPath, bReadOnly);
        if (m_xConfig != null)
        {
            m_xPrimaryKeys = (XNameAccess)UnoRuntime.queryInterface(XNameAccess.class, m_xConfig.getByName("PrimaryKeys"));
            m_xSecondaryKeys = (XNameAccess)UnoRuntime.queryInterface(XNameAccess.class, m_xConfig.getByName("SecondaryKeys"));
        }
    }

    //-------------------------------------------
    /** @short  close the environment.
     */
    public void after()
        throws java.lang.Exception
    {
        m_xConfig           = null;
        m_xGlobalAccelCfg   = null;
        m_xModuleAccelCfg   = null;
        m_xDocumentAccelCfg = null;
        m_xSmgr             = null;
    }

    //-------------------------------------------
    /** @todo document me.
     */
    public void checkGlobalAccelCfg()
        throws java.lang.Exception
    {
        log.println("\n---- check Global accelerator configuration: ----");

        String[] sKeys;
        XNameAccess xPrimaryAccess = (XNameAccess)UnoRuntime.queryInterface(XNameAccess.class,m_xPrimaryKeys.getByName("Global"));
        XNameAccess xSecondaryAccess = (XNameAccess)UnoRuntime.queryInterface(XNameAccess.class, m_xSecondaryKeys.getByName("Global"));

        sKeys = new String[] { "A_MOD1" };
        impl_checkGetKeyCommands(m_xGlobalAccelCfg, xPrimaryAccess, sKeys);

        sKeys = new String[] { "PASTE", "X_SHIFT" };
        String[] sCommands = new String[] { ".uno:test", ".uno:test" };
        impl_checkSetKeyCommands(m_xGlobalAccelCfg, xPrimaryAccess, xSecondaryAccess, sKeys, sCommands);

        sKeys = new String[] { "C_MOD1", "CUT" };
        impl_checkRemoveKeyCommands(m_xGlobalAccelCfg, xPrimaryAccess, xSecondaryAccess, sKeys);

        String[] sCommandList = new String[] { ".uno:Paste", ".uno:CloseWin" };
        impl_checkGetPreferredKeyEventsForCommandList(m_xGlobalAccelCfg, xPrimaryAccess, sCommandList);
    }

    //-------------------------------------------
    /** @todo document me.
     */
    public void checkModuleAccelCfg()
        throws java.lang.Exception
    {
        String[] sModules = new String[]
        {
            "com.sun.star.frame.StartModule",
            "com.sun.star.drawing.DrawingDocument",
            "com.sun.star.presentation.PresentationDocument",
            "com.sun.star.sheet.SpreadsheetDocument",
            "com.sun.star.text.TextDocument",
            // add other modules here
        };

        for (int i=0; i<sModules.length; ++i)
        {
            log.println("\n---- check accelerator configuration depending module: " + sModules[i] + " ----");

            PropertyValue[] aProp = new PropertyValue[2];
            aProp[0] = new PropertyValue();
            aProp[0].Name  = "ModuleIdentifier";
            aProp[0].Value = sModules[i];
            aProp[1] = new PropertyValue();
            aProp[1].Name  = "Locale";
            aProp[1].Value = "en-US";

            XInitialization xInit = (XInitialization)UnoRuntime.queryInterface(XInitialization.class, m_xModuleAccelCfg);
            xInit.initialize(aProp); // to fill cache

            XNameAccess xPrimaryModules = (XNameAccess)UnoRuntime.queryInterface(XNameAccess.class, m_xPrimaryKeys.getByName("Modules"));
            XNameAccess xSecondaryModules = (XNameAccess)UnoRuntime.queryInterface(XNameAccess.class, m_xSecondaryKeys.getByName("Modules"));

            String[] sKeys;
            XNameAccess xPrimaryAccess = (XNameAccess)UnoRuntime.queryInterface(XNameAccess.class, xPrimaryModules.getByName(sModules[i]));
            XNameAccess xSecondaryAccess = (XNameAccess)UnoRuntime.queryInterface(XNameAccess.class, xSecondaryModules.getByName(sModules[i]));

            //--------------------------------------------
            if (sModules[i].equals("com.sun.star.presentation.PresentationDocument"))
                sKeys = new String[] { "A_SHIFT_MOD1_MOD2" };
            else if (sModules[i].equals("com.sun.star.sheet.SpreadsheetDocument"))
                sKeys = new String[] { "B_MOD1" };
            else if (sModules[i].equals("com.sun.star.text.TextDocument"))
                sKeys = new String[] { "F11_MOD1" };
            else
                sKeys = new String[] { "A_MOD1" };
            impl_checkGetKeyCommands(m_xModuleAccelCfg, xPrimaryAccess, sKeys);

            //--------------------------------------------
            String[] sCommands;
            if (sModules[i].equals("com.sun.star.presentation.PresentationDocument"))
            {
                sKeys = new String[] { "A_SHIFT_MOD1_MOD2" };
                sCommands = new String[] { ".uno:test" };
            }
            else if (sModules[i].equals("com.sun.star.sheet.SpreadsheetDocument"))
            {
                sKeys = new String[] { "B_MOD1" };
                sCommands = new String[] { ".uno:test" };
            }
            else if (sModules[i].equals("com.sun.star.text.TextDocument"))
            {
                sKeys = new String[] { "F11_MOD1" };
                sCommands = new String[] { ".uno:test" };
            }
            else
            {
                sKeys = new String[] { "PASTE" };
                sCommands = new String[] { ".uno:test" };
            }
            impl_checkSetKeyCommands(m_xModuleAccelCfg, xPrimaryAccess, xSecondaryAccess, sKeys, sCommands);

            //--------------------------------------------
            if (sModules[i].equals("com.sun.star.presentation.PresentationDocument"))
                sKeys = new String[] { "A_SHIFT_MOD1_MOD2" };
            else if (sModules[i].equals("com.sun.star.sheet.SpreadsheetDocument"))
                sKeys = new String[] { "F5_SHIFT_MOD1" };
            else if (sModules[i].equals("com.sun.star.text.TextDocument"))
                sKeys = new String[] { "BACKSPACE_MOD2" };
            else
                sKeys = new String[] { "C_MOD1" };
            impl_checkRemoveKeyCommands(m_xModuleAccelCfg, xPrimaryAccess, xSecondaryAccess, sKeys);

            //--------------------------------------------
            String[] sCommandList;
            if (sModules[i].equals("com.sun.star.presentation.PresentationDocument"))
                sCommandList = new String[] { ".uno:Presentation" };
            else if (sModules[i].equals("com.sun.star.sheet.SpreadsheetDocument"))
                sCommandList = new String[] { ".uno:InsertCell" };
            else if (sModules[i].equals("com.sun.star.text.TextDocument"))
                sCommandList = new String[] { ".uno:SelectionModeBlock" };
            else
                sCommandList = new String[] { ".uno:Cut" };
            impl_checkGetPreferredKeyEventsForCommandList(m_xModuleAccelCfg, xPrimaryAccess, sCommandList);
        }
    }

    //-------------------------------------------
    /** @todo document me.
     */
    public void checkDocumentAccelCfg()
        throws java.lang.Exception
    {
        log.println("\n---- check Document accelerator configuration: ----");

        String sDocCfgName;

        sDocCfgName = "file:///c:/test.cfg";
        SaveDocumentAcceleratorConfiguration(sDocCfgName);

        sDocCfgName = "file:///c:/test.cfg";
        LoadDocumentAcceleratorConfiguration(sDocCfgName);
    }

    //-------------------------------------------
    /** @todo document me.
     */
    private void impl_checkGetKeyCommands(XAcceleratorConfiguration xAccelCfg, XNameAccess xAccess, String[] sKeys)
        throws java.lang.Exception
    {
        log.println("check getKeyCommands...");

        for (int i=0; i<sKeys.length; ++i)
        {
            if (xAccess.hasByName(sKeys[i]) && getCommandFromConfiguration(xAccess, sKeys[i]).length()>0)
            {
                log.println("** get command by " + sKeys[i] + " **");

                String sCmdFromCache = new String(); // get a value using XAcceleratorConfiguration API
                String sCmdFromConfiguration = new String(); // get a value using configuration API

                // GET shortcuts/commands using XAcceleratorConfiguration API
                sCmdFromCache = xAccelCfg.getCommandByKeyEvent(convertShortcut2AWTKey(sKeys[i]));
                log.println(sKeys[i] + "-->" + sCmdFromCache + ", by XAcceleratorConfiguration API");

                // GET shortcuts/commands using configuration API
                sCmdFromConfiguration = getCommandFromConfiguration(xAccess, sKeys[i]);
                log.println(sKeys[i] + "-->" + sCmdFromConfiguration + ", by configuration API");

                assure("values are different by XAcceleratorConfiguration API and configuration API!", sCmdFromCache.equals(sCmdFromConfiguration));

                String sLocale = "es";
                setOfficeLocale(sLocale);
                sCmdFromConfiguration = getCommandFromConfiguration(xAccess, sKeys[i]);
                log.println(sKeys[i] + "-->" + sCmdFromConfiguration + ", by configuration API" + " for locale:"+ getOfficeLocale());

                sLocale = "en-US";
                setOfficeLocale(sLocale); //reset to default locale
            }
            else
            {
                log.println(sKeys[i] + " doesn't exist!");
            }
        }
    }

    //-------------------------------------------
    /** @todo document me.
     */
    private void impl_checkSetKeyCommands(XAcceleratorConfiguration xAccelCfg, XNameAccess xPrimaryAccess, XNameAccess xSecondaryAccess, String[] sKeys, String[] sCommands)
        throws java.lang.Exception
    {
        log.println("check setKeyCommands...");

        for (int i=0; i<sKeys.length; ++i)
        {
            if (!xPrimaryAccess.hasByName(sKeys[i]) && !xSecondaryAccess.hasByName(sKeys[i]))
            {
                xAccelCfg.setKeyEvent(convertShortcut2AWTKey(sKeys[i]), sCommands[i]);
                xAccelCfg.store();
                if (xPrimaryAccess.hasByName(sKeys[i]))
                    log.println("add " + sKeys[i] + " successfully!");
                else
                    log.println("add " + sKeys[i] + " failed!");
            }
            else if (xPrimaryAccess.hasByName(sKeys[i]))
            {
                String sOriginalCommand = getCommandFromConfiguration(xPrimaryAccess, sKeys[i]);
                if (!sCommands[i].equals(sOriginalCommand))
                {
                    xAccelCfg.setKeyEvent(convertShortcut2AWTKey(sKeys[i]), sCommands[i]);
                    xAccelCfg.store();

                    String sChangedCommand = getCommandFromConfiguration(xPrimaryAccess, sKeys[i]);
                    if (sCommands[i].equals(sChangedCommand))
                        log.println("change " + sKeys[i] + " successfully!");
                    else
                        log.println("change " + sKeys[i] + " failed!");
                }
                else
                    log.println(sKeys[i] + " already exist!");
            }
            else if (xSecondaryAccess.hasByName(sKeys[i]))
            {
                String sOriginalCommand = getCommandFromConfiguration(xSecondaryAccess, sKeys[i]);
                if (!sCommands[i].equals(sOriginalCommand))
                {
                    xAccelCfg.setKeyEvent(convertShortcut2AWTKey(sKeys[i]), sCommands[i]);
                    xAccelCfg.store();

                    String sChangedCommand = getCommandFromConfiguration(xPrimaryAccess, sKeys[i]);
                    if (sCommands[i].equals(sChangedCommand))
                        log.println("change " + sKeys[i] + " successfully!");
                    else
                        log.println("change " + sKeys[i] + " failed!");
                }
                else
                    log.println(sKeys[i] + " already exist!");
            }
        }
    }

    //-------------------------------------------
    /** @todo document me.
     */
    private void impl_checkRemoveKeyCommands(XAcceleratorConfiguration xAccelCfg, XNameAccess xPrimaryAccess, XNameAccess xSecondaryAccess, String[] sKeys)
        throws java.lang.Exception
    {
        log.println("check removeKeyCommands...");

        for (int i=0; i<sKeys.length; i++)
        {
            if (!xPrimaryAccess.hasByName(sKeys[i]) && !xSecondaryAccess.hasByName(sKeys[i]))
            {
                log.println(sKeys[i] + " doesn't exist!");
            }
            else if (xPrimaryAccess.hasByName(sKeys[i]))
            {
                xAccelCfg.removeKeyEvent(convertShortcut2AWTKey(sKeys[i]));
                xAccelCfg.store();
                if (!xPrimaryAccess.hasByName(sKeys[i]))
                    log.println("Remove " + sKeys[i] + " successfully!");
                else
                    log.println("Remove " + sKeys[i] + " failed!");
            }
            else if (xSecondaryAccess.hasByName(sKeys[i]))
            {
                xAccelCfg.removeKeyEvent(convertShortcut2AWTKey(sKeys[i]));
                xAccelCfg.store();
                if (!xSecondaryAccess.hasByName(sKeys[i]))
                    log.println("Remove " + sKeys[i] + " successfully!");
                else
                    log.println("Remove " + sKeys[i] + " failed!");
            }
        }
    }

    //-------------------------------------------
    /** @todo document me.
     */
    private void impl_checkGetPreferredKeyEventsForCommandList(XAcceleratorConfiguration xAccelCfg, XNameAccess xPrimaryAccess, String[] sCommandList)
        throws java.lang.Exception
    {
        log.println("check getPreferredKeyEventsForCommandList...");

        Object[] oKeyEvents = xAccelCfg.getPreferredKeyEventsForCommandList(sCommandList);
        for (int i = 0; i < oKeyEvents.length; i++)
        {
            log.println("get preferred key for command "+ sCommandList[i] + ":");

            KeyEvent aKeyEvent = (KeyEvent)AnyConverter.toObject(KeyEvent.class, oKeyEvents[i]);
            String sKeyEvent = convertAWTKey2Shortcut(aKeyEvent);
            log.println(sKeyEvent);

            String sCmdFromConfiguration = getCommandFromConfiguration(xPrimaryAccess, sKeyEvent);
            log.println(sCmdFromConfiguration);
            if (sCommandList[i].equals(sCmdFromConfiguration))
                log.println("get preferred key correctly!");
            else
                log.println("get preferred key failed!");
        }
    }

    //-------------------------------------------
    /** @todo document me.
     */
    private String getCommandFromConfiguration(XNameAccess xAccess, String sKey)
        throws java.lang.Exception
    {
        String sCommand = new String();

        if (xAccess.hasByName(sKey))
        {
            XNameAccess xKey = (XNameAccess)UnoRuntime.queryInterface(XNameAccess.class, xAccess.getByName(sKey));
            XNameAccess xCommand = (XNameAccess)UnoRuntime.queryInterface(XNameAccess.class, xKey.getByName("Command"));

            String sLocale = getOfficeLocale();
            if (xCommand.hasByName(sLocale))
                sCommand = (String)UnoRuntime.queryInterface(String.class, xCommand.getByName(sLocale));
        }

        return sCommand;
    }

    //-------------------------------------------
    /** @todo document me.
     */
    private void insertKeyToConfiguration(XNameAccess xAccess, String sKey, String sCommand)
        throws java.lang.Exception
    {
        XNameContainer xContainer = (XNameContainer)UnoRuntime.queryInterface(XNameContainer.class, xAccess);
        if (!xContainer.hasByName(sKey))
        {
            XSingleServiceFactory xFac = (XSingleServiceFactory)UnoRuntime.queryInterface(XSingleServiceFactory.class, xContainer);
            XInterface xInst = (XInterface)UnoRuntime.queryInterface(XInterface.class, xFac.createInstance());
            xContainer.insertByName(sKey, xInst);
        }

        XNameAccess xKey = (XNameAccess)UnoRuntime.queryInterface(XNameAccess.class, xContainer.getByName(sKey));
        XNameContainer xCommand = (XNameContainer)UnoRuntime.queryInterface(XNameContainer.class, xKey.getByName("Command"));
        String sLocale = getOfficeLocale();
        if (xCommand.hasByName(sLocale))
            xCommand.insertByName(sLocale, sCommand);
        else
            xCommand.replaceByName(sLocale, sCommand);
    }

    //-------------------------------------------
    /** @todo document me.
     */
    private void removeKeyFromConfiguration(XNameAccess xAccess, String sKey)
        throws java.lang.Exception
    {
        XNameContainer xContainer = (XNameContainer)UnoRuntime.queryInterface(XNameContainer.class, xAccess);
        if (xContainer.hasByName(sKey))
            xContainer.removeByName(sKey);
    }

    //-------------------------------------------
    /** @todo document me.
     */
    private void LoadDocumentAcceleratorConfiguration(String sDocCfgName)
        throws java.lang.Exception
    {
        XSingleServiceFactory xStorageFactory = (XSingleServiceFactory)UnoRuntime.queryInterface(
            XSingleServiceFactory.class,
            m_xSmgr.createInstance("com.sun.star.embed.StorageFactory"));

        Object aArgs[] = new Object[2];
        aArgs[0] = sDocCfgName;
        aArgs[1] = new Integer(com.sun.star.embed.ElementModes.READ);
        XStorage xRootStorage = (XStorage)UnoRuntime.queryInterface(
            XStorage.class,
            xStorageFactory.createInstanceWithArguments(aArgs));

        XStorage xUIConfig = xRootStorage.openStorageElement("Configurations2", com.sun.star.embed.ElementModes.READ);

        PropertyValue aProp = new PropertyValue();
        aProp.Name = "DocumentRoot";
        aProp.Value = xUIConfig;
        Object[] lArgs = new Object[1];
        lArgs[0] = aProp;

        XInitialization xInit = (XInitialization)UnoRuntime.queryInterface(XInitialization.class, m_xDocumentAccelCfg);
        xInit.initialize(lArgs);

        String test = m_xDocumentAccelCfg.getCommandByKeyEvent(convertShortcut2AWTKey("F2"));
        log.println(test);
    }

    //-------------------------------------------
    /** @todo document me.
     */
    private void SaveDocumentAcceleratorConfiguration(String sDocCfgName)
        throws java.lang.Exception
    {
        XSingleServiceFactory xStorageFactory = (XSingleServiceFactory)UnoRuntime.queryInterface(
            XSingleServiceFactory.class,
            m_xSmgr.createInstance("com.sun.star.embed.StorageFactory"));

        Object aArgs[] = new Object[2];
        aArgs[0] = sDocCfgName;
        aArgs[1] = new Integer(com.sun.star.embed.ElementModes.WRITE);
        XStorage xRootStorage = (XStorage)UnoRuntime.queryInterface(
            XStorage.class,
            xStorageFactory.createInstanceWithArguments(aArgs));

        XStorage xUIConfig = xRootStorage.openStorageElement("Configurations2", com.sun.star.embed.ElementModes.WRITE);

        XUIConfigurationManager xCfgMgr = (XUIConfigurationManager)UnoRuntime.queryInterface(
            XUIConfigurationManager.class,
            m_xSmgr.createInstance("com.sun.star.ui.UIConfigurationManager"));

        XUIConfigurationStorage xUICfgStore = (XUIConfigurationStorage)UnoRuntime.queryInterface(
            XUIConfigurationStorage.class,
            xCfgMgr);
        xUICfgStore.setStorage(xUIConfig);

        XPropertySet xUIConfigProps = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xUIConfig);
        xUIConfigProps.setPropertyValue("MediaType", "application/vnd.sun.xml.ui.configuration");

        if (xCfgMgr != null)
        {
            XAcceleratorConfiguration xTargetAccMgr = (XAcceleratorConfiguration)UnoRuntime.queryInterface(
                XAcceleratorConfiguration.class,
                xCfgMgr.getShortCutManager());
            XUIConfigurationPersistence xCommit1 = (XUIConfigurationPersistence)UnoRuntime.queryInterface(
                XUIConfigurationPersistence.class, xTargetAccMgr);
            XUIConfigurationPersistence xCommit2 = (XUIConfigurationPersistence)UnoRuntime.queryInterface(
                XUIConfigurationPersistence.class, xCfgMgr);
            xCommit1.store();
            xCommit2.store();

            XTransactedObject xCommit3 = (XTransactedObject)UnoRuntime.queryInterface(
                XTransactedObject.class, xRootStorage);
            xCommit3.commit();
        }
    }

    //-------------------------------------------
    /** @todo document me.
     */
    private com.sun.star.awt.KeyEvent convertShortcut2AWTKey(String sShortcut)
        throws java.lang.Exception
    {
        com.sun.star.awt.KeyEvent aKeyEvent = new com.sun.star.awt.KeyEvent();
        KeyMapping aKeyMapping = new KeyMapping();
        String[] sShortcutSplits = sShortcut.split("_");

        aKeyEvent.KeyCode = aKeyMapping.mapIdentifier2Code(sShortcutSplits[0]);
        for (int i = 1; i < sShortcutSplits.length; i++)
        {
            if (sShortcutSplits[i].equals("SHIFT"))
                aKeyEvent.Modifiers |= com.sun.star.awt.KeyModifier.SHIFT;
            else if (sShortcutSplits[i].equals("MOD1"))
                aKeyEvent.Modifiers |= com.sun.star.awt.KeyModifier.MOD1;
            else if (sShortcutSplits[i].equals("MOD2"))
                aKeyEvent.Modifiers |= com.sun.star.awt.KeyModifier.MOD2;
        }

        return aKeyEvent;
    }

    //-------------------------------------------
    /** @todo document me.
     */
    private String convertAWTKey2Shortcut(com.sun.star.awt.KeyEvent aKeyEvent)
        throws java.lang.Exception
    {
        String sShortcut;

        KeyMapping aKeyMapping = new KeyMapping();
        sShortcut = aKeyMapping.mapCode2Identifier(aKeyEvent.KeyCode);

        if ((aKeyEvent.Modifiers & com.sun.star.awt.KeyModifier.SHIFT) == com.sun.star.awt.KeyModifier.SHIFT)
            sShortcut += "_SHIFT";
        if ((aKeyEvent.Modifiers & com.sun.star.awt.KeyModifier.MOD1) == com.sun.star.awt.KeyModifier.MOD1)
            sShortcut += "_MOD1";
        if ((aKeyEvent.Modifiers & com.sun.star.awt.KeyModifier.MOD2) == com.sun.star.awt.KeyModifier.MOD2)
            sShortcut += "_MOD2";

        return sShortcut;
    }

    //-------------------------------------------
    /** @todo document me.
     */
    private String getOfficeLocale()
        throws java.lang.Exception
    {
        String sLocale = new String();

        String sConfigPath = "org.openoffice.Setup";
        boolean bReadOnly = true;
        XNameAccess xRootConfig = openConfig(m_xSmgr, sConfigPath, bReadOnly);

        if (xRootConfig != null)
        {
            XNameAccess xLocale = (XNameAccess)UnoRuntime.queryInterface(XNameAccess.class, xRootConfig.getByName("L10N"));
            XPropertySet xSet = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xLocale);
            sLocale = (String)xSet.getPropertyValue("ooLocale");
        }

        return sLocale;
    }

    //-------------------------------------------
    /** @todo document me.
     */
    private void  setOfficeLocale(String sLocale)
        throws java.lang.Exception
    {
        String sConfigPath = "org.openoffice.Setup";
        boolean bReadOnly = false;
        XNameAccess xRootConfig = openConfig(m_xSmgr, sConfigPath, bReadOnly);

        if (xRootConfig != null)
        {
            XNameAccess xLocale = (XNameAccess)UnoRuntime.queryInterface(XNameAccess.class, xRootConfig.getByName("L10N"));
            XPropertySet xSet = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xLocale);
            xSet.setPropertyValue("ooLocale", sLocale);
            XChangesBatch xBatch = (XChangesBatch)UnoRuntime.queryInterface(XChangesBatch.class, xRootConfig);
            xBatch.commitChanges();
        }
    }

    //-------------------------------------------
    /** @todo document me.
     */
    private XNameAccess openConfig(XMultiServiceFactory xSMGR,
                            String               sConfigPath ,
                            boolean              bReadOnly   )
        throws java.lang.Exception
    {
        XMultiServiceFactory xConfigRoot = (XMultiServiceFactory)UnoRuntime.queryInterface(
                                                XMultiServiceFactory.class,
                                                xSMGR.createInstance("com.sun.star.configuration.ConfigurationProvider"));

        PropertyValue[] lParams = new PropertyValue[2];
        lParams[0] = new PropertyValue();
        lParams[0].Name = "nodepath";
        lParams[0].Value = sConfigPath;

        lParams[1] = new PropertyValue();
        lParams[1].Name = "locale";
        lParams[1].Value = "*";

        Object aConfig;
        if (bReadOnly)
            aConfig = xConfigRoot.createInstanceWithArguments(
                            "com.sun.star.configuration.ConfigurationAccess",
                            lParams);
        else
            aConfig = xConfigRoot.createInstanceWithArguments(
                            "com.sun.star.configuration.ConfigurationUpdateAccess",
                            lParams);

        XNameAccess xConfig = (XNameAccess)UnoRuntime.queryInterface(
                            XNameAccess.class,
                            aConfig);

        if (xConfig == null)
            throw new com.sun.star.uno.Exception("Could not open configuration \"" + sConfigPath + "\"");

        return xConfig;
    }
}
